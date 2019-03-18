/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * - rework jitve_hello to demo asmfmt on the math kernel example
 * - merge with ve_regs (symbolic registers, scopes) work should continue
 *   in a separate directory (before things get overly complicated).
 */
#include "asmfmt.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#define STR0(...) #__VA_ARGS__
#define STR(...) STR0(__VA_ARGS__)

using namespace std;
void test_hello(){
#if 0
    COMM(print out a Hello World 'debug' string as we exit)
        COMM(relocatable no-symbols version of write(1,"Hello world\n",2))
        COMM(This blob acts kind of like a void foo(void), in caller stack-frame)
        TTR(sic   %s1)
        LAB(icbase:                 # label *after* sic)
        TTR(lea   %s2, strstart-icbase(,%s1))
        TTR(ld1b  %s3, szstring-icbase(,%s1))
        TTR(ld    %s4, 0x18(,%tp)   # SHM_OFFSET)
        TTR(lea   %s0, 1            #  __NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h)
        TTR(shm.l %s0, 0x0(%s4))
        TTR(lea   %s1, 1            # fd=1 for stdout)
        TTR(shm.l %s1, 0x8(%s4))
        TTR(shm.l %s2, 0x10(%s4)    # ptr-to-bytes)
        TTR(shm.l %s3, 0x18(%s4)    # #-of-bytes)
        TTR(monc)
        TTR()
        TTR(b.l	(,%lr))
        COMM()
        COMM(const data can be stored in the I-cache too:)
        LAB(szstring:)
        TTR(.byte strend-strstart)
        LAB(strstart:)
        TTR(.ascii "Hello world\n")
        LAB(strend:)
        TTR(.align 3	# well, if we wanted more code...)
        ;
#endif
    string asmFile("tmp_jitve_test_hello.S");  // file extension required
    {
        AsmFmtCols kernel_hello; // assembler line pretty printer
        kernel_hello
            .lcom(STR(print out a Hello World 'debug' string as we exit))
            .lcom(STR(relocatable no-symbols version of write(1,"Hello world\n",2)))
            .lcom(STR(This blob acts kind of like a void foo(void), in caller stack-frame))
            .ins()
            .lcom(STR(Input: None))
            .lcom(STR(Output: %s0, always return lucky 7))
            .lcom(STR(Clobbers: %s0, %s1, %s2, %s3, %s4))
            .def("BP","%s1","Base pointer for const 'here' data")
            .def("REL(L)", "L-icbase(,BP)", "relocatable address of 'here' data")
            .ins("sic   BP","%s1 is icbase, used as base ptr")
            .lab("icbase") // comment TODO                 # label *after* sic)
            .com("Relocatable, so const data ptr wrt bp")
            .ins("lea   %s2, REL(strstart)", "%s2 = char* hello")
            .ins("ld1b  %s3, REL(szstring)", "%s3 = len")
            .ins("ld    %s4, 0x18(,%tp)", "SHM_OFFSET")
            .ins("lea   %s0, 1",    "__NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h")
            .ins("shm.l %s0, 0x0(%s4)  ")
            .ins("lea   %s1, 1         ", "fd=1 for stdout")
            .ins("shm.l %s1, 0x8(%s4)")
            .ins("shm.l %s2, 0x10(%s4) ", "ptr-to-bytes")
            .ins("shm.l %s3, 0x18(%s4) ", "#-of-bytes")
            .ins("monc","veos system call!")
            .ins()
            .ins("or %s0,0,(61)0", STR(always return "lucky 7"))
            .ins("b.l	(,%lr)", "return (no epilogue)")
            .ins()
            .lcom("const data can be stored in the I-cache too:")
            .lab("szstring")
            .ins(".byte strend-strstart")
            .lab("strstart")
            .ins(STR(.ascii "Hello world\n"))
            .lab("strend")
            .ins(".align 3","well, if we wanted more code...")
            .undef("REL")
            .undef("BP")
            ;
        auto kh = kernel_hello.flush(); // kh.str() with an implied kh.clear()
        std::ofstream ofs(asmFile);
        ofs<<kh;
        if(!ofs.good()) cout<<" Issues writing "<<asmFile<<"?\n"<<endl;
        ofs.close();
        cout<<"kernel_hello.str():   also in "<<asmFile<<"\n"<<kh<<endl;
    }
    if(0){ cout<<"asm2bin(\""<<asmFile<<"\",verbose)..."; }
    auto bytes_bin = asm2bin( asmFile, 0/*verbose*/ );                  // .S --> .bin
    if(1){ cout<<" binary blob of "<<bytes_bin<<" bytes"<<endl; }
    cout<<"ExecutablePage page( fname_bin(\""<<asmFile<<"\")"<<endl;
    // NO: segfaults on ve: ExecutablePage page( fname_bin(asmFile) );  // .bin --> codepage
    string binFile = fname_bin(asmFile);
    ExecutablePage page( binFile );   // .bin --> codepage
    {
        printf(" going to bsic into page @ %p\n",page.addr()); fflush(stdout);
        printf(" tmp_kernel_hello takes no [register!] args\n"); fflush(stdout);
        unsigned long ret;
        void *page_addr = page.addr();
        asm("### AAA");
        asm("ld %s12, %[page]\n"
                "bsic %lr,(,%s12)\n"
                "or %[ret],0,%s0 # // move asm %s0 return value into C register of 'ret'\n"
                :[ret]"=r"(ret)
                :[page]"m"(page_addr)
                :"%s12","%s0","%s1","%s2","%s3","%s4");
        // Notes:
        //      illegal to clobber %lr, apparently:)
        //      ncc silently ignores the assembler code if you try [page]"m"(page.addr())
        asm("### BBB");
        printf("... and we actually returned, return value %08lx = %ld\n",ret,ret); fflush(stdout);
    }
}

int main(int,char**)
{
    test_hello();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
