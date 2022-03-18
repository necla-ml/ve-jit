/** \file
 * demo creating JIT jit library of NumberGuesser functions (can run on x86)
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
DllFile srcCodeNumberGuesser( struct param const* const p )
{
    DllFile df; // return value
    std::string parmstr;
    parmstr = paramString(p);
    df.basename = "numberGuess_"+parmstr;

    // we use intrinsics.  suffix matches build recipe in "bin.mk"
#if defined(__ve)
    //df.suffix = "-vi.cpp";     // C++ VE source, DOES NOT WORK (undefined std::__1::cout)
    df.suffix = "-vi.c";     // C++ VE source, with VE intrinsics (compiled via clang)
#else
    df.suffix = "-x86.cpp";  // C++ x86 source
#endif

    std::string fn_declare = "demoError_t "+df.basename+"()";
    df.syms.push_back(SymbolDecl(df.basename,
                "vednn ConvolutionForward "+paramString(p),
                fn_declare));


    Cunit pr("program");
    auto& includes = pr["includes"]<<Endl;
    auto& macros = pr["macros"];
    includes
#if defined(__ve)
        //
        // I have not gotten VE intrinsics compilation working with clang++ + nc++ library.
        // Theres is undefined symbol
        //    std::__1::cout
        //
        //>>CSTR(#include "veintrin.h")           // old intrinsics (no longer exists)
        >>CSTR(#include "velintrin.h")          // current VE itnrinsics header
        >>"#include <stdio.h>"
        >>"#include <assert.h>"
        >>"#include <stdint.h>"
#else
        // g++ compilation is OK with C++
        >>"#include <iostream>"
        >>"#include <cassert>"
        >>"#include <cstdint>"
#endif
        ;
    macros
        >>"#define VLEN (256)"
#if !defined(__ve)
        >>"using namespace std;"
#endif
        >>"typedef int demoError_t;"
        ;
    auto & fns =
#if defined(__ve)
        pr["body"] // C, no extern "C" needed.
#else
        // mk_extern_c(pr,"extern_C").after(pr["/macros"])["body"];
        mk_extern_c(pr,"extern_C")["body"];
    // mk_extern_c(pr,"extern_C")["body/.."];
#endif
    ;

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
#if defined(__ve)
        >>OSSFMT("printf(\"%s %u.  That took me %s thought\\\n\"")
        >>"    , intro, "<<hashvar
        >>"    , /*JIT optimized decision here*/ "<<(
                example_parm<100? CSTR("almost no")
                :example_parm<200? CSTR("a little")
                :example_parm<300? CSTR("a lot of")
                :CSTR("a huge amount")
                )<<");";
#else
        >>OSSMFT("std::cout<<intro<<"<<hashvar<<"<<\".  That took me \"")
        // JIT eliminate of run-time branch (lol)
        >>"    <</*JIT optimized decision here*/ "<<(
                example_parm<100? CSTR("almost no")
                :example_parm<200? CSTR("a little")
                :example_parm<300? CSTR("a lot of")
                :CSTR("a huge amount")
                )
        >>"    <<\" thought\"<<std::endl;"
#endif
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
    printf(" got %d sets of parameters\n", nParams); fflush(stdout);

    for(int i=0; i<nParams; ++i){
        // sometimes parameters are difficult to set correctly,
        // and to avoid segfault etc and still test something...
        printf("mkConsistent..%d\n",i);
        mkConsistent( &pParams[i] );
    }

    DllBuild dllbuild;
    dllbuild.verbose=7;
    for(int i=0; i<nParams; ++i){
        struct param const& jitAlgParms = pParams[i];
        //
        // - Given `param` describing one algorithm,
        //   1. GENERATE CODE for one source file, and
        //   2. record its PUBLIC SYMBOLS
        //      - often a single testing/timing subroutine (no 'main')
        //
        DllFile df = srcCodeNumberGuesser( &jitAlgParms );
        //
        // - Not shown:
        //   - generate private headers (once, independent of param)
        //   - generate private sources (once, no public symbols)
        //
        dllbuild.push_back(df);
    }
    unique_ptr<DllOpen> plib;
    {
#if defined(__ve)
//#warning "VE compilation!"
#define TMP_CJIT_LIB_DIR "tmp_cjitDemo"
#else
#define TMP_CJIT_LIB_DIR "tmp_cjitDemo_x86"
#endif
        std::string mkEnv;
        {
            //std::ostringstream oss;
            // see bin.mk  --  for x86, C86FLAGS and CXX86FLAGS etc.
            //oss<<"CFLAGS=-I'" CSTR(VEDNNX_DIR) "'"
            //     " CXXFLAGS=-I'" CSTR(VEDNNX_DIR) "'"
            //     " LDFLAGS='-L" CSTR(VEDNNX_DIR) "/lib -Wl,-rpath," CSTR(VEDNNX_DIR) "/lib'"
            //     ;
            //mkEnv=oss.str();
        }
        // Usually you can just do a single call
        char const* libname=
#if defined(__ve)
            "cjitDemo"
#else
            "cjitDemo_x86"
#endif
        ;
        plib = dllbuild.safe_create( libname, TMP_CJIT_LIB_DIR, mkEnv );
        // which either returns a valid pointer, or throws (with debug info)
        // `safe_create` postcondition:
        assert(plib);
        cout<<" dllbuild --> library "<<dllbuild.getLibName()<<endl;
    }
#if 1
    DllOpen& lib = *plib;
    for(size_t i=0U; i<lib.nSrc(); ++i){
        cout<<"\nsrc file #"<<i<<" syms";
        auto symnames = lib[i];
        for(auto s: symnames) cout<<" "<<s;
        cout<<endl;
        // in this demo, every source file had 1 public symbol, some test function.
#if defined(__ve)
        assert( symnames.size() == 2 ); // -vi.c --> "symbol" and "unroll_symbol" are produced
#else
        assert( symnames.size() == 1 || symnames.size() == 2);
#endif
        void* symaddr = lib[symnames[0]];
        // coerce to proper test function type
        typedef demoError_t (*JitFn)();
        JitFn jitfn = (JitFn)symaddr;
        // invoke the test function (it takes no parameters)
        cout<<" jit "<<symnames[0]<<"() ..."<<endl;
        jitfn();
    }
#endif

    free(pParams);
    cout<<"\nGoodbye"<<endl; cout.flush();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
