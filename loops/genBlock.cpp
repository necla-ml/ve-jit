/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * loop blocking plan + Ref output + jit with kernel 'none' and 'print'.
 *
 * Here we emit a single JIT file with multiple 'C' tests
 * as opposed to libvednn strategy where every test is a separate file.
 *
 * 3 original loops --> 6 loops [9 for vectorization]
 *
 * The loop blocking strategy is very simplistic
 * We mostly try for easy cases, where vector length is 1 (scalar)
 * or has a simple relation to the inner loop lengths.
 *
 * 1. TODO
 * - add more "real" kernels: CHECK, and HASH impls should be supported
 * - execute jit code "right away" (using dllbuild techniques, see cjitDemo.cpp)
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
    /// \group primary settings
    //@{
    int ii, jj, kk;
    int iB, jB, kB;
    int iVl, jVl, kVl;      // these increase (bounded by MVL), equiv to {iv(),jv(),kv()}
    //@}
    /// \group secondary variables
    //@{
    int iLpo, jLpo, kLpo;
    int iLp, jLp, kLp;
    float iCost, jCost, kCost;
    //@}
    int ijkOuter() const {return iLpo*jLpo*kLpo;}
    int ijkInner() const {return iLp*jLp*kLp;}
    int kerns() const {return ijkOuter() * ijkInner();}
    // Need a better cost model.
    double ijkCost() const {return iCost+jCost+kCost;}
    // per-loop simd lengths.   kVl == iv()*jv()*kv()
    int iv() const {return iVl;}
    int jv() const {return jVl/iVl;}
    int kv() const {return kVl/jVl;}
    std::string str() const;
    void prt(std::ostream& os) const { os<<"// "<<str()<<endl; }
    std::string suffix() const; ///< a canonical function-name suffix
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
Cblock& timeit_foo(std::string const& foo, Cblock& fns, Cblock& call, int const reps);

/** generate semi-correct loop blocking code.
 * only a few simple cases OK, as in genBlock0.cpp. */
std::string code(BlockingPlan const& p);

