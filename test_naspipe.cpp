/** \file pstream.h piping demo
 */
#include "pstreams-1.0.1/pstream.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

/** Demonstrate \ref pstream.h usage.
 *
 * - pipe string \c ve_asm_code into \c pipe_command and get back:
 *   - stdout --> string \c disassembly
 *   - stderr --> string \c errors
 *   - pipe_command exit code --> int \c exit_status
 *   - pipe_command errno     --> int \c exit_errno
 */
void naspipe_demo(){
    // a simple example from output of 'cpp tmp_jitve_test_hello.S' ...
    // the .S file come from jit_ve_hello.c (or jitpp_hello.cpp)
    std::string ve_asm_code =
        "# 1 \"tmp_jitve_test_hello.S\"\n"
        "# 1 \"<built-in>\"\n"
        "# 1 \"<command-line>\"\n"
        "# 1 \"/usr/include/stdc-predef.h\" 1 3 4\n"
        "# 1 \"<command-line>\" 2\n"
        "# 1 \"tmp_jitve_test_hello.S\"\n"
        "# 11 \"tmp_jitve_test_hello.S\"\n"
        "    sic %s1 # %s1 is icbase, used as base ptr\n"
        "icbase:\n"
        "\n"
        "    lea %s2, strstart-icbase(,%s1) # %s2 = char* hello\n"
        "    ld1b %s3, szstring-icbase(,%s1) # %s3 = len\n"
        "    ld %s4, 0x18(,%tp)\n"
        "    lea %s0, 1 # __NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h\n"
        "    shm.l %s0, 0x0(%s4)\n"
        "    lea %s1, 1 # fd=1 for stdout\n"
        "    shm.l %s1, 0x8(%s4)\n"
        "    shm.l %s2, 0x10(%s4)\n"
        "    shm.l %s3, 0x18(%s4)\n"
        "    monc # veos system call!\n"
        "\n"
        "    or %s0,0,(61)0 # always return \"lucky 7\"\n"
        "    b.l (,%lr) # return (no epilogue)\n"
        "\n"
        "\n"
        "szstring:\n"
        "    .byte strend-strstart\n"
        "strstart:\n"
        "    .ascii \"Hello world\"\n"
        "strend:\n"
        "    .align 3 # well, if we wanted more code...\n"
        ;
    std::string pipe_command("./naspipe.sh");
    std::string disassembly;
    std::string errors;
    int exit_status, exit_errno;
    // pipe string 've_asm_code' into 'pipe_command' and get back:
    //  - stdout --> string 'disassembly'
    //  - stderr --> string 'errors'
    //  - pipe_command exit code --> int exit_status
    //  - pipe_command errno     --> int exit_errno
    {
        using namespace redi;
        redi::pstream jitcmd(pipe_command,
                pstreams::pstdin | pstreams::pstdout | pstreams::pstderr);
        jitcmd << ve_asm_code << peof; // maybe
        {
            jitcmd.err();
            std::stringstream ss_err;
            ss_err << jitcmd.rdbuf();
            errors = ss_err.str();
        }
        {
            jitcmd.out();
            std::stringstream ss_out;
            ss_out << jitcmd.rdbuf();
            disassembly = ss_out.str();
        }
        jitcmd.close();
        exit_status = jitcmd.rdbuf()->status();
        exit_errno  = jitcmd.rdbuf()->error();
    }
    cout<<" ******* DISASSEMBLY ******** "<<endl
        <<disassembly
        <<endl;
    cout<<" *******   STDERR    ******** "<<endl
        <<errors
        <<" ******* pipe exit status = "<<exit_status<<" errno = "<<exit_errno
        <<(exit_status==0? " (All OK)": " Ouch!!!")<<endl;
    cout<<"\nGoodbye"<<endl;
}
int main(int,char**){
    naspipe_demo();
    return 0;
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
