/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "cjit.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h> // access
#include <link.h>
#include <dlfcn.h>

#ifdef __cplusplus
# define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#if !defined(__ELF_NATIVE_CLASS) /* VE compatibility patch */
// see /opt/nec/ve/musl/include/link.h
#if UINTPTR_MAX > 0xffffffff
#define __ELF_NATIVE_CLASS 64
#else
#define __ELF_NATIVE_CLASS 32
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
static const ElfW(Dyn) *
FindTag(const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
  for (; dyn->d_tag != DT_NULL; ++dyn) {
    if (dyn->d_tag == tag) {
      return dyn;
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
static size_t
FindVal(const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
  const ElfW(Dyn) *entry = FindTag(dyn,tag);
  if(!entry) printf(" error: tag %d not found\n",(int)tag);
  return (entry? entry->d_un.d_val: 0);
}

#ifdef __cplusplus
extern "C" {
#endif

    void * dlopen_rel(char const* const relpath, int opt){
        // first try relpath "as is" (maybe it is libm.so from some known dir)
        dlerror(); // clear error
        void * ret = dlopen(relpath,opt);
        if(!ret){ // if not, then maybe it is a file
            const char * const abspath = realpath(relpath,NULL);
            if(!abspath){
                char const* err=dlerror();
                printf(" %s not a path? %s\n",(err?err:"unknown error"));
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

    ///////////////////////////////////////////////////////////////////////////////
#define Title(...) do{printf("-------------------------------------------------" \
        "------------------------------\n" __VA_ARGS__); fflush(stdout);}while(0)

    void dl_dump(void * const handle){
#ifndef _GNU_SOURCE
# warning Not using GNU extensions (dl_dump may not work)
#endif
#if 1 // try to compile anyway
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
        // Some aliases
#define GetTag(tag) FindTag(dyn_start, tag)
#define GetVal(tag) FindVal(dyn_start, tag)
#define GetPtr(tag) FindPtr(load_addr, dyn_start, tag)
#define IterTag(tag) \
        for (const ElfW(Dyn) * dyn = GetTag(tag); dyn; dyn = FindTag(++dyn, tag))
        // Get the string table


#if !defined(__ve)
        const char * const strtab = (const char *)GetPtr(DT_STRTAB);
        const size_t strtabsize   =               GetVal(DT_STRSZ);
#else
        const char * const strtab = (const char *)(load_addr + GetTag(DT_STRTAB)->d_un.d_ptr);
        const size_t strtabsize   =                            GetVal(DT_STRSZ);
        fflush(stdout);
        assert( strtab[0] == '\0' );
#endif
        Title("String Table: %p (%" PRIu64")\n", strtab, strtabsize);
        // Get the so name (For g++ and nc++ this section is OPTIONAL)
        const ElfW(Dyn) *dt_soname = GetTag(DT_SONAME);
        if(dt_soname) Title("SO Name: %s\n", &strtab[load_addr + dt_soname->d_un.d_ptr]);
        // Get the needed libraries
        Title("Needed:\n");
        IterTag(DT_NEEDED) {
            const size_t index = dyn->d_un.d_val;
            assert(index < strtabsize);
            printf(" - %s\n", &strtab[dyn->d_un.d_val]);
        }
        // Get the symbol table
        typedef ElfW(Sym) SymEnt;
        const ElfW(Dyn) * symsec = GetTag(DT_SYMTAB);
#if !defined(__ve)
        const SymEnt * const symtab = (const SymEnt*)GetVal(DT_SYMTAB);
        const size_t symentsize     =                GetVal(DT_SYMENT);
#else
        const SymEnt * const symtab = (const SymEnt*)(load_addr + symsec->d_un.d_ptr);
        const size_t symentsize     =                GetVal(DT_SYMENT);
#endif
        const SymEnt * syment = symtab;
        Title("Symbols:\n");
#if 0
        printf("|%-40s|%-10s|%-10s|%-8s|%-16s|%-8s|\n", "Name", "Info",
                "Visibility", "Section", "Addr", "Size");
#else
        printf("|%-40s|%-4s|%-4s|%-3s|%-5s|%-16s|%-8s|\n", "Name", "Bind","Type",
                "Vis", "Sect", "Addr", "Size");
#endif
        //assert(*(char*)syment == '\0');
        syment = (const SymEnt*)((const uint8_t*)(syment) + symentsize);
        //while (*(char*)syment != '\0'syment->st_shndx != STN_UNDEF)
        for(int i=0; ;++i, syment = (const SymEnt*)((const uint8_t*)(syment) + symentsize))
        {
            //printf("i=%d st_name %u syment @ %p\n", i, syment->st_name, (void const*)syment); fflush(stdout);
            if( syment->st_name >= strtabsize ){
                printf("last symbol!  st_name = %d = 0x%x\n", (int)syment->st_name, (int)syment->st_name); fflush(stdout);
                break;
            }
            if( syment->st_shndx == STN_UNDEF ){
                //printf("(undef)");
                continue;
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
        }
#endif
        return;
    }
#ifdef __cplusplus
} // extern "C"
#endif

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