void prtCode(BlockingPlan const& p){
    cout<<code(p)<<endl;
}
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
        cout<<" ijk={"<<i<<","<<j<<","<<k<<"} simd-vl["<<iv<<","<<jv<<","<<kv<<"]"
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
                if(v>=2) iloops();
                for(i=io; i<min(io+iB,ii); i+=iv){           // inner loop vectorizations
                    for(j=jo; j<min(jo+jB,jj); j+=jv){       // with iv,jv,kv --> vectors
                        for(k=ko; k<min(ko+kB,kk); k+=kv){   // a[vl], b[vl], c[vl]
                            // Before simdization, loop increments iv, jv, kv
                            // would all be one.
                            //
                            // Simdization of inner loops creates a virtual set
                            // of three loops with following CONVENTION:
                            ix.clear();
                            if(v>=2){ sloops(); cout<<endl; }
                            assert(ix.size()==0);
                            //size_t pushes=0U;
                            for(is=i; is<min(i+iv,ii); ++is){       // iv*jv*kv is normal
                                for(js=j; js<min(j+jv,jj); ++js){     // vector length
                                    for(ks=k; ks<min(k+kv,kk); ++ks){ // s for simd
                                        //cout<<"    simd {"<<is<<","<<js<<","<<ks<<"}"<<endl;
                                        ix.push_back({is,js,ks});
                                        //++pushes;
                                    }}}
                            if(v>=3){
                                cout<<"ref["<<ix.size()<<"] : "; //cout<<" pushes<<" pushes ";
                                if(!ix.empty()){
                                    size_t l=0U;
                                    cout<<"{"<<ix[l].i<<","<<ix[l].j<<","<<ix[l].k<<"} ... ";
                                    l=ix.size()-1U;
                                    cout<<"{"<<ix[l].i<<","<<ix[l].j<<","<<ix[l].k<<"}"<<endl;
                                }
                            }
                            if(v>=4) ixout("ref: ");
                            // when some iloops execute "past end", inner loops
                            // can execute zero times (ix may be empty)
                            if(!ix.empty()){
                                if(v>=4) cout<<" save ix["<<ix.size()<<"] and clear.  ret.size()="<<ret.size()<<endl;
                                ret.push_back(ix);
                                ix.clear();
                                assert(ix.size()==0);
                            }
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
            int const krn
            , std::string name=""
            , int const v=0/*verbose*/)
        : pr((name.empty()?std::string{"BlockingBase"}:name), "C", v),
        fns_(nullptr), run_(nullptr), krn_(krn)
        {/*not yet usable*/}
    ~BlockingBase() {}
    cprog::Cblock& fns() const; ///< test routines go here
    cprog::Cblock& run() const; ///< init() function (possibly `main`)
    cprog::Cunit pr;
  protected:
    cprog::Cblock* fns_;        ///< outside outer loops, often top-level function scope
    cprog::Cblock* run_;        ///< where the fused-loop goes
    int const krn_;
};
inline cprog::Cblock& BlockingBase::fns() const { assert(fns_); return *fns_; }
inline cprog::Cblock& BlockingBase::run() const { assert(run_); return *run_; }

//inline KrnBlk3&       BlockingBase::krn() const { assert(krn_); return *krn_; }
/** a single [timing-test] subroutine */
struct BlockingMain final : public BlockingBase {
    BlockingMain(int const krn, int const v=0/*verbose*/);
    void add(BlockingPlan const& p);
  private:
    void add_none(BlockingPlan const& p);
    void add_print(BlockingPlan const& p);
    void add_check(BlockingPlan const& p);
    void add_hash(BlockingPlan const& p);

    /** emit a[],b[],c[] calculation loops.
     * \return innermost block which should do something with a,b,c. */
    static cprog::Cblock& emit_loops(BlockingPlan const& p, Cblock& krnBlk);

    /** Fill in innermost a,b,c block according to \c krn.
     * \c after should point to after the timing loop (not part of reported time).
     * Ex. after=krnBlk["..* /tloop/"]
     */
    void emit_print(cprog::Cblock& abc, cprog::Cblock& after);
};

#if 1==2
#include "../dllbuild.hpp"
    // following 2 (code and public symbol[s]) must coordinate
    void add_test(BlockingTests &bt, BlockingPlan const& p);
    void add_syms(DllFile &df, BlockingPlan const& p);
/** create a single source file with multiple symbols */
struct BlockingSubroutine : public BlockingBase {
    DllFile df; ///< test subroutine names go into \c vector<SymbolDecl> \c df.syms
};
struct BlockingTests : public BlockingBase {
    BlockingTests(int const krn, int const v=0/*verbose*/);
    cprog::Cblock& fns() const {assert(fns_); return *fns_;}
};
#endif

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
    // Begin with producing a compilable 'tmpScaffold.cpp`
    // with one [or more] `timeit` routines.
    //
    cout<<"\n BlockingMain scaffold:"<<endl;
    //BlockingMain bm(KRNBLK3_NONE, 1/*verbose*/);
    BlockingMain bm(KRNBLK3_PRINT, 1/*verbose*/);
    cout<<"Tree:\n"; bm.pr.dump(cout); cout<<endl; // might be long?
    cout<<bm.pr.str()<<endl;

    for(int i=0; i<min(bps.size(),size_t{5}); ++i){
        cout<<"----- BlockPlan #"<<i<<" "<<string(70,'-')<<endl;
        bm.add(bps[i]);
    }
    cout<<"Final Tree:\n"; bm.pr.dump(cout); cout<<endl; // might be long?

    ofstream ofs("tmpScaffold.cpp");
    ofs<<bm.pr.str()<<endl;
    ofs.close();
    //
    //  now produce a library of timeit_foo functions,
    //  load the dll, and invoke each of them in turn (right away)
    //  TODO use dllbuild.hpp functionality
    //
#if 1==2
    DllBuild dllbuild;
    {
        BlockingTests bt(KRNBLK3_NONE, 1/*verbose*/); // scaffold for tests (includes,support fns)
        cout<<"Tree:\n"; bt.pr.dump(cout); cout<<endl;
        cout<<bt.pr.str()<<endl;

        DllFile df;                     // source file[s] and public symbols
        df.basename = "genBlock";
        df.suffix = "-x86.cpp";
        for(int i=0; i<min(bps.size(),size_t{5}); ++i){
            cout<<"----- BlockPlan #"<<i<<" "<<string(70,'-')<<endl;
            add_test( bt, bps[i] );   // test subroutine --> BlockingTests
            add_symbol( df, bps[i] ); // public symbol of add_test --> DllFile
        }

        cout<<"Tree:\n"; bt.pr.dump(cout); cout<<endl;
        cout<<bt.pr.str()<<endl;
        df.code = bt.pr.str(); // BlockingTests --> single source file (finally)

        dllbuild.push_back(df); // sourcefile + syms --> DllBuild
    }

    // build dll, open it, read syms
    char const* subdir="tmp_genBlock";
    std::string mkEnv; // not used?
    unique_ptr<DllOpen> plib = dllbuild.safe_create(
            "genBlock", subdir, mkEnv );
    DllOpen& lib = *plib;

    // for all files, for all test symbols (in lib), call them
    for(size_t i=0U; i<lib.nSrc(); ++i){ // here, only one source file
        cout<<"\nsrc file #"<<i<<" syms";
        auto symnames = lib[i];
        for(auto s: symnames) cout<<" "<<s;
        cout<<endl;

        assert( symnames.size() >= 1 );
        for(auto s: symnames){
            void* symaddr = lib[s];
            // TODO: adapt next section
            // coerce to proper test function type
            typedef demoError_t (*JitFn)();
            JitFn jitfn = (JitFn)symaddr;
            // invoke the test function (it takes no parameters)
            cout<<" jit "<<symnames[0]<<"() ..."<<endl;
            jitfn();
        }
    }
#endif

    cout<<"\nGoobye"<<endl;
}

