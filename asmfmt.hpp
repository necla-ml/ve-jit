#ifndef ASMFMT_HPP
#define ASMFMT_HPP
#include "asmfmt_fwd.hpp"
#include <sstream>
#include <deque>
#include <iostream>
#include <assert.h>

#if ASMFMTREMOVE < 1
inline void AsmFmtCols::clear()
{
    a->str("");
    a->clear();
}
inline std::string AsmFmtCols::str() const {
    return a->str();
}

inline AsmFmtCols * AsmFmtCols::setParent( AsmFmtCols* p ) {
    AsmFmtCols *ret=parent;
    parent = p;
    return ret;
}

template<typename PAIRCONTAINER> inline
std::size_t AsmFmtCols::scope( PAIRCONTAINER const& pairs, std::string const& block_name ){
    assert( stack_defs.size() == stack_undefs.size() );
    // trimmed substitution --> macro strings
    StringPairs defs;
    //std::deque<std::string> un;
    {
        bool name_out = false;
        std::string comment("{ BEG ");
        comment.append(block_name);
        if( stack_undefs.empty() ) comment.append(" (GLOBAL)");

        auto const iend = pairs.end();
        auto       iter = pairs.begin();
        std::string line;
        for( ; iter != iend; ++iter ){
            if( !name_out ){
                line = this->fmt_def(iter->first, iter->second, comment);
                name_out = true;
            }else{
                line = this->fmt_def(iter->first, iter->second);
            }
            (*a) << line;
            defs.push_trimmed(iter->first, iter->second);
        }
        if(!name_out) rcom(comment);
    }
    // create the reverse-order #undef string immediately?
    stack_undefs.push_back(defs2undefs(defs, block_name));
    stack_defs  .push_back(defs);
    //std::cout<<"\nscope-->undefs:\n"<<stack_undefs.top()<<std::endl;
    return defs.size();
}
#endif //ASMFMTREMOVE < 1

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // ASMFMT_HPP
