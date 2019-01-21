#include "cblock.hpp"

#ifdef MAIN_CBLOCK
using namespace cprog;
using namespace std;
int main(int,char**){
    Cunit pr("program");
    pr.v = 3;
    auto & includes = pr.root["includes"];
    auto & macros = pr.root["macros"];
    auto & functions = pr.root["functions"];
    auto & end = pr.root["end"];
    pr["includes"].append("#include <iostream>");
    pr["macros"].append("#define MSG \"hello\"");
    auto & extern_c = pr["extern_C"]["open"].append("#ifdef __cplusplus\nextern \"C\" {");
    pr["extern_C"]["close"].append("}//extern \"C\"");
    auto & main = pr["functions"]["main"];
    main["beg"].append("int foo() {");
    main["mid"].append("return 7");
    main["end"].append("}");
    main.after(extern_c);
    //main.after("extern_C/open");
    pr.v = 0;
    pr.write(cout);
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif // MAIN_CBLOCK
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
