/** \file
 * C demo of VE assembler optimization  (it is a demo, not the fully optimized loadreg)
 *
 * DEPRECATED : looks like it was abandoned soon after it was created.
 *              jitve_math has the add_sub optimizations now,
 *              and libveli has the optimized "loadreg" codes
 *              (much more sophisticated/exhaustive than here).
 *
 * - Show how to:
 *   - allocate an executable page,
 *   - cross-assemble a small VE assembler kernel,
 *   - execute the kernel function
 *   - and even return
 *
 * - We kinda' do a function call, but the jit code:
 *   - executes in caller stack frame
 *   - any args must be passed in registers only
 *   - code must be fully relocatble and need no external symbols
 *
 * - The incredibly important function we optimize is
 *   adding and subtracting a uint64_t constant from a register,
 *   producing \f$register \pm CONSTANT\f$ outputs.
 *   - in general, this takes 4 ops
 *     - 2 to load the constant into a register,
 *     - 2 more to produce the sum and difference.
 *   - but add and sub sometimes can use immediate operands
 *     - there is some slight asymmetry here.
 *   - and sometimes the load can be done in 1 op.
 *   - and of course adding and subtractin zero is pretty easy.
 *   - you can go wild and recognize some longer bti sequences
 *     that are mathematically related to immediate constants
 *     (ex. load some big constant via mul IMM,K or other arithmetic ops,
 *      but such constants are likely pretty infrequent.)
 */