/** inject timing utility, init and teardown code. */
void timeit_prep(Cblock& utils, Cblock& main)
{
    if(tag_once(utils,"timeit")){
        Cblock& tmit = utils["timeit"];
        tmit>>"static double cyc2ns = 1.0; // really cycle2ns()";
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

        main["first"]["+timeit"]
            >>"cyc2ns = cycle2ns();"
            >>"printf(\" cyc2ns = %f\\n\", cyc2ns);"
            ;
        //main["init"]>>OSSFMT("uint64_t const nrep = "<<nrep<<";");
        main["last"]["-timeit"]>>"printf(\"bogus=%llu\\n\", (unsigned long long)bogus);";

        utils["oss"]>>"static ostringstream oss;";
        set_once(utils.getRoot()["includes"],"iomanip","#include <iomanip>");
        utils["vecprt"]
            >>"template<typename T>"
            >>"std::string vecprt(int const n, int const wide, T* v, int const vl){"
            >>"    std::ostringstream oss;"
            >>"    for(int i=0; i<vl; ++i){"
            >>"        if( i < n ){ oss<<\" \"<<std::setw(wide)<<v[i]; }"
            >>"        if( i == n && i < vl-n ){ oss<<\" ... \"; }"
            >>"        if( i >= n && i >= vl-n ){ oss<<\" \"<<std::setw(wide)<< v[i]; }"
            >>"    }"
            >>"    return oss.str();"
            >>"}"
            ;
    }
}

Cblock& timeit_foo(std::string const& foo, Cblock& fns, Cblock& call, int const reps)
{
    cout<<" timeit_foo( foo="<<foo<<", fns, call, reps="<<reps<<" );"<<endl;
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
        //>>"// "<<p.str(); // oh, do not have original blocking plan available
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
        >>OSSFMT("int      const foo_reps = "<<reps<<";")
        >>OSSFMT("double   const foo_cycles")
        >>OSSFMT("    = timeit_"<<foo<<"( foo_reps );")
        >>OSSFMT("uint64_t const foo_ns = to_ns(foo_cycles);")
        >>OSSFMT("printf(\" %s{%llu reps avg %llu cycles = %llu ns}\\n\",")
        >>OSSFMT("        \""<<foo<<"\",")
        >>"        (unsigned long long)foo_reps,"
        >>"        (unsigned long long)foo_cycles,"
        >>"        (unsigned long long)foo_ns );"
        ;
    return kern;
};
BlockingMain::BlockingMain(int const krn, int const v/*=0,verbose*/)
: BlockingBase(krn, "blk", v) // pr set up, other ptrs still NULL
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
    fns_ = &pr.root["fns"];
    (*fns_)["first"]; // reserve a node
    CBLOCK_SCOPE(run,"int main(int argc,char**argv)",pr,pr.root["main"]);
    // NEW: use tag function scope, for upward-find operations
    //      will require extending the find string to match tag!
    run.setType("FUNCTION");
    run_ = &run;
    run["first"]; // reserve a node
    // outer path root.at("**/main/body")

