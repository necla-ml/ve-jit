/** \file
 * demo creating JIT jit library of NumberGuesser functions
 */
#include "cblock.hpp"
#include "dllbuild.hpp"
#include "jitpipe.hpp"
#include <unistd.h>  // getopt
#include <string>
#include <sstream>
#include <fstream>

using namespace std;
using namespace cprog;

static ostringstream oss;

struct param { int64_t i; };
static std::string paramString(struct param const* const p){
    assert(p!=nullptr);
    return OSSFMT("i"<<p->i);
}
int readParamFile(struct param **ppParams, const char *pParamPath ){
    cout<<" ignoring pParamPath "<<pParamPath<<endl;
    uint32_t nParams = 3U;
    struct param *pParams = nullptr;
    pParams = (struct param *) malloc(sizeof(struct param) * nParams) ;
    assert(pParams);
    for(uint32_t i=0U; i<nParams; ++i){
        // we will generate 3 jit routines, each with specific parameters:
        pParams[i].i = 111*i;  // fake parameters
    }
    assert(ppParams);
    *ppParams = pParams ;
    return nParams ;
}
inline int mkConsistent( struct param* p ){
    assert(p);
    int bad=0;
    if( p->i <= 0 ){
        ++bad;
        p->i = 13; // try to fix parameter sets that I can't handle
    }
    return bad;
}
typedef int demoError_t;

/** based on a very short (slow) direct_default3.c */
DllFile numberGuesserSrc( struct param const* const p )
{
    DllFile df; // return value
    std::string parmstr;
    parmstr = paramString(p);
    df.basename = "numberGuess_"+parmstr;

    // we use intrinsics.  suffix matches build recipe in "bin.mk"
    //df.suffix = "-vi.c";     // C VE source, with VE intrinsics
    df.suffix = "-x86.cpp";  // C++ x86 source

    std::string fn_declare = "demoError_t "+df.basename+"()";
    df.syms.push_back(SymbolDecl(df.basename,
                "vednn ConvolutionForward "+paramString(p),
                fn_declare));


    Cunit pr("program");
    auto& includes = pr["includes"]<<Endl;
    auto& macros = pr["macros"];
    includes // oops, x86 target! >>CSTR(#include "veintrin.h")
        >>"#include <iostream>"
        >>"#include <cassert>"
        >>"#include <cstdint>"
        ;
    macros
        >>"#define VLEN (256)"
        >>"using namespace std;"
        >>"typedef int demoError_t;"
        ;
    //auto & fns = mk_extern_c(pr,"extern_C").after(pr["/macros"])["body"];
    auto & fns = mk_extern_c(pr,"extern_C")["body"];
    //auto & fns = mk_extern_c(pr,"extern_C")["body/.."];

    auto& fn = mk_func(pr,"fn",fn_declare).after(fns)["body"];

    // get the vars here first.
    //const int64_t batch          = p->batchNum; // etc.
    const int64_t local_jit_const = 7;
    const int64_t example_parm = p->i; // can use a #define instead.
    string hashvar = OSSFMT("hash_"<<example_parm);

#if 0 // very old way (no undef)
    // then emit them as constant cjit values (or #define them)
    // for C++, T const might be better (T const "as-good-as-macro", and properly typed)
    //#define CONST1(var) <<("\nint64_t const " #var " = "+asDec(var))
    // but for 'C', #define may hold less surprises
#define CONST1(var) >>("#define " #var " "+asDec(var))
    //auto& fn_const =
    fn["const"]
        CONST1(local_jit_const)
        ;
#else // modern way
#define DEF(VAR) def(#VAR,VAR)
    fn["defines"].DEF(local_jit_const).def("PARM",example_parm);
#endif

    fn["init"]
        >>CSTR(char const * const intro = " Hello, I think your favorite number is ";)
        >>OSSFMT("uint32_t "<<hashvar<<" = 0;")
        ;
    CBLOCK_SCOPE(loop_n,"for(int64_t n=0; n<PARM; ++n)",pr,fn);
    loop_n
        >>OSSFMT(hashvar<<" = "<<hashvar<<" * 23456789U + local_jit_const;")
        ;
    fn["modulo"]
        >>OSSFMT(hashvar<<" %= 1000U;")
        ;
    fn["output"]
        >>OSSFMT("cout<<intro<<"<<hashvar<<"<<\".  That took me \"")
        // JIT eliminate of run-time branch (lol)
        >>"    <</*JIT optimized decision here*/ "<<(
                example_parm<100? CSTR("almost no")
                :example_parm<200? CSTR("a little")
                :example_parm<300? CSTR("a lot of")
                :CSTR("a huge amount")
                )
        >>"    <<\" thought\"<<endl;"
        ;
    fn["exit"]>>"return 0/*VEDNN_SUCCESS*/;"
        ;

    pr["end-of-file"]>>"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr.v = 0;
    // Note: 'write' currently has side-effect of "emptying" the tree. Subject to change!
    //cout<<string(80,'-')<<endl;
    //pr.write(cout);
    //cout<<string(80,'-')<<endl;
    //pr.dump(cout);
    //cout<<endl;
    cout<<string(80,'-')<<pr.str() <<string(80,'-')<<endl;
    cout<<string(80,'-')<<pr.tree()<<string(80,'-')<<endl;
    df.code = pr.str();
    return df;
}

