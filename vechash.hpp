#ifndef VECHASH_HPP
#define VECHASH_HPP

#include "asmfmt_fwd.hpp"
#include "throw.hpp"
#include <assert.h>
// tmp debug
#include <iostream>
#include <iomanip>

#ifndef FOR
#define FOR(I,VL) for(int I=0;I<VL;++I)
#endif

namespace scramble64 {
    // These might be held in a common scalar register
    extern uint64_t const r1;          //< REHASH_A
    extern uint64_t const r2;          //< REHASH_B
    extern uint64_t const r3;          //< REHASH_C [opt]
}

/** Hash a sequence of vectors \c v_i[0..vl[i]-1].
 * - Properties:
 *   - The accumulating hash will depend on:
 *     - the overall data sequence, \c data[0..\sum_i(vl[i])-1]
 *   - The accumulatin hash will \b not depend on:
 *     - any particular choice non-zero vector length sequence \c vl[i]
 * - Method:
 *   - internal state remembers sequence position \c j and current \c hashval
 *   - map incoming \c 0..vl[i]-1 indices to current sequence position \c vs[]=j..j+vl[i]-1
 *   - \b hash both \c v[0..vl-1] and \c vs[]
 *   - \b sum those hash vectors
 *   - \b xor the sum vector into current \c hashval
 * - Require:
 *   - all \c vl<=MVL, where \c MVL is given to the constructor.
 *     - \c MVL is max vector length (for SIMD register).
 *
 * - Special versions:
 *   - \c VecHash2::hash_combine(a[],b[],vl) hashes a pair of sequences
 *   - \c VecHashConstVl::hash_combine(v) is a faster version for constant \c vl==mvl.
 */
class VecHash {
  public:
    /** helper for generic vector hashing.
     * \p mvl limits vector length of incoming vectors.
     *
     */
    VecHash( int const mvl, uint32_t const seed=0 )
        : mvl(mvl), mem(new uint64_t[mvl*3])
          , hashVal(0), j((uint64_t)seed<<32) //, j0(j)
          // vs,vx,vy simulate vector registers
          // (better if the memory is on separate cache pages)
          , vs(mem+0), vx(mem+mvl), vy(mem+mvl+mvl), vz(mem+3*mvl)
    {
        if(!mem) THROW("Out of memory");
        init();
    }
    ~VecHash(){
        delete[](mem);
    }
    // /// return number of values hashed (sum of all \c vl combined)
    // uint64_t size() const { return j-j0; }
    /// return current hash value (non-memoized xor-reduction)
    uint64_t u64() const {
        return hashVal;
    }
    /// set current hash value (for testing)
    uint64_t u64(uint64_t const h) {
        return hashVal = h;
    }
    int const mvl;
    /** basic hash_combiner for a vectorized sequence */
    void hash_combine( uint64_t const* v, int const vl ){
        assert( vl > 0 && vl <= mvl ); // vl==0 OK, but wasted work.
        using namespace scramble64;  // r1, r2, r3 scramblers
        //FOR(i,vl) vs[i] = i;
        FOR(i,vl) vz[i] = j + vs[i];
        FOR(i,vl) vy[i] = r2 * v[i];            // hash v[]
        FOR(i,vl) vz[i] = r1 * vz[i];           // hash vs[]=j..j+vl-1
        FOR(i,vl) vx[i] = vy[i] + vy[i];        // add hash_v[] and hash_vs[]
        // Note: for vector state would want to VMV be vl if vl<MVL, I think
        // But for now I will reduce every vx[] right away.
        r = 0;
        FOR(i,vl) r ^= vx[i];
        hashVal ^= r;                           // hashVal ^= vx[0]^vx[1]^...^vx[vl-1]
        j += vl;
    }
    static void kern_asm_begin( AsmFmtCols &a, char const* client_vs=nullptr,
            uint32_t const seed=0 );
    static void kern_asm( AsmFmtCols &parent,
            std::string va, std::string vl, std::string hash );
    static void kern_asm_end( AsmFmtCols &a );
  private:
    void init(){
        FOR(i,mvl) vs[i] = i;
        FOR(i,mvl) vx[i] = 0;
    }
    uint64_t *mem;      // to simulate vector registers
    uint64_t hashVal;   // accumulating sequence hash value
    //uint64_t j0;        // initial value of j
    uint64_t j;         // incrementing sequence position, j0, j0+1, ...
    uint64_t *vs;       // vr[0..mvl-1] seq register for (j..j+vl-1)
    // temporary registers
    uint64_t *vx;       // vx[0..mvl-1] scratch register
    uint64_t *vy;       // vx[0..mvl-1] scratch register
    uint64_t *vz;       // vz[0..mvl-1] scratch register
    uint64_t r;         // scalar scratch register
};
/** VecHash2 accepting two same-sized sequences.
 *
 * To hash together two same-length vectors, hash{a[],b[]}:
 *
 * 1. <B>Recommended:</B> You may use \c VecHash2::hash_combine(a,b,vl).  
 *
 * 2. You \b could provide distinct \c seed values for 2 \c VecHash.
 *   It is better to to have the index position hashes being exactly the
 *   same because we are using XOR reduction.  Ex. a[] could have seed
 *   0 and b[] could have seed 1.  The separate \c VecHash for a[] and b[]
 *   then get combined to get a hash for the combined {a[],b[]}.
 *
 * 3. You <B>should not</B> use \c VecHash::hash_combine(a,vl),
 *    then \c VecHash::hash_combine(b,vl) and repeat -- such a hash would
 *    epend on the choices you make for \c vl.
 */
