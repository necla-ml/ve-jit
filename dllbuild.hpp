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
class DllOpen{
  public:
    void* operator[](std::string const& s) const {return dlsyms.at(s);}
    ~DllOpen() {
        libHandle = nullptr;
        files.clear();
    }
  private:
    friend class DllBuild;
    std::string basename;
    void *libHandle;
    std::unordered_map< std::string, void* > dlsyms;
    // optional...
    std::string libname;            ///< full path
    std::vector<std::string> files; ///< full paths of all jit code files
    DllOpen() : basename(), libHandle(nullptr), dlsyms(), libname(), files() {}
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
/** SubDir has create-writable or throw semantics, and set absolute path */
struct SubDir{
    SubDir() : subdir(), abspath() {}
    // create subdir, then set current dir, and absolute path to subdir
    SubDir(std::string subdir);
    std::string subdir;
    // calculated
    std::string abspath;    // = cwd/subdir
};
/** basename*.{c|cpp|s|S} compilable code file */
struct DllFile {
    std::string basename;
    std::string suffix;             ///< *.{c|cpp|s|S}
	std::string code;
    std::vector<SymbolDecl> syms;   ///< just the public API symbols
    // optional...
	std::string comment;
    /** write comment+code to <subdir.abspath>/<basename><suffix>.
     * \return \c abspath */
    std::string  write(SubDir const& subdir);
    static std::string obj(std::string fname);   ///< %.{c,cpp,s,S} --> %.o \throw on err
    std::string const& getFilePath() const {return this->abspath;}
  private:
    std::string objname;        ///< set by \c write
    friend class DllBuild;
    std::string abspath;
};
/** create empty, append various DllFile, then \c create() the DllOpen.
 *
 * \note This is intended for 'C' jit programs, but can handle assembler
 * until you get a real JIT assembler to bypass all filesystem operations.
 */
struct DllBuild : std::vector<DllFile> {
    DllBuild() : std::vector<DllFile>(), prepped(false), made(false),
    dir(), basename(), libname(), mkfname(), fullpath()
    {}
    /** For testing -- create build files (ok for host- or cross-compile).
     * /post \c dir is left with all files necessary to build the library
     *       via 'make', as well as a header with any known fwd decls.
     * If cross-compiling, stop after 'prep' or 'make' stage, because you
     * can't run any dll code. */
    void prep(std::string basename, std::string dir=".");
    /** For testing -- . \pre you have all the [cross-]compiling tools. */
    void make();
    /** open and load symbols, \throw if not \c prepped and \c made */
    DllOpen create(){ return dllopen(); }
    /** \c prep, \c make and load all public symbols (JIT scenario).
     * Use this when caller is able to execute the machine code in the dll
     * (i.e. VE invoking host cross-compile for VE target). */
    DllOpen create(std::string basename, std::string dir="."){
        if(!prepped){prep(basename,dir); prepped=true;}
        if(!made){make(); made=true;}
        return dllopen();
    }
    /** return \c libname, or \throw if not \c prepped */
    std::string const & getLibName() const;
  private:
    DllOpen dllopen();
    bool prepped;
    bool made;
    SubDir dir;
    std::string basename;
    std::string libname;        ///< libbasename.so
    std::string mkfname;        ///< basename.mk
    std::string fullpath;       ///< absolute path to libname
};
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
