#include "jitpage.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>      // isprint
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>     // _SC_PAGE_SIZE, sysconf, pathconf, access
#include <stdlib.h>     // system, realpath
#include <sys/stat.h>   // mkdir
#include <stddef.h>

#ifdef __cplusplus
# define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include <link.h>
#include <dlfcn.h>

#if !defined(__ELF_NATIVE_CLASS) /* VE compatibility patch */
// see /opt/nec/ve/musl/include/link.h
#if UINTPTR_MAX > 0xffffffff
#define __ELF_NATIVE_CLASS 64
#else
#define __ELF_NATIVE_CLASS 32
#endif
#endif

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
        if(snprintf(&mk_cmd[0],100,"make VERBOSE=1 -f bin.mk %s.bin%c",basename,'\0')<=0) goto errors;
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

int createDirectoryAnyDepth(char const *path) {
    long const sz = pathconf(".",_PC_PATH_MAX);
    char* opath = (char*)malloc(sz);
    char *p; 
    size_t len; 
    int mkdir_flags = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    strncpy(opath, path, sz);
    len = strlen(opath); 
    if(opath[len - 1] == '/') 
        opath[len - 1] = '\0'; 

    for(p = opath; *p; p++) 
    {
        if(*p == '/' || *p == '\\') 
        { 
            *p = '\0'; 
            if(access(opath, W_OK)) 
                mkdir(opath,mkdir_flags);
            *p = '/';
        }
    }
    if(access(opath, W_OK))
        mkdir(opath, mkdir_flags);
    int ret = access(opath,W_OK);
    free(opath);
    return ret;
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

#if JIT_DLFUNCS
//--------------------- dl_dump stuff

///////////////////////////////////////////////////////////////////////////////
void * dlopen_rel(char const* const relpath, int opt){
    // first try relpath "as is" (maybe it is libm.so from some known dir)
    dlerror(); // clear error
    printf(" dlopen(%s,opt) ", relpath); fflush(stdout);
    void * ret = dlopen(relpath,opt);
    if(ret){
        printf(" succeeded\n"); fflush(stdout);
    }else{ // if not, then maybe it is a file
        char const* err=dlerror();
        printf(" failed, error: %s\n", (err?err:"unknown error")); fflush(stdout);
        const char * const abspath = realpath(relpath,NULL);
        if(!abspath){
            printf(" %s not a relative file path?\n",relpath);
        }else{
            printf(" relpath  : %s\n",relpath);
            printf(" abspath  : %s\n",abspath);
            dlerror(); // clear error
            ret = dlopen(abspath,opt);
            if(!ret){
                char const* err=dlerror();
                printf(" dlopen error: %s\n",(err?err:"unknown error"));
            }
            free((void*)abspath);
        }
    }
    return ret;
}

/** use 0 to see full output */
#define REMOVE_CODE 0

///////////////////////////////////////////////////////////////////////////////
#define BumpDyn(dyn,tag) do{ \
    for (; dyn->d_tag != DT_NULL; ++dyn) { \
        if (dyn->d_tag == tag) { \
            break; \
        } \
    } \
}while(0)
static const ElfW(Dyn) *
FindTag(const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
    BumpDyn(dyn,tag);
    return dyn->d_tag==tag? dyn: NULL;
}

///////////////////////////////////////////////////////////////////////////////
static size_t
FindVal(const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
    size_t ret = 0;
    BumpDyn(dyn,tag);
    if( dyn->d_tag == tag ) ret = dyn->d_un.d_val;
    else printf(" error: tag %d not found\n",(int)tag); // avoid assert in helpers
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
#if 0
static const void *
FindPtr(const ElfW(Addr) load_addr,
        const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
    for (; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == tag) {
            //return (const void *)(dyn->d_un.d_ptr - load_addr); // segfault?
            return (const void *)(dyn->d_un.d_ptr);
        }
    }
    assert(false);
}
#else
    static const void *
FindPtr(const ElfW(Addr) load_addr,
        const ElfW(Dyn)     *dyn,
        const ElfW(Sxword)   tag)
{
    const void *ret = NULL;
    BumpDyn(dyn,tag);
    // note: x86 stores actual pointer value, VE stores relative value?
    if( dyn->d_tag == tag )
#if !defined(__ve)
        ret = (const void*)dyn->d_un.d_ptr;
#else
    ret = (const void*)(load_addr + dyn->d_un.d_ptr);
#endif
    else printf(" error: tag %d not found\n",(int)tag); // avoid assert in helpers
    return ret;
}
#endif

