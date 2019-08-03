/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * loop blocking strategy for Aurora.
 * Nicer JIT version of triple loop blocking.
 * 3 original loops --> 6 loops.
 *
 * We mostly try for easy cases, where vector length is 1 (scalar)
 * or has a simple relation to the inner loop lengths.
 */
#include "genBlock-kernels.hpp"
#include "../stringutil.hpp"
#include "../fuseloop.hpp"
#include "../cblock.hpp"
#include <limits>
#include <fstream>
using namespace std;
using namespace loop;
using namespace cprog;

#define INSCMT(BLK,INS,CMT) do{ \
    auto ins=(INS); \
    auto cmt=(CMT); \
    (BLK)>>OSSFMT(left<<setw(40)<<ins<<" // "<<cmt); \
} while(0)

#if 1
/** false if cb[sub] exists, else true, and create empty cb[sub] TAG node. */
static bool tag_once(Cblock &cb, std::string const& sub){
    Cblock *found = cb.find(sub);
    if(!found) cb[sub].setType("TAG");
    return found==nullptr;
}
#endif

/** emit code only once. \c sub is a simple string (not a fancy relative path). */
static void set_once(Cblock &cb, std::string const& sub, std::string const& code){
    if(!cb.find(sub)) cb[sub]>>code;
}

template<typename T> inline constexpr T divup(T const n, T const d) {return (n+d-1)/d;}
/** Split 3 loops into 3 outer + 3 inner SIMD loops by \em blocking.
 * Begin with 3 loops, unblocked:
 * ```
 * for(int i=0; i<ii, ++ii){
 *   for(int j=0; j<jj, ++jj){
 *     for(int k=0; k<kk; ++kk){
 *     // vectors a[vl],b[vl],c[vl] of indices in above loop order
 *     KERNEL(a,b,c);
 * }}}
 * ```
 * Generic (complex) blocking makes 6 \b scalar loops:
 * ```
 * for(int io=0; io<ii; io+=iB){    // outer loops, scalar indices
 * for(int jo=0; jo<jj; jo+=jB){    // blocking factors
 * for(int ko=0; ko<kk; ko+=kB){    // iB, jB, kB
 *   for(int i=io; i<io+min(io+iB,ii); ++i){    // inner loops
 *   for(int j=jo; j<jo+min(jo+jB,jj); ++j){    // packed into vectors
 *   for(int k=ko; k<ko+min(ko+kB,kk); ++k){    // a[vl], b[vl], c[vl]
 *     // vectors a[vl],b[vl],c[vl] of indices in above loop order
 *     KERNEL(a,b,c);
 * }}}}}}
 * ```
 * Simdization further conceptually modifies the 3 inner loops by
 * incrementing i,j,k by iv,jv,kv vector lengths, and in a 'virtual'
 * set of inner loops generating vector register results of nominal
 * length `iv*jv*kv`.  For our purposes we also put a complicating
 * `min` into the simdization loops.  This is shown explicitly as a
 * set of 9 loops in `gold reference` function \ref refTriples<T>
 *
 * For this test program, we MUST:
 *
 * - Have inner i,j loops that avoid the 'min'.
 *   - \c ii%iB==jj%jB==0 <em>exact multiples</em>
 *   - FOR SIMPLICITY, require this
 *
 * Other preferences relate to ease of producing the 3 index vectors
 *
 * - Prefer having at least on of iB,jB,kB == 1
 *   - vector becomes a broadcasted scalar
 *     - or some kernels can avoid the broadcast and use scalar ops directly.
 *   - double-loop vectorization has in-depth analysis via fl6.cpp etc.
 *   - The simplified loop can be positioned fairly flexibly if order
 *     is not too important (choose between unroll/block).
 *     - we'll use the loops exactly as above to ease testing
 */
struct BlockingPlan{
    int ii, jj, kk;
    int iB, jB, kB;
    int iVl, jVl, kVl;      // these increase (bounded by MVL)
    int iLpo, jLpo, kLpo;
    int iLp, jLp, kLp;
    float iCost, jCost, kCost;
    int ijkOuter() const {return iLpo*jLpo*kLpo;}
    int ijkInner() const {return iLp*jLp*kLp;}
    int kerns() const {return ijkOuter() * ijkInner();}
    // Need a better cost model.
    double ijkCost() const {return iCost+jCost+kCost;}
    // per-loop simd lengths
    int iv() const {return iVl;}
    int jv() const {return jVl/iVl;}
    int kv() const {return kVl/jVl;}
    std::string str() const;
    void prt() const {
        cout<<"// "<<str()<<endl;
    }
};

template<typename T> struct Triples { T i, j, k; };
/** Apply a simple simdization strategy to a vector of index triples.
 * \return "gold standard" reference vectors for simdized triple-loop-fusion.
 */
template<typename T>
std::vector<std::vector<Triples<T>>> refTriples(
        T const ii,T const jj,T const kk,       // original loop limits
        T const iB, T const jB, T const kB,     // blocking factors
        T const iv, T const jv, T const kv);    // vectorization lengths
template<typename T>
std::vector<std::vector<Triples<T>>> refTriples( BlockingPlan const& p );

/** inject timing utilities, globals, init and teardown code [just once] */
void timeit_prep(Cblock& utils, Cblock& main);

/** inject a `timeit_foo` function for your \c foo.
 * \return inner Cblock in `timeit_`foo where your kernel goes.
 * - timeit_foo goes to a subblock fns[foo]
 * - invoke/print ------> subblock call[foo]
 */
Cblock& timeit_foo(char const* const foo, Cblock& fns, Cblock& call, int const reps);

