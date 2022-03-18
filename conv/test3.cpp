#include <iostream>
#include <iomanip>
#include <regex>
#include <string>
#include <iterator>
#include <map>
#include <cassert>

using namespace std;

// replace 1st exact match of 'search'
std::string& findReplace(std::string &data, std::string search, std::string replaceStr){
    if(data.find(search) != std::string::npos) {
        data.replace(data.find(search), search.size(), replaceStr);
    }
    return data;
}

std::string& findReplaceAll(std::string &data, std::string search, std::string replaceStr){
    while(data.find(search) != std::string::npos) {
        data.replace(data.find(search), search.size(), replaceStr);
    }
    return data;
}
// NEW function to test:
std::string shorten_layername(std::string const& in){
#if 0
    // R code
    x = ifelse(str_detect(x,"jit"),
               str_extract(x,".*jit[^_]*"), # drop any [rare] super-long suffix
               x)
    x = sub("(.*)mb[-_](.*)",       "\\1\\2B",x)       # jit prefix to suffix char
    x = sub("(.*)unroll[-_](.*)",   "\\1\\2U",x)   # jit prefix to suffix char
    x = str_replace(x,"cjitConvFwd","FJ")
    x = sub("(.*)vecC[-._]?(.*)",   "\\1VC\\2",x)   # VC suffix
    x = sub("VCxw",                 "1VC_X",x)     # restore _xw as _X no-thread suffix
    x = sub("(.*)_xw$",             "\\1_X",x)      # change other _xw suffix to _X
    x = sub("cnvFwd[-._]?",         "F",x)          # If mixed, could use "F" for "Forward"
    # for libvednn-std things: (just in case)
    x = sub("libvednn-std:",  "std.",x)
    x = sub("dil(\\d+)_?",    "d\\1", x)
    x = sub("str(\\d+)_?",    "s\\1", x)
    x = sub("padsame_?",      "pS",   x)
    x = sub("pad(\\d+)_?",    "p\\1", x)
    x = sub("ker(\\d+)_?",    "k\\1", x)
    x = sub("_?ow2X",         "ow2X", x)
    x = sub("_ioaligned",     "al", x)

    x = make.names(x);   # replace all unrecognized chars for factor level with '.' (notably ':')
    # this leaves letters, numbers, dot and underline
    #   (and must start with letter or dot-not-followed-by-number)
    #   (and must not conflict with R reserved words)

    Example contractions:
    Fd1s1pSk3_c1024x_T
    cnvFwd-d1s2p1k4owU128_xw            --> Fd1s2p1k4owU128_X
    libvednn-std:dil1_str1_pad0_ker1_T  --> std.d1s1p0k1T
    mb-unroll_cjitConvFwd1q             --> FJ1qBU
#endif
    // shorten s
    std::string s = in;
    std::smatch m;
    // jit layer name shortenings
    if(regex_search(s, m, regex(".*jit[^_]*")))
        s = m[0];
    s = regex_replace(s, regex{"mb[-_](.*)"}, "$1B");
    s = regex_replace(s, regex{"unroll[-_](.*)"}, "$1U");
    findReplace(s, "cjitConvFwd", "FJ");
    // vecC--> VC  and a no-threads _X suffix
    s = regex_replace(s, regex{"vecC[-._]?(.*)"}, "VC$1");
    //findReplace(s, "VCxw", "VC_X");
    s = regex_replace(s, regex{"VCxw$"}, "_X");             // VCxw suffix --> _X
    s = regex_replace(s, regex{"_xw$"}, "_X");              // _xw suffix --> _X
    // forward convolutions begin with 'F'
    s = regex_replace(s, regex{"cnvFwd[-._]?"}, "F");       // cnvFwd (beginning?) --> F
    // for libvednn-std shortenings: (just in case)
    findReplace(s, "libvednn-std:", "std.");
    s = regex_replace(s, regex{"dil(\\d+)_?"}, "d$1");
    s = regex_replace(s, regex{"str(\\d+)_?"}, "s$1");
    s = regex_replace(s, regex{"padsame_?"}, "pS");
    s = regex_replace(s, regex{"pad(\\d+)_?"}, "p$1");
    s = regex_replace(s, regex{"ker(\\d+)_?"}, "k$1");
    s = regex_replace(s, regex{"_?ow2X"}, "ow2X");
    s = regex_replace(s, regex{"_ioaligned"}, "al");
    // 'R' make.names resembles the following char replacement
    s = regex_replace(s, regex{"[^a-zA-Z0-9._]"}, ".");
    // store shortened name
    return s;
}
/** map existing layernames to rule-based shortenings.
 * @param l long name.
 * @return map of l to short name. */
