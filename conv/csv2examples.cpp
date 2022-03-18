#include "csv2examples.hpp"
#include "csv2examples.h"
#include "conv_features.h"
#include "convParamxx.h" // ConvParam from string
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring> // strcmp
#include <cmath>
#include <exception>
#include <map>
#include <regex>

/** \file .csv file read impl
 * Provides both the C and the C++ api.
 * The C functions are implemented in terms of the C++ code.
 */

#define RTERR(string) throw(std::runtime_error(string))

// These were moved into convParam.h, as 'C' variables
#define hi_dim ::allFeature_sz
#define floats_names ::allFeature_names
#define hi_dim_values ::subFeature_values_sz

namespace expt
{

using std::vector;
using std::set;
using std::string;
using std::map;
using std::regex;
using std::left;
using std::setw;
// why not std::regex_search, std::regex_match, std::regex_replace?

using std::cout;
using std::endl;
using std::sort;
using std::remove_if;
using std::ifstream;
using std::istringstream;
using std::getline;

// for floats printed to a certain number of digits,
// what is their max precision (i.e. lowest "10^n" unit).
//
// f=0.00473 fixed_point_precision(f)=1e-05
// f=0.004 fixed_point_precision(f)=0.001
// f=1.23 fixed_point_precision(f)=0.01
// f=0.001 fixed_point_precision(f)=0.001
// f=nan fixed_point_precision(f)=1
// f=inf fixed_point_precision(f)=1
static float fixed_point_precision(float f){
    f = fabs(f);
    int pscale = (int)floor(log10(f));
    float scale = pow(10.0, -pscale);
    f = f * scale; // so "done" threshold can be constant.

    int p;
    float prec;
    float diff = 1.0f;
    int iter=0;
    while( diff > 1e-5 ){
        if (++iter > 10){
            RTERR("failed to find fixed_point_precision");
            break;
        }
        // precision estimate, a power of ten
        p = (int)floor(log10(f));
        prec = pow(10.0, p);
        // is (remaining) f almost exactly a multiple of prec?
        float fapprox = (int)(f/prec) * prec;
        diff = fabs(f - fapprox);
        f = diff; // Remove some most-significant digit, and repeat
    }
    return pow(10.0, p+pscale);
}

static float min_nonzero_abs(std::vector<float> const& vf){
    size_t i=0U;
    float a = 0.0f; // return value if vf is empty or all-zero
    // init a as first nonzero abs we encounter
    for( ; i<vf.size(); ++i){
        if(vf[i]!=0){
            a = fabs(vf[i]);
            ++i;
            break;
        }
    }
    // continue looking for a smaller nonzero abs value
    for( ; i<vf.size(); ++i){
        float v = fabs(vf[i]);
        if(v<a && v>0.0f){
            a = v;
        }
    }
    return a;
}

/** safe check of opaque pointer and conversion to underlying C++ object.*/
AllRunsxx * cvt_or_throw(AllRuns *ar){
    if (ar==nullptr || ar->opaque==nullptr)
        RTERR("ERROR: unusable AllRuns* -- ar or ar->opaque was NULL");
    return static_cast<AllRunsxx*>(ar->opaque);
}
AllRunsxx const* cvt_or_throw(AllRuns const* ar){
    if (ar==nullptr || ar->opaque==nullptr)
        RTERR("ERROR: unusable AllRuns* -- ar or ar->opaque was NULL");
    return static_cast<AllRunsxx*>(ar->opaque);
}


enum class CSVState {
    UnquotedField,
    QuotedField,
    QuotedQuote         // 2nd " is either last or ("" that means single-quote)
};

std::vector<std::string> readCSVRow(const std::string &row) {
    CSVState state = CSVState::UnquotedField;
    std::vector<std::string> fields {""};
    size_t i = 0; // index of the current field
    for (char c : row) {
        switch (state) {
            case CSVState::UnquotedField:
                switch (c) {
                    case ',': // end of field
                              fields.push_back(""); i++;
                              break;
                    case '"': state = CSVState::QuotedField;
                              break;
                    default:  fields[i].push_back(c);
                              break; }
                break;
            case CSVState::QuotedField:
                switch (c) {
                    case '"': state = CSVState::QuotedQuote;
                              break;
                    default:  fields[i].push_back(c);
                              break; }
                break;
            case CSVState::QuotedQuote:
                switch (c) {
                    case ',': // , after closing quote
                              fields.push_back(""); i++;
                              state = CSVState::UnquotedField;
                              break;
                    case '"': // "" -> "
                              fields[i].push_back('"');
                              state = CSVState::QuotedField;
                              break;
                    default:  // end of quote
                              state = CSVState::UnquotedField;
                              break; }
                break;
        }
    }
    return fields;
}

// construct from std::string data (for testing)
AllRunsxx::AllRunsxx(std::string s) : AllRunsxx( istringstream(s) ) {
    if(1) {
        cout<<"+AllRunsxx(string)"<<endl;
    }
}
// construct from csv-filename
AllRunsxx::AllRunsxx(char const *fname) : AllRunsxx( std::ifstream{fname} ) {
    if(1) {
        cout<<"+AllRunsxx(fname="<<fname<<")"<<endl;
    }
}

/** Parser for this constructor could be much faster.
 *
 * \todo: dynamically track the factor values as things are
 * being read in.  This allows future OneRunRaw strings to
 * point into factor memory, instead of needing to be separately
 * malloced/freed.  \b OR hide the OneRunRaw and keep it as
 * temporary internal detail?   But get rid of strdup and its
 * malloc/free hassles!
 *
 */
AllRunsxx::AllRunsxx(std::istream&& in)
    : status(0)
    , filtered(false)
    , massaged(false)
    , grouped(false)
    , vrr()
    , group_lo()
    , setParams()
    , setNames()
    , setLayertypes()
    , setLayers()
    , factorParam()
    , factorLayertype()
    , factorLayer()
    , fprec_ms(0.0f)
    , fprec_gf(0.0f)
    , ms_gf_nonzero(false)
    , expts()
{
    if(!in.good()) // (use "is_open", for ifstream)
        RTERR("AllRunsxx constructor given bad input stream");
    // Gotcha:  do NOT loop as
    //    while(!in.eof()){
    //        getline(in,row);
    //    }
    // - eof() might never be true.
    // - need also to check failbit and badbit
    std::string row;
    char const *accepted_line1 =
        "param,name,layertype,layer,best,reps,threads,ms,Gflops,err,ops";
    int line = 0; // for error report?
    while(getline(in,row)){ // bool conversion of getline ret val checks errors
        ++line;
        if(row.size() == 0 || row[0] == '#'){
            continue; // skip empty or #-comments
        }
        if(row == accepted_line1){
            break;
        }
        // first significant line (column names) was correct
        cout<<"Got line: <"<<row<<'>'<<endl;
        cout<<"Expected: "<<accepted_line1<<endl;
        RTERR("unrecognized conv .csv data file");
    }
    while(getline(in,row)) {
        ++line;
        // read line, convert to string fields
        if(row.size() == 0 || row[0] == '#')
            continue; // skip empty or #-comments
        auto fields = readCSVRow(row);
        if(fields.size() != 11){
            cout<<"Cols : "<<accepted_line1<<endl;
            cout<<"Line : <"<<row<<'>'<<endl;
            cout<<"Line parse error.  Fields:"<<endl;
            for(size_t i=0; i<fields.size(); ++i){
                cout<<"   "<<i<<": <"<<fields[i]<<">"<<endl;
            }
            RTERR(string(".csv expected 10 fields"));
        }
        // convert to OneRunRaw 'C' struct
        // invalid conversions will throw.
        //
        // Our std::set trackers are insert-only --
        // this doesn't invalidate iterators or refs, so we can point to values
        // as const char* and they will remain valid for AllRunsxx life.
        // This means `vrr` need not free this string memory.
        // It also means I avoid 'strdup' and its malloc/free issues.
        //   (Issue turns into "lifetime of this AllRunsxx" object)
        OneRunRaw rr;
        //string s;
        //rr.params = strdup(fields[0].c_str());
        {
            auto const it_bool = setParams.insert(string(fields[0]));
            rr.params = it_bool.first->c_str();
        }
        {
            auto const it_bool = setNames.insert(string(fields[1]));
            rr.name = it_bool.first->c_str();
        }
        {
            if(fields[2].size() != 1)
                RTERR("Expected layertype to be a single char");
            char c = fields[2][0];
            setLayertypes.insert(c);
            rr.layertype = c;
        }
        {
#if 1 // 'layer' impl name shortening (default)
            // MUST be done in constructor, for string memory reasons.
            // layer name shortenings are NOT stored.
            // translation via C api 'layer_short' is fastest
            int const bufsz=100; char buf[bufsz];
            layer_short(fields[3].c_str(), buf, bufsz);
            //cout<<left<<setw(50)<<fields[3]<<' '<<left<<setw(20)<<buf;
            //auto const it_bool = setLayers.insert(string(&buf[0]));
            char const* pbuf = &buf[0];
            auto const it_bool = setLayers.insert(string{pbuf});
            //cout<<' '<<*it_bool.first;
            //cout<<endl;
#else // no 'layer' impl name shortening
            char const* pbuf = fields[3].c_str();
            auto const it_bool = setLayers.insert(string{pbuf});
#endif
            rr.layer = it_bool.first->c_str();
        }
        rr.best = stoi(fields[4]);
        rr.reps = stoi(fields[5]);
        rr.threads = stoi(fields[6]);
        rr.ms = stof(fields[7]);
        rr.gflops = stof(fields[8]);
        rr.err = stof(fields[9]);
        rr.ops = stof(fields[10]);

        // save it (FIXME maybe we don't need to save it?)
        vrr.push_back(rr);

        // We have set up char const* into our grow-only 'set's.
        // factor-->int vector is set up during (after) 'filter'
    }
    if(0) {
        cout<<" shortened setLayers? ";
        for(auto & x: setLayers){ cout<<' '<<x; }
        cout<<endl;
    }
    if(in.bad())
        RTERR("Error while reading AllRunsxx input");
}

/** Fudge factor for omp parallelism of `mb` work units for available omp threads.
 * Taken from jitconv.cpp
 *
 * \b Very approximately, ops_factor * ops / (ops/us) yields a value
 * that can be compared to some CONST to decide if parallelism might be
 * good/bad.   Roughly, CONST ~ (omp overhead) * (ops/us).
 *
 * @param mb number of ||izable work units (minibatch, for VEDNN_WRAP_DEFAULT)
 * @return a multiplier for number of operations to help determine when to
 *         parallelize the `mb` work units.
 */
/** expand ConvParamRaw into float vector dest[hi_dim].
 * @pre dest non-NULL has room for hi_dim floats.
 *
 * Many of these features were from a small decision
 * tree.  After network pruning, it might be possible
 * to expand/prune this base set of features.
 *
 * Each will form a channel vector of width hi_dim.
 * Each experiment has "layers" number of channels.
 * Absent layers get zero-vectors
 * Present layers get vectors from here.
 *
 * \sa `floats_names`
 *
 * \todo Finetune boolean feature thresholds.  These might use a second
 * set of generic feature thresholds, say one or two per (16?,32?,48?) basic
 * features, subject to non-triviality, non-equality conditions.  These can
 * be trivially evaluated by VE vector ops.
 *
 * boolean prediction needed many more features!
 * in branch dev-gemmtree of vednn, have "_X" serial version thresholds:
 * doItr:
 *      float mbfactor                      = ops_factor(pConv->pParamIn->batch);
 *      float const ops_mbpara              = pConv->ops * mbfactor;
 *      float const ops_mbpara_force_unwrap = 8.e7;
 *      if( doItr_xw
 *              && actual0->wrap == VEDNN_WRAP_DEFAULT
 *              && pConv->pParamIn->batch > 1
 * --> missing: && ops_mbpara <= ops_mbpara_force_unwrap
 *              && vednn_get_num_threads() > 1 ){
 *                      nwrap = 2;
 * doJit:
 *      
 */
#if 0 // original, private func, now moved to convParam.h
static void floats(ConvParamRaw const& c,
        int threads,
        double ops,
        float *dest)
{
    size_t i=0U;
    // Raw conv params (missing = dirn, layout)
    // These are converted STRICTLY POSITIVE floats
    dest[i++] = (float)c.batchNum;
    dest[i++] = (float)c.group;
    dest[i++] = (float)c.inChannel;
    dest[i++] = (float)c.inHeight;
    dest[i++] = (float)c.inWidth;
    dest[i++] = (float)c.outChannel;
    dest[i++] = (float)c.outHeight;
    dest[i++] = (float)c.outWidth;
    dest[i++] = (float)c.kernHeight;
    dest[i++] = (float)c.kernWidth;
    dest[i++] = (float)c.strideHeight;
    dest[i++] = (float)c.strideWidth;
    dest[i++] = (float)(c.padHeight+1);
    dest[i++] = (float)(c.padWidth+1);
    dest[i++] = (float)(c.dilationHeight-c.dil_start+1);
    dest[i++] = (float)(c.dilationWidth -c.dil_start+1);
    assert( i == 16 );
#if 1
    auto const ibase = i;
#ifndef NDEBUG
    for(size_t j=0; j<i; ++j){
        if(dest[j] <= 0.0)
            RTERR("base floats not strictly positive!");
    }
#endif
    // inverses of above
    for( ; i<ibase+ibase; ++i){
        dest[i] = 1.0/dest[i-ibase];
    }
    assert(i == ibase + ibase);
#endif
    // misc other "important factors"
    // mb, booleans
    /* 32 */ dest[i++] = (float)(c.batchNum == 1);
    dest[i++] = (float)(c.batchNum > 1 && c.batchNum <= 8);
    dest[i++] = (float)(c.batchNum < 8);
    // mbic, mbocosz
    dest[i++] = (float)(c.batchNum * c.inChannel);
    dest[i++] = (float)(c.batchNum * c.inChannel < 208);
    int mbocosz = c.batchNum * c.outChannel
        * c.outHeight * c.outWidth;
    dest[i++] = (float)mbocosz;
    dest[i++] = (float)(mbocosz < 5152);
    dest[i++] = (float)(mbocosz < 240384); // !
    dest[i++] = (float)(mbocosz < 284736); // !
    // new, other things for vednnConvolutionForward.c (or vednnConvolutionOk.c)
    //assert(i==41);
    /* 41 */ dest[i++] = (float)(c.inChannel % 1024 == 0); // (not req'd?) used for Fd1s1pSk3_c1024x_T
    dest[i++] = (float)(c. inChannel / c.group <= 1024);
    dest[i++] = (float)(c.outChannel / c.group <= 256);
    dest[i++] = (float)(c.inWidth <= 256);
    dest[i++] = (float)(c.outHeight * c.outWidth <= 16); // part of vecC test
    dest[i++] = (float)(c.outHeight * c.outWidth <= 4096); // from _ok.c
    dest[i++] = (float)(c.outHeight * c.outWidth <= 16
            || (c.outHeight * c.outWidth < 64
                && c.outHeight * c.outWidth <= c.inChannel)); // full vecC condition
    dest[i++] = (float)((c. inWidth & 0x1) == 0); //iw2X
    dest[i++] = (float)((c.outWidth & 0x1) == 0); //ow2X
    dest[i++] = (float)(c. inChannel == c.group); //ic1
    dest[i++] = (float)(c.outHeight == (c.inHeight - c.kernHeight) / c.strideHeight + 1);
    dest[i++] = (float)(c.outWidth == (c.inWidth - c.kernWidth) / c.strideWidth + 1);
    // ow, stride
    //assert(i==51);
    /* 53 */ dest[i++] = (float)(c.outWidth <= 64); // 48? 53?
    dest[i++] = (float)(c.outWidth <= 128); // 256?
    int const sh = c.strideHeight, sw = c.strideWidth;
    int const ssz = sh*sw;
    dest[i++] = (float)(ssz);
    dest[i++] = (float)(ssz == 1); // sh==sw==1
    dest[i++] = (float)(sh==2 && sw==2); // new
    dest[i++] = (float)(sw==1); // new, for gemm.1sw1
    // kernel bools
    int const kh = c.kernHeight, kw = c.kernWidth;
    bool ksq = (kh == kw);
    //assert(i==57);
    /* 59 */ dest[i++] = (float)(ksq); // ksq
    dest[i++] = (float)(ksq && kh==1);
    dest[i++] = (float)(ksq && kh==2);
    dest[i++] = (float)(ksq && kh==3);
    dest[i++] = (float)(ksq && kh==4);
    dest[i++] = (float)(ksq && kh==5);
    int ksz = kh * kw;
    dest[i++] = (float)(ksz);
    dest[i++] = (float)(ksz < 3); // from tree!
    // pad bools
    int ph=c.padHeight, pw=c.padWidth;
    bool psq = (ph == pw);
    /* 67 */dest[i++] = (float)psq;
    dest[i++] = (float)(ph==0 && pw==0); // p0
    dest[i++] = (float)(psq && ksq && kh == ph*2+1); // psqS (remove?) XXX
    //dest[i++] = (float)(kh == ph*2 + 1); //phS  (new)
    //dest[i++] = (float)(kw == pw*2 + 1); //pwS  (new
    dest[i++] = (float)(ph == (kh-1)/2 && pw == (kw-1)/2); // phS & pwS for _padsame
    //dest[i++] = (float)(pw == (kw-1)/2 ); // pwS
    /* 71 */ dest[i++] = (float)(c.outHeight == c.inHeight && c.outWidth == c.inWidth);
    //dest[i++] = (float)(c.outWidth  == c.inWidth );
    // Note: d1s1pS has strangeness for even kernel sizes (asymmetric padding
    //       is required to not keep same output size).
    //       I notice dil1_str1_padsame_ker2 is never run (it may have incorrect code!)

    // Some attributes corresponded to now-removed gemm "species" selection
    //
    // Since gemm impls are internally decided, should NOT try to predict
    // which gemm impls actually got used. XXX
    //
    // kernHeight+kernWidth==2 && padHeight+padWidth==0 && strideHeight+strideWidth>2 // k1p0_str
    //dest[i++] = (float)(kh+kw==2 && ph+pw==0 && sh+sw>2); // k1p0_str
    // channels < 2*nThreads && channels%nThreads && channels*output_h > 1 
    //dest[i++] = (float)((c.inChannel / c.group) * c.outHeight > 1);
    // (kernWidth == 1 && kernHeight == 1 && strideWidth == 1 && strideHeight == 1 && padWidth == 0 && padHeight == 0) // no_im2col
    //dest[i++] = (float)(c.kernWidth==1 && c.kernHeight==1
    //        && c.strideWidth==1 && c.strideHeight==1
    //        && c.padWidth==0 && c.padHeight==0);

    // dil bools
    if(1){ 
        int dh=c.dilationWidth - c.dil_start + 1;
        int dw=c.dilationHeight - c.dil_start + 1;
        int no_dil = (dh==1 && dw==1);
        dest[i++] = no_dil;
    }else{
        dest[i++] = 13.13; // TESTING 123
    }

    // misc: threads, layout, propKind, ....
    /* 73 */ dest[i++] = threads;
    dest[i++] = (float)(threads==1);
    dest[i++] = (float)(threads>1);
    dest[i++] = ops * 1.e-9;
    float const ops_mbpara = ops * ::ops_factor(c.batchNum, threads);
    dest[i++] = ops_mbpara * 1.e-9;
    dest[i++] = ops_mbpara < 8.e7;      // doItr threshold for "foo_X" unthreaded
    dest[i++] = ops_mbpara < 1.e10;     // doJit threshold "foo_X"


    // dilation wasn't a decision factor in small tree?
    //assert( i == hi_dim );
    if (i != (unsigned)hi_dim) {
        std::cout<<"WRONG hi_dim="<<hi_dim<<" but i="<<i<<std::endl;
        throw "programmer error in floats(ConvParamRaw,float*)";
    }

    //
    // double-check code match...
    //
    if(1){
        // features via new 'C' interface:
        int idx[80];            // 0,1,...,79
        float vals[80];
        for(int i=0; i<80; ++i)
            idx[i] = i;
        int err = allFeature_extract( &c, idx, 80,
                threads, ops, vals);
        if (!err) {
            for(int i=0; i<80; ++i) {
                // vals[i] approx == dest[i]
                if (vals[i] != dest[i]){
                    ++err;
                    cout<<"original C++ floats did not match "
                            "allFeatures_extract for feature "
                            <<i<<endl;
                }
            }
        }
        // Now check the streamlined 'C' version of this function
        err = allFeatures( &c, threads, ops, vals);
        if (!err) {
            for(int i=0; i<80; ++i) {
                // vals[i] approx == dest[i]
                if (vals[i] != dest[i]){
                    ++err;
                    cout<<"original C++ floats did not match "
                            "allFeatures for feature "
                            <<i<<endl;
                }
            }
        }
        if (err) cout<<"new C api might have issues!"<<endl;
        else cout<<"new C api allFeature_extract OK!"<<endl;
        assert(err==0);
    }
}
#else
// punt to 'C' api
static void floats(ConvParamRaw const& c,
        int threads,
        double ops,
        float *dest)
{
    /*err = */ allFeatures( &c, threads, ops, dest );
}
#endif

static void floats_values(ConvParamRaw const& c,
        int threads,
        double ops,
        float *dest)
{
    size_t i=0U;
    // Raw conv params (missing = dirn, layout)
    // These are converted STRICTLY POSITIVE floats
    dest[i++] = (float)c.batchNum;
    dest[i++] = (float)c.group;
    dest[i++] = (float)c.inChannel;
    dest[i++] = (float)c.inHeight;
    dest[i++] = (float)c.inWidth;
    dest[i++] = (float)c.outChannel;
    dest[i++] = (float)c.outHeight;
    dest[i++] = (float)c.outWidth;
    dest[i++] = (float)c.kernHeight;
    dest[i++] = (float)c.kernWidth;
    dest[i++] = (float)c.strideHeight;
    dest[i++] = (float)c.strideWidth;
    dest[i++] = (float)(c.padHeight+1);
    dest[i++] = (float)(c.padWidth+1);
    dest[i++] = (float)(c.dilationHeight-c.dil_start+1);
    dest[i++] = (float)(c.dilationWidth -c.dil_start+1);
    assert( i == 16 );
#if 1
    auto const ibase = i;
#ifndef NDEBUG
    for(size_t j=0; j<i; ++j){
        if(dest[j] <= 0.0)
            RTERR("base floats not strictly positive!");
    }
#endif
    // inverses of above
    for( ; i<ibase+ibase; ++i){
        dest[i] = 1.0/dest[i-ibase];
    }
    assert(i == ibase + ibase);
#endif
    if (i != (size_t)hi_dim_values) {
        std::cout<<"WRONG hi_dim="<<hi_dim<<" but i="<<i<<std::endl;
        throw "programmer error in floats(ConvParamRaw,float*)";
    }
}


static int const exptFloat_pad = 128;
ExptFloat* AllRunsxx::exptFloat_alloc(std::string xwhat/*="default"*/) const
{
    int in_dim  = hi_dim; // ok for (xwhat == "default" || xwhat == "bool")
    int xtype = EXPT_X_DEFAULT;
    if (xwhat == "" // python may send this
            || xwhat == "default" || xwhat == "bool") {
        ;
    }else if (xwhat == "values") {
        in_dim = hi_dim_values;
        xtype = EXPT_X_VALUES;
    }else{
        throw "Illegal argument xwhat for exptFloat_alloc";
    }
    int out_dim = (int)factorLayer.size();
    int const pad = exptFloat_pad;
    int nfloats = pad + in_dim + pad + out_dim + pad;

    float* mem = (float*)malloc(sizeof(float)*(nfloats));
    if(mem == nullptr)
        RTERR(string("Out of memory"));
    // todo: cacheline alignment of 'mem' internal pointers?
    ExptFloat *ef = new ExptFloat{ xtype, out_dim, in_dim, 0.0f/*max_gf*/,
        EXPT_Y_UNKNOWN,
        &mem[pad],
        &mem[pad + in_dim + pad] };
    return ef;
}

/*static*/ void AllRunsxx::exptFloat_free( ExptFloat *ef){
    if(ef){
        if(ef->expt){
            // possible warning: if xtype or ytype < EXPT_SET,
            // then the program has allocated, but perhaps not used,
            // some allocation (or double free or memory corruption)
            int const pad = exptFloat_pad;
            free((void*)(&ef->expt[ -pad ]));
            const_cast<int&>(ef->xtype) = EXPT_X_UNKNOWN;
            ef->max_gf=0.0f;
            ef->ytype = EXPT_Y_UNKNOWN;
            ef->expt=nullptr;
            ef->y=nullptr;
            const_cast<int&>(ef->c) = 0;
            const_cast<int&>(ef->w) = 0; // [opt for debug]
            delete(ef);
        }
    }
}

/** Form float vector `y[channels]` target for an Expt.
 * The y vector is used to form a loss function.  y for present
 * Measurements comes from here, whereas y values for absent
 * measurements will be zero.
 *
 * NO normalization of y is done here.  We just extract
 * the desired values.
 */
void AllRunsxx::get_y(int const i, float *max_gf, float *y,
                      string what/*="norm_gf"*/) const
{
    Expt const& e = expts.at(i);
    Expt::Measurement *m = e.m;
    auto const msz = e.msz;
    // ysz is the total # of possibilities for m.layer (for now)
    size_t ysz = factorLayer.size();
    for(size_t i=0U; i<ysz; ++i)
        y[i] = 0.0f;  // absent things map to all-zero.
    *max_gf = 0.0f;
    if(msz){
        { // max Gflops over Present data
            float mx = 1.0e-6;
            for(size_t i=0U; i<msz; ++i){
                mx = std::max(mx, m[i].gflops);
            }
            *max_gf = mx;
        }
        if(what=="gf"){
            // measured Gflops
            // for each present item (layer)...
            for(size_t i=0U; i<msz; ++i){
                int layer = m[i].layer;
                y[layer] = m[i].gflops;
            }
        }else if(what=="norm_gf"){
            // Gflops normalized so fastest is 1.0
            float const rescale = (1.0 / *max_gf);
            for(size_t i=0U; i<msz; ++i){
                int layer = m[i].layer;
                y[layer] = m[i].gflops * rescale;
            }
        }else{
            RTERR(string("Error: get_y unknown 'what'"));
        }
    }
}

//
// TODO   is x once per Expts[i]?  then it is just a simple vector!
//        and NOT A MATRIX.  FIXME
//
//void AllRunsxx::get_x(int const i, float *x, string what/*="default"*/) const
void AllRunsxx::get_x(int const i, float *x, int xtype) const
{
    Expt const& e = expts.at(i);
    //std::cout<<"get_x("<<i<<","<<(void*)x<<")"
    //        " into dest x@"<<(void*)x<<std::endl;
    //int const chan = (int)factorLayer.size();
    //int const wide = hi_dim;
    // x[chan,dims]
    //for(int i=0; i<chan; ++i){
    //    floats(/*ConvParamRaw*/e.c, &x[i*wide]);
    //}
    // NEW: don't create 'chan' identical rows in a matrix, just one vector
    // TODO: also need threads, layout, ... from Expt
    if (xtype == EXPT_X_BOOL) { // equiv EXPT_X_DEFAULT
        floats(/*ConvParamRaw*/e.c, e.threads, e.ops, &x[0]);
    }else if (xtype == EXPT_X_VALUES) {
        floats_values(e.c, e.threads, e.ops, &x[0]);
    }// else assert/throw ?
}

/**
 * ExptFloat::expt has \e w width convolution features.  Features begin with
 * basic convolution parameters and their inverses, expanded by other
 * information-rich features obtained from looking at optimal decision
 * trees.
 *
 * Use `AllRunsxx::layer(i)` for the ExptFloat::c channel (impl) names
 * of ExptFloat::y
 */
/*static*/ char const* const* AllRunsxx::featureNames( std::string xwhat )
{
    char const* const* ret = nullptr;
    if (xwhat == "default" || xwhat == "bool")
        ret = floats_names;
    else if (xwhat == "values")
        ret = subFeature_values_names;
    // else assert/throw
    return ret;
}
/*static*/ size_t AllRunsxx::feature_n( std::string xwhat ){
    size_t ret = 0;
    if (xwhat == "default" || xwhat == "bool")
        ret = hi_dim;
    else if (xwhat == "values")
        ret = hi_dim_values;
    return ret;
}

void AllRunsxx::exptFloat(size_t const i, ExptFloat *ef,
        string ywhat/*="norm_gf"*/) const
{
    assert(ef!=nullptr);
    //assert(ef->xtype < EXPT_X_SET); // no, CAN re-populate an existing ExptFloat
    if (ef->xtype >= EXPT_SET)
        ef->xtype -= EXPT_SET;
    assert(ef->c = (int)factorLayer.size());
    assert(ef->w = (int)hi_dim);
    // ef->ytype -- if ywhat is empty, and ef->ytype != EXPT_Y_UNKNOWN, use ytype?
    if (ywhat.empty()) {
        if (ef->ytype >=EXPT_SET)
            ef->ytype -= EXPT_SET;
        if (ef->ytype == EXPT_Y_NORM_GF){
            ywhat = string{"norm_gf"};
        }else if (ef->ytype == EXPT_Y_GF) { // Gflops
            ywhat = string{"gf"};
        }
    }else{
        if (ywhat == "norm_gf")
            ef->ytype = EXPT_Y_NORM_GF;
        else if (ywhat == "gf")
            ef->ytype = EXPT_Y_GF;
    }
    assert(ef->ytype < EXPT_SET);
    assert(ef->expt != nullptr);
    assert(ef->y != nullptr);
    // ... or ... resize if not right-sized?

    get_x(i, ef->expt, ef->xtype); // set of features might have some options too
    ef->xtype += EXPT_SET;
    get_y(i, &ef->max_gf, ef->y, ywhat);
    ef->ytype += EXPT_SET;
}
ExptFloat* AllRunsxx::exptFloat(size_t const i,
        string xwhat/*="default"*/,
        string ywhat/*=norm_gf*/) const
{
    //std::cout<<" exptFloat("<<i<<","<<xwhat<<","<<ywhat<<")"<<endl;
    ExptFloat *ef = exptFloat_alloc(xwhat); // chan and dim and fresh malloc all set up
    get_x(i, ef->expt, ef->xtype);
    ef->xtype += EXPT_SET;
    get_y(i, &ef->max_gf, ef->y, ywhat);
    if (ef->ytype < EXPT_SET) ef->ytype += EXPT_SET;
    return ef;        // MUST be freed with exptFloat_free(ExptFloat *)
}



namespace rmv {
    struct StringFind{
        string needle;
        StringFind(string needle): needle(needle) {}
        bool operator()(string const& haystack){
            return haystack.find(needle) != std::string::npos;
        }
    };
    struct CharIs{
        char match;
        CharIs(char const match): match(match) {}
        bool operator()(char const c){
            return c == match;
        }
    };
}

void AllRunsxx::filter(bool const verbose/*=false*/)
{
    if(!filtered){
        // 1. Apply filters to remove extras
        //    Our private "set" string storage remains full-sized,
        //    so char const* members of vrr[] remain OK.
        auto cond = [](OneRunRaw const& rr){
            //static rmv::StringFind stdImpl0("std");
            static rmv::CharIs stdImpl('*');
            static rmv::CharIs refImpl('R');
            //bool stdimpl0 = stdImpl0(rr.layer);
            bool stdimpl = stdImpl(rr.layertype); // faster and equivalent
            bool refimpl = refImpl(rr.layertype);
            bool ret = stdimpl || refimpl
                // ?allow jit
                // ?check dirn (PropKind)
                // ?check layout
                ;
            //if(ret){
            //    cout<<"Removing! stdimpl0="<<stdimpl0<<" stdimpl="<<stdimpl<<" refimpl="<<refimpl<<endl;
            //}
            return ret;
        };
        vrr.erase( std::remove_if(vrr.begin(), vrr.end(), cond),
                   vrr.end());

        // Factors are determined from "whatever remains in vrr".
        //    (Or by repeating the filtering on our "set" storage items?)
        // These string points point into our "set" objects (cheap)
        if (1) { // temp sets
            std::set<string> filtParams;
            //std::set<std::string> filtNames;           // ignored, often set at "wip"
            std::set<char>   filtLayertypes;
            std::set<string> filtLayers;
            for(auto const& r: vrr){
                filtParams.insert(r.params).second;
                filtLayertypes.insert(r.layertype);
                filtLayers.insert(r.layer);
            }

            factorParam.clear();
            for(auto s: filtParams){ // char const* MUST come from our "set" objects
                auto found = setParams.find(s);
                if(found==setParams.end())
                    RTERR("Program error: filtered values must be a subset");
                factorParam.push_back(found->c_str());
            }

            factorLayertype.clear();
            factorLayertype = vector<char>{filtLayertypes.begin(), filtLayertypes.end()};

            factorLayer.clear();
            for(auto s: filtLayers){ // char const* MUST come from "setXXX"
                auto found = setLayers.find(s);
                if(found==setLayers.end())
                    RTERR("Program error: filtered values must be a subset");
                factorLayer.push_back(found->c_str());
            }
        }
#if 0
        // shorten layer info in setLayers and factorLayer ?and vrr
        for(auto & x: setLayers){
            int const bufsz=100; char buf[bufsz];
            layer_short(x.c_str(), buf, bufsz);
            x = string{&buf[0]};
        }
        if(1) {
            cout<<" shortened setLayers? ";
            for(auto & x: setLayers){ cout<<x; }
            cout<<endl;
        }
#endif

        if(verbose){
            cout<<"factorParam["<<factorParam.size()<<"]"<<endl;
            // may be very many of these
            cout<<"factorLayertype["<<factorLayertype.size()<<"] alphabetical"<<endl;
            // expect about 3 of these
            for(auto const l: factorLayertype){
                cout<<" layertype: "<<l<<endl;
            }
            cout<<"factorLayer["<<factorLayer.size()<<"] alphabetical"<<endl;
            // probably 50-60 remain of these
            for(auto const l: factorLayer){
                cout<<"     layer: "<<l<<endl;
            }
        }
#if 0
        // now this is done during construction.  So filtering should
        // rearrange (reorder) the factor levels.
        
        // 2. Set up factors by examining remaining entries
        std::set<char const*> params;
        std::set<char> layertypes;
        std::set<char const*> layers;
        for(auto const rr: vrr){ // for each OneRunRaw .csv line
            params.insert(rr.params);
            layertypes.insert(rr.layertype);
            layers.insert(rr.layer);
        }
        auto factorParam = vector<char const*>{params.begin(), params.end()};
        auto factorLayertype = vector<char>{layertypes.begin(), layertypes.end()};
        auto factorLayer = vector<char const*>{layers.begin(), layers.end()};
        if(verbose){
            cout<<"factorParam["<<factorParam.size()<<"]"<<endl;
            cout<<"factorLayertype["<<factorLayertype.size()<<"]"<<endl;
            cout<<"factorLayer["<<factorLayertype.size()<<"]"<<endl;
        }
#endif
        filtered=true;
    }
}

// after filter, these "to integer" factor conversions make sense
int AllRunsxx::intParam(char const* s) const
{
#ifndef NDEBUG
    bool sorted = std::is_sorted(factorParam.begin(), factorParam.end(),
                                 [](char const* a, char const* b)
                                 {return strcmp(a,b) < 0;});
    assert(sorted);
#endif
    // first not-less-than
    auto it = std::lower_bound( factorParam.begin(), factorParam.end(), s,
                                [](char const* const& a, char const* const& b){
                                return strcmp(a,b) < 0;
                                });
    if(it == factorParam.end())
        RTERR("illegal intParam factor conversion");
    // Note: call as "distance(first,last)"
    return (int)std::distance(factorParam.begin(), it);
}
int AllRunsxx::intLayertype(char s) const
{
#ifndef NDEBUG
    bool sorted = std::is_sorted(factorLayertype.begin(), factorLayertype.end());
    assert(sorted);
#endif
    // first not-less-than
    auto it = std::lower_bound( factorLayertype.begin(), factorLayertype.end(), s);
    if(it == factorLayertype.end()){
        cout<<"Trying to convert <"<<s<<">"<<" to integer"<<endl;
        cout<<"Known char values: ";
        for(char c: factorLayertype) cout<<c;
        cout<<endl;
        RTERR("illegal intLayertype factor conversion");
    }
    return (int)std::distance(factorLayertype.begin(), it);
}
int AllRunsxx::intLayer(char const* s) const
{
#ifndef NDEBUG
    bool sorted = std::is_sorted(factorLayer.begin(), factorLayer.end(),
                                 [](char const* a, char const* b)
                                 {return strcmp(a,b) < 0;});
    assert(sorted);
#endif
    // first not-less-than
    auto it = std::lower_bound( factorLayer.begin(), factorLayer.end(), s,
                                [](char const* a, char const* b)
                                {return strcmp(a,b) < 0;});
    if(it == factorLayer.end())
        RTERR("illegal intLayer factor conversion");
    return (int)std::distance(factorLayer.begin(), it);
}

void AllRunsxx::massage_ms_gflops()
{
    // extract the vectors we'll be examining
    size_t const sz = vrr.size();
    vector<float> vms(sz);
    vector<float> vgf(sz);
    vector<float> vop(sz);
    for(size_t i=0; i<sz; ++i){
        vms[i] = vrr[i].ms;
        vgf[i] = vrr[i].gflops;
        vop[i] = vrr[i].ops;
    }

    // fixed point precision is determined ONLY first time through
    if(fprec_ms == 0.0f){
        float x = min_nonzero_abs(vms);
        fprec_ms = fmax(fixed_point_precision(x), 1e-5f);
    }
    if(fprec_gf == 0.0f){
        float x = min_nonzero_abs(vgf);
        fprec_gf = fmax(fixed_point_precision(x), 1e-5f);
    }

    // set zeros to half estimated printout precision, or from
    // "the other nonzero".
    if(ms_gf_nonzero == false){
        // This need be done just once (merge_dups preserved non-zero-ness)
        for(size_t i=0; i<sz; ++i){
            if(vms[i] <= 0.0f){
                if(vgf[i] > 0.0f){
                    vms[i] = 1.e-6 * vop[i] / vgf[i];
                }
            }
            if(vgf[i] <= 0.0f){
                if(vms[i] > 0.0f){
                    vgf[i] = 1.e-6 * vop[i] / vms[i];
                }
            }
            if(vms[i] <= 0.0f){
                vms[i] = 0.5*fprec_ms;
            }
            if(vgf[i] <= 0.0f){
                vgf[i] = 0.5* fprec_gf;
            }
            assert( vgf[i] > 0.0f );
            assert( vms[i] > 0.0f );
        }
        ms_gf_nonzero = true;
    }

    // In principle we *could* break ties in "one" by consulting "other".
    // But we have not yet established grouping

    // A quick estimate of sig digit accuracy is how many multiples
    // of global fprec.  This can be done per record.
    // The risk is that we create groupwise things that *might* be
    // tied in both values (but if fprec values are correct, that is
    // not likely).
    // If "roughly same" precision, make the "larger" the ref pt.
    //    (this **probably** breaks any "ties" too, I hope)
    for(size_t i=0; i<sz; ++i){
        int iprec_ms = int(floor(vms[i]/fprec_ms));
        int iprec_gf = int(floor(vgf[i]/fprec_gf));
        // order of 'if's important...
        if( iprec_ms > iprec_gf ){
            vgf[i] = 1.e-6 * vop[i] / vms[i];
        }else if(iprec_gf > iprec_ms){
            vms[i] = 1.e-6 * vop[i] / vgf[i];
        // fallback concordance (probably "breaks ties" correctly)
        }else if(vms[i] > vgf[i]){
            vgf[i] = 1.e-6 * vop[i] / vms[i];
        }else{
            vms[i] = 1.e-6 * vop[i] / vgf[i];
        }
    }

    // write back potential modifications of vms, vgf
    //   (vop is const)
    for(size_t i=0; i<sz; ++i){
        vrr[i].ms     = vms[i];
        vrr[i].gflops = vgf[i];
    }
}

void AllRunsxx::massage_merge_dups(bool const verbose/*=false*/)
{
    filter();
    if(!massaged){
        //
        // 1.
        // Guess ms and Gflops precision from file content, knowing that it
        // was printed to some few significant digits
        //     Note: this was done **after** merge within load.R merge_dups!
        //float ms_prec;
        //float gf_prec;
        massage_ms_gflops();
        //
        // 2. sort.
        sort( vrr.begin(), vrr.end(), []
                   (OneRunRaw const &a, OneRunRaw const& b) {
                   // params:up
                   int cmp = strcmp(a.params, b.params);
                   if(cmp) return cmp<0;
                   // params equal ... threads:up
                   if(a.threads != b.threads) return (a.threads < b.threads);
                   // threads equal ... layer:up
                   cmp = strcmp(a.layer,b.layer);
                   return cmp<0; // == means we'll merge them
                   });
        //
        // 2b. [NEW] replace op counts (from file) with exact (calc) count
        {
            // simple loop (could be faster)
            size_t iprev=0U, i=1U;
            auto set_vrr_ops = [](OneRunRaw& rr){
                rr.ops = conv::ConvParam{rr.params}.ops();
            };
            set_vrr_ops(vrr[0]);
            for(; i<vrr.size(); ++i){
                OneRunRaw const &a = vrr[iprev];
                OneRunRaw       &b = vrr[i];
                bool ops_same = (strcmp(a.params,b.params)==0);
                if(ops_same) {
                    b.ops = a.ops;
                }else{
                    set_vrr_ops(b);
                    iprev = i;
                }
            }
        }

        if(verbose){
            cout<<"After massage_merge_dups sort:"<<endl;
            dump(/*raw*/true);
        }
        //
        // 3. merge_dups
        size_t iprev=0U, i=1U;
        int accum_beg = iprev, accum_end = iprev+1;
        int nmerges = 0;
        for(; i<vrr.size(); ++i){
            OneRunRaw const &a = vrr[accum_beg];
            OneRunRaw const &b = vrr[i];
            bool domerge
                =  strcmp(a.params,b.params)==0
                && a.threads == b.threads
                && strcmp(a.layer, b.layer)==0;
            if(domerge) { // accumulate b into accum
                if(verbose) cout<<"domerge! accum_beg="<<accum_beg<<" i="<<i<<endl;
                continue;
            }
            // b,i are "different" from a,accum_prev.
            // Complete prev merge (if any)
            accum_end = i;
            if(verbose && accum_end <= accum_beg+1){
                cout<<"  (no merge)"<<endl;
            }
            if(accum_end > accum_beg+1){
                if(verbose) cout<<"  (merging accum_beg="<<accum_beg<<" accum_end="<<i<<")"<<endl;
                // merge accum beg--end into a
                ++nmerges;
                int sum_reps = 0;
                float sum_ms = 0.0f;
                float sum_gflops = 0.0f;
                float max_err = 0.0f;
                for(int j=accum_beg; j<accum_end; ++j){
                    // best:    potentially screwed up by merge
                    sum_reps   += vrr[j].reps;
                    sum_ms     += vrr[j].ms;
                    sum_gflops += vrr[j].gflops;
                    max_err    = std::max(max_err, vrr[j].err);
                    // threads -- should be equal for each
                    // ops -- should be equal for each
                }
                float acc_inv = 1.0 / (accum_end - accum_beg);
                // a is const, so let's use vrr[accum_beg] here
                vrr[accum_beg].reps   = sum_reps;
                vrr[accum_beg].ms     = sum_ms * acc_inv;
                vrr[accum_beg].gflops = sum_gflops * acc_inv;
                vrr[accum_beg].err    = max_err;
                // mark beg+1--end for later removal
                for(int j=accum_beg+1; j<accum_end; ++j){
                    vrr[j].params = nullptr;
                }
            }
            // current i is ref point for next mergables
            accum_beg = i;
        }
        // Remove "marked-for-removal" elements of vrr
        // completing "merge_dups" function of load.R
        if(nmerges){
            auto new_vrr_end = std::remove_if
                (vrr.begin(), vrr.end(), [](OneRunRaw const& a)
                 { return a.params == nullptr; });
            vrr.erase(new_vrr_end, vrr.end());
        }
    }
}
void AllRunsxx::massage()
{
    filter();
    if(!massaged){
        // 1. Adjust each ms, gflops, ops to concord and provide max resolution
        //    (quantization hits you esp. around printed zeros)
        // 2. sort param ascending, thread ascending, layer ascending
        // 3. merge_dups (increase 'reps' field, and average)
        //    (readjust ms/gflops)
        massage_merge_dups();
        // 4. sort param ascending, thread ascending, ms ascending
        // 5. record param+threads group start indices
        //
        
        // Again, check that post-merge ms and gflops are precise and properly related.
        massage_ms_gflops();
        //
        // 4. sort param ascending, thread ascending, ms ascending
        sort( vrr.begin(), vrr.end(), []
                   (OneRunRaw const &a, OneRunRaw const& b) {
                   // params:up
                   int cmp = strcmp(a.params, b.params); 
                   if(cmp) return cmp<0;
                   // params equal ... threads:up
                   if(a.threads != b.threads) return (a.threads < b.threads);
                   // threads equal ... ms:up
                   return a.ms < b.ms;
                   });
        //
        // 5. record param+threads[+dirn?] group_lo start indices
        //    (also correct the "best" entries, which we don't really use)
        auto grp_same = [](OneRunRaw const& a, OneRunRaw const&b){
            return (a.params==b.params) // pointers of same-string have same value!
                && (a.threads==b.threads)
                // dirn/PropKind ?
                ;
        };
        group_lo.clear();
        for(size_t i=0; i<vrr.size(); ++i){
            bool fresh_grp = (i==0 || !grp_same(vrr[i], vrr[i-1]));
            vrr[i].best = fresh_grp;
            if(fresh_grp){
                group_lo.push_back((int)i);
            }
        }
        group_lo.push_back(vrr.size()); // and include 1 more "end" index

        //
        //
        massaged=true;
    }
}

//static bool cmpMeasurementTime( Expt::Measurement const& a,
//                                Expt::Measurement const& b ){
//    return a.ms < b.ms;
//}
void AllRunsxx::group(bool const verbose/*=false*/)
{
    massage(); // sets up ... and group_lo experiment grouping
    if(!grouped){
        if(verbose){
            cout<<"group() begins with factors..."<<endl;
            cout<<"factorParam["<<factorParam.size()<<"]"<<endl;
            // may be very many of these
            cout<<"factorLayertype["<<factorLayertype.size()<<"] alphabetical"<<endl;
            // expect about 3 of these
            for(auto const l: factorLayertype){
                cout<<" layertype: "<<l<<endl;
            }
            cout<<"factorLayer["<<factorLayer.size()<<"] alphabetical"<<endl;
            // probably 50-60 remain of these
            for(auto const l: factorLayer){
                cout<<"     layer: "<<l<<endl;
            }
            cout<<endl;
        }
        // Create and store the Expt (1 per group)
        expts.clear();
        // last group_lo as actually 'end' index -- don't include in loop
        size_t const ngrp = group_lo.size()-1U;
        expts.resize(ngrp); // uninitialized!
        for(size_t g=0U; g<ngrp; ++g){
            auto lo = group_lo[g], hi = group_lo[g+1];
            if(verbose) cout<<" group vrr["<<lo<<","<<hi<<")..."<<endl;
            auto const& vrrlo = vrr[lo];
            Expt & e = expts[g]; // now init all fields

            conv::ConvParam cp{vrrlo.params};
            if(cp.check() == CONVCHECK_ERROR)
                RTERR("Convolution parameter string did not make sense");
            e.c = cp.raw(); // converter to ConvParamRaw

            e.threads   = vrrlo.threads; // all are equal
            e.ops       = vrrlo.ops;
            e.msz = (hi - lo);
            e.m = (Expt::Measurement*)malloc(e.msz * sizeof(Expt::Measurement));
            if(!e.m) RTERR("Out of memory");
            for(auto j=0U; j<e.msz; ++j){
                // populate e.m[r] records from raw data in rr
                OneRunRaw const& rr = vrr[lo+j];
                Expt::Measurement & emj = e.m[j];
                //if(verbose)cout<<"rr.layertype="<<rr.layertype<<" rr.layer="<<rr.layer<<endl;
                emj.layertype = intLayertype(rr.layertype); //char-->int
                emj.layer = intLayer(rr.layer); //char const*-->int
                //if(verbose)cout<<"--> emj.layertype="<<emj.layertype<<" emj.layer="<<emj.layer<<endl;
                emj.reps = rr.reps; //int
                emj.ms = rr.ms;
                emj.gflops = rr.gflops;
                emj.err = rr.err;
            }
            // vrr entries were sorted ms:up, so too will e.m
        }
        grouped=true;
    }
}

/** `group()` does mallocs for Expt::Measurement that
 * destructor should free.   The mallocs are for presenting a
 * 'C'-style interface (size `msz` and raw pointer `m`). */
AllRunsxx::~AllRunsxx()
{
    for(auto& e: expts){
        Expt::Measurement* pm = e.m;
        e.msz = 0;
        e.m = nullptr;
        if(pm) free(pm);
    }
}


#if 0
void print_test_data_single( struct TestData const* test_data, int const t,
        int const a, int const z, double const HZ,
        char const* header/*=nullptr*/ ){
    // for spreadsheet, better to print header in every line
    //if(header && header[0]!='\0'){
    //    printf("%s\n", header);
    //}
    vector<TestTime> ttime;
    ttime.reserve(256);
    for(size_t ntd=a; ntd<z; ++ntd){
        struct TestData const* td = &test_data[ntd];
        if( td->test != t )
            continue;
        double const f = 1.0e3 / HZ;
        double const time = td->sum_times * f / td->reps; // average ms
        ttime.push_back({(int)ntd,time});
    }
    // sort ttime[0..nttime)
    qsort( &ttime[0], ttime.size(), sizeof(struct TestTime), cmp_TestTime );
    char const* fastest="**";
    for(size_t ntt=0; ntt<ttime.size(); ++ntt){
        struct TestData const* td = &test_data[ttime[ntt].ntd];
        double const f = 1.0e3 / HZ;
        double const time = td->sum_times * f / td->reps; // average ms
        // Gop/s = Mop/ms
        double const gops = (td->ops>0? td->ops*1.0e-6 / time: 0.0);
        printf( "%c %25s %s %4ux %9.3f t%d ~%.4f %6.2fG %s",
                testData_impl_char(td->impl_type),
                td->impl_name,
                fastest, td->reps, time, (int)__vednn_omp_num_threads, (double)td->diff,
                gops, td->test_name);
        if(header    && header[0]   !='\0') printf(" %s", header);
        // NEW: descr no holds a test-wide comment,
        //      like param_cstr_short or test duplicate info.
        //if(td->descr && td->descr[0]!='\0') printf(" %s", td->descr);
        printf("\n");
        fastest = " |";
    }
}
void print_test_data( struct TestData const* test_data, int const a, int const z,
        double const HZ, int const v/*=1*/){
    assert( a>=0 );
    assert( z>=a );
    if( a >= z ) return;
    int minTest = 1<<30;
    int maxTest = 0;   //
    for(int i=a; i<z; ++i){
        if(test_data[i].test > maxTest) maxTest = test_data[i].test;
        if(test_data[i].test < minTest) minTest = test_data[i].test;
    }
    if(v){
        printf("*** print_test_data[%d..%d) covers tests %d..%d",a,z,minTest,maxTest);
        if(minTest==maxTest && test_data[a].descr && test_data[a].descr[0] != '\0')
            printf(" %s",test_data[a].descr);
        printf("\n");
    }
    for(int t=minTest; t<=maxTest; ++t){
        print_test_data_single(test_data, t, a, z, HZ);
    }
}
/** print all (nEntry) tests (impls) in \c test_data with nice test label line */
void print_test_data( struct TestDataRepo const& tdRepo,
        struct param const *pNetwork, int nEntry ){
    char buf[100];
    //char header[150];
    for(int t=0; t<nEntry; ++t){
        struct param const *pNw = &pNetwork[t];
        param_cstr_short(pNw,buf,100);
        //snprintf(header,150,"Layer %-30s %s_n%s", pNw->pName, buf, pNw->pName);
        //print_test_data_single(&tdRepo[0],t, 0,tdRepo.size(), tdRepo.hertz,header);
        print_test_data_single(&tdRepo[0],t, 0,tdRepo.size(), tdRepo.hertz,buf);
    }
}
#endif

void AllRunsxx::dump(OneRunRaw const& r) // static member fn
{
    printf( " %c %25s | %4ux t%-2d %9.4f ms %.3e G ~%g %s %s\n",
            r.layertype,
            r.layer,
            r.reps,
            r.threads,
            r.ms,
            r.gflops,
            r.err,
            r.name,
            r.params
          );
    cout.flush();
}
void AllRunsxx::dump(Expt const& e) const // nonstatic member fn
{
    int const bufsz=100;
    char buf[bufsz];
    to_cstr_short( &e.c, buf, bufsz);
    cout<<" "<<&buf[0]<<" t"<<e.threads
            <<" ops="<<e.ops<<" msz="<<e.msz<<endl;
    for(size_t i=0U; i<e.msz; ++i){
        Expt::Measurement const& m = e.m[i];
        // debug:
        //cout<<"m.layertype="<<m.layertype<<" of "<<factorLayertype.size()
        //        <<" m.layer="<<m.layer<<" of "<<factorLayer.size()<<endl;
        printf( " %c %25s | %3ux %9.4f ms %6.4fG ~%g\n",
                factorLayertype.at(m.layertype), // a char
                factorLayer.at(m.layer),
                m.reps,
                m.ms,
                m.gflops,
                m.err
              );
    }
    cout.flush();
}

// Print expts[i] as table.  If expts empty (just constructed), dump vrr[i] instead.
void AllRunsxx::dump(size_t i, bool const raw/*=false*/) const
{
    if(!grouped || raw==true){
        //cout<<"dump vrr["<<i<<"] HERE"<<endl;
        auto const& r = vrr.at(i);
        cout<<"vrr["<<i<<"] ";
        dump(r);
    }else{
        //cout<<"dump expts["<<i<<"] of "<<expts.size()<<" HERE"<<endl;
        Expt const& e = expts.at(i);
        cout<<"expts["<<i<<"] ";
        // The c_str_short USUALLY will be identical with the raw string
        //cout<<" cf factorParams "<<factorParam.at(i);
        dump(e);
    }
}
// Print expts[beg..end-1] as table. (or vrr[beg..end-1] if expts-empty or raw)
void AllRunsxx::dump(size_t beg, size_t end, bool const raw/*=false*/) const
{
    if(!grouped || raw==true){
        //cout<<"dump vrr["<<beg<<","<<end<<") of "<<vrr.size()<<" HERE"<<endl;
        for(size_t i=beg; i<end; ++i){
            cout<<"vrr["<<i<<"] ";
            dump(vrr.at(i));
        }
    }else{
        //cout<<"dump expts["<<beg<<","<<end<<") of "<<expts.size()<<" HERE"<<endl;
        for(size_t i=beg; i<end; ++i){
            cout<<"expts["<<i<<"] ";
            // The c_str_short USUALLY will be identical with the raw string
            //cout<<" cf factorParams "<<factorParam.at(i);
            dump(expts.at(i));
            cout<<"\n";
        }
    }
}
void AllRunsxx::dump(bool const raw/*=false*/) const
{
    if(!grouped || raw){
        //cout<<"dump vrr[0,"<<vrr.size()<<")"<<endl;
        dump(0U, vrr.size(), /*raw*/true);
    }else{
        //cout<<"dump expts[0,"<<expts.size()<<")"<<endl;
        dump(0U, expts.size(), /*raw*/false);
    }
}

}// namespace expt::

