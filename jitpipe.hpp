#ifndef JITPIPE_HPP
#define JITPIPE_HPP
/** \file pstream.h piping demo
 * Here we demo vejitpage.sh
 */
#include "pstreams-1.0.1/pstream.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdio> // std::tmpnam

/** Base class for command pipes with std::string input and outputs
 * Use std::string to do something like
 * `cat in_string | pipe_cmd 1>out_string 2>err_string`.
 *
 * After the pipe_cmd we get std::string versions of stdout and stderr.
 */
struct PstreamPipe;

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
        : pipe_command(pipe_command),
          out(), err(), status(-1), error(-1)
    {}
    /** getter for the pipe command */
    std::string cmd() const {return pipe_command;}

    /** compile \c cpp_asm_code into a binary blob.
     * Sets this->out (binary blob), this->status (exit status),
     * this->err (stderr), and this_errno.
     * \return exit status of \c pipe_command
     */
    int run( std::string cpp_asm_code );

    /** \group JITpipe output */
    //@{
    std::string out; ///< binary blob of machine code, output of this->command
    std::string err; ///< stderr of this->command
    int status;
    int error;
    //@}
  protected:
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

#endif // JITPIPE_HPP
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
