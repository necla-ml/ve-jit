/** \file test register load optimizations.
 * Full rebuild, test, and compile single blob of long tests that
 * take test case number IN[%s0] and return ERR[%s2] and load
 * that tests target constant into %s3[OUT].  ERR is set if
 * the IN value is out of range, o/w tmp_veli_loadreg_big.S
 * executes a test by jumping to the IN'th 3-instruction block.
 * The 3-insttruction blocks consist of one or two ops to
 * load the target value into OUT, and a return op.
 *
 * The tests, and code generation do not need to run on the VE,
 * so in the following we use the *-x86 binary.  I/O-heavy jobs,
 * and invoking system commands from VE tends to be a bit slow.
 *
 * In the following example, replace -J with -j if you do
 * not have access to ncc tools (nas, nobjcopy, nobjdump,...)
 * ```
 * make force; make veli_loadreg-x86 && \
 * { ./veli_loadreg-x86 -a >& vl.log; \
 *   rm -f tmp_veli_loadreg_big.* && \
 *   { ./veli_loadreg-x86 -h; ./veli_loadreg-x86 -a; } >& vl-a.log \
 *   ./veli_loadreg-x86 -J >& vl-j.log \
 *   && make -f bin.mk tmp_veli_loadreg_big.bin \
 *   && ls -l tmp_veli_loadreg_big.*; }
 * ```
 */

#include "velogic.hpp"
#include "asmfmt.hpp"
#include "codegenasm.hpp"
#include "jitpage.hpp"      // New: -R (run) option

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
//#include <unordered_set>
#include <set>
#include <vector>
#include <array>

#define STR0(...) #__VA_ARGS__
#define STR(...) STR0(__VA_ARGS__)

using namespace std;

// I began with unordered_set, but it is vital to reproduce the order, so
typedef std::set<uint64_t> Set;

Set init_mvals(int const verbose=1){
    Set mvals;
    {
        // add all r'0+1+' mvals and their complements
        if(verbose) cout<<hex<<setfill('0')<<endl;
        for(uint64_t i=0, mval=0; i<64; mval=(mval<<1U)+1U, ++i ){
            mvals.insert( mval); // mval = 0... 1...
            mvals.insert(~mval); //~mval = 1... 0...
            if(verbose)
                cout<<setw(2)<<dec<<setfill('0')<<i<<hex<<" 0x"<<setw(16)<<mval
                    <<"   "<<setw(16)<<~mval
                    <<"   "<<dec<<setw(3)<<setfill(' ')<<-(int64_t)i<<"=0x"<<hex<<setw(16)<<-i
                    <<endl;
        }
        if(verbose) cout<<dec<<setfill(' ');
    }
    return mvals;
}

void print_mvals(){
    init_mvals(1);
}

Set init_tvals(){
    Set targ; // target for various Mval,Ival PairS
    Set mvals = init_mvals(0);
    cout<<"\n Using "<<mvals.size()<<" mvals for binary-op targets"<<endl;
    uint64_t const one=1U;
    uint64_t mval;
    uint64_t ival;
    // generate a set of target around exact formula values
    for(int64_t ival = -64; ival<=63; ++ival){
        uint64_t const i = (uint64_t)ival;
        for(auto const m: mvals){
            std::array<uint64_t,8> exact =
            { i&m, i|m, i^m, ~(i&m), 
                i+m, i-m, (uint64_t)((int64_t)i-(int64_t)m), m<<(int)(i&0xf) };
            for(auto const e: exact){
                targ.insert( e );
                targ.insert( e+1U );
                targ.insert( e-1U );
            }
        }
    }
    cout<<" generated "<<targ.size()<<" nice targets"<<endl;
    // generate all [1,2]-bit patterns and complements
    for(int i=0; i<64; ++i){
        targ.insert(  ((uint64_t{1}<<i)  ) );
        targ.insert( ~((uint64_t{1}<<i)  ) );
        targ.insert(  ((uint64_t{0x3}<<i)) );
        targ.insert( ~((uint64_t{0x3}<<i)) );
    }
    // generate all 'or's of 4 consecutive 1's
    for(int i=0; i<60; ++i)
        for(int j=i+1; j<60; ++j)
            targ.insert( (uint64_t{1}<<i) | (uint64_t{1}<<j) );
    cout<<" now have "<<targ.size()<<", after simple bit patterns"<<endl;
    // make sure to cover full range [-1024,+1024]
    for(int64_t i=-1024; i<=1024; ++i){
        targ.insert((uint64_t)i);
    }
    cout<<" now have "<<targ.size()<<", after adding [-1024,1024]"<<endl;
    // Using 128 mvals for binary-op targets
    // generated 16299 nice targets
    // now have 17726, after simple bit patterns
    // now have 18726, after adding [-1024,1024]
    //  conclusion:   most >7bit values will go be lea (up to 2^31),
    //                and after, only a tiny fraction have 1-op load.
    //     2-op loads could search first for different
    //     instruction type for the first "lea" (lo 32bit) load.
    return targ;
}
/**construct a program that assembles the snippets into
 * a simple const-size \e switch-case test, where input
 * in range 0..tvals.size()-1 returns tvals[i] loaded
 * into %s3 (OUT) */
