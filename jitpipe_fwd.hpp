#ifndef JITPIPE_FWD_HPP
#define JITPIPE_FWD_HPP
/** \file pstream.h support to run compilation tools.
 * Must compile with at least \c -std=c++11 (uses \c nullptr)
 */
#include "pstreams-1.0.1/pstream.h"
#include <sstream>
#include <cstdio>   // tmpnam, tempnam?

/** Base class for command pipes with std::string input and outputs
 * Use std::string to do something like
 * `cat in_string | pipe_cmd 1>out_string 2>err_string`.
 *
 * After the pipe_cmd we get std::string versions of stdout and stderr.
 */
struct PstreamPipe;

/** JIT a std::string of 'C' code into a dynamic library.
 * Use this to create lib<compiler>_<basename>.so files.
 * The std::string of 'C' code is left around in a temporary file.
 * Defaults to compiling the lib with same compiler that is compiling
 * this header, but can be overridden. */
class DllPipe;

// Here are some VE-related pipe examples
struct CPPpipe;         // run std::string through C preprocessor
struct JITpipe;         // run std::string through cpp and nas (VE assembler) --> binary blob
struct HEXDUMPpipe;     // run std::string through hexdump -C
struct BLOBDISpipe;     // run std::string through binary-blob disassembly

/** non-overlapping global \c needle --> \c replace in \c haystack */
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

struct PstreamPipe {
    /** shell script that runs cpp, then nas,
     * then nobjcopy to convert the ELF object file into a binary blob
     * and puts the binary blob onto stdout. */
    PstreamPipe( std::string pipe_command )
        : out(), err(), status(-1), error(-1)
          , pipe_command(pipe_command)
    {}
    /** getter for the pipe command */
    std::string cmd() const {return pipe_command;}

    /** Run the \c pipe_command, getting it \c out and \c err.compile \c cpp_asm_code into a binary blob.
     * Sets this->out (binary blob), this->status (exit status),
     * this->err (stderr), and this_errno.
     * \return exit status of \c pipe_command
     *
     * You can supply an empty string to pipe 'nothing' into \c pipe_command
     */
    virtual int run( std::string cpp_asm_code );

    /** \group JITpipe output */
    //@{
    std::string out; ///< binary blob of machine code, output of this->command
    std::string err; ///< stderr of this->command
    int status;
    int error;
    //@}
  protected:
#if 0
    /** derived classes may want to dynamically generate the 'pipe' command */
    int run( std::string new_pipe_command, std::string cpp_asm_code ){
        pipe_command = new_pipe_command;
        this->run(cpp_asm_code);
    }
#endif

    std::string const pipe_command;
};

inline int PstreamPipe::run( std::string cpp_asm_code )
{
    // pipe string 've_asm_code' into 'pipe_command' and get back:
    //  - stdout --> string 'disassembly'
    //  - stderr --> string 'errors'
    //  - pipe_command exit code --> int status
    //  - pipe_command errno     --> int error
    using namespace redi;
    redi::pstream jitcmd(pipe_command,
            pstreams::pstdin | pstreams::pstdout | pstreams::pstderr);

    // "cat" cpp_asm_code into the pipe
    jitcmd << cpp_asm_code << peof;

    {
        jitcmd.err();
        std::stringstream ss_err;
        ss_err << jitcmd.rdbuf();
        this->err = ss_err.str();   // stderr --> std::string
    }
    {
        jitcmd.out();
        std::stringstream ss_out;
        ss_out << jitcmd.rdbuf();
        this->out = ss_out.str();   // stdout --> std::string
    }
    jitcmd.close();                 // retrieve exit status and errno
    this->status = jitcmd.rdbuf()->status();
    this->error  = jitcmd.rdbuf()->error();
    return status;                  // return exit status, OK == 0
};

struct CPPpipe : public PstreamPipe {
    CPPpipe() : PstreamPipe("cpp -") {}
};
/** This pipe invokes a shell script that runs cpp, then nas,
 * then nobjcopy to convert the ELF object file into a binary blob
 * and puts the binary blob onto stdout. */
struct JITpipe : public PstreamPipe {
    JITpipe() : PstreamPipe("./vejitpage.sh") {}
};

/** invoke a 'hexdump -C' pipe. */
struct HEXDUMPpipe : public PstreamPipe {
    HEXDUMPpipe() : PstreamPipe("hexdump -C") {}
    // alternatively, you can run a short bash script with "-c":
    //HEXDUMPpipe() : PstreamPipe("bash -c 'cat - > foo; hexdump -C foo'") {}
};