struct VecHash2 {
    /** helper for generic vector hashing.
     * \p mvl limits vector length of incoming vectors.
     *
     */
    VecHash2( int const mvl, uint32_t const seed=0 )
        : mvl(mvl), mem(new uint64_t[4*mvl]), hashVal(0), j((uint64_t)seed<<32) //, j0(j)
          , vs(mem+0), vx(mem+mvl), vy(mem+mvl+mvl), vz(mem+3*mvl)
    {
        if(!mem) THROW("Out of memory");
        init();
        std::cout<<"+vh2: seed="<<seed<<" j="<<std::hex<<j<<std::dec<<std::endl;
    }
    ~VecHash2(){
        delete[](mem);
    }
    /// return current hash value (non-memoized xor-reduction)
    uint64_t u64() const {
        return hashVal;
    }
    /// set current hash value (for testing)
    uint64_t u64(uint64_t const h) {
        return hashVal = h;
    }
    int const mvl;
    /** hash combine a pair of same-size vectorized sequences. */
    uint64_t hash_combine( uint64_t const* v, uint64_t const* w, int const vl ){
        assert( vl > 0 && vl <= mvl ); // vl==0 OK, but wasted work.
        using namespace scramble64;  // r1, r2, r3 scramblers
        //FOR(i,vl) vs[i] = i;
        FOR(i,vl) vx[i] = r2 * v[i];            // hash v[]
        FOR(i,vl) vy[i] = j + vs[i];    // Note: vs[i] could be a maintained register
        FOR(i,vl) vz[i] = r3 * w[i];            // hash w[]
        FOR(i,vl) vy[i] = r1 * vy[i];           // hash vs[]=j..j+vl-1
        FOR(i,vl) vx[i] = vx[i] + vz[i];        // hash_v[] + hash_w[]
        FOR(i,vl) vz[i] = vx[i] + vy[i];        // add hash_vs[]
        std::cout<<" pre-xor-reduce vx[0]="<<std::hex<<vx[0]<<std::dec<<std::endl;
        std::cout<<" pre-xor-reduce vy[0]="<<std::hex<<vy[0]<<std::dec<<std::endl;
        std::cout<<" pre-xor-reduce vz[0]="<<std::hex<<vz[0]<<std::dec<<std::endl;
        r = 0;
        FOR(i,vl) r ^= vz[i];
        std::cout<<" xor-reduce r="<<std::hex<<r<<std::dec<<std::endl;
        hashVal ^= r;                           // hashVal ^= vx[0]^vx[1]^...^vx[vl-1]
        j += vl;
        return hashVal;
    }
    uint64_t* getVhash() const { return &vz[0]; }
    static void kern_asm_begin( AsmFmtCols &ro_regs, AsmFmtCols &state,
            char const* client_vs=nullptr, uint32_t const seed=0 );
    static void kern_asm( AsmFmtCols &parent,
            std::string va, std::string vb, std::string vl, std::string hash );
    static void kern_asm_end( AsmFmtCols &a );
  private:
    void init(){
        FOR(i,mvl) vs[i] = i;   // const vector register
    }
    uint64_t *mem;      // to simulate vector registers
    uint64_t hashVal;   // accumulating sequence hash value
    //uint64_t j0;        // initial value of j (seed<<32)
    uint64_t j;         // incrementing sequence position.
    uint64_t *vs;       // vr[0..mvl-1] seq register (j..j+vl-1)
    // temporary registers
    uint64_t *vx;       // vx[0..mvl-1] scratch register
    uint64_t *vy;       // vx[0..mvl-1] scratch register
    uint64_t *vz;       // vz[0..mvl-1] scratch register
    uint64_t r;         // scalar scratch register
};
/** Hash a sequence of \c vector_i[0..vl[i]-1] \b and its [nonzero] \c vl partition.
 * - Properties:
 *   - The accumulating hash will depend on:
 *     1. the overall data sequence, \c vector[0..\sum_i(vl[i])-1]
 *     2. the non-zero vector length sequence vl[i]
 * - State update: (much easier if we are allowed to depend on vl)
 *   - State is vr[mvl], vx[mvl]
 *   - Init \c vr are widely dispersed numbers.
 *   - Add \c r1*(vr[vl]+v[vl]) to state \c vx[vl]
 *   - vr[vl] = vr[vl] + vl
 * - Hash value:
 *   - parallel xor of vx[0..mvl-1]
 * - Require:
 *   - all vl[i] <= MVL, where MVL is given to the constructor.
 */
