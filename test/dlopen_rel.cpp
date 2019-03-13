#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
/** \file
 * test just dlopen_rel */

#ifdef __cplusplus
# define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include <link.h>
#include <dlfcn.h>

///////////////////////////////////////////////////////////////////////////////
// try to dlopen, and if that fails,
// attempt to create an absolute path and try again.
static void * dlopen_rel(char const* const relpath, int opt){
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
      //free((void*)abspath);
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
int main(const int argc, const char * const * const argv) {
  assert((argc == 2) && (argc == 2));
  const char * const lib = argv[1];
  void * const handle = dlopen_rel(lib, RTLD_LAZY);
  printf("Program: %s %s",argv[0],argv[1]);
  assert(handle != 0);
  //dl_dump(handle);
  //printf(" dlclose..."); fflush(stdout);
  dlclose(handle);
  printf(" DONE!\n"); fflush(stdout);
  printf("Goodbye\n"); fflush(stdout);
  return 0;
}

/* vim: set ts=2 sw=2 et: */

