#ifndef ASMFMT_HPP
#define ASMFMT_HPP
#include "asmfmt_fwd.hpp"
#include <sstream>
#include <deque>
#include <iostream>

#if ASMFMTREMOVE < 1
inline std::string AsmFmtCols::str() const {
    return (*a).str();
}

template<typename PAIRCONTAINER> inline
std::size_t AsmFmtCols::scope( PAIRCONTAINER const& pairs, std::string block_name ){
    std::deque<std::string> un;
    {
        bool name_out = false;
        std::string comment("{ BEG ");
        comment.append(block_name);
        auto const iend = pairs.end();
        auto       iter = pairs.begin();
        for( ; iter != iend; ++iter ){
            if( !name_out ){
                (*a) << this->fmt_def(iter->first, iter->second, comment);
                name_out = true;
            }else{
                (*a) << this->fmt_def(iter->first, iter->second);
            }
            un.push_back(iter->first);
        }
        if(!name_out) rcom(comment);
    }
    // we will reverse the order for #undefs
    AsmFmtCols undefs;
    {
        bool name_out = false;
        std::string comment("} END ");
        comment.append(block_name);
        auto const uend = un.crend();
        auto       uter = un.crbegin();
        for( ; uter != uend; ){
            auto undef = *uter;
            ++uter;
            if( uter == uend ){
                undefs.undef(undef,comment);
                name_out = true;
            }else{
                undefs.undef(undef);
            }
        }
        if(!name_out) rcom(comment);
    }
    // move the undefs string [a bunch of #undef lines] onto our scope-stack
    stack_undefs.push(undefs.flush());
    //std::cout<<"\nscope-->undefs:\n"<<stack_undefs.top()<<std::endl;
    return un.size();
}
#endif //ASMFMTREMOVE < 1

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // ASMFMT_HPP
