#ifndef THROW_HPP
#define THROW_HPP
#include <stdexcept>
#include <iostream>
#include <sstream>
/** \file
 * C++ THROW macro that accepts '<<' print strings verbatim.
 *
 * \todo A better THROW macro would create class to throw the error, and then
 * invoke a class object.  This approach creates a separate compilation block
 * for the bulky 'ostringstream' code.  No perceptible difference to the user,
 * but replacing bulky compiler IR with a tiny function call to the local
 * 'throw' funciton no longer makes the THROWing function IR look huge.  If you
 * don't do this, then if you THROW from even a very short function, the
 * compiler may <em>absolutely refuse to inline</em> the function.  This can
 * drastically slow down your program.
 * Above trick works with older C++ compilers.  I'm not sure if using lambda
 * functions might be a more concise way to achieve the same effect.
 */
#if 0
#define THROW_PRETTY_IF(FILE,LINE,COND,MSG) do{}while(0)
#define THROW_PRETTY( FILE, LINE, MSG ) do{}while(0)
#define THROW_UNLESS( COND, MSG ) do{}while(0)
#define TODO(MSG) do{}while(0)
#define THROW(MSG) do{}while(0)
#else
//static std::runtime_error throwmsg(char const* file, int line, char const* msg){
//    ostringstream oss;
//    oss<<" ERROR: "<<file<<':'<<line<<" "<<msg<<std::endl;
//    return std::runtime_error(oss.str());
//}
#define THROW_PRETTY_IF( FILE, LINE, COND, MSG ) do{ \
    if(!(COND)){ \
        std::ostringstream oss; \
        oss<<" ERROR: "<<FILE<<':'<<LINE<<" "<<#COND<<MSG<<std::endl; \
        throw std::runtime_error(oss.str()); \
    } \
}while(0)
#define THROW_PRETTY( FILE, LINE, MSG ) do{ \
    std::ostringstream oss; \
    oss<<" ERROR: "<<FILE<<':'<<LINE<<" "<<MSG<<std::endl; \
    throw std::runtime_error(oss.str()); \
}while(0)
//#define THROW_UNLESS( COND, MSG ) THROW_PRETTY_IF(__PRETTY_FUNCTION__, __LINE__, COND, MSG)
#define THROW_UNLESS( COND, MSG ) THROW_PRETTY_IF(__FILE__, __LINE__, COND, MSG)
#define TODO(MSG) THROW_PRETTY( __PRETTY_FUNCTION__, __LINE__, "TODO: "<<MSG)
#define THROW(MSG) THROW_PRETTY( __PRETTY_FUNCTION__, __LINE__, MSG)

#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // THROW_HPP