/**
 * - NEVER simdize across the exit of [all 3] inner loops
 *   - Now scalar outer loops can easily by assigned to threads.
 *     - So vl0 is set (large) before entering inner loops, and
 *     - might have a single <em>short vl</em> at exit of inner loops.
 *
 * - We do not consider loop re-orderings -- these may simplify
 *   loop structure.  For example if only 2 of 3 inner loops are
 *   vectorized, they can nicely be the first 2 or the last 2.
 *   - and then we may get better code by the in-depth analysis
 *     of double-loop fusion \ref fl6.cpp
 */
template<typename T>
std::vector<std::vector<Triples<T>>> refTriples(
        T const ii,T const jj,T const kk,       // original loop limits
        T const iB, T const jB, T const kB,     // blocking factors
        T const iv, T const jv, T const kv)     // vectorization lengths
{
    int const v=1; // verbosity in [0,3]
    std::vector<std::vector<Triples<T>>> ret;
    ret.reserve( (17ULL*ii*jj*kk) / (16ULL*iv*jv*kv*16) );
    std::vector<Triples<T>> ix;
    ix.reserve( (17ULL*iB*jB*kB) / (16ULL*iv*jv*kv) );
    T io,jo,ko; // outer loop counters (step iB,jB,kB)
    T i, j, k ; // inner loop counters (step iv,jv,kv)
    T is,js,ks; // simd  loop counters (unit steps)
    auto ixout=[&](char const* pfx){
        cout<<pfx;
        for(size_t l=0; l<ix.size(); ++l)
            cout<<"{"<<ix[l].i<<","<<ix[l].j<<","<<ix[l].k<<"}";
        cout<<endl;
    };
    auto oloops=[&](){
        cout<<" outer"

        <<"{0.."<<ii<<"By"<<iB
        <<",0.."<<jj<<"By"<<jB
        <<",0.."<<kk<<"By"<<kB
        <<"}";
    };
    auto iloops=[&](){
        cout<<" inner"
        <<"{"<<io<<"To"<<min(io+iB,ii)<<"By"<<iv
        <<","<<jo<<"To"<<min(jo+jB,jj)<<"By"<<jv
        <<","<<ko<<"To"<<min(ko+kB,kk)<<"By"<<kv
        <<"}";
    };
    auto sloops=[&](){
        cout<<" ijk={"<<i<<","<<j<<","<<k<<"}simd"
        <<"{"<<i<<"To"<<min(i+iv,ii)<<(i>=min(i+iv,ii)?"XXX":"")
        <<","<<j<<"To"<<min(j+jv,jj)<<(j>=min(j+jv,jj)?"XXX":"")
        <<","<<k<<"To"<<min(k+kv,kk)<<(k>=min(k+kv,kk)?"XXX":"")
        <<"}";
    };
    if(v>=2){ cout<<"ref BEGIN "; oloops(); cout<<"\n"; }
    for(io=0; io<ii; io+=iB){           // outer loops, scalar indices
        for(jo=0; jo<jj; jo+=jB){       // blocking factors
            for(ko=0; ko<kk; ko+=kB){   // iB, jB, kB
                // Note: vectorization might want to change loop order
                //       here.  Ex. i and k vectorized, j not is strange.
                // Perhaps handle by filtering "difficult" BlockingPlans
                ix.clear();
                if(v>=2) iloops();
                for(i=io; i<min(io+iB,ii); i+=iv){           // inner loop vectorizations
                    for(j=jo; j<min(jo+jB,jj); j+=jv){       // with iv,jv,kv --> vectors
                        for(k=ko; k<min(ko+kB,kk); k+=kv){   // a[vl], b[vl], c[vl]
                            // Before simdization, loop increments iv, jv, kv
                            // would all be one.
                            //
                            // Simdization of inner loops creates a virtual set
                            // of three loops with following CONVENTION:
                            if(v>=2){ sloops(); cout<<endl; }
                            for(is=i; is<i+min(i+iv,ii); ++is){       // iv*jv*kv is normal
                                for(js=j; js<min(j+jv,jj); ++js){     // vector length
                                    for(ks=k; ks<min(k+kv,kk); ++ks){ // s for simd
                                        ix.push_back({is,js,ks});
                                    }}}
                            if(v>=3) ixout("ref: ");
                            // when some iloops execute "past end", inner loops
                            // can execute zero times (ix may be empty)
                            if(!ix.empty()){ ret.push_back(ix); ix.clear(); }
                        }}}
            }}}
    if(v>=2) cout<<"ref END ["<<ret.size()<<" SIMD inner kernels]"<<endl;
    else if(v>=1) cout<<" ref["<<ret.size()<<" krn@vl "<<iv*jv*kv<<"]"<<endl;
    return ret;
};
template<typename T>
std::vector<std::vector<Triples<T>>> refTriples( BlockingPlan const& p )
{
    return refTriples<T>(p.ii, p.jj, p.kk, p.iB, p.jB, p.kB, p.iv(), p.jv(), p.kv());
}

/** make triple-loop blocking plans and analyze their cost.
 * \return sorted list of BlockingPlan, best first. */
std::vector<BlockingPlan> mkBlockingPlans(int const ii, int const jj, int const kk);
void prtCode(BlockingPlan const& p);

static ostringstream oss;

struct BlockingBase{
    BlockingBase(
            int const which
            , std::string name=""
            , int const v=0/*verbose*/)
        : pr((name.empty()?std::string{"BlockingBase"}:name), "C", v),
        outer_(nullptr), inner_(nullptr), krn_(nullptr)
        {/*not yet usable*/}
    ~BlockingBase() {if(krn_){ delete krn_; krn_=nullptr;}};
    cprog::Cblock& outer();     ///< outside outer loops, often func scope
    cprog::Cblock& inner();     ///< where the fused-loop goes
    KrnBlk3& krn();
    cprog::Cunit pr;
  protected:
    cprog::Cblock* outer_;       ///< outside outer loops, often top-level function scope
    cprog::Cblock* inner_;       ///< where the fused-loop goes
    KrnBlk3* krn_;  ///< owned, references *outer_ and *inner_
};

