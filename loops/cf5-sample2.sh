#!/bin/bash
# I now have several commands to run through a standard case coverage test.
#
# Examples:
#   To run cf3u.sh (loop fusion, unrolled) with VL II JJ arguments covering many cases:
#      ./mega1-sample.sh ./cf3u.sh >& cf3u-sample.log
#   This will run the -kCHECK kernel, which tests for correct output vectors
#   This will leave behind cfu/FOO-vi.c and cfu/FOO.log files
#
#   To run cf3.sh (loop fusion) with VL II JJ arguments and -kPRINT kernel:
#      ./mega1-sample.sh ./cf3.sh -kPRINT >& cf3-sample.log
#   This will leave behind cf/FOO-vi.c and cf/FOO.log files
#
if [ "$1" ]; then cmd="$1"; shift; else cmd="./cf5 -t"; fi
echo "cmd is ${cmd}"
${cmd} 256 1:4 1024 $* \
	&& ${cmd} 256 1:3 1024 $* \
	&& ${cmd} 256 2:3 1024 $* \
	&& ${cmd} 256 1:2 1024 $* \
	&& ${cmd} 256 1:1 1024 $* \
	&& ${cmd} 256 1:8 992 $* \
	&& ${cmd} 256 2:8 992 $* \
	&& ${cmd} 256 2:4:8 992 $* \
	&& ${cmd} 256 2:4:6:8 992 $* \
	&& ${cmd} 256 1:1 768 $* \
	&& ${cmd} 256 1:3 768 $* \
	&& ${cmd} 256 1:4 768 $* \
	&& ${cmd} 256 1:2 768 $* \
	&& ${cmd} 256 1:3 512 $* \
	&& ${cmd} 256 1:2 512 $* \
	&& ${cmd} 256 1:1 512 $* \
	&& ${cmd} 256 1:4 512 $* \
	&& ${cmd} 256 1:6 384 $* \
	&& ${cmd} 256 1:2 384 $* \
	&& ${cmd} 256 1:4 384 $* \
	&& ${cmd} 256 1:5 384 $* \
	&& ${cmd} 256 1:3 257 $* \
	&& ${cmd} 256 1:1 257 $* \
	&& ${cmd} 256 1:7 257 $* \
	&& ${cmd} 256 1:2 257 $* \
	&& ${cmd} 256 1:2 256 $* \
	&& ${cmd} 256 1:4 256 $* \
	&& ${cmd} 256 1:3 256 $* \
	&& ${cmd} 256 1:8 255 $* \
	&& ${cmd} 256 1:3:8 255 $* \
	&& ${cmd} 256 1:3:5:8 255 $* \
	&& echo "batch 1 done"
${cmd} 256 1:2:4:16 160 $* \
	&& ${cmd} 256 1:2:4:12 160 $* \
	&& ${cmd} 256 1:2:4:8 160 $* \
	&& ${cmd} 256 1:2:4:9 64 $* \
	&& ${cmd} 256 1:2:4:12 64 $* \
	&& ${cmd} 256 1:2:4:13 64 $* \
	&& ${cmd} 256 1:2:4:16 64 $* \
	&& ${cmd} 256 1:2:4:13 60 $* \
	&& ${cmd} 256 1:2:4:16 48 $* \
	&& ${cmd} 256 1:2:4:16 33 $* \
	&& ${cmd} 256 1:2:4:9 32 $* \
	&& ${cmd} 256 1:2:4:16 32 $* \
	&& ${cmd} 256 1:2:4:13 20 $* \
	&& ${cmd} 256 1:2:3:4 4 $* \
	&& ${cmd} 256 1:2:3:3 3 $* \
	&& ${cmd} 256 1:2:2:3 3 $* \
	&& ${cmd} 256 0:0:1:1 1 $* \
	&& ${cmd} 255 1:2:3:3 256 $* \
	&& ${cmd} 255 0:0:1:1 256 $* \
	&& ${cmd} 255 1:2:4:7 256 $* \
	&& ${cmd} 255 1:1:2:2 256 $* \
	&& ${cmd} 255 1:2:4:14 128 $* \
	&& ${cmd} 255 1:2:4:12 85 $* \
	&& ${cmd} 255 1:2:4:6 85 $* \
	&& ${cmd} 255 1:2:4:4 85 $* \
	&& ${cmd} 255 1:2:4:7 85 $* \
	&& ${cmd} 255 1:2:4:9 85 $* \
	&& ${cmd} 255 1:2:4:10 85 $* \
	&& ${cmd} 255 1:2:4:12 64 $* \
	&& ${cmd} 255 1:2:4:15 34 $* \
	&& ${cmd} 255 1:2:4:16 32 $* \
	&& ${cmd} 255 1:2:4:16 16 $* \
	&& echo "batch 2 done"
${cmd} 16 1:2:4:16 9 $* \
	&& ${cmd} 15 1:3:4:30 10 $* \
	&& ${cmd} 12 1:3:4:7 16 $* \
	&& ${cmd} 12 1:3:4:6 16 $* \
	&& ${cmd} 12 1:3:4:12 16 $* \
	&& ${cmd} 12 1:3:4:12 8 $* \
	&& ${cmd} 12 1:3:4:24 8 $* \
	&& ${cmd} 9  1:3:4:9 16 $* \
	&& ${cmd} 9  1:3:4:9 8 $* \
	&& ${cmd} 9  1:3:4:9 4 $* \
	&& ${cmd} 9  1:3:4:16 4 $* \
	&& ${cmd} 9  1:3:4:9 2 $* \
	&& ${cmd} 8  1:3:4:16 1 $* \
	&& ${cmd} 8  1:3:4:9 1 $* \
	&& ${cmd} 7  1:3:4:15 1 $* \
	&& ${cmd} 5  1:3:4:16 1 $* \
	&& ${cmd} 5  1:3:4:15 1 $* \
	&& ${cmd} 4  1:3:4:16 1 $* \
	&& echo "EVERYTHING LOOKS GOOD"
echo "cf5-sample2.sh ${cmd} ... DONE"
