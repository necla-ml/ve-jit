/** \file
 * \deprecated because asm.hpp has been split into several components
 */
#include "asm-old.hpp"

#include <iostream>
using namespace std;
using namespace ve;

static int testNum=0;
#define TEST(MSG) do{ \
    if(MSG) cout<<(char const*)MSG; \
    ++testNum; \
    cout<<" "<<testNum; \
    if(MSG) cout<<endl; \
}while(0)

struct Tester{
    static void test_construct();
};

void Tester::test_construct(){
    TEST("test_construct...");
    {
        Sym sym("MySymbol");
        cout<<" created Sym "<<sym.name<<" RegId="<<sym.id<<endl;
        cout<<" operator<< : "<<sym<<endl;
    }
    TEST(nullptr);
    {
        Regs regs;
        // print a private data member (we are a "Tester" friend)
        cout<<"*regs.s[9].sym = "<<*regs.s[9].sym<<endl;
    }
    TEST("SafeSym USED+SCALAR Test");
    {
        Regs regs;
        SafeSym nItems = regs.alloc(REG_T(USED|SCALAR),"nItems");
        cout<<nItems<<endl;
    }
    TEST("SafeSym operator-> Test");
    {
        SafeSym unset;
        int err = 0;
        try{
            cout<<" unset->id = "<<unset->id<<endl; cout.flush();
        }catch(...){ cout<<" unset->id THREW (good)"<<endl; ++err; }
        assert(err==1);
    }
    TEST("regs.alloc");
    {
        Regs regs;
        cout<<" alloc... "; cout.flush();
        SafeSym nItems = regs.alloc(SCALAR,"nItems");
        cout<<" alloced! "; cout.flush();
        cout<<nItems<<endl;
        // an allocated SafeSym should be usable, even not active (no "value")
        cout<<" nItems->staleness="<<nItems->staleness<<endl; cout.flush();
        cout<<" reg_flags="<<(void*)(intptr_t)nItems.reg_flags()<<endl; cout.flush();
        cout<<" timestamp="<<nItems.time()<<endl; cout.flush();
    }
    TEST("Declare Many");
    {
        char boring_names[80*10];
        for(int i=0; i<800; ++i) boring_names[i] = 'X';
        for(int i=0; i<80; ++i){
            boring_names[10*i+0] = 'X';
            boring_names[10*i+1] = '0'+(i/10); // tens digit
            boring_names[10*i+2] = '0'+i%10;   // ones digit
            boring_names[10*i+3] = '\0';       // string terminator
        }

        Regs regs;
        SafeSym lots[80];
        cout<<" def SafeSym="<<lots[0]<<endl;
        // Issue:
        //   declared registers are assigned, but have no "value".
        //   "indeterminate"
        //   So if we run out and "spill" an unset register we just
        //   set its RegId to IDlast (invalid).
        // Then:
        //   at first "write" to mem-indeterminate register, we
        //   re-allocate a real register, spilling some other
        //   register.  indeterminate-registers can be spilled
        // Alt:
        //   registers are left unassigned until first use
        //      *** MUCH SIMPLER ***
        //   and maybe even more optimal
        for(int i=0; i<80; ++i){
            // original "FORCE" behavior: spill regs at declaration-time
            lots[i] = regs.alloc(SCALAR, &boring_names[10*i]);
            cout<<lots[i]<<endl; cout.flush();
        }
    }
    int const maxBoring = 300;
    char boring_names[maxBoring*10];
    for(int i=0; i<maxBoring*10; ++i) boring_names[i] = 'X';
    for(int i=0; i<maxBoring; ++i){
        boring_names[10*i+0] = 'X';
        boring_names[10*i+1] = '0'+(i/10); // tens digit
        boring_names[10*i+2] = '0'+i%10;   // ones digit
        boring_names[10*i+3] = '\0';       // string terminator
    }
    TEST("Allocate Many");
    {
        Regs regs;
        SafeSym lots[80];
        cout<<" def SafeSym="<<lots[0]<<endl;
        // Issue:
        //   declared registers are assigned, but have no "value".
        //   "indeterminate"
        //   So if we run out and "spill" an unset register we just
        //   set its RegId to IDlast (invalid).
        // Then:
        //   at first "write" to mem-indeterminate register, we
        //   re-allocate a real register, spilling some other
        //   register.  indeterminate-registers can be spilled
        // Alt:
        //   registers are left unassigned until first use
        //      *** MUCH SIMPLER ***
        //   and maybe even more optimal
        for(int i=0; i<80; ++i){
            // original "FORCE" behavior: spill regs at declaration-time
            lots[i] = regs.alloc(REG_T(USED|SCALAR), &boring_names[10*i]);
            //cout<<lots[i]<<endl; cout.flush();
            cout<<' '<<asmname(lots[i]->id)<<endl; cout.flush();
        }
    }
    TEST("Declare Many");
    {
        Regs regs;
        SafeSym lots[80];
        for(int i=0; i<80; ++i){
            lots[i] = regs.declare(REG_T(USED|SCALAR), &boring_names[10*i]);
            // operator* (equiv. *lots[i].safe()) throws if out-of-scope
            cout<<'.'<<*lots[i]<<endl; cout.flush();
        }
    }
    TEST("{Declare,use} x 5");
    {
        Regs regs;
        SafeSym lots[80];
        for(int i=0; i<5; ++i){
            cout<<"i="<<i<<" ";
            lots[i] = regs.declare(REG_T(USED|SCALAR), &boring_names[10*i]);
            regs.out(lots[i]);
            cout<<"\n\t"<<lots[i];
            cout<<endl;
        }
        //Is above equiv to regs::alloc?
        // No.  alloc--> assigned register+spill, but still inactive
        for(int i=6; i<10; ++i){
            cout<<"i="<<i<<" ";
            lots[i] = regs.alloc(REG_T(USED|SCALAR), &boring_names[10*i]);
            cout<<"\n\t"<<lots[i];
            cout<<endl;
        }
        for(int i=11; i<15; ++i){
            cout<<"i="<<i<<" ";
            lots[i] = regs.alloc(REG_T(USED|SCALAR), &boring_names[10*i]);
            regs.out(lots[i]);
            cout<<"\n\t"<<lots[i];
            cout<<endl;
        }
    }
    TEST("Scope Test");
    {
        Regs regs;
        SafeSym x = regs.declare(REG_T(USED|SCALAR), "x");
        cout<<" initscope: "<<x<<endl;
        regs.out(x);
        cout<<"      out-> "<<x<<endl;
        // mov(x,0) (some asm statements)
        regs.scope_begin();
        SafeSym y = regs.declare(REG_T(USED|SCALAR), "x");
        cout<<" sub-scope: "<<y<<endl;
        regs.out(y);
        cout<<"      out-> "<<y<<endl;
        cout<<" sub-scope-end!"; cout.flush();
        regs.scope_end();
#if 1 // this broke at some point, but moving on to new impl anyway
        char const* okmsg=nullptr;
        try{
            cout<<" illegal out..."; cout.flush();
            regs.out(y);
        }catch(...){
            okmsg = " Good: out-of-scope threw exception";
        }
        if(okmsg) cout<<okmsg<<endl;
        else{ cout<<"Ohoh, expected out-of-scope!"<<endl;
            exit(1);
        }
#endif
    }
    TEST("{Declare 1,  Use 1 Random} Cycle");
    {
        Regs regs;
        SafeSym lots[80];
        srand(1234567);
        for(int i=0; i<80; ++i){
            lots[i] = regs.declare(REG_T(USED|SCALAR), &boring_names[10*i]);
            cout<<'.'<<*lots[i]<<endl; cout.flush();
            if(i>1){
                int symb = rand() % i;
                // signal a WRITE to symb -- this allocates a reg is symb does not have one
                regs.out(lots[symb]);
                cout<<endl;
            }

        }
    }
}

int main(int,char**){
    Tester::test_construct();
    cout<<"\nGoodbye - ran "<<testNum<<" tests"<<endl;
    return 0;
}
