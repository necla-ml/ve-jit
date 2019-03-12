#include "cblock.hpp"
#include <sstream>

#ifdef MAIN_CBLOCK
using namespace cprog;
using namespace std;

#define MUST_THROW(CODE) do{ \
    int threw = 0; \
    try{ \
        CODE; \
    }catch(...){ \
        threw = 1; \
    } \
    if( threw == 0 ){ \
        std::cout<<"Error: following code should have thrown:\n" \
        << #CODE << std::endl; \
        THROW("Stopping now"); \
    } \
}while(0)

/** This uses simple paths only, and demonstrates how output
 * order can be controlled.
 * - Simple Pre/Post-Indent "manipulator" demo.
 * - Notice that begin/middle/end constructs are awkward.
 */
void test_cblock_basic(){
    Cunit pr("program");
    pr.v = 3;
    pr["comments"]>>"// Cunit output from "<<__FILE__;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"];
    auto& macros = pr["macros"];
    auto& extern_c = pr["extern_C"];
    extern_c["beg"]
        <<"\n#ifdef __cplusplus"    // can appen w/ embedded newlines
        "\nextern \"C\" {"
        >>"#endif //C++";           // Note: >> auto-supplies initial newline
    pr["extern_C"]["end"]
        >>"ifdef __cplusplus"
        >>"extern \"C\""
        >>"endif //C++";
    //auto& functions = (pr["functions"].after(extern_c["beg"]));
    auto& functions = extern_c["beg"]["functions"]; // simpler equiv
    // '\:' --> '\\:'
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr["includes"]<<"#include <stdio.h>";
    macros<<"#define MSG \"hello\"";    // easy to add to previously defined code blocks
    macros>>CSTR(#define MSG2 "hello"); // Note: CSTR to auto-escape embeded '"'
    auto& foo = functions["foo"];
    // Klunky beg/middle/end w/ manipulator to adjust write context
    foo["beg"]<<"int foo() {"<<PostIndent(+2);
    foo["mid"]<<"return 7;";
    foo["end"]<<"}"<<PreIndent(-2);
    functions["bar"]<<"int bar(){ return 43; }\n";

    // output order can be adjusted with after: (subtree move)
    functions["foo"].after(functions["bar"]);

    //main.after("extern_C/open");
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_path(){
    Cunit pr("program");
    pr.v = 3;
    Cblock& root = pr.root;
    assert( root.find("") == nullptr );
    assert( root.find("asdfalsdkjfalsdkfj") == nullptr );
    assert( root.find("/") == &root );
    assert( root.find("/..") == &root );
    assert( root.find("/../") == &root );
    assert( root.find(".") == &root );
    assert( root.find("..") == &root );
    assert( root.find("./") == &root );
    assert( root.find("../") == &root );
    assert( root.find("./asdf") == nullptr );
    assert( root.find("../asdf") == nullptr );
    assert( root.find("*/asdf") == nullptr );
    assert( root.find("*/open") == nullptr );
    // creates "/includes", no var because never refered to later
    pr["includes"]<<"#include <stdio.h>";
    assert( root.find("includes") != nullptr );
    assert( root.find("includes")->getName() == "includes" );
    assert( root.find("includes") == &pr["includes"] );
    assert( root.find("/includes") != nullptr );
    assert( root.find("./includes") != nullptr );
    assert( root.find("../includes") != nullptr ); // because .. of root is root again
    assert( root.find("open") == nullptr );
    // Very important to use 'auto&' instead of plain 'auto'
    auto& macros = pr["macros"];
    auto& extern_c = pr["extern_C"];
    assert( extern_c.find("../open") == nullptr );
    assert( extern_c.find("./includes") == nullptr );
    assert( extern_c.find("../includes") != nullptr );
    assert( extern_c.find("/../includes") != nullptr );
    assert( extern_c.find("extern_C") != nullptr );
    extern_c["open"]<<
        "\n#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif //C++\n";
    assert( extern_c.find("../open") == nullptr );
    assert( extern_c.find("./includes") == nullptr );
    assert( extern_c.find("../includes") != nullptr );
    assert( root.find("open") == nullptr );
    assert( extern_c.find("open") != nullptr );
    assert( extern_c.find("./open") != nullptr );
    assert( extern_c.find("./open//") != nullptr );
    assert( root.find("*/open") != nullptr );
    assert( root.find("**/open") != nullptr );
    assert( root.find("extern_C/open") != nullptr );
    pr["extern_C"]["close"] // or as multiple strings
        >>"#ifdef __cplusplus\n"
        <<"}//extern \"C\"\n"
        <<"#endif //C++\n\n";
    //auto& functions = (pr["functions"].after(extern_c["open"]));
    auto& functions = extern_c["open"]["functions"]; // simpler equiv
    // '\:' --> '\\:'
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr["includes"]<<"#include <stdio.h>";
    macros<<"#define MSG \"hello\"";
    auto& foo = functions["foo"];
    // Klunky beg/middle/end w/ manipulator to adjust write context
    foo["beg"]<<"int foo() {"<<PostIndent(+2);
    foo["mid"]<<"return 7;";
    foo["end"]<<"}"<<PreIndent(-2);
    functions["bar"]<<"int bar(){ return 43; }\n";

    // output order can be adjusted with after:
    functions["foo"].after(functions["bar"]);
    assert( root.find("*/foo") == nullptr );
    assert( root.find("**/foo") != nullptr );
    assert( root.find("/extern_C/open/functions/bar/foo/mid/") != nullptr );
    assert( root.find("extern_C/open/functions/bar/foo/mid/") != nullptr ); 
    assert( root.find("program/extern_C/open/functions/bar/foo/mid/") == nullptr ); 
    assert( functions.find("..*/open") != nullptr );
    cout<<"\n\n"<<endl; cout.flush();
    assert( foo.find("..*/bar/") != nullptr ); // find in recursive "parent/sibling tree"

    // Cblock::operator[p] was extended.
    // ORIGINAL behaviour for non-path p [no /] is to create the component
    // if it is not an immediate subblock.
    // NEW behaviour allows p to be a path,
    // and throws if p.empty() or p is a nonexistent path.
    try{
        std::cout<<"\n\n Cblock::at(p) tests"<<std::endl;
        assert( root.at(".").getName() == "program" );
        assert( root.at("..").getName() == "program" );
        assert( root.at("/macros").getName() == "macros" );
        assert( root.at("./macros").getName() == "macros" );
        assert( root.at("macros").getName() == "macros" );
        assert( root.at("*/open").getName() == "open" );
        assert( root.at("**/foo").getName() == "foo" );
        assert( root.at("*/../macros").getName() == "macros" );
    }catch(...){
        cout<<" Caught something\n";
        throw;
    }
    std::cout<<"\n\n Cblock::at(p) THROW tests"<<std::endl;
    MUST_THROW(root["*"]); // Paranoia about bugs wrt wildcards
    MUST_THROW(root[".illegal"]);
    MUST_THROW(root[".illegal"]);
    MUST_THROW(root.at("*"));
    MUST_THROW(root.at("*illegal"));
    MUST_THROW(root.at(".illegal"));

    MUST_THROW(root.at("asdfqewrasdf"));
    MUST_THROW(root.at("macrossss"));
    MUST_THROW(root.at("*/foo"));
    MUST_THROW(root.at("**/asdlkfj"));
    MUST_THROW(root.at("/extern_c/open")); // should be capital C
    MUST_THROW(root.at("never_seen_path"));

    try{
        std::cout<<"\n\n Cblock::operator[](p) tests"<<std::endl;
        assert( root["."].getName() == "program" );
        assert( root[".."].getName() == "program" );
        assert( root["/macros"].getName() == "macros" );
        assert( root["./macros"].getName() == "macros" );
        assert( root["macros"].getName() == "macros" );
        assert( root["*/../macros"].getName() == "macros" );
        assert( root["*/open"].getName() == "open" );
        assert( root["*/open"].fullpath() == "/extern_C/open" );
        assert( root["**/open"].getName() == "open" );
        // 1-component ==> create if never seen ...
        assert( root["never_seen_path"].getName() == "never_seen_path" );
    }catch(...){
        cout<<" Caught something\n";
        throw;
    }
    std::cout<<"\n\n Cblock::operator[](p) THROW tests"<<std::endl;
    MUST_THROW(root["./newsub"]); // cf. root["newsub"] which never fails
    MUST_THROW(root["*/foo"]);
    MUST_THROW(root["**/asdlkfj"]);
    MUST_THROW(root["/extern_c/open"]); // should be capital C

    //main.after("extern_C/open");
    pr.v = 2;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_short(){
    Cunit pr("program");
    pr.v = 0;
    pr["comments"]>>"// Cunit output from "<<__FILE__;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"]>>"#include <stdio.h>";
    pr["macros"]>>"#define MSG \"hello\"";
    //mk_extern_c(pr,"extern_C").after(pr["macros"]);
    //  new function: after can accept an absolute path
    mk_extern_c(pr,"extern_C").after("/macros");

    // creates "functions", appends foo/decl foo/body foo/end
    auto& foo_body = mk_func(pr,"foo","int foo()").after(pr["functions"])["body"];
    // for complex items, foo_body will itself get subdivided!

    // append(string) to _code is really only useful for simple stuff.
    // complex cases should name all sections/blocks ...
    foo_body["entry"]<<"int ret=-1;";   // foo/body/entry (anchored)

    // OK, this seems a reasonable idiom .. if00 --> path "/**/foo/body/if00/body"
    auto& if00 = mk_scope(pr,"if00","if(!defined(__cplusplus))").after(foo_body)["body"]; {
        if00<<"ret = 1;";
    }
    // with macros, a bit more readable, else00 --> Cblock path "/**/foo/body/else00/body"
    CBLOCK_SCOPE(else00,"else",pr,foo_body) {
        else00<<"ret = 2;";
    }
    CBLOCK_SCOPE(for00,"for(int i=0;i<10;++i)",pr,foo_body) {
        for00<<"ret = (ret^magic); //just to demo"
            >>"ret += i*magic2;\t// how to 'randomize' ret a bit";
        // oh, I wanted a magic const to be hoisted up into "entry" code...
        foo_body["entry"]>>"int const magic=0x12345678;";
        // or do an 'upward search' for a previously created code block (or stub)
        //for00.up["entry"]>>"int const magic2=0x23456789;";
        for00["..*/entry"]>>"int const magic2=0x23456789;";
    }
    //foo_body["exit"]<<"printf(\"%s\\nGoodbye\",MSG);\nreturn ret;";
    foo_body["exit"]<<CSTR(printf("%s\nGoodbye",MSG);\nreturn ret;);

    // short functions can be ... very short
    //mk_func(pr,"bar","int bar()").after(pr["functions"])["body"]<<"return 7";
#if 1-1
    // can we do Pre(Indent(-2)) for _pre.push_front(...) ?
    // or Pre-Manip, Pre+Manip for _pre.push_front/back <----
#endif
    // output order can be adjusted with after:
    pr["functions"].after("/**/extern_C/body"); // The "/**" is just for show
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    //main.after("extern_C/open");
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}

void test_cblock_short2(){
    Cunit pr("program");
    pr.v = 0;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["comments"]>>"// Cunit output from "<<__FILE__;
    pr["includes"]>>"#include <iostream>";
    pr["macros"]>>"define MSG \"hello\"";
    mk_extern_c(pr,"extern_C").after("/macros");
    // a somewhat more complicated function...
    auto& foo_body = mk_func(pr,"foo","int randomizer()").after(pr["functions"])["body"];
    foo_body["entry"]<<"int ret=-1;";   // foo/body/entry (anchored)
    int opt_level = 0; // my JIT decision making procedure

    CBLOCK_SCOPE(if00,"if(__FILE__[0]=='b')",pr,foo_body) {
        if00<<"ret = 1;";
    }
    CBLOCK_SCOPE(else00,"else",pr,foo_body) {
        else00<<"ret = 2;";
    }
    foo_body["preloop"]<<"// I have selected JIT randomization method "<<asDec(opt_level);
    CBLOCK_SCOPE(for00,"for(int i=0; i<10; ++i)",pr,foo_body) {
        CBLOCK_SCOPE(for01,"for(int j=i;i<10;++j)",pr,for00) {
            // Scenario: I have many JIT possibilities, but I decide to
            // use the following code ....
            if(opt_level==0) { // Oh, maybe I want my original JIT version ...
                for01<<"ret = (ret^magic); //just to demo"
                    // original version ...
                    >>"ret += (i*23+j)*magic2;\t// how to 'randomize' ret a bit";

                // I JIT realize this optimization uses some
                // undefined const values into foo/body/entry
                // ... NO PROBLEM ... Let's add to that code snippet
                foo_body["entry"]>>"int const magic=0x12345678;";   // exact destn known
                for01["..*/entry"]>>"int const magic2=rand();";     // alt "up-search-near" method
                // oh, I JIT realize I need yet another C header
                pr["includes"]>>"#include <stdlib.h>";
            }else if(opt_level==1){
                for01<<"ret = ret + i + j"; // this JIT is faster (but maybe not so random)
            }else{
                // OTHER JIT impls of foo are not shown, but may need entirely different
                //                 sets of code and patches to upward blocks of pr
            }
        }//j-loop
    }//i-loop
    foo_body["exit"]<<CSTR(printf("%s\nGoodbye",MSG);\nreturn ret;);
    pr["functions"].after("/**/extern_C/body"); // allow generous search (it is not directly under root)
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_dump(){
    Cunit pr("program");
    pr.v = 0;
    // overall structure, also demo new ','-operator (auto-supplies an initial newline)
    pr["comments"]>>"// Cunit output from "<<__func__;
    pr["includes"]>>"#include <assert.h>";
    pr["macros"];
    auto& cfuncs = mk_extern_c(pr,"extern_C")["body"]; // many mk_FOO have a "body" section
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // now fill in a function
    pr.v = 10;
    pr["/"].dump(cout);
    cout<<"\nHmmmm.  Let's look at dump to see how one instruction worked\n\n"<<endl;
    auto& macs = mk_func(pr,"macs","int macs(int i)")
        .after(cfuncs)["body"]
        <<"assert(i>=0);";
    pr["/"].dump(cout);
    macs>>"return 75/i;";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}

/** based on a very short (slow) direct_default3.c */
string cjitConvolutionForward00( int const verbosity=0 /*struct param const* const p*/ )
{
    Cunit pr("program");
    pr["includes"]<<Endl<<CSTR(#include "vednn.h")
        <<Endl<<CSTR(#include "veintrin.h")
        <<"\n#include <stdio.h>"
        <<"\n#include <stdlib.h>"
        <<"\n#include <assert.h>"
        ;
    pr["macros"]<<"\n#define VLEN (256)"
        ;
    //auto & fns = mk_extern_c(pr,"extern_C").after(pr["/macros"])["body"];
    auto & fns = mk_extern_c(pr,"extern_C")["body"];
    //auto & fns = mk_extern_c(pr,"extern_C")["body/.."];

    std::string fn_declare;
    {
        std::string funcname("cjitConvFwd00");
        std::ostringstream oss;
        oss<<"void "<<funcname<<"("
            <<"\n        const vednnTensorParam_t * restrict      pParamIn,"
            <<"\n        const void * restrict                    pDataIn,"
            <<"\n        const vednnFilterParam_t * restrict      pParamKernel,"
            <<"\n        const void * restrict                    pDataKernel,"
            <<"\n        const vednnConvolutionParam_t * restrict pParamConv,"
            <<"\n        const vednnTensorParam_t * restrict      pParamOut,"
            <<"\n        void * restrict                          pDataOut"
            <<"\n        )";
        fn_declare = oss.str();
    }
    auto& fn = mk_func(pr,"fn",fn_declare).after(fns)["body"];

    // get the vars here first.
    const int64_t batch          = 52;
    const int64_t group          = 2;
    const int64_t inChannel      = 100;
    const int64_t inHeight       = 27;
    const int64_t inWidth        = 27;
    const int64_t outChannel     = 100;
    const int64_t outHeight      = 27;
    const int64_t outWidth       = 27;
    const int64_t kernHeight     = 3;
    const int64_t kernWidth      = 3;
    const int64_t strideHeight   = 1;
    const int64_t strideWidth    = 1;
    const int64_t padHeight      = 1;
    const int64_t padWidth       = 1;
    const int64_t dilationHeight = 1; // mkl-dnn value plus one
    const int64_t dilationWidth  = 1;
    assert( outWidth > 0 );

    const int64_t inChannelGroup  = inChannel  / group;   // equal to pDataKernel->inChannel
    const int64_t outChannelGroup = outChannel / group;   // equal to pDataKernel->outChannel

    const int64_t inHW = inHeight * inWidth;
    const int64_t kernHW = kernHeight * kernWidth;
    const int64_t outHW = outHeight * outWidth;

    // then emit them as constant cjit values (or #define them)
//#define CONST1(var) >>("int64_t const " #var " = "+asDec(var))
// #define is better, because is is definitely usable with 'C' compiler
#define CONST1(var) >>("#define " #var " "+asDec(var))
    //auto& fn_const =
    fn["const"]
        CONST1(batch            )
        CONST1(group            )
        CONST1(inChannel        )
        CONST1(inHeight         )
        CONST1(inWidth          )
        CONST1(outChannel       )
        CONST1(outHeight        )
        CONST1(outWidth         )
        CONST1(kernHeight       )
        CONST1(kernWidth        )
        CONST1(strideHeight     )
        CONST1(strideWidth      )
        CONST1(padHeight        )
        CONST1(padWidth         )
        CONST1(dilationHeight   )
        CONST1(dilationWidth    )

        CONST1(inChannelGroup   )
        CONST1(outChannelGroup  )

        CONST1(inHW             )
        CONST1(kernHW           )
        CONST1(outHW            )
        ;
#if 0
    const float * restrict pIn     = pDataIn;
    const float * restrict pKernel = pDataKernel;
    //float * restrict const pOut    = pDataOut;
    float * restrict pOut    = pDataOut;
#endif
    auto& fn_ptrs = fn["ptrs"];
    fn_ptrs>>"float const * restrict pIn  = pDataIn;"
        >>"float const * restrict pKernel = pDataKernel;"
        >>"float * restrict pOut = pDataOut;"
        ;

    //auto& fn_vec_init =
    fn["vec_init"]
        >>"_ve_lvl(VLEN);"
        >>"const __vr vzeros = _ve_vbrdu_vs_f32(0.0f); // lower 32-bits are zero bits, so same as _ve_pvbrd_vs_i64(0UL)"
        >>"const __vr vrseq = _ve_vseq_v();"
        >>"const int64_t sw_x_VLEN = strideWidth * VLEN;"
        >>"int64_t const vl_x_init = outWidth /*- x0=0*/ < VLEN ? outWidth /*- x0=0*/ : VLEN ;"
        >>"int64_t vl = vl_x_init;"
        >>"_ve_lvl(vl);"
        >>"__vr const vrj_init = _ve_vaddsl_vsv(-padWidth,  _ve_vmulsl_vsv(strideWidth, vrseq));"
        ;

    CBLOCK_SCOPE(loop_n,"for(int64_t n=0; n<batch; ++n)",pr,fn);
    CBLOCK_SCOPE(loop_g,"for(int64_t g=0; g<group; ++g)",pr,loop_n); // OK sub-tree
    loop_g
        >>"const int64_t outGroupOffset  = g * outChannelGroup * outHW;"
        >>"const int64_t inGroupOffset   = g * inChannelGroup * inHW;"
        >>"const int64_t kernGroupOffset = g * outChannelGroup * inChannelGroup * kernHW;"
        >>"const float *pIn_0 = pIn + inGroupOffset + (n * inChannel + 0) * inHW;"
        ;
    CBLOCK_SCOPE(loop_k,"for(int64_t k=0 ; k<outChannelGroup; ++k)",pr,loop_g);
    loop_k
        >>"int64_t outIndex = outGroupOffset + (n * outChannel + k) * outHW;"
        >>"const float * restrict pKern_gk = pKernel + kernGroupOffset"
        >>"                                + (k * inChannelGroup + 0) * kernHW;"
        >>"//int64_t kIndex_0 = kernGroupOffset + (k * inChannelGroup + 0) * kernHW;"
        ;
    CBLOCK_SCOPE(loop_y,"for(int64_t y=0 ; y<outHeight; ++y)",pr,loop_k);
    loop_y
        >>"const int64_t i = y * strideHeight - padHeight;"
        >>""
        >>"int64_t kh_end=0;"
        >>"const int64_t kh_tmp = dilationHeight-i-1;"
        >>"const int64_t kh_beg= (i>=0? 0: kh_tmp / dilationHeight);"
        >>"if (i < inHeight){"
        >>"  kh_end = (inHeight + kh_tmp) / dilationHeight;"
        >>"  if (kh_end >= kernHeight) kh_end = kernHeight;"
        >>"}"
        >>""
        >>"int64_t vl = vl_x_init;"
        >>"_ve_lvl(vl);"
        >>"__vr vrj = vrj_init;"
          ;
    CBLOCK_SCOPE(loop_x0,"for(int64_t x0=0 ; x0<outWidth; x0+=VLEN)",pr,loop_y);
    loop_x0
            >>"const int64_t vl = outWidth - x0 < VLEN ? outWidth - x0: VLEN;"
            >>"_ve_lvl(vl);"
            >>"__vr vrsum = vzeros;"
            ;
    CBLOCK_SCOPE(loop_r,"for (int64_t r = kh_beg; r < kh_end; ++r)",pr,loop_x0);
    loop_r>>"vrw = vrj";
    CBLOCK_SCOPE(loop_s,"for (int64_t s = 0; s < kernWidth; s++)",pr,loop_r);
    loop_s
        >>"__vm256 vm2 = _ve_vfmkl_mcv(VECC_GE, vrw);        // condition(0 <= w)"
        >>"__vm256 vm3 = _ve_vfmkl_mcv(VECC_IG, _ve_vcmpsl_vsv(inWidth,vrw));  // condition(w < inWidth)"
        >>"__vm256 vm23  = _ve_andm_mmm(vm2, vm3);"
        ;
    CBLOCK_SCOPE(loop_c,"for (int64_t c = 0; c < inChannelGroup; ++c)",pr,loop_s);
    loop_c
        >>"const float *pIn = pIn_0 + c*inHW + (i+r*dilationHeight)*inWidth"
        >>"                 + x0*strideWidth-padWidth + s*dilationWidth;"
        >>"const float *pKerValue = pKern_gk + c*kernHW + r*kernWidth +s;"
        >>"__vr vrin = _ve_vldu_vss(4*strideWidth,pIn) ;"
        >>"vrin = _ve_vmrg_vvvm(vzeros, vrin, vm23) ;"
        >>"vrsum = _ve_vfmads_vvsv(vrsum, *pKerValue, vrin) ;"
        ;
    loop_s["induce vrw"]// BEFORE the '}' of loops_s (embedded blanks OK, but harder to read
        >>"vrw = _ve_vaddsl_vsv(dilationWidth,  vrw) ; // <--- vector induced"
        ;
    // loop_r path: .../loop_x0/body/loop_r/body
    //loop_r[".."] // too early (add to loop_x0/body; before loop_r even begins)
    //loop_r["../.."] // same as above
    //loop_r["end"] // too early, before "}//loop_r", creates path loop_x0/body/loop_r/body/end
    //loop_r["../end"]      // OK, adds to /**loop_x0/body/loop_r/end
    //loop_r[".."]["done"]  // OK, adds to append-tree /**/loop_x0/body/loop_r/done
    loop_x0["induce+store"]       // OK, adds to /**/loop_x0/body/induce+store
        >>"_ve_vstu_vss(vrsum, 4, pOut) ;"
        >>"vrj = _ve_vaddsl_vsv(sw_x_VLEN,vrj); // induce to avoid full recalc"
        >>"pOut += vl; // visible speedup cf. outIndex+=vl"
        ;
    fn["exit"]>>"return VEDNN_SUCCESS;" ;


    pr["end"]>>"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // debug: 'str()' should bypass the write-counting thing and always work
    auto const sz0 = pr.str().size();
    cout<<" pr.str().size() = "<<sz0;
    assert( sz0 > 0 );
    auto const sz1 = pr.str().size();
    cout<<" pr.str().size() = "<<sz1;
    assert( sz1 == sz0 );
    // For demo program, dump the full tree...
    cout<<string(80,'-')<< pr.tree() <<string(80,'-')<<endl;
    cout<<string(80,'-')<< pr.str() <<string(80,'-')<<endl;

    pr.v = verbosity;
    return pr.str();
}

#if 0 // original function, for comparison...
vednnConvolutionForward_direct_default3(
    const vednnTensorParam_t * restrict   pParamIn,
    const void * restrict       pDataIn,
    const vednnFilterParam_t * restrict   pParamKernel,
    const void * restrict       pDataKernel,
    const vednnConvolutionParam_t * restrict   pParamConv,
    const vednnTensorParam_t * restrict   pParamOut,
    void * restrict         pDataOut
)
{
  const int64_t batch      = pParamIn->batch;
  const int64_t inChannel  = pParamIn->channel;
  const int64_t inWidth    = pParamIn->width;
  const int64_t inHeight   = pParamIn->height;
  const int64_t outChannel = pParamOut->channel;
  const int64_t outWidth   = pParamOut->width;
  const int64_t outHeight  = pParamOut->height;
  const int64_t kernWidth  = pParamKernel->width;
  const int64_t kernHeight = pParamKernel->height;
  assert( outWidth > 0 );

  const int64_t group          = pParamConv->group;
  const int64_t strideWidth    = pParamConv->strideWidth;;
  const int64_t strideHeight   = pParamConv->strideHeight;
  const int64_t padWidth       = pParamConv->padWidth;
  const int64_t padHeight      = pParamConv->padHeight;
  const int64_t dilationWidth  = pParamConv->dilationWidth;
  const int64_t dilationHeight = pParamConv->dilationHeight;

  const int64_t inChannelGroup  = inChannel  / group;   // equal to pDataKernel->inChannel
  const int64_t outChannelGroup = outChannel / group;   // equal to pDataKernel->outChannel

  const float * restrict pIn     = pDataIn;
  const float * restrict pKernel = pDataKernel;
  //float * restrict const pOut    = pDataOut;
  float * restrict pOut    = pDataOut;

  const int64_t inHW = inHeight * inWidth;
  const int64_t kernHW = kernHeight * kernWidth;
  const int64_t outHW = outHeight * outWidth;


  _ve_lvl(VLEN) ; // <----- VERY VERY VERY IMPORTANT to remember this init !!! 1.
  const __vr vzeros = _ve_vbrdu_vs_f32(0.0f) ; // lower 32-bits are zero bits, so same as _ve_pvbrd_vs_i64(0UL)
  const __vr vrseq = _ve_vseq_v();
  const int64_t sw_x_VLEN = strideWidth * VLEN;
  int64_t const vl_x_init = outWidth /*- x0=0*/ < VLEN ? outWidth /*- x0=0*/ : VLEN ;
  int64_t vl = vl_x_init;
  _ve_lvl(vl) ;
  __vr const vrj_init = _ve_vaddsl_vsv(-padWidth,  _ve_vmulsl_vsv(strideWidth, vrseq));

  //int64_t const kByMax = 1;
  //int64_t const zero = 0;

  for (int64_t n = 0; n < batch; n++) {
    for (int64_t g = 0; g < group; g++) {
      const int64_t outGroupOffset  = g * outChannelGroup * outHW;
      const int64_t inGroupOffset   = g * inChannelGroup * inHW;
      const int64_t kernGroupOffset = g * outChannelGroup * inChannelGroup * kernHW;
      const float *pIn_0 = pIn + inGroupOffset + (n * inChannel + 0) * inHW;
      for(int64_t k=0 ; k<outChannelGroup; ++k) {

        int64_t outIndex = outGroupOffset + (n * outChannel + k) * outHW;
        const float * restrict pKern_gk = pKernel + kernGroupOffset + (k * inChannelGroup + 0) * kernHW;
        //int64_t kIndex_0 = kernGroupOffset + (k * inChannelGroup + 0) * kernHW;

        for (int64_t y=0; y<outHeight; y++) {
          const int64_t i = y * strideHeight - padHeight;

          int64_t kh_end=0;
          const int64_t kh_tmp = dilationHeight-i-1;
          const int64_t kh_beg= (i>=0? 0: kh_tmp / dilationHeight);
          if (i < inHeight){
            kh_end = (inHeight + kh_tmp) / dilationHeight;
            if (kh_end >= kernHeight) kh_end = kernHeight;
          }

          int64_t vl = vl_x_init;
          _ve_lvl(vl) ;
          __vr vrj = vrj_init;
          for ( int64_t x0=0; x0<outWidth; x0+=VLEN )
          {
            const int64_t vl = outWidth - x0 < VLEN ? outWidth - x0 : VLEN ;
            _ve_lvl(vl) ;
            __vr vrsum = vzeros;
            // slower:
            //    any use ov _ve_lvs_svs_u64/f32
            //    any type of blocking 'c' loop (many ways tried)
            //    clang prefetch will not compile
            //    precalc offset expressions (cannnot distribute scalar calc better than clang)
            for (int64_t r = kh_beg; r < kh_end; ++r) {
              //const int64_t h = i + r * dilationHeight; // kh_beg,kh_end guarantee h in [0,outHeight)
              __vr vrw = vrj;
              for (int64_t s = 0; s < kernWidth; s++) {
                __vm256 vm2 = _ve_vfmkl_mcv(VECC_GE, vrw) ;        // condition(0 <= w)
                __vm256 vm3 = _ve_vfmkl_mcv(VECC_IG, _ve_vcmpsl_vsv(inWidth,vrw)) ;  // condition(w < inWidth)
                __vm256 vm23  = _ve_andm_mmm(vm2, vm3) ;
                for (int64_t c = 0; c < inChannelGroup; ++c)
                {
                  const float *pIn = pIn_0 + c*inHW + (i+r*dilationHeight)*inWidth + x0*strideWidth-padWidth + s*dilationWidth;

                  const float *pKerValue = pKern_gk + c*kernHW + r*kernWidth +s;
                  __vr vrin = _ve_vldu_vss(4*strideWidth,pIn) ;
                  vrin = _ve_vmrg_vvvm(vzeros, vrin, vm23) ;
                  vrsum = _ve_vfmads_vvsv(vrsum, *pKerValue, vrin) ;
                } // inChannel

                vrw = _ve_vaddsl_vsv(dilationWidth,  vrw) ; // <--- vector induced (not fully calc)
              } // s .. kernWidth
            } // r .. kernHeight
            //_ve_vstu_vss(vrsum, 4, pOut+outIndex) ;
            _ve_vstu_vss(vrsum, 4, pOut) ;
            vrj = _ve_vaddsl_vsv(sw_x_VLEN,vrj); // induce to avoid full recalc
            //outIndex += vl ; /* MUST always execute (before break) */
            pOut += vl; // visible speedup
          } // x
        } // y
      } //k..kMax..kBy (outChannelGroup)
    } // group
  } // batch

  return VEDNN_SUCCESS;
}
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
void test_cblock_macros(){
    Cunit pr("program");
    pr.v = 0;
    // overall structure
    pr["comments"]>>"// Cunit output from "<<__func__;
    pr["includes"]>>"#include <assert.h>";
    pr["macros"];
    auto& cfuncs = mk_extern_c(pr,"extern_C")["body"]; // many mk_FOO have a "body" section
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // now fill in a function
    pr.v = 10;
    auto& macs = mk_func(pr,"macs","int macs(int i)").after(cfuncs)["body"]
        <<"assert(i>=0);";
    macs["exit"]>>"return 75/i;";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
int main(int,char**){
    test_cblock_basic();
    test_cblock_path();
    test_cblock_short();
    test_cblock_short2();
    test_cblock_dump();
    test_cblock_macros();
    string code = cjitConvolutionForward00(); // optional arg: verbosity=0
    cout<<string(80,'-')<< code <<string(80,'-')<<endl;
    assert(code.size()>0);
    cout<<"\nGoodbye"<<endl; cout.flush();
}
#endif // MAIN_CBLOCK
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
