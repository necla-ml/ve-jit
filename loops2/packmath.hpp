#ifndef PACKMATH_HPP
#define PACKMATH_HPP
/** \file
 * packed integer math via floating point ops.
 * Aurora has no packed-integer ops, but if the integer range
 * is [sign+] 12-bit, then floating versions of integer arithmetic
 * is \b exact.  Without overflow, ops are exact to [sign+] 24-bits.
 * So we can support many ops for <em>512-long [u]f{12|24}</em>
 * vectors.
 *
 * This \e f12 and \e uf12 support exact multiply to 24-bit results,
 * so \e libdivide style of fast division can be done on
 * <em>512-long integer-type vectors</em> which can vectorize
 * nested loop index calculations.
 *
 * On the other hand, aurora has less support for packed-vector
 * loads.  Using unpacked loads is ok if data is pairwise-packed
 * and nicely aligned, which may reduce options for loop ordering.
 *
 * Aurora has no modulo operations,
 *
 * \note:
 * Aurora has no full-precision integer multiplies.
 * i.e. u32 * u32 --> u64
 * SO ... should have a numeric class
 *        u32x which is u32 stored as u64,
 *        which is the way to get exact multiply.
 *        <em>you never need to sign-extend</em> a \e u32x
 */
#include <utility>
#include <cstdint>
#include <cassert>

/** 12-bit integer ops via floating point.
 * A packed floating point vector f[512] can do many packed integer
 * ops exactly if each float is really a uf12.
 * Simulating with float also allows new ops, since VE fixed point packed
 * ops have * +,-,cmp,max,min but \e no packed *,/,fma .
 *
 * - Speed is not a goal here.
 * - Intent is to develop and test algorithms,
 * - and later output the [vector] floating point versions.
 */
class uf12 {
    private:
        float v;
    public:
        explicit uf12(uint32_t const x) : v(x) {assert( x < 2048U );}
        uf12(uf12 const& x) : v(x.v) {chk();} // might not be nice
        //uf12(uf12      & x) : v(x.v) {chk();} // might not be nice
        //explicit uf12(uf12 const&& x) : v(x.v) {chk();} // might not be nice
        explicit uf12(float const x) : v(x) {assert(exact());}
        bool exact() const {return v == (float)(int)(v);}
        bool nice() const {return (uint32_t)v < 2048U;}
        void chk() const {assert( exact() );}
        uint32_t u32() const { return (uint32_t)v; }
        uf12 operator+(uf12 const b) const { uf12 ret{v+b.v}; ret.chk(); return ret;}
        uf12 operator-(uf12 const b) const {uf12 ret{v-b.v}; ret.chk(); return ret;}
        // TODO exact mult of uf12 should return a uf24 result (TBD)
        uf12 operator*(uf12 const b) const {uf12 ret{v*b.v}; ret.chk(); return ret;}
        // TODO fast / and %
        uf12 operator/(uf12 const b) const {uf12 ret{u32()/b.u32()}; ret.chk(); return ret;}
        uf12 operator%(uf12 const b) const {uf12 ret{u32()%b.u32()}; ret.chk(); return ret;}
        std::pair<uf12,uf12> divmod(uf12 const b) const;

        uf12& operator+=(uf12 const b) {v+=b.v; chk(); return *this;}
        uf12& operator-=(uf12 const b) {v-=b.v; chk(); return *this;}
        uf12& operator*=(uf12 const b) {v*=b.v; chk(); return *this;}
        uf12& operator/=(uf12 const b) {v=u32()/b.u32(); chk(); return *this;}
        uf12& operator%=(uf12 const b) {v=u32()%b.u32(); chk(); return *this;}

        uf12 operator>>(int const b) const {uf12 ret{(uint32_t)v>>b}; ret.chk(); return ret;}
        uf12 operator<<(int const b) const {uf12 ret{(uint32_t)v<<b}; ret.chk(); return ret;}
};
inline std::pair<uf12,uf12> uf12::divmod(uf12 const b) const {
    uf12 d = *this / b;
    uf12 m = *this - d*b;
    return std::pair<uf12,uf12>(d,m);
}

#ifdef MAIN
#include <iostream>
using namespace std;
int main(int,char**){
    uf12 x(123U);
    uf12 y(7U);
    uf12 z = x + y;
    assert( z.u32() == 130U );
    assert( (uf12(2U) - uf12(5U)).u32() == uint32_t(-3) ); // underflow as expected
    assert( (uf12(2047U) - uf12(2047U)).u32() == 0U );
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif
#endif // PACKMATH_HPP
