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
    uint64_t vl = VL;
    uint64_t v[MVL];
    uint64_t exp_sum[MVL];
#define K 0x870a52f7ca464a45
    FOR(i,MVL) v[i] = K+i;
    FOR(i,MVL) exp_sum[i] = (i+1)*K + i*(i+1) / 2; // correct sums
    // check case 'vl'
    uint64_t vl_sum=0;
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
            "xor %s43,%s43,%s43\n"
            "vbrd %v40,%s43\n"
            "vbrd %v41,%s43\n"
            "vseq %v40\n"
            "vaddu.l %v40,%[sK],%v40\n"
            //"vaddu.l %v41,%v40,%v40\n"
            //"vsubu.l %v40,%v41,%v40\n"
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
    uint64_t asm_sum=v1[0];
    printf("vl=%lu vl_sum=%lu asm_sum=%lu v1beg=%lu",
            (lu)vl,(lu)vl_sum,(lu)asm_sum,(lu)v1beg);
    int ret = (v1[0] == asm_sum );
    fflush(stdout);
    assert( v1[0] == asm_sum );
    assert( v1[0] == K*vl + (vl-1)*vl/2 );
    assert( v1[0] == exp_sum[vl-1] );
    printf("\nGoodbye\n");
    return ret;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.