std::map<std::string, std::string> mkLayerNameMap( std::vector<std::string> l )
{
    std::map<std::string, std::string> ret;
#if 0
    // R code
    x = ifelse(str_detect(x,"jit"),
               str_extract(x,".*jit[^_]*"), # drop any [rare] super-long suffix
               x)
    x = sub("(.*)mb[-_](.*)",       "\\1\\2B",x)       # jit prefix to suffix char
    x = sub("(.*)unroll[-_](.*)",   "\\1\\2U",x)   # jit prefix to suffix char
    x = str_replace(x,"cjitConvFwd","FJ")
    x = sub("(.*)vecC[-._]?(.*)",   "\\1VC\\2",x)   # VC suffix
    x = sub("VCxw",                 "1VC_X",x)     # restore _xw as _X no-thread suffix <-- Note the 'R' typo (now fixed)
    x = sub("(.*)_xw$",             "\\1_X",x)      # change other _xw suffix to _X
    x = sub("cnvFwd[-._]?",         "F",x)          # If mixed, could use "F" for "Forward"
    # for libvednn-std things: (just in case)
    x = sub("libvednn-std:",  "std.",x)
    x = sub("dil(\\d+)_?",    "d\\1", x)
    x = sub("str(\\d+)_?",    "s\\1", x)
    x = sub("padsame_?",      "pS",   x)
    x = sub("pad(\\d+)_?",    "p\\1", x)
    x = sub("ker(\\d+)_?",    "k\\1", x)
    x = sub("_?ow2X",         "ow2X", x)
    x = sub("_ioaligned",     "al", x)
    x = make.names(x);   # replace all unrecognized chars for factor level with '.' (notably ':')

    Example contractions:
    Fd1s1pSk3_c1024x_T
    cnvFwd-d1s2p1k4owU128_xw            --> Fd1s2p1k4owU128_X
    libvednn-std:dil1_str1_pad0_ker1_T  --> std.d1s1p0k1T
    mb-unroll_cjitConvFwd1q             --> FJ1qBU
#endif

    // ugly regexp compile time here, but don't care
    for(auto const& x: l){
        // shorten x
        std::string s = x;
        std::smatch m;
        if(regex_search(s, m, regex(".*jit[^_]*"))) 
            s = m[0];
        s = regex_replace(s, regex{"mb[-_](.*)"}, "$1B");
        s = regex_replace(s, regex{"unroll[-_](.*)"}, "$1U");
        findReplace(s, "cjitConvFwd", "FJ");
        s = regex_replace(s, regex{"vecC[-._]?(.*)"}, "VC$1");
        //findReplace(s, "VCxw", "VC_X");
        s = regex_replace(s, regex{"VCxw$"}, "VC_X");           // VCxw suffix --> _X
        s = regex_replace(s, regex{"_xw$"}, "_X");              // _xw suffix --> _X
        s = regex_replace(s, regex{"cnvFwd[-._]?"}, "F");       // cnvFwd (beginning?) --> F
        // for libvednn-std things: (just in case)
        findReplace(s, "libvednn-std:", "std.");
        s = regex_replace(s, regex{"dil(\\d+)_?"}, "d$1");
        s = regex_replace(s, regex{"str(\\d+)_?"}, "s$1");
        s = regex_replace(s, regex{"padsame_?"}, "pS");
        s = regex_replace(s, regex{"pad(\\d+)_?"}, "p$1");
        s = regex_replace(s, regex{"ker(\\d+)_?"}, "k$1");
        s = regex_replace(s, regex{"_?ow2X"}, "ow2X");
        s = regex_replace(s, regex{"_ioaligned"}, "al");
        // 'R' make.names is mostly similar to the following char replacement
        s = regex_replace(s, regex{"[^a-zA-Z0-9._]"}, ".");
        // store shortened name
        ret[x] = s;
    }
    return ret;
}

