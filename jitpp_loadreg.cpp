/** \file
 * - rework jitve_hello to demo asmfmt on the math kernel example
 * - merge with ve_regs (symbolic registers, scopes) work should continue
 *   in a separate directory (before things get overly complicated).
 */
#include "jitpage.h"
#include "asmfmt.hpp"
#include "codegenasm.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#define STR0(...) #__VA_ARGS__
#define STR(...) STR0(__VA_ARGS__)

/** call ```uint64_t loadreg_opt() [[abi=jit]]```, returning a constant value in %s0.
 * - jit code for loadreg_opt is in \c page
 * - execution begins at \c page->addr
 * - \c page code may only clobber \c %s0 and \c %s1
 * - VE inline assembly ==> probably need VE ncc compiler
 * - please use plain 'C' for this [and other] jit abi wrappers
 * \return 0 or 1 error count (returned %s0 must equal \c expect) */
int call_loadreg( JitPage *page, uint64_t const expect){
    int nerr = 0;
    uint64_t cval;
    //
    // Here is how to pass in an argument,
    // call the kernel
    // and retrieve return values from output registers
    //
#if defined(_ve)
    asm(//"\tlea %s0,0x666\n"   /*help find this place in asm*/
            "\tlea %s12, (,%[page])\n"  /* lea + "r" this time, instead of ld + "m" */
            "\tbsic %lr,(,%s12)\n"
            "\tor %[cval], 0,%s1\n" /* retrieve kernel compute results */
            :[cval]"=r"(cval)
            :[page]"r"(page->mem)
            :"%s0","%s1","%s12"
       );
#else
    cval = expect;      // g++ : just pretend everything worked perfectly
#endif
    // Check for errors in our kernel outputs:
    if( cval != expect ) ++nerr;
    if(nerr) printf(" loadreg()_opt--> %0lX, expected %0lX [%lu %ld]\n",
            (unsigned long)cval,   (unsigned long)expect,
            (unsigned long)expect, (unsigned long)expect );
    fflush(stdout);
    return nerr;
}

using namespace std;

