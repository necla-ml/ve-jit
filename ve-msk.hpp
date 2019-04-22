/** \file
 * compiles with c++17 (remove asserts from constexpr for c++11)
 */
#include <cstdint>
#include <cassert> // note: c++11 constexpr may not allow assert
#include <algorithm>    // std::min
#include <iosfwd>


/** VE bitmask helper. VE bitmasks are 4 uint64, where each is bit-reversed.
 * For now, this is only C++ (no asm). */
struct Msk256 {
    uint64_t m[4];
    static int const bits = 256; // C++11 allows
    Msk256() { m[0]=0UL; m[1]=0UL; m[2]=0UL; m[3]=0UL; }
    static bool constexpr iok(int i) { return (i & ~255) == 0; }

#if defined(__ve) && __cplusplus >= 201402L
#define CONSTEXPR_SUPPORT 2

#elif defined(__ve)
#define CONSTEXPR_SUPPORT 1

#elif defined(__GNUC__)
#if __GNUC__ > 7 || ( __GNUC__ == 7 && __GNUC_MINOR__ >= 2 )
#define CONSTEXPR_SUPPORT 2 /* refine when I can check this */
#else
#define CONSTEXPR_SUPPORT 0
#endif

#else
#warning "ve-msk.hpp with new compiler -- please add this compiler to known constexpr support cases"
#define CONSTEXPR_SUPPORT 0 /* please refine this as more compiler info becomes available */
#endif

#if CONSTEXPR_SUPPORT==0 // safest, ok even with g++-4.8.5
    bool get(int i) const {
        return m[i/64] & (uint64_t{1}<<(63-i%64)); }
    bool get_(int i) const {
        return m[i/64] & (uint64_t{1}<<(63-i%64)); }
#elif CONSTEXPR_SUPPORT==1 // nc++ -std=c++11 can accept this
    bool constexpr get(int i) const {
        return m[i/64] & (uint64_t{1}<<(63-i%64)); }
    bool constexpr get_(int i) const {
        return m[i/64] & (uint64_t{1}<<(63-i%64)); }
#elif CONSTEXPR_SUPPORT==2 // nc++ -std=c++14 is OK with assert in constexpr
    bool constexpr get(int i) const {
        assert(iok(i));
        return m[i/64] & (uint64_t{1}<<(63-i%64)); }
    bool constexpr get_(int i) const {
        assert(iok(i));
        return m[i/64] & (uint64_t{1}<<(63-i%64)); }
#endif
    void set() { m[3]=m[2]=m[1]=m[0]= ~0UL; }
    void clr() { m[3]=m[2]=m[1]=m[0]=  0UL; }
    void set(int const i) { if(iok(i)){ set_(i); }}
    void set_(int const i) {
        int s=i/64, r=63-i%64;
        m[s] |= (uint64_t{1}<<r); }
    void clr(int const i) { if(iok(i)){ clr_(i); }}
    void clr_(int const i) {
        int s=i/64, r=63-i%64;
        m[s] &= ~((uint64_t{1}<<r)); }
    /** If b<e and by>0, bump -ve b up by \c by until >=0, and then
     * set bits b+N*by for N=0,1,etc while < min(e,256).
     * Otherwise do nothing. */
    void set(int b, int e, int const by=1){
        if(b<e && by>0){
            while(b<0) b+=by;
            e = std::min(e,256);
            set_(b,e,by);
        }}
    void set_(int b, int const e, int const by=1){
        for( ; b<e; b+=by){
            int s=b/64, r=63-b%64;
            m[s] |= uint64_t{1}<<r; }}
    void clr(int b, int e, int by=1){
        if(b<e && by>0){
            while(b<0) b+=by;
            e = std::min(e,256);
            clr_(b, e, by);
        }}
    void clr_(int b, int const e, int const by=1){
        for(; b<e; b+=by){
            int s=b/64, r=63-b%64;
            m[s] &= ~(uint64_t{1}<<r); }}
    /** Set every n'th bit, for n>0, by default starting with bit b [default n-1].
     * Other bits remain unchanged.
     * Can begin at given bit \c b if you want.
     * -ve b is bumped to a nonnegative value be additions of 256.
     * Ignore nonsense n<=0.
     * \post ```
     * for(int i=0; i<Msk256::bits; ++i){
     *   if(i>=b && (i-b)%n==0) assert(msk.get(i)==1);
     * }
     * ```
     */
    void setevery(int n, int b=-1){
        if(n>0) { while(b<0) b+=n; setevery_(n,b); }
    }
    /** Set bits b, b+n, b+2n, etc, until b is outside [0,255].
     * Other bits remain unchanged.
     * n==0 sets bit b only. */
    void setevery_(int const n, int b){
        if(n==0) set(b);
        else for(; iok(b); b+=n){
            int s=b/64, r=63-b%64;
            m[s] |= uint64_t{1}<<r; }}
    /** Clear every n'th bit, for n>0, by default starting with bit n-1.
     * Other bits remain unchanged.
     * Can begin at given bit \c b if you want.
     * -ve b is bumped to a nonnegative value be additions of 256.
     * Ignore nonsense n<=0. */
    void clrevery(int const n, int b=-1){
        if (n>0) { while(b<0) b+=n; clrevery_(n, b); }}
    /** Clear bits b, b+n, b+2n, etc, until b is outside [0,255].
     * Other bits remain unchanged.
     * n==0 clears bit b only. */
    void clrevery_(int const n, int b){
        if(n==0) clr(b);
        else for(; iok(b); b+=n){
            int s=b/64, r=63-b%64;
            m[s] &= ~(uint64_t{1}<<r);
            //std::cout<<" c"<<b<<":"<<m[s];
        }}
    void shr(int const n){
        if(n==0) return;
        if(n<0) shl(n);
        int d=n/64, o=n%64, r=64-o, rmask=((1<<r)-1);
        if(d==0){
            m[3] = ((m[3]<<o)) | ((m[2]>>r)&rmask);
            m[2] = ((m[2]<<o)) | ((m[1]>>r)&rmask);
            m[1] = ((m[1]<<o)) | ((m[0]>>r)&rmask);
            m[0] = ((m[0]<<o));
        }else if(d==1){
            if(o==0){ m[3]=m[2]; m[2]=m[1]; m[1]=m[0];
            }else{
                m[3] = ((m[2]<<o)) | ((m[1]>>r)&rmask);
                m[2] = ((m[1]<<o)) | ((m[0]>>r)&rmask);
                m[1] = ((m[0]<<o));
            }
            m[0]=0UL;
        }else if(d==2){
            if(o==0){ m[3]=m[1]; m[2]=m[0];
            }else{
                m[3] = ((m[1]<<o)) | ((m[0]>>r)&rmask);
                m[2] = ((m[0]<<o));
            }
            m[1]=0UL; m[0]=0UL;
        }else if(d==3){
            if(o==0) m[3]=m[0];
            else m[3] = ((m[0]<<o));
            m[2] = 0UL;
            m[1] = 0UL;
            m[0] = 0UL;
        }
    }
    void shl(int n){
        assert(false); // there are no fast shift ops to masks (but there is a double-scalar shift op)
    }
    Msk256& operator=(Msk256 const& y) = default;
    bool operator==(Msk256 const& y) { return m[0]==y.m[0] && m[1]==y.m[1] && m[2]==y.m[2] && m[3]==y.m[3]; }
    Msk256& operator|=(Msk256 const& y) { m[0]|=y.m[0]; m[1]|=y.m[1]; m[2]|=y.m[2]; m[3]|=y.m[3]; return *this;}
    Msk256& operator&=(Msk256 const& y) { m[0]&=y.m[0]; m[1]&=y.m[1]; m[2]&=y.m[2]; m[3]&=y.m[3]; return *this;}
    Msk256& mand(Msk256 const& y) { m[0]&=y.m[0]; m[1]&=y.m[1]; m[2]&=y.m[2]; m[3]&=y.m[3]; return *this; }
    Msk256& mandnot(Msk256 const& y) { m[0]&=~y.m[0]; m[1]&=~y.m[1]; m[2]&=~y.m[2]; m[3]&=~y.m[3]; return *this; }
    Msk256& mor(Msk256 const& y) { m[0]|=y.m[0]; m[1]|=y.m[1]; m[2]|=y.m[2]; m[3]|=y.m[3]; return *this; }
    Msk256& mornot(Msk256 const& y) { m[0]|=~y.m[0]; m[1]|=~y.m[1]; m[2]|=~y.m[2]; m[3]|=~y.m[3]; return *this; }
};
/** VE packed vector ops use 2 bitmasks bit-wise interleaved,
 * Beginning in an even-numbered %vm2, %vm4, ... register.
 * %vm0 has special handling. */
