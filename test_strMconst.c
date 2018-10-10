
#include "jitve_util.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_strMconst(void){
  char mconst[80];
  int nerr = 0;
  char expect[80];
  {
    int lz=0;               // N leading zeroes (MSB)
    uint64_t val = ~0UL;    // followed by all-1 bits
    for( ; lz<=63; val>>=1, ++lz){
      // skip zero = (64)0 because we prefer (0)1
      expect[0]='\0';
      sprintf(expect,"(%d)0",lz);
      printf("expect %016lx --> %s",val,expect);
      int ok = strMconst(mconst, val);
      printf(" ok=%d expect=%s mconst=%s\n",ok,expect,mconst); fflush(stdout);
      if( !ok ) ++nerr;
      else{
        if( strlen(expect) != strlen(mconst) ){ ok=0; ++nerr; printf(" lenX"); }
        if( strcmp(expect,mconst) != 0 ){ ok=0; ++nerr; printf(" strX"); }
      }
      if(!ok){ printf(" warning: val=%016lx expect=%s differs from mconst=%s\n",val,expect,mconst); fflush(stdout); }
      else{ printf(" %s-OK",expect); fflush(stdout); }
      printf("\n"); fflush(stdout);
      assert(ok);
    }
    printf("\n");
    lz = 0;
    val = ~0UL;
    //val>>=1; ++lz; // because for zero we prefer (0)1 instead of (64)0
    for( ; lz<=63; val>>=1, ++lz){
      // skip (64)1 because we prefer to use (0)0 for -1
      expect[0]='\0';
      sprintf(expect,"(%d)1",lz);
      printf("expect %016lx --> %s",~val,expect);
      int ok = strMconst(mconst, ~val); // leading-1s? negate the leading-0s case!
      printf(" ok=%d expect=%s mconst=%s\n",ok,expect,mconst); fflush(stdout);
      if( !ok ) ++nerr;
      else{
        if( strlen(expect) != strlen(mconst) ){ ok=0; ++nerr; printf(" lenX"); }
        if( strcmp(expect,mconst) != 0 ){ ok=0; ++nerr; printf(" strX"); }
      }
      if(!ok){ printf(" warning: val=%016lx expect=%s differs from mconst=%s\n",~val,expect,mconst); fflush(stdout); }
      else{ printf(" %s-OK",expect); fflush(stdout); }
      printf("\n"); fflush(stdout);
      assert(ok);
    }
  }
  assert(nerr==0);
}

main()
{
  test_strMconst();      // recognize constants that can be written in (N)M bitstring format
  printf(__FILE__ "DONE: Goodbye!\n");
}
