/** \file
 * put 2 .c functions and some const data into a library,
 * with some nice structure indicating what symbols to
 * load.
 */
#include <string>
#include <vector>
#include <unordered_map>
#include <dlfcn.h>
/** DllOpen loads void* symbols from a [jit] library.
 *
 * Path to JIT library:
 *
 * 1. assemble DllFile::code with various SymbolDecl in public API
 * 2. append various DllFile to a DllBuild
 * 3. DllOpen = DllBuild::create(basename [,directory="."]
 * 4. Add back type info to symbol and use:
 *    - Ex. int (*foo)(int const i) = (int(*)(int const)) (dllopen["foo"]);
 *    - Ex. bar_t bar = (bar) (dllopen["bar"];)
 *    - Ex. uint64_t *maskData = dllopen("maskData"); // uint64_t maskData[4]
 *
 * Q: might it eventually be better to support loading a libary and
 *    reading **all** its public symbols?
 */
struct DllOpen{
    std::string basename;
    void *libHandle;
    std::unordered_map< char const*, void* > dlsyms;
    void* operator[](char const* s) const {return dlsyms.at(s);}
    // optional...
    std::string libname;            ///< full path
    std::vector<std::string> files; ///< full paths of all jit code files
    DllOpen() : basename(), libHandle(nullptr), dlsyms(), libname(), files() {}
    ~DllOpen() {
        libHandle = nullptr;
        files.clear();
    }
};
/** single-symbol data. */
struct SymbolDecl{
    SymbolDecl(std::string symbol, std::string comment="", std::string fwddecl="")
        : symbol(symbol), comment(comment), fwddecl(fwddecl)
        {}
    std::string symbol;
    // opt.
    std::string comment;
    std::string fwddecl;
};
/** <symbol>.c input data. */
struct DllFile {
    std::string basename;
    std::string suffix;
	std::string code;
    std::vector<SymbolDecl> syms;  // just the public API symbols
    // optional...
	std::string comment;
  private:
    friend class DllBuild;
    std::string dir;
    std::string fullpath;
};
/** create empty, append various DllFile, then \c create() the DllOpen.
 *
 * \note This is intended for 'C' jit programs, but can handle assembler
 * until you get a real JIT assembler to bypass all filesystem operations.
 */
struct DllBuild : std::vector<DllFile> {
    /** For testing -- create build files (ok for host- or cross-compile).
     * /post \c dir is left with all files necessary to build the library
     *       via 'make', as well as a header with any known fwd decls.
     * If cross-compiling, stop after 'prep' or 'make' stage, because you
     * can't run any dll code. */
    void prep(std::string basename, std::string dir=".");
    /** For testing -- . \pre you have all the [cross-]compiling tools. */
    void make();
    /** \c prep, \c make and load all public symbols (JIT scenario).
     * Use this when caller is able to execute the machine code in the dll
     * (i.e. VE invoking host cross-compile for VE target). */
    DllOpen create(std::string basename, std::string dir="."){
        if(!prepped){prep(basename,dir); prepped=true;}
        if(!made){make(); made=true;}
        return dllopen();
    }
  private:
    DllOpen dllopen();
    bool prepped;
    bool made;
    std::string basename;
    std::string dir;
    std::string fullpath;
};
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
