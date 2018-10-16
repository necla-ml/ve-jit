
#include "symScopeUid.hpp"

#include <iostream>
using namespace std;

using namespace scope;
using namespace scope::detail;

#if 1
namespace scope {

// just for demo...
inline std::string bogusname(unsigned const uid){
    std::ostringstream oss;
    oss<<uid;
    return oss.str();
}
// interesting that this somewhat different-looking constructor can be supplied like
// a specialization, out-of-class...
template<class BASE>
    template<typename... Arg>
inline ParSymbol<BASE>::ParSymbol(/*Ssym *ssym,*/ unsigned uid, unsigned scope, Arg&&... arg)
    : BASE(bogusname(uid).c_str(), std::forward<Arg>(arg)...)
      , uid(uid), scope(scope), ssym(nullptr), active(true)
{
    int const verbose=1;
    if(verbose){
        std::cout<<" Par(id"<<uid<<",sc"<<scope<<")" // <<ssym@"<<(void*)ssym
            <<"'"<<BASE::name<<"'"; std::cout.flush();
    }
}
}//scope::
#endif

static int testNum=0;
#define TEST(MSG) do{ \
    ++testNum; \
    std::cout<<"@@@ TEST "<<testNum<<" @@@ "; \
    if(MSG) std::cout<<MSG; \
    std::cout<<std::endl; \
}while(0)

#if 0
struct VerboseSymbolState {
    void setActive(bool state) {
        this->active = active;
        cout<<(active?"+":"-");
    }
    bool getActive() const { return active; }
  private:
    bool active;
};
#endif

