#ifndef JITPIPE_HPP
#define JITPIPE_HPP
/** \file pstream.h piping demo
 * Here we demo vejitpage.sh
 */
#include "pstreams-1.0.1/pstream.h"
#include "regs/throw.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdio> // std::tmpnam
#include <cassert>  // assert
#include <cstring>  // strerror
#include <fstream>

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

inline std::string multiReplace(
        const std::string search,
        const std::string replace,
        std::string subject)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}
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
 */
class DllPipe {
  public:
    /** \group helpers */
    //@{
    enum CC { gcc, ncc, clang, nclang, ncompilers/*last real compiler*/
        , /* other ways to set compiler */ same /* default */, env };
    //static std::array<std::string,CC::ncompilers> const compilers;

    static constexpr char const* pathSep() {
#ifdef _WIN32
        return "\\";
#else
        return "/";
#endif
    }
    /** return current working directory (#include <filesystem> is c++17) */
    static std::string getPath() {
        long const sz = pathconf(".",_PC_PATH_MAX); // assume we are interested cwd
        if(sz<=0) THROW("Invalid max path length?");
        char* const temp=(char*)malloc((size_t)sz);
        if(temp==nullptr) THROW("Out of memory");
        if ( getcwd(temp, sz) != 0) 
            return std::string(temp);
        int error = errno;
        switch ( error ) {
            // sz>0 alreay checked (no EINVAL)
            // PATH_MAX includes the terminating nul (no ERANGE)
          case EACCES: THROW("Access denied");
          case ENOMEM: THROW("Insufficient storage"); // is this possible?
          default: THROW("Unrecognised errno="<<error);
        }
    }

#if 0 // using 'compilers' makes header-only impl more difficult
    //std::array<std::string,DllPipe::CC::ncompilers> const
    //DllPipe::compilers = {"gcc", "ncc"};
    /** If default compiler options work for you... */
    static std::string getCompiler( CC compiler ){
        std::string ret;
        // set this->cc according to CC compilers
        if(compiler == env){ // use ${CC} from environment (if none, use 'same')
            char *ptr;
            if( (ptr=std::getenv("CC")) == nullptr ){
                compiler = same;
            }else{
                ret.assign(ptr);
            }
        }
        if( ret.empty() ){
            if(compiler == same){ // use 'same compiler as the one compiling this program'
#if defined(__ve)
                compiler = CC::ncc;
#else
                //TODO: clang
                compiler = CC::gcc;
#endif
            }
            if(compiler == CC::gcc) ret.assign(compilers[gcc]);
            else if(compiler == CC::ncc) ret.assign(compilers[ncc]);
        }
        return ret;
    }
#else
    /** If default compiler options work for you... */
    static std::string getCompiler( CC compiler ){
        //std::cout<<" 0: compiler=CC::"<<compiler<<std::endl;
        std::string ret;
        // set this->cc according to CC compilers
        if(compiler == env){ // use ${CC} from environment (if none, use 'same')
            //std::cout<<" A: compiler=CC::"<<(int)compiler<<std::endl;
            char *ptr;
            if( (ptr=std::getenv("CC")) == nullptr ){
                compiler = same;
            }else{
                ret.assign(ptr);
            }
        }
        if( ret.empty() ){
            if(compiler == same){ // use 'same compiler as the one compiling this program'
                //std::cout<<" B: compiler=CC::"<<(int)compiler<<std::endl;
#if defined(__ve)
                compiler = CC::ncc;
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
                compiler = CC::gcc;
#elif defined(__clang_major) && defined(__ve)
                compiler = CC::nclang;
#elif defined(__clang_major)
                compiler = CC::clang; // x86 version
#else
#error "unknown DllPipe::CC compiler enum"
#endif
            }
            if(compiler == CC::gcc){
                //std::cout<<" gcc: compiler=CC::"<<(int)compiler<<std::endl;
                ret.assign("gcc");
            }else if(compiler == CC::ncc){
                //std::cout<<" ncc: compiler=CC::"<<(int)compiler<<std::endl;
                ret.assign("ncc");
            }else if(compiler == CC::clang){
                //std::cout<<" clang: compiler=CC::"<<(int)compiler<<std::endl;
                ret.assign("clang"); // maybe
            }else if(compiler == CC::nclang){
                //std::cout<<" nclang: compiler=CC::"<<(int)compiler<<std::endl;
                // TODO recompile latest github clang and test this:
                ret.assign("clang -target ve-linux -O3 -fno-vectorize"
                        " -fno-slp-vectorize -fno-crash-diagnostics");
            }
            //std::cout<<" final: compiler=CC::"<<(int)compiler<<std::endl;

            //else leave ret empty
        }
        return ret;
    }
#endif
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
    /** Use gcc to compile ccode into \c lib<CC><B>basename</B>.so, where
     * CC is the first word of the compile command (gcc|ncc|...).
     * Usually you will use the other constructor, unless you want to supply
     * special compilers, or compiler arguments.
     * \c v is verbosity [0,1,2].
     *
     * TODO: introduce a .cpp file for this long function, put into libjit1
     */
    DllPipe(std::string basename, std::string ccode, std::string compiler_string, int const v=2)
        //: outDir("."), // OK, but full path might be need for correctness sometimes
        : outDir(getPath()),
        basename(basename), libname(""), ccode_tmpfile(""), cc(compiler_string)
    {
        if(v>=2)std::cout<<"+DllPipe("<<basename<<",ccode["<<ccode.size()<<"]"
            " compiler_string=<"<<compiler_string<<">)"<<std::endl;

        // instead of passing a string to some script, we'll
        // create a temp file here (and remove it while libXX.so is made).
        mkTmpfile( ccode );
        assert( !ccode_tmpfile.empty() ); // if not, we should have thrown an error

        if(v>=2)std::cout<<" DllPipe selected compiler "<<this->cc<<std::endl;
        //std::string bare_compiler = cc.substr(0,cc.find_first_of(" \t\n;"));
        // Oh. sometimes that target is not a function of the first word ..
        // NOT a function of the compiler name (ex. clang --target ve-linux...
        // A nicer generic way might be '--version' ...
        std::string libhow("unknown"); // how was the library generated?
        {
            // set libhow to {unknown, gcc, ncc, clang or nclang}.
            try{
                std::string versionCmd(this->cc+" --version");
                PstreamPipe pVersion(versionCmd);
                pVersion.run("");
                if( pVersion.error ){
                    std::cout<<"stdout:\n"<<pVersion.out<<std::endl;
                    std::cout<<"stderr:\n"<<pVersion.err<<std::endl;
                    THROW("could not determine compiler target by "<<versionCmd);
                }else{
                    if(v>=2)std::cout<<"stdout:\n"<<pVersion.out<<std::endl;
                    if(v>=2)std::cout<<"stderr:\n"<<pVersion.err<<std::endl;
                    std::string first4 = pVersion.err.substr(0,4);
                    if(first4.empty()) first4 = pVersion.out.substr(0,4);
                    if(v>=3)std::cout<<" first 4 chars: <"<<first4<<">"<<std::endl;
                    if( first4=="gcc " || first4=="gc++" )
                        libhow = "gcc";
                    if( first4=="ncc " || first4=="nc++" )
                        libhow = "ncc";
                    else if( first4=="clan" ){
                        if( pVersion.out.find("Target: ve-") != std::string::npos )
                            libhow = "nclang";
                        else
                            libhow = "clang";
                    }
                }
            }catch(...){
                if(v>=1)std::cout<<"Trouble determining output type, assuming ";
                if(v>=2)std::cout<<" libhow = "<<libhow<<std::endl;
            }
        }

        //this->libname = "lib"+bare_compiler+"_"+basename+".so";
        this->libname = "lib"+libhow+"_"+basename+".so";
        if(v>=2)std::cout<<" Will try to create "<<this->libname<<std::endl;

        std::string cmd = this->cc+" -shared -o "+libname+" -fPIC "+ccode_tmpfile;
        if(v>=1)std::cout<<" DllPipe command: "<<cmd<<std::endl;
        PstreamPipe doit(cmd);
        // hmm. pipe error handling could be common code.
        auto const status = doit.run(std::string("")); // this "pipe" doesn't read from stdin

        if( status ){
            if(v>=-1) std::cout<<" Warning: "<<cmd
                <<"\n          returned status="<<status<<" ("<<doit.status<<")"
                <<"\n               and errno  "<<doit.error<<std::endl;
        }else{
            if(v>=2)std::cout<<" Good: "<<cmd
                <<"\n       exited properly, status="<<doit.status
                    <<" errno="<<doit.error<<std::endl;
        }
        if(v>=2){if( doit.out.size() ) std::cout<<">>> stdout:\n"<<doit.out<<std::endl;}
        if(v>=2){if( doit.err.size() ) std::cout<<">>> stderr:\n"<<doit.err<<std::endl;}
        system(("ls -l "+libname+" "+ccode_tmpfile).c_str());
        if( doit.status == 0 ){
            if(v>=2)std::cout<<" (removing the tmp file)"<<std::endl;
            //system(("rm -f "+ccode_tmpfile).c_str());
            if( remove(ccode_tmpfile.c_str()) != 0 ){ // posix 'rm' function call.
                THROW(strerror(errno));
            }
        }else{
            // create compilation error log file, appending to the '.c' file
            try{
                std::ofstream ofs(ccode_tmpfile, std::fstream::out | std::fstream::app);
                ofs<<"#if 0 // COMPILATION ERROR LOG\n"
                    <<"\nDllPipe tried to run a PstreamPipe with command:\n"
                    <<"\n\t"<<cmd<<"\n\n"
                    <<" The 'run' call returned status="<<doit.status
                    <<"-------- STDOUT --------\n"
                    <<doit.out<<"\n"
                    <<"-------- STDERR --------\n"
                    <<doit.err<<"\n"
                    <<"#endif"<<std::endl;
                ofs.close();
            }catch(...){
                std::cout<<"Problem writing compilation information to "<<ccode_tmpfile<<std::endl;
                throw;
            }
            if(v>=-1)std::cout<<" compilation info appended to "<<ccode_tmpfile<<std::endl;
        }
        // check 'libname' exists and throw if not
    }
    ~DllPipe(){}
    /** return the library name, lib<B>basename</B>.so */
    std::string lib() { return libname; }
    std::string libFullPath() { return libname.empty()? libname: outDir+pathSep()+libname; }

