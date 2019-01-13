/** \file test logic correctness for "loadreg" optimizations */
#include "velogic.hpp"
#include "asmfmt.hpp"
#include "codegenasm.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#define STR0(...) #__VA_ARGS__
#define STR(...) STR0(__VA_ARGS__)

/** \return fixed length debug string for numeric values */
template<typename Name> inline std::string
HexDec64( Name const name, uint64_t value ){
    using namespace std;
    ostringstream oss;
    oss<<right<<setw(20)<<name
        <<" 0x"<<hex<<setfill('0')<<setw(16)<<value<<setfill(' ')<<dec;
    bool isi = isIval(value), ism = isMval(value);
    oss<<left;
    if( isi ) oss<<" I "<<setw(3)<<(int)value;
    else      oss<<setw(6)<<"";
    if( ism ) oss<<" M "<<setw(6)<<jitimm(value);
    else      oss<<setw(9)<<"";
    if( !isi && !ism )
        oss<<" "<<left<<setw(21)<<value; // decimal
    return oss.str();
}

#define HEXDEC(STR,INT) do{ uint64_t i=(INT); \
    cout<<HexDec64(STR,i)<<endl; \
}while(0)

#define HD(...) HEXDEC( #__VA_ARGS__, (__VA_ARGS__) )

using namespace std;

#if 1 // empty examples
std::string prgiFoo(uint64_t start)
{
    int kase = 0; // for simple stuff, don't need execution path machinery.
    string program;
    AsmFmtCols prog("");
    prog.lcom(STR(__FUNCTION__) " (i="+jithex(start)+" = "+jitdec(start));
    string func("Foo");
    if(1){ // if you need relative addressing ...
        prog
#if 0
            .def("STR0(...)", "#__VA_ARGS__")
            .def("STR(...)",  "STR0(__VA_ARGS__)")
            .def("CAT(X,Y)", "X##Y")
            .def("FN",func)             // func name (characters)
            .def("FNS", "\""+func+"\"") // quoted-string func name
            .def("L(X)", "CAT("+func+"_,X)")
            .def("SI","%s0",            "input value")
            .def("ALG","%s1",           "alg modifier [rarely]")
            .def("BP","%s2",            "internal base pointer for const 'here' data")
#endif
            // prgiFoo tests have real outputs:
            .def("OUT","%s3",           "output value")
            .def("OUT2","%s4",          "2nd output [sometimes]")
            .def("VOUT","%v0",          "vector outputs")
            .def("VOUT2","%v1",         " Ex. loop_init(ii,jj)-->vl + 2 vectors?")
            // veliFoo tests have 
            //.def("SE","%s2", "error output")
            //.def("SO","%s3", "other output (code path? secondary output?)")
            // all can use tmp scalar registers (that are in VECLOBBER list of wrpiFoo.cpp) 
            .def("T0","%s40")
            .def("T1","%s41") // etc
            // and some vector registers (also in VECLOBBER?)
            .def("V1","%V1")  // etc
            // macros for relocatable branching
            .def("REL(YOURLABEL)", "L(YOURLABEL) - L(BASE)(,BP)", "relocatable address of 'here' data")
            // The following is needed if you use branches
            .ins("sic   BP","%s1 is icbase, used as base ptr")
            .lab("L(BASE)")     // this label is reserved
            // Here is an example branch
            .ins("b REL(USELESS)","demonstrate a useless branch")
            .lab("L(USELESS)")
            .ins() // blank line
            .com("SNIPPET START")
            .com("YOUR CODE")
            // add more core code to prog, %s0 is the input, output|error-->%s2,...
            .ins()
            .com("or OUT, 0,(0)1",      "prgIFoo output")
            //.com("or " SE ",0,... veliFoo logic test error detected")
            //
            // and return to caller, "parm has been loaded in %s0"
            //
            .com("SNIPPET END")
            .ins("b.l (,%lr)",          "return (no epilogue)")
            .ins()
            // here is an example of storing some const data in the executable page
            .lab("L(DATA)")
            .lab("L(szstring)")
            .ins(".byte L(strend) - L(strstart)", "asciz data length")
            .lab("L(strstart)")
            .ins(".ascii FNS")
            .lab("L(strend)")
            .undef("FN").undef("FNS").undef("L")
            //.ins(".align 3" "; " L(more) ":", "if you want more code")
            ;
        program = prog.flush();
    }
    if(1){ // verbose ?
        cout<<__FUNCTION__<<" --> program:\n"
            <<program.substr(0,4096)<<endl;
    }
    return program;
}
VeliErr     veliFoo(uint64_t start, uint64_t count){
    uint64_t end = start + count; // ok if wraps
    for(uint64_t in=start; in!=end; ++in){
        // same logic as prgiFoo, but NO 'prog' operations
        // i.e. the **logic** of YOUR CODE
    }
}
#endif // empty sample templates