template<typename SET>
std::string getBigTest(SET const& tvals){
    // see veliFoo.cpp for suggestions about macros
    AsmFmtCols prog("");
    prog.lcom(STR0(__FUNCTION__) " (tvals.size() = "+jitdec(tvals.size()));
    string func("LRBIG");
    OpLoadregStrings ops;
    prog
        // we use labels...
        .def("STR0(...)", "#__VA_ARGS__")
        .def("STR(...)",  "STR0(__VA_ARGS__)")
        .def("CAT(X,Y)", "X##Y")
        .def("FN",func)             // func name (characters)
        .def("FNS", "\""+func+"\"") // quoted-string func name
        .def("L(X)", "CAT("+func+"_,X)")
        .def("SI","%s0",            "input value")
        .def("BP","%s1",            "base JitPage address")
        // macros for relocatable branching
        .def("REL(YOURLABEL)", "L(YOURLABEL)-L(BASE)(,BP)", "relocatable address of 'here' data")
        // The following is needed if you use branches
        .ins("sic   BP","%s1 is *_BASE, used as base ptr")
        .lab("L(BASE)")     // this label is reserved
        // Here is an example branch
        //.ins("b REL(USELESS)","demonstrate a useless branch")
        //.lab("L(USELESS)")
        .ins() // blank line
        .com("SNIPPET START");
    prog.def("INP", "%s0", "input: 0 <= test case number < tvals.size()")
        // unused .def("MAXIN", "%s1")
        .def("ERR", "%s2")
        .def("OUT","%s3",           "output value")
        .def("OTHER","%s4",         "other output")
        .def("T0","%s40");
#define WORKING 6 // OK, after ? incremental steps I think the bugs were fixed :(
#if WORKING==0
    // debug: return ERR=OUT=666 always
    prog.ins("lea ERR,666");
    prog.ins("lea OUT,666");
    prog.ins("b.l (,%lr)", "return without doing anything useful");
#elif WORKING==1
    // debug: return ERR=OUT=-1 if IN is negative (test branches) else +1
    prog.ins("lea ERR,-1");
    prog.ins("lea OUT,-1");
    prog.ins("blt.l INP, (,%lr)", "conditional return");
    prog.ins("lea ERR,+1");
    prog.ins("lea OUT,+1");
    prog.ins("b.l (,%lr)", "unconditional return");
#elif WORKING==2 // same but now using a relative address
    // debug: return ERR=OUT=-1 if IN is negative (test branches) else +1
    prog.ins("bge.l INP, REL(OK1)");
    prog.ins("lea ERR,-1");
    prog.ins("lea OUT,-1");
    prog.ins("b.l (,%lr)", "unconditional return");
    prog.lab("L(OK1)");
    prog.ins("lea ERR,+1");
    prog.ins("lea OUT,+1");
    prog.ins("b.l (,%lr)", "unconditional return");
#elif WORKING==3 // test 0 <= INP < tvals.size() (ret -1 | 0 | +1 only)
    // ve BC, BCS, BCF load **absolute** address into IC
    prog.ins("bge.l INP, REL(OK1)", "jump ABSOLUTE")
        .ins("lea ERR,-1")
        .ins("lea OUT,-1")
        .ins("b.l (,%lr)", "unconditional return")
        .lab("L(OK1)")
        .ins();
    ops = opLoadregStrings( tvals.size() ); // all these set value of OUT, maybe use T0
    assert(!ops.lea.empty());
    prog.ins(ops.lea,"load tvals.size()="+jitdec(tvals.size()))
        // ve BCR op **adds** an offset to current IC
        // nas replace L(OK2) with it relative address automatically.
        .ins("brlt.l.t INP,OUT, L(OK2)","test and jump RELATIVE")
        .ins("lea ERR,+1")
        .ins("lea OUT,+1")
        .ins("b.l (,%lr)", "error 2, INP test case too high")
        .lab("L(OK2)")
        .ins();
    prog.ins("lea ERR,0")
        .ins("lea OUT,0")
        .ins("b.l (,%lr)", "error 2, INP test case too high")
        .ins();
#else // WORKING>=4
    // defines, comments done...
    prog.ins("bge.l INP, REL(OK1)")
        .ins("lea ERR,-1")
        .ins("lea OUT,0")
        .ins("b.l (,%lr)", "unconditional return")
        .lab("L(OK1)")
        .ins();
    ops = opLoadregStrings( tvals.size() ); // all these set value of OUT, maybe use T0
    assert(!ops.lea.empty());
    prog.ins(ops.lea,"load tvals.size()="+jitdec(tvals.size()))
        .ins("brlt.l.t INP,OUT, L(OK2)","test case < tvals.size()?")
        .ins("lea ERR,+1")
        .ins("lea OUT,0")
        .ins("b.l (,%lr)", "error 2, INP test case too high")
        .lab("L(OK2)")
        .ins();
    // INP was in open range [ 0, tvals.size() )
    prog.ins("lea ERR,0")
        .ins("lea OUT,0")
        .ins();
    //
    //  each case is like:
    //     jit code to load %s3 with tvals[i]
    //     b.l (,%lr) # return
    //     if( jit code was a single-instruction ) NOP
    //
    //  so jump addr is CASE0 + INP * 24
    prog.lcom("JUMP TABLE for FIXED-SIZE code entries")
        .ins("lea T0,24")
        .ins("mulu.l T0, INP,T0", "byte offset of IN'th test");
        // branch can only accept <disp>(,<reg>)  "AS" format, so
        // we first calculate the absolute address into T0 (ASX format ok for LEA)
        // and then branch to that absolute address
#if WORKING==4 // return the absolute address only (do not branch into the table)
    prog..ins("lea OTHER, L(CASE_0)-L(BASE)(,BP)", "calc absolute address of 0'th test")
        .ins("addu.l OTHER,OTHER,T0", "absolute address of test code")
        // oh. nas says can't use index AND displacement ??
        .ins("b.l (,%lr)", "unconditional return")
        .lab("L(CASE_0)")
        .lcom("never get here")
        .ins();
#elif WORKING >= 5
    //
    // WORKING >= 5 will attempt the "switch" via computed-goto ...
    // actually full ASX format IS allowed for bsic !!!
    //
    prog
        .ins("lea OTHER, L(CASE_0)-L(BASE)(,BP)", "calc absolute address of 0'th test")
        .rcom("OTHER=addr of CASE_0")
        .ins("addu.l OTHER,OTHER,T0", "absolute address of test code")
        .rcom("OTHER=addr of INP'th test case")
        // oh. nas says can't use index AND displacement ??
        //.ins("bsic.l OTHER, 8(T0,OTHER)", "branch to absolute address,")
        .ins()
        .ins("b.l (,OTHER)", "branch to absolute address,")
        .ins("nop") // check the generated asm code...
        .ins(" nop")
        .ins(" b.l (,%lr)", "test op field with leading space")
        // note that if you remember the OTHER return value, you can
        // branch directly to that case again.
        .lab("L(CASE_0)");
    //
    // Now populate tvals.size() test cases to load OUT with some value.
    //
#if WORKING==5
    for(uint64_t i=0U; i<tvals.size(); ++i){
        // Test the switch-via-computed-go-to:
        //   every test case 'i' returns exactly 'i'.  Always.
        //   valid range is 31-bit, +ve, so just use 'lea' always.
        // opLoadregStrings(i) returns a choice of impls,
        ops = opLoadregStrings(i);  // all these set value of OUT, maybe use T0
        assert( !ops.lea.empty() );
        prog.ins( ops.lea, "return i="+jitdec(i)+" 1-op lea");
        prog.ins("b.l (,%lr)", "return from test case "+jitdec(i));
        prog.ins("nop");
        // call with INP(%s0) in range 0..tvals.size()
        // should return ERR(%s2) == 0 and OUT(%s3) == INP(%s0)
    }
#elif WORKING>=6
    int64_t i=0;
    // TODO: sort the tvals in increasing order, then loop...
    //enum {Lea, Log, Ari, Shl, Lea2};
    for(auto t: tvals){ // .. in some unknown REPRODUCIBLE order (it is a hash set)
        // Now (at long last) every test case will execute the "optimal"
        // way to load INP=tvals[i] into the OUT register.
        ops = opLoadregStrings(t);
        // If we wanted to use min ops we could use     prgiLoadreg  (returns 1 string)
        // favoring     lea > log > shl > ari > lea2
        // Here let's mix it up a bit more...
        string one_op = (
                    !ops.log.empty()? ops.log
                    : !ops.shl.empty()? ops.shl
                    : !ops.lea.empty()? ops.lea
                    : !ops.ari.empty()? ops.ari
                    : string("") );
        if(!one_op.empty()){
            string comment = "case i="+jitdec(i)+" 1-op load of "+hexdec(t);
            cout<<comment<<endl;
            prog.ins()
                .com(comment)
                .ins( one_op )
                .ins("b.l (,%lr)", "return from case "+jitdec(i))
                .ins("nop");
        }else{
            string two_op = ops.lea2;
            assert( !two_op.empty() );
            string comment = "case i="+jitdec(i)+" 2-op load of "+hexdec(t);
            cout<<comment<<endl;
            prog.ins()
                .com(comment)
                .ins( two_op )
                .ins("b.l (,%lr)", "return from case "+jitdec(i));
        }
        ++i;
    }
#else
#error "WORKING test case not implemented?"
#endif
#endif
#endif
    //

    prog.com("SNIPPET END")
        .ins("b.l (,%lr)",          "return (no epilogue)")
        .ins()
        // here is an example of storing some const data in the executable page
        .lab("L(DATA)")
        .lab("L(szstring)")
        .ins(".byte L(strend) - L(strstart)", "asciz data length")
        .lab("L(strstart)")
        .ins(".ascii FNS")
        .lab("L(strend)")
        .ins(".ascii \""+jitdec(WORKING)+"\"", "WORKING")
        .undef("FN").undef("FNS").undef("L")
        //.ins(".align 3" "; " L(more) ":", "if you want more code")
        ;
    std::string program = prog.flush();
    if(1){ // verbose ?
        cout<<__FUNCTION__<<" --> program:\n"
            <<program<<endl;
    }
    return program;

}
#if defined(__ve)
//
// wrpiLoadregBit was thrown into wrpiFoo.cpp
// because that file gets compiled *without* -std=c++11.
// With -std=c++11, nc++ will not accept extended asm,
// so 'wrp_veli_loadreg_big' is found in wrpiFoo.cpp instead of here.
#endif
int main(int argc,char** argv)
{
    int a=0, opt_a=1, opt_h=0, opt_j=0, opt_r=0;
    if(argc > 1){
        for( ; a+1<argc && argv[a+1][0]=='-'; ++a){
            char *c = &argv[1][1];
            for( ; *c != '\0'; ++c){
                if(*c=='h'){
                    cout<<" "<<argv[0]<<" [-h|a|j|J]"<<endl;
                    cout<<" Purpose: test VE loadreg logic\n"
                        "          quick load of any known value into a scalar register\n"
                        "          loops over interesting cases (~19k tests)\n"
                        "  default: run logic tests,\n"
                        "  -a    assembler output + logic tests\n"
                        "Alt run mode:\n"
                        "  -j    jit code that takes as input a test#, and does a switch() in asm\n"
                        "                 --> tmp_veli_loadreg_big.S\n"
                        "  -J    jit code AND use ncc tools to create VE binary blob\n"
                        "                 --> tmp_veli_loadreg_big.bin\n"
                        "            (does 'make -f bin.mk tmp_veli_loadreg_big.bin')\n"
                        "            Note that the .bin is ~ 450k bytes, so to load it\n"
                        "            you may need to mmap more than a single page of memory\n"
                        "Alt run mode:\n"
                        "  -R    Run it: only if running on the VE, we load the big .bin file\n"
                        "        run all the test cases, and check the output correctness.\n"
                        "        If the .bin file already exists, we do not recreate it.\n"
                        "  -h    this help"
                        <<endl;
                    opt_h = 1;
                }else if(*c=='a'){ ++opt_a;
                }else if(*c=='j'){ ++opt_j;
                }else if(*c=='J'){ opt_j+=2;
                }else if(*c=='R'){ ++opt_r;
                }
            }
        }
    }
    cout<<" args: opt_a="<<opt_a<<", opt_j="<<opt_j<<", opt_r="<<opt_r<<endl;
    if( opt_h )
        return 0;

    auto const tvals = init_tvals();
    cout<<"=== init_tvals() DONE"<<endl;
    char const* const tmp_base="tmp_veli_loadreg_big";
#if defined(__ve)
    if(opt_r){
        cout<<" -R [Run] option..."<<endl;
        //system((string("ls -l ")+tmp_base+".bin").c_str());
        cout<<" loading JitPage..."<<endl; cout.flush();
        JitPage jp={nullptr,0,2};
        cout<<jp<<endl; cout.flush();
        int const verbose_jit = 9;
        if(bin2jitpage(tmp_base, &jp, verbose_jit)){
            cout<<jp<<endl; cout.flush();
            cout<<" Marking as read+exec..."<<endl; cout.flush();
            jitpage_readexec(&jp);
            cout<<setw(80)<<setfill('-')<<""<<setfill(' ')<<endl;
            cout<<" Running tests... WORKING = "<<WORKING<<endl; cout.flush();
#if WORKING==0
            for(int64_t t=-1; t<=2; ++t){
                cout<<" test "<<t<<" HERE"<<endl; cout.flush();
                VeliErr const e = wrpiLoadregBig(&jp, t);
                cout<<" RETURNED! "<<e<<endl; cout.flush();
                assert( e.i == t );
                assert( e.error == 666 );
                assert( e.output == 666 );
            }
#elif WORKING==1 || WORKING==2 // return -1 if IN<0, else +1 (2 adds a relative branch)
            for(int64_t t= -2; t<=2; ++t){
                cout<<" test "<<t<<" HERE"<<endl; cout.flush();
                VeliErr const e = wrpiLoadregBig(&jp, t);
                cout<<" RETURNED!"<<e<<endl; cout.flush();
                uint64_t const expect = (t<0? ~0ULL: 1ULL);
                assert( e.i == t );
                assert( e.error == expect );
                assert( e.output == expect );
            }
#elif WORKING==3 // return -1 | 0 | +1 according to IN wrt [0,tvals.size()) range
            // Ahaa. my original failed to use the REL macro for relative jumps!
            int64_t mx = tvals.size();
            for(int64_t t: std::array<int64_t,8>({-2,-2,0,1,mx-2,mx-1,mx,mx+1})){
                cout<<" test "<<t<<" HERE"<<endl; cout.flush();
                VeliErr const e = wrpiLoadregBig(&jp, t);
                cout<<" RETURNED!"<<e<<endl; cout.flush();
                uint64_t const expect = (t<0? ~0ULL: t<tvals.size()? 0ULL: 1ULL);
                assert( e.i == t );
                assert( e.error == expect );
                assert( e.output == expect );
            }
#elif WORKING==4 // return -1 | abs address | +1 according to IN wrt [0,tvals.size()) range
            // Ahaa. my original failed to use the REL macro for relative jumps!
            int64_t mx = tvals.size();
            VeliErr prev={1,1,1,1}; // some error case
            for(int64_t t: std::array<int64_t,10>({-2,-2,0,1,2,mx-3,mx-2,mx-1,mx,mx+1})){
                cout<<" test "<<t<<" HERE"<<endl; cout.flush();
                VeliErr const e = wrpiLoadregBig(&jp, t);
                cout<<" RETURNED!"<<e<<endl; cout.flush();
                assert( e.i == t );
                uint64_t const error = (t<0? ~0ULL: t<tvals.size()? 0ULL: 1ULL);
                if(error){ assert( e.error == error ); assert( e.output == 0 ); }
                else{
                    assert( e.error == 0 );
                    assert( e.output == 0 );
                    // e.other is absolute address for test t
                }
                if( e.error == 0 && prev.error == 0 && e.i == prev.i + 1 ){
                    assert( e.other == prev.other + 24 );
                    // The "switch" in JitPage computes addresses based on 3 ops
                    // per case, and every VE op is 8 bytes of machine code.
                    // (The ops are 1--2 ops for OUT, 1 op for return and 1--0 NOP)
                }
            }
#elif WORKING==5 // valid test case 'i' returns exactly 'i' (lea only, no special ops)
            // here the goal is execute the correct computed goto,
            // ...and return.  Every goto just loads the test number as output.
            for(int64_t i=0; i<tvals.size(); ++i){
                cout<<" test "<<i<<", input "<<i<<" HERE"<<endl; cout.flush();
                VeliErr const e = wrpiLoadregBig(&jp, i);
                cout<<"      result "<<e<<endl; cout.flush();
                assert( e.i == i );
                assert( e.error == 0 );
                assert( e.output == i );
            }
#elif WORKING>=6
            int64_t i=0;
            // TODO: sort the tvals in increasing order, then loop...
            for(auto t: tvals){ // .. in some unknown REPRODUCIBLE order (it is a hash set)
                cout<<" test "<<i<<", input ith tval="<<hexdec(t)<<" HERE"<<endl;
                //
                // for input i=0.1,..., output is t [i'th item in tvals]
                VeliErr const e = wrpiLoadregBig(&jp, i);
                cout<<"      result "<<e<<endl;
                assert( e.i == i );
                assert( e.error == 0 );
                // The i'th test case should load tvals[i] into a register:
                assert( e.output == t );
                ++i;
            }
#endif
        }else{
            cout<<" Problems encountered loading binary blob file "<<tmp_base<<".bin"<<endl;
            cout.flush();
            return 1;
        }
        jitpage_free(&jp);
        opt_r=0;
        opt_j=0;
        opt_a=0;
    }
#endif
    cout<<" args: opt_a="<<opt_a<<", opt_j="<<opt_j<<", opt_r="<<opt_r<<endl;
    if(opt_r){
#if !defined(__ve)
        cout<<" We are not compiled on VE, so we won't load the VE jit code."<<endl;
#endif
        cout<<" (reverting to -J option to try to [re]create the .bin file)"<<endl;
        opt_j=2;
    }
    cout<<" args: opt_a="<<opt_a<<", opt_j="<<opt_j<<", opt_r="<<opt_r<<endl;
    if(opt_j){
        cout<<" -"<<(opt_j==1?"j [jit]":"J [jit+nas]")<<" option..."<<endl;
        auto program = getBigTest(tvals); // cpp+nas code for switch(test_number) loadreg cases
        std::ofstream fout(tmp_base+string(".S"));
        fout<<program;
        fout.close();
        cout<<"Wrote huge test case program to tmp_veli_loadreg_big.S"<<endl;
        if(opt_j>1){
            // here is one way to make the VE binary blob.
            // this also produces some other files (.asm,.o,.bin,.dump)
            system("make -f bin.mk tmp_veli_loadreg_big.bin");
        }
        opt_a=1;
    }
    if(opt_a) {
        cout<<" simulated logic tests for loadreg"<<endl;
        print_mvals(); // reference list of mvals, used internally by init_tvals
        for(auto t: tvals){
            VeliErr e = { 7, 7, 7 };
            e = veliLoadreg(t /*,count=1U*/);
            cout<<"\n------------\n"
                <<"veliLoadreg("<<t<<" "<<e.i<<") --> "
                <<" error="<<e.error
                <<" other="<<e.other
                <<endl;
            if(opt_a){
                string code = prgiLoadreg(t);
            }
        }
        cout<<"\n All "<<tvals.size()<<" coverage tests PASSED";
    }
    cout<<"\nGoodbye"<<endl;

}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