/** invoke a 'binary blob disassemble' pipe.
 * Interesting because it shows how several bash commands can be used,
 * as well as mktemp to avoid tmp file name clashes. */
struct BLOBDISpipe : public PstreamPipe {
    BLOBDISpipe() : PstreamPipe(
            multiReplace( "tmpBLOB", std::tmpnam(nullptr),
                "bash -c 'cat > tmpBLOB; "
                "nobjdump -b binary -mve -D tmpBLOB; "
                "rm -f tmpBLOB'" ))
            {}
};

/** Unlike PstreamPipe, the DllPipe constructor also causes the commands to be run.
 * Because the command is dynamically constructed, we do have a 'run' function
 * like PstreamPipe.  So construct GccDllPipe as needed and remember the libnname
 * for you to use in dlopen, etc.
 * \post tmp C-code file cleaned up, unless there was an error
 * \post lib<compiler>_<basename>.so exists, actual name in \c lib() or \c libFullPath().
 *
 * \throw on any error, and for compilation errors append all info to end of tmp file,
 *        which is not removed.
 * 
 * TODO: want a 'C' version fo this, extern "C" { dllPipe_nclang(basename, ccode); }
 *       to create clanc 'C' libbasename.so JIT.
 *       [and maybe some other compiler-specific ones: gcc|ncc|clang? C|C++?]
 */
class DllPipe {
  public:
    /** \group helpers */
    /** many details and long constructor moved to jitpipe.hpp */
    //@{
    enum CC { gcc, ncc, clang, nclang, ncompilers/*last real compiler*/
        , /* other ways to set compiler */ same /* default */, env };
    //static std::array<std::string,CC::ncompilers> const compilers;

    /** just for completeness, "/" virtually always. */
    static constexpr char const* pathSep();

    /** return current working directory (#include <filesystem> is c++17) */
    static std::string getPath();

    /** Help the easy constructor convert a cross-compile hint
     * into a cross-compiler string (can include some args) */
    static std::string getCompiler( CC compiler );
    //@}

    /** easy compiler choice ~ same, or select with enum for ENV ${CC}, or gcc or ncc.
     * delegating constructor. Default compiler is \c same <em>as current</em>.
     *
     * Ex. Using clang with VE target can be forced with `DllPipe("foo", ccode, nclang)`
     *    so you can emit 'C' code using the llvm builtins for VE vector ops.
     *    I think the library name [without path, as \c lib()] would be something like
     *    libve_foo.so
     *
     * \c v is verbosity [0,1,2]. */
    DllPipe(std::string basename, std::string ccode, enum CC compiler=same, int const v=2)
        : DllPipe(basename, ccode, getCompiler(compiler), v)
    {}
    /** Use gcc to compile \c ccode into \c lib<how><B>basename</B>.so .
     * - where \em how indicates the cross-compiler.
     * - \c basename is the supplied string (describe the kernel function)
     * - \c ccode is the 'C' program.
     * - \c compiler_string allows you to use a custom cross-compiler,
     *   but typically you will use the other constructor giving an
     *   optional enum \c CC value.
     * - Usually you will use the other constructor, unless you want to supply
     * - special compilers, or compiler arguments.
     * - \c v is verbosity [-1,0,1,2].
     * - NEW: this function is long. moved to \ref jitpipe.hh impl file
     */
    DllPipe(std::string basename, std::string ccode,
            std::string compiler_string, int const v=2);

    ~DllPipe(){}
    /** return the library name, lib<B>basename</B>.so */
    std::string lib() { return libname; }
    std::string libFullPath() { return libname.empty()? libname: outDir+pathSep()+libname; }

  private:
    /** Set a tmp filename into ccode_tmpfile.
     * could be common code, if base and suffix were args, and we returned a non-empty string */
    void mkTmpfile(std::string code);

    std::string outDir;         ///< output directory "." (but could use getPath())
    std::string basename;
    std::string libname;
    std::string ccode_tmpfile;
    std::string cc;             ///< compiler (possible with options?)
};
// I'd like it possible to be header-only...
//std::array<std::string,DllPipe::CC::ncompilers> const
//DllPipe::compilers = {"gcc", "ncc"};

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // JITPIPE_FWD_HPP
