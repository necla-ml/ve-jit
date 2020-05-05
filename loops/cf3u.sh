#!/bin/bash
vl=256
ii=32
jj=16
kern=CHECK
unroll=0
vl_opt="-t"
if [ "$1" == "-a" ]; then vl_opt="-a"; shift; fi
if [ "$1" ]; then vl="$1"; shift; fi
if [ "$1" ]; then ii="$1"; shift; fi
if [ "$1" ]; then jj="$1"; shift; fi
if [ "$1" ]; then kern="$1"; shift; fi
if [ "$1" ]; then unroll="$1"; shift; fi
if [ $unroll -le 0 ]; then unroll=8; fi
#
# edit:  -totmp-vi.c to use vl "as stated"
#        -aotmp-vi.c to allow ALT vl
#        -kPRINT to just print the vectors
#        -kCHECK to just assert correctness (./cf3 --help for more options)
#
if [ ! -d "cfu" ]; then mkdir cfu; else rm cfu/*-vi.i cfu/*-vi.s; fi
log="cfu/${kern}-${vl}_${ii}_${jj}.log"
code="cfu/${kern}-${vl}_${ii}_${jj}-vi"
cmd="./cf3 -u${unroll} ${vl_opt}o${code}.c -k${kern}"
{ (cd .. && make) && make cf3 \
	&& echo "Command : $cmd $vl $ii $jj" \
	&& $cmd $vl $ii $jj; \
} >& $log && echo GOOD, was able to run $cmd $vl $ii $jj || echo OHOH
if [ -f "${code}.c" ]; then cp "${code}.c" ./cf3-vi.c; fi
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -E ${code}.c -o ${code}.i >> $log
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -S ${code}.c -o ${code}.s >> $log 2>&1 \
	&& echo "GOOD, ${code}.s" || echo "BAD, could not compile ${code}.c"
{ clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics ${code}.c -o ${code} && ./${code}; } >> $log 2>&1 \
	&& echo "GOOD, ${code} ran, see $log" || echo "BAD, could not run ${code}, see $log"
cp $log ./cf3u.log
cat $log