struct BlockingMain final : public BlockingBase {
    BlockingMain(int const krn, int const v=0/*verbose*/);
};

int main(int argc, char**argv){
    int ii=20, jj=3, kk=96;
    int a=0, opt_h=0;
    if(argc > 1){
        if (argv[1][0]=='-'){
            char *c = &argv[1][1];
            if (*c=='h'){
                cout<<" genBlock II JJ KK"<<endl;
                cout<<"   MVL = max vector length = 256 (fixed)"<<endl;
                cout<<"   II    = 1st loop a=0..II-1"<<endl;
                cout<<"   JJ    = 2nd loop b=0..JJ-1"<<endl;
                cout<<"   KK    = 3rd loop c=0..KK-1"<<endl;
                cout<<" triple loop --> loop over vector registers a[VLEN], b[VLEN] c[VLEN]"<<endl;
                opt_h = 1;
            }
            ++a; // one extra arg was used up
        }
    }
    if(opt_h) return 0;

    if(argc > a+1) ii = atof(argv[a+1]);
    if(argc > a+2) jj = atof(argv[a+2]);
    if(argc > a+3) kk = atof(argv[a+3]);
    cout<<" genBlock ii="<<ii<<", jj="<<jj<<", kk="<<kk<<endl;
    
    vector<BlockingPlan> bps = mkBlockingPlans(ii,jj,kk);
    cout<<" sorted BlockingPlan:"<<endl;
    for(auto &bp: bps){
        std::string s=bp.str();
        cout<<s<<endl;
    }
    cout<<"\n Code for [up to] first 5 plans:"<<endl;
    for(int i=0; i<min(bps.size(),size_t{5}); ++i){
        cout<<"----- BlockPlan #"<<i<<" "<<string(70,'-')<<endl;
        prtCode(bps[i]);
        auto gold = refTriples<int>(bps[i]);
        cout<<"Ref output:"<<endl;
        std::vector<int> tmp; tmp.reserve(256);
        for(size_t g=0U; g<gold.size(); ++g){
            auto const& vt = gold[g];           // vector of Triple {i,j,k}
            int const n=8; int const w=2;
            tmp.clear();
            for(size_t i=0U; i<vt.size(); ++i) tmp.push_back(vt[i].i);
            cout<<setw(4)<<g<<" a["<<vt.size()<<"]: "
                <<vecprt(n,w, tmp, tmp.size())<<endl;
            tmp.clear();
            for(size_t i=0U; i<vt.size(); ++i) tmp.push_back(vt[i].j);
            cout<<"     b["<<vt.size()<<"]: "
                <<vecprt(n,w, tmp, tmp.size())<<endl;;
            tmp.clear();
            for(size_t i=0U; i<vt.size(); ++i) tmp.push_back(vt[i].k);
            cout<<"     c["<<vt.size()<<"]: "
                <<vecprt(n,w, tmp, tmp.size())<<endl;;
        }
    }

    //
    // remind myself how to generate a dll of routines [and invoke them]
    //
    // Begin with producing a compilable 'tmpScaffold.c`
    // with one [or more] `timeit` routines.
    //
    cout<<"\n BlockingMain scaffold:"<<endl;
    BlockingMain bt(KRNBLK3_NONE, 1/*verbose*/);
    cout<<bt.pr.str()<<endl;
    ofstream ofs("tmpScaffold.c");
    ofs<<bt.pr.str()<<endl;
    ofs.close();
    //
    //  now produce a library of timeit_foo functions,
    //  load the dll, and invoke each of them in turn (right away)
    //  TODO use dllbuild.hpp functionality
    //

    cout<<"\nGoobye"<<endl;
}

/** inject timing utility, init and teardown code. */
void timeit_prep(Cblock& utils, Cblock& main)
{    
    if(tag_once(utils,"timeit")){
        Cblock& tmit = utils["timeit"];
        tmit>>"static double cyc2ns = 1.0; // really cycle2ns()";
        main["first"]["+timeit"]
            >>"cyc2ns = cycle2ns();"
            >>"printf(\" cyc2ns = %f\\n\", cyc2ns);"
            ;
        tmit
            >>"static inline uint64_t clock_ns() {"
            >>"    return (uint64_t)(__cycle()*cyc2ns+0.5);"
            >>"}"
            >>"static inline unsigned long long to_ns(double avg_cyc){"
            >>"    return (unsigned long long)( avg_cyc * cyc2ns );"
            >>"}"
            >>"static inline double to_ns_f(double avg_cyc){"
            >>"    return ( avg_cyc * cyc2ns );"
            >>"}"
            ;
        tmit>>"static uint64_t bogus=0;";
        main["last"]["-timeit"]>>"printf(\"bogus=%llu\\n\", (unsigned long long)bogus);";
    }
}