void test_loadreg(char const* const cmd, unsigned long const parm, int const opt_level){
    cout<<" __FUNCTION__ :"<<__FUNCTION__<<":"<<endl;
    cout<<" __PRETTY_FUNCTION__ :"<<__PRETTY_FUNCTION__<<":"<<endl;
    string veasm_code;
    if(0){
        // more advanced: I output the assembler code to a std::string
        string fname(""); // do not output to file (just get the std::string)
        AsmFmtCols loadreg; // assembler line pretty printer
        loadreg.lcom("Input:    None")
            .lcom("Output:   %s0 : loaded with some jit constant")
            .lcom("Clobbers: %s0, maybe %s1")
            ;
        veasm_code = loadreg.flush();
        cout<<"veasm_code:\n"<<veasm_code<<endl;
    }
    printf("\nTest: %s\n", cmd);
    printf("        opt_level=%d\n",opt_level);
    printf("        parm=0x%16lx = %lu = %ld\n",parm, parm, (signed long)parm);

    int kase = 0; // for simple stuff, don't need execution path machinery.
    string program;
    //
    // program ~ jit subroutine to load 'parm' into '%s0'
    //
    {
        printf("// create the JIT assembly code\n");
        fflush(stdout);
        AsmFmtCols prog("");
        prog.lcom(STR(Input:    None))
            .lcom(STR(Output:   %s0 : loaded with some jit constant))
            .lcom(STR(Clobbers: %s0, maybe %s1))
            ;
        //
        // break up parm into <hi,lo> 32-bit words
        //
        uint32_t const hi = ((parm >> 32) & 0xffffFFFF); // unsigned shift-right
        uint32_t const lo = (uint32_t)parm;              // truncate lsbs
        switch(opt_level){
          case(0):
        {
            // Here's an unoptimized version (3 instructions)
            prog.ins("lea %s0,"+jithex(lo),        "lo bits, sign extended")
                .ins("and %s0,%s0,(32)0",          "remove any sign extension")
                .ins("lea.sl %s0,"+jithex(lo)+"(,%s2)", "reg = "+jithex(parm)+" = "+jitdec(parm));
            kase=0;
        }
          break;
          case(1):
        {
            uint64_t const lo7 = (parm & 0x7f);
            //int64_t  const sext7 = ((lo7&0x40)==0x40? lo7|~0x7f: lo7);
            //assert( sext7 == (int64_t)(lo7<<57) >> 57 );
            int64_t const sext7 = (int64_t)(lo7<<57) >> 57; // easier to code
            // 1-op possibilities depend on range of parm.
            uint64_t const lo6 = (parm & 0x3f); // used?
            bool isI = (int64_t)parm >= -64 && (int64_t)parm <= 63; // I : 7-bit immediate
            //             so if(isI), then sext7 is the integer "I" value
            bool isM = isMval(parm); // M : (m)B 0<=m<=63 B=0|1 "m high B's followed by not-B's"
            bool is31bit = ( (parm&0x8fffFFFF) == parm );
            bool is32bit = ( (parm&0xffffFFFF) == parm );
            bool isSext32 = (hi==0xffffFFFFF);
            assert( isI == ((int64_t)parm == sext7) ); // equiv condition

            // lea possibilities (kase+=ones)
            string reg="%s0";
            string lea;
            if(is31bit){
                kase+=1;
                lea="lea "+reg+","+jithex(lo)+"# load: 31-bit";
            }else if(isSext32){
                kase+=2;
                lea="lea "+reg+","+jithex(lo)+"# load: sext(32-bit)";
            }else if(lo==0){
                kase+=3;
                lea="lea.sl "+reg+","+jithex(hi)+"# load: hi-only";
            }
            // (more lea kases below, in 2-op section)
            if(!lea.empty()) lea.append("->"+jithex(parm)+" = "+jitdec(parm));

            int const v=0; // verbose
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
                kase+=10;
                if(v) cout<<" kase "<<kase<<endl;
                log="or "+reg+","+jitdec(sext7)+",(0)1 # load: small I";
                if(v) cout<<log<<endl;
            }else if(isM){
                kase+=20;
                if(v) cout<<" kase "<<kase<<endl;
                log="or "+reg+",0,"+jitimm(parm)+"# load: (M){0|1}";
            }else{ // search for logical combination of I and M values
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
                    kase+=30;
                    uint64_t const ival = parm&0x3f;
                    uint64_t const mval = parm&~0x3f;
                    if(v)cout<<" 30kase "<<kase<<endl;
                    if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
                    if(v)cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
                    assert( parm == (ival | mval) );
                    log+="\n @or "+reg+",0x"+jithex(ival)+","+jitimm(mval)+"# load: or(I,M)";
                    assert( isMval(parm^sext7) ); // is xor the only case we need?
                }
                if(isMval(parm & ~0x3f)){ // can we OR some lsbs? (lo7 sign test not needed)
                    kase+=40;
                    // Ex. 0b1\+ 0\+ 0[01]{1,6}         7th bit '0', so sext has high bit all zero
                    // Ex. 0b0\+ 1\+ 0[01]{1,6}
                    uint64_t const ival = lo7;
                    uint64_t const mval = parm&~0x3f;
                    if(v)cout<<" 40kase "<<kase<<endl;
                    if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
                    if(v)cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
                    assert( parm == (ival | mval) );
                    log+="\n $or "+reg+",0x"+jithex(lo7)+","+jitimm(parm&~0x3f)+"# load: or(I>0,M)";
                    assert( isMval(parm^sext7) ); // is xor the only case we need?
                }
                // B----------------------------
                // M   0... 1... |1111111|
                // I   0... 0...  0xx10xx
                // xor 0... 1...  1yy01yy  imm M = 0x3f | (~0x3f | xor) vs xor sext7<0
                //  B**   isMval(parm&03f)
                if(lo7>=0 && isMval(parm|0x3f)){
                    // equiv kase 60, but small I > 0
                    assert(isMval(parm^sext7) );
                    kase+=50;
                    uint64_t const ival = (~parm&0x3f);
                    uint64_t const mval = parm|0x3f;
                    if(v)cout<<" 50kase "<<kase<<endl;
                    if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
                    if(v)cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
                    assert( parm == (ival ^ mval) );
                    log+="\n ^xor "+reg+",0x"+jithex(ival)+","+jitimm(mval)+"# load: xor(I,M)";
                    // NO! assert( isMval(parm & ~0x3f) ); SEPARATE CASE
                    assert( isMval(parm^sext7) ); // is xor the only case we need?
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
                    kase+=60;
                    if(v)cout<<" 60kase "<<kase<<endl;
                    int64_t const ival = sext7;
                    uint64_t const mval = parm^sext7;
                    if(v)cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
                    if(v)cout<<" mval =0x"<<hex<<mval<<dec<<endl;
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
            if(ari.empty()){
                // SLL %, Sy|M, Sz|N      note Sy is an Mvalue!
                //   Q: is this done on arith unit? or shifting unit?
                int oksr=0;
                for(int sr=1; sr<64; ++sr){
                    if( parm == (parm>>sr<<sr) && isMval(parm>>sr) ){
                        oksr=sr; // use smallest shift
                        break;
                    }
                }
                if(oksr){
                    kase+=100;
                    assert( parm == ((parm>>oksr)<<oksr) );
                    ari="sll "+reg+","+jitimm(parm>>oksr)+","+jitdec(oksr);
                }
            }
            // fast arith possibilities (add,sub) TODO (may cover variation in 8th bit, at least)
            // consider signed ops (easier)
            //if parm = I[-64,63] + M, then isMval( parm - I )  (brute force check?)
            //if parm = I[-64,63] - M, then M = I[-64,63] - parm (also covers M = parm+64?)
            // cover sext7 in 2 loops, because positive values are easier to read. (also prefer unsigned over signed sub)
            if(ari.empty()) for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
                if( isMval(parm - (uint64_t)ival) ){
                    kase+=200;
                    ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
                }
            }
            if(ari.empty()) for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
                if( isMval(parm - (uint64_t)ival) ){
                    kase+=300;
                    ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
                }
            }
            // Q; Is it correct that SUB (unsigned subtract) still sign-extends the 7-bit Immediate in "Sy" field?
            if(ari.empty()) for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
                // P = I - M <==> M = I - P
                uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
                if( isMval(mval) ){
                    kase+=400;
                    uint64_t out = (uint64_t)ival - (uint64_t)mval;
#if 0 // debug
                    if(1)cout<<" parm =0x"<<hex<<setw(16)<<parm<<dec<<endl;
                    if(1)cout<<"~parm =0x"<<hex<<setw(16)<<~parm<<dec<<endl;
                    if(1)cout<<" ival =0x"<<hex<<setw(16)<<ival <<dec<<" = "<<ival<<endl;
                    if(1)cout<<" mval =0x"<<hex<<setw(16)<<mval<<dec<<endl;
                    if(1)cout<<"~mval =0x"<<hex<<setw(16)<<~mval<<dec<<endl;
                    if(1)cout<<"  out =0x"<<hex<<setw(16)<< out<<dec<<endl;
                    if(1)cout<<"~ out =0x"<<hex<<setw(16)<<~ out<<dec<<endl;
                    if(! ((uint64_t)parm == out) ){
                        cout<<" FAILED?: assert( (uint64_t)parm == ((uint64_t)mval - (uint64_t)ival) );"<<mval;
                        cout.flush();
                    }
#endif
                    assert( (uint64_t)parm == out );
                    ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(mval)+"# load: subu(I,M)";
                }
            }
            if(ari.empty()) for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
                // P = I - M <==> M = I - P
                uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
                if( isMval((uint64_t)ival - parm) ){
                    kase+=500;
                    uint64_t out = (uint64_t)ival - (uint64_t)mval;
                    assert( (uint64_t)parm == out );
                    ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(parm-(uint64_t)ival)+"# load: subu(I,M)";
                }
            }
            // do not check multiply, since it might take longer that 2-op sequences
            if(!ari.empty()) ari.append("->"+jithex(parm)+" = "+jitdec(parm));

            bool const have_1op_load = !( lea.empty() && log.empty() && ari.empty() );

            if( have_1op_load ){
                cout<<" lea: "<<lea<<endl;
                cout<<" log: "<<log<<endl;
                cout<<" ari: "<<ari<<endl;
                // a dedicated "lea generator" would have options to prefer lea vs logical vs arith,
                // perhaps based on context.
                //
                // Maybe a good preference is log > shift > arith > lea ???
                //  but if context code uses arith, then arith should be last place
                //  (i.e. instruction choice depends on execution units of previous
                //   and following statements)
                // unless decoder for Mval takes some unexpected time? ?..?
                //
                if( !lea.empty() )
                    prog.ins(lea);
                else if( !log.empty() )
                    prog.ins(log);
                else if (!ari.empty() )
                    prog.ins(ari);
            }else{ // assert( !have_1op_load );
                // extend lea case (previously used 1,2,3)
                if(0){ // long-hand check, mirroring VE lea behaviour:
                    //
                    // lea:         Sx <-- Sy + Sz + sext(D,64)
                    // lea.sl:      Sx <-- Sy + Sz + (sext(D,64)<<32)
                    //
                    // #1) lea TMP, lo
                    uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-0 or all-1
                    // -----> lea.sl TMPLO, lo
                    // #2) lea.sl OUT, <hi or hi+1>>(,tmplo)
                    uint64_t dd = (lo>=0? hi: hi+1); // hi+1 to counteract the all-1
                    uint64_t tmp2 = dd << 32;    // tmp2 = (sext(tmphi,64)<<32)
                    uint64_t out = tmp2 + tmplo;
                    // -----> lea.sl OUT, DD(,TMPLO);
                    assert( parm == out );
                }
                assert( lo != 0 );
                if((int32_t)lo >= 0){
                    kase+=4;
                    assert( hi != 0 );
                    lea="lea "+reg+","+jithex(lo)+"# lo>0"
                        + ";lea.sl "+reg+","+jithex(hi)+"(,"+reg+") # load: 2-op";
                    // simulate and check...
                    uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-0 or all-1
                    uint64_t dd = ((int32_t)lo>=0? hi: hi+1); // hi+1 to counteract the all-1
                    uint64_t tmp2 = dd << 32;    // tmp2 = (sext(tmphi,64)<<32)
                    uint64_t out = tmp2 + tmplo;
                    assert( parm == out );
                }else{ // sign extension means lea.sl of hi will have -1 added to it...
                    kase+=5;
                    lea="lea "+reg+","+jithex(lo)+"# (int)lo<0"
                        + ";lea.sl "+reg+","+jithex(hi+1U)+"(,"+reg+") # load: 2-op";
                    // simulate and check...
                    uint64_t tmplo = (int64_t)(int32_t)lo; // lo bits ok, hi bits all-0 or all-1
                    uint64_t dd = ((int32_t)lo>=0? hi: hi+1); // hi+1 to counteract the all-1
                    uint64_t tmp2 = dd << 32;    // tmp2 = (sext(tmphi,64)<<32)
                    uint64_t out = tmp2 + tmplo;
                    assert( parm == out );
                }
                lea.append("->"+jithex(parm)+" = "+jitdec(parm));
                // TODO: check for lea+or combos, etc.
                prog.ins(lea);
            }
        }
          break;
          default:
        assert(!"illegal opt_level for kernel. Use 0, 1 or 2");
        }
        //
        // and return to caller, "parm has been loaded in %s0"
        //
        prog.ins("b.l (,%lr)", "return");
        prog.lcom("finished loadreg_opt"+jitdec(opt_level)+" case "+jitdec(kase)+"\n");
        program = prog.flush();
    }
    assert( program.size() < 4095 ); // will add a zero terminator
    char kernel[4096U];
    snprintf(kernel,4096,"%s%c",program.c_str(),'\0');
    printf("        kase=%d\n", kase);
    printf("Raw kernel string:\n%s\n-------------",&kernel[0]);
    // (the file routines are plenty verbose...)
    //
    // uniquely name the kernel, in case we wanted several variants
    //                           and to later inspect each kernel
