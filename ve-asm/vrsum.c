/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * simple demo of Vector-reduction summation op \e vsum.l */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#define MVL 256
#define VL 10 
#define FOR(i,vl) for(uint64_t i=0; i<(vl); ++i)
typedef unsigned long lu;
int main(int argc, char**argv){
    int ret=0;

    uint64_t vl = VL;
    // Does gcc "register asm local variables" work?
    //
    //asm __volatile__ ("### VLA");
    //register volatile uint64_t vl asm("%s44"); vl = VL;
    ////asm __volatile__ ("lvl %s44":::);
    //asm __volatile__ ("### VLB");
    //
    // NO - it COMPILES but does NOT use %s44. (ex. %s24)
    //    Even worse, it ignores the 'volatile', spilling
    //    and restoring vl from stack (to other register)
    // So DO NOT USE THEM.  Even gcc explicitly does not support
    // them to pass fixed regs into later asm constructs !!

    uint64_t v[MVL];
    uint64_t exp_sum[MVL];
#define K 0x870a52f7ca464a45
    FOR(i,MVL) v[i] = K+i;
    FOR(i,MVL) exp_sum[i] = (i+1)*K + i*(i+1) / 2; // correct sums
    // check case 'vl'
    uint64_t vl_sum=0;
    printf("\nAll partial sums [C code]\n");
    FOR(i,vl){
        vl_sum += v[i];
        printf(" %lu: sum %lu %lu\n",
                (lu)i,(lu)exp_sum[i],(lu)vl_sum);
    }
    printf(" vl_sum = sum_{i=0}^{i=%lu}( i ) = %lu\n",
            (lu)vl, (lu)vl_sum);
    fflush(stdout);
    assert( vl_sum == exp_sum[vl-1] );
    // now do asm version
    printf("\nAssembler version...\n");
#define WHICH 0
#if WHICH==0
    // Sept 10 C++ user guide:
    // "The only Extended asm syntax specified to scalar register, vector register, vector mask
    // register, vector index register in an Assembler-Template is to specify clobbered
    // register."
    {
        uint64_t volatile v0[256];
        uint64_t volatile v1[256];
        uint64_t volatile *v0_addr = &v0[0];
        uint64_t volatile *v1_addr = &v1[0];
        FOR(i,MVL) v0[i]=0;
        FOR(i,MVL) v1[i]=0;
        uint64_t volatile v1beg;
        uint64_t volatile sK = K;
        asm __volatile__ ("### AAA");
        asm(
                "lvl %[vl]\n"
                "lea %s43,666\n"
                "vbrd %v41,%s43\t\t# %v41 = all-666\n"
                "vseq %v40\t\t\t# v40 = 0,1,2,...\n"
                "vaddu.l %v40,%[sK],%v40\t#    + sK\n"
                "vsum.l %v41,%v40\n"
                "vst %v40,8,%[v0_addr]\n"
                "vst %v41,8,%[v1_addr] # v1[0] = sum^{vl}( v0[i] )\n"
                "lvs %[v1beg],%v41(0)\n"
                : [v1beg]"=r"(v1beg)
                : [vl]"r"(vl)
                , [v0_addr]"r"(v0_addr)
                , [v1_addr]"r"(v1_addr)
                , [sK]"r"(sK)
                : "%s43","%v40","%v41","memory"
           );
        asm __volatile__ ("### BBB");
        FOR(i,vl) assert( v0[i] == K+i );
        FOR(i,vl) printf("i=%-3lu v0[i]=%-10lu v1[i]=%-10lu\n",
                (lu)i,(lu)v0[i],(lu)v1[i]);
        uint64_t asm_sum=v1[0];
        printf("vl=%lu vl_sum=%lu asm_sum=%lu v1beg=%lu",
                (lu)vl,(lu)vl_sum,(lu)asm_sum,(lu)v1beg);
        ret = (v1[0] == asm_sum );
        fflush(stdout);
        assert( v1[0] == asm_sum );
        assert( v1[0] == K*vl + (vl-1)*vl/2 );
        assert( v1[0] == exp_sum[vl-1] );
    }
#elif WHICH==1
    {
        uint64_t volatile v0[256];
        uint64_t volatile v1[256];
        uint64_t volatile v1beg;  // the scalar sum
        uint64_t volatile sK = K;
        asm __volatile__ ("### AAA");
        asm(
                "lvl %[vl]\n"
                "xor %s43,%s43,%s43\n"
                "vbrd %v41,%s43\n"
                "vseq %v40\n"
                "vaddu.l %v40,%[sK],%v40\n"
                "vsum.l %v41,%v40\n"
                "lvs %[v1beg],%v41(0)\t\t\t# v1beg = v41[0] = sum(v40[i])\n"
                : [v1beg]"=r"(v1beg)
                : [vl]"r"(vl)
                , [sK]"r"(sK)
                : "%s43","%v40","%v41" // only clobbered mem is write-clause v1beg
           );
        asm __volatile__ ("### BBB");
        printf("vl=%lu vl_sum=%lu v1beg=%lu",
                (lu)vl,(lu)vl_sum,(lu)v1beg);
        ret = (vl_sum == v1beg );
        fflush(stdout);
    }
#elif WHICH==2
    // There seems still no way to pass vector registers in/out of asm blocks
    {
        uint64_t volatile v0[256];
        uint64_t volatile v1[256];
#pragma _NEC vreg(v0)
#pragma _NEC vreg(v1)
        FOR(i,MVL) v0[i]=i;
        FOR(i,MVL) v1[i]=0;
        uint64_t volatile v1beg;  // the scalar sum
        uint64_t volatile sK = K;
        asm __volatile__ ("### AAA");
        asm(
                "lvl %[vl]\n"
                "xor %s43,%s43,%s43\n"
                "vbrd %v41,%s43\n"
                "vseq %[v40]\n"
                "vaddu.l %[v40],%[sK],%v40\n"
                "vsum.l %v41,%v40\n"
                "lvs %[v1beg],%v41(0)\t\t\t# v1beg = v41[0] = sum(v40[i])\n"
                : [v1beg]"=r"(v1beg)
                , [v40]"=r"(v0) //vreg(v0) comes in as scalar
                //, [v40]"=r"(*v0) // also as scalar
                : [vl]"r"(vl)
                , [sK]"r"(sK)
                : "%s43","%v40","%v41","memory"
           );
        asm __volatile__ ("### BBB");
        printf("vl=%lu vl_sum=%lu v1beg=%lu",
                (lu)vl,(lu)vl_sum,(lu)v1beg);
        ret = (vl_sum == v1beg );
        fflush(stdout);
    }
#endif // WHICH
    printf("\nGoodbye\n");
    return ret;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.