// first define prgiFoo (it has *more* code), then prune it to create veliFoo.
VeliErr     veliLoadreg(uint64_t start, uint64_t count/*=1U*/)
{
    uint64_t kase = 0; // for simple stuff, don't need execution path machinery.
    VeliErr ret;
    ret.i = start;
    ret.error = 0; // set via KASE macro, if condition fails
    ret.other = 0;
    int const v = 1; // verbose
    //string program;
    //
    // program ~ jit subroutine to load 'parm' into '%s0'
    //
    uint64_t const parm = start; // a const version
    //
    // break up parm into <hi,lo> 32-bit words
    //
    uint32_t const hi = ((parm >> 32) & 0xffffFFFF); // unsigned shift-right
    uint32_t const lo = (uint32_t)parm;              // 32 lsbs (trunc)
    uint64_t const lo7 = (parm & 0x7f);              // 7 lsbs (trunc)
    int64_t  const sext7 = (int64_t)(lo7<<57) >> 57; // 7 lsbs (trunc, sign-extend)
    // OLD int64_t const sext7 = ((lo7&0x40)==0x40? lo7|~0x7f: lo7);
    //     assert( sext7 == (int64_t)(lo7<<57) >> 57 );
    //
    // extras (try to keep min number of requirements for asm code)
    // 1-op possibilities depend on range of parm.
    bool isI = (int64_t)parm >= -64 && (int64_t)parm <= 63; // I : 7-bit immediate
    //             so if(isI), then sext7 is the integer "I" value
    bool isM = isMval(parm); // M : (m)B 0<=m<=63 B=0|1 "m high B's followed by not-B's"
    assert( isI == ((int64_t)parm == sext7) ); // equiv condition
    assert( lo7 >= 0 );
    //assert( isM == (popcount(parm) == nTrailOnes(parm)) || popcount(parm) == nLeadingOnes(parm) );
    //
    //

    // lea possibilities (kase+=ones)
    string reg="%s0";
    //string lea;
#define KASEDBG do \
    { \
        cout<<" kase "<<where<<" isI="<<isI<<" isM="<<isM<<" " STR(__FUNCTION__)<<endl; \
        HD(parm); HD(hi); HD(lo); HD(sext7); \
    }while(0)
#define HAVE(WHERE) ((kase& (uint64_t{1}<<(WHERE))) != 0)
    /** requires variables: int v; VeliErr ret.
     * \tparm STR is << sequence like: " The value is "<<a<<" I think." */
#define KASE(WHERE,STR,...) do \
    { \
        uint64_t const where = (WHERE); \
        if(v) cout<<" @"<<where<<" "<<STR; \
        kase += uint64_t{1} << where; \
        try { \
            if( !(__VA_ARGS__) ){ \
                /* failed condition */ \
                if( ret.error == 0 ) ret.error = where; \
                KASEDBG; \
                cout<<" Failure "<<STR<<" kase "<<where<<" "<<__FUNCTION__<<":"<<__LINE__; \
                cout<<endl; cout.flush(); \
                THROW("Failure "<<STR<<" kase "<<where); \
            } \
        } catch (...) { /**/ } \
        cout<<endl; \
    } while(0)
    { // lea logic
        bool is31bit = ( (parm&0x8fffFFFF) == parm );
        bool is32bit = ( (parm&0xffffFFFF) == parm );
        bool hiOnes  = ( (int)hi == -1 );
        if(v){ HD(hi); HD(lo);
            cout<<" is31bit="<<is31bit<<" is32bit="<<is32bit<<" hiOnes="<<hiOnes<<endl;
        }
        if(is31bit){
            KASE(1,"lea 31-bit",parm == (parm&0x3fffFFFF));
            //lea="lea "+reg+","+jithex(lo)+"# load: 31-bit";
        }else if((int)lo < 0 && hiOnes){
            assert( (int64_t)parm < 0 ); assert((int)lo < 0);
            KASE(2,"lea 32-bit -ve",parm == (uint64_t)(int64_t)(int32_t)lo);
            //lea="lea "+reg+","+jithex(lo)+"# load: sext(32-bit)";
        }else if(lo==0){
            KASE(3,"lea.sl hi only",lo == 0);
            //lea="lea.sl "+reg+","+jithex(hi)+"# load: hi-only";
        }
        // (more lea kases below, in 2-op section)
        //if(!lea.empty()) lea.append("->"+jithex(parm)+" = "+jitdec(parm));
    }

    if(v){
        HD(parm);
        //uint64_t const lo6 = (parm & 0x3f);
        //cout<<" lo6  =0x"<<hex<<lo6  <<dec<<" = "<<lo6;
        cout<<" sext7=0x"<<hex<<sext7<<dec<<" = "<<sext7;
        cout<<"     lo7  =0x"<<hex<<lo7  <<dec<<" = "<<lo7;
        cout<<"     bit6 =  "<<(parm&(1<<6))<<endl; // the msb of 7-bit lo7
    }
    // logical possibilities, from simple to more complex (kase+=tens)
    string log;
    // M   1... 0... |0000000|
    // I   1... 1...  1xxxxxx          I = sext7 < 0
    // or  1... 1...  1xxxxxx  1 isI   // or I,(0)1 sext7<0 && ((parm&~0x3f)==~0x3f)
    //also 0... 0...  0??????
    if(isI){
        KASE(4,"or OUT,"+jitdec(sext7)+",(0)1", parm==sext7 && isIval(parm));
        //log="or "+reg+","+jitdec(sext7)+",(0)1 # load: small I";
    }
    if(isM){
        KASE(5,"or OUT,0,"+jitimm(parm), isMval(parm));
        log="or "+reg+",0,"+jitimm(parm)+"# load: (M){0|1}";
    }
    { // search for logical combination of I and M values
        // Here I might go through more possibilities than strictly required
        // in order to see how few separate cases need be considered.
        // VE has quite a few logic ops... How do they operate?
        //
        // set or reset 7 LSBs to zero, and check if that is an M value
        // alt. check properties of sign-extending the lowest 7 bits
        // M   1... 0... |0000000|
        // I   1... 1...  1xxxxxx      I = sext7 < 0
        // xor 0... 1...  1xxxxxx      imm M = (~0x3f & ~xor) ***
        // eqv 1... 0...  0yyyyyy      imm M = ~sext7 | (~0x3f|eqv), I = ~sext7 ***
        // and 0... 0...  0000000       // or 0,(0)1
        // or  1... 1...  1xxxxxx  1 isI   // or I,(0)1 sext7<0 && ((parm&~0x3f)==~0x3f)
        //
        // M   1... 0... |0000000|
        // I   0... 0...  0xx10xx      I = sext7 > 0
        // or  1... 0...  0xx10xx  A** M = (~0x3f & or) ***
        // and 0... 0...  0000000       // or 0,(0)1
        // xor 1... 0...  0yy01yy      M = (~0x3f & xor), I = ~sext7 ***
        // eqv 0... 0...  0xx10xx       // or I,(0)1  sext7>0 && parm==sext7
        //
        // M   0... 1... |1111111|
        // I   0... 0...  0xx10xx
        // eqv 1... 0...  0xx10xx      imm M = 0x3f | (~0x3f | ~eqv)
        // xor 0... 1...  1yy01yy  B** imm M = 0x3f | (~0x3f | xor) vs xor sext7<0
        // and 0... 0...  0xx10xx  0 isI  M = -1 = (0)0
        //
        // M   0... 1... |1111111|
        // I   1... 1...  1xx10xx
        // xor 1... 0...  0yy01yy
        // eqv 0... 1...  1xx10xx
        // and 0... 1...  1xx10xx
        // or  1... 1...  1111111

        // A---------------------------
        // M   1... 0... |0000000|
        // I   0... 0...  0xx10xx      I = lo7 (trunc)
        // or  1... 0...  0xx10xx  A** M = (~0x3f & or) ***
        //     ------------------------
        // I   1... 1...  1xx10xx      I = lo7 (trunc)
        // or  1... 1...  1xx10xx  devolves to OR,I(sext7),M(0) [above]
        //if(sext7 >= 0 && isMval(parm&~0x3f)){
        if(isMval(parm&~0x3f)){
            //assert(sext7 > 0); holds if 'isI' and 'isM' tests preclude getting here
            // test for lo7 is convenient, but print I-value is sext7
            // (all higher bits are ???)
            uint64_t const ival = sext7;        // parm&0x3f (trunc)
            uint64_t const mval = parm&~0x3f;
            if(v){HD(ival); HD(mval);}
            KASE(6,"or OUT, parm&0x3f "+jithex(ival)+", parm&~0x3f "+jitimm(mval),
                    parm == (ival | mval) && isIval(ival) && isMval(mval));
            //log+="\n @or "+reg+","+jithex(ival)+","+jitimm(mval)+"# load: or(I,M)";
        }

        // B----------------------------
        // M   0... 1... |1111111|
        // I   0... 0...  0xx10xx
        // xor 0... 1...  1yy01yy  imm M = 0x3f | (~0x3f | xor) vs xor sext7<0
        //     ------------------------
        // I   1... 1...  1xx10xx
        // xor 1... 1...  0yy01yy (probably or,I,0 case, above)
        //  B**   isMval(parm&03f)
        // NO assert( isMval(parm|0x3f) == isMval(parm^sext7) ); // simpler alt test
        if(isMval(parm|0x3f)){
            assert( isMval(parm^sext7) ); // more general
            // If parm|0x3f = ....111111 is an Mval, it must be (m)0
            // and xor with an ival can 
            // equiv kase 60, but small I > 0
            uint64_t const ival = (~parm &0x3f);
            uint64_t const mval = parm|0x3f;
            if(v) {HD(ival); HD(mval); }
            // NO assert( sext7 >= 0 );
            //assert( sext7 < 0 ); holds if 'isI' and 'isM' tests preclude getting here
            //assert(sext7 > 0); holds if 'isI' and 'isM' tests preclude getting here
            KASE(7,"xor OUT, ~parm&0x3f "+jithex(ival)+", parm|0x3f "+jitimm(mval),
                    parm == (ival ^ mval) && isIval(ival) && isMval(mval));
            //log+="\n ^xor "+reg+","+jithex(ival)+","+jitimm(mval)+"# load: xor(I,M)";
        }
        // NO if(HAVE(7)) assert(HAVE(6));

        //----------------------------
        // M   1... 0... |0000000|
        // I   1... 1...  1xxxxxx      I = sext7 < 0
        // xor 0... 1...  1xxxxxx      imm M = (~0x3f & ~xor) ***
        // M   1... 0... |0000000|
        // I   0... 0...  0xx10xx      I = sext7 > 0
        // xor 1... 0...  0yy01yy      M = (~0x3f & xor), I = ~sext7 ***
        if(isMval(parm^sext7) ){ // xor rules don't depend on sign of lo7
            // if A = B^C, then B = A^C and C = B^A (Generally true)
            if(v)cout<<" 60kase "<<kase<<endl;
            int64_t const ival = sext7;
            uint64_t const mval = parm^sext7;
            if(v) {HD(ival); HD(mval); }
            KASE(9,"xor OUT, "+jithex(ival)+","+jitimm(mval),
                    parm == (ival ^ mval) && isIval(ival) && isMval(mval));
            assert( parm == (ival ^ mval) );
            //log+="\n #xor "+reg+","+jitdec(sext7)+","+jitimm(mval)+"# load: xor(I,M)";
        }
        // NO if(HAVE(9)) assert(HAVE(7));
        // NO if(HAVE(9)) assert(HAVE(55));
        if( HAVE(5) ) assert(HAVE(9)); // but 4 and 5 are the most readable (check first)
        if( HAVE(6) ) assert(HAVE(9)); // both (6)=OR and (9)=XOR same I,M
        if( HAVE(7) ) assert(HAVE(9)); // however (7) is more readable (check second)
        if( HAVE(9) ) assert( HAVE(7) || HAVE(6) ); // check last, pretty general.
        // Interesting:
        //    XOR 9 case is the only one that needs to be coded!

        // NO : if(HAVE(7)) assert(HAVE(7));
        // NO : if(HAVE(7)) assert(HAVE(7));
#if 0
        if(v)cout<<" parm|0x3f =0x"<<hex<<(parm|0x3f)<<dec<<" = "<<(parm|0x3f)<<endl;
        if(log.empty() && isMval(parm|0x3f)){ // 7th lsb == 1 to sign extend
            kase+=40;
            // AND with sext7 retains the upper bits and allows some variation in lower 6 bits
            // Ex. 0b1\+ 0\+ 1[01]{1,6}$        7th lsb always '1',
            // Ex. 0b0\+ 1\+ 1[01]{1,6}$        so sext7 has all higher bits set
            int64_t const ival = lo7;           // > 0
            uint64_t const mval = parm | 0x3f;
            if(v)cout<<" parm =0x"<<hex<<parm<<endl;
            if(v)cout<<" lo7  =0x"<<lo7<<endl;
            if(v)cout<<" sext7=0x"<<sext7<<endl;
            if(v)cout<<" mval =0x"<<mval<<dec<<endl;
            assert( parm == (ival & mval) );
            assert( sext7 == (0xffffFFFFffff80 & lo7) );
            log="xor "+reg+","+jitdec(ival)+","+jitimm(mval)+"# load: and(I<0,M)";
        }
#endif
        if(v)cout<<" parm&~0x3f =0x"<<hex<<(parm&~0x3f)<<dec<<" = "<<(parm&~0x3f)<<endl;
    }
    if(!log.empty()) log.append("->"+jithex(parm)+" = "+jitdec(parm));

    string ari; // kase+=hundreds
    //
    // start by checking immediate shifts.
    // Note that the immediate const is an M-value!
    // SLL %, Sy|M, Sz|N            (usually Sy takes an Ivalue)
    //
    // So on VE, SLL method generates bit pattern [0]+[1]+[0]+
    // or {some zeros}{some ones}{some zeros} up to 64-bits
    //
    // expect shift to be faster than add-sub, and multipy to be
    // too slow to compete with any 2-op approach
    //
    if(ari.empty()){
        // search for an unsigned right-shift that can regenerate parm
        int oksr=0;
        for(int sr=1; sr<64; ++sr){
            if( parm == (parm>>sr<<sr) && isMval(parm>>sr) ){
                oksr=sr; // use smallest shift
                break;
            }
        }
        if(oksr){
            uint64_t mval = parm >> oksr;
            if(v){HD(mval); HD(oksr);}
            KASE(19,"sll OUT,"+jitimm(mval)+","<<jitdec(oksr),
                    parm == (mval << oksr) && isMval(mval));
            //ari="sll "+reg+","+jitimm(parm>>oksr)+","+jitdec(oksr);
        }
    }
    // fast arith possibilities (add,sub) TODO (may cover variation in 8th bit, at least)
    // consider signed ops (easier)
    //if parm = I[-64,63] + M, then isMval( parm - I )  (brute force check?)
    //if parm = I[-64,63] - M, then M = I[-64,63] - parm (also covers M = parm+64?)
    // cover sext7 in 2 loops, because positive values are easier to read. (also prefer unsigned over signed sub)
    for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
        // P = I + M <===> M == P - I
        uint64_t const mval = parm - (uint64_t)ival;
        if( isMval(mval) ){
            if(v){HD(ival); HD(mval);}
            KASE(20,"addu.l OUT,"+jitdec(ival)+", "+jitimm(mval),
                    parm == ival + mval && isIval(ival) && isMval(mval));
            //ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
            break;
        }
    }
    for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
        uint64_t const mval = parm - (uint64_t)ival;
        if( isMval(mval) ){
            if(v){HD(ival); HD(mval);}
            KASE(21,"addu.l OUT,"+jitdec(ival)+","+jitimm(mval),
                    parm == ival + mval && isIval(ival) && isMval(mval));
            //ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
            break;
        }
    }
    // Q; Is it correct that SUB (unsigned subtract) still sign-extends the 7-bit Immediate in "Sy" field?
    for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
        // P = I - M <==> M = I - P
        uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
        if( isMval(mval) ){
            if(v){HD(ival); HD(mval);}
            uint64_t out = (uint64_t)ival - (uint64_t)mval;
            KASE(22,"subu.l OUT,"+jitdec(ival)+","+jitimm(mval),
                    parm == out  && isIval(ival) && isMval(mval));
            //ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
            break;
        }
    }
    for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
        uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
        if( isMval(mval) ){
            if(v){HD(ival); HD(mval);}
            uint64_t out = (uint64_t)ival - (uint64_t)mval;
            KASE(23,"subu.l OUT, "+jitdec(ival)+","+jitimm(mval),
                    parm == out  && isIval(ival) && isMval(mval));
            //ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
            break;
        }
    }
    // NO : if( HAVE(20)||HAVE(21) ) assert( HAVE(22)||HAVE(23) );
    // NO : if( HAVE(22)||HAVE(23) ) assert( HAVE(20)||HAVE(21) );
    // so add, sub cover disjoint cases
    if(HAVE(4)) assert(HAVE(20));
    if(HAVE(4)) assert(HAVE(22));
    if(HAVE(5)) assert(HAVE(20));
    // no if(HAVE(5)) assert(HAVE(22));
    if(HAVE(6)) assert(HAVE(20));               // YES
    // no! if(HAVE(20) || HAVE(21) || HAVE(22) || HAVE(23)) assert(HAVE(6));
    // no if(HAVE(20) || HAVE(21) || HAVE(22) || HAVE(23)) assert(HAVE(7));
    // no if(HAVE(6)) assert(HAVE(21));
    // no if(HAVE(6)) assert(HAVE(22));
    // no if(HAVE(6)) assert(HAVE(23));
    // no if(HAVE(7)) assert(HAVE(20));
    // no if(HAVE(7)) assert(HAVE(21));
    // no if(HAVE(7)) assert(HAVE(22));
    if(HAVE(7)) assert(HAVE(23));               // YES
    if(HAVE(9)) assert(HAVE(20) || HAVE(21) || HAVE(22) || HAVE(23));
    // no if(HAVE(20) || HAVE(21) || HAVE(22) || HAVE(23)) assert(HAVE(9)||HAVE(4)||HAVE(5));
    //    this means there are some ADD/SUB cases that have no or/xor solution.
    // do not check multiply, since it might take longer that 2-op sequences
    if(!ari.empty()) ari.append("->"+jithex(parm)+" = "+jitdec(parm));

    //bool const have_1op_load = !( lea.empty() && log.empty() && ari.empty() );

    //if( have_1op_load ){
    //    ;
    //}else
    //if(lo!=0)
    { // assert( !have_1op_load );
        //
        // NOTE: I should also create a 32-bit loadreg that exercises the different
        //       possible execution units.
        // Rationale:
        //       Instead of lea + lea.sl, I can use logic+load or arith+load or shift+load
        //       which mixes different instruction types.
        //
        //       In absence of preceding/following instruction-type context, this seems
        //       a good approach to on average distribute nearby instruction types
        //       among different execution units.
        //
        // extend lea case (previously used 1,2,3)
#if 1
        //
        // 'C' version of lea + lea.sl combo.
        //  Input: <hi,lo> words
        //  Output: Sx = <hi,lo>
        //  LEA operations:
        //    lea    :  Sx <-- Sy + Sz + sext(D,64)
        //    lea.sl :  Sx <-- Sy + Sz + (sext(D,64)<<32)
        // lea TMP, lo
        uint64_t tmplo = (int64_t)(int32_t)lo;
        // lea.sl OUT, <hi or hi+1> (,TMP)
        //        -ve lo will fill hi with ones i.e. -1
        //        so we add hi+1 to MSBs to restore desired sums
        uint64_t tmphi = ((int32_t)lo>=0? hi: hi+1);
        uint64_t tmp2 = tmphi << 32;    // (sext(D,64)<<32)
        uint64_t out = tmp2 + tmplo;     // lea.sl lea_out, tmphi(,lea_out);
        assert( parm == out );
#endif
        if((int32_t)lo >= 0){
            // simulate and check...
            uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-0
            uint64_t dd = /**/ hi /**/;
            uint64_t tmp2 = dd << 32;           // tmp2 = (sext(tmphi,64)<<32)
            uint64_t out = tmp2 + tmplo;
            KASE(30,"lea+lea.sl", parm == out );
            //lea="lea "+reg+","+jithex(lo)+"# lo>0"
            //    + ";lea.sl "+reg+","+jithex(hi)+"(,"+reg+") # load: 2-op";
        }else{ // sign extension means lea.sl of hi will have -1 added to it...
            // simulate and check...
            uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-1
            uint64_t dd = /**/ hi+1 /**/;       // hi+1 to counteract the all-1
            uint64_t tmp2 = dd << 32;           // tmp2 = (sext(tmphi,64)<<32)
            uint64_t out = tmp2 + tmplo;
            KASE(31,"lea+lea.sl", parm == out );
            //lea="lea "+reg+","+jithex(lo)+"# (int)lo<0"
            //    + ";lea.sl "+reg+","+jithex(hi+1U)+"(,"+reg+") # load: 2-op";
        }
        //lea.append("->"+jithex(parm)+" = "+jitdec(parm));
        // TODO: check for lea+or combos, etc.
        //prog.ins(lea);
    }
    //cout<<" lea: "<<lea<<endl;
    //cout<<" log: "<<log<<endl;
    //cout<<" ari: "<<ari<<endl;
    //
    // and return to caller, "parm has been loaded in %s0"
    //