struct Msk512 {
    Msk256 a, b;
    Msk512() : a(), b() {}
    static bool constexpr iok(int i) { return (i & ~511) == 0; }
    bool get(int i) const {
        //assert(iok(i));
        return (i&1)? a.get_(i>>1): b.get_(i>>1); }
    bool get_(int i) const {
        //assert(iok(i));
        return (i&1)? a.get_(i>>1): b.get_(i>>1); }
    void set() { a.set(); b.set(); }
    void clr() { a.clr(); b.clr(); }
    void set(int const i) { if(iok(i)){ set_(i); }}
    void set_(int const i){
        int x=i>>1; if((i&1)) a.set(x); else b.set(x); }
    void clr(int const i) { if(iok(i)){ clr_(i); }}
    void clr_(int const i){
        int x=i>>1; if((i&1)) a.clr(x); else b.clr(x); }
    void set(int bb, int e, int const by=1){
        if(bb<e && by>0){
            while(bb<0) bb+=by;
            e = std::min(e,512);
            set_(bb,e,by);
        }}
    void set_(int bb, int const e, int const by=1){
        for(; bb<e; bb+=by){
            int x=bb>>1; if((bb&1)) a.set_(x); else b.set_(x); }}
    void clr(int bb, int e, int const by=1){
        if(bb<e && by>0){
            while(bb<0) bb+=by;
            e = std::min(e,512);
            set_(bb,e,by);
        }}
    void clr_(int bb, int const e, int const by=1){
        for(; bb<e; bb+=by){
            int x=bb>>1; if((bb&1)) a.clr_(x); else b.clr_(x); }}
    // TODO api like Msk256 !!!
    void setevery(int const n, int bb=-1){
        if(n){ while(bb<0) bb+=n; setevery_(n,bb); }}
    void setevery_(int const n, int bb){ //bb>0 n!=0
        //std::cout<<" Msk512::setevery("<<n<<","<<bb<<") "; std::cout.flush();
        if(n) for(; iok(bb); bb+=n){
            int x=bb>>1; if((bb&1)) a.set_(x); else b.set_(x); }}
    void clrevery(int const n, int bb=-1){
        while(bb<0) bb+=n;
        clrevery_(n,bb); }
    void clrevery_(int const n, int bb){ //bb>0 n!=0
        if(n) for(; iok(bb); bb+=n){
            int x=bb>>1; if((bb&1)) a.clr_(x); else b.clr_(x); }}
};

std::ostream& operator<<(std::ostream&os, Msk256 const& m256);

std::ostream& operator<<(std::ostream&os, Msk512 const& m512);

/* vim: set sw=4 ts=4 et: */
