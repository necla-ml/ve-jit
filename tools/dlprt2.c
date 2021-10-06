/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
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
  for (; dyn->d_tag != DT_NULL; ++dyn) {
    if (dyn->d_tag == tag) {
      return dyn->d_un.d_val;
    }
  }
  assert(false);
}

///////////////////////////////////////////////////////////////////////////////
static const void *
FindPtr(const ElfW(Addr) load_addr,
  const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
  for (; dyn->d_tag != DT_NULL; ++dyn) {
    if (dyn->d_tag == tag) {
      return (const void *)(dyn->d_un.d_ptr - load_addr);
    }
  }
  assert(false);
}

///////////////////////////////////////////////////////////////////////////////
#define Title(...) do{printf("-------------------------------------------------" \
  "------------------------------\n" __VA_ARGS__); fflush(stdout);}while(0)

///////////////////////////////////////////////////////////////////////////////
int main(const int argc, const char * const * const argv) {
  assert((argc == 2) && (argc == 2));
  const char * const lib = argv[1];
  Title("Loading: %s\n", lib);
  //void * const handle = dlopen(lib, RTLD_LAZY);
  void * const handle = dlopen(lib, RTLD_GLOBAL);
  assert(handle != 0);
#ifdef _GNU_SOURCE
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
  const char * const strtab = (const char * const)GetVal(DT_STRTAB);
  const size_t strtabsize   =                     GetVal(DT_STRSZ);
#else
  const char * const strtab = (const char * const)(load_addr + GetTag(DT_STRTAB)->d_un.d_ptr);
  const size_t strtabsize   =                     GetVal(DT_STRSZ);
  fflush(stdout);
  assert( strtab[0] == '\0' );
#endif
  Title("String Table: %p (%" PRIu64")\n", strtab, strtabsize);
  // Get the so name
#if !defined(__ve)
  Title("SO Name: %s\n", &strtab[GetVal(DT_SONAME)]);
#else
  const ElfW(Dyn) *dt_soname = GetTag(DT_SONAME);
  if(dt_soname) Title("SO Name: %s\n", &strtab[load_addr + dt_soname->d_un.d_ptr]);
#endif
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
  printf("|%-40s|%-4s|%-4s|%-10s|%-8s|%-16s|%-8s|\n", "Name", "Bind","Type",
    "Visibility", "Section", "Addr", "Size");
  //assert(*(char*)syment == '\0');
  syment = (const SymEnt*)((const uint8_t*)(syment) + symentsize);
  //while (*(char*)syment != '\0'syment->st_shndx != STN_UNDEF)
  for(int i=0; ;++i, syment = (const SymEnt*)((const uint8_t*)(syment) + symentsize))
  {
    if( syment->st_name >= strtabsize ){
      break;
    }
    if( syment->st_shndx == STN_UNDEF ){
	    //printf("(undef)");
	    continue;
    }
    printf("i=%d st_name %u syment @ %p\n", i, syment->st_name, (void const*)syment); fflush(stdout);
    //printf("|%-16s", (syment->st_name? (char const*)&strtab[syment->st_name]:"")); fflush(stdout);
    printf("|%-40s", (char const*)&strtab[syment->st_name]); fflush(stdout);
    printf("|%10u", syment->st_info); fflush(stdout);
#define ELFW(X) CAT3(ELF,__ELF_NATIVE_CLASS,_##X)
#define CAT3(e,w,x) CAT3_(e,w,x)
#define CAT3_(e,w,x) e##w##x
#define ELFmac(MAC,ARG) (MAC(ARG))
#define StBind(st_info) ELFmac(ELFW(ST_BIND),st_info)
#define StType(st_info) ELFmac(ELFW(ST_TYPE),st_info)
    //printf("|%4d", (int)StBind(syment->st_info)); fflush(stdout);
    //printf("|%4d", (int)StType(syment->st_info)); fflush(stdout);
    printf("|%10u", syment->st_other); fflush(stdout);
    printf("|%8u", syment->st_shndx); fflush(stdout);
    printf("|%-16p", (const void*)(syment->st_value)); fflush(stdout);
    printf("|%8" PRIu64, syment->st_size); fflush(stdout);
    printf("|\n"); fflush(stdout);
#if 0
    if (syment->st_shndx != STN_UNDEF)
    {
      assert(syment->st_name);
      assert(syment->st_name < strtabsize);
      printf("|%-16s|%10u|%10u|%8u|%-16p|%8" PRIu64"|\n",
        &strtab[syment->st_name], syment->st_info, syment->st_other,
        syment->st_shndx, (const void*)(syment->st_value), syment->st_size);
    }else{
	    printf("STN_UNDEF");
	    continue;
    }
#endif
  }
#else
# warning Not using GNU extensions
#endif
  dlclose(handle);
  return 0;
}

