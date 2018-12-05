#ifndef VTYPES_HPP_
#define VTYPES_HPP_
#include <stdexcept>
#include <sstream>
#include <vector>
#include <limits>

#include <cstdint>

#ifndef MVL
#define MVL 256
#endif

/** Indicate possibly restricted range.
 * * lo == hi == 0 (unbounded)
 * * lo <  hi      valid range is lo .. hi INCLUSIVE
 * * lo >= hi      illegal, or never-OK ?
 */
struct Ubound {
    uint64_t const lo;
    uint64_t const hi;
    constexpr Ubound(): lo(0), hi(0) {}
    explicit Ubound(uint64_t const hi): lo(0), hi(hi) {}
    explicit Ubound(uint32_t const hi): lo(0), hi(hi) {}
    explicit Ubound(int64_t const hi): lo(0), hi(hi){
        if( hi < 0L ) oops(__FUNCTION__,"hi must be +ve");
    }
    explicit Ubound(int32_t const hi): lo(0), hi(hi){
        if( hi < 0  ) oops(__FUNCTION__,"hi must be +ve");
    }
    inline constexpr bool ok(uint64_t const i) const { return hi>lo? i>=lo && i<=hi: true; }
    inline constexpr bool ok(uint32_t const i) const { return hi>lo? i>=lo && i<=hi: true; }
    inline bool ok( int64_t const i) const {
        if (i<0) oops(__FUNCTION__,"i must be +ve");
        return hi>lo? (i>=lo && i<=hi) : true;
    }
    inline bool ok( int32_t const i) const {
        if (i<0) oops(__FUNCTION__,"i must be +ve");
        return hi>lo? i>=lo && i<=hi: true;
    }
private:
    bool oops(char const* fn, char const* msg) const {
        std::ostringstream oss;
        oss<<fn<<" "<<msg;
        throw std::out_of_range(oss.str());
        return false;
    }
};
struct Ibound {
    int64_t const lo;
    int64_t const hi;
    constexpr Ibound(): lo(0), hi(0) {}
    explicit Ibound(uint64_t const hi): lo(0), hi(hi) {}
    explicit Ibound(uint32_t const hi): lo(0), hi(hi) {}
    explicit Ibound(int64_t const hi): lo(0), hi(hi) {}
    explicit Ibound(int32_t const hi): lo(0), hi(hi) {}
    inline constexpr bool ok(uint64_t const i) const {
        if(lo<0||hi<<0) oops(__FUNCTION__,"Ibound(u64) with a -ve bound");
        return i >= lo && i <= hi; }
    inline constexpr bool ok(uint32_t const i) const {
        if(lo<0||hi<<0||hi>(int64_t)std::numeric_limits<uint32_t>::max()) oops(__FUNCTION__,"Ibound(u32) with inappropriate bound");
        return i >= lo && i <= hi; }
    inline bool ok( int64_t const i) const { return i>=lo && i<=hi; }
    inline bool ok( int32_t const i) const { return i>=lo && i<=hi; }
private:
    bool oops(char const* fn, char const* msg) const {
        std::ostringstream oss;
        oss<<fn<<" "<<msg;
        throw std::out_of_range(oss.str());
        return false;
    }
};
class Vu64{
class Vu64{
public:
    typedef uint64_t type;
    typedef std::vector<type> vt;
    std::vector<type>  v;
    int vl; ///< 0..MVL (0 ~ unusable)
    /** Model a vector register of uint64_t with max = initial size \c vlen */
    Vu64(int const vlen=MVL) : v(vlen), vl(vlen) {}
    vt::reference       operator[]( vt::size_type pos )      { return v[pos]; }
    vt::const_reference operator[]( vt::size_type pos )const { return v[pos]; }
    vt::reference       at( vt::size_type pos )      { if(pos>=vl)oops(__FUNCTION__,""); return v.at(pos); }
    vt::const_reference at( vt::size_type pos )const { if(pos>=vl)oops(__FUNCTION__,""); return v.at(pos); }
    vt::size_type       size()     const noexcept { return vl; }
    vt::size_type       capacity() const noexcept { return v.size(); }
private:
    bool oops(char const* fn, char const* msg) const {
        std::ostringstream oss;
        oss<<fn<<" "<<msg;
        throw std::out_of_range(oss.str());
        return false;
    }
};
struct Vi64{
    typedef int64_t type;
    std::vector<type>  v;
    int vl; // 0..256
    Vi64(int const vlen) : v(vlen), vl(vlen) {}
};
struct Vu32{
    typedef uint32_t type;
    std::vector<type>  v;
    int vl; // 0..256
    Vu32(int const vlen) : v(vlen), vl(vlen) {}
};
struct Vi32{
    typedef uint32_t type;
    std::vector<type>  v;
    int vl; // 0..256
    Vi32(int const vlen) : v(vlen), vl(vlen) {}
};
// vim: sw=4 ts=4 et cindent ai cino=^=lg\:0,j1,(0,wW1 formatoptions=croql syntax=cpp.doxygen
#endif // VTYPES_HPP_