#if 0
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
#endif
}

void BlockingMain::add(BlockingPlan const& p){
    switch(krn_){
      case(KRNBLK3_NONE): add_none(p); break;
      case(KRNBLK3_HASH): add_hash(p); break;
      case(KRNBLK3_PRINT): add_print(p); break;
      case(KRNBLK3_CHECK): add_check(p); break;
      default: THROW(OSSFMT("Unsupported kernel, krn_ = "<<krn_));
    }
}
void BlockingMain::add_none(BlockingPlan const& p){
    set_once(pr["includes"],"timer.h", "#include \"../timer.h\"");
    timeit_prep(fns(),run()); // inject timing function helper code
    // - `timeit(foo)`:
    //   1. emit boilerplate 'timeit_foo' function to 'utils[foo]'
    //   2. call it and print avg time in 'main[foo]'
    auto subroutine = pr.name+p.suffix();
    auto& krnBlk = timeit_foo(subroutine, fns(), run(), 1000);
    cout<<"timeit_foo --> krnBlock @ "<<krnBlk.fullpath()<<endl;
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
}
void BlockingMain::add_hash(BlockingPlan const& p){
}

void BlockingMain::add_print(BlockingPlan const& p){
    set_once(pr["includes"],"timer.h", "#include \"../timer.h\"");
    timeit_prep(fns(),run()); // inject timing function helper code
    auto subroutine = pr.name+p.suffix();

    set_once(pr["includes"],"sstream", "#include <sstream>");
    set_once(pr["includes"],"iostream", "#include <iostream>");
    set_once(pr["includes"]["last"],"std","using namespace std;");

    auto& krnBlk = timeit_foo(subroutine, fns(), run(), 1);
    string descr = p.str();
    krnBlk["..*/tloop/.."] // before timing loop
        >>OSSFMT("cout<<\"BlockingPlan\\n "<<descr<<"\"<<endl;")
        ;
    krnBlk>>"uint64_t fooOut = seed;";

    //auto s = code(p);   // vectorized a[], b[], c[] outputs
    //krnBlk>>s;

    auto& abc = emit_loops(p, krnBlk);                  // kernel code for a[],b[],c[] 
    auto& untimed = abc["..*/tloop"]["last"];           // after timing loop
    cout<<" EMIT_PRINT !"<<endl;
    emit_print(abc, untimed);

    // this is still in timing loop:
    krnBlk["../last"]>>"bogus ^= fooOut; // somehow update bogus using fooOut [,seed,...]";
}