#if 0
    char kernel_name[80];
    if( parm < 1000000 )
        snprintf(kernel_name,80,"tmp_kernel_addsub_%lu_opt%d\0",parm,opt_level);
    else
        snprintf(kernel_name,80,"tmp_kernel_addsub_0x%lx_opt%d\0",parm,opt_level);
    printf(" test_kernel_math(%lx) --> JIT code %s.S:\n%s",parm,kernel_name,kernel_math); fflush(stdout);

    // create .bin file
    asm2bin(kernel_name, kernel_math);  // creates .S and .bin file
    // show it
    char line[80];
    snprintf(line,80,"nobjdump -b binary -mve -D %s.bin\0", kernel_name);
    system(line);

    // load the blob into an executable code page
    char const* basename = &kernel_name[0];
    Jitpage page = bin2jitpage( basename );

    int nerr=0;
    if(page.addr==NULL){
        printf("Oops trying to get the executable code page!\n");
        ++nerr;
    }else{
        // Execute the blob, using inline asm to follow the register-call
        // convention adopted by this kernel. Use a wide range of inputs
        // to the add_sub kernel.
        unsigned long arg = 0;
        nerr += call_addsub( &page, arg, parm );
        if (nerr==0){
            for(arg=1U; arg>0UL; arg<<=4){
                nerr += call_addsub(&page, arg, parm );
            }
        }
    }
    printf("Done test_kernel_math(%lx,opt%d) with %u errors\n",(unsigned long)parm, opt_level, nerr);
    fflush(stdout);
    if(nerr){ // provide a grep-able block of output
        printf(">>>>>>>> error:\n%s\n",kernel_math);
    }
    assert(nerr == 0);
    jitpage_free(&page);