class VecVlHash{
    /** helper for generic vector hashing.
     * \p maxmvl we can hash_combine up to some max vector length */
    VecVlHash( int const mvl )
        : mvl(mvl), mem(new uint64_t(3*mvl))
          //, hashVal(scramble64::r2), j(0)
          , vs(mem+0), vx(mem+mvl), vy(mem+2*mvl)
    { if(!mem) THROW("Out of memory"); init(); }
    ~VecVlHash(){
        delete[](mem);
    }
    int const mvl;
    /// return current hash value
    uint64_t u64() {
        uint64_t r = 0;
        FOR(i,mvl) r ^= vx[i];          // NB: mvl
        return r;
    }
    void hash_combine( uint64_t const* v, int const vl ){
        assert( vl >= 0 && vl <= mvl );
        FOR(i,vl) vy[i] = v[i] + vs[i];
        FOR(i,vl) vy[i] = scramble64::r2 * vy[i];   // vx = r2 * (v+vs)
        FOR(i,vl) vx[i] = vx[i] + vy[i];
        FOR(i,vl) vs[i] = vl + vs[i];
    }
  private:
    void init(){
        FOR(i,mvl) vy[i] = i;
        FOR(i,mvl) vx[i] = 0;
        FOR(i,mvl) vs[i] = 0x99999999UL*vy[i];
    }
    uint64_t * const mem;
    uint64_t *vs;       // vr[0..mvl-1] state seq register
    uint64_t *vx;       // vx[0..mvl-1] state register
    uint64_t *vy;       // vx[0..mvl-1] scratch register
    //uint64_t r;         // scalar scratch register
};
/** If most of the time you will supply sequence data at common vector length.
 * We use VMV and masked ops for occasional \c hash_combine calls with reduced vector length.
 */