void test1(){
    //typedef SymScopeUid<> Ssu;
    //typedef SymScopeUid<VerboseSymbolState> Ssu;
    typedef SymScopeUid Ssu;
    TEST("Construct SymScopeUid");
    {
        Ssu ssu;
        ssu.prtCurrentSymbols();
        assert( ssu.scope() == 1U );
        assert( ssu.nScopeSymbols() == 0U );
        assert( ssu.scopeOf(1U) == 0U );
        assert( ssu.active(1U) == 0U );
        cout<<endl;
    }
    TEST("global scope");
    {
        Ssu ssu;
        auto x = ssu.newsym();
        assert( x!=0U );
        auto y = ssu.newsym();
        assert( y!=0U );
        assert( y!=x );
        assert( ssu.scopeOf(x) == 1U );
        assert( ssu.scopeOf(y) == 1U );
        assert( ssu.active(x) == 1U );
        assert( ssu.active(y) == 1U );
        cout<<endl; ssu.prtCurrentSymbols(); cout<<endl;
        auto zscope = ssu.begin_scope();
        assert( ssu.scope() == 2U );
        auto z = ssu.newsym();
        assert( z > y );
        assert( ssu.scopeOf(x) == 1U );
        assert( ssu.scopeOf(y) == 1U );
        assert( ssu.active(x) == 1U );
        assert( ssu.active(y) == 1U );
        assert( ssu.scopeOf(z) == 2U );
        assert( ssu.active(z) == 2U );
        cout<<endl; ssu.prtCurrentSymbols(); cout<<endl;
        ssu.end_scope();
        cout<<endl; ssu.prtCurrentSymbols(); cout<<endl;
        assert( ssu.scopeOf(x) == 1U );
        assert( ssu.scopeOf(y) == 1U );
        assert( ssu.active(x) == 1U );
        assert( ssu.active(y) == 1U );
        assert( ssu.scopeOf(z) == 2U );
        cout<<" ssu.active(z) = "<<ssu.active(z)<<endl;
        assert( ssu.active(z) == 0U ); // <---

        ssu.activate_scope(zscope);
        assert( ssu.scopeOf(z) == 2U );
        assert( ssu.active(z) == 2U );
        ssu.end_scope();
        assert( ssu.scopeOf(z) == 2U );
        assert( ssu.active(z) == 0U );

    }
}
void test2(){
    //typedef SymScopeUid<> Ssu;
    //typedef SymScopeUid<VerboseSymbolState> Ssu;
    typedef SymStates<> Sst;
    TEST("Construct SymScopeUid");
    {
        Sst sst;
        sst.prtCurrentSymbols();
        assert( sst.scope() == 1U );
        assert( sst.nScopeSymbols() == 0U );
        assert( sst.scopeOf(1U) == 0U );
        assert( sst.active(1U) == 0U );
        cout<<endl;
    }
    TEST("global scope");
    {
        Sst sst;
        auto x = sst.newsym();
        assert( x!=0U );
        auto y = sst.newsym();
        assert( y!=0U );
        assert( y!=x );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        auto zscope = sst.begin_scope();
        cout<<"scope="<<zscope<<" "; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scope() == 2U );
        auto z = sst.newsym();
        cout<<"z="<<z<<endl; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl; cout.flush();
        cout.flush();
        cout<<" sst.active(z)="<<sst.active(z)<<endl;
        cout.flush();
        assert( sst.active(z) );
        assert( z > y );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U ); // oops?
        sst.end_scope();
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        cout<<" sst.active(z) = "<<sst.active(z)<<endl;
        assert( sst.active(z) == 0U ); // <---

        sst.activate_scope(zscope);
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U );
        sst.end_scope();
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 0U );

    }
}
void test2b(){
    // the new ParSymbol<BASE> also has active(), getActive() and uid members...
    typedef SymStates<ParSymbol<ExBaseSym>> Sst;
    TEST("Construct SymScopeUid");
    {
        Sst sst;
        sst.prtCurrentSymbols();
        assert( sst.scope() == 1U );
        assert( sst.nScopeSymbols() == 0U );
        assert( sst.scopeOf(1U) == 0U );
        assert( sst.active(1U) == 0U );
        cout<<endl;
    }
    // Any more is doomed, because ParSymbol constructor requires
    // that the first arg be a back-pointer to the SymStates object.
    //
    // This is just a minor difference and was patched by providing
    // a back-pointerless constructor, that assigned a bogusname
    // to any symbol.
    //
    // Named symbols use class SymbStates instead.
    TEST("global scope");
    {
        Sst sst;
        auto x = sst.newsym();
        assert( x!=0U );
        auto y = sst.newsym();
        assert( y!=0U );
        assert( y!=x );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
    }
    TEST("sub scope");
    {
        Sst sst;
        auto x = sst.newsym();
        auto y = sst.newsym();
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        auto zscope = sst.begin_scope();
        cout<<"scope="<<zscope<<" "; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scope() == 2U );
        auto z = sst.newsym();
        cout<<"z="<<z<<endl; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl; cout.flush();
        cout.flush();
        cout<<" sst.active(z)="<<sst.active(z)<<endl;
        cout.flush();
        assert( sst.active(z) );
        assert( z > y );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U ); // oops?
        sst.end_scope();
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        cout<<" sst.active(z) = "<<sst.active(z)<<endl;
        assert( sst.active(z) == 0U ); // <---

        sst.activate_scope(zscope);
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U );
        sst.end_scope();
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 0U );

    }
}
void test3(){
    //typedef SymScopeUid<> Ssu;
    //typedef SymScopeUid<VerboseSymbolState> Ssu;
    //typedef SymStates<> Sst;
    typedef SymbStates<ExBaseSym> Ssym;
    //typedef ParSymbol<ExBaseSym> Psym;
    TEST("Construct SymScopeUid");
    {
        Ssym ssym;
        ssym.prtCurrentSymbols();
        assert( ssym.scope() == 1U );
        assert( ssym.nScopeSymbols() == 0U );
        assert( ssym.scopeOf(1U) == 0U );
        assert( ssym.active(1U) == 0U );
        cout<<endl;
    }
    TEST("Add 2 symbols to Global scope");
    {
        Ssym ssym;
        cout<<"Init syms "; ssym.prtCurrentSymbols(); cout<<endl;
        assert( ssym.scope() == 1U );
        assert( ssym.nScopeSymbols() == 0U );
        assert( ssym.scopeOf(1U) == 0U );
        assert( ssym.active(1U) == 0U );

        auto x = ssym.newsym("x");
        assert( x!=0U );
        auto y = ssym.newsym("y");
        assert( y!=0U );
        assert( y!=x );
        assert( ssym.scopeOf(x) == 1U );
        assert( ssym.scopeOf(y) == 1U );
        assert( ssym.active(x) == 1U );
        assert( ssym.active(y) == 1U );
        cout<<"\nFinal syms "; ssym.prtCurrentSymbols(); cout<<endl;
    }
    TEST("global scope");
    {
        Ssym ssym;
        auto x = ssym.newsym("x");
        auto y = ssym.newsym("y");
        cout<<endl; ssym.prtCurrentSymbols(); cout<<endl;
        auto zscope = ssym.begin_scope();
        cout<<"scope="<<zscope<<" "; cout.flush();
        ssym.prtCurrentSymbols(); cout<<endl;
        assert( ssym.scope() == 2U );
        auto z = ssym.newsym("z");
        cout<<"z="<<z<<endl; cout.flush();
        ssym.prtCurrentSymbols(); cout<<endl; cout.flush();
        cout.flush();
        cout<<" ssym.active(z)="<<ssym.active(z)<<endl;
        cout.flush();
        assert( ssym.active(z) );
        assert( z > y );
        assert( ssym.scopeOf(x) == 1U );
        assert( ssym.scopeOf(y) == 1U );
        assert( ssym.active(x) == 1U );
        assert( ssym.active(y) == 1U );
        assert( ssym.scopeOf(z) == 2U );
        assert( ssym.active(z) == 2U );
        cout<<"scope "<<ssym.scope(); ssym.prtCurrentSymbols(); cout<<endl;
        ssym.end_scope();
        cout<<"scope "<<ssym.scope(); ssym.prtCurrentSymbols(); cout<<endl;
        assert( ssym.scopeOf(x) == 1U );
        assert( ssym.scopeOf(y) == 1U );
        assert( ssym.active(x) == 1U );
        assert( ssym.active(y) == 1U );
        assert( ssym.scopeOf(z) == 2U );
        cout<<" ssym.active(z) = "<<ssym.active(z)<<endl;
        assert( ssym.active(z) == 0U ); // <---

        ssym.activate_scope(zscope);
        assert( ssym.scopeOf(z) == 2U );
        assert( ssym.active(z) == 2U );
        ssym.end_scope();
        assert( ssym.scopeOf(z) == 2U );
        assert( ssym.active(z) == 0U );

    }
}

int main(int,char**){
    cout<<"======== test1() ==========="<<endl;
    test1();
    cout<<"======== test2() ==========="<<endl;
    test2();
    cout<<"======== test2b() =========="<<endl;
    test2b();
    cout<<"======== test3() ==========="<<endl;
    test3();
    cout<<"\nGoodbye - ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
