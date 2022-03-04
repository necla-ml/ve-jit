#ifndef DLLBUILD_HPP
#define DLLBUILD_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * put 2 .c functions and some const data into a library,
 * with some nice structure indicating what symbols to
 * load.
 */
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>           // std::unique_ptr
#ifndef NDEBUG
#include <iostream>
#include "throw.hpp"
#endif

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
    DllOpen(DllOpen const& other) = delete;
    DllOpen& operator=(DllOpen const& other) = delete;
    /** How many tests (or source files) are in dll? */
    size_t nSrc() const {
        return dlsyms_num.size(); // equiv files.size()
    }
    /** dll generated from multiple files/tests, can retain i-->vector<symbolnames>.
     * \c i ~ which DllFile in a DllBuild, ~ which "test parameters".
     * \pre \c i in range 0..nSrc()-1 . */
    std::vector<std::string> operator[](size_t const i) const {
#ifndef NDEBUG
        if(i>=this->dlsyms_num.size()) THROW("DllOpen["<<i<<"] not present");
#endif
        return dlsyms_num[i];
    }
    int srcTag(size_t const i) const {
#ifndef NDEBUG
        if(i>=this->dlsyms_num.size()) THROW("DllOpen["<<i<<"] not present");
#endif
        return tag[i];
    }
    std::string const& srcFile(size_t const i) const {
#ifndef NDEBUG
        if(i>=this->dlsyms_num.size()) THROW("DllOpen["<<i<<"] not present");
#endif
        return files[i];
    }

    /** dll symbol name \c s --> pointer-to-item(typically function) */
    void* operator[](std::string const& s) const {
#ifndef NDEBUG
        if(dlsyms.find(s) == dlsyms.end()) THROW("DllOpen["<<s<<"] not present");
#endif
        return dlsyms.at(s);
    }
    bool contains(std::string const& s) const{return dlsyms.find(s)!=dlsyms.end();}
    std::unordered_map< std::string, void* > const& getDlsyms() const {return dlsyms;}
    std::vector<std::vector<std::string>> const& getDlsrcs() const {return dlsyms_num;}
    std::vector<std::string> const& getDlfiles() const {return files;}
    ~DllOpen();
  private:
    friend struct DllBuild;
    std::string basename;
    void *libHandle;
    /** \b always have a map symbol-->address of the \e known JIT symbols. */
    std::unordered_map< std::string, void* > dlsyms; // all, no particular order
    /// \group library and source-file data
    /// Typically one set of test params creates one JIT source file.
    /// Multiple JIT source files can be combined into library \c libname
    //@{
    std::string libname;            ///< full path
    std::vector<std::string> files; ///< full paths of all jit source files
    std::vector<int>         tag;   ///< user tag (params?) per source file
    std::vector<std::vector<std::string>> dlsyms_num; ///< known symbols of each jit source file
    //@}
    DllOpen();
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
    DllFile() : basename(), suffix(), code(), syms(), comment(), objects(), abspath() {}
    int tag;                        ///< up to user (test number? parameter set?)
    std::string basename;
    std::string suffix;             ///< *.{c|cpp|s|S}
    std::string code;
    std::vector<SymbolDecl> syms;   ///< just the public API symbols
    // optional...
	std::string comment;
    /** write comment+code to <subdir.abspath>/<basename><suffix>.
     * \return \c abspath */
    std::string  write(SubDir const& subdir);
    static std::vector<std::string> obj(std::string fname);   ///< %.{c,cpp,s,S} --> %.o \throw on err
    std::string const& getFilePath() const {return this->abspath;}
    std::string short_descr() const;
  private:
    std::vector<std::string> objects;        ///< set by \c DllBuild.prep
    friend struct DllBuild;
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
    {
#ifndef NDEBUG
        std::cout<<" +DllBuild"<<std::endl;
#endif
    }
    ~DllBuild()
    {
#ifndef NDEBUG
        std::cout<<" -DllBuild"<<std::endl;
#endif
    }
    /** debug: show some state */
    void dump(std::ostream& os);

    /**  NEW \b recommended create, but on error dump and throw.
     * \todo DllBuild `env` parm in \c DllBuild::create / \c safe_create not implemented?
     * \post return value `true` (usable DllOpen pointer).
     * */
    std::unique_ptr<DllOpen> safe_create(
            std::string basename,
            std::string dir=".",
            std::string env="");
    // (probably enough to handle random compiler segfaults by
    // just calling safe_create again)

    /** return \c libname, or \throw if not \c prepped */
    std::string const & getLibName() const;

    /// @group Individual steps
    /// You could also call individual steps yourself, for more control.
    /// For example, `safe_create` won't use `skip_prep`.
    //@{
    /** Create source files and Makefile.
     *  ok for host- or cross-compile
     * /post \c dir is left with all files necessary to build the library
     *       via 'make', and (?) a header with any known fwd decls.
     * If cross-compiling, stop after 'prep' or 'make' stage, because you
     * can't run any dll code.
     *
     * Keep one copy of duplicate DllFile (warn, or throw depending on how suspicious things seem)
     *
     * \note prepended 'all:' target to bin.mk depends on details of bin.mk!
     */
    void prep(std::string basename, std::string dir=".");
    /** If possible, re-use existing Makefile of a previous \c prep. */
    void skip_prep(std::string basename, std::string dir=".");

    /** Run 'make'.
     * \c env is prefixed to the 'make' command, and could include things like
     * CFLAGS='...' LDFLAGS='...'
     * \pre \c this->prepped and you have all the [cross-]compiling tools.
     * \post \c libname exists as file at \c fullpath
     */
    void make(std::string env="");
    /** If \c fullpath jit library exists, skip the 'make'. */
    void skip_make(std::string env="");
    /** open and load symbols, \throw if not \c prepped and \c made */
    std::unique_ptr<DllOpen> create(){ return dllopen(); }
    /** \c prep, \c make and load all public symbols (JIT scenario).
     * Use this when caller is able to execute the machine code in the dll
     * (i.e. VE invoking host cross-compile for VE target). */
    std::unique_ptr<DllOpen> create(
            std::string basename,
            std::string dir=".",
            std::string env=""){
        if(!prepped){prep(basename,dir); prepped=true;}
        if(!made){make(env); made=true;}
        return dllopen();
    }
    //@}
  private:
    /** during \c prep weed out tests that might create duplicate symbols? */
    //void remove_duplicate_files();
  private:
    std::unique_ptr<DllOpen> dllopen();
    bool prepped;
    bool made;
    SubDir dir;                 ///< build dir (set via \c prep or skip_prep)
    std::string basename;
    std::string libname;        ///< libbasename.so
    std::string mkfname;        ///< basename.mk
    std::string fullpath;       ///< absolute path to libname {dir.abspath}/{libname}
};
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // DLLBUILD_HPP