///////////////////////////////////////////////////////////////////////////////
#define Title(...) do{printf("-------------------------------------------------" \
        "------------------------------\n" __VA_ARGS__); fflush(stdout);}while(0)

void dl_dump(void * const handle){
#ifndef _GNU_SOURCE
# warning Not using GNU extensions (dl_dump may not work)
#endif
#if 1 // try to compile anyway
#if REMOVE_CODE < 5
    // Get the link map
    const struct link_map * link_map = 0;
    const int ret = dlinfo(handle, RTLD_DI_LINKMAP, &link_map);
    const struct link_map * const loaded_link_map = link_map;
    assert(ret == 0);
    assert(link_map != 0);
    Title("Libraries:\n");
    while (link_map->l_prev) {
        link_map = link_map->l_prev;
    }
    while (link_map) {
        printf(" - %s (0x%016" PRIX64 ")\n", link_map->l_name, link_map->l_addr);
        link_map = link_map->l_next;
    }
#if REMOVE_CODE < 4
    // Process the dynamic sections
    const ElfW(Dyn) * const dyn_start = loaded_link_map->l_ld;
    printf("dyn_start @ %p\n",(void const*)dyn_start);
    const ElfW(Addr) load_addr = loaded_link_map->l_addr;
    printf("load_addr @ %p\n",(void const*)load_addr);
    Title("Dynamic Sections (%s):\n", loaded_link_map->l_name);
    printf("|%-20s|%-10s|%-16s|%-16s|%-16s|\n", "Tag", "Tag", "Value", "Ptr",
            "Offset");
    for (const ElfW(Dyn) * dyn = dyn_start; dyn->d_tag != DT_NULL; ++dyn) {
        //printf("dyn @ %p\n",(void const*)dyn);
        switch (dyn->d_tag) {
#define print(tag) \
            printf("|%-20s", #tag); fflush(stdout); \
            printf("|%-10d", (int)(dyn->d_tag)); fflush(stdout); \
            printf("|%-16ld", (long int)(dyn->d_un.d_val)); fflush(stdout); \
            printf("|%-16p", (const void *)(dyn->d_un.d_ptr)); fflush(stdout); \
            printf("|%-16p", (const void *)(dyn->d_un.d_ptr - load_addr)); \
            printf("|\n"); fflush(stdout); \
            break
#define case(tag) case tag: print(#tag)
#define default(tag) default: print(#tag)
            case (DT_NEEDED);           /* Name of needed library */
          case (DT_PLTRELSZ);         /* Size in bytes of PLT relocs */
          case (DT_PLTGOT);           /* Processor defined value */
          case (DT_HASH);             /* Address of symbol hash table */
          case (DT_STRTAB);           /* Address of string table */
          case (DT_SYMTAB);           /* Address of symbol table */
          case (DT_RELA);             /* Address of Rela relocs */
          case (DT_RELASZ);           /* Total size of Rela relocs */
          case (DT_RELAENT);          /* Size of one Rela reloc */
          case (DT_STRSZ);            /* Size of string table */
          case (DT_SYMENT);           /* Size of one symbol table entry */
          case (DT_INIT);             /* Address of init function */
          case (DT_FINI);             /* Address of termination function */
          case (DT_SONAME);           /* Name of shared object */
          case (DT_RPATH);            /* Library search path (deprecated) */
          case (DT_SYMBOLIC);         /* Start symbol search here */
          case (DT_REL);              /* Address of Rel relocs */
          case (DT_RELSZ);            /* Total size of Rel relocs */
          case (DT_RELENT);           /* Size of one Rel reloc */
          case (DT_PLTREL);           /* Type of reloc in PLT */
          case (DT_DEBUG);            /* For debugging; unspecified */
          case (DT_TEXTREL);          /* Reloc might modify .text */
          case (DT_JMPREL);           /* Address of PLT relocs */
          case (DT_BIND_NOW);         /* Process relocations of object */
          case (DT_INIT_ARRAY);       /* Array with addresses of init fct */
          case (DT_FINI_ARRAY);       /* Array with addresses of fini fct */
          case (DT_INIT_ARRAYSZ);     /* Size in bytes of DT_INIT_ARRAY */
          case (DT_FINI_ARRAYSZ);     /* Size in bytes of DT_FINI_ARRAY */
          case (DT_RUNPATH);          /* Library search path */
          case (DT_FLAGS);            /* Flags for the object being loaded */
          case (DT_ENCODING);         /* Start of encoded range */
          /* This is a duplicate value Have submitted this as a possible bug:
           * http://sourceware.org/bugzilla/show_bug.cgi?id=15733
           */
          //      case (DT_PREINIT_ARRAY);    /* Array with addresses of preinit fct*/
          case (DT_PREINIT_ARRAYSZ);  /* size in bytes of DT_PREINIT_ARRAY */
          case (DT_NUM);              /* Number used */
          case (DT_LOOS);             /* Start of OS-specific */
          case (DT_HIOS);             /* End of OS-specific */
          case (DT_LOPROC);           /* Start of processor-specific */
          case (DT_HIPROC);           /* End of processor-specific */
          case (DT_PROCNUM);          /* Most used by any processor */
          case (DT_GNU_HASH);         /* GNU-style hash table.  */
          case (DT_VERDEF);           /* Address of version definition table */
          case (DT_VERDEFNUM);        /* Number of version definitions */
          case (DT_VERNEED);          /* Address of table with needed versions */
          case (DT_VERNEEDNUM);       /* Number of needed versions */
          case (DT_VERSYM);           /* The versioning entry types. */
          case (DT_RELACOUNT);
          case (DT_CHECKSUM);
          case (DT_GNU_PRELINKED);    /* Prelinking timestamp */
          default(UNKNOWN);
#undef print
#undef case
        }
    }
#if REMOVE_CODE < 3
    // Some aliases
#define GetTag(tag) FindTag(dyn_start, tag)
#define GetVal(tag) FindVal(dyn_start, tag)
#define GetPtr(tag) FindPtr(load_addr, dyn_start, tag)
#define IterTag(tag) \
    for (const ElfW(Dyn) * dyn = GetTag(tag); dyn; dyn = FindTag(++dyn, tag))
    // Get the string table
    const char * const strtab = (const char *)GetPtr(DT_STRTAB);
    const size_t strtabsize   =               GetVal(DT_STRSZ);
    Title("String Table: %p (%lu)\n", strtab, (long unsigned)strtabsize);
    // Get the so name (For g++ and nc++ this section is OPTIONAL)
    const ElfW(Dyn) *dt_soname = GetTag(DT_SONAME);
    if(strtab && dt_soname) Title("SO Name: %s\n", &strtab[dt_soname->d_un.d_val]);
#if REMOVE_CODE < 2
    // Get the needed libraries
    Title("Needed:\n");
    IterTag(DT_NEEDED) {
        const size_t index = dyn->d_un.d_val;
        if(!strtab || index >= strtabsize)
            printf(" - (error: no strtab[%lu] available)\n", index);
        else
            printf(" - %s\n", &strtab[dyn->d_un.d_val]);
    }
#if REMOVE_CODE < 1
    // Get the symbol table
    typedef ElfW(Sym) SymEnt;
    const ElfW(Dyn) * symsec = GetTag(DT_SYMTAB);
    if(!symsec){
        Title("Symbols: no DT_SYMTAB section");
    }else{ // DT_SYMTAB section exists
        const SymEnt * const symtab = (const SymEnt*)GetPtr(DT_SYMTAB);
        const size_t symentsize     =                GetVal(DT_SYMENT);
        const SymEnt * syment = symtab;
        Title("Symbol Table: %p (entry size %lu)\n",(void*)symtab,(long unsigned)symentsize);
#if 0
        printf("|%-40s|%-10s|%-10s|%-8s|%-16s|%-8s|\n", "Name", "Info",
                "Visibility", "Section", "Addr", "Size");
#else
        printf("|%-40s|%-4s|%-4s|%-3s|%-5s|%-16s|%-8s|\n", "Name", "Bind","Type",
                "Vis", "Sect", "Addr", "Size");
#endif
        //assert(*(char*)syment == '\0');
        // The first entry is ALWAYS STN_UNDEF.
        syment = (const SymEnt*)((const uint8_t*)(syment) + symentsize);
        //while (*(char*)syment != '\0'syment->st_shndx != STN_UNDEF)
        for(int i=0; ;++i, syment = (const SymEnt*)((const uint8_t*)(syment) + symentsize))
        {
            //printf("i=%d st_name %u syment @ %p\n", i, syment->st_name, (void const*)syment); fflush(stdout);
            if( syment->st_shndx == STN_UNDEF ){
                printf("(undef)\n");
                //continue;
            }
            if( syment->st_name >= strtabsize
                    /* above usually good enough, but often */
                    /* DT_STRTAB and DT_SYMTAB are together and OK to also check */
                    || (void*)syment == (void*) strtab
              ){
                // NOTE: There is no "clean" way to get number of entries,
                // but following seems easiest way to break out of the loop
                // (It might actually be enforced in the ELF code, but I did not check)
                printf("last symbol!  st_name = %d = 0x%x\n", (int)syment->st_name,
                        (int)syment->st_name);
                printf("Exit with syment @ %p\n",(void*)syment);
                fflush(stdout);
                break;
            }
            //printf("|%-16s", (syment->st_name? (char const*)&strtab[syment->st_name]:"")); fflush(stdout);
            printf("|%-40s", (char const*)&strtab[syment->st_name]); fflush(stdout);
#if 0
            printf("|%10u", syment->st_info); fflush(stdout);
#else

#define ELFW(X) CAT3(ELF,__ELF_NATIVE_CLASS,_##X)
#define CAT3(e,w,x) CAT3_(e,w,x)
#define CAT3_(e,w,x) e##w##x
#define ELFmac(MAC,ARG) (MAC(ARG))
#define StBind(st_info) ELFmac(ELFW(ST_BIND),st_info)
#define StType(st_info) ELFmac(ELFW(ST_TYPE),st_info)
            int st_bind = (int)StBind(syment->st_info);
            int st_type = (int)StBind(syment->st_info);
            {
                char const* bind=NULL;
                switch(st_bind){
                  case(STB_LOCAL): bind="LOC"; break;
                  case(STB_GLOBAL): bind="GLO"; break;
                  case(STB_WEAK): bind="WEAK"; break;
                }
                if(bind) printf("|%4s",bind);
                else     printf("|%4d",st_bind);
                fflush(stdout);
            }
            {
                char const* type=NULL;
                switch(st_type){
                  case(STT_NOTYPE): type="?"; break;
                  case(STT_OBJECT): type="OBJ"; break;
                  case(STT_FUNC): type="FUNC"; break;
                  case(STT_SECTION): type="SECT"; break;
                  case(STT_FILE): type="FILE"; break;
                  case(STT_COMMON): type="COMMON"; break;
                  case(STT_TLS): type="TLS"; break;
                }
                if(type) printf("|%4s",type);
                else     printf("|%4d",st_type);
                fflush(stdout);
            }
#endif
#if 1
            printf("|%3u", syment->st_other); fflush(stdout);
#else
#define StVis(syment) ELFmac(ELFW(ST_VISIBILITY),(syment)->st_other)
            printf("| %c ", (StVis(syment)? '+': '-'));
#endif
#if 0
            printf("|%8u", syment->st_shndx); fflush(stdout);
#else
            int st_shndx = syment->st_shndx;
            {
                char const* sec=NULL;
                if(st_shndx == SHN_XINDEX) sec="XNDX";
                if(sec) printf("|%5s", sec);
                else    printf("|%5d", st_shndx);
            }
#endif
            printf("|%-16p", (const void*)(syment->st_value)); fflush(stdout);
            printf("|%8" PRIu64, syment->st_size); fflush(stdout);
            printf("|\n"); fflush(stdout);
        }// end loop over symbol table entries
    }// end if DT_SYMTAB exists
#endif //REMOVE_CODE < 1
#endif //REMOVE_CODE < 2
#endif //REMOVE_CODE < 3
#endif //REMOVE_CODE < 4
#endif //REMOVE_CODE < 5
#endif
    return;
}

#endif // JIT_DLFUNCS

#ifdef __cplusplus
} // extern "C"
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
