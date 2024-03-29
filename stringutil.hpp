#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * pure-header std::string stuff */

#include "intutil.hpp"
#include <vector>
#include <cstdio>   // tmpnam, tempnam?
#include <unistd.h> // close
#include <sstream>
#include <iomanip>
#include <algorithm>    // std::find?

/** Assuming ostringstream named \c oss, format some stuff returning a string,
 * and flush \c oss so it can be reused.
 * \pre there exists a scratch \c oss in current scope. */
#define OSSFMT(...) (oss<<__VA_ARGS__, flush(oss))
/** format arbitrary '<<' code into ostringstream \c oss, output as a line
 * to CBLOCK, and flush \c oss so it can be reused.
 * \pre there exists a scratch \c oss in current scope. */
#define CBLK(CBLOCK,...) do{ oss<<__VA_ARGS__; CBLOCK>>oss.str(); oss.str(""); }while(0)
/** nicely format instruction and comment to a Cblock.
 * Ex: `INSCMT(cblk, OSSFMT("i=i+"<<increment<<";"), "jit increment")` */
#define INSCMT(BLK,INS,CMT) do{ \
    auto ins=(INS); \
    auto cmt=(CMT); \
    (BLK)>>OSSFMT(left<<setw(40)<<ins<<" // "<<cmt); \
} while(0)

/// \group simple constant outputs
//@{
/** Return current string, with side effect of emptying \c oss.
 * Aids ostringstream reuse during formatted string productions. */
inline std::string flush(std::ostringstream& oss){
    std::string s(oss.str());
    oss.str("");
    return s;
}
template<typename T>
inline std::string jitdec(T const t){
    std::ostringstream oss;
    oss << t;
    return oss.str();
}
template<typename T>
inline std::string jithex(T const t){
    std::ostringstream oss;
    oss << "0x" << std::hex << t << std::dec;
    return oss.str();
}
/** decimal for |i| less than a million; o/w hex */
inline std::string hexdec(int64_t const i){
    return (i>-1000000 && i<1000000
            ? jitdec(i)
            : jithex(i));
}
inline std::string asDec(std::size_t s){
    std::ostringstream oss;
    oss<<s;
    return oss.str();
}
//@}

/// \group string modification
//@{
/** This is q quick'n'dirty std::regex replacement.
 * To do more complex things, like replace \e word \c needle,
 * you can pull in &lt;regex&gt; and instead do something like:
 * ```std::regex_replace(haystack,std::regex("\\b"+needle+"\\b"),replace);```
 */
inline std::string multiReplace(
        const std::string needle,
        const std::string replace,
        std::string haystack)
{
    size_t const nlen = needle.length();
    size_t const rlen = replace.length();
    size_t nLoc = 0;;
    while ((nLoc = haystack.find(needle, nLoc)) != std::string::npos) {
        haystack.replace(nLoc, nlen, replace);
        nLoc += rlen;
    }
    return haystack;
}

/** Search inclusive integer range[s] \c irs [beg,end] for a string \c pfx+jitdec(ir)
 * that is \b NOT in a given \c exclude vector of strings.
 * Range \c ir is searched from beg to end (up or down).
 * \return first nonexcluded string pfxN (N = integer value) (or empty string)
 * ranges are unchecked.
 *
 * Ex. free_pfx( vs, "%s", 63, 34 ) with vs={"%s0","cat","%s63","%s60","%v61",%s62"}
 *     should return "%s61".  This example tries to return a scalar VE
 *     register that is not preserved in the VE 'C' abi.
 *
 * For an assembler 'main' routine, you might modify this to use %s0-%s7
 */
inline std::string free_pfx( std::vector<std::string> const& exclude,
        std::string pfx = "%s",
        std::vector<std::pair<int,int>> const& irs
        = std::vector<std::pair<int,int>>{{63,34}} )
{
    std::ostringstream trial;
    size_t const pfx_sz = pfx.size();
    trial<<pfx;
    for(auto const& ir: irs){
        int       i  =ir.first;
        int const end=ir.second;
        int const inc = (i<end? +1: -1);
        for(;;){
            //trial.replace( pfx_sz, std::string::npos, tostring(ir) );
            //  'tostring' is supposedly C++11, but was not supported?
            trial.clear();          // re-use the ostringstream buffer
            trial.seekp(pfx_sz);    // position the 'put' pointer
            trial<<i;
            std::string t = trial.str();
            //std::cout<<" ?"<<t;
            if(std::find(exclude.begin(),exclude.end(),t) == exclude.end()){
                // found one that is not excluded...
                //std::cout<<"\n free_pfx-->"<<t<<std::endl;
                return t;
            }
            if( i == end )
                break;
            i += inc;
        }
    }
    // client is expected deal with empty-string properly !
    return std::string();
}



