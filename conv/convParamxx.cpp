// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#include "convParamxx.h" // this is really a C++ header
#include <string>
#include <cassert>
#include <stdexcept>
#include <regex>
#include <map>

#define mb batchNum
#define g group
#define ic inChannel
#define ih inHeight
#define iw inWidth
#define oc outChannel
#define oh outHeight
#define ow outWidth
#define kh kernHeight
#define kw kernWidth
#define sh strideHeight
#define sw strideWidth
#define ph padHeight
#define pw padWidth
#define dh dilationHeight
#define dw dilationWidth

using namespace std;

namespace conv { //}{

// replace 1st exact match of 'search'
static inline
std::string& findReplace(std::string &data, std::string search, std::string replaceStr){
    if(data.find(search) != std::string::npos) {
        data.replace(data.find(search), search.size(), replaceStr);
    }
    return data;
}

// replace all exact matches of 'search'
static inline
std::string& findReplaceAll(std::string &data, std::string search, std::string replaceStr){
    while(data.find(search) != std::string::npos) {
        data.replace(data.find(search), search.size(), replaceStr);
    }
    return data;
}

std::string shorten_layername(std::string const& in){
#if 0
    // R code (oldish - R code had typos, fixed in R and here)
    x = ifelse(str_detect(x,"jit"),
               str_extract(x,".*jit[^_]*"), # drop any [rare] super-long suffix
               x)
    x = sub("(.*)mb[-_](.*)",       "\\1\\2B",x)       # jit prefix to suffix char
    x = sub("(.*)unroll[-_](.*)",   "\\1\\2U",x)   # jit prefix to suffix char
    x = str_replace(x,"cjitConvFwd","FJ")
    x = sub("(.*)vecC[-._]?(.*)",   "\\1VC\\2",x)   # VC suffix
    x = sub("VCxw",                 "1VC_X",x)     # 'R' TYPO !!!restore _xw as _X no-thread suffix
    x = sub("(.*)_xw$",             "\\1_X",x)      # change other _xw suffix to _X
    x = sub("cnvFwd[-._]?",         "F",x)          # If mixed, could use "F" for "Forward"
    # for libvednn-std things: (just in case)
    x = sub("libvednn-std:",  "std.",x)
    x = sub("dil(\\d+)_?",    "d\\1", x)
    x = sub("str(\\d+)_?",    "s\\1", x)
    x = sub("padsame_?",      "pS",   x)
    x = sub("pad(\\d+)_?",    "p\\1", x)
    x = sub("ker(\\d+)_?",    "k\\1", x)
    x = sub("_?ow2X",         "ow2X", x)
    x = sub("_ioaligned",     "al", x)

    # replace all unrecognized chars for factor level with '.' (notably ':')
    x = make.names(x);
    # this leaves letters, numbers, dot and underline
    #   (and must start with letter or dot-not-followed-by-number)
    #   (and must not conflict with R reserved words)

    Example contractions:
    Fd1s1pSk3_c1024x_T
    cnvFwd-d1s2p1k4owU128_xw            --> Fd1s2p1k4owU128_X
    libvednn-std:dil1_str1_pad0_ker1_T  --> std.d1s1p0k1T
    mb-unroll_cjitConvFwd1q             --> FJ1qBU
#endif
    // shorten s
    std::string s = in;
    std::smatch m;
    // jit layer name shortenings, remove possible long suffix,
    // prefixes--> 1 letter suffix, forward-->begin with 'F'
    if(regex_search(s, m, regex(".*jit[^_]*")))
        s = m[0];
    s = regex_replace(s, regex{"mb[-_](.*)"}, "$1B");
    s = regex_replace(s, regex{"unroll[-_](.*)"}, "$1U");
    findReplace(s, "cjitConvFwd", "FJ");
    // vecC--> VC  and a no-threads _X suffix
    s = regex_replace(s, regex{"vecC[-._]?(.*)"}, "VC$1");
    s = regex_replace(s, regex{"VCxw$"}, "VC_X");            // VCxw suffix --> _X
    s = regex_replace(s, regex{"_xw$"}, "_X");              // _xw suffix --> _X
    // forward convolutions begin(we hope) with 'F'
    s = regex_replace(s, regex{"cnvFwd[-._]?"}, "F");       // cnvFwd (beginning?) --> F
    // for libvednn-std shortenings: (just in case)
    findReplace(s, "libvednn-std:", "std.");
    s = regex_replace(s, regex{"dil(\\d+)_?"}, "d$1");
    s = regex_replace(s, regex{"str(\\d+)_?"}, "s$1");
    s = regex_replace(s, regex{"padsame_?"}, "pS");
    s = regex_replace(s, regex{"pad(\\d+)_?"}, "p$1");
    s = regex_replace(s, regex{"ker(\\d+)_?"}, "k$1");
    s = regex_replace(s, regex{"_?ow2X"}, "ow2X");
    s = regex_replace(s, regex{"_ioaligned"}, "al");

    // New: since gemm im2col decisions are internalized, remap all
    //      distinguished 'gemm.foo' --> 'gemm'.
    //      We do NOT need to predict correctly *which* gemm variant gets used.
    s = regex_replace(s, regex{"Fgemm(:.*)"}, "Fgemm"); // gemm:foo --> gemm, but leave std.gemm

    // 'R' make.names resembles the following char "legal letters" replacement
    // e.g. ':' --> '.'
    s = regex_replace(s, regex{"[^a-zA-Z0-9._]"}, ".");
    // store shortened name
    return s;
}

/** These long names derive from raw 'jitconv' test outputs.
 * - The long namings set up in `libvednn/src/wrap/vednnConvolution_ok`.
 * - They're based on actual symbol names in libvednn, with prefixes/suffixes.
 * - We want such long strings to have shorter versions for analysis work.
 */
static vector<string> stdConvLayerNames = {
    "cjitConvFwd1q", "cjitConvFwd6", "cnvFwd-d1p0", "cnvFwd-d1p0_owU128", "cnvFwd-d1p0_owU128_xw",
    "cnvFwd-d1p0_xw", "cnvFwd-d1s1p0", "cnvFwd-d1s1p0_owU128", "cnvFwd-d1s1p0_owU128_xw", "cnvFwd-d1s1p0_xw",
    "cnvFwd-d1s1p0k3iw2XU256_ow2X_ioaligned", "cnvFwd-d1s1p0k3iw2XU256_ow2X_ioaligned_xw",
    "cnvFwd-d1s1p0k4iwU256", "cnvFwd-d1s1p0k4iwU256_xw", "cnvFwd-d1s1pS","cnvFwd-d1s1pS_xw", "cnvFwd-d1s1pSk3",
    "cnvFwd-d1s1pSk3_c1", "cnvFwd-d1s1pSk3_c1_xw", "cnvFwd-d1s1pSk3_c1024x_T",
    "cnvFwd-d1s1pSk3_c1owU128", "cnvFwd-d1s1pSk3_c1owU128_xw", "cnvFwd-d1s1pSk3_T", "cnvFwd-d1s1pSk3_xw",
    "cnvFwd-d1s1pSk5", "cnvFwd-d1s1pSk5_xw", "cnvFwd-d1s1pSk5owU128", "cnvFwd-d1s1pSk5owU128_xw",
    "cnvFwd-d1s2p1k3owU128", "cnvFwd-d1s2p1k3owU128_xw", "cnvFwd-d1s2p1k4owU128", "cnvFwd-d1s2p1k4owU128_xw",
    "cnvFwd-def", "cnvFwd-def_xw", "cnvFwd-gemm:0", "cnvFwd-gemm:0k1p0a", "cnvFwd-gemm:1", "cnvFwd-gemm:1k1p0a",
    "cnvFwd-gemm:1sw1", "cnvFwd-gemm:2", "cnvFwd-gemm:2sw1",
    "cnvFwd-owU128", "cnvFwd-owU128_T", "cnvFwd-owU128_xw", "cnvFwd-p0k1",
    "cnvFwd-p0k1_owU128", "cnvFwd-p0k1_owU128_xw", "cnvFwd-p0k1_xw",
    "cnvFwd-s1p0k1", "cnvFwd-s1p0k1_T", "cnvFwd-s1p0k1_xw",
    "cnvFwd-vecC", "cnvFwd-vecC_xw", "cnvFwd-vecC-d1p0k1", "cnvFwd-vecC-d1p0k1_xw",
    "cnvFwd-vecC-d1p0k1cU1024", "cnvFwd-vecC-d1p0k1cU1024_xw", "cnvFwd-vecC-d1s1pSk3", "cnvFwd-vecC-d1s1pSk3_xw",
    "gemm-Ref",
    "libvednn-std:dil1_str1_pad0_ker1_T", "libvednn-std:dil1_str1_padsame_ker3_c1024x_T",
    "libvednn-std:dil1_str1_padsame_ker3_T", "libvednn-std:gemm:0", "libvednn-std:gemm:0k1p0a",
    "libvednn-std:gemm:1", "libvednn-std:gemm:1k1p0a", "libvednn-std:gemm:2", "libvednn-std:mb-default",
    "libvednn-std:mb-dil1_str1_pad0", "libvednn-std:mb-dil1_str1_pad0_ker3_iw2XU256_ow2X_ioaligned",
    "libvednn-std:mb-dil1_str1_pad0_ker4_iwU256", "libvednn-std:mb-dil1_str1_pad0_owU128",
    "libvednn-std:mb-dil1_str1_padsame", "libvednn-std:mb-dil1_str1_padsame_ker3_c1",
    "libvednn-std:mb-dil1_str1_padsame_ker3_c1_owU128", "libvednn-std:mb-dil1_str1_padsame_ker5",
    "libvednn-std:mb-dil1_str1_padsame_ker5_owU128", "libvednn-std:mb-owU128", "libvednn-std:mb-vecC",
    "libvednn-std:mb-vecC_dil1_pad0_ker1", "libvednn-std:mb-vecC_dil1_pad0_ker1_cU1024",
    "libvednn-std:mb-vecC_dil1_str1_pad1_ker3", "libvednn-std:owU128_T",
    "mb-cjitConvFwd1q", "mb-cjitConvFwd6", "mb-unroll_cjitConvFwd1q", "mb-unroll_cjitConvFwd6",
    "unroll_cjitConvFwd1q", "unroll_cjitConvFwd6"
};

/** map existing layernames to rule-based shortenings.
 * @param l long name.
 * @return map of l to short name. */
std::map<std::string, std::string> mkLayerNameMap( std::vector<std::string> l )
{
    std::map<std::string, std::string> ret;
    // ugly regexp compile time here, but don't care
    for(auto const& x: l){
        string s = x;
        ret[x] = shorten_layername(s);
    }
    return ret;
}

/// Return 2 if consistent, 1 if ok-but-inconsistent, 0 if crazy.
///
/// consistent ~ output size matches dense memory layout exactly
/// inconsistent ~ output size exceeds dense dimensioning
/// crazy ~ illegal convolution spec (something out of range)
///
/// alt impl `return this->raw()->check()`.
/// <B>keep code in sync with cConvParm.c</B>
enum ConvParamCheck
ConvParam::check() const
{
#if 1
    // equiv, but let's not (to avoid copying)
    ConvParamRaw r = this->raw();
    return ::convParamCheck( &r );
#else
    // Instead do it on "this->" variables (should match cConvParam.c)

    // Missing: check dirn, and (later) 3D fields
    bool check_range = (dil_start==0 || dil_start==1)
            && dirn >= 0   // weak enum check
            && layout >= 0 // weak enum check
            && batchNum>0 && group>0
            && inChannel>0 && inHeight>0 && inWidth>0
            && outChannel>0 && outHeight>0 && outWidth>0
            && kernHeight>0 && kernWidth>0
            && strideHeight>0 && strideWidth>0
            && padHeight>=0 && padWidth>=0
            && dilationHeight>=dil_start && dilationWidth>=dil_start
            ;
    if (!check_range)
        return CONVCHECK_ERROR;

    // group must be in [1,min(ic,oc)] and divide ic and oc exactly.
    if (!(         group <= inChannel  && (inChannel % group) == 0
                && group <= outChannel && (outChannel % group) == 0
         ))
        return CONVCHECK_ERROR;

    // "consistent" implies a packed memory layout with no "holes", so
    // a.k.a. "dense" layout with strides ~ multiples of tensor dims.

    // calculate perfect, minimal outHeight, outWidth
    int poh = compute_out(ih,kh,sh,ph,dh,dil_start);
    int pow = compute_out(iw,kw,sw,pw,dw,dil_start);

    if (oh == poh && ow == pow)
        return CONVCHECK_DENSE; // libvednn requires dense memory layout

    if (oh < poh || ow < pow) // not enough space for output --> ERROR
        return CONVCHECK_ERROR;

    // at least one of oh, ow is sparse (still OK for libvednn)
    return CONVCHECK_SPARSE;
#endif
}

ConvParamRaw ConvParam::raw() const {
    ConvParamRaw ret;
    // copy vals into 'this'
    ret.dil_start = this->dil_start;
    ret.dirn      = this->dirn;
    ret.layout    = this->layout;
    ret.mb        = this->mb;
    ret.g         = this->g ;
    ret.ic        = this->ic;
    ret.ih        = this->ih;
    ret.iw        = this->iw;
    ret.oc        = this->oc;
    ret.oh        = this->oh;
    ret.ow        = this->ow;
    ret.kh        = this->kh;
    ret.kw        = this->kw;
    ret.sh        = this->sh;
    ret.sw        = this->sw;
    ret.ph        = this->ph;
    ret.pw        = this->pw;
    ret.dh        = this->dh;
    ret.dw        = this->dw;
    return ret;
}

void ConvParam::copy_to( ConvParamRaw * const r ) const {
    assert( r != nullptr );
    // copy vals into 'this'
    r->dil_start = this->dil_start;
    r->dirn      = this->dirn;
    r->layout    = this->layout;
    r->mb        = this->mb;
    r->g         = this->g ;
    r->ic        = this->ic;
    r->ih        = this->ih;
    r->iw        = this->iw;
    r->oc        = this->oc;
    r->oh        = this->oh;
    r->ow        = this->ow;
    r->kh        = this->kh;
    r->kw        = this->kw;
    r->sh        = this->sh;
    r->sw        = this->sw;
    r->ph        = this->ph;
    r->pw        = this->pw;
    r->dh        = this->dh;
    r->dw        = this->dw;
}

void ConvParam::copy_from( ConvParamRaw const * const r ){
    assert( r != nullptr );
    // copy vals into 'this'
    this->dil_start = r->dil_start;
    this->dirn      = r->dirn;
    this->layout    = r->layout;
    this->mb        = r->mb;
    this->g         = r->g;
    this->ic        = r->ic;
    this->ih        = r->ih;
    this->iw        = r->iw;
    this->oc        = r->oc;
    this->oh        = r->oh;
    this->ow        = r->ow;
    this->kh        = r->kh;
    this->kw        = r->kw;
    this->sh        = r->sh;
    this->sw        = r->sw;
    this->ph        = r->ph;
    this->pw        = r->pw;
    this->dh        = r->dh;
    this->dw        = r->dw;
    this->_ops      = ~0ULL;    // <-- Erase vestiges of opcount too!
}

std::string ConvParam::str() const {
    ConvParamRaw cpd = raw();
    static const int bufsz=120;
    char buf[bufsz];
    to_cstr( &cpd, &buf[0], bufsz ); 
    return string(&buf[0]);
}

std::string ConvParam::str_short() const {
    ConvParamRaw cpd = raw();
    static const int bufsz=120;
    char buf[bufsz];
    to_cstr_short( &cpd, &buf[0], bufsz ); 
    return string(&buf[0]);
}


/// - Interpret paramString for OneDNN/Vednn convention,
/// - fill in missing values,
/// - make values consistent (TODO).
/// @throw invalid_argument or range_error
ConvParam::ConvParam(std::string paramString)
    : ConvParam() // delegating constructor
{
    ConvParamRaw r;
    int errs;
    {
        char const* cstr = paramString.c_str();
        char *name = nullptr;

        errs = readConvParamRaw( cstr, &r, &name );

        if(name){
            if (!errs) this->name = name;
            free(name); // name is from "C" malloc
        }
    }
    if (!errs)
        copy_from(&r);
    // no return value from constructor
    // We may still be an illegal convolution, see mkDense (for libvednn)
}

ConvParam::ConvParam(std::string paramString, ConvParam const& ovr)
    : ConvParam() // delegating constructor
{
    ConvParamRaw r;
    int errs;
    {
        char const* cstr = paramString.c_str();
        char *name = nullptr;

        ConvParamRaw ovr_raw = ovr.raw();
        errs = readConvParamOvr( cstr, &ovr_raw, &r, &name );

        if(name){
            if (!errs) this->name = name;
            free(name); // name is from "C" malloc
        }
    }
    if (!errs)
        copy_from(&r);
    // no return value from constructor
    // We may still be an illegal convolution, see mkDense (for libvednn)
}

// private worker
void ConvParam::mk_dense( ConvParam const* ovr )
{
    // this->check() COULD be CONVCHECK_ERROR.  We will "fix" everything.
    ConvParamRaw r;
    this->copy_to( &r );
    if (ovr == nullptr) {
        mkConsistent( &r );
    } else {
        ConvParamRaw overrides;
        ovr->copy_to( &overrides );
        mkConsistentOverrides( &r, &overrides );
    }
    this->copy_from( &r );
}

#ifdef __cplusplus
extern "C"
{
#endif
 
using namespace conv;
void layer_short(char const* layer, char *buf, int bufsz)
{
    static auto const mp = mkLayerNameMap( stdConvLayerNames );
    string s;
    auto it = mp.find(string{layer});
    if( it != mp.end() ){
        s = it->second;
    }else{ // slower regex translation
        s = string(layer);
        s = shorten_layername(s);
    }
    strncpy(buf, s.c_str(), bufsz);
}

#ifdef __cplusplus
}//extern "C"
#endif
// force some instantiations
class ConvParam;

}//conv::
