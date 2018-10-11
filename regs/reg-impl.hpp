#ifndef REG_IMPL_HPP
#define REG_IMPL_HPP
#error "This file has been deprecated"
/** \file
 * Some key \ref reg-base.hpp features (like constructors) can only be defined AFTER
 * a chipset-specific header such as \reg ref-aurora.hpp defines at least some of the
 * chipset-specific forward declarations in reg-base.hpp.
 *
 * Those functions are here!
 */

// there is only one impl for now, Aurora...
// in practice you could adjust this file for different register-set impls
#include "reg-aurora.hpp"

#if 0 // this is chip-specific. NO NO NO
inline std::underlying_type<RegisterBase::Vlen>::type RegisterBase::vlen() const {
    return flags.vlen.z() != Int(Vlen::def)
        ? static_cast<std::underlying_type<Vlen>::type>(flags.vlen.z())
        : defMaxVlen(this->rid);
}
#endif

#if 0
/** special registers can be declared with preset Flags settings */
inline RegisterBase::RegisterBase( RegId rid, RegFlags const&& f )
    : rid(valid(rid)? rid: invalidReg() ),
    flags(f)
{}

inline RegisterBase::RegisterBase( RegId rid )
    : rid(rid), flags(defRegFlags(rid)) // throw if !valid(rid)
{}
#endif

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
#endif // REG_IMPL_HPP