int main(int,char**)
{
    string text = "Quick brown fox";
    regex vowel_re("a|e|i|o|u");

    // write the results to an output iterator
    regex_replace(ostreambuf_iterator<char>(cout),
                  text.begin(), text.end(), vowel_re, "*");

    // construct a string holding the results
    cout << '\n' << regex_replace(text, vowel_re, "[$&]") << endl;

    // The main difference seems that backreferences use $1, not \1
    cout<<"\n";
    string s0("mb-unroll_cjitConvFwd123dumb_bell99x"); // jit prefix to suffix char
    string s = s0;
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" XXX\\d+-->YYY "<<right
        <<(s=regex_replace(s, regex{"XXX\\d+"}, "YYY"))<<endl;
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" bell\\d+-->b99 "<<right
        <<(s=regex_replace(s, regex{"bell\\d+"}, "b99"))<<endl;
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" subst 1st digits "<<right
        // oops, LAST group: <<(s=regex_replace(s, regex{"(.*)\\d+(.*)"}, "$1<digits>$2"))<<endl;
        <<(s=regex_replace(s, regex{"\\d+(.*)"}, "NN$1"))<<endl;
    std::smatch m;
    if(regex_search(s, m, regex{".*Fwd[^_]*"})){  // regex_match matches ENTIRE s
        //cout<<" (prefix) "<<m.prefix();
        //cout<<" (match) "<<m[0];
        //cout<<" (suffix) "<<m.suffix();
        s = m[0];
    }
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" remove _.* suffix"<<right
        <<s<<endl;
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" mb-->suffix "<<right
        //<<(s=regex_replace(s0, regex{"(.*)mb[-_](.*)"}, "$1$2B"))<<endl;
        // BETTER: replace 1st mb_.
        // Note:   prefix copied by default
        <<(s=regex_replace(s, regex{"mb[-_](.*)"}, "$1B"))<<endl;
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" unroll-->suffix "<<right
        //<<(s=regex_replace(s, regex{"unroll[-_](.*)"}, "$1U"))<<endl;
        <<(s=regex_replace(s, regex{"unroll[-_](.*)"}, "$1U"))<<endl;
    cout<<left<<setw(50)<<s<<left<<setw(20)<<" subst FJ "<<right
        <<(findReplace(s, "cjitConvFwd", "FJ"))<<endl;

    cout<<"\n";

    vector<string> in = {
        "cjitConvFwd1q", "cjitConvFwd6", "cnvFwd-d1p0", "cnvFwd-d1p0_owU128", "cnvFwd-d1p0_owU128_xw",
        "cnvFwd-d1p0_xw", "cnvFwd-d1s1p0", "cnvFwd-d1s1p0_owU128", "cnvFwd-d1s1p0_owU128_xw", "cnvFwd-d1s1p0_xw",
        "cnvFwd-d1s1p0k3iw2XU256_ow2X_ioaligned", "cnvFwd-d1s1p0k3iw2XU256_ow2X_ioaligned_xw",
        "cnvFwd-d1s1p0k4iwU256", "cnvFwd-d1s1p0k4iwU256_xw", "cnvFwd-d1s1pS","cnvFwd-d1s1pS_xw", "cnvFwd-d1s1pSk3",
        "cnvFwd-d1s1pSk3_c1", "cnvFwd-d1s1pSk3_c1_xw", "cnvFwd-d1s1pSk3_c1024x_T",
        "cnvFwd-d1s1pSk3_c1owU128", "cnvFwd-d1s1pSk3_c1owU128_xw", "cnvFwd-d1s1pSk3_T", "cnvFwd-d1s1pSk3_xw",
        "cnvFwd-d1s1pSk5", "cnvFwd-d1s1pSk5_xw", "cnvFwd-d1s1pSk5owU128", "cnvFwd-d1s1pSk5owU128_xw",
        "cnvFwd-d1s2p1k3owU128", "cnvFwd-d1s2p1k3owU128_xw", "cnvFwd-d1s2p1k4owU128", "cnvFwd-d1s2p1k4owU128_xw",
        "cnvFwd-def", "cnvFwd-def_xw", "cnvFwd-gemm:0", "cnvFwd-gemm:0k1p0a", "cnvFwd-gemm:1", "cnvFwd-gemm:1k1p0a",
        "cnvFwd-gemm:1sw1", "cnvFwd-gemm:2", "cnvFwd-gemm:2sw1",
        "cnvFwd-owU128", "cnvFwd-owU128_T", "cnvFwd-owU128_xw", "cnvFwd-p0k1",
        "cnvFwd-p0k1_owU128", "cnvFwd-p0k1_owU128_xw", "cnvFwd-p0k1_xw",
        "cnvFwd-s1p0k1", "cnvFwd-s1p0k1_T", "cnvFwd-s1p0k1_xw",
        "cnvFwd-vecC", "cnvFwd-vecC_xw", "cnvFwd-vecC-d1p0k1", "cnvFwd-vecC-d1p0k1_xw",
        "cnvFwd-vecC-d1p0k1cU1024", "cnvFwd-vecC-d1p0k1cU1024_xw", "cnvFwd-vecC-d1s1pSk3", "cnvFwd-vecC-d1s1pSk3_xw",
        "gemm-Ref",
        "libvednn-std:dil1_str1_pad0_ker1_T", "libvednn-std:dil1_str1_padsame_ker3_c1024x_T",
        "libvednn-std:dil1_str1_padsame_ker3_T", "libvednn-std:gemm:0", "libvednn-std:gemm:0k1p0a",
        "libvednn-std:gemm:1", "libvednn-std:gemm:1k1p0a", "libvednn-std:gemm:2", "libvednn-std:mb-default",
        "libvednn-std:mb-dil1_str1_pad0", "libvednn-std:mb-dil1_str1_pad0_ker3_iw2XU256_ow2X_ioaligned",
        "libvednn-std:mb-dil1_str1_pad0_ker4_iwU256", "libvednn-std:mb-dil1_str1_pad0_owU128",
        "libvednn-std:mb-dil1_str1_padsame", "libvednn-std:mb-dil1_str1_padsame_ker3_c1",
        "libvednn-std:mb-dil1_str1_padsame_ker3_c1_owU128", "libvednn-std:mb-dil1_str1_padsame_ker5",
        "libvednn-std:mb-dil1_str1_padsame_ker5_owU128", "libvednn-std:mb-owU128", "libvednn-std:mb-vecC",
        "libvednn-std:mb-vecC_dil1_pad0_ker1", "libvednn-std:mb-vecC_dil1_pad0_ker1_cU1024",
        "libvednn-std:mb-vecC_dil1_str1_pad1_ker3", "libvednn-std:owU128_T",
        "mb-cjitConvFwd1q", "mb-cjitConvFwd6", "mb-unroll_cjitConvFwd1q", "mb-unroll_cjitConvFwd6",
        "unroll_cjitConvFwd1q", "unroll_cjitConvFwd6"
    };
    vector<string> out = {
        "FJ1q", "FJ6", "Fd1p0", "Fd1p0_owU128", "Fd1p0_owU128_X",
        "Fd1p0_X", "Fd1s1p0", "Fd1s1p0_owU128", "Fd1s1p0_owU128_X", "Fd1s1p0_X",
        "Fd1s1p0k3iw2XU256ow2Xal", "Fd1s1p0k3iw2XU256ow2Xal_X",
        "Fd1s1p0k4iwU256", "Fd1s1p0k4iwU256_X", "Fd1s1pS", "Fd1s1pS_X", "Fd1s1pSk3",
        "Fd1s1pSk3_c1", "Fd1s1pSk3_c1_X", "Fd1s1pSk3_c1024x_T",
        "Fd1s1pSk3_c1owU128", "Fd1s1pSk3_c1owU128_X", "Fd1s1pSk3_T", "Fd1s1pSk3_X",
        "Fd1s1pSk5", "Fd1s1pSk5_X", "Fd1s1pSk5owU128", "Fd1s1pSk5owU128_X",
        "Fd1s2p1k3owU128", "Fd1s2p1k3owU128_X", "Fd1s2p1k4owU128", "Fd1s2p1k4owU128_X",
        "Fdef", "Fdef_X", "Fgemm.0", "Fgemm.0k1p0a", "Fgemm.1", "Fgemm.1k1p0a",
        "Fgemm.1sw1", "Fgemm.2", "Fgemm.2sw1",
        "FowU128", "FowU128_T", "FowU128_X", "Fp0k1",
        "Fp0k1_owU128", "Fp0k1_owU128_X", "Fp0k1_X",
        "Fs1p0k1", "Fs1p0k1_T", "Fs1p0k1_X",
        "FVC",
        "FVC_X", // This was "F1VC_X" in older 'R' code due to a dtree.R typo
        "FVCd1p0k1", "FVCd1p0k1_X", "FVCd1p0k1cU1024",
        "FVCd1p0k1cU1024_X", "FVCd1s1pSk3", "FVCd1s1pSk3_X",
        "gemm.Ref",
        "std.d1s1p0k1T", "std.d1s1pSk3c1024x_T",
        "std.d1s1pSk3T", "std.gemm.0", "std.gemm.0k1p0a",
        "std.gemm.1", "std.gemm.1k1p0a", "std.gemm.2", "std.defaultB",
        "std.d1s1p0B", "std.d1s1p0k3iw2XU256ow2XalB",
        "std.d1s1p0k4iwU256B", "std.d1s1p0owU128B",
        "std.d1s1pSB", "std.d1s1pSk3c1B",
        "std.d1s1pSk3c1_owU128B", "std.d1s1pSk5B",
        "std.d1s1pSk5owU128B", "std.owU128B", "std.VCB",
        "std.VCd1p0k1B", "std.VCd1p0k1cU1024B",
        "std.VCd1s1p1k3B", "std.owU128_T",
        "FJ1qB", "FJ6B", "FJ1qBU", "FJ6BU",
        "FJ1qU", "FJ6U"
    };
    assert(in.size() == out.size());
    std::map<std::string, std::string> in2out;
    in2out = mkLayerNameMap(in);
    int err = 0;
    for(size_t i=0U; i< in.size(); ++i){
        string a = in[i];
        string m = in2out[a];
        string expected = out[i];
        cout<<left<<setw(50)<<a<<' '<<left<<setw(20)<<m;
        if (m != expected){
            ++err;
            cout<<" but expected "<<expected;
        }
        cout<<endl;
    }
    if(err) cout<<"\nWe had "<<err<<" unexpected layer name translations"<<endl;
    else cout<<"\nExcellent, implementation name shortenings were as expected"<<endl;

    // try again, via C api. verify shortening is nilpotent.
    cout<<"\n\nshorten_layername (C-ish api function)"<<endl;
    vector<string> shorter;
    for(size_t i=0U; i< in.size(); ++i){
        string a = in[i];
        string s = shorten_layername(a);
        cout<<left<<setw(50)<<a<<' '<<left<<setw(20)<<s;
        shorter.push_back(s);
        cout<<endl;
    }
    cout<<"\n\nshorten_layername (check if nilpotent)"<<endl;
    err = 0;
    for(size_t i=0U; i< in.size(); ++i){
        string a = shorter[i];
        string s = shorten_layername(a);
        cout<<left<<setw(50)<<a<<' '<<left<<setw(20)<<s;
        if( s != a ) {
            cout<<" NOT EQUAL 2nd time shortened";
            ++err;
        }
        cout<<endl;
    }
    if(err) cout<<"\nWe had "<<err<<" non-nilpotent name shortenings"<<endl;
    else cout<<"\nExcellent, name shortenings were nilpotent"<<endl;



}
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
