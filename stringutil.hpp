#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP
/** \file
 * pure-header std::string stuff */
#include "throw.hpp"
#include <cstdio>   // tmpnam, tempnam?
#include <unistd.h> // close

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

template<typename T>
std::string jitdec(T const t){
    std::ostringstream oss;
    oss << t;
    return oss.str();
}
template<typename T>
std::string jithex(T const t){
    std::ostringstream oss;
    oss << "0x" << std::hex << t << std::dec;
    return oss.str();
}
inline std::string hexdec(int64_t const i){
    return (i>-1000000 && i<1000000
            ? jitdec(i)
            : jithex(i));
}

/** open and close a tmp file from \c mkstemp, returning its name. */
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
