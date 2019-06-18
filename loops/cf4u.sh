#!/bin/bash
vl=256
ii=32
jj=16
kern=CHECK
prog=cf4
tmpdir="tmp${prog}"
unroll=-1
vl_opt="-t"
if [ "$1" == "-a" ]; then vl_opt="-a"; shift; fi
if [ "$1" ]; then vl="$1"; shift; fi
if [ "$1" ]; then ii="$1"; shift; fi
if [ "$1" ]; then jj="$1"; shift; fi
if [ "$1" ]; then kern="$1"; shift; fi
if [ "$1" ]; then unroll="$1"; shift; fi
if [ $unroll -lt 0 ]; then unroll=8; fi
if [ $unroll -gt 0 ]; then tmpdir="tmp${prog}u${unroll}"; fi
#
# edit:  -uotmp-vi.c to use vl "as stated"
#        -aotmp-vi.c to allow ALT vl
#        -kPRINT to just print the vectors
#        -kCHECK to just assert correctness (./cf4 --help for more options)
#
if [ ! -d "${tmpdir}" ]; then mkdir ${tmpdir}; else rm ${tmpdir}/*-vi ${tmpdir}/*-vi.i ${tmpdir}/*-vi.s; fi
log="${tmpdir}/${kern}-${vl}_${ii}_${jj}.log"
code="${tmpdir}/${kern}-${vl}_${ii}_${jj}-vi"
cmd="./${prog} -k${kern} -u${unroll} ${vl_opt}o${code}.c"
{ (cd .. && make) && make VERBOSE=1 ${prog} \
	&& echo "Cmd : ${cmd}" \
	&& $cmd $vl $ii $jj; \
} >& $log && echo GOOD, was able to run $cmd $vl $ii $jj || echo OHOH
if [ -f "${code}.c" ]; then cp "${code}.c" ./${prog}-vi.c; fi
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -E ${code}.c -o ${code}.i >> $log
clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -S ${code}.c -o ${code}.s >> $log 2>&1 \
	&& echo "GOOD, ${code}.s" || echo "BAD, could not compile ${code}.c"
{ clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics ${code}.c -o ${code} && ./${code}; } >> $log 2>&1 \
	&& echo "GOOD, ${code} ran, see $log" || echo "BAD, could not run ${code}, see $log"
cp $log ./${prog}${vl_opt}${unroll}.log
cat $log
echo "Log file $log"
echo "Log copy ./${prog}${vl_opt}${unroll}.log"
