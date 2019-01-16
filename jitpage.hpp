#ifndef JITPAGE_HPP
#define JITPAGE_HPP
#include "jitpage.h"

#include <iostream>
#include <sstream>

inline std::ostream& operator<<(std::ostream& os, JitPage const& jp){
    std::ostringstream oss;
    oss<<" JitPage{mem="<<std::hex<<std::setfill('0')<<std::setw(10)<<jp.mem
        <<",len="<<std::dec<<jp.len<<",verbosity="<<jp.verbosity<<"}";
    return os<<oss.str();
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // JITPAGE_HPP

