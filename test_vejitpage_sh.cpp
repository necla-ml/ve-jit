/** \file pstream.h piping demo
 * Here we demo vejitpage.sh
 */
#include "pstreams-1.0.1/pstream.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdio> // std::tmpnam

using namespace std;

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

std::string replaceString(std::string subject,
        const std::string search,
        const std::string replace) {
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
            replaceString(
                "bash -c 'cat > tmpBLOB; "
                "nobjdump -b binary -mve -D tmpBLOB; "
                "rm -f tmpBLOB'"
                , "tmpBLOB"
                , std::tmpnam(nullptr)))
            {}
};

using namespace std;

/** run some hard-wired cpp+asm code through some pipes. */
void vejitpage_demo(){
    // here we use some output that uses 'C' macros to be more readable
    // careful! embedded \n and " need extra escaping
    std::string cpp_asm_code =
        "// auto-generated via AsmFmtCols!\n"
        "    // print out a Hello World 'debug' string as we exit\n"
        "    // relocatable no-symbols version of write(1,\"Hello world\\n\",2)\n"
        "    // This blob acts kind of like a void foo(void), in caller stack-frame\n"
        "\n"
        "    // Input: None\n"
        "    // Output: %s0, always return lucky 7\n"
        "    // Clobbers: %s0, %s1, %s2, %s3, %s4\n"
        "#   define BP         %s1               /* Base pointer for const 'here' data */\n"
        "#   define REL(L)     L-icbase(,BP)     /* relocatable address of 'here' data */\n"
        "    sic         BP                      #  %s1 is icbase, used as base ptr\n"
        "icbase:\n"
        "                // Relocatable, so const data ptr wrt bp\n"
        "    lea         %s2, REL(strstart)      #  %s2 = char* hello\n"
        "    ld1b        %s3, REL(szstring)      #  %s3 = len\n"
        "    ld          %s4, 0x18(,%tp)\n"
        "    lea         %s0, 1                  #  __NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h\n"
        "    shm.l       %s0, 0x0(%s4)\n"
        "    lea         %s1, 1                  #  fd=1 for stdout\n"
        "    shm.l       %s1, 0x8(%s4)\n"
        "    shm.l       %s2, 0x10(%s4)\n"
        "    shm.l       %s3, 0x18(%s4)\n"
        "    monc                                #  veos system call!\n"
        "\n"
        "    or          %s0,0,(61)0             #  always return \"lucky 7\"\n"
        "    b.l         (,%lr)                  #  return (no epilogue)\n"
        "\n"
        "    // const data can be stored in the I-cache too:\n"
        "szstring:\n"
        "    .byte       strend-strstart\n"
        "strstart:\n"
        "    .ascii      \"Hello world\\n\"\n"
        "strend:\n"
        "    .align      3                       #  well, if we wanted more code...\n"
        "#undef  REL\n"
        "#undef  BP\n"
        ;
    //CPPpipe jp; // easier test case...
    JITpipe jp;
    jp.run(cpp_asm_code);
    cout<<" JITpipe status="<<jp.status<<" errno="<<jp.error;
    //if( jp.err.size() )
        cout <<"  stderr:\n"<<jp.err<<" ------\n\n";
        cout<<" stdout.size() = "<<jp.out.size()<<"\n\n"; cout.flush();
    if( jp.status==0 ){
        cout<<" binary blob size = "<<jp.out.size()<<endl;
        if(1){ // first show a simple pipe to output a readable blob
            HEXDUMPpipe hd; // blob -> hexdump
            cout<<"\n I will run hd pipe command\n\t"<<hd.cmd()<<endl;
            hd.run(jp.out); // push the binary blob string into hexdump
            cout<<" HEXDUMPpipe status="<<hd.status<<" errno="<<hd.error;
            if( hd.err.size() )
                cout<<"  stderr:\n"<<hd.err<<" ------\n";
            cout<<" stdout: ("<<hd.out.size()<<" bytes)\n"
                <<hd.out;
        }
        if(1){ // now show a trickier pipe disassembling the blob using bash (no shell script)
            BLOBDISpipe hd; // blob -> disasm via tmp file
            cout<<" I will run hd pipe command\n\t"<<hd.cmd()<<endl;
            hd.run(jp.out); // push the binary blob string into hexdump
            cout<<" BLOBDISpipe status="<<hd.status<<" errno="<<hd.error;
            if( hd.err.size() )
                cout<<"  stderr:\n"<<hd.err<<" ------\n";
            cout<<" stdout: ("<<hd.out.size()<<" bytes)\n"
                <<hd.out;
        }
    }
}
int main(int,char**){
    cout<<" This program takes a std::string of cpp + VE assembly code\n"
        <<" and runs it through a shell script that acts like a pipe:\n"
        <<"     std::string input --> script (pipe)\n"
        <<"                       --> stdout+stderr also as std::string\n"
        <<" We first run vejitpage.sh to get a binary blob of VE machine code.\n"
        <<" Then we pass that string through the standard hexdump pipe,\n"
        <<" and then through a disassemble-binary-blob pipe.\n"
        <<"\n"
        <<" The disassembly pipe is interesting because it shows how you\n"
        <<" might run a multiline bash script entirely from C++. The example creates\n"
        <<" and cleans up a temporary file (because the disassembler cannot accept\n"
        <<" input from stdin).\n"
        <<" While using a script allows you to modify the script without\n"
        <<" recompiling, you can hardwire a script into your C++ code.\n"
        <<endl;
    vejitpage_demo();
    return 0;
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