//
// ====================== C API ======================
//

#ifdef __cplusplus
extern "C"
{
 
using namespace expt;

#else
#error "This file must be compiled via C++"
#endif

int readConvCsv(char const* fname, AllRuns *ar){
    int ret = -1; // error!
    if( ar != nullptr ){
        AllRunsxx* pobj = new AllRunsxx(fname);
        if( pobj->getStatus() ) { // some error
            delete pobj;
            ar->opaque = nullptr;
        }else{
            ar->opaque = (void*)pobj;
            ret = pobj->getStatus();
        }
    }
    return ret;
}

void allruns_free(AllRuns **ar){
    if (ar != nullptr && *ar != nullptr){
        AllRunsxx * pobj = static_cast<AllRunsxx*>((*ar)->opaque);
        if (pobj != nullptr){
            delete pobj;
        }
        (*ar)->opaque = nullptr;
        *ar = nullptr;
    }
}

size_t allruns_raw_sz(AllRuns const* ar){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getVrr().size();
}

OneRunRaw const* allruns_raw(AllRuns const* ar, int ith){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return &obj.getVrr().at(ith);
}


size_t allruns_expt_sz(AllRuns const* ar){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.expt_sz();
}

Expt const* allruns_expt(AllRuns const* ar, int ith){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.expt(ith);
}

/** run ar..group().  Then C api uses allruns_expt and allruns_expt_sz
 * to access individual data. */
void mkExpts( AllRuns *ar
        //, TBD filters: enum PropKind dirn, enum Layout layout
        )
{
    AllRunsxx & pobj = *cvt_or_throw(ar);
    pobj.group(); // and any other preprocessing
}

/** - If \c *dest is already set up, \c xwhat is ignored.
 *  - If *dest is a nullptr, *dest is allocated using \c xwhat.
 */
void toExptFloat(AllRuns *ar, size_t const i, ExptFloat **dest, char const* xwhat){
    AllRunsxx & obj = *cvt_or_throw(ar);
    obj.group(); // and any other preprocessing
    assert(dest != nullptr);
    if(*dest == nullptr){
        if (xwhat == nullptr || xwhat[0] == '\0') {
            xwhat = "default";
        }
        *dest = obj.exptFloat_alloc(std::string{xwhat});
    }
    assert(*dest != nullptr);
    // We hard-wired "normalized Gflops" as supplied y-vector
    string what{"norm_gf"};
    // `*dest` compatibility is checked in `exptFloat`
    obj.exptFloat(i, *dest, what);
}

void exptFloat_free(ExptFloat **dest){
    if(dest!=nullptr){
        if(*dest!=nullptr){
            AllRunsxx::exptFloat_free(*dest);
        }
        *dest = nullptr;
    }
}

char const* strParam    (AllRuns const* ar, int i){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getFactorParam().at(i);
}

char        strLayertype(AllRuns const* ar, int i){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getFactorLayertype().at(i);
}
char const* strLayer    (AllRuns const* ar, int i){
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getFactorLayer().at(i);
}
int intParam    (AllRuns const* ar, char const* param)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.intParam(param);
}
int intLayertype(AllRuns const* ar, char const c)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.intLayertype(c);
}
int intLayer    (AllRuns const* ar, char const* layer)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.intLayer(layer);
}
size_t intParam_sz    (AllRuns const* ar)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getFactorParam().size();
}
size_t intLayertype_sz(AllRuns const* ar)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getFactorLayertype().size();
}
size_t intLayer_sz    (AllRuns const* ar)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    return obj.getFactorLayer().size();
}

void allruns_print(AllRuns const*ar, Expt const* e)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    assert(e != nullptr);
    obj.dump(*e);
}

void printExpts(AllRuns const* ar, int beg, int end)
{
    AllRunsxx const& obj = *cvt_or_throw(ar);
    int mod = 0;
    if(beg < 0) {beg = 0; ++mod;}
    if(end > obj.expt_sz()) {end = obj.expt_sz(); ++mod;}
    if(mod) printf(" range-->[%d,%d) ", beg, end);
    if(beg >= end){
        printf("expts[%d,%d) is empty\n", beg, end);
    }else{
        obj.dump(beg, end, /*raw*/false);
    }
}

#ifdef __cplusplus
}//extern "C"
#endif
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
