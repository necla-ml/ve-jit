/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include "jitve_util.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/mman.h>
#include <unistd.h>   // _SC_PAGE_SIZE
void asm2bin(char const* basename, char const* cpp_asm_code){
    assert(strlen(basename)<80);
    char file_S[100];
    snprintf(&file_S[0],100,"%s.S\0",basename);
    printf("creating .S file %s\n",basename); fflush(stdout);
    {
        FILE* f_S = fopen(&file_S[0], "w");
        fputs(cpp_asm_code, f_S);
        fclose(f_S);
    }
    printf("created  .S file %s\n",basename); fflush(stdout);
    {
        char mk_cmd[100];
        //snprintf(&mk_cmd[0],100,"VERBOSE=0 make -f bin.mk %s.bin\0",basename);
        snprintf(&mk_cmd[0],100,"make -f bin.mk %s.bin\0",basename);
        printf("cmd: %s\n",mk_cmd); fflush(stdout);
        system(mk_cmd);
        sleep(1);
        fflush(stdout);
        fflush(stderr);
        snprintf(&mk_cmd[0],100,"ls -ld %s.*",basename);
        system(mk_cmd);
        sleep(1);
        fflush(stdout);
        fflush(stderr);
    }
    printf("asm2bin(\"%s\",code): DONE\n",basename); fflush(stdout);
}
/** load a .bin file into page[4k] buffer.
 * assert .bin in <= 4k bytes first.
 * return */
size_t bin2page(char const* basename, char const* page){
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

/** more flexible API.  Return NULL on error. */
Jitpage bin2jitpage(char const* basename){
    size_t nread = 0U;
    assert(strlen(basename)<80);
    char file_bin[100];
    snprintf(&file_bin[0],100,"%s.bin\0",basename);
    printf("opening <%s>\n",file_bin); fflush(stdout);
    FILE* f_bin = fopen(file_bin,"rb");
    printf("opened  %s\n",file_bin); fflush(stdout);
    char *page = NULL;
    size_t page_len = 0U;
    if( f_bin==NULL ){
        printf(" bin2page(\"%s\",page): Missing file %s\n",basename,file_bin);
    }else{
        fseek(f_bin,0,SEEK_END);
        long const fsize = ftell(f_bin);
        printf(" %s has %ld bytes\n",file_bin,fsize);
        if( fsize<=0 || fsize > 4096 ){
            printf(" bin2page(\"%s\",page): %s bad file size %ld\n",basename,file_bin,fsize);
        }else{
            {
                ssize_t const page_size = sysconf(_SC_PAGE_SIZE);
                size_t const min_bytes = (fsize<page_size? page_size: fsize);
                page_len = (min_bytes + page_size-1)/page_size*page_size;
                page = mmap(
                        NULL,             // address
                        page_len,         // size
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1,               // fd (not used here)
                        0);               // offset (not used here)
                if(page == MAP_FAILED){
                    printf("mmap executable page len=%lu FAILED\n",(long unsigned)page_len); fflush(stdout);
                    page = NULL;
                }
            }
            if(page){
                printf("reading %s\n",file_bin); fflush(stdout);
                fseek(f_bin,0,SEEK_SET); // rewind(f_bin)
                nread = fread((void*)page, (size_t)sizeof(char), (size_t)fsize, f_bin);
                printf(" bin2page read %lu bytes from %s\n",nread,file_bin); fflush(stdout);
                assert( nread == (size_t)fsize );
                nread = (size_t)fsize;
            }
        }
    }
    Jitpage jitpage = {page, page_len};
    return jitpage;
}
void jitpage_free( Jitpage* jitpage ){
    if(jitpage->addr!=NULL){
        munmap((void*)jitpage->addr, jitpage->len);
    }
    *(char**)&jitpage->addr = NULL;
    *(size_t*)&jitpage->len = 0U;
}

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
}

/** create and M-string constant.
 * nas can use some constants in format (N)M.
 * Here N=repeat count 0..64
 * and M is 0|1.
 * The constant is read as "N high-position M-bits"
 * ... followed by all-(~M) bits.
 *
 * \ret 0/1 for fail/success
 * If we return 1, then mconst is set to the string.
 *
 * NEEDS thorough testing !!!!!!!!
 *
 * string buffer \c mconst bounds \b unchecked !
 * \pre \c mconst can hold >= 6 characters
 *      (incl space for terminating nul)
 */
int strMconst(char *mconst,uint64_t const parm){
    printf(" strMconst(char*,%lx)...",parm); fflush(stdout);
    if(parm==0UL){
        sprintf(mconst,"(0)1"); // zero 1s (rest 0), 0
    }else if(~parm==0UL){
        sprintf(mconst,"(0)0"); // zero 0s (rest 1), -1
    }else if(((parm+1)&(parm)) == 0){ // some zeroes, followed by all-ones
        printf("(N)0..."); fflush(stdout);
        int n=0; uint64_t p = parm;
        while(p >>= 1) ++n;
        printf(";n=%d",n); fflush(stdout);
        sprintf(mconst,"(%d)0",63-n);
    }else if(((~parm+1)&(~parm)) == 0){ // some ones, followed by all-zeros
        printf("(N)1..."); fflush(stdout);
        int n=0; uint64_t p = ~parm;
        while(p >>= 1) ++n;
        printf(";~n=%d",n); fflush(stdout);
        sprintf(mconst,"(%d)1",63-n);
    }else{
        mconst[0]='\0';
        printf("X\n"); fflush(stdout);
        return 0;
    }
    printf("%s\n",mconst);
    return 1;
}
/* vim: set ts=4 sw=4 et: */
