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
#include "genBlock-jit.hpp"
#include "../stringutil.hpp"
#include "../fuseloop.hpp"
#include "../cblock.hpp"
#include <limits>
#include <fstream>
using namespace std;
using namespace loop;
using namespace cprog;

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
    //  triple-induction >> double-induction >> single.
    //  power-of-two >> evenly divisible
    //  ...
    //  lower VL "all else equiv"
    double ijkCost() const {return iCost+jCost+kCost;}
    // per-loop simd lengths.   kVl == iv()*jv()*kv()
    int iv() const {return iVl;}
    int jv() const {return jVl/iVl;}
    int kv() const {return kVl/jVl;}
    std::string str() const;
    void prt(std::ostream& os) const { os<<"// "<<str()<<endl; }
    std::string suffix() const; ///< a canonical function-name suffix for plan
    std::string iijjkk() const; ///< "_iNjNkN" string where N is value of ii,jj,kk
};
/* Splitting + Blocking can save even more (sometimes).
 *  Ex. 7x7x904 --> plans like 
 * Splitting + Blocking can save even more (sometimes).
 * Ex. Hmmm. need a better 'cost' hierarchy, to dissuade triple-vectorization inductions
 * time cost ~ 0.2[often 2 muls and some subtracts, EVERY inner induction, unless...]
 *             * 1.0 [time measured in inner kernel time units]
 *             * kerns(). */

template<typename T>
std::vector<std::vector<Triples<T>>> refTriples( BlockingPlan const& p );

template<typename T>
std::vector<std::vector<Triples<T>>> refTriples( BlockingPlan const& p )
{
    return refTriples<T>(p.ii, p.jj, p.kk, p.iB, p.jB, p.kB, p.iv(), p.jv(), p.kv());
}

void prtCode(BlockingPlan const& p);

/** inject timing utilities, globals, init and teardown code [just once] */
void timeit_prep(Cblock& utils, Cblock& main);

/** inject a `timeit_foo` function for your \c foo.
 * \return inner Cblock in `timeit_`foo where your kernel goes.
 * - timeit_foo goes to a subblock fns[foo]
 * - invoke/print ------> subblock call[foo]
 */
Cblock& timeit_foo(std::string const& foo, Cblock& fns, Cblock& call, int const reps);

/** make triple-loop blocking plans and analyze their cost.
 * \return sorted list of BlockingPlan, best first. */
std::vector<BlockingPlan> mkBlockingPlans(int const ii, int const jj, int const kk);

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
/** Multiple loop-blocking timing tests.
 *
 * We coordinate some activities:
 * - Constructor sets fairly empty paths:
 *   - pr["first","features","includes","fns","run","run/first"]
 * - add(krn)
 *   - timeit_prep sets up timing utilities (done elsewhere)
 * - emit_FOO
 *   - timeit_foo outputs a generic timing loop \em subroutine
 *   - emit_loops emits [mostly] into the timing loop itself,
 *   - with a customizable inner kernel, \em FOO, to use the vector results
 *     of the blocking loops (e.g. -kPRINT, -kCHECK, -kHASH)
 */
struct BlockingMain final : public BlockingBase {
    BlockingMain(int const krn, int const v=0/*verbose*/);

    /** adds function and a call to it from 'main'.
     * Usually this is the only function you'll want to use. */
    void add(BlockingPlan const& p);

    /** emit a[],b[],c[] calculation loops.
     * \return \b innermost block, \c abc, which should do something with a,b,c.
     * - blockingScope = \c abc["..*\/blockingScope"] will have subpaths like:
     *   - "decl", "precalc", "outerloops",
     *   - "**\/ko" (last outer loop)
     *   - "**\/loop_k" (last inner loop)
     *
     * public only to inspect the 'raw' result for debug.
     */
    // some kernels might need "precalc" outer_ and inner_ blocks
    //this->krn_ = mkBlockingTestKernel(krn,*outer_,*inner_ /*, defaults?*/ );
    // perhaps return a tuple of Cblock, when that is needed.
    static cprog::Cblock& emit_loops(BlockingPlan const& p, Cblock& krnBlk);

  private:
    void add_none(BlockingPlan const& p);
    void add_print(BlockingPlan const& p);
    void add_check(BlockingPlan const& p);
    void add_hash(BlockingPlan const& p);

