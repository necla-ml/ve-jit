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
if [ "$*" ]; then cmd="$*"; else cmd="cfuse2 -t"; fi
echo "cmd is ${cmd}"
${cmd} 256 4 1024 \
	&& ${cmd} 256 3 1024 \
	&& ${cmd} 256 2 1024 \
	&& ${cmd} 256 1 1024 \
	&& ${cmd} 256 8 992 \
	&& ${cmd} 256 1 768 \
	&& ${cmd} 256 3 768 \
	&& ${cmd} 256 4 768 \
	&& ${cmd} 256 2 768 \
	&& ${cmd} 256 3 512 \
	&& ${cmd} 256 2 512 \
	&& ${cmd} 256 1 512 \
	&& ${cmd} 256 4 512 \
	&& ${cmd} 256 6 384 \
	&& ${cmd} 256 2 384 \
	&& ${cmd} 256 4 384 \
	&& ${cmd} 256 5 384 \
	&& ${cmd} 256 3 257 \
	&& ${cmd} 256 1 257 \
	&& ${cmd} 256 7 257 \
	&& ${cmd} 256 2 257 \
	&& ${cmd} 256 2 256 \
	&& ${cmd} 256 4 256 \
	&& ${cmd} 256 3 256 \
	&& ${cmd} 256 8 255
${cmd} 256 16 160 \
	&& ${cmd} 256 12 160 \
	&& ${cmd} 256 8 160 \
	&& ${cmd} 256 9 64 \
	&& ${cmd} 256 12 64 \
	&& ${cmd} 256 13 64 \
	&& ${cmd} 256 16 64 \
	&& ${cmd} 256 13 60 \
	&& ${cmd} 256 16 48 \
	&& ${cmd} 256 16 33 \
	&& ${cmd} 256 9 32 \
	&& ${cmd} 256 16 32 \
	&& ${cmd} 256 13 20 \
	&& ${cmd} 256 2 4 \
	&& ${cmd} 256 1 3 \
	&& ${cmd} 256 2 3 \
	&& ${cmd} 256 1 1 \
	&& ${cmd} 255 3 256 \
	&& ${cmd} 255 1 256 \
	&& ${cmd} 255 7 256 \
	&& ${cmd} 255 2 256 \
	&& ${cmd} 255 14 128 \
	&& ${cmd} 255 12 85 \
	&& ${cmd} 255 6 85 \
	&& ${cmd} 255 4 85 \
	&& ${cmd} 255 7 85 \
	&& ${cmd} 255 9 85 \
	&& ${cmd} 255 10 85 \
	&& ${cmd} 255 12 64 \
	&& ${cmd} 255 15 34 \
	&& ${cmd} 255 16 32 \
	&& ${cmd} 255 16 16
${cmd} 16 16 9 \
	&& ${cmd} 12 7 16 \
	&& ${cmd} 12 6 16 \
	&& ${cmd} 12 3 16 \
	&& ${cmd} 12 12 8 \
	&& ${cmd} 12 24 8 \
	&& ${cmd} 9 9 16 \
	&& ${cmd} 9 9 8 \
	&& ${cmd} 9 9 4 \
	&& ${cmd} 9 16 4 \
	&& ${cmd} 9 9 2 \
	&& ${cmd} 8 16 1 \
	&& ${cmd} 8 9 1 \
	&& ${cmd} 7 15 1 \
	&& ${cmd} 5 16 1 \
	&& ${cmd} 5 15 1 \
	&& ${cmd} 4 16 1 \
	&& echo "EVERYTHING LOOKS GOOD"
echo "mega1-sample.sh ${cmd} ... DONE"
