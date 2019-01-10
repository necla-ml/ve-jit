/** \file
 * - rework jitve_hello to demo asmfmt on the math kernel example
 * - merge with ve_regs (symbolic registers, scopes) work should continue
 *   in a separate directory (before things get overly complicated).
 */
#include "asmfmt.hpp"

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
int call_loadreg( Jitpage *page, uint64_t const expect){
    int nerr = 0;
    uint64_t cval;
    //
    // Here is how to pass in an argument,
    // call the kernel
    // and retrieve return values from output registers
    //
    asm(//"\tlea %s0,0x666\n"   /*help find this place in asm*/
            "\tlea %s12, (,%[page])\n"  /* lea + "r" this time, instead of ld + "m" */
            "\tbsic %lr,(,%s12)\n"
            "\tor %[cval], 0,%s1\n" /* retrieve kernel compute results */
            :[cval]"=r"(cval)
            :[page]"r"(page->addr)
            :"%s0","%s1" "%s12"
       );
    // Check for errors in our kernel outputs:
    if( cval != expect ) ++nerr;
    if(nerr) printf(" loadreg()_opt--> %0lX, expected %0lX [%lu %ld]\n",
            (unsigned long)cval,   (unsigned long)expect,
            (unsigned long)expect, (unsigned long)expect );
    fflush(stdout);
    return nerr;
}

using namespace std;

