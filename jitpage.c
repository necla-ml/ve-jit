#include "jitpage.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

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

char* bin2jitpage(char const* basename, struct JitPage *jitpage, int const v){
    int ok=1;
    if(basename == NULL || strlen(basename)>=40){
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
        snprintf(&file_bin[0],50,"%s.bin",basename); file_bin[50-1]='\0'/*paranoia*/;
        if(v>=2){ printf("opening %s\n",file_bin); fflush(stdout); }
        f_bin = fopen(file_bin,"rb");
        if( f_bin==NULL ){
            if(v>=0){ printf(" bin2page(\"%s\",page): Missing file %s\n",basename,file_bin); }
            ok = 0;
        }
    }
    if(ok){
        fseek(f_bin,0,SEEK_END);
        long const fsize = ftell(f_bin);
        if(v>=2){ printf(" %s has %ld bytes\n",file_bin,fsize); }
        if( fsize==0 ){
            if(v>=0){ printf(" bin2page(\"%s\",page): %s bad file size %ld\n",basename,file_bin,fsize); }
            ok = 0;
        }else{
            ssize_t const page_size = sysconf(_SC_PAGE_SIZE);
            size_t const min_bytes = (fsize<page_size? page_size: fsize);
            page_len = (min_bytes + page_size-1)/page_size*page_size;
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
            if(v>=2){ printf(" bin2page read %lu bytes from %s\n",nread,file_bin); fflush(stdout); }
            if( nread != (size_t)fsize ){
                if(v>=1){ printf(" bin2page unexpected # of read bytes\n"); fflush(stdout); }
            }
        }
    }
    jitpage->mem = (void*)page;
    jitpage->len = page_len;
    //jitpage->pos = 0;
    jitpage->verbosity = v;
    return (char*)(jitpage->mem);
}

void jitpage_readexec(struct JitPage *jitpage){
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

int jitpage_free(struct JitPage* page){
    int status = EINVAL;
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
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
