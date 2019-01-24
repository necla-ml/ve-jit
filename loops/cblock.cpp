#include "cblock.hpp"

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
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"];
    auto& macros = pr["macros"];
    auto& extern_c = pr["extern_C"];
    extern_c["open"]<<
        "\n#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif //C++\n";
    pr["extern_C"]["close"] // or as multiple strings
        <<"\n#ifdef __cplusplus\n"
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
        <<"\n#ifdef __cplusplus\n"
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
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"]<<"\n#include <stdio.h>";
    pr["macros"]<<"\n#define MSG \"hello\"";
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
            <<"\nret += i*magic2;\t// how to 'randomize' ret a bit";
        // oh, I wanted a magic const to be hoisted up into "entry" code...
        foo_body["entry"]<<"\nint const magic=0x12345678;";
        // or do an 'upward search' for a previously created code block (or stub)
        //for00.up["entry"]<<"\nint const magic2=0x23456789;";
        for00["..*/entry"]<<"\nint const magic2=0x23456789;";
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

string asDec(size_t s){
    ostringstream oss;
    oss<<s;
    return oss.str();
}

void test_cblock_short2(){
    Cunit pr("program");
    pr.v = 0;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"]<<"\n#include <iostream>";
    pr["macros"]<<"\n#define MSG \"hello\"";
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
                    <<"\nret += (i*23+j)*magic2;\t// how to 'randomize' ret a bit";

                // I JIT realize this optimization uses some
                // undefined const values into foo/body/entry
                // ... NO PROBLEM ... Let's add to that code snippet
                foo_body["entry"]<<"\nint const magic=0x12345678;";   // exact destn known
                for01["..*/entry"]<<"\nint const magic2=rand();";     // alt "up-search-near" method
                // oh, I JIT realize I need yet another C header
                pr["includes"]<<"\n#include <stdlib.h>";
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
    // overall structure
    pr["comments"]<<"\n// Cunit output from "<<__func__;
    pr["includes"]<<"\n#include <assert.h>";
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
    macs<<"\nreturn 75/i;";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_macros(){
    Cunit pr("program");
    pr.v = 0;
    // overall structure
    pr["comments"]<<"\n// Cunit output from "<<__func__;
    pr["includes"]<<"\n#include <assert.h>";
    pr["macros"];
    auto& cfuncs = mk_extern_c(pr,"extern_C")["body"]; // many mk_FOO have a "body" section
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // now fill in a function
    pr.v = 10;
    auto& macs = mk_func(pr,"macs","int macs(int i)").after(cfuncs)["body"]
        <<"assert(i>=0);";
    macs["exit"]<<"\nreturn 75/i;";
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
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif // MAIN_CBLOCK
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
