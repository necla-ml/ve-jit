#include "jitpage.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>  // isprint
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>   // _SC_PAGE_SIZE
#include <stdlib.h> // system

#if 0
/** file-local verbosity, modified w/ \c jitpage_verbose.
 * Probably should be added as a function argument everywhere,
 * to make library more robust to threads/processes.
 */
static int v;

void jitpage_verbose(int const verbose){
	v = verbose;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

int asm2bin(char const* basename, char const* const cpp_asm_code, int const v/*erbose*/){
    assert(strlen(basename)<80);
    char file_S[100];
    snprintf(&file_S[0],100,"%s.S%c",basename,'\0');
    if(v){printf("creating .S file %s\n",basename); fflush(stdout);}
    {
        FILE* f_S = fopen(&file_S[0], "w");
        if(f_S==NULL) goto errors;
        if(fputs(cpp_asm_code, f_S)<0) goto errors;
        if(fclose(f_S)) goto errors;
    }
    if(v>1){printf("created  .S file %s\n",basename); fflush(stdout);}
    {
        char mk_cmd[100];
        //snprintf(&mk_cmd[0],100,"VERBOSE=0 make -f bin.mk %s.bin%c",basename,'\0');
        if(snprintf(&mk_cmd[0],100,"make -f bin.mk %s.bin%c",basename,'\0')<=0) goto errors;
        if(v){printf("cmd: %s\n",mk_cmd); fflush(stdout);}
        if(system(mk_cmd)!=0) goto errors;
        sleep(1);
        fflush(stdout);
        fflush(stderr);
        if(snprintf(&mk_cmd[0],100,"ls -ld %s.*",basename)<=0) goto errors;
        if(system(mk_cmd)!=0) goto errors;
        sleep(1);
        fflush(stdout);
        fflush(stderr);
    }
    if(v>1){printf("asm2bin(\"%s\",code): DONE\n",basename); fflush(stdout);}
    return 0;
errors:
    return 1;
}
char* bin2jitpage(char const* basename, JitPage *jitpage, int const v){
    int ok=1;
    if(basename == NULL || strlen(basename)>=70){
        if(v>=-1){ printf("bin2jitpage missing or too-long basename\n"); fflush(stdout); }
        ok = 0;
    }
    if(jitpage == NULL ){
        if(v>=-1){ printf("bin2jitpage JitPage must be non-NULL\n"); fflush(stdout); }
        ok = 0;
    }
    char *page = NULL;
    size_t page_len = 0;
    char file_bin[50];
    FILE* f_bin = NULL;
    if(ok){
        if(v>=2){ printf(" bin file VE machine code: %s.bin\n", basename); fflush(stdout); }
        snprintf(&file_bin[0],50,"%s.bin",basename);
        file_bin[50-1]='\0'/*paranoia*/;
        if(v>=2){ printf("opening %s\n",file_bin); fflush(stdout); }
        f_bin = fopen(file_bin,"rb");
        if( f_bin==NULL ){
            if(v>=0){ printf(" bin2jitpage(\"%s\",page): Missing file %s\n",basename,file_bin); }
            ok = 0;
        }
    }
    if(ok){
        fseek(f_bin,0,SEEK_END);
        long const fsize = ftell(f_bin);
        if(v>=2){ printf(" %s has %ld bytes\n",file_bin,fsize); fflush(stdout); }
        if( fsize==0 ){
            if(v>=0){ printf(" bin2jitpage(\"%s\",page): %s bad file size %ld\n",basename,file_bin,fsize); }
            ok = 0;
        }else{
            // VE note: page_size seems to be 64M (cf. 4k typical of x86),
            //          so there is a big penalty to having lots of jit pages
            //          around without supporting jitpage merging.
            // TODO: support JitPage merging:
            //  - 1 JitPage stores multiple blobs, each with start addr, len,
            //  - and 'head' and 'next' JitPage* (forming a fwd list).
            //  - and 'state' state.
            //  - 'head' and 'next' entries modified ONLY by bin2jitpage
            //  - free just marks as freed, checks if all blobs are free before dealloc.
            //  - readexec works on all contained blobs
            //    - oh well, don't have mixed code/data jit blobs?
            //  - bin2jitpage tries "merge-after-existing" first, and if no space creates a
            //    new mmap region (with a new 'head' entry);
            ssize_t const page_size = sysconf(_SC_PAGE_SIZE);
            size_t const min_bytes = (fsize<page_size? page_size: fsize);
            page_len = (min_bytes + page_size-1)/page_size*page_size;
            if(v>=2){ printf(" blob fits into %lu bytes (page_size %ld)\n",
                    (unsigned long)page_len, (signed long)page_size); fflush(stdout); }
            page = (char*)mmap(
                    NULL,             // address
                    page_len,         // size
                    PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1,               // fd (not used here)
                    0);               // offset (not used here)
            if(page == MAP_FAILED){
                if(v>=0){ printf("mmap executable page len=%lu FAILED\n",(long unsigned)page_len); fflush(stdout); }
                page = NULL;
                page_len = 0;
                ok = 0;
            }
        }
        if(ok){
            if(v>=2){ printf("reading %s\n",file_bin); fflush(stdout); }
            fseek(f_bin,0,SEEK_SET); // rewind(f_bin)
            size_t const nread = fread((void*)page, (size_t)sizeof(char), (size_t)fsize, f_bin);
            if(v>=2){ printf(" bin2jitpage read %lu bytes from %s\n",nread,file_bin); fflush(stdout); }
            if( nread != (size_t)fsize ){
                if(v>=1){ printf(" bin2jitpage unexpected # of read bytes\n"); fflush(stdout); }
            }
        }
    }
    jitpage->mem = (void*)page;
    jitpage->len = page_len;
    //jitpage->pos = 0;
    jitpage->verbosity = v;
    if(v>=2){
        printf(" Return JitPage{mem=%lX, len=%ld, verbosity=%d)\n",
                (long unsigned)jitpage->mem, (long signed)jitpage->len, jitpage->verbosity);
        fflush(stdout);
    }
    return (char*)(jitpage->mem);
}

void jitpage_readexec(JitPage *jitpage){
    if( jitpage==NULL ){
        printf(" jitpage_readexec jipage==NULL!\n"); fflush(stdout);
    }else{
        int const v = jitpage->verbosity;
        if( jitpage->mem==NULL ){
            if(v>=1){ printf(" jitpage_readexec bad/missing jitpage/mem\n"); fflush(stdout); }
        }else{
            int status = mprotect(jitpage->mem, jitpage->len, PROT_READ|PROT_EXEC);
            if(status && v>=1){ printf(" jitpage_readexec status=%d\n",status); fflush(stdout); }
        }
    }
}

int jitpage_free(JitPage* page){
    int status = 0;
    if( page && page->mem ){ // don't warn about NULLs
        status = munmap(page->mem, page->len);
        if(status){
            if(page->verbosity>=0){
                printf(" jitpage_free(%p,%lu) problem: status=%d\n",
                        page->mem,(unsigned long)(page->len), status);
                fflush(stdout);
            }
        }else{
            if(page->verbosity>=2){ printf(" jitpage_free mem=%p len=%lu\n",page->mem,(unsigned long)page->len); fflush(stdout);
            }
        }
        page->mem = NULL;
        page->len = 0;
        page->verbosity = 0;
    }
    return status;
}

void hexdump(char const* page, size_t sz){
    // reproduce hexdump -C "canonical hex+ASCII" output format
    for(size_t b=0; b<sz; b+=16){
        printf("%08lx ",(unsigned long)b);
        size_t bbend = (b+16 < sz? b+16: sz);
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

#ifdef __cplusplus
} // extern "C"
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
