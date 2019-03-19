/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#ifndef EXECHASH_HPP
#define EXECHASH_HPP
#include "../r64.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <functional>   // hash

/** An order-sensitive running hash, representing an execution path.
 * If you want some insensitivity to number of mark executions,
 * supply maxMark. You can add integers or strings.   A trace and
 * mark-->string are maintained in case you wish to interpret or
 * print the trace definition. */
struct ExecHash {
    ExecHash(std::size_t maxMark=3) : v(0), r(0x55), maxMark(maxMark)
                                      , hint(), hstr()
                                      , markCnt(), markVec(), markStr() {}
    /** You could add raw integer values */
    ExecHash& operator += (std::size_t const marker){
        auto const cnt = ++markCnt[marker];
        if( cnt <= maxMark ){
            mixin(marker);
            markVec.push_back(marker);
        }
        return *this;
    }
    /** But it's nicer to use strings to uniquely mark the execution path */
    ExecHash& operator += (std::string const s){
        auto const marker = hstr(s);
        auto const cnt = ++markCnt[marker];
        if( cnt <= maxMark ){
            mixin(marker);
            markVec.push_back(marker);
            markStr[marker] = s; // could assert it's a fresh insert
        }
        return *this;
    }
    /** \return fixed length "0xNN.." string */
    std::string hex() const {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::setfill('0') // output is fixed-length
            << std::setw(sizeof(std::size_t)*2) << v;
        return oss.str();
    }
    /** \return fixed length "ExecHash{ 0xNN.. }" string */
    std::string str() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') // output is fixed-length
            << "ExecHash{ 0x" << std::setw(sizeof(std::size_t)*2) <<v <<" }";
        return oss.str();
    }
    /** \return pretty-printed execution trace.
     * Use +=(std::string) during execution to get an interpretable \c dump(). */
    std::string dump() const { // pretty-printed (one per line) trace
        std::ostringstream oss;
        oss<<" execution trace dump, maxMark="<<maxMark<<", hash="<<this->hex()<<"\n";
        std::map<std::size_t,std::size_t> myCnt;
        for(auto const mark: markVec){
            auto const cnt = ++myCnt[mark];
            oss<<"        "<<std::setw(2)<<cnt;
            {
                auto const found = markCnt.find(mark);
                if( found != markCnt.end() ){
                    oss<<"/"<<std::setw(2)<<std::left<<found->second<<std::right;
                }else{
                    oss<<" ??";
                }
            }
            {
                auto const found = markStr.find(mark);
                if( found != markStr.end() ){   // marks can be added as nice strings
                    oss<<" "<<found->second;
                }else{                          // or less interpretable raw integers
                    oss<<" "<<std::hex<<std::setfill('0')
                        <<mark<<std::dec<<std::setfill(' ');
                }
            }
            oss<<"\n";
        }
        oss<<"}//end trace "<<this->hex()<<"\n";
        return oss.str();
    }
    operator std::size_t() const { return v; }
    std::map<std::size_t,std::size_t> const& getCounts() const {return markCnt;}
    std::vector<std::size_t> const& trace() const {return markVec;}
private:
    /** core hashing mechanism.
     * Each \e item \c x is first mapped to 64-bit range and xor'ed.
     * Then the a \b sequential pseudo-random number is also xor'ed,
     * so that hash \c v reflects \b ordering of the added items \c x.
     */
    void mixin(std::size_t const x) {
        v ^= R64(x)();  // next pseudorand after 'x'
        v ^= (++r)();   // xor with next pseudo-rand
    }
    std::size_t v;      ///< our hash, updated via \e +=
    R64 r;              ///< sequential pseudo-random. do not change it's seed!
    std::size_t const maxMark;          ///< up to how many times do we track any location?
    std::hash<std::size_t> hint;        ///< integer hashing object
    std::hash<std::string> hstr;        ///< string hashing object
    // for debug
    std::map<std::size_t,std::size_t> markCnt;  ///< map mark-->count up to maxMark (can exceed \c maxMark)
    std::vector<std::size_t>          markVec;  ///< \em hashed marks in sequence (first \c maxMark only)
    std::map<std::size_t,std::string> markStr;  ///< marks-->string for readable \c dump()
};

inline std::ostream& operator<<(std::ostream& os, ExecHash const& execHash){
    return os << execHash.str();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // EXECHASH_HPP
