#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

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

/** use 0 to see full output */
#define REMOVE_CODE 8

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
// COPIED from jitpage.c
static void * dlopen_rel(char const* const relpath, int opt){
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
///////////////////////////////////////////////////////////////////////////////
int main(const int argc, const char * const * const argv) {
  assert((argc == 2) && (argc == 2));
  const char * const lib = argv[1];
  printf("Program: %s %s",argv[0],argv[1]);
  printf(" Attempting dlopen_rel(%s,RTLD_LAZY)\n",lib); fflush(stdout);
  void * const handle = dlopen_rel(lib, RTLD_NOW);
  assert(handle != 0);
#if REMOVE_CODE < 8
  dl_dump(handle);
  printf(" dlclose..."); fflush(stdout);
  dlclose(handle);
#endif
  printf(" DONE!\n"); fflush(stdout);
  printf("Goodbye\n"); fflush(stdout);
  return 0;
}

/* vim: set ts=2 sw=2 et: */