class VecHashMvl {
    static int const mvl = 256;
    /** \p mvl is the usual [and limiting] vector length for incoming data.
     */
    VecHashMvl( uint32_t const seed=0 )
        : /*mvl(VecHashMvl_MVL),*/ mem(new uint64_t[mvl*4])
          , hashVal(scramble64::r2), j((uint64_t)seed<<32) //, j0(j)
          // vs,vx,vy simulate vector registers
          // (better if the memory is on separate cache pages)
          , vs(mem+0*mvl), vx(mem+1*mvl), vy(mem+2*mvl), vz(mem+3*mvl)
#ifndef NDEBUG
          , nCombine(0)
#endif
    {
        if(!mem) THROW("Out of memory");
        init((uint64_t)seed<<32);
    }
    ~VecHashMvl(){ delete[](mem); }
#if 0
    /** vl==mvl special case */
    void hash_combine( uint64_t const* v ) {
        FOR(i,mvl) vy[i] = r1 * vs[i];       // hash of vs[]=j..j+vl-1
        FOR(i,mvl) vx[i] = r2 * vx[i];       // vx *= r1
        FOR(i,mvl) vy[i] = vy[i] ^ vx[i];    // vx ^= hash_vs[]
        // stop now.  Full Merkle-Damgard would also:
        //      FOR(i,vl) vy[i] = r3 * vy[i];
        //      FOR(i,vl) vy[i] = vy[i] ^ vx[i]
        FOR(i,mvl) vs[i] += vl/*j*/;
    }
#endif
    /** If only a few times you need lower vector length,
     * OR if mvl is 256 and you have a VMV vector rotate [by 256]. */
    void hash_combine( uint64_t const* v, int const vl ){
        assert( vl > 0 && vl <= mvl ); // vl==0 OK, but wasted work.
        using namespace scramble64;
        FOR(i,vl) vy[i] = r1 * vs[i];       // hash of vs[]=j..j+vl-1
        FOR(i,vl) vx[i] = r2 * vx[i];       // vx *= r1
        FOR(i,vl) vy[i] = vy[i] ^ vx[i];    // vx ^= hash_vs[]
        // stop now.  Full Merkle-Damgard would also:
        //      FOR(i,vl) vy[i] = r3 * vy[i];
        //      FOR(i,vl) vy[i] = vy[i] ^ vx[i]
        //
        // Idea: the xor-reduction could be replaced by vector rotate.
        //    State registers become vs[] and vx[]
        if( vl<mvl ){
            uint64_t sy = vl;
            // svl old_vl
            // lvl 256 -- this might increase latency a bit.
            FOR(i,mvl) vz[i] = vx[ (sy+i)%mvl ];    // vmv vz,sy,vx [requires vz!=vx]
            // if mvl!=256, above rotation is more involved (VMV, mask, VCP)
            //
            // Note: when fully ramped up. save/restore VL becomes "just another scope"
            // save vl
            FOR(i,mvl) vx[i] = vz[i];
            FOR(i,mvl) vs[i] += vl/*j*/;
            // pop vl
            // lvl old_vl
            // But now we are maybe just as complicated as xor-reduction!
        }
    }
    /// return current hash value (non-memoized xor-reduction of \c vx)
    uint64_t u64() const {      // non-trivial finalizer
        uint64_t r = 0;
        FOR(i,mvl) r ^= vx[i];
        return r;
    }
  private:
    void init(uint64_t const seed){
        FOR(i,mvl) vx[i] = i;
        FOR(i,mvl) vs[i] = seed + vx[i];
        FOR(i,mvl) vx[i] = 0;
    }
    uint64_t *mem;      // to simulate vector registers
    uint64_t hashVal;   // accumulating sequence hash value
    //uint64_t j0;        // initial value of j
    uint64_t j;         // incrementing sequence position ~ vs[0]
    uint64_t *vs;       // vr[0..mvl-1] seq register (j..j+vl-1)
    uint64_t *vx;       // vx[0..mvl-1] state register
    // temporary registers
    uint64_t *vy;       // vx[0..mvl-1] scratch register
    uint64_t *vz;       // vz[0..mvl-1] scratch register
    uint64_t r;         // scalar scratch register
#ifndef NDEBUG
    int nCombine;       // combining one or two vectors (we can't switch)
#endif
};
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // VECHASH_HPP