char const* default_parameter_file="mb8g1_ic3ih27iw270_oc16oh14ow135_kh3kw3_ph1pw1_sw2sh2_dw1dh1";
static void help(){
    printf( "\ncjitDemo [args]"
            "\n   -p PATH   convolution parameter file"
            "\n             -- or --  a STRING like"
            "\n   -p mb27g1_ic3ih22iw22_oc100oh22ow22_kh5kw5_ph3pw3_sw1sh1_dw1dh1"
            "\n             where '_' are ignored."
            "\n   [ -p %s ]"
            "\n optional:"
            "\n", default_parameter_file);
}
int main(int argc,char**argv){
#ifdef VEDNNX_DIR
    cout<<" VEDNNX_DIR = \""<<CSTR(VEDNNX_DIR)<<"\""<<endl;
#endif
    //vednnConvolutionParam_t pParamConv = {0,0};
    //struct param p = {8,1, 3,32,32, 3,32,32, 3,3, 1,1, 1,1, 1,1, "cnvname" };
    extern int optind;
    extern char *optarg;
    int opt;
    char const * pParamPath = NULL ;

#define PARAMBUFSZ 80
    char paramBuf[PARAMBUFSZ+1];
    while ((opt = getopt(argc, argv, "p:P:")) != -1) {
        switch (opt) {
        case 'P':
            pParamPath = optarg;
            break;
        case 'p':
            snprintf(paramBuf,PARAMBUFSZ,"%s",optarg);
            pParamPath = &paramBuf[0];
            break;
        default: /* '?' */
                     fprintf(stderr, "Unknown option.\n");
                     help();
                     exit(1);
        }
    }
    if (optind < argc) {
        fprintf(stderr, "Unexpected argument after options\n");
        exit(1);
    }
    if ( pParamPath == NULL ) {
        //pParamPath = "./params/conv/alexnet.txt";
        pParamPath = default_parameter_file;
        fprintf(stderr, "Parameter file, '-p' option, defaults to %s.\n", pParamPath);
    }

    printf("PARAMETER FILE           = %s\n",      pParamPath);
    printf(" setting params...\n"); fflush(stdout);
    struct param *pParams ;
    int nParams = readParamFile( &pParams, pParamPath );
#if 0
    if( nParams <= 0 ) {
        printf(" Trying as a parameter string...\n"); fflush(stdout);
        nParams = readParamString( &pParams, pParamPath );
        printf(" readParamstring --> %d\n",nParams); fflush(stdout);
        if( nParams <= 0 ) {
            printf("Bad params from %s\n", pParamPath); fflush(stdout);
            exit(1);
        }
    }
#endif
    printf(" got %d sets of parameters\n", nParams); fflush(stdout);

    for(int i=0; i<nParams; ++i){
        // Convolution tests don't handle some illegal cases well
        //    (segfault or bus error)
        // Some acceptable configs report bad DIFF and also get massaged
        // to "exact-sized" output height and width.
        //
        // This allows you to use randomly-edited parameter files and
        // at least avoid segfaults/bus errors/GEMM illegal value messages.
        // (exact output height width can be tricky to guess correctly).
        //
        printf("mkConsistent..%d\n",i);
        mkConsistent( &pParams[i] );
    }

    int nerr = 0;
    DllBuild dllbuild;
    for(int i=0; i<nParams; ++i){
        struct param const& pConv = pParams[i];
        DllFile df = numberGuesserSrc(&pConv);
        dllbuild.push_back(df);
    }
#if defined(__ve)
#define TMP_CJIT_LIB_DIR "tmp_cjitDemo"
#else
#define TMP_CJIT_LIB_DIR "tmp_cjitDemo-x86"
#endif
    dllbuild.prep("cjitDemo",TMP_CJIT_LIB_DIR);

    std::string mkEnv;
    {
        std::ostringstream oss;
        oss<<"CFLAGS=-I'" CSTR(VEDNNX_DIR) "'"
             " CXXFLAGS=-I'" CSTR(VEDNNX_DIR) "'"
             " LDFLAGS='-L" CSTR(VEDNNX_DIR) "/lib -Wl,-rpath," CSTR(VEDNNX_DIR) "/lib'"
             ;
        mkEnv=oss.str();
    }
    cout<<" mkEnv:\n\t"<<mkEnv<<endl;
    dllbuild.make();
    if(1){
        cout<<"\nDllBuild expects symbols...\n";
        cout<<"Library: "<<dllbuild.getLibName()<<endl;
        for(auto const& df: dllbuild){
            cout<<"   File: "<<df.basename<<df.suffix<<" :"<<endl;
            for(auto const& sym: df.syms){
                cout<<"      "<<sym.symbol<<endl;
                if(!sym.comment.empty()){prefix_lines(cout, sym.comment, "        // ");cout<<endl;}
                if(!sym.fwddecl.empty()){prefix_lines(cout, sym.fwddecl, "        ");cout<<endl;}
            }
        }
    }
    auto plib = dllbuild.create(); // unique_ptr<DllOpen>
    if(nerr){
        cerr<<" Encountered "<<nerr<<" errors during C-file and dll file creation."<<endl;
        exit(2);
    }
    if(!plib){
#if !defined(__ve)
        cout<<" Warning: DllBuild for x86 is a new feature..."<<endl;
#else
        THROW(" dllopen failed");
#endif
    }
#if 1
    DllOpen& lib = *plib;
    for(size_t i=0U; i<lib.nSrc(); ++i){
        cout<<" src file #"<<i<<" syms";
        auto symnames = lib[i];
        for(auto s: symnames) cout<<" "<<s;
        cout<<endl;
        // in this demo, every source file had 1 public symbol, some test function.
        assert( symnames.size() == 1 );
        void* symaddr = lib[symnames[0]];
        // coerce to proper test function type
        typedef demoError_t (*JitFn)();
        JitFn jitfn = (JitFn)symaddr;
        // invoke the test function (it takes no parameters)
        jitfn();
    }
#endif

    cout<<"\nGoodbye"<<endl; cout.flush();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
