#!/bin/bash
vl=256
ii=32
jj=16
if [ "$1" ]; then vl=$1; shift; fi
if [ "$1" ]; then ii=$1; shift; fi
if [ "$1" ]; then jj=$1; shift; fi
#
# edit:  -totmp-vi.c to use vl "as stated"
#        -aotmp-vi.c to allow ALT vl
#        -kPRINT to just print the vectors
#        -kCHECK to just assert correctness (./cfuse2 --help for more options)
#
cmd="./cfuse2 -totmp-vi.c -kCHECK"
{ (cd .. && make) && make cfuse2 && $cmd $vl $ii $jj; \
} >& cfuse2.log && echo GOOD, was able to run $cmd $vl $ii $jj || echo OHOH
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -E tmp-vi.c -o tmp-vi.i >> cfuse2.log
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -S tmp-vi.c -o tmp-vi.s >> cfuse2.log 2>&1 \
	&& echo "GOOD, tmp-vi.s" || echo "BAD, could not compile tmp-vi.c"
{ clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c -o tmp-vi && ./tmp-vi; } >> cfuse2.log 2>&1 \
	&& echo "GOOD, tmp-vi ran" || echo "BAD, could not run tmp-vi"
