#include "cblock.hpp"

#ifdef MAIN_CBLOCK
using namespace cprog;
using namespace std;

/** This uses simple paths only, and demonstrates how output
 * order can be controlled.
 * - Simple Pre/Post-Indent "manipulator" demo.
 * - Notice that begin/middle/end constructs are awkward.
 */
void test_cblock_basic(){
    Cunit pr("program");
    pr.v = 3;
    // Very important to use 'auto&' instead of plain 'auto'
    auto& includes = pr["includes"];
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
    pr["includes"]<<"#include <iostream>";
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
    // Very important to use 'auto&' instead of plain 'auto'
    auto& includes = pr["includes"];
    assert( root.find("includes") != nullptr );
    assert( root.find("includes")->getName() == "includes" );
    assert( root.find("includes") == &pr["includes"] );
    assert( root.find("/includes") != nullptr );
    assert( root.find("./includes") != nullptr );
    assert( root.find("../includes") != nullptr ); // because .. of root is root again
    assert( root.find("open") == nullptr );
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
    pr["extern_C"]["close"] // or as multiple strings
        <<"\n#ifdef __cplusplus\n"
        <<"}//extern \"C\"\n"
        <<"#endif //C++\n\n";
    //auto& functions = (pr["functions"].after(extern_c["open"]));
    auto& functions = extern_c["open"]["functions"]; // simpler equiv
    // '\:' --> '\\:'
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr["includes"]<<"#include <iostream>";
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

    //main.after("extern_C/open");
    pr.v = 2;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
int main(int,char**){
    test_cblock_basic();
    test_cblock_path();
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif // MAIN_CBLOCK
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