#if 0
    prog.ins("b.l (,%lr)", "return");
    prog.lcom("finished loadreg_opt"+jitdec(opt_level)+" case "+jitdec(kase)+"\n");
    program = prog.flush();

    assert( program.size() < 4095 ); // will add a zero terminator
    char kernel[4096U];
    snprintf(kernel,4096,"%s%c",program.c_str(),'\0');
    printf("        kase=%d\n", kase);
    printf("Raw kernel string:\n%s\n-------------",&kernel[0]);
    return prog;
#endif
    cout<<" test "<<setw(20)<<parm<<" "<<setw(4)<<ret.error<<" "<<setw(4)<<ret.other<<" "<<ret.i<<endl;
    //
    // Now check for equivalent cases (some may be more readable)
    //
#undef HAVE
#undef KASE
#undef KASEDBG
    return ret;
}

/** suggested VE code string, ready to compile into an ABI=jit \ref JitPage */
std::string prgiLoadreg(uint64_t start)
{
    std::string func("LoadS"); // basename for labels, etc.
    uint64_t const parm = start;
    int kase = 0; // for simple stuff, don't need execution path machinery.
    string program;
    AsmFmtCols prog("");
    prog.lcom(func+"( i = "+jithex(start)+" = "+jitdec(start)+" )");
    uint32_t const hi = ((parm >> 32) & 0xffffFFFF); // unsigned shift-right
    uint32_t const lo = (uint32_t)parm;              // 32 lsbs (trunc)
    uint64_t const lo7 = (parm & 0x7f);              // 7 lsbs (trunc)
    int64_t  const sext7 = (int64_t)(lo7<<57) >> 57; // 7 lsbs (trunc, sign-extend)
    bool isI = (int64_t)parm >= -64 && (int64_t)parm <= 63; // I : 7-bit immediate
    bool isM = isMval(parm); // M : (m)B 0<=m<=63 B=0|1 "m high B's followed by not-B's"

    string lea;
    { // lea logic
        bool is31bit = ( (parm&0x8fffFFFF) == parm );
        bool is32bit = ( (parm&0xffffFFFF) == parm );
        bool hiOnes  = ( (int)hi == -1 );
        if(is31bit){
            //KASE(1,"lea 31-bit",parm == (parm&0x3fffFFFF));
            lea="lea OUT, "+(lo<=1000000? jitdec(lo): jithex(lo))
                +"# load: 31-bit";
        }else if((int)lo < 0 && hiOnes){
            assert( (int64_t)parm < 0 ); assert((int)lo < 0);
            //KASE(2,"lea 32-bit -ve",parm == (uint64_t)(int64_t)(int32_t)lo);
            lea="lea OUT, "+(parm >= -100000? jitdec((int32_t)lo): jithex(lo))
                +"# load: sext(32-bit)";
        }else if(lo==0){
            //KASE(3,"lea.sl hi only",lo == 0);
            lea="lea.sl OUT, "+jithex(hi)+"# load: hi-only";
        }
        // (more lea kases below, in 2-op section)
        //if(!lea.empty()) lea.append("->"+jithex(parm)+" = "+jitdec(parm));
    }
    string log;
    { // bit ops logic
        // simple cases: I or M zero
        if(log.empty() && isI){
            //KASE(4,"or OUT,"+jitdec(sext7)+",(0)1", parm==sext7);
            log="or OUT, "+jitdec(sext7)+",(0)1 # load: small I";
        }
        if(log.empty() && isM){
            //KASE(5,"or OUT,0,"+jitimm(parm), isMval(parm));
            log="or OUT, 0,"+jitimm(parm)+"# load: (M){0|1}";
        }
        //if(isMval(parm&~0x3f)){...}
        // KASE 6 subsumed by KASE 9 (equivalent I, M)
        if(log.empty() && isMval(parm|0x3f)){
            assert( isMval(parm^sext7) ); // more general, but this is more readable
            uint64_t const ival = (~parm &0x3f);
            uint64_t const mval = parm|0x3f;
            //KASE(7,"xor OUT, ~parm&0x3f "+jithex(ival)+", parm|0x3f "+jitimm(mval),
            //        parm == (ival ^ mval) && isIval(ival) && isMval(mval));
            log="xor OUT, "+jithex(ival)+","+jitimm(mval)+"# load: xor(I,M)";
        }
        if(log.empty() && isMval(parm^sext7) ){ // xor rules don't depend on sign of lo7
            // if A = B^C, then B = A^C and C = B^A (Generally true)
            int64_t const ival = sext7;
            uint64_t const mval = parm^sext7;
            //KASE(9,"xor OUT, "+jithex(ival)+","+jitimm(mval),
            //        parm == (ival ^ mval) && isIval(ival) && isMval(mval));
            log="xor OUT, "+jitdec(sext7)+","+jitimm(mval)+"# load: xor(I,M)";
        }
    }
    string shl; // shift left
    {
        // search for an unsigned right-shift that can regenerate parm
        int oksr=0;
        for(int sr=1; sr<64; ++sr){
            if( parm == (parm>>sr<<sr) && isMval(parm>>sr) ){
                oksr=sr; // use smallest shift
                break;
            }
        }
        if(oksr){
            uint64_t mval = parm >> oksr;
            //KASE(19,"sll OUT,"+jitimm(mval)+","<<jitdec(oksr),
            //        parm == (mval << oksr) && isMval(mval));
            shl="sll OUT, "+jitimm(mval)+","+jitdec(oksr);
        }
    }
    string ari; // arithmetic ops (+,-)
    {
        if(ari.empty()) for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
            // P = I + M <===> M == P - I
            uint64_t const mval = parm - (uint64_t)ival;
            if( isMval(mval) ){
                //KASE(20,"addu.l OUT,"+jitdec(ival)+", "+jitimm(mval),
                //        parm == ival + mval && isIval(ival) && isMval(mval));
                ari="addu.l OUT, "+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
                break;
            }
        }
        if(ari.empty()) for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
            uint64_t const mval = parm - (uint64_t)ival;
            if( isMval(mval) ){
                //KASE(21,"addu.l OUT,"+jitdec(ival)+","+jitimm(mval),
                //        parm == ival + mval && isIval(ival) && isMval(mval));
                ari="addu.l OUT, "+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
                break;
            }
        }
        // Q; Is it correct that SUB (unsigned subtract) still
        //    sign-extends the 7-bit Immediate in "Sy" field?
        if(ari.empty()) for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
            // P = I - M <==> M = I - P
            uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
            if( isMval(mval) ){
                //uint64_t out = (uint64_t)ival - (uint64_t)mval;
                //KASE(22,"subu.l OUT,"+jitdec(ival)+","+jitimm(mval),
                //        parm == out  && isIval(ival) && isMval(mval));
                ari="subu.l OUT, "+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
                break;
            }
        }
        if(ari.empty()) for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
            uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
            if( isMval(mval) ){
                //uint64_t out = (uint64_t)ival - (uint64_t)mval;
                //KASE(23,"subu.l OUT, "+jitdec(ival)+","+jitimm(mval),
                //        parm == out  && isIval(ival) && isMval(mval));
                ari="subu.l OUT, "+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
                break;
            }
        }
    }
    //
    // eventually you might want to reprioritize execution units
    // base on previous or ensuing instruction types
    //
    string chosen = (
            !lea.empty()? lea
            : !log.empty()? log
            : !shl.empty()? shl
            : !ari.empty()? ari : string("")
            );
    if(chosen.empty()){ // 2-op load
        //uint64_t tmplo = (int64_t)(int32_t)lo;
        // lea.sl OUT, <hi or hi+1> (,TMP)
        //        -ve lo will fill hi with ones i.e. -1
        //        so we add hi+1 to MSBs to restore desired sums
        uint64_t dd = ((int32_t)lo>=0? hi: hi+1);
        //uint64_t tmp2 = tmphi << 32;    // (sext(D,64)<<32)
        //uint64_t out = tmp2 + tmplo;     // lea.sl lea_out, tmphi(,lea_out);
        //assert( parm == out );
        lea="lea T0, "+jithex(lo)+"# lo sign-extends"
            + ";lea.sl OUT, "+jithex(dd)+"(,T0) # 2-op load";
        chosen = lea;
    }
    assert( !chosen.empty() );

    if(1){
        prog
#if 0
            .def("STR0(...)", "#__VA_ARGS__")
            .def("STR(...)",  "STR0(__VA_ARGS__)")
            .def("CAT(X,Y)", "X##Y")
            .def("FN",func)             // func name (characters)
            .def("FNS", "\""+func+"\"") // quoted-string func name
            .def("L(X)", "CAT("+func+"_,X)")
            .def("SI","%s0",            "input value")
            .def("ALG","%s1",           "alg modifier [rarely]")
            .def("BP","%s2",            "internal base pointer for const 'here' data")
            // prgiFoo tests have real outputs:
#endif
            .def("OUT","%s3",           "output value")
#if 0
            .def("OUT2","%s4",          "2nd output [sometimes]")
            .def("VOUT","%v0",          "vector outputs")
            .def("VOUT2","%v1",         " Ex. loop_init(ii,jj)-->vl + 2 vectors?")
            // veliFoo tests have 
            //.def("SE","%s2", "error output")
            //.def("SO","%s3", "other output (code path? secondary output?)")
            // all can use tmp scalar registers (that are in VECLOBBER list of wrpiFoo.cpp) 
#endif
            .def("T0","%s40")
            .def("T1","%s41") // etc
#if 0
            // and some vector registers (also in VECLOBBER?)
            .def("V1","%V1")  // etc
            // macros for relocatable branching
            .def("REL(YOURLABEL)", "L(YOURLABEL)    -L(BASE)(,BP)", "relocatable address")
#endif
            .com("SNIPPET START");
        prog.ins(chosen);
            // add more core code to prog, %s0 is the input, output|error-->%s2,...
        prog.com("SNIPPET END")
            .ins("b.l (,%lr)",          "return (no epilogue)")
            .undef("FN").undef("FNS").undef("L")
            ;
        program = prog.flush();
    }
    if(1){ // verbose ?
        cout<<func<<" --> program:\n"
            <<program.substr(0,4096)<<endl;
    }
    return program;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