/** Remove front and back whitespace.
 * \c whitespace may include null */
inline std::string trim(const std::string& str,
                        const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

/** replace internal sequences matching any chars in \c whitespace
 * with a \c fill string. \c whitespace may include null */
inline std::string reduce(const std::string& str,
                          const std::string& fill = " ",
                          const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}
//@}

/// \group VE immediate operands
//@{
/** string for VE 'M' representation of a 64-bit value.
 * 'M' representation is (M)B,
 * where M is 0..63 and B is 0|1,
 * meaning "M B's followed by not-B's".
 * \throw if \c t cannot be represented in such format
 */
template<typename T>
std::string jitimm(T const t){
    int64_t i=static_cast<int64_t>(t);
    // common and special cases
    if(i==0) return std::string("(0)1");
    if(i==-1) return std::string("(0)0");
    if(i==1) return std::string("(63)0");
    // remaing cases generic
    std::ostringstream oss;
    bool const neg = i<0;
    char const* leading_bit=(neg? "1": "0");    // string for 1|0 MSB
    if(neg) i = ~i;                             // now i is positive
    if(!positivePow2(i+1))
        THROW("jitimm("<<t<<") not representable as 64-bit N(B) N B's followed by rest not-B");
    int const trailing = popcount(i); // in |t|
    oss<<"("<<64-trailing<<")"<<leading_bit;
    return oss.str();
}
template<typename T>
bool isIval(T const t){
    typename std::make_signed<T>::type i = t;
    return i >= -64 && i <= 63;
}
template<typename T>
bool isMval(T const t){
    int64_t i=static_cast<int64_t>(t);
    // common and special cases
    if(i==0) return true;
    if(i==-1) return true;
    if(i==1) return true;
    // remaing cases generic
    //std::ostringstream oss;
    bool const neg = i<0;
    //char const* leading_bit=(neg? "1": "0");    // string for 1|0 MSB
    if(neg) i=~i;                               // now i == |t|, positive
    if(!positivePow2(i+1))
        //THROW("oops")
        return false;
    //int const trailing = popcount(i); // in |t|
    //oss<<"("<<64-trailing<<")"<<leading_bit;
    //return oss.str();
    return true;
}
template<typename T>
bool isimm(T const t){ // deprecated original name
    return isMval(t);
}
//@} VE immediate operands

/** string up-to-n first, dots, up-to-n last of vector \c v[0..vl-1] w/ \c setw(wide) */
template<typename T>
std::string vecprt(int const n, int const wide, std::vector<T> v, int const vl){
    assert( v.size() >= (size_t)vl );
    std::ostringstream oss;
    for(int i=0; i<vl; ++i){
        if( i < n ){ oss<<" "<<std::setw(wide)<<v[i]; }
        if( i == n && i < vl-n ){ oss<<" ... "; }
        if( i >= n && i >= vl-n ){ oss<<" "<<std::setw(wide)<< v[i]; }
    }
    return oss.str();
}
template<typename T>
std::string vecprt(int const n, int const wide, T* v, int const vl){
    std::ostringstream oss;
    for(int i=0; i<vl; ++i){
        if( i < n ){ oss<<" "<<std::setw(wide)<<v[i]; }
        if( i == n && i < vl-n ){ oss<<" ... "; }
        if( i >= n && i >= vl-n ){ oss<<" "<<std::setw(wide)<< v[i]; }
    }
    return oss.str();
}

/** open and close a tmp file from \c mkstemp, returning its name.
 * Cygwin would have to use _mktemp, I think. */
inline std::string my_tmpnam() {
    std::string templ("tmpXXXXXX");
    char * templ_data = const_cast<char*>(templ.data()); // c++17 for non-const 'data()'
    int fd = mkstemp(templ_data); // c11 specifies zero-termination
    if(fd==-1) THROW("issues creating temp file "<<templ);
    close(fd);
    return templ;
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif //STRINGUTIL_HPP
