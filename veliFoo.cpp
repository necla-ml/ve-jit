/** \file test logic correctness for "loadreg" optimizations */
#include "velogic.hpp"
#include "asmfmt.hpp"
#include "codegenasm.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#define STR0(...) #__VA_ARGS__
#define STR(...) STR0(__VA_ARGS__)

using namespace std;

#if 1 // empty examples
std::string prgiFoo(uint64_t start){
    int kase = 0; // for simple stuff, don't need execution path machinery.
    string program;
    AsmFmtCols prog("");
    prog.lcom(STR(__FUNCTION__) " (i=0x"+jithex(start)+" = "+jitdec(start));
    if(1){ // if you need relative addressing ...
        prog
            .def("STR0(...)", "#__VA_ARGS__")
            .def("STR(...)",  "STR0(__VA_ARGS__)")
            .def("FN",STR(__FUNCTION__))           // func name (characters)
            .def("FNS", "\"" STR(__FUNCTION__) "\"") // quoted-string func name
            .def("L(X)", STR(__FUNCTION__##X))
            .def("SI","%s0",            "input value")
            .def("ALG","%s1",           "alg modifier [rarely]")
            .def("BP","%s2",            "internal base pointer for const 'here' data")
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
            .ins("sic   BP","%s1 is icbase, used as base ptr")
            .lab("L(BASE)")     // this label is reserved
            .ins("b REL(USELESS)","demonstrate a useless branch")
            .lab("L(USELESS)")
            .ins()
            .com("YOUR CODE")
            // add more core code to prog, %s0 is the input, output|error-->%s2,...
            .ins()
            .com("or OUT, 0,(0)1",      "prgIFoo output")
            //.com("or " SE ",0,... veliFoo logic test error detected")
            //
            // and return to caller, "parm has been loaded in %s0"
            //
            .ins("b.l (,%lr)",          "return (no epilogue)")
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
VeliErr veliLoadreg(uint64_t start){
    uint64_t kase = 0; // for simple stuff, don't need execution path machinery.
    VeliErr ret;
    ret.i = start;
    ret.error = 0; // set via KASE macro, if condition fails
    ret.other = 0;
    int const v=1; // verbose
    //string program;
    //
    // program ~ jit subroutine to load 'parm' into '%s0'
    //
    uint64_t const parm = start; // a const version
    //
    // break up parm into <hi,lo> 32-bit words
    //
    uint32_t const hi = ((parm >> 32) & 0xffffFFFF); // unsigned shift-right
    uint32_t const lo = (uint32_t)parm;              // truncate lsbs
    uint64_t const lo7 = (parm & 0x7f);
    int64_t  const sext7 = (int64_t)(lo7<<57) >> 57;
    // OLD int64_t const sext7 = ((lo7&0x40)==0x40? lo7|~0x7f: lo7);
    //     assert( sext7 == (int64_t)(lo7<<57) >> 57 );
    //
    // extras (try to keep min number of requirements for asm code)
    uint64_t const lo6 = (parm & 0x3f);
    // 1-op possibilities depend on range of parm.
    bool isI = (int64_t)parm >= -64 && (int64_t)parm <= 63; // I : 7-bit immediate
    //             so if(isI), then sext7 is the integer "I" value
    bool isM = isMval(parm); // M : (m)B 0<=m<=63 B=0|1 "m high B's followed by not-B's"
    bool is31bit = ( (parm&0x8fffFFFF) == parm );
    bool is32bit = ( (parm&0xffffFFFF) == parm );
    bool isSext32 = (hi==0xffffFFFFF);
    assert( isI == ((int64_t)parm == sext7) ); // equiv condition
    //assert( isM == (popcount(parm) == nTrailOnes(parm)) || popcount(parm) == nLeadingOnes(parm) );
    //
    //

    // lea possibilities (kase+=ones)
    string reg="%s0";
    //string lea;
#define HEXDEC(STR,INT) do{ uint64_t i=(INT); \
    cout<<right<<setw(20)<<STR<<" 0x"<<hex<<setfill('0')<<setw(16)<<i<<setfill(' ')<<dec \
    <<" = "<<setw(16) <<i; \
    if( (int64_t)i < 0 ) cout<<" = "<<(int64_t)i; \
    cout<<endl; \
}while(0)
#define HD(...) HEXDEC( #__VA_ARGS__, (__VA_ARGS__) )
#define KASEDBG do{ \
    cout<<" kase "<<where<<" isI="<<isI<<" isM="<<isM<<" " STR(__FUNCTION__)<<endl; \
    HD(parm); HD(hi); HD(lo); HD(sext7); \
}while(0)
#define KASE(WHERE,STR,...) do{ \
    uint64_t const where = (WHERE); \
    if(v) cout<<" @"<<where<<" "; \
    kase += uint64_t{1} << where; \
    try { \
        if( !(__VA_ARGS__) ){ \
            /* failed condition */ \
            if( ret.error == 0 ) ret.error = where; \
            KASEDBG; \
            THROW("Failure "<<STR<<" kase "<<where); \
        } \
    } catch (...) { /**/ } \
}while(0)
#define HAVE(WHERE) ((kase& (uint64_t{1}<<(WHERE))) != 0)
    if(is31bit){
        KASE(1,"lea 31-bit",parm == (parm&0x3fffFFFF));
        //lea="lea "+reg+","+jithex(lo)+"# load: 31-bit";
    }else if(isSext32){
        KASE(2,"lea 32-bit -ve",parm == (uint64_t)(int64_t)(int32_t)lo);
        //lea="lea "+reg+","+jithex(lo)+"# load: sext(32-bit)";
    }else if(lo==0){
        KASE(3,"lea.sl hi only",lo == 0);
        //lea="lea.sl "+reg+","+jithex(hi)+"# load: hi-only";
    }
    // (more lea kases below, in 2-op section)
    //if(!lea.empty()) lea.append("->"+jithex(parm)+" = "+jitdec(parm));

    if(v) cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
    if(v) cout<<" lo6  =0x"<<hex<<lo6  <<dec<<" = "<<lo6<<endl;
    if(v) cout<<" lo7  =0x"<<hex<<lo7  <<dec<<" = "<<lo7<<endl;
    if(v) cout<<" sext7=0x"<<hex<<sext7<<dec<<" = "<<sext7<<endl;
    if(v) cout<<" bit6 =  "<<(parm&(1<<6))<<endl; // the msb of 7-bit lo7
    // logical possibilities, from simple to more complex (kase+=tens)
    string log;
    // M   1... 0... |0000000|
    // I   1... 1...  1xxxxxx          I = sext7 < 0
    // or  1... 1...  1xxxxxx  1 isI   // or I,(0)1 sext7<0 && ((parm&~0x3f)==~0x3f)
    //also 0... 0...  0??????
    if(isI){
        KASE(4,"or OUT,Imm,(0)1", true);
        //log="or "+reg+","+jitdec(sext7)+",(0)1 # load: small I";
    }else if(isM){
        KASE(5,"or OUT,0,Mval", true);
        log="or "+reg+",0,"+jitimm(parm)+"# load: (M){0|1}";
    }else{ // search for logical combination of I and M values
        // Here I might go through more possibilities than strictly required
        // in order to see how few separate cases need be considered.
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
        // I   1... 1... |1xxxxxx
        //
        // A---------------------------
        // M   1... 0... |0000000|
        // I   0... 0...  0xx10xx      I = sext7 > 0
        // or  1... 0...  0xx10xx  A** M = (~0x3f & or) ***
        if(log.empty() && lo7>=0 && isMval(parm&~0x3f)){
            uint64_t const ival = parm&0x3f;
            uint64_t const mval = parm&~0x3f;
            if(v)cout<<" 30kase "<<kase<<endl;
            if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
            if(v)cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
            KASE(6,"or OUT, parm&0x3f, M(parm&~0x3f)",
                    parm == (ival | mval));
            //log+="\n @or "+reg+",0x"+jithex(ival)+","+jitimm(mval)+"# load: or(I,M)";
        }
        if(isMval(parm & ~0x3f)){ // can we OR some (6) lsbs? (maybe equiv to above) (was 3f)
            // Ex. 0b0\+ 1\+ 0[01]{1,6}
            uint64_t const ival = lo7;
            uint64_t const mval = parm&~0x3f;
            if(v)cout<<" 40kase "<<kase<<endl;
            if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
            if(v)cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
            KASE(7,"or OUT, parm&0x3f, M(parm&~0x3f)",
                    parm == (ival | mval) && isMval(mval) && isIval(ival));
            //log+="\n $or "+reg+",0x"+jithex(lo7)+","+jitimm(parm&~0x3f)+"# load: or(I>0,M)";
        }
        if(HAVE(7)) assert( HAVE(6) ); // can we amalgamate?
        // B----------------------------
        // M   0... 1... |1111111|
        // I   0... 0...  0xx10xx
        // xor 0... 1...  1yy01yy  imm M = 0x3f | (~0x3f | xor) vs xor sext7<0
        //  B**   isMval(parm&03f)
        if(lo7>=0 && isMval(parm|0x3f)){
            assert( (lo7>=0 && isMval(parm|0x3f)) ); // original test
            assert(isMval(parm^sext7) ); // simpler alt test
            // equiv kase 60, but small I > 0
            uint64_t const ival = (~parm&0x3f);
            uint64_t const mval = parm|0x3f;
            if(v)cout<<" 50kase "<<kase<<endl;
            if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
            if(v)cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
            KASE(8,"xor OUT,Ival,Mvval", parm == (ival ^ mval) );
            //log+="\n ^xor "+reg+",0x"+jithex(ival)+","+jitimm(mval)+"# load: xor(I,M)";
            assert( isMval(parm & ~0x3f) ); // ok, so far
        }
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
            if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
            if(v)cout<<" mval =0x"<<hex<<mval<<dec<<endl;
            KASE(9,"xor OUT,Ival,Mvval", parm == (ival ^ mval) );
            assert( parm == (ival ^ mval) );
            log+="\n #xor "+reg+","+jitdec(sext7)+","+jitimm(mval)+"# load: xor(I,M)";
        }
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
            KASE(40,"sll OUT,Mval,"<<jitdec(oksr), parm == ((parm>>oksr)<<oksr) );
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
            KASE(41,"addu.l OUT,+ve,Mval",parm == ival + mval);
            //ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
        }
    }
    for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
        uint64_t const mval = parm - (uint64_t)ival;
        if( isMval(mval) ){
            KASE(42,"addu.l OUT,+ve,Mval",parm == ival + mval);
            //ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
        }
    }
    // Q; Is it correct that SUB (unsigned subtract) still sign-extends the 7-bit Immediate in "Sy" field?
    for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
        // P = I - M <==> M = I - P
        uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
        if( isMval(mval) ){
            uint64_t out = (uint64_t)ival - (uint64_t)mval;
            KASE(44,"subu.l OUT,+ve,Mval",parm == out );
            //ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
        }
    }
    for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
        uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
        if( isMval(mval) ){
            uint64_t out = (uint64_t)ival - (uint64_t)mval;
            KASE(45,"subu.l OUT,-ve,Mval",parm == out );
            //ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
        }
    }
    // do not check multiply, since it might take longer that 2-op sequences
    if(!ari.empty()) ari.append("->"+jithex(parm)+" = "+jitdec(parm));

    //bool const have_1op_load = !( lea.empty() && log.empty() && ari.empty() );

    //if( have_1op_load ){
    //    ;
    //}else
    if(1){ // assert( !have_1op_load );
        // extend lea case (previously used 1,2,3)
        assert( lo != 0 );
        //
        // lea:         Sx <-- Sy + Sz + sext(D,64)
        // lea.sl:      Sx <-- Sy + Sz + (sext(D,64)<<32)
        // lea TMP, lo
        uint64_t lea_out = (int64_t)(int32_t)lo;
        // lea.sl OUT, <hi or hi+1>, 
        uint64_t tmphi = (lo>=0? hi: hi+1); // -ve lo puts 1-bits into hi, so hi+1 to get correct sum
        uint64_t tmp2 = tmphi << 32;    // (sext(D,64)<<32)
        uint64_t out = tmp2 + tmphi;     // lea.sl lea_out, tmphi(,lea_out);
        assert( parm == out );
        if((int32_t)lo > 0){
            // simulate and check...
            uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-0 or all-1
            uint64_t dd = ((int32_t)lo>=0? hi: hi+1); // hi+1 to counteract the all-1
            uint64_t tmp2 = dd << 32;    // tmp2 = (sext(tmphi,64)<<32)
            uint64_t out = tmp2 + tmplo;
            KASE(50,"lea+lea.sl", parm == out );
            //lea="lea "+reg+","+jithex(lo)+"# lo>0"
            //    + ";lea.sl "+reg+","+jithex(hi)+"(,"+reg+") # load: 2-op";
        }else{ // sign extension means lea.sl of hi will have -1 added to it...
            // simulate and check...
            uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-0 or all-1
            uint64_t dd = ((int32_t)lo>=0? hi: hi+1); // hi+1 to counteract the all-1
            uint64_t tmp2 = dd << 32;    // tmp2 = (sext(tmphi,64)<<32)
            uint64_t out = tmp2 + tmplo;
            KASE(51,"lea+lea.sl", parm == out );
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
#else
    return ret;
#endif
}

VeliErr     veliLoadreg(uint64_t start, uint64_t count){
    uint64_t end = start + count; // ok if wraps
    for(uint64_t in=start; in!=end; ++in){
        ;
    }
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