    /** Fill in innermost a,b,c block according to \c krn.
     * \c after should point to after the timing loop (not part of reported time).
     * Ex. after=krnBlk["..* /tloop/"]
     */
    void emit_print(cprog::Cblock& abc, cprog::Cblock& after);
    void emit_check(cprog::Cblock& allocate,
            cprog::Cblock& clear,
            cprog::Cblock& abc,
            cprog::Cblock& check,
            std::string refVar);
};

void prtCode(BlockingPlan const& p){
    Cunit pr("dummy","C",0);
    BlockingMain::emit_loops(p, pr.root);
    cout<<pr.str()<<endl;
}

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
    int which = KRNBLK3_NONE;
    if(argc > 1){
        if (argv[1][0]=='-'){
            char *c = &argv[1][1];
            if (*c=='h'){
                cout<<" genBlock [-h] [-kKERN] II JJ KK"<<endl;
                cout<<"   MVL = max vector length = 256 (fixed)"<<endl;
                cout<<"   II    = 1st loop a=0..II-1"<<endl;
                cout<<"   JJ    = 2nd loop b=0..JJ-1"<<endl;
                cout<<"   KK    = 3rd loop c=0..KK-1"<<endl;
                cout<<" -kKERN  Choose KERN from [NONE],PRINT,CHECK,HASH"<<endl;
                cout<<" triple loop --> loop over vector registers a[VLEN], b[VLEN] c[VLEN]"<<endl;
                opt_h = 1;
            }else if(*c=='k'){
                std::string kern=string(++c);
                if(kern=="NONE") which=KRNBLK3_NONE;
                else if(kern=="HASH") which=KRNBLK3_HASH;
                else if(kern=="PRINT") which=KRNBLK3_PRINT;
                else if(kern=="CHECK") which=KRNBLK3_CHECK;
                else{
                    cout<<"-kKERN "<<c<<" not supported (stays at "<<krnblk3_name(which)<<")"<<endl;
                }
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
    cout<<bps.size()<<" sorted BlockingPlans for ii="<<ii<<" jj="<<jj<<" kk="<<kk<<endl;
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
    BlockingMain bm(which, 1/*verbose*/);
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
void timeit_prep(Cblock& includes, Cblock& utils, Cblock& main)
{
    if(tag_once(utils,"timeit")){
        set_once(includes,"timer.h", "#include \"../timer.h\"");
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
        set_once(includes,"iomanip", "#include <iomanip>");
        set_once(includes,"sstream", "#include <sstream>");
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
        set_once(includes["last"],"std","using namespace std;");
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
        >>"uint64_t t=0; // sum of reps execution times"
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
    run.setType("FUNCTION");
    run_ = &run;
    run["first"]; // reserve a node
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
cprog::Cblock& BlockingMain::emit_loops(BlockingPlan const& p, Cblock& krnBlk){
#if 0 // TODO
    struct Opcount {
        uint32_t div, mul ,add ,shift;
        /** add cost for divide-by-known-\c n. */
        jitdiv(uint32_t n); // jit division resolves to some simple combination of basic ops
        jitdivmod(uint32_t n); // often need both divide-by-n and modulo-n results
    };
#endif
    auto& pr = krnBlk.getRoot();
    // all of krnBlk points into some "tloop/body" of timeit_foo
    //auto& kf = krnBlk["../last"];
    CBLOCK_SCOPE(blockingScope,"",pr,krnBlk);
    // k["first"] ?
    int const &ii=p.ii, &jj=p.jj, &kk=p.kk;

    string descr = p.str();
    blockingScope
        >>OSSFMT("// "<<descr)
        >>CSTR(#define FOR(I,VL) for(int I=0;I<VL;++I))
        ;
    auto& decl = blockingScope["decl"]
        >>OSSFMT("typedef int vr["<<p.kVl<<"];")
        >>"vr a, b, c;"
        ;
    decl["vl"]
        >>OSSFMT("int const vl="<<p.kVl<<";")
        // reset to nonconst if we ever have 'last loop' issues
        ;
    auto& precalc = blockingScope["precalc"]
        >>"// [opt] precalc"
        ;
    auto use_sqij = [&decl,&precalc](){
        if(tag_once(decl,"sqij")){
            decl["sqij"]>>"vr sqij;";
            precalc>>"FOR(n,vl) sqij[n] = n;";
        }
    };
    auto use_cnt = [&decl](){
        if(tag_once(decl,"cnt")){
            decl["cnt"]>>"uint64_t cnt=0U;";
            //precalc>>"uint64_t cnt=0U;";
        }
    };
    auto use_ta = [&decl](){if(tag_once(decl,"ta")){ decl["ta"]>>"vr ta;";}};
    //auto use_tb = [&decl](){if(tag_once(decl,"tb")){ decl["tb"]>>"vr tb;";}};

    // outer loops
    blockingScope["outerloops"]
        >>OSSFMT("// outer loop blocking factors: "<<p.iLpo<<" "<<p.jLpo<<" "<<p.kLpo);
    auto outerComment = [](Cblock& cb, int iLpo, int ii, int iB){
        char const* cmt = (iLpo<=1? " once, unblocked"
                : ii%iB==0? " norem": " rem");
        auto const sz=cb.code_str().size();
        if(sz<40) cb<<string(40-sz,' ');
        cb<<cmt;
    };
    auto ioFor = (p.iLpo>1? OSSFMT("for(int io=0; io<"<<ii<<"; io+="<<p.iB<<")")
            : "int const io=0;");
    CBLOCK_SCOPE(io,ioFor,pr,blockingScope);
    outerComment(io["../beg"], p.iLpo, ii, p.iB);

    auto joFor = (p.jLpo>1? OSSFMT("for(int jo=0; jo<"<<jj<<"; jo+="<<p.jB<<")")
            : "int const jo=0;");
    CBLOCK_SCOPE(jo,joFor,pr,io);
    outerComment(jo["../beg"], p.jLpo, jj, p.jB);

    auto koFor = (p.kLpo>1? OSSFMT("for(int ko=0; ko<"<<kk<<"; ko+="<<p.kB<<")")
            : "int const ko=0;");
    CBLOCK_SCOPE(ko,koFor,pr,jo);
    outerComment(ko["../beg"], p.kLpo, kk, p.kB);

#if 0 // nosimd
    if(p.iB>1){
        k>>"  for(int i=io; i<io+"<<p.iB<<"; ++i){ // iv="<<p.iv();
        k>>"   FOR(n,vl) a[n] = ko + n%"<<p.jB*p.kB<<"; // ?";
    }else{
        k>>"   for(int i=io; i==io; ++i){ // iv="<<p.iv();
        k>>"    FOR(n,vl) a[n] = ko;";
    }
    if(p.jB>1){
        k>>"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv();
        k>>"     FOR(n,vl) b[n] = ji + n%"<<p.kB<<"; // ?";
    }else{
        k>>"   for(int j=jo; j==jo; ++j){ // jv="<<p.jv();
        k>>"    FOR(n,vl) a[n] = ko;";
    }
    k>>"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl;
    k>>"      prtijk(i,j,k);";
#endif
    ko
        >>OSSFMT("// outer limits: "<<p.ii <<" "<<p.jj <<" "<<p.kk )
        >>OSSFMT("// blocking    : "<<p.iB  <<" "<<p.jB  <<" "<<p.kB  )
        >>OSSFMT("// inner vls   : "<<p.iv()<<" "<<p.jv()<<" "<<p.kv()<<"  vl="<<p.kVl)
        >>OSSFMT("// inner loops : "<<p.iLp <<" "<<p.jLp <<" "<<p.kLp )
        ;
    bool const need_ihi = ii%p.iB!=0;
    bool const need_jhi = jj%p.jB!=0;
    bool const need_khi = kk%p.kB!=0;
    if(need_ihi){
        ko>>OSSFMT("int const ihi = (io+"<<p.iB<<"<"<<p.ii<<"? io+"<<p.iB<<": "<<ii<<");");
    }
    if(need_jhi) ko
        >>OSSFMT("int const jhi = (jo+"<<p.jB<<"<"<<p.jj<<"? jo+"<<p.jB<<": "<<jj<<");");
    if(need_khi) ko
        >>OSSFMT("int const khi = (ko+"<<p.kB<<"<"<<p.kk<<"? ko+"<<p.kB<<": "<<kk<<");");
    if(need_ihi && !need_jhi && !need_khi){
        precalc
            >>OSSFMT("int const iLast = "<<ii/p.iv()*p.iv()<<";")
            >>OSSFMT("int const ivLast = "<<p.kVl * (ii%p.iB) / p.iB<<";");
    }
    //int const ilim=min(io+iB,ii);
    //int const jlim=min(jo+jB,jj);
    //int const klim=min(ko+kB,kk);

    // inner loops
    string ilim=(need_ihi? string("ihi"): OSSFMT("io+"<<p.iB));
    auto iFor = (p.iB<=1? string("int const i=io;")
            : OSSFMT("for(int i=io; i<"<<ilim<<"; i+="<<p.iv()<<")"));
    CBLOCK_SCOPE(loop_i,iFor,pr,ko);
#define DEF_ADIV(FORMULA) \
    unsigned adiv = (unsigned)(FORMULA); \
    string sdiv = #FORMULA
    DEF_ADIV(p.kVl / p.iv());
    if(p.iB<=1){
        loop_i>>"FOR(n,vl) a[n] = i;";
    }else{
        loop_i["../beg"]<</*            */ OSSFMT(" A iB="<<p.iB<<" iv="<<p.iv());
        use_sqij();
        //INSCMT(loop_i, OSSFMT("int adiv = "<<pvl*p.jLp*p.kLp),
        //INSCMT(loop_i, OSSFMT("int adiv = "<<p.jv()*p.kv()),
        //        OSSFMT("divmod by p.vl*jLp*kLp="<<p.jv()*p.kv()<<"*"<<p.jLp<<"*"<<p.kLp));
        loop_i>>OSSFMT("unsigned const adiv = "<<adiv<<"; // "<<sdiv),
        loop_i>>OSSFMT("FOR(n,vl) a[n] = io + sqij[n]/adiv;");
        //INSCMT(loop_i, OSSFMT("FOR(n,vl) a[n] = io + sqij[n]/"<<p.kVl<<";"), "div by VL");
        // not always used:
        //loop_i>>OSSFMT("FOR(n,vl) ta[n] = sqij[n]%"<<p.jB*p.kB<<";");
    }

    string jlim=(need_jhi? string("jhi"): OSSFMT("jo+"<<p.jB));
    auto jFor = (p.iB<=1? string("int const j=jo;")
            : OSSFMT("for(int j=jo; j<"<<jlim<<"; j+="<<p.jv()<<")"));
    CBLOCK_SCOPE(loop_j,jFor,pr,loop_i);
    loop_j["../beg"]<</*                */ OSSFMT("jB="<<p.jB<<" jv="<<p.jv());
    if(p.iB>1 && p.jB>1){
        use_sqij();
        use_ta();
        loop_i>>OSSFMT("FOR(n,vl) ta[n] = sqij[n]%"<<adiv<<";");
        loop_j>>OSSFMT("FOR(n,vl) b[n] = jo + ta[n]/"<<adiv<<";");
    }else if(p.jB>1){
        use_sqij();
        loop_j>>OSSFMT("FOR(n,vl) b[n] = jo + sqij[n]/"<<p.kv()<<";");
    }else{
        loop_j>>"FOR(n,vl) b[n] = jo;";
    }

    string klim=(need_khi? string("khi"): OSSFMT("ko+"<<p.kB));
    auto kFor = (p.kB<=1? string("int const k=ko;")
            : OSSFMT("for(int k=ko; k<"<<klim<<"; k+="<<p.kv()<<")"));
    CBLOCK_SCOPE(loop_k,kFor,pr,loop_j);
    if(p.kB>1){
        use_sqij();
#if 0
        loop_k>>OSSFMT("FOR(n,vl) c[n] = ko + sqij[n]%"<<p.kv()<<";");
#elif 1
        use_ta();
        decl>>"vr tb, td;";
    //loop_k>>CSTR(#define prt(a) do{oss<<#a<<"["<<setw(3)<<vl<<"]: "<<vecprt(8,2, a, vl)<<endl;}while(0)\n);
        loop_k>>OSSFMT("FOR(n,vl) ta[n] = sqij[n]/"<<p.kv()<<"; // divmod p.kv="<<p.kv());
        loop_k>>OSSFMT("FOR(n,vl) tb[n] = sqij[n]%"<<p.kv()<<";");
        loop_k>>OSSFMT("FOR(n,vl) c[n] = ko + tb[n];");
    //loop_k>>"oss<<\"cccccccc\\n\"; prt(ta); prt(tb); prt(c);";
        loop_k>>OSSFMT("FOR(n,vl) td[n] = ta[n]%"<<p.jv()<<";");
        loop_k>>OSSFMT("FOR(n,vl) b[n] = jo + td[n];");
    //loop_k>>"oss<<\"bbbbbbbb\\n\"; prt(ta); prt(tb); prt(b);";
        if(p.iB>1){
            decl>>"vr tc;";
            loop_k>>OSSFMT("FOR(n,vl) tc[n] = ta[n]/"<<p.jv()<<"; // divmod p.jv="<<p.jv());
            //loop_k>>OSSFMT("FOR(n,vl) ta[n] = ta[n]/"<<p.iv()<<"; // divmod p.iv="<<p.iv());
            //loop_k>>OSSFMT("FOR(n,vl) tb[n] = ta[n]%"<<p.iv()<<";");
            loop_k>>OSSFMT("FOR(n,vl) a[n] = io + tc[n]%"<<p.iv()<<"; // mod p.iv="<<p.iv());
            //loop_k>>"oss<<\"aaaaaaaa\\n\"; prt(ta); prt(tb); prt(a);";
            //loop_k>>OSSFMT("oss <<\" (ihi-io) * (jhi-jo) * (khi-ko) = \""
            //    <<(ihi-io) * (jhi-jo) * (khi-ko)<<"\"\\n\";");
        }
    //loop_k >>"#undef prt";
#else
#endif
    }else{
        loop_k>>"FOR(n,vl) c[n] = ko;";
    }

    // pre core kernel
    bool const vlNonConst = (p.ii%p.iB) || (p.jj%p.jB) || (p.kk%p.kB);
    //int const ijkB = p.iB*p.jB*p.kB;
    if(vlNonConst){
        decl["vl"].set(OSSFMT("int vl="<<p.kVl<<";"));  // default 'vl' was 'const'
        decl["vl"]<<OSSFMT(" // ii%iB="<<p.ii%p.iB<<" jj%jB="<<p.jj%p.jB<<" kk%kB="<<p.kk%p.kB);
        ko["last"]>>OSSFMT("vl="<<p.kVl<<";");          // re-init 'vl' beforeInnerLoops
        use_sqij();
        use_cnt(); // vector extract 0'th element is slow on Aurora.  Maintain 'cnt'
        // XXX check!
        if(need_ihi && !need_jhi && !need_khi){
            // ok: loop_k>>OSSFMT("vl = min(vl, (ihi-io)*"<<p.kVl<<"/"<<p.iB<<");");
            loop_k>>"if(i==iLast) vl=ivLast;";
        }
    }else{
        decl["vl"]<<" // const!";
    }
    // core kernel
    auto& kabc = loop_k["abc"];         // <-- our return value (innermost loop)
    // post core kernel
    auto& kz = loop_k["last"]["last"];
    if(decl.find("sqij")){                // have_sqij?
        kz>>"FOR(n,vl) sqij[n] += vl;";
    }
    if(decl.find("cnt")){
        kz>>"cnt += vl;";
    }
    kz>>"#undef FOR";

    //kf>>"cout<<oss.str()<<endl;";       // should go to "../last" to avoid timing it

    return kabc;
}

void BlockingMain::add_none(BlockingPlan const& p){
    timeit_prep(pr["includes"],fns(),run()); // inject timing function helper code

    auto subroutine = pr.name+p.suffix();
    auto& krnBlk = timeit_foo(subroutine, fns(), run(), 1000);
    //cout<<"timeit_foo --> krnBlock @ "<<krnBlk.fullpath()<<endl;

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
    krnBlk[".."]["last"]>>"bogus ^= fooOut; // somehow update bogus using fooOut [,seed,...]";
}

void BlockingMain::add_print(BlockingPlan const& p){
    timeit_prep(pr["includes"],fns(),run()); // inject timing function helper code
    set_once(pr["includes"],"iostream", "#include <iostream>");

    auto subroutine = pr.name+p.suffix();
    auto& krnBlk = timeit_foo(subroutine, fns(), run(), 1);

    string descr = p.str();
    string suffix = p.suffix();
    krnBlk["..*/tloop/.."] // before timing loop
        >>OSSFMT("cout<<\"BlockingPlan"<<suffix<<"\\n "<<descr<<"\"<<endl;")
        ;
    krnBlk>>"uint64_t fooOut = seed;";

    auto& abc = emit_loops(p, krnBlk);                  // kernel code for a[],b[],c[] 
    //auto& untimed = abc["..*/tloop"]["last"];         // after timing loop
    auto& untimed = abc["..*/tloop/body"]["last"];      // at end of timing loop
    emit_print(abc, untimed);

    // this is still in timing loop:
    krnBlk[".."]["last"]>>"bogus ^= fooOut; // somehow update bogus using fooOut [,seed,...]";
}

void BlockingMain::emit_print(cprog::Cblock& abc, cprog::Cblock& untimed){
    abc>>CSTR(#define prtijk(i,j,k) do{oss<<" {"<<i<<","<<j<<","<<k<<"}";}while(0)\n);
    abc>>"oss<<\"  outer \"; prtijk(io,jo,ko);";
    abc>>"oss<<\"  inner \"; prtijk(i ,j ,k ); oss<<\"\\n\";";
    abc>>"#undef prtijk";
#if 0 // self-contained
    abc>>CSTR(#define prt(a) do{oss<<#a<<": ";for(int u=0;u<vl;++u)oss<<' '<<a[u]; oss<<endl;}while(0)\n);
#else // if genBlock is already using vecprt, the jit routine can use it too (?)
    abc>>CSTR(#define prt(a) do{oss<<#a<<"["<<setw(3)<<vl<<"]: "<<vecprt(8,2, a, vl)<<endl;}while(0)\n);
#endif
    abc
        //>>"prt(sqij); prt(ta);"
        >>"prt(a); prt(b); prt(c);"
        >>"#undef prt";

    abc >>"uint64_t const elem = seed%vl; // try to hinder optimization"
        >>"fooOut += a[elem]+b[elem]+c[elem];";

    INSCMT(untimed,"cout<<oss.str()<<endl;","untimed");
    untimed>>"oss.clear(); oss.str(\"\");";
}

#if 0
std::vector<std::vector<Triples<uint64_t>>> gold_u64(
        uint64_t const ii, uint64_t const jj, uint64_t const kk,  // original loop limits
        uint64_t const iB, uint64_t const jB, uint64_t const kB,  // blocking factors
        uint64_t const iv, uint64_t const jv, uint64_t const kv)  // vectorization lengths
{
    return refTriples<uint64_t>(ii,jj,kk, iB,jB,kB, iv,jv,kv);
}
#endif

/** define a static const \em variable with reference output.
 * Type will be `static const vector<vector<Triples<uint64_t>>>`.
 * \c refVar could be \c OSSFMT("ref"<<p.ijk()) for some BlockingPlan p.
 * Initialization just calls a template function from a header file.
 */
static inline void ref_prep( Cblock& includes, Cblock& statics,
        BlockingPlan const &p, std::string refVar ){
    set_once(includes,"genBlock-jit", "#include \"genBlock-jit.hpp\"");
    if(!statics.find(refVar)){
        statics[refVar]
            >>OSSFMT("auto const "<<refVar<<" = refTriples(")
            >>OSSFMT("        /*ii,jj,kk loop limits   */ "<<p.ii<<", "<<p.jj<<", "<<p.kk<<",")
            >>OSSFMT("        /*iB,jB,kB blocking      */ "<<p.iB<<", "<<p.jB<<", "<<p.kB<<",")
            >>OSSFMT("        /*iv,jv,kv vectorization */ "<<p.iv()<<", "<<p.jv()<<", "<<p.kv()<<");") ;
    }
}

void BlockingMain::add_check(BlockingPlan const& p){
    timeit_prep(pr["includes"],fns(),run());  // inject timing function helper code
    string refVar = OSSFMT("ref"+p.suffix());
    ref_prep(pr["includes"], fns(), p, refVar); // inject ref calc variable (1 per p.suffix())
    set_once(pr["includes"],"iostream", "#include <iostream>");

    auto subroutine = pr.name+p.suffix();
    auto& krnBlk = timeit_foo(subroutine, fns(), run(), 1);

    string descr = p.str();
    string suffix = p.suffix();
    krnBlk["..*/tloop/.."] // before timing loop
        >>OSSFMT("cout<<\"BlockingPlan"<<suffix<<"\\n "<<descr<<"\"<<endl;")
        ;
    krnBlk>>"uint64_t fooOut = seed;";

    auto& abc = emit_loops(p, krnBlk);                  // kernel code for a[],b[],c[] 

    // at beginning of test routine, generate RefTriples
#if 0
    auto& fn_first = abc["..*/fn_foo/first"];
    fn_first>>OSSFMT("auto gold = gold_u64("<<p.ii<<","<<p.jj<<","<<p.kk<<", "
            <<p.iB<<","<<p.jB<<","<<p.kB<<", "
            <<p.iv()<<","<<p.jv()<<","<<p.kv()<<");");
#endif
    // TODO we need something available in a header, or in libjit (not some function in this file)
    // TODO separate header with refBlk3.hpp, triple-loop-blocking reference template functions,
    // so that JIT 'check' routine can run the reference calc, once, before running the set of
    // contained jit impls.   **also** support multi-function case of 1 BlockingPlan but
    // multiple implementations (still all in one JIT file).
    // i.e. 'refrun_BlockingPlanDescriptionSuffix' should run just once, and its results be
    // created/used from ANY impl that matches the 'BlockingPlanDescriptionSuffix'

    auto& blockingScope = abc["..*/blockingScope"];
    auto& tloop = blockingScope["..*/tloop"];
    auto& allocate = tloop["..*/body/first"]; // up to function, first code in function
    auto& clear = blockingScope["body/precalc"];
    auto& check = blockingScope["body"]["last"]; // check every repetition
    emit_check(allocate, clear, abc, check, refVar);

    // this is still in timing loop:
    krnBlk[".."]["last"]>>"bogus ^= fooOut; // somehow update bogus using fooOut [,seed,...]";
}

/** Record the vectors in \c abc, and in \c check block, compare to RefTriples. */
void BlockingMain::emit_check(cprog::Cblock& allocate,
        cprog::Cblock& clear,
        cprog::Cblock& abc,
        cprog::Cblock& check,
        std::string refVar)
{
    abc >>"uint64_t const elem = seed%vl; // try to hinder optimization"
        >>"fooOut += a[elem]+b[elem]+c[elem];";
    if(1){ // also print?
        abc>>CSTR(#define prtijk(i,j,k) do{oss<<" {"<<i<<","<<j<<","<<k<<"}"<<endl;}while(0)\n);
        abc>>"prtijk(i,j,k);";
        abc>>"#undef prtijk";
        abc>>CSTR(#define prt(a) do{oss<<#a<<"["<<setw(3)<<vl<<"]: "<<vecprt(8,2, a, vl)<<endl;}while(0)\n);
        abc >>"prt(a);"
            >>"//prt(ta);"
            >>"prt(b);"
            >>"prt(c);";
        abc >>"#undef prt";
        INSCMT(check,"cout<<oss.str()<<endl;","untimed");
        check>>"oss.clear(); oss.str(\"\");";
    }
    if(1){ // also check?
        allocate
            >>OSSFMT("size_t const nRefTriples = nTriples("+refVar+");")
            >>"vector<Triples<int>> triples;"
            >>"triples.reserve(nRefTriples);"
            >>"vector<size_t> vls;"
            >>OSSFMT("vls.reserve("<<refVar<<".size());")
            >>"size_t nerr=0U;"
            ;
        // additional debug: pretty-print the reference output
        allocate
            >>CSTR(#define prt(s,i,a) do{oss<<s<<"_"<<i<<"["<<setw(3)<<a.size()<<"]: "<<vecprt(8,2, a.data(), (int)a.size())<<endl;}while(0)\n)
            >>OSSFMT("for(unsigned v=0U; v<"<<refVar<<".size(); ++v){")
            >>"oss<<endl;"
            >>OSSFMT("    prt(\"ref a\",v,ivec("<<refVar<<",v));")
            >>OSSFMT("    prt(\"ref b\",v,jvec("<<refVar<<",v));")
            >>OSSFMT("    prt(\"ref c\",v,kvec("<<refVar<<",v));")
            >>"}"
            >>"#undef prt"
            ;
        clear
            >>"triples.clear();"
            >>"vls.clear();"
            ;
        abc // update
            >>"vls.push_back(vl);"
            >>"FOR(n,vl) triples.push_back({ a[n],b[n],c[n] });"
            ;
        check
            >>OSSFMT("nerr = check(triples,vls, "<<refVar<<" );")
            >>"if(nerr) break;"
            ;
    }
}


void BlockingMain::add_hash(BlockingPlan const& p){
}

/** string for the "problem" part of the BlockingPlan */
std::string BlockingPlan::iijjkk() const{
    return OSSFMT("_i"<<ii<<"j"<<jj<<"k"<<kk);
}
/** tiny string for "problem" and minimal "solution". */
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
/** lengthy descriptive string about "problem" and "solution" of BlockingPlan. */
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
                            //cout<<" good="<<good;
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

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