#include "jitpage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>   // _SC_PAGE_SIZE
//char const* code = ".string \"Hello world\"\n"; // this is an assembly kernel
#define STR0(s...) #s
#define LAB(s...) STR0(s\n)
#define COMM(s...) "\t# " #s "\n"
#define TTR(s...) "\t" #s "\n"
void jit_parm_opt1( unsigned long parm, char *jit_parm, size_t const jsz ){
    printf("optimized JIT kernel to load constant value %lu\n",parm); fflush(stdout);
    {
        size_t n=0;
        n += snprintf(&jit_parm[n],jsz-n, "\t# load JIT parm %lx = %lu = %ld into %%s2\n",
                parm, parm, (signed long)parm);
        assert(n<jsz);
        uint32_t const lo = (uint32_t)parm;
        // 4 combinations of lo and/or hi bits needing to be set
        if( lo ){
            n += snprintf(&jit_parm[n],jsz-n, "\tlea %%s2, 0x%x\n", lo);
            assert(n<jsz);
            printf("(int)lo<0 is %d\n",((int)lo<0? 1: 0));
            uint32_t const hi = (uint32_t)(parm>>32) + ((int)lo<0? 1: 0);
            // lea.sl ADDS to existing content,
            //    so if lo was sign-extended into the top 32 bits,
            //    we must decrement hi to get the correct result here!
            if( hi ){
                n += snprintf(&jit_parm[n],jsz-n, "\tlea.sl %%s2, 0x%x(,%%s2)\n", hi);
                assert(n<jsz);
            }else{
                if((int)lo<0) n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-ones hi bits\n");
                else          n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-zero hi bits\n");
                assert(n<jsz);
            }
        }else{
            n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-zero lo bits\n");
            assert(n<jsz);
            uint32_t const hi = (uint32_t)(parm>>32); // lo was zero, no fixup
            if( hi ){
                // this ALSO zero the lo 32 bits, as required
                n += snprintf(&jit_parm[n],jsz-n, "\tlea.sl %%s2, 0x%x\n", hi);
                assert(n<jsz);
            }else{
                // all bits zero
                n += snprintf(&jit_parm[n],jsz-n,
                        "\t# fast load zero\n"
                        "\tor  %%s2,0,(0)1\n");
                assert(n<jsz);
                // Note: or take sy or I,   sz or M args
                //       read (0)1 as "zero 1's, followed by rest of bits not-1"
            }
        }
    }
}
void jit_parm_opt2( unsigned long const parm, char * const jit_parm, size_t const jsz ){
    size_t n=0;
    long const lparm = (long)parm;
    char mconst[6];
    int ok=0, add_ok=0, sub_ok=0, loaded_neg_parm=0;
    if(parm==0){
        printf("// just copy register s0 --> s1 and s2\n"); fflush(stdout);
        JOUT("%s", COMM(#add/sub zero is really easy!)
                TTR(or %s1,0,%s0)       /* this is NOT a format-string! (no %%) */
                TTR(or %s2,0,%s0));
        ok=add_ok=sub_ok = 1;
    }
    if(!ok && strMconst(mconst, parm)){
        // M-constants can be embedded directly into BOTH add/sub (fastest,no extra reg load)
        //printf(" M-const!"); fflush(stdout);
        JOUT(COMM(add/sub Mconst)
                "\taddu.l %%s1, %%s0,%s\n"
                "\tsubu.l %%s2, %%s0,%s\n", mconst, mconst);
        ok=add_ok=sub_ok = 1;
    }
    if(!ok){
        printf("// fast load JIT const into %%s2 (subtract NEEDS this)\n");
        JOUT(COMM(load JIT parm %lu into %%s2),parm);
        assert(parm != 0UL);  // we already handled the trivial case
        uint32_t const lo = (uint32_t)parm;
        if(lo){
            uint32_t const hi = (uint32_t)(parm>>32);
            if((int32_t)lo > 0 && hi == 0){
                JOUT(COMM(lo > 0, hi == 0) "\tlea %%s2, 0x%x\n", lo);
            }else if((int32_t)lo > 0){ assert( hi != 0 );
                // The next 2 load cases can be generalized by
                // looping over Imm in [-63,64],
                // and also by instructions such as addu.l, which covers
                // a few more constants
                // I don't think the cases covered will by particularly common
                if( strMconst(mconst, parm/2) && parm/2*2 == parm ){
                    JOUT(COMM(Imm * 2) "\tmulu.l %%s2, 2, %s\n", mconst);
                }else if( strMconst(mconst, lparm/(-20)) && lparm/(-20)*(-20) == lparm ){
                    JOUT(COMM(Imm * -20) "\tmulu.l %%s2, -20, %s\n", mconst);
                }else{
                    JOUT(COMM(lo > 0)  "\tlea %%s2, 0x%x\n", lo);
                    JOUT(COMM(hi != 0) "\tlea.sl %%s2, 0x%x(,%%s2)\n", hi);
                }
            }else if( hi == 0xFFffFFffU ){ assert( (int32_t)lo < 0 );
                JOUT(COMM(lo < 0 and hi all-ones) "\tlea %%s2, 0x%x\n", lo);
            }else{ assert( (int32_t)lo < 0 );
                if( strMconst(mconst, parm/2) && parm/2*2 == parm ){
                    JOUT(COMM(Imm * 2) "\tmulu.l %%s2, 2, %s\n", mconst);
                }else if( strMconst(mconst, lparm/(-20)) && lparm/(-20)*(-20) == lparm ){
                    JOUT(COMM(Imm * -20) "\tmulu.l %%s2, -20, %s\n", mconst);
                }else{
                    JOUT(COMM(lo < 0) "\tlea %%s2, 0x%x\n", lo);
                    // lea.sl ADDS to existing content,
                    //    which is all-1s if prev lea sign-extended.
                    uint32_t const hip1 = hi + 1U; assert(hip1 != 0);
                    JOUT(COMM(hi+1 into hi bits) "\tlea.sl %%s2, 0x%x(,%%s2)\n", hip1);
                }
            }
        }else{
            JOUT(COMM(skip loading all-zero lo bits));
            uint32_t const hi = (uint32_t)(parm>>32); // lo was zero, no fixup
            if( hi ) // this ALSO zeros the lo 32 bits, as required
                JOUT(COMM(lo==0 hi!=0) "\tlea.sl %%s2, 0x%x\n", hi);
            else // all bits zero
                assert(!"never get here: parm=0 was already handled");
        }
    }
    if(!ok && (!add_ok || !sub_ok)){
        // the ADD can start right away **if** we have an I-value, in [-64,63]
        printf("   parm=%lu = %lx = %ld\n", parm, parm, (signed long)parm);
        if( !add_ok && lparm >= -64 && lparm <= 63 ){
            // Note: sy~Imm, Sz~Mconst   not commutative
            JOUT(COMM(add Imm) "\taddu.l %%s1,%ld,%%s0\n", lparm);
            add_ok = 1;
        }
        if( !add_ok ){ // o.w. the ADD can do a register-register OP
            JOUT("%s", TTR(addu.l %s1,%s0,%s2));
            add_ok = 1;
        }
        if( !sub_ok ){
            JOUT("%s", TTR(subu.l %s2,%s0,%s2));
            sub_ok = 1;
        }
    }
    printf("opt2 returns string:\n%s\n",&jit_parm[0]);
    fflush(stdout);
#undef JOUT
}
/** call ```uint64_t loadreg_opt() [[abi=jit]]```, returning a constant value in %s0.
 * - jit code for loadreg_opt is in \c page
 * - execution begins at \c page->mem
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
    asm(//"\tlea %s0,0x666\n"   /*help find this place in asm*/
            "\tlea %s12, (,%[page])\n"  /* lea + "r" this time, instead of ld + "m" */
            "\tbsic %lr,(,%s12)\n"
            "\tor %[cval], 0,%s1\n" /* retrieve kernel compute results */
            :[cval]"=r"(cval)
            :[page]"r"(page->mem)
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
void test_kernel_loadreg(char const* const cmd, unsigned long const parm, int const opt_level){
    printf("\nTest: %s\n", cmd);
    printf("        opt_level=%d (unused)\n",opt_level);
    printf("        parm=0x%16lx = %lu = %ld\n",parm, parm, (signed long)parm);
    printf("      Here we create JIT assembly code to load parm into a register");
    printf("      Depending on the size of the constant, we will output different\n");
    printf("      assembly code; i.e. we do a tiny JIT optimization\n");
    char kernel_loadreg_opt[4096U];
    {
        stringstrem
        AsmFmtCols ve_loadreg(ss);
        printf("// create the JIT assembly code\n");
        fflush(stdout);
        char const* kernel_math_begin =
            COMM(runtime JIT value unsigned long 'parm')
            COMM(input:   %s0)
            COMM(output:  %s1 = %s0 + parm)
            COMM(         %s2 = %s0 - parm)
            COMM(clobbers: nothing else)
            ;
        // Here's a reasonable strategy:
        // 1) stick runtime value parm into register %s2 (various possibilities!)
        size_t const jsz=240;
        char jit_parm[240];
        size_t n=0;
        // and then add/sub %s2 from %s0, returning %s1 and %s2 final values
        char const* kernel_math_end =
            TTR(addu.l %s1,%s0,%s2)
            TTR(subu.l %s2,%s0,%s2)
            TTR(b.l	(,%lr))
            ;
        switch(opt_level){
            case(0):
                // WRONG: lea.sl CLOBBERS the high bits, so if 
                // Here's an unoptimized version
                n += snprintf(jit_parm,jsz-n,
                        "\t# load JIT parm into %%s2\n"
                        "\tlea    %%s1, 0x%x        # might sign-extend into hi bits\n"
                        "\tor     %%s1, 0, (32)0    # keep only lowest 32 bits\n"
                        "\tlea.sl %%s2, 0x%x(,%%s1) # add the high bits\n",
                        (uint32_t)parm,  (uint32_t)(parm>>32) + ((int32_t)parm<0? 1: 0));
                assert(n<jsz);
                break;
            case(1):
                // if any half of the uint64_t parm is zero, we don't need to load it.
                jit_parm_opt1(parm, jit_parm, jsz);
                break;
            case(2):
                printf("   parm=%lu = %lx = %ld\n", parm, parm, (signed long)parm);
                jit_parm_opt2(parm, jit_parm, jsz);
                kernel_math_end = TTR(b.l (,%lr)); // just the return instruction!
                break;
            default:
                assert(!"illegal opt_level for kernel. Use 0, 1 or 2");
        }
        snprintf(kernel_math,4096,"\t# %s\n\t# opt_level=%d\n%s\n%s\n%s", cmd,
                opt_level, kernel_math_begin, jit_parm, kernel_math_end);
    }
    //printf("Raw kernel string:\n%s\n-------------",&kernel_math[0]);
    // (the file routines are plenty verbose...)
    //
    // uniquely name the kernel, in case we wanted several variants
    //                           and to later inspect each kernel
    char kernel_name[80];
    if( parm < 1000000 )
        snprintf(kernel_name,80,"tmp_kernel_addsub_%lu_opt%d\0",parm,opt_level);
    else
        snprintf(kernel_name,80,"tmp_kernel_addsub_0x%lx_opt%d\0",parm,opt_level);
    printf(" test_kernel_math(%lx) --> JIT code %s.S:\n%s",parm,kernel_name,kernel_math); fflush(stdout);

    // create .bin file
    asm2bin(kernel_name, kernel_math, 2/*verbose*/);  // creates .S and .bin file
    // show it
    char line[80];
    snprintf(line,80,"nobjdump -b binary -mve -D %s.bin\0", kernel_name);
    system(line);

    // load the blob into an executable code page
    char const* basename = &kernel_name[0];
    JitPage jp;
    int nerr=0;
    if(bin2jitpage( basename, jp, 2/*verbose*/ ) == NULL){
        ++nerr;
    }else{
        if(page.mem==NULL){
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
    }
    printf("Done test_kernel_math(%lx,opt%d) with %u errors\n",(unsigned long)parm, opt_level, nerr);
    fflush(stdout);
    if(nerr){ // provide a grep-able block of output
        printf(">>>>>>>> error:\n%s\n",kernel_math);
    }
    jitpage_free(&jp);
    assert(nerr == 0);
}

main()
{
    printf(" I think that -13UL is %ld\n", -13UL);
    printf(" I think that -(13UL) is %ld\n", -(13UL));
    unsigned long const posit = 13UL;
    unsigned long const negat = -posit;
    printf(" I think that posit is %ld\n", (signed long)posit);
    printf(" I think that negat is %ld\n", (signed long)negat);
    // DOIT supplies the actual function call as a string to the
    // function being called (as the first function argument)
#define DOIT(function,...) do{ \
    printf("Function, Args: %s\n", #function ", " #__VA_ARGS__); \
    function( #function "(char*," #__VA_ARGS__ ")", \
              __VA_ARGS__ ); \
}while(0)
    for(int opt_level=0; opt_level<3; ++opt_level){
        DOIT(test_kernel_math,0U,opt_level);
        DOIT(test_kernel_math,1U,opt_level);
        DOIT(test_kernel_math,57,opt_level);
        DOIT(test_kernel_math,0x0000000000ffFFffUL, opt_level); // an Mconst, (52)0
        DOIT(test_kernel_math,0x00000000FFffFFffUL, opt_level); // an Mconst, (52)0
        DOIT(test_kernel_math,0x00001fffffffffffUL, opt_level); // an Mconst, (19)0
        DOIT(test_kernel_math,0xFFFF000000000000UL, opt_level); // an Mconst, (16)1
        DOIT(test_kernel_math,0xffFFffFF00000000UL, opt_level); // Mconst
        DOIT(test_kernel_math,0x00000000bbeeaaddUL, opt_level);
        DOIT(test_kernel_math,0xddeeaadd00000000UL, opt_level);
        DOIT(test_kernel_math,0x00000033ffFFffFFUL, opt_level);
        DOIT(test_kernel_math,0x5555555555000000UL, opt_level);
        DOIT(test_kernel_math,0x9999999999000000UL, opt_level);
        DOIT(test_kernel_math,0x0000003300000000UL, opt_level);
        DOIT(test_kernel_math,0x1122334455667788UL, opt_level);
        DOIT(test_kernel_math,0x00000000FFFFffffUL*2UL, opt_level);
        DOIT(test_kernel_math,0x000000000000FFFFUL*(-20), opt_level);
        DOIT(test_kernel_math,0x0001FFFFffffFFFFUL*(-20), opt_level);
        DOIT(test_kernel_math,posit, opt_level);
        DOIT(test_kernel_math,negat, opt_level);
        printf("-(17UL)\n");
        DOIT(test_kernel_math,-(17UL),opt_level);  // -13UL is really -60  (mistaken hex?)
    }
#undef DOIT
    system("ls -l tmp_kernel_addsub_*.S");
    system("ls -l tmp_kernel_addsub_*.bin");
    printf("\n");
}
/* vim: set ts=4 sw=4 et: */
