#!/bin/bash
echo "cf.sh args: $*"
usage() {
	echo "$0 usage:"
	echo "  $0 [-h] [flXXX] [-a] [vl [ii [jj [KERN [UNROLL]]]]] [...]"
	echo "  flXXX  program to run [fl6]. Can use 'fl6 -vv' to supply extra options"
	echo "  -a use alternate VL [use given vl]"
	echo "  vl ii jj    VL for(0..ii)(0..jj)"
	echo "  KERN        [CHECK]|HASH|PRINT|..."
	echo "  UNROLL      [8] use 0 for no_unroll"
	exit 0
}
vl=256
ii=32
jj=16
kern=CHECK
prog=fl6
unroll=-1
vl_opt="-t"
if [ "$1" == "-h" ]; then usage; shift; fi
if [ "${1::2}" == "fl" ]; then prog="$1"; shift; fi
if [ "$1" == "-a" ]; then vl_opt="-a"; shift; fi
if [ "$1" ]; then vl="$1"; shift; fi
if [ "$1" ]; then ii="$1"; shift; fi
if [ "$1" ]; then jj="$1"; shift; fi
if [ "$1" ]; then kern="$1"; shift; fi
if [ "$1" ]; then unroll="$1"; shift; fi
other_args="$*"
tmpdir="tmp${prog}"
if [ $unroll -lt 0 ]; then unroll=8; fi
if [ $unroll -gt 0 ]; then tmpdir="tmp${prog}u${unroll}"; fi
#
# edit:  -uotmp-vi.c to use vl "as stated"
#        -aotmp-vi.c to allow ALT vl
#        -kPRINT to just print the vectors
#        -kCHECK to just assert correctness (./fl6 --help for more options)
#
if [ ! -d "${tmpdir}" ]; then mkdir ${tmpdir}; else rm ${tmpdir}/*-vi ${tmpdir}/*-vi.i ${tmpdir}/*-vi.s; fi
log="${tmpdir}/${kern}-${vl}_${ii}_${jj}.log"
code="${tmpdir}/${kern}-${vl}_${ii}_${jj}-vi"
cmd="./${prog} -k${kern} -u${unroll} ${other_args} ${vl_opt}o${code}.c"
clang_opts="-target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics ${CLANG_FLAGS}"
err=0
{ (cd .. && make) && make VERBOSE=1 ${prog} \
	&& echo "Cmd : ${cmd}" \
	&& $cmd $vl $ii $jj; \
} >& $log && echo GOOD, was able to run $cmd $vl $ii $jj || { err=$?; echo OHOH; }
if [ -f "${code}.c" ]; then cp "${code}.c" ./${prog}-vi.c; fi
if [ $err -eq 0 ]; then
	clang ${clang_opts} -E ${code}.c -o ${code}.i >> $log
	err=$?
fi
if [ $err -eq 0 ]; then
	clang ${clang_opts} -S ${code}.c -o ${code}.s >> $log 2>&1 \
	&& echo "GOOD, ${code}.s" || { err=$?; echo "BAD, could not compile ${code}.c"; }
fi
if [ $err -eq 0 ]; then
	echo "clang ${clang_opts} ${code}.c -o ${code} && ./${code}" >> $log 2>&1 \
	&& { clang ${clang_opts} ${code}.c -o ${code} && ./${code}; } >> $log 2>&1 \
	&& echo "GOOD, ${code} ran, see $log" || { err=$?; echo "BAD, could not run ${code}, see $log"; }
fi
cp $log ./${prog}${vl_opt}${unroll}.log
cat $log
if [ ! $err -eq 0 ]; then
	echo "error: fl.sh FAILED";
else
	echo "Goodbye: fl.sh OK";
fi
echo "Log file $log"
echo "Log copy ./${prog}${vl_opt}${unroll}.log"
