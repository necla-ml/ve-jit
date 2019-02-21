/** \file
 * Execute a JIT hello-world kernel
 * - Show how to:
 *   - allocate an executable page,
 *   - cross-assemble a small VE assembler kernel,
 *   - execute the kernel function
 *     - which prints "hello, world"
 *   - and even return
 *
 * - We kinda' do a function call, but the jit code:
 *   - executes in caller stack frame
 *   - any args must be passed in registers only
 *   - code must be fully relocatble and need no external symbols
 */
#include "jitpage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>   // _SC_PAGE_SIZE
//char const* code = ".string \"Hello world\"\n"; // this is an assembly kernel
#define STR0(s...) #s
#define LAB(s...) STR0(s\n)
#define COMM(s...) "\t# " #s "\n"
#define TTR(s...) "\t" #s "\n"
char const* kernel_hello =
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

/** load a .bin file into page[4k] buffer.
 * assert .bin in <= 4k bytes first.
 * return */
static size_t bin2page(char const* basename, char const* page){
    size_t nread = 0U;
    assert(strlen(basename)<80);
    assert(strlen(basename)>0);
    {
        char ls[100];
        //snprintf(&mk_cmd[0],100,"VERBOSE=0 make -f bin.mk %s.bin\0",basename);
        snprintf(&ls[0],100,"ls -ld %s.bin",basename);
        printf("running <%s>\n",ls); fflush(stdout);
        system(ls);
    }
    char file_bin[100];
    snprintf(&file_bin[0],100,"%s.bin\0",basename);
    printf("opening %s\n",file_bin); fflush(stdout);
    FILE* f_bin = fopen(file_bin,"rb");
    if( f_bin==NULL ){
        printf(" bin2page(\"%s\",page): Missing file %s\n",basename,file_bin);
    }else{
        fseek(f_bin,0,SEEK_END);
        long fsize = ftell(f_bin);
        printf(" %s has %ld bytes\n",file_bin,fsize);
        if( fsize==0 || fsize > 4096 ){
            printf(" bin2page(\"%s\",page): %s bad file size %ld\n",basename,file_bin,fsize);
        }else{
            printf("reading %s\n",file_bin); fflush(stdout);
            fseek(f_bin,0,SEEK_SET); // rewind(f_bin)
            nread = fread((void*)page, (size_t)sizeof(char), (size_t)fsize, f_bin);
            printf(" bin2page read %lu bytes from %s\n",nread,file_bin); fflush(stdout);
            //assert( nread == (size_t)fsize );
            nread = (size_t)fsize;
        }
    }
    return nread;
}

void test_kernel_hello(){  
  printf("\nTest: load and execute a jit \"hello world\" page\n");
  printf("\n      The blob comes from assembling a C string of assembler code\n");
  printf("      generated using some handy-dandy C macros --> tmp_kernel_hello.S\n");
  printf("      and then using 'make -f bin.mk' to convert it to binary blob.\n");
  printf("\n      We then use inline assembler to load the blob registers\n");
  printf("      and execute instructions directly from the blob via its\n");
  printf("      simple register-only calling convention\n\n");
  fflush(stdout);
  asm2bin("tmp_kernel_hello", kernel_hello, 2/*verbose*/);     // creates tmp_kernel_hello.S and tmp_kernel_hello.bin
  system("nobjdump -b binary -mve -D tmp_kernel_hello.bin");
  system("hexdump -C tmp_kernel_hello.bin");
  //char page[4096];
  char *page = mmap(
      NULL,             // address
      4096,             // size
      PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_ANONYMOUS,
      -1,               // fd (not used here)
      0);               // offset (not used here)
  for(int i=0; i<4096; ++i) page[i] = '\0';
  size_t sz = bin2page("tmp_kernel_hello",page);
  hexdump(page,sz);
  printf("Going to execute the .bin memory page, in current stack-frame\n"); fflush(stdout);
  { // load a pointer, make a copy, just to be sure we can
    void *pagecpy=NULL;
    asm("ld %s0, %[ptr]\n"
        "or %[copy],0,%s0\n"
        :[copy]"=r"(pagecpy)
        :[ptr]"m"(page)
        :"%s0"); // illegal to clobber %lr
    printf(" page @ %p pagecpy @ %p\n",page,pagecpy); fflush(stdout);
    assert( pagecpy == page );
  }
  {
    printf(" going to bsic into page @ %p\n",page); fflush(stdout);
    printf(" tmp_kernel_hello takes no [register!] args\n"); fflush(stdout);
    asm("ld %s12, %[page]\n"
        "bsic %lr,(,%s12)"
        : /* no return value */
        :[page]"m"(page)
        :"%s12", "%s0", "%s1", "%s2", "%s3" ); // illegal to clobber %lr, apparently:)
    printf("... and we actually returned!"); fflush(stdout);
  }
}

