#ifndef __VE_DIVMOD_HPP
#define __VE_DIVMOD_HPP
#include <cstdint>

namespace cprog
{
// fwd decl:
class Cblock;

/** return FASTDIV_jj(V,JJ,VOUT) macro to produce vector \c vDiv=vReg/jj
 * \c jj is a constant divisor.
 * \pre vReg has u32 values stored in a u64 vector register.
 * \pre jj>0
 * \pre if vIn_hi>0, then vReg input values are assumed to be &lt; \c vIn_hi
 * \return number of operations required.
 *
 * \c vIn_hi optional range-restriction may give better op-codes. For fused loops
 * `for(0..ii)for(0..jj)` with some max VL, you should set \c vIn_hi=ii*jj+VL,
 * and if this is < FASTDIV_SAFEMAX we'll use 2-op mul-shr instead.
 *
 * - Method:
 *   - count ops for 'struct fastdiv' method (mul, add(?), shift)
 *   - if jj is 2^N, use shift/mask (prev method finds this solution)
 *   - if op count is 3 and range restrictions met, use computeM_uB method (mul,shift)
 */
int mk_FASTDIV(Cblock& cb, uint32_t const jj, uint32_t const vIn_hi=0);

/** return DIVMOD macro to produce vectors \c vDiv=vReg/jj and \c vMod=vReg%jj where
 * \c jj is a constant divisor.
 *
 * \c vIn_hi optional range-restriction may give better op-codes. For fused loops
 * `for(0..ii)for(0..jj)` with some max VL, you should set \c vIn_hi=ii*jj+VL,
 * and if this is < FASTDIV_SAFEMAX we'll use 2-op mul-shr instead.
 *
 * \pre vReg has u32 values stored in a u64 vector register.
 * \pre jj>0
 * \return number of operations required.
 *
 * - Method:
 *   - if jj is 2^N, use shift/mask
 *   - else if vReg has a range restriction, check if can use computeM_uB method (mul,shift)
 *   - else use fastdiv method (mul, add(?), shift)
 * - for modulus, use either
 *   - mask for jj=2^N
 *   - else mul-sub
 * 
 * \todo
 *  NOT CONSIDERED: are DIVMOD_jj_MUL|ADD immediate constants?
 *  Would the constants be better off in register variables?
 *  Is clang avoid possible lea crap in the inner loop code for non-immediate constants?
 *  (i.e. larger-than-u32 multipliers may use 3-op load on VE, instead of minimal 2 or 1-op sequence)
 * 
 *  If lea's are bothersome, then we would try to load constants into scalar registers
 *  in some (other?) outer block instead of having local scalar regs.
 *  VE scalars are often limited range or nice-looking bitmasks.
 * 
 *  For assembly (when will I have extend asm to beef up intrinsics?)
 *  I also have optimized scalar-load code that takes at most 2 ops and might mix
 *  execution units a little better than pure-lea approaches.
 * 
 */
int mk_DIVMOD(Cblock& cb, uint32_t const jj, uint32_t const vIn_hi=0);

}//cprog::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // __VE_DIVMOD_HPP