Cblock& timeit_foo(char const* const foo, Cblock& fns, Cblock& call, int const reps)
{
    //auto timeit = [&](char const* foo) -> Cblock&
    //
    // under Cblock 'fns', add a timing subroutine called 'timeit_foo'
    // leaving an empty Cblock (our return value) where a kernel can be put.
    //
    Cunit& pr = fns.getRoot();
    CBLOCK_SCOPE(fn_foo,OSSFMT("uint64_t timeit_"<<foo<<"( int const reps )"),pr,fns);
    fn_foo["first"].setType("FUNCTION")
        >>"static uint64_t state=12345ULL; // minimal PCG32 impl, inc=rep|1"
        >>"uint64_t t=0;"
        ;
    CBLOCK_SCOPE(tloop,"for(uint64_t rep=0; rep<(uint64_t)reps; ++rep)",pr,fn_foo);
    tloop["init"]
        >>"uint64_t oldstate = state;"
        >>"state = state * 6364136223846793005ULL + ((uint64_t)rep|1ULL);"
        >>"oldstate =  oldstate^(oldstate >> 18u);"
        >>"uint64_t const rot = oldstate>>59u;"
        >>"uint64_t const seed = (oldstate>>rot) | (oldstate<<(64-rot)); // seed ~ a PCG random sequence"
        >>"uint64_t const t0 = __cycle(); // NOT careful. e.g. no cpuid op HERE for x86"
        >>"// --- kernel "<<foo<<" "<<string(40,'-')
        ;
    Cblock& kern = tloop[foo];      // MUST modify 'bogus' unpredicatably
    tloop["time"]
        >>"// --- kernel "<<foo<<" "<<string(40,'-')
        >>"uint64_t const t1 = __cycle();"
        >>"t += t1 - t0;"
        ;
    tloop["last"];
    fn_foo["ret"]>>"return (double)t / reps ; // return avg cycle count of 1 'kern'";
    //
    // under Cblock 'call', inject a call to `timeit_foo` and print the time
    //
    CBLOCK_SCOPE(callfoo,"if(1)",pr,call);
    callfoo
        >>OSSFMT("int const foo_reps = "<<reps<<";")
        >>OSSFMT("double "<<foo<<"_cycles = timeit_"<<foo<<"( foo_reps );")
        >>OSSFMT("uint64_t "<<foo<<"_ns = to_ns("<<foo<<"_cycles);")
        >>OSSFMT("printf(\" %s{%llu reps avg %llu cycles = %llu ns}\\n\", \""<<foo<<"\",")
        >>"        (unsigned long long)foo_reps,"
        >>OSSFMT("        (unsigned long long)"<<foo<<"_cycles,")
        >>OSSFMT("        (unsigned long long)"<<foo<<"_ns);")
        ;
    return kern;
};
BlockingMain::BlockingMain(int const krn, int const v/*=0,verbose*/)
: BlockingBase(krn, "KrnBlk3", v) // pr set up, other ptrs still NULL
{
    // upper-level tree structure, above the fused-loop.
    pr.root["first"];                           // reserve room for preamble, comments
    pr.root["features"]
        >>"#define _POSIX_C_SOURCE 200809L";
    auto& inc = pr.root["includes"];
    //inc["velintrin.h"]>>"#include \"veintrin.h\"";
    //inc["velintrin.h"]>>"#include \"velintrin.h\"";
    inc["stdint.h"]>>"#include <stdint.h>";
    inc["stdio.h"]>>"#include <stdio.h>";

    // create a somewhat generic tree
    auto& utils = pr.root["utils"];
    auto& fns = pr.root["fns"];
    fns["first"]; // reserve a node for optional function definitions
    CBLOCK_SCOPE(main,"int main(int argc,char**argv)",pr,fns);
    // NEW: use tag function scope, for upward-find operations
    //      will require extending the find string to match tag!
    main.setType("FUNCTION");
    // outer path root.at("**/main/body")
    set_once(inc,"timer", "#include \"../timer.h\"");
    timeit_prep(utils,main); // inject timing function helper code

    { // **HERE** is a test kernel, injected into a `timeit_foo` function.
        // - `timeit(foo)`:
        //   1. emit boilerplate 'timeit_foo' function too utils[foo]
        //   2. call it and print avg time in main[foo]
        auto& krnBlk = timeit_foo("foo",utils,main,1000);
        //
        // - to avoid compiler over-optimization,
        //   - kernel MUST [finally] modify `bogus`,
        //   - and CAN use 'seed' to help avoid compiler optimization
        // - `bogus` can be modified \em inside timing block ( \c timeit("foo") )
        //   - or \em outside ( \c timeit("foo")["../last"] )
        //
        krnBlk
            >>"uint64_t fooOut = seed;"
            >>"for(int j=0; j<1000; ++j) fooOut ^= fooOut*23456789ULL+j;"
            ;
        // timing = ADD,MUL,ADD,XOR + loop overhead (INCR, BRANCH)
        // the loop overhead IS timed, but the `bogus` update need not be timed
        krnBlk["../last"]>>"bogus ^= fooOut; // somehow update bogus using fooOut [,seed,...]";
        //
        // End example kernel 'foo'
        //
    }

    main["init"]>>"uint64_t const nrep = 1000;";
    //CBLOCK_FOR(loop1,-1,"for(int iloop1=0; iloop1<nrep; ++iloop1)",main);
    //CBLOCK_FOR(loop2,-1,"for(int iloop2=0; iloop2<1; ++iloop2)",loop1);
    //if(v){
    //    main["first"]>>"printf(\"BlockingMain outer loop\\n\");";
    //    loop2  >>"printf(\"BlockingMain inner loop @ (%d,%d)\\n\",iloop1,iloop2);";
    //}

    //loop2["first"]; // --> 'fd' inner setup code used by all splits

    //if(v>1){
    //    cout<<"\nScaffold";
    //    cout<<pr.tree()<<endl;
    //}

    //this->outer_ = &main;  // outer @ root.at("**/main/body")
    //this->inner_ = &loop2;    // inner @ outer.at("**/loop2/body")

    //if(v>2){
    //    cout<<" outer @ "<<outer_->fullpath()<<endl;
    //    cout<<" inner @ "<<inner_->fullpath()<<endl;
    //}

    //this->krn_ = mkBlockingTestKernel(krn,*outer_,*inner_ /*, defaults?*/ );
}
std::string BlockingPlan::str() const {
    oss.clear(); oss.str("");
    auto prtOuter=[](int const nloop, int const to, int const by, char const* By="By"){
        oss <<" ";
        if(nloop==1) oss <<"once";
        else{
            oss <<nloop<<"*(To"<<to;
            if(by!=1) oss <<By<<by;
            oss <<")";
        }
    };
    auto prtInner=[](int const nloop, int const to, int const by, char const* By="By"){
        oss <<" "<<nloop<<"*";
        if(to > 1){
            oss <<"(To"<<to;
            if(by!=1) oss <<By<<by;
            oss <<")";
        }else{
            assert(by>0);
        }
    };
    prtOuter(iLpo,ii,iB);
    prtOuter(jLpo,jj,jB);
    prtOuter(kLpo,kk,kB);
    auto outer = oss.str();
    oss.str("");
    prtInner(iLp,iB,iv(),"Vl");
    prtInner(jLp,jB,jv(),"Vl");
    prtInner(kLp,kB,kv(),"Vl");
    auto inner = oss.str();
    oss.str("");
    oss <<left<<setw(30)<<outer<<setw(30)<<inner;
    oss <<" {"<<" "<<ijkOuter()<<"*"<<ijkInner()<<"="<<kerns()<<" krn@Vl"<<kVl
        <<" $"<<ijkCost()
        <<" ["<<iCost<<"+"<<jCost<<"+"<<kCost
        <<"]}";
    auto ret=oss.str();
    oss.clear(); oss.str("");
    return ret;
}
/**
 * \todo use set of costs, to correctly do "all else equal" logic
 *
 * \todo current krn+cost model gives WRONG result sometimes.
 * Ex. 13 x 13 x 13 --> 12-krn solutions have triple-vl induction (very bad)
 * ```
 * 2*(To13By7) 2*(To13By7) 3*(To13By5) 1*(To7Vl7) 1*(To7Vl7)   1*(To5Vl5)   { 12*1=12 krn@Vl245 $78.07 [6+26+46.07]}
 * 13*(To13)   once        once        1*         1*(To13Vl13) 1*(To13Vl13) { 13*1=13 krn@Vl169 $40.0501 [0+0+40.0501]}
 * ```
 * but the 13-krn solution with a trivial inner-loop induction!
 * Consider that triple-vl induction is maybe 6-10 vecops (guess), ~O(small kernel)
 * So we really need a scalar|vector op count estimate (w/ estimate for kernel op count)
 */
