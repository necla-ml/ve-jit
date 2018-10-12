/** \file
 * \deprecated because symbStates.hpp has been replaced with symScopeUid.hpp and
 *             its \e official scope::SymbStates class now.  (renamed as -old files)
 */
#include "symbStates-old.hpp"

#include <iostream>
using namespace std;

static int testNum=0;
#define TEST(MSG) do{ \
    ++testNum; \
    std::cout<<"@@@ TEST "<<testNum<<" @@@ "; \
    if(MSG) std::cout<<MSG; \
    std::cout<<std::endl; \
}while(0)

void test1(){
    typedef SymbStates<ExBaseSym> Ssym;
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
#if 1
    TEST("Add 2 symbols to Global scope");
    {
        Ssym ssym;
        ssym.prtCurrentSymbols();
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
        cout<<endl; ssym.prtCurrentSymbols(); cout<<endl;
        cout<<endl;
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
        assert( ssym.active(z) == 2U ); // oops?
        ssym.end_scope();
        cout<<endl; ssym.prtCurrentSymbols(); cout<<endl;
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
#endif
}

int main(int,char**){
    cout<<"======== test1() ==========="<<endl;
    test1();
    cout<<"\nGoodbye - ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