cprog::Cblock& BlockingMain::emit_loops(BlockingPlan const& p, Cblock& krnBlk){
    auto& pr = krnBlk.getRoot();
    // all of krnBlk points into some "tloop/body" of timeit_foo
    //auto& kf = krnBlk["../last"];
    CBLOCK_SCOPE(k,"",pr,krnBlk);
    // k["first"] ?
    auto& kabc = k["abc"];              // <-- our return value (innermost loop)
    auto& kz = k[".."]["last"];

    string descr = p.str();
    k>>OSSFMT("// "<<descr);
    int const &ii=p.ii, &jj=p.jj, &kk=p.kk;
    k>>CSTR(#define FOR(I,VL) for(int I=0;I<VL;++I)\n);
    k>>OSSFMT("int vl="<<p.kVl<<";\n");
    k>>OSSFMT("typedef int vr["<<p.kVl<<"];\n");
    k>>"vr a, b, c, sq, tmp, ta, tb, tc;\n";
    k>>OSSFMT("// outer loops: "<<p.iLpo<<" "<<p.jLpo<<" "<<p.kLpo<<"\n");
    if(p.iLpo>1){
        k>>OSSFMT("for(int io=0; io<"<<ii<<"; io+="<<p.iB<<"){ //"<<(ii%p.iB==0?" norem":" rem")<<"\n");
    }else{
        k>>"{int const io=0;\n";
    }
    if(p.jLpo>1){
        k>>OSSFMT(" for(int jo=0; jo<"<<jj<<"; jo+="<<p.jB<<"){ //"<<(jj%p.jB==0?" norem":" rem")<<"\n");
    }else{
        k>>" {int const jo=0;\n";
    }
    if(p.kLpo>1){
        k>>OSSFMT("  for(int ko=0; ko<"<<kk<<"; ko+="<<p.kB<<"){ //"<<(kk%p.kB==0?" norem":" rem")<<" "<<p.ijkOuter()<<" outer\n");
    }else{
        k>>"  {int const ko=0;\n";
    }
    k>>"   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;\n";
#if 0 // nosimd
    if(p.iB>1){
        k>>"  for(int i=io; i<io+"<<p.iB<<"; ++i){ // iv="<<p.iv()<<"\n";
        k>>"   FOR(n,vl) a[n] = ko + n%"<<p.jB*p.kB<<"; // ?\n";
    }else{
        k>>"   for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
        k>>"    FOR(n,vl) a[n] = ko;\n";
    }
    if(p.jB>1){
        k>>"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv()<<"\n";
        k>>"     FOR(n,vl) b[n] = ji + n%"<<p.kB<<"; // ?\n";
    }else{
        k>>"   for(int j=jo; j==jo; ++j){ // jv="<<p.jv()<<"\n";
        k>>"    FOR(n,vl) a[n] = ko;\n";
    }
    k>>"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
    k>>"      prtijk(i,j,k);\n";
#endif
    k>>OSSFMT("   FOR(n,vl) sq[n] = n;\n");
    k>>OSSFMT("   // inner loops: "<<p.iLp <<" "<<p.jLp <<" "<<p.kLp <<"\n");
    k>>OSSFMT("   // inner vls  : "<<p.iv()<<" "<<p.jv()<<" "<<p.kv()<<"\n");
    k>>OSSFMT("   // inner block: "<<p.iB  <<" "<<p.jB  <<" "<<p.kB  <<"\n");
    if(p.iB>1){
        k>>OSSFMT("   for(int i=io; i<io+"<<p.iB<<"; i+="<<p.iv()<<"){    // A iB="<<p.iB<<" iv="<<p.iv()<<"\n");
        k>>OSSFMT("    FOR(n,vl) a[n] = ko + sq[n]/"<<p.jB*p.kB<<";    // divmod by jB*kB="<<p.jB<<"*"<<p.kB<<"\n");
        k>>OSSFMT("    FOR(n,vl) ta[n] = sq[n]%"<<p.jB*p.kB<<";\n");
    }else{
        //k>>"  for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
        k>>"   {int const i=io;\n";
        k>>"    FOR(n,vl) a[n] = i;\n";
    }

    if(p.iB>1 && p.jB>1){
        k>>OSSFMT("    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // a jB="<<p.jB<<" jv="<<p.jv()<<"\n");
        k>>OSSFMT("     FOR(n,vl) b[n] = jo + ta[n]/"<<p.jv()<<";\n");
    }else if(p.jB>1){
        k>>OSSFMT("    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // b jB="<<p.jB<<" jv="<<p.jv()<<"\n");
        k>>OSSFMT("     FOR(n,vl) b[n] = jo + sq[n]/"<<p.kv()<<";        // kB="<<p.kB<<" kv="<<p.kv()<<"\n");
    }else{
        k>>"    {int const j=jo;\n";
        k>>"    FOR(n,vl) b[n] = jo;\n";
    }

    if(p.kB>1){
        k>>OSSFMT("     for(int k=ko; k<ko+"<<p.kB<<"; k+="<<p.kv()<<"){    // kv="<<p.kv()<<" vl="<<p.kVl<<"\n");
        k>>OSSFMT("      FOR(n,vl) c[n] = ko + sq[n]%"<<p.kv()<<"; // mod by kB="<<p.kB<<" [kv="<<p.kv()<<"] shortcut\n");
    }else{
        k>>"     {int const k=ko;\n";
        k>>"      FOR(n,vl) c[n] = ko;\n";
    }

    int const ijkB = p.iB*p.jB*p.kB;
    if(ijkB%p.kVl){
        k>>OSSFMT("      vl = min(vl, sq[0]+vl - "<<ijkB%p.kVl<<");\n");
    }
    kz>>"      FOR(n,vl) sq[n] += vl;\n";
    kz>>"}}}}}}";
    kz>>"#undef FOR\n";

    //kf>>"cout<<oss.str()<<endl;\n";        // should go to "../last" to avoid timing it

    return kabc;
}

void BlockingMain::emit_print(cprog::Cblock& abc, cprog::Cblock& untimed){
    abc>>CSTR(#define prtijk(i,j,k) do{oss<<" {"<<i<<","<<j<<","<<k<<"}"<<endl;}while(0)\n);
    abc>>"      prtijk(i,j,k);\n";
    abc>>"#undef prtijk\n";
#if 0 // self-contained
    oss<<CSTR(#define prt(a) do{oss<<#a<<": ";for(int u=0;u<vl;++u)oss<<' '<<a[u]; oss<<endl;}while(0)\n);
#else // if genBlock is already using vecprt, the jit routine can use it too (?)
    abc<<CSTR(#define prt(a) do{oss<<#a<<"["<<setw(3)<<vl<<"]: "<<vecprt(8,2, a, vl)<<endl;}while(0)\n);
#endif
    abc >>"      prt(a);"
        >>"      //prt(ta);"
        >>"      prt(b);"
        >>"      prt(c);";
    abc>>"#undef prt\n";

    abc>>"fooOut += a[0]+b[0]+c[0];";

    untimed>>"cout<<oss.str()<<endl;";
    untimed>>"oss.clear(); oss.str(\"\");";
}

void BlockingMain::add_check(BlockingPlan const& p){
}

std::string BlockingPlan::str() const {
    oss.clear(); oss.str("");
    auto prtOuter=[](int const nloop, int const to, int const by){
        oss <<" ";
        if(nloop==1){
            assert(to==by);
            oss <<"once";
        }else if(by==1){
            assert(nloop==to);
            oss<<to<<"*";
        }else{
            oss <<nloop<<"*To"<<to;
            if(by!=1) oss <<"By"<<by;
        }
    };
    // inner loop stride IS the vector length for that loop
    auto prtInner=[](int const nloop, int const to, int const vl){
        oss <<" "<<nloop<<"*";
        assert(vl>0);
        if(to > 1){
            oss <<"To"<<to;
            if(vl!=1){
                oss<<"v";
                if(vl!=to) oss<<vl;
            }
        }
    };
    prtOuter(iLpo,ii,iB);
    prtOuter(jLpo,jj,jB);
    prtOuter(kLpo,kk,kB);
    auto outer = oss.str();
    oss.str("");
    prtInner(iLp,iB,iv());
    prtInner(jLp,jB,jv());
    prtInner(kLp,kB,kv());
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
std::string BlockingPlan::suffix() const{
    oss.clear(); oss.str("");
    oss<<"_vl"<<kVl;
    auto prt=[](int const to, int const by, int const vl){
        int const nloopOuter = (to+by-1)/by;
        oss<<to;
        if(nloopOuter==1){
            assert(by==to);
        }else{
            if(by!=1) oss<<"By"<<by;
        }
        if(by>1 && vl==by) oss<<"v";
        else if(vl!=1) oss<<"v"<<vl;
    };
    oss<<"_i"; prt(ii,iB,iv());
    oss<<"_j"; prt(jj,jB,jv());
    oss<<"_k"; prt(kk,kB,kv());
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

std::string code(BlockingPlan const& p)
{
    ostringstream oss;
    p.prt(oss);
    int const &ii=p.ii, &jj=p.jj, &kk=p.kk;
    oss<<CSTR(#define prt(a) do{oss<<#a<<": ";for(int u=0;u<vl;++u)oss<<' '<<a[u]; oss<<endl;}while(0)\n);
    oss<<CSTR(#define prtijk(i,j,k) do{oss<<" {"<<i<<","<<j<<","<<k<<"}"<<endl;}while(0)\n);
    oss<<CSTR(#define FOR(I,VL) for(int I=0;I<VL;++I)\n);
    oss<<"int vl="<<p.kVl<<";\n";
    oss<<"typedef int vr["<<p.kVl<<"];\n";
    oss<<"vr a, b, c, sq, tmp, ta, tb, tc;\n";
    oss<<"// outer loops: "<<p.iLpo<<" "<<p.jLpo<<" "<<p.kLpo<<"\n";
    if(p.iLpo>1){
        oss<<"for(int io=0; io<"<<ii<<"; io+="<<p.iB<<"){ //"<<(ii%p.iB==0?" norem":" rem")<<"\n";
    }else{
        oss<<"{int const io=0;\n";
    }
    if(p.jLpo>1){
        oss<<" for(int jo=0; jo<"<<jj<<"; jo+="<<p.jB<<"){ //"<<(jj%p.jB==0?" norem":" rem")<<"\n";
    }else{
        oss<<" {int const jo=0;\n";
    }
    if(p.kLpo>1){
        oss<<"  for(int ko=0; ko<"<<kk<<"; ko+="<<p.kB<<"){ //"<<(kk%p.kB==0?" norem":" rem")<<" "<<p.ijkOuter()<<" outer\n";
    }else{
        oss<<"  {int const ko=0;\n";
    }
    oss<<"   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;\n";
#if 0 // nosimd
    if(p.iB>1){
        oss<<"  for(int i=io; i<io+"<<p.iB<<"; ++i){ // iv="<<p.iv()<<"\n";
        oss<<"   FOR(n,vl) a[n] = ko + n%"<<p.jB*p.kB<<"; // ?\n";
    }else{
        oss<<"   for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
        oss<<"    FOR(n,vl) a[n] = ko;\n";
    }
    if(p.jB>1){
        oss<<"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv()<<"\n";
        oss<<"     FOR(n,vl) b[n] = ji + n%"<<p.kB<<"; // ?\n";
    }else{
        oss<<"   for(int j=jo; j==jo; ++j){ // jv="<<p.jv()<<"\n";
        oss<<"    FOR(n,vl) a[n] = ko;\n";
    }
    oss<<"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
    oss<<"      prtijk(i,j,k);\n";
#else // simd 'c'
    oss<<"   FOR(n,vl) sq[n] = n;\n";
    oss<<"   // inner loops: "<<p.iLp <<" "<<p.jLp <<" "<<p.kLp <<"\n";
    oss<<"   // inner vls  : "<<p.iv()<<" "<<p.jv()<<" "<<p.kv()<<"\n";
    oss<<"   // inner block: "<<p.iB  <<" "<<p.jB  <<" "<<p.kB  <<"\n";
    if(p.iB>1){
        oss<<"   for(int i=io; i<io+"<<p.iB<<"; i+="<<p.iv()<<"){    // A iB="<<p.iB<<" iv="<<p.iv()<<"\n";
        oss<<"    FOR(n,vl) a[n] = ko + sq[n]/"<<p.jB*p.kB<<";    // divmod by jB*kB="<<p.jB<<"*"<<p.kB<<"\n";
        oss<<"    FOR(n,vl) ta[n] = sq[n]%"<<p.jB*p.kB<<";\n";
    }else{
        //oss<<"  for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
        oss<<"   {int const i=io;\n";
        oss<<"    FOR(n,vl) a[n] = i;\n";
    }

    if(p.iB>1 && p.jB>1){
        oss<<"    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // a jB="<<p.jB<<" jv="<<p.jv()<<"\n";
        oss<<"     FOR(n,vl) b[n] = jo + ta[n]/"<<p.jv()<<";\n";
    }else if(p.jB>1){
        oss<<"    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // b jB="<<p.jB<<" jv="<<p.jv()<<"\n";
        oss<<"     FOR(n,vl) b[n] = jo + sq[n]/"<<p.kv()<<";        // kB="<<p.kB<<" kv="<<p.kv()<<"\n";
    }else{
        oss<<"    {int const j=jo;\n";
        oss<<"    FOR(n,vl) b[n] = jo;\n";
    }

    if(p.kB>1){
        oss<<"     for(int k=ko; k<ko+"<<p.kB<<"; k+="<<p.kv()<<"){    // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
        oss<<"      FOR(n,vl) c[n] = ko + sq[n]%"<<p.kv()<<"; // mod by kB="<<p.kB<<" [kv="<<p.kv()<<"] shortcut\n";
    }else{
        oss<<"     {int const k=ko;\n";
        oss<<"      FOR(n,vl) c[n] = ko;\n";
    }

    int const ijkB = p.iB*p.jB*p.kB;
    if(ijkB%p.kVl){
        oss<<"      vl = min(vl, sq[0]+vl - "<<ijkB%p.kVl<<");\n";
    }
    oss<<"      prtijk(i,j,k);\n";
    oss<<"      prt(a); prt(ta); prt(b); prt(c);\n";
    oss<<"      FOR(n,vl) sq[n] += vl;\n";
#endif
    oss<<"}}}}}}"<<endl;
    oss<<"#undef FOR\n";
    oss<<"#undef prtijk\n";
    oss<<"#undef prt\n";
    oss<<"cout<<oss.str()<<endl;\n";        // should go to "../last"

    return oss.str();
}


// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