void test_kernel_loadreg(char const* const cmd, unsigned long const parm, int const opt_level){
    string veasm_code;
    {
        // more advanced: I output the assembler code to a std::string
        string fname(""); // do not output to file (just get the std::string)
        AsmFmtCols loadreg(); // assembler line pretty printer
        loadreg
            .lcom(STR(Input:    None))
            .lcom(STR(Output:   %s0 : loaded with some jit constant))
            .lcom(STR(Clobbers: %s0, maybe %s1))
            ;
        veasm_code = loadreg.flush();
        cout<<"veasm_code:\n"<<veasm_code<<endl;
    }
    printf("\nTest: %s\n", cmd);
    printf("        opt_level=%d\n",opt_level);
    printf("        parm=0x%16lx = %lu = %ld\n",parm, parm, (signed long)parm);
    printf("        kase=%d\n", kase);

    int kase = 0; // for simple stuff, don't need execution path machinery.
    string program;
    //
    // program ~ jit subroutine to load 'parm' into '%s0'
    //
    {
        printf("// create the JIT assembly code\n");
        fflush(stdout);
        AsmFmtCols prog(ss);
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
            int64_t  const sext7 = (lo7&0x40? lo7&0xffffFFFFffffFF8: lo7);
            // 1-op possibilities depend on range of parm.
            bool isI = (int64_t)parm >= -64 && (int64_t)parm <= 63; // I : 7-bit immediate
            assert( isI == ((int64_t)parm == sext7) ); // equiv condition
            //             so if(isI), then sext7 is the integer "I" value
            bool isM = isimm(parm); // M : (m)B 0<=m<=63 B=0|1 "m high B's followed by not-B's"
            bool is31bit = ( (parm&0x8fffFFFF) == parm );
            bool is32bit = ( (parm&0xffffFFFF) == parm );
            bool isSext32 = (hi==0xffffFFFFF);

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
                lea="lea.sl "+reg+","+jihex(hi)+"# load: hi-only";
            }
            // (more lea kases below, in 2-op section)
            if(!lea.empty()) lea.append("->"+jithex(parm)+" = "+jitdec(parm));

            // logical possibilities, from simple to more complex (kase+=tens)
            string log;
            if(isI){
                kase+=10;
                log="or "+reg+","+jitdec(sext7)+",(0)1 # load: small I";
            }else if(isM){
                kase+=20;
                log="or "+reg+",0,"+jitimm(parm)+"# load: (M){0|1}";
            }else{ // search for logical combination of I and M values
                // set or reset 7 LSBs to zero, and check if that is an M value
                // alt. check properties of sign-extending the lowest 7 bits
                if( isimm(sext7^lo7) ){
                    kase+=30;
                    // if A = B^C, then B = A^C and C = B^A
                    uint64_t mval = sext7^lo7;
                    uint64_t ival = sext7;
                    assert( mval & ival == sext7 );
                    log="xor "+reg+","+jitdec(sext7)+","+jitimm(mval)+"# load: xor(I,M)";
                }
                if(log.empty() && isimm(parm|0x7f) && sext7 < 0){ // 7th lsb == 1 to sign extend
                    kase+=40;
                    // AND with sext7 retains the upper bits and allows some variation in lower 6 bits
                    // Ex. 0b1\+ 0\+ 1[01]{1,6}$        7th lsb always '1',
                    // Ex. 0b0\+ 1\+ 1[01]{1,6}$        so sext7 has all higher bits set
                    log="and "+reg+",~"+jithex(~lo7)+","+jitimm(parm|0x3f)+"# load: and(I<0,M)";
                }
                if(log.empty() && isimm(parm & ~0x3f)){ // can we OR some lsbs?
                    kase+=50;
                    // Ex. 0b1\+ 0\+ 0[01]{1,6}         7th bit '0', so sext has high bit all zero
                    // Ex. 0b0\+ 1\+ 0[01]{1,6}
                    log="or "+reg+","+jithex(lo7)+","+jitimm(parm&~0x3f)+"# load: or(I>0,M)";
                }
            }
            if(!log.empty()) log.append("->"+jithex(parm)+" = "+jitdec(parm));

            string ari; // kase+=hundreds
            // fast arith possibilities (add,sub) TODO (may cover variation in 8th bit, at least)
            // consider signed ops (easier)
            //if parm = I[-64,63] + M, then isimm( parm - I )  (brute force check?)
            //if parm = I[-64,63] - M, then M = I[-64,63] - parm (also covers M = parm+64?)
            // cover sext7 in 2 loops, because positive values are easier to read. (also prefer unsigned over signed sub)
            for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
                if( isimm(parm - (uint64_t)ival) ){
                    kase+=100;
                    ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
                }
            }
            if(ari.empty()) for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
                if( isimm(parm - (uint64_t)ival) ){
                    kase+=200;
                    ari="addu.l "+reg+","+jitdec(ival)+","+jitimm(parm-ival)+"# load: add(I,M)";
                }
            }
            // Q; Is it correct that SUB (unsigned subtract) still sign-extends the 7-bit Immediate in "Sy" field?
            if(ari.empty()) for( int64_t ival = 0; ari.empty() && ival<=63; ++ival ){
                if( isimm((uint64_t)ival - parm) ){
                    kase+=300;
                    ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(parm-(uint64_t)ival)+"# load: subu(I,M)";
                }
            }
            if(ari.empty()) for( int64_t ival = -1; ari.empty() && ival>=-64; --ival ){
                if( isimm((uint64_t)ival - parm) ){
                    kase+=400;
                    ari="subu.l "+reg+","+jitdec(ival)+","+jitimm(parm-(uint64_t)ival)+"# load: subu(I,M)";
                }
            }
            // do not check multiply, since it might take longer that 2-op sequences
            if(!ari.empty()) ari.append("->"+jithex(parm)+" = "+jitdec(parm));

            have_1op_load = !( lea.empty() && log.empty() && ari.empty() );

            if( have_1op_load ){
                // a dedicated "lea generator" would have options to prefer lea vs logical vs arith,
                // perhaps based on context.
                if( !lea.empty() )
                    prog.ins(lea);
                else if( !log.empty() )
                    prog.ins(log);
                else if (!ari.empty() )
                    prog.ins(ari);
            }else{ // assert( !have_1op_load );
                // extend lea case (previously used 1,2,3)
                assert( lo != 0 );
                if((int32_t)lo > 0){
                    kase+=4;
                    assert( hi != 0 );
                    lea="lea "+reg+","+jithex(lo)+"# lo>0
                        + ";lea.sl "+reg+","+jithex(hi)+"(,"+reg+") # load: 2-op";
                }else{ // sign extension means lea.sl of hi will have -1 added to it...
                    kase+=5;
                    lea="lea "+reg+","+jithex(lo)+"# (int)lo<0"
                        + ";lea.sl "+reg+","+jithex(hi+1U)+"(,"+reg+") # load: 2-op";
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
        prog.lcom("finished loadreg_opt"+jitdec(opt_level)+" case "<<kase<<"\n");
        program = a.flush();
    }
    assert( program.size() < 4095 ); // will add a zero terminator
    char kernel[4096U];
    snprintf(kernel_loadreg,4096,"%s\0",program);
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
    test_loadreg();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