void test_kernel_math(unsigned long parm){
  printf("\nTest: test_kernel_math\n");
  printf("      Here we create JIT assembly code to add and subtract a constant\n");
  printf("      to %s0, returning %s0+parm and %s0-parm in %s1 and %s2.\n");
  printf("      Depending on the size of the constant, we will output different\n");
  printf("      assembly code; i.e. we do a tiny JIT optimization\n");
  char kernel_math[1024U];
  {
    // create the JIT assembly code
    char const* kernel_math_begin =
      COMM(runtime JIT value unsigned long 'parm')
      COMM(input:   %s0)
      COMM(output:  %s1 = %s0 + parm)
      COMM(         %s2 = %s0 - parm)
      COMM(clobbers: nothing else)
      ;
    // The JIT part sticks runtime value parm into register %s2
    char jit_parm[240]; size_t const jsz=240;
#if 0
    // Here's an unoptimized version:
    snprintf(jit_parm,80,
        "\t# load JIT parm into %%s2\n"
        "\tlea    %%s2, %x\n"
        "\tlea.sl %%s2, %x(,%%s2)\n",
        (uint32_t)parm,  (uint32_t)(parm>>32));
    char const* kernel_math_end =
      TTR(addu.l %s1,%s0,%s2)
      TTR(subu.l %s2,%s0,%s2)
      TTR(b.l	(,%lr))
      ;
#elif 0
    // if any half of the uint64_t is zero, we don't need to load it.
    printf("optimized JIT kernel to load constant value %lu\n",parm); fflush(stdout);
    {
      size_t n=0;
      n += snprintf(&jit_parm[n],jsz-n, "\t# load JIT parm %lu into %%s2\n",parm);
      assert(n<jsz);
      if( parm == 0UL ){
        n += snprintf(&jit_parm[n],jsz-n,
            "\t# fast load zero\n"
            "\tor  %s2,0,0\n");
        assert(n<jsz);
      }else{
        uint32_t const lo = (uint32_t)parm;
        if( lo ){
          n += snprintf(&jit_parm[n],jsz-n, "\tlea %%s2, %x\n", lo);
          assert(n<jsz);
        }else{
          n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-zero lo bits\n");
          assert(n<jsz);
        }
        uint32_t const hi = (uint32_t)(parm>>32);
        if( hi ){
          n += snprintf(&jit_parm[n],jsz-n, "\tlea.sl %%s2, %x\n", hi);
          assert(n<jsz);
        }else{
          n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-zero hi bits\n");
          assert(n<jsz);
        }
      }
    }
    printf("%s",jit_parm); fflush(stdout);
    char const* kernel_math_end =
      TTR(addu.l %s1,%s0,%s2)
      TTR(subu.l %s2,%s0,%s2)
      TTR(b.l	(,%lr))
      ;
#elif 1 // even more optimized
    char const* kernel_math_end;
    int add_ok=0, sub_ok=0;
    size_t n=0; jit_parm[0]='\0';
    if(parm==0){
      kernel_math_end =       // just copy register s0 --> s1 and s2
        TTR(or %s1,0,%s0)
        TTR(or %s2,0,%s0)
        TTR(b.l	(,%lr))
        ;
      add_ok = sub_ok = 1;
    }
    if(!add_ok){
      // M-constants can be embedded directly into BOTH add/sub (fastest,no extra reg load)
      char mconst[6];
      //strMconst(mconst, parm);
      //if( mconst[0] == '(' )
      if(strMconst(mconst,parm))
      {
        printf(" M-const!"); fflush(stdout);
        n += snprintf(&jit_parm[n],jsz-n, "\t#add/sub Mconst\n"
            "\taddu.l %%s1, %%s0,%s\n"
            "\tsubu.l %%s2, %%s0,%s\n", mconst, mconst);
        assert(n<jsz);
        add_ok = sub_ok = 1;
      }
      kernel_math_end = TTR(b.l (,%lr));
    }
    if(!add_ok){ // subtract NEEDS const in %s2...
      {
        // fast load JIT cont into %s2
        n += snprintf(&jit_parm[n],jsz-n, "\t# load JIT parm %lu into %%s2\n",parm);
        assert(n<jsz);
        assert(parm != 0UL);  // we already handled the trivial case
        uint32_t const lo = (uint32_t)parm;
        if( lo ){
          n += snprintf(&jit_parm[n],jsz-n, "\tlea %%s2, %x\n", lo);
          assert(n<jsz);
        }else{
          n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-zero lo bits\n");
          assert(n<jsz);
        }
        uint32_t const hi = (uint32_t)(parm>>32);
        if( hi ){
          n += snprintf(&jit_parm[n],jsz-n, "\tlea.sl %%s2, %x\n", hi);
          assert(n<jsz);
        }else{
          n += snprintf(&jit_parm[n],jsz-n, "\t# skip loading all-zero hi bits\n");
          assert(n<jsz);
        }
      }
      // the ADD can start right away **if** we have an I-value, in [-64,63]
      long lparm = (long)parm;
      if( !add_ok && lparm >= -64 && lparm <= 63 ){
        n += snprintf(&jit_parm[n],jsz-n, "\t#add Imm\n\taddu.l %%s1,%ld,%s0\n",lparm);
        assert(n<jsz);
        add_ok = 1;
      }
      if( !add_ok ){ // o.w. the ADD can do a register-register OP
        n += snprintf(&jit_parm[n],jsz-n, "\taddu.l %%s1,%%s0,%%s2\n");
        assert(n<jsz);
        add_ok = 1;
      }
      if( !sub_ok ){
        n += snprintf(&jit_parm[n],jsz-n, "\tsubu.l %%s1,%%s0,%%s2\n");
        assert(n<jsz);
        sub_ok = 1;
      }
      kernel_math_end = TTR(b.l (,%lr));
    }
#endif
    snprintf(kernel_math,1024,"%s\n%s\n%s", kernel_math_begin, jit_parm, kernel_math_end);
  }
  // uniquely name the kernel, in case we wanted several variants
  //                           and to later inspect each kernel
  char kernel_name[80];
  snprintf(kernel_name,80,"tmp_kernel_addsub%lu\0",parm);
  printf(" test_kernel_math(%lx) --> JIT code %s.S:\n%s",parm,kernel_name,kernel_math); fflush(stdout);
  // create .bin file
  asm2bin(kernel_name, kernel_math, 2/*verbose*/);  // creates .S and .bin file

  char line[80];
  snprintf(line,80,"nobjdump -b binary -mve -D %s.bin\0", kernel_name);
  system(line);

  int nerr=0;
  JitPage jp;
  if(bin2jitpage( kernel_name, &jp, 2/*verbose*/) == NULL){
    ++nerr;
  }else{
    for(unsigned long arg=1U; arg>0UL; arg<<=4){
      unsigned long ret1, ret2;
      //
      // Here is how to pass in an argument,
      // call the kernel
      // and retrieve return values from output registers
      //
      asm(//"\tlea %s0,0x666\n"   /*help find this place in asm*/
          "\tlea %s0,  (,%[arg])\n"
          "\tlea %s12, (,%[page])\n"  /* lea + "r" this time, instead of ld + "m" */
          "\tbsic %lr,(,%s12)\n"
          "\tor %[ret1], 0,%s1\n" /* retrieve kernel compute results */
          "\tor %[ret2], 0,%s2\n"
          :[ret1]"=r"(ret1), [ret2]"=r"(ret2)
          :[arg]"r"(arg), [page]"r"(jp.mem)
          :"%s0","%s1", "%s2", "%s12"
         );
      printf("kernel(%016lx) --> ret1=%016lx, ret2=%016lx\n",arg,ret1,ret2);
      // Check for errors in our kernel outputs:
      if( ret1 != arg+parm ) ++nerr;
      if( ret2 != arg-parm ) ++nerr;
      fflush(stdout);
    }
  }
  printf("Done test_kernel_math(%lu) with %u errors\n",(unsigned long)parm, nerr); fflush(stdout);
  jitpage_free(&jp);
  assert(nerr == 0);
}

main()
{
  printf("remaking asmkern1.bin (from .S)\n"); fflush(stdout);
  system("bash -c 'rm -f asmkern1.bin; make -f bin.mk asmkern1.bin'");
  system("ls -l asmkern1*");
  printf("remade asmkern1.bin\n");
  test_kernel_hello(); // take no args, return value of SYS_write [unknown]
  printf("\n");
}
/* vim: set ts=2 sw=2 et: */