std::vector<BlockingPlan> mkBlockingPlans(int const ii, int const jj, int const kk)
{
    vector<BlockingPlan> vBp;
    int minInnerKernels = numeric_limits<int>::max();
#define MVL 256
    // Initial Loop:
    //      for(i=0..ii by 1)
    // --> Outer Loop: always non-simd
    // for(io=0..ii by iB) :            iLoopo = (ii+iB-1)/iB;
    //                                  iB=iVl/ii<MVL/ii
    // and Inner Loop:
    // for(i=io..min(io+iB,ii) by 1) :  iVl=1 or iB;
    //                                  iLoopi = (iB+iVl-1)/iVl
    //
    // iB in [1..ii] such that iB*ii <= MVL, or iB <= MVL/ii
    int const vv=0; // verbose looping summary?
    int const v=0;  // verbose inner summary?
    int const iBmax = max(1,min(MVL/ii, ii));
    cout<<" iBmax ~ MVL/ii = "<<MVL/ii<<endl;
    float iCost=0.f, jCost=0.f, kCost=0.f;
    for(int iB=1; iB<=iBmax; ++iB){
        //int iVl = iB*ii;
        //int iLp = 1;
        //if( iVl > MVL ){ iVl=MVL; iLp = (ii+iB-1)/iB; }
        // OUTER loop
        int iLpo = (ii+iB-1)/iB;
        // actually 2 cases, iVL allowed (<=MVL, with iLp=1) or not (iVl=MVL, and iLp > 1)
        for(int ic=0; ic<2; ++ic){
            int iVl=0, iLp=0;
            iCost=0.f;
            switch(ic){
              case(0): iVl=1;       // inner loop non-SIMD
                       iLp=iB;
              break;
              default:
              case(1): if(iB>1){    // inner loop SIMD
                           iVl=ve_vlen_suggest(iB);  // iB when <= MVL
                           iLp=(iB+iVl-1)/iVl;
                       }
            }
            if(iVl<=0) continue;
            if(vv>=1) cout<<" ii="<<ii<<" iB="<<iB<<char('a'+ic)
                <<" iVl="<<iVl<<" iLpo="<<iLpo<<" iLp="<<iLp;

            if(iB!=1 && iB!=ii) iCost+=3;
            if(iLpo * iB != ii) iCost+=3;
            if(iLp > 1) iCost+=3;
            //if(!positivePow2(iVl)) iCost+=0.;

            // jB can got from 1 to jj such that iVL*jB <= MVL, or jB <= MVL/iVL
            int const jBmax = max(1,min(MVL/iVl,jj));
            if(vv>=1)cout<<"  jBmax ~ MVL/(iVl["<<iVl<<"]) = "<<MVL/iVl<<endl;
            assert(iVl<=MVL);

            for(int jB=1; jB<=jBmax; ++jB){
                //int jVl = jB*jj*iVl;
                //int jLp = 1;
                //if( jVl > MVL ){ jVl=iVl; jLp = (jj+jB-1)/jB; }
                int jLpo = (jj+jB-1)/jB;
                int jVlPrev=0, jVl=0, jLp=0;
                for(int jc=0; jc<2; ++jc){
                    jCost = 0.f;
                    switch(jc){
                      case(0): jVl=iVl;
                               jLp=jB;
                      break;
                      case(1): if(jB>1){
                                   jVl=ve_vlen_suggest(iVl*jB);
                                   jLp=(jB+jVl-1)/jVl;
                               }
                      break;
                      default: // needs more analysis:
                      case(2): if(jB>1){
                                   jVl=ve_vlen_suggest(iB*jB);
                                   jLp=(jB+jVl-1)/jVl;
                                   iCost=0.f; jCost=0.5f;
                               }
                    }
                    if(jVl==jVlPrev) continue;
                    jVlPrev = jVl;
                    if(jVl<=0) continue;
                    if(vv>=2) cout<<" jj="<<jj<<" jB="<<jB<<char('a'+jc)
                        <<" jVl="<<jVl<<" jLpo="<<jLpo<<" jLp="<<jLp;

                    if(jB!=1 && jB!=jj) jCost+=3;
                    if(jLpo * jB != jj) jCost+=3;
                    if(jLp > 1) jCost+=2;
                    if(iVl>1 && !positivePow2(jVl/iVl)) jCost+=20;

                    // kB ...
                    int const kBmax = max(1,min(MVL/jVl,kk));
                    if(vv>=2)cout<<"  kBmax ~ MVL/(jVl["<<jVl<<"]) = "<<MVL/jVl<<endl;
                    assert(jVl<=MVL);
                    for(int kB=1; kB<=kBmax; ++kB){
                        int kLpo = (kk+kB-1)/kB;
                        int kVlPrev=0, kVl=0, kLp=0;
                        for(int kc=0; kc<2; ++kc){
                            kCost = 0.0f;
                            switch(kc){
                              case(0): kVl=jVl;
                                       kLp=kB;
                              break;
                              case(1): if(kB>1){
                                           kVl=ve_vlen_suggest(jVl*kB);
                                           kLp=(kB+kVl-1)/kVl;
                                       }
                              break;
                              default: // needs more analysis:
                              case(2): if(kB>1){
                                           kVl=ve_vlen_suggest(iB*jB*kB);
                                           kLp=(kB+kVl-1)/kVl;
                                           iCost=0.f; jCost=0.f; kCost=0.9f;
                                       }
                            }
                            if(kVl==kVlPrev) continue;
                            kVlPrev = kVl;
                            if(kVl<=0) continue;
                            assert(kVl<=MVL);

                            if(kB!=1 && kB!=kk) kCost+=3;
                            if(kLpo * kB != kk) kCost+=3;
                            if(kLp > 1) kCost+=1;
                            if(jVl>1 && !positivePow2(kVl/jVl)) kCost+=40;
                            // slightly penalize long vector lengths (so get short vector length, all else equal)
                            kCost += (kVl/32) * 0.01;
                            // "all else equal" prefer oloop order iLp<jLp<kLp
                            // oloop [incr+branch] count = iLp + iLp*jLp + iLp*jLp*kLp
                            //int const oloopIncrCount = iLp*(1+jLp*(1+kLp));
                            //kCost += oloopIncrCount*0.0001;
                            // Use simpler cost for "right permutation of outer loops"
                            float const oloopBadPerm = (iLpo>jLpo)*0.0001 + (jLpo>kLpo)*0.0002; // 0,1,2,3
                            float const iloopBadPerm = (iLp >jLp )*0.0004 + (jLp >kLp )*0.0008;
                            kCost += oloopBadPerm + iloopBadPerm;
                            //if(oloopBadPerm>0.0) cout<<" iLpo,jLpo,kLpo="<<iLpo<<","<<jLpo<<","<<kLpo<<"  oloopBadPerm = "<<oloopBadPerm<<endl;
                            int const ijkOuter = iLpo*jLpo*kLpo;
                            int const ijkInner = iLp*jLp*kLp;
                            int const kerns = ijkOuter * ijkInner;

                            if(vv>=3) cout<<" kk="<<kk<<" kB="<<kB<<char('a'+kc)
                                <<" kVl="<<kVl<<" kLpo="<<kLpo<<" kLp="<<kLp
                                    <<" ijkOuter="<<ijkOuter<<" ijkInner="<<ijkInner
                                    //<<" kerns="<<kerns<<" cost="<<iCost+jCost+kCost
                                    <<" $"<<iCost<<"+$"<<jCost<<"+$"<<kCost<<" =$"<<iCost+jCost+kCost
                                    <<endl;

                            BlockingPlan bp = {
                                .ii=ii, .jj=jj, .kk=kk,
                                .iB=iB, .jB=jB, .kB=kB,
                                .iVl=iVl, .jVl=jVl, .kVl=kVl,
                                .iLpo=iLpo, .jLpo=jLpo, .kLpo=kLpo,
                                .iLp=iLp, .jLp=jLp, .kLp=kLp,
                                .iCost=iCost, .jCost=jCost, .kCost=kCost,
                            };
                            int const good = 0
                                + (kerns <= minInnerKernels)
                                + (kerns < minInnerKernels);
                            if(v >= 3-good){
                                cout<<(good==0?"bad  ": good==1?" ok  ": "BEST ");
                                //cout<<left<<setw(25)<<OSSFMT("iB,jB,kB="<<iB<<","<<jB<<","<<kB);
                                cout<<left<<setw(15)<<OSSFMT( //"iB,jB,kB="<<
                                        iB<<char('a'+ic)
                                        <<' '<<jB<<char('a'+jc)
                                        <<' '<<kB<<char('a'+kc));
                                cout<<bp.str()<<endl;
#if 0
                                cout<<iLpo<<"*To"<<ii<<","<<iB<<":"<<iLp<<"@vl"<<iVl<<" "
                                    <<jLpo<<"*To"<<jj<<","<<jB<<":"<<jLp<<"@vl"<<jVl<<" "
                                    <<kLpo<<"*To"<<kk<<","<<kB<<":"<<kLp<<"@vl"<<kVl<<" "
                                    <<" {o"<<ijkOuter<<"*i"<<ijkInner<<" "<<kerns<<" kerns"
                                    <<" $"<<iCost<<"+$"<<jCost<<"+$"<<kCost
                                    <<" =$"<<iCost+jCost+kCost
                                    <<"}"<<endl;
#endif
                            }
                            //if(good>=2) vBp.clear(); // to avoid all non-minimal-kerns ?
                            if(good>=1){ // best or tied? save it!
                                vBp.push_back(bp);
                            }
                            if(good==2) minInnerKernels = kerns;
                        }
                    }
                }
            }
        }
    }
    if(1){
        std::stable_sort( vBp.begin(), vBp.end(),
                [](BlockingPlan const& a, BlockingPlan const& b)->bool{
                if( a.kerns() < b.kerns() ) return true;
                if( a.ijkCost() < b.ijkCost() ) return true;
                return false;
                });
    }
    if(!vBp.empty()){
        cout<<"Found "<<vBp.size()<<" decompositions.  Best had just "
            <<vBp.front().kerns()<<" inner Kernel executions"<<endl;
    }else{
        cout<<"empty vector of BlockingPlan (huh?)"<<endl;
    }
    return vBp;
}