  private:
    /** could be common code, if base and suffix were args, and we returned a non-empty string */
    void mkTmpfile(std::string code){
        std::string base = basename;
        std::string suffix(".c");

        if( code.empty() )
            THROW(" Error: DllPipe with no 'C' code string? ");
        if( !ccode_tmpfile.empty() ){
            std::cerr<<" Did you forget to 'run' the DllPipe on temporary file "<<ccode_tmpfile<<" ?"<<std::endl;
        }
        // Write, then set this->ccode_tmpfile
        {
            //std::string tmpfile(std::tmpnam(nullptr));
            std::string tmpfile(tempnam(outDir.c_str(),"tmp")+std::string("_")+base+suffix);
            std::cout<<" DllPipe writing code to "<<tmpfile<<" ..."<<std::endl;
            try{
                std::ofstream ofs(tmpfile);
                ofs<<code<<std::endl;
                ofs.close();
            }catch(...){
                THROW("Problem creating "<<tmpfile<<" 'C'-code file)");
            }
            this->ccode_tmpfile = tmpfile;
        }
    }

    std::string outDir;         ///< output directory "." (but could use getPath())
    std::string basename;
    std::string libname;
    std::string ccode_tmpfile;
    std::string cc;             ///< compiler (possible with options?)
};
// I'd like it possible to be header-only...
//std::array<std::string,DllPipe::CC::ncompilers> const
//DllPipe::compilers = {"gcc", "ncc"};
#endif // JITPIPE_HPP
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
