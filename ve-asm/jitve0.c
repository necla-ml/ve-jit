/** \file
 * demo using bin.mk to create VE binary blob from \c .S input.
 * We don't actually run anything.  In principle I'd like to have
 * a 'nas' stand-ing available the could run on x86 :) */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
void asm2bin(char const* basename, char const* cpp_asm_code){
  assert(strlen(basename)<30);
  char file_S[50];
  snprintf(&file_S[0],50,"%s.S\0",basename);
  printf("creating .S file %s\n",basename); fflush(stdout);
  {
    FILE* f_S = fopen(&file_S[0], "w");
    fputs(cpp_asm_code, f_S);
    fclose(f_S);
  }
  printf("created  .S file %s\n",basename); fflush(stdout);
  {
    char mk_cmd[100];
    snprintf(&mk_cmd[0],100,"make -f ../bin.mk %s.bin\0",basename);
    printf("cmd: %s\n",mk_cmd); fflush(stdout);
    system(mk_cmd);
  }
  printf("\nNote: we have not loaded and executed either binary blob in this test\n");
  printf("asm2bin(\"%s\",code): DONE\n",basename); fflush(stdout);
}
/** load a .bin file into page[4k] buffer.
 * assert .bin in <= 4k bytes first.
 * return */
size_t bin2page(char const* basename, char const* page){
  size_t nread = 0U;
  assert(strlen(basename)<30);
  char file_bin[50];
  snprintf(&file_bin[0],50,"%s.bin\0",basename);
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
  fflush(stdout);
  return nread;
}
  
//char const* code = ".string \"Hello world\"\n"; // this is an assembly kernel
#define STR0(s...) #s
#define LAB(s...) STR0(s\n)
#define COMM(s...) "\t# " #s "\n"
#define TTR(s...) "\t" #s "\n"
char const* code =
COMM(print out a Hello World 'debug' string as we exit)
COMM(relocatable no-symbols version of write(1,"Hello world\n",2))
LAB(icbase:)
TTR(sic %s1)
TTR(lea  %s2, strstart-icbase(,%s1))
TTR(ld1b %s3, szstring-icbase(,%s1))
TTR(ld  %s4, 0x18(,%tp)	# SHM_OFFSET)
TTR(lea %s0,1		#  __NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h)
TTR(shm.l %s0, 0x0(%s4))
TTR(lea %s1,1		# fd=1 for stdout)
TTR(shm.l %s1, 0x8(%s4))
TTR(shm.l %s2, 0x10(%s4)	# ptr-to-bytes)
TTR(shm.l %s3, 0x18(%s4)	# #-of-bytes)
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

void hexdump(char const* page, size_t sz){
  // reproduce hexdump -C "canonical hex+ASCII" output format
  for(size_t b=0; b<sz; b+=16){
    printf("%08lx ",(unsigned long)b);
    int bbend = (b+16 < sz? b+16: sz);
    for(size_t bb=b; bb<bbend; ++bb){
      if( bb-b == 8 ) printf(" ");
      printf(" %02x",(unsigned char)page[bb]);
    }
    printf("  |");
    for(size_t bb=b; bb<bbend; ++bb){
      printf("%c",(isprint(page[bb])? page[bb]: '.'));
    }
    printf("|\n");
  }
  fflush(stdout);
}

main()
{
  printf("jitve0: BEGINS -------------------------------------------\n");
  printf("jitve0: test assembler file or string --> binary blob file\n");
  printf("Testing bin.mk: .asm-->.bin binary blob creation\n");
  fflush(stdout);
  system("bash -c 'rm -f asmkern1.bin; make -f bin.mk asmkern1.bin'");
  printf("remade asmkern1.bin\n");
  fflush(stdout);
  system("ls -l asmkern1.*");
  printf("ls Command done!\n");
  char page[4096];
  for(int i=0; i<4096; ++i) page[i] = '\0';
  size_t sz = bin2page("asmkern1",page);
  hexdump(page,sz);

  printf("Testing C-macros asm -->.bin using asm2bin\n");
  printf("Here is the C-macro asm string:\n%s\n",code);
  asm2bin("sys_tmp", code);     // creates sys_tmp.S and sys_tmp.bin
  fflush(stdout);
  system("ls -l sys_tmp.S sys_tmp.bin");
  system("nobjdump -b binary -mve -D sys_tmp.bin");
  system("hexdump -C sys_tmp.bin");
  sz = bin2page("sys_tmp",page);
  hexdump(page,sz);
  printf(__FILE__ " DONE!\n");
}
/* vim: set ts=2 sw=2 et: */