/** print code loop blocking (only a few simple tests, as in genBlock0.cpp). */
void prtCode(BlockingPlan const& p){
    p.prt();
    int const &ii=p.ii, &jj=p.jj, &kk=p.kk;
    cout<<"int vl="<<p.kVl<<";\n";
    cout<<"typedef int vr["<<p.kVl<<"];\n";
    cout<<"vr a, b, c, sq, tmp, ta, tb, tc;\n";
    cout<<"// outer loops: "<<p.iLpo<<" "<<p.jLpo<<" "<<p.kLpo<<"\n";
    if(p.iLpo>1){
        cout<<"for(int io=0; io<"<<ii<<"; io+="<<p.iB<<"){ //"<<(ii%p.iB==0?" norem":" rem")<<"\n";
    }else{
        cout<<"{int const io=0;\n";
    }
    if(p.jLpo>1){
        cout<<" for(int jo=0; jo<"<<jj<<"; jo+="<<p.jB<<"){ //"<<(jj%p.jB==0?" norem":" rem")<<"\n";
    }else{
        cout<<" {int const jo=0;\n";
    }
    if(p.kLpo>1){
        cout<<"  for(int ko=0; ko<"<<kk<<"; ko+="<<p.kB<<"){ //"<<(kk%p.kB==0?" norem":" rem")<<" "<<p.ijkOuter()<<" outer\n";
    }else{
        cout<<"  {int const ko=0;\n";
    }
    cout<<"   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;\n";
#if 0 // nosimd
    if(p.iB>1){
        cout<<"  for(int i=io; i<io+"<<p.iB<<"; ++i){ // iv="<<p.iv()<<"\n";
        cout<<"   FOR(n,vl) a[n] = ko + n%"<<p.jB*p.kB<<"; // ?\n";
    }else{
        cout<<"   for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
        cout<<"    FOR(n,vl) a[n] = ko;\n";
    }
    if(p.jB>1){
        cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv()<<"\n";
        cout<<"     FOR(n,vl) b[n] = ji + n%"<<p.kB<<"; // ?\n";
    }else{
        cout<<"   for(int j=jo; j==jo; ++j){ // jv="<<p.jv()<<"\n";
        cout<<"    FOR(n,vl) a[n] = ko;\n";
    }
    cout<<"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
    cout<<"      prtijk(i,j,k);\n";
#else // simd 'c'
    cout<<"   FOR(n,vl) sq[n] = n;\n";
    cout<<"   // inner loops: "<<p.iLp <<" "<<p.jLp <<" "<<p.kLp <<"\n";
    cout<<"   // inner vls  : "<<p.iv()<<" "<<p.jv()<<" "<<p.kv()<<"\n";
    cout<<"   // inner block: "<<p.iB  <<" "<<p.jB  <<" "<<p.kB  <<"\n";
    if(p.iB>1){
        cout<<"   for(int i=io; i<io+"<<p.iB<<"; i+="<<p.iv()<<"){    // A iB="<<p.iB<<" iv="<<p.iv()<<"\n";
        cout<<"    FOR(n,vl) a[n] = ko + sq[n]/"<<p.jB*p.kB<<";    // divmod by jB*kB="<<p.jB<<"*"<<p.kB<<"\n";
        cout<<"    FOR(n,vl) ta[n] = sq[n]%"<<p.jB*p.kB<<";\n";
    }else{
        //cout<<"  for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
        cout<<"   {int const i=io;\n";
        cout<<"    FOR(n,vl) a[n] = i;\n";
    }

    if(p.iB>1 && p.jB>1){
        cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // a jB="<<p.jB<<" jv="<<p.jv()<<"\n";
        cout<<"     FOR(n,vl) b[n] = jo + ta[n]/"<<p.jv()<<";\n";
    }else if(p.jB>1){
        cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // b jB="<<p.jB<<" jv="<<p.jv()<<"\n";
        cout<<"     FOR(n,vl) b[n] = jo + sq[n]/"<<p.kv()<<";        // kB="<<p.kB<<" kv="<<p.kv()<<"\n";
    }else{
        cout<<"    {int const j=jo;\n";
        cout<<"    FOR(n,vl) b[n] = jo;\n";
    }

    if(p.kB>1){
        cout<<"     for(int k=ko; k<ko+"<<p.kB<<"; k+="<<p.kv()<<"){    // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
        cout<<"      FOR(n,vl) c[n] = ko + sq[n]%"<<p.kv()<<"; // mod by kB="<<p.kB<<" [kv="<<p.kv()<<"] shortcut\n";
    }else{
        cout<<"     {int const k=ko;\n";
        cout<<"      FOR(n,vl) c[n] = ko;\n";
    }

    int const ijkB = p.iB*p.jB*p.kB;
    if(ijkB%p.kVl){
        cout<<"      vl = min(vl, sq[0]+vl - "<<ijkB%p.kVl<<");\n";
    }
    cout<<"      prtijk(i,j,k);\n";
    cout<<"      prt(a); prt(ta); prt(b); prt(c);\n";
    cout<<"      FOR(n,vl) sq[n] += vl;\n";
#endif
#if 0
    cout<<"   for(int il=0; il<"<<p.iLp<<"; il+="<<p.iv()/p.iLp<<"){\n";
    cout<<"    FOR(n,vl) a[n]  = io + (il*"<<p.iv()<<"+n)/"<<p.jB*p.kB<<";\n";
    cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv()<<"\n";
    cout<<"     FOR(n,vl) b[n]  = jo + n/"<<p.kB<<";\n";
    if(1 || p.kB>1){
        cout<<"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
        //cout<<"      FOR(n,vl) c[n]  = ko + n%"<<p.kB<<";\n";
    }else{
        cout<<"     { // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
        //cout<<"      FOR(n,vl) c[n]  = ko;\n";
    }
    //cout<<"      FOR(n,vl) c[n]  = ko + n%"<<p.kB<<";\n";           // ko + 012301230123 (kB=4)
    //cout<<"      FOR(n,vl) tmp[n]= n/"<<p.kB<<";\n";                //      000011112222...
    //cout<<"      FOR(n,vl) b[n]  = jo + tmp[n]%"<<p.jB<<";\n";      // jo + 000011110000 (jB=2)
    //cout<<"      FOR(n,vl) tmp[n]= tmp[n]/"<<p.iB<<";\n";           //      000000001111111122222222
    //cout<<"      FOR(n,vl) a[n]  = io + tmp[n];\n";                 // io + --"--
    cout<<"      prt(a); prt(b); prt(c);\n";
#endif
    cout<<"}}}}}}"<<endl;
#if 0
    // Here are some sample loops, from genBlock0.cpp
#define FOR(VAR,VL) for(int VAR=0; VAR<(VL); ++VAR)
#define prt(a) do{cout<<#a<<": ";for(int u=0;u<vl;++u)cout<<' '<<a[u]; cout<<endl;}while(0)
#define prtijk(i,j,k) do{cout<<" {"<<i<<","<<j<<","<<k<<"}"<<endl;}while(0)
    if(1){
        // 1*To3,3:1@vl3 1*To3,3:1@vl9 1*To3,3:1@vl27  {o1*i1 1 kerns $0+$0.5+$0.7 =$1.2}
        int vl=27;
        typedef int vr[27];
        vr a, b, c, sq, ta;
        // outer loops: 1 1 1
        {int const io=0;
            {int const jo=0;
                {int const ko=0;
                    FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;
                    FOR(n,vl) sq[n] = n;
                    // inner loops: 1 1 1
                    // inner vls  : 3 3 3
                    // inner block: 3 3 3
                    for(int i=io; i<io+3; i+=3){    // A iB=3 iv=3
                        FOR(n,vl) a[n] = ko + sq[n]/9;    // divmod by jB*kB=3*3
                        FOR(n,vl) ta[n] = sq[n]%9;
                        for(int j=jo; j<jo+3; j+=3){    // 0 jB=3 jv=3
                            FOR(n,vl) b[n] = jo + ta[n]/3;
                            for(int k=ko; k<ko+3; k+=3){    // kv=3 vl=27
                                FOR(n,vl) c[n] = ko + sq[n]%3; // mod by kB=3 [kv=3] shortcut
                                prtijk(i,j,k);
                                prt(a); prt(ta); prt(b); prt(c);
                                FOR(n,vl) sq[n] += vl;
                            }}}}}}
    }
    if(1){
        // 1*To1,1:1@vl1 1*To3,3:1@vl3 1*To3,3:1@vl9  {o1*i1 1 kerns $0+$0+$0.7 =$0.7}
        int vl=9;
        typedef int vr[9];
        vr a, b, c, sq, tmp, ta, tb, tc;
        // outer loops: 1 1 1
        {int const io=0;
            {int const jo=0;
                {int const ko=0;
                    FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;
                    FOR(n,vl) sq[n] = n;
                    // inner loops: 1 1 1
                    // inner vls  : 1 3 3
                    // inner block: 1 3 3
                    {int const i=io;
                        FOR(n,vl) a[n] = i;
                        for(int j=jo; j<jo+3; j+=3){    // 1 jB=3 jv=3
                            FOR(n,vl) b[n] = jo + sq[n]/3/*kB*/; // 
                            for(int k=ko; k<ko+3; k+=3){    // kv=3 vl=9
                                FOR(n,vl) c[n] = ko + sq[n]%3; // mod by kB=3 [kv=3] shortcut
                                prtijk(i,j,k);
                                prt(a); prt(ta); prt(b); prt(c);
                                FOR(n,vl) sq[n] += vl;
                            }}}}}}
    }
    if(1){
        // 1*To1,1:1@vl1 1*To3,3:1@vl3 1*To3,3:1@vl9  {o1*i1 1 kerns $0+$0+$0.7 =$0.7}
        int vl=9;
        typedef int vr[9];
        vr a, b, c, sq, tmp, ta, tb, tc;
        // outer loops: 1 1 1
        {int const io=0;
            {int const jo=0;
                {int const ko=0;
                    FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;
                    FOR(n,vl) sq[n] = n;
                    // inner loops: 1 1 1
                    // inner vls  : 1 3 3
                    // inner block: 1 3 3
                    {int const i=io;
                        FOR(n,vl) a[n] = i;
                        for(int j=jo; j<jo+3; j+=3){    // b jB=3 jv=3
                            FOR(n,vl) b[n] = jo + sq[n]/3;        // kB=3 kv=3
                            for(int k=ko; k<ko+3; k+=3){    // kv=3 vl=9
                                FOR(n,vl) c[n] = ko + sq[n]%3; // mod by kB=3 [kv=3] shortcut
                                prtijk(i,j,k);
                                prt(a); prt(ta); prt(b); prt(c);
                                FOR(n,vl) sq[n] += vl;
                            }}}}}}
    }
#endif
}


// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
