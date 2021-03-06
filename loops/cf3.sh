#!/bin/bash
vl=256
ii=32
jj=16
kern=CHECK
if [ "$1" ]; then vl="$1"; shift; fi
if [ "$1" ]; then ii="$1"; shift; fi
if [ "$1" ]; then jj="$1"; shift; fi
if [ "$1" ]; then kern="$1"; shift; fi
#
# edit:  -totmp-vi.c to use vl "as stated"
#        -aotmp-vi.c to allow ALT vl
#        -kPRINT to just print the vectors
#        -kCHECK to just assert correctness (./cf3 --help for more options)
#
if [ ! -d "cf" ]; then mkdir cf; else rm cf/*-vi cf/*-vi.i cf/*-vi.s; fi
log="cf/${kern}-${vl}_${ii}_${jj}.log"
code="cf/${kern}-${vl}_${ii}_${jj}-vi"
cmd="./cf3 -to${code}.c -k${kern}"
{ (cd .. && make) && make cf3 && $cmd $vl $ii $jj; \
} >& $log && echo GOOD, was able to run $cmd $vl $ii $jj || echo OHOH
if [ -f "${code}.c" ]; then cp "${code}.c" ./cf3-vi.c; fi
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -E ${code}.c -o ${code}.i >> $log
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -S ${code}.c -o ${code}.s >> $log 2>&1 \
	&& echo "GOOD, ${code}.s" || echo "BAD, could not compile ${code}.c"
{ clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics ${code}.c -o ${code} && ./${code}; } >> $log 2>&1 \
	&& echo "GOOD, ${code} ran, see $log" || echo "BAD, could not run ${code}, see $log"
cp $log ./cf3.log
cat $log