#endif
}
int main(int,char**)
{
    uint64_t const one=1U;
    uint64_t mval;
    uint64_t ival;
    uint64_t parm;
    int opt_level = 1;
    string cmd;
    if(0){ // ok
        parm = (one<<49)-27; // 1-op ADD(I,M)
        string cmd="ldreg"+jitdec(opt_level)+"-2^49-27";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    if(0){ // oh, this is a trivial 6-bit "lea I" case
        parm = ((one<<49)-1) & 0x29; // with 7 lsbs 0, achieve Mval
        string cmd="ldreg"+jitdec(opt_level)+"-and (2^49-1),0x29";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    if(1){
        assert( isMval((one<<49)-1) );
        ival = 0x25;
        mval = (one<<49)-1U;
        parm = ival ^ mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" xor I,(15)0";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = ~mval;
        parm = ival ^ mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" or I,(15)1";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    if(1){
        assert( isMval((one<<49)-1) );
        ival = 0x2c;
        mval = (one<<49)-1U;
        parm = ival | mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" xor I,(15)0";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = ~mval;
        parm = ival | mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" or I,(15)1";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    if(1){
        assert( isMval((one<<49)-1) );
        ival = -13;
        mval = (one<<49)-1U;
        parm = ival ^ mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        string cmd="ldreg"+jitdec(opt_level)+" xor I,(15)0";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = ~mval;
        parm = ival ^ mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" or I,(15)1";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    if(1){
        assert( isMval((one<<49)-1) );
        ival = -13;
        mval = (one<<49)-1U;
        parm = ival | mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" xor I,(15)0";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = ~mval;
        parm = ival | mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" or I,(15)1";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    cout<<" hello world "<<endl;
    if(1) for(int i=-128; i<=128; ++i){
        ival = i;
        cout<<" === ival = "<<i<<endl;
        mval = (one<<49)-1U;
        parm = ival ^ mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" xor I,(15)0";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = ~mval;
        parm = ival ^ mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" or I,(15)1";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = (one<<49)-1U;
        parm = ival | mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" xor I,(15)0";
        test_loadreg( cmd.c_str(), parm, opt_level );
        mval = ~mval;
        parm = ival | mval;
        cout<<" ival =0x"<<hex<<ival <<dec<<" = "<<ival<<endl;
        cout<<" mval =0x"<<hex<<mval <<dec<<" = "<<mval<<endl;
        cout<<" parm =0x"<<hex<<parm <<dec<<" = "<<parm<<endl;
        cmd="ldreg"+jitdec(opt_level)+" or I,(15)1";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
    if(1){ // OK
        parm = 0x3ULL << 40;
        string cmd="ldreg"+jitdec(opt_level)+"-xor ~(2^49-1),0x2r79";
        test_loadreg( cmd.c_str(), parm, opt_level );
    }
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
