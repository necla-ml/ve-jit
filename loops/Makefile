SHELL=/bin/bash	
.PHONY: all more force force-lib mega1-sample.log mega1
all: fuse2 fuse2lin mega1-sample.log
	@echo 'ALL DONE with make all'
	@echo ' extra targets:'
	@echo '    more  force-lib ../libjit1[-x86].a'
	@echo '    mega1 mega1.log mega2 mega2.log'
more: all tf2-1.log tf2-2.log tf2-3.log tf2-4.log tf2-0.log tf2-5.log tf2-m.log tf2-sample.log

# This may be somewhat lengthy
force: force-lib
	rm -f tf2-1.log tf2-2.log tf2-3.log tf2-4.log tf2-0.log tf2-5.log tf2-m.log
	$(MAKE) 2>&1 | tee tf2-force.log
.PHONY: ../libjit1-x86.a
../libjit1-x86.a:
	#$(MAKE) -C .. $(patsubst ../%,%,$@)
	rm -f $@ ../*-x86.o
	$(MAKE) -C .. $(patsubst ../%,%,$@)
force-lib:
	rm -f ../*-x86.a ../*-x86.o
	$(MAKE) -C .. libjit1-x86.a

LDFLAGS?=
LDLIBS?=
X86FLAGS:=
X86LIBS:=
X86LIBS+=-ldl
ifneq ($(CC:ncc%=ncc),ncc)
LDLIBS+=${X86LIBS}
endif

fuse2: ../stringutil.hpp ../codegenasm.hpp ../asmfmt_fwd.hpp ../asmfmt_fwd.hpp
fuse2: fuse2.cpp vechash.cpp ../libjit1-x86.a ../libveli-x86.a
	echo "CC=${CC}"
	echo "LDLIBS=${LDLIBS}"
	g++ -Wall -std=c++11 -ggdb -O3 $(filter-out %.hpp,$^) -o $@ ${X86FLAGS} ${X86LIBS}
.PHONY: mega1
# Instead of randomly trying a bunch of tf2-FOO.log targets,
# we now record and execution hash of program flow to create
# a set of "interesting" test cases.
# 1. 'make mega1' to rebuild mega1-uniqs.log (several minutes)
# 2. update the mega1-sample.log target in this Makefile
# 3. rerun 'make mega1-sample.log' to cover a large variety of execution paths
mega1: Makefile fuse2
	# for VE, concentrate on example codes at VL=256 or 255.
	rm -f $@.log; \
	( \
	for vlen in 256 255 254 16 8 9 15 13 12 11 10 7 6 5 4 3 24 48 127 128 253 2 1; do \
	for jj in 3 4 5 6 7 8 9 10 11 12 13 14 16 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 36 40 41 42 43 48 60 63 64 72 84 85 86 128 160 192 224 255 256 257 384 511 512 513 640 768 896 992 1023 1024 1025 1152 1280 2 1; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./fuse2 -t $$vlen $$ii $$jj; \
	done; done; done; \
	) >& $@.log;
	grep 'ExecHash' $@.log > $@-hash.log;
	# oh.  I want --unique FIRST ENCOUNTERED
	sort -k2,3 --unique --stable $@-hash.log > $@-uniq.log
	sort -r -n -k5 -k7 --stable $@-uniq.log > $@-uniqs.log
mega1.log mega1-hash.log mega1-uniq.log mega1-uniqs.log:
	if [ ! -f "$@" ]; then $(MAKE) mega1; fi
mega2: Makefile fuse2
	rm -f $@.log; \
	( \
	for vlen in 256 8 1 2 3 4 5 24 120 127 255 256; do \
	for jj in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 23 24 48 59 60 61 62 63 64 65 256 1024 1023; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./fuse2 -t $$vlen $$ii $$jj; \
	done; done; done; \
	) >> $@.log;
	grep 'ExecHash' $@.log >> $@-hash.log; \
	sort -k2,3 --unique $@-hash.log > $@-hash-unique.log
mega1-sample.log: Makefile fuse2
	# all code paths from mega1-uniqs.log,
	#    stressing VL=256 and Vl=255
	#    some code paths still have smaller VL examples
	( echo "$@" \
	&& ./fuse2 -t 256 2 1024 \
	&& ./fuse2 -t 256 1 1024 \
	&& ./fuse2 -t 256 3 1024 \
	&& ./fuse2 -t 256 4 1024 \
	&& ./fuse2 -t 256 4 768 \
	&& ./fuse2 -t 256 2 768 \
	&& ./fuse2 -t 256 3 768 \
	&& ./fuse2 -t 256 3 512 \
	&& ./fuse2 -t 256 1 512 \
	&& ./fuse2 -t 256 2 512 \
	&& ./fuse2 -t 256 4 512 \
	&& ./fuse2 -t 256 4 384 \
	&& ./fuse2 -t 256 1 257 \
	&& ./fuse2 -t 256 3 257 \
	&& ./fuse2 -t 256 2 257 \
	&& ./fuse2 -t 256 4 256 \
	&& ./fuse2 -t 256 2 256 \
	&& ./fuse2 -t 256 3 256 \
	&& ./fuse2 -t 256 8 160 \
	&& ./fuse2 -t 256 12 64 \
	&& ./fuse2 -t 256 13 64 \
	&& ./fuse2 -t 256 9 64 \
	&& ./fuse2 -t 256 16 64 \
	&& ./fuse2 -t 256 13 60 \
	&& ./fuse2 -t 256 16 48 \
	&& ./fuse2 -t 256 16 33 \
	&& ./fuse2 -t 256 16 32 \
	&& ./fuse2 -t 256 9 32 \
	&& ./fuse2 -t 256 13 20 \
	&& ./fuse2 -t 256 2 4 \
	&& ./fuse2 -t 256 1 3 \
	&& ./fuse2 -t 256 2 3 \
	&& ./fuse2 -t 256 1 1 \
	&& ./fuse2 -t 255 1 256 \
	&& ./fuse2 -t 255 3 256 \
	&& ./fuse2 -t 255 12 85 \
	&& ./fuse2 -t 255 10 85 \
	&& ./fuse2 -t 255 6 85 \
	&& ./fuse2 -t 255 4 85 \
	&& ./fuse2 -t 255 12 64 \
	&& ./fuse2 -t 255 15 34 \
	&& ./fuse2 -t 255 16 16 \
	&& ./fuse2 -t 9 9 16 \
	&& ./fuse2 -t 9 9 4 \
	&& ./fuse2 -t 9 9 2 \
	&& ./fuse2 -t 8 9 1 \
	&& ./fuse2 -t 8 16 1 \
	&& ./fuse2 -t 7 15 1 \
	&& ./fuse2 -t 5 16 1 \
	&& ./fuse2 -t 5 15 1 \
	&& ./fuse2 -t 4 16 1 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
sample1.log: fuse2 Makefile
	# This runs the first-encountered ExecHash paths of mega1-hash-unique.log
	# to give a sampling of "different" code output strategies.
	( echo "$@" \
	&& ./fuse2 -t 8 2 24 \
	&& ./fuse2 -t 8 2 32 \
	&& ./fuse2 -t 1 1 25 \
	&& ./fuse2 -t 4 3 4 \
	&& ./fuse2 -t 1 1 9 \
	&& ./fuse2 -t 4 2 2 \
	&& ./fuse2 -t 8 4 16 \
	&& ./fuse2 -t 8 1 24 \
	&& ./fuse2 -t 8 2 8 \
	&& ./fuse2 -t 8 3 16 \
	&& ./fuse2 -t 8 3 24 \
	&& ./fuse2 -t 4 7 4 \
	&& ./fuse2 -t 8 2 12 \
	&& ./fuse2 -t 8 1 32 \
	&& ./fuse2 -t 6 2 3 \
	&& ./fuse2 -t 1 1 17 \
	&& ./fuse2 -t 1 1 1 \
	&& ./fuse2 -t 8 1 16 \
	&& ./fuse2 -t 8 3 8 \
	&& ./fuse2 -t 8 2 16 \
	&& ./fuse2 -t 2 1 2 \
	&& ./fuse2 -t 4 5 4 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@

tf2-m.log: fuse2 Makefile
	( echo "$@" \
		&& ./fuse2 -m  8 1 1 \
		&& ./fuse2 -t  256 1024  1 \
		&& ./fuse2 -t  256 1500  1 \
		&& ./fuse2 -t  256 512  2 \
		&& ./fuse2 -t  256 16  64 \
		&& ./fuse2 -t  256 21  64 \
		&& ./fuse2 -t  256 1 256 \
		&& ./fuse2 -t  256 2 256 \
		&& ./fuse2 -t  256 3 256 \
		&& ./fuse2 -t  256 4 256 \
		&& ./fuse2 -t  256 1 512 \
		&& ./fuse2 -t  256 4 512 \
		&& ./fuse2 -t  256 3 1024 \
		&& ./fuse2 -t  256 256 3 \
		&& ./fuse2 -t  255 256 3 \
		&& ./fuse2 -t  255 170 3 \
		&& ./fuse2 -t  255 120 3 \
		&& ./fuse2 -t  255 85 3 \
		&& ./fuse2 -t  255 60 3 \
		&& ./fuse2 -t  255 5 510 \
		&& ./fuse2 -t  255 5 500 \
		&& ./fuse2 -t  255 5 500 \
		&& ./fuse2 -t  256 5 384 \
		&& ./fuse2 -t  256 13 13 \
		&& ./fuse2 -t  256 26 13 \
		&& ./fuse2 -t  256 5 256 \
		&& ./fuse2 -t  256 2 512 \
		&& ./fuse2 -t  255 1 255 \
		&& ./fuse2 -t  255 2 255 \
		&& ./fuse2 -t  255 3 255 \
		&& ./fuse2 -t  255 4 255 \
		&& ./fuse2 -t  255 2 510 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@

tf2-1.log: fuse2 Makefile # vl%jj == 0 (case 1)
	( echo "$@" \
		&& ./fuse2 -t  8  1  1 \
		&& ./fuse2 -t  8  2  1 \
		&& ./fuse2 -t  8  3  1 \
		&& ./fuse2 -t  8  4  1 \
		&& ./fuse2 -t  8  5  1 \
		&& ./fuse2 -t  8  6  1 \
		&& ./fuse2 -t  8  7  1 \
		&& ./fuse2 -t  8  8  1 \
		&& ./fuse2 -t  8 24  1 \
		&& ./fuse2 -t  8 28  1 \
		&& ./fuse2 -t  8 96  1 \
		&& ./fuse2 -t  8 97  1 \
		&& ./fuse2 -t  8  1  2 \
		&& ./fuse2 -t  8  2  2 \
		&& ./fuse2 -t  8  3  2 \
		&& ./fuse2 -t  8  4  2 \
		&& ./fuse2 -t  8  5  2 \
		&& ./fuse2 -t  8  6  2 \
		&& ./fuse2 -t  8  7  2 \
		&& ./fuse2 -t  8  8  2 \
		&& ./fuse2 -t  8 24  2 \
		&& ./fuse2 -t  8 28  2 \
		&& ./fuse2 -t  8 96  2 \
		&& ./fuse2 -t  8 97  2 \
		&& ./fuse2 -t  8  1  3 \
		&& ./fuse2 -t  8  2  3 \
		&& ./fuse2 -t  8  3  3 \
		&& ./fuse2 -t  8  4  3 \
		&& ./fuse2 -t  8  5  3 \
		&& ./fuse2 -t  8  1  4 \
		&& ./fuse2 -t  8  2  4 \
		&& ./fuse2 -t  8  3  4 \
		&& ./fuse2 -t  8  4  4 \
		&& ./fuse2 -t  8  5  4 \
		&& ./fuse2 -t  8  6  4 \
		&& ./fuse2 -t  8  7  4 \
		&& ./fuse2 -t  8  8  4 \
		&& ./fuse2 -t  8 24  4 \
		&& ./fuse2 -t  8 28  4 \
		&& ./fuse2 -t  8 96  4 \
		&& ./fuse2 -t  8 97  4 \
		&& ./fuse2 -t  8  1  5 \
		&& ./fuse2 -t  8  1  6 \
		&& ./fuse2 -t  8  1  7 \
		&& ./fuse2 -t  8  1  8 \
		&& ./fuse2 -t  8  2  8 \
		&& ./fuse2 -t  8  3  8 \
		&& ./fuse2 -t  8  4  8 \
		&& ./fuse2 -t  8  5  8 \
		&& ./fuse2 -t  8  6  8 \
		&& ./fuse2 -t  8  7  8 \
		&& ./fuse2 -t  8  8  8 \
		&& ./fuse2 -t  8 24  8 \
		&& ./fuse2 -t  8 25  8 \
		&& ./fuse2 -t  8 26  8 \
		&& ./fuse2 -t  8 27  8 \
		&& ./fuse2 -t  8 28  8 \
		&& ./fuse2 -t  8 29  8 \
		&& ./fuse2 -t  8 30  8 \
		&& ./fuse2 -t  8 31  8 \
		&& ./fuse2 -t  8 32  8 \
		&& ./fuse2 -t  8 33  8 \
		&& ./fuse2 -t  8 96  8 \
		&& ./fuse2 -t  8 97  8 \
		&& ./fuse2 -t  7  1  3 \
		&& ./fuse2 -t  7  2  3 \
		&& ./fuse2 -t  7  3  3 \
		&& ./fuse2 -t  7  4  3 \
		&& ./fuse2 -t  7  5  3 \
		&& ./fuse2 -t  9  1  3 \
		&& ./fuse2 -t  9  2  3 \
		&& ./fuse2 -t  9  3  3 \
		&& ./fuse2 -t  9  4  3 \
		&& ./fuse2 -t  9  5  3 \
		&& ./fuse2 -t 15  1  2 \
		&& ./fuse2 -t 15  2  2 \
		&& ./fuse2 -t 15  3  2 \
		&& ./fuse2 -t 15  4  2 \
		&& ./fuse2 -t 15  5  2 \
		&& ./fuse2 -t 15  6  2 \
		&& ./fuse2 -t 15  7  2 \
		&& ./fuse2 -t 15  1  4 \
		&& ./fuse2 -t 15  2  4 \
		&& ./fuse2 -t 15  3  4 \
		&& ./fuse2 -t 15  1  8 \
		&& ./fuse2 -t 15  1  5 \
		&& ./fuse2 -t 15  2  5 \
		&& ./fuse2 -t 15  3  5 \
		&& ./fuse2 -t 15  4  5 \
		&& ./fuse2 -t 15  5  5 \
		&& ./fuse2 -t 15  6  5 \
		&& ./fuse2 -t 15  7  5 \
		&& ./fuse2 -t 15 15  5 \
		&& ./fuse2 -t 15 40  5 \
		&& ./fuse2 -t 15 41  5 \
		&& ./fuse2 -t 15 42  5 \
		&& ./fuse2 -t 15 43  5 \
		&& ./fuse2 -t 15 44  5 \
		&& ./fuse2 -t 15 45  5 \
		&& ./fuse2 -t 15 46  5 \
		&& ./fuse2 -t 15 47  5 \
		&& ./fuse2 -t 15 48  5 \
		&& ./fuse2 -t 15 49  5 \
		&& ./fuse2 -t 15 50  5 \
		&& ./fuse2 -t 15  1  6 \
		&& ./fuse2 -t 15  2  6 \
		&& ./fuse2 -t 15  1 10 \
		&& ./fuse2 -t 15  1 14 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
tf2-2.log: fuse2 Makefile # jj > vl && jj%vl == 0 (case 2)
	( echo "$@" \
		&& ./fuse2 -t  3  1 27 \
		&& ./fuse2 -t  3  2 27 \
		&& ./fuse2 -t  3  3 27 \
		&& ./fuse2 -t  3  4 27 \
		&& ./fuse2 -t  3 13 27 \
		&& ./fuse2 -t  3 24 27 \
		&& ./fuse2 -t  3 96 27 \
		&& ./fuse2 -t  3 97 27 \
		&& ./fuse2 -t  9  1 90 \
		&& ./fuse2 -t  9  2 90 \
		&& ./fuse2 -t  9  3 90 \
		&& ./fuse2 -t  9  4 90 \
		&& ./fuse2 -t  8  1 16 \
		&& ./fuse2 -t  8  2 16 \
		&& ./fuse2 -t  8  3 16 \
		&& ./fuse2 -t  8  4 16 \
		&& ./fuse2 -t  8 13 16 \
		&& ./fuse2 -t  8 24 16 \
		&& ./fuse2 -t  8 96 16 \
		&& ./fuse2 -t  8 97 16 \
		&& ./fuse2 -t  8  7 24 \
		&& ./fuse2 -t  8 12 24 \
		&& ./fuse2 -t  8 13 24 \
		&& ./fuse2 -t  8 24 24 \
		&& ./fuse2 -t  8 96 24 \
		&& ./fuse2 -t  8 97 24 \
		&& ./fuse2 -t  8  7 56 \
		&& ./fuse2 -t  8 12 56 \
		&& ./fuse2 -t  8 13 56 \
		&& ./fuse2 -t  8 24 56 \
		&& ./fuse2 -t  8 96 56 \
		&& ./fuse2 -t  8 97 56 \
		&& ./fuse2 -t  7  1 14 \
		&& ./fuse2 -t  7  2 14 \
		&& ./fuse2 -t  7  3 14 \
		&& ./fuse2 -t  7  4 14 \
		&& ./fuse2 -t  7 13 14 \
		&& ./fuse2 -t  7 24 14 \
		&& ./fuse2 -t  7 96 14 \
		&& ./fuse2 -t  7 97 14 \
		&& ./fuse2 -t  7  7 21 \
		&& ./fuse2 -t  7 12 21 \
		&& ./fuse2 -t  7 13 21 \
		&& ./fuse2 -t  7 24 21 \
		&& ./fuse2 -t  7 96 21 \
		&& ./fuse2 -t  7 97 21 \
		&& ./fuse2 -t  7  7 63 \
		&& ./fuse2 -t  7 12 63 \
		&& ./fuse2 -t  7 13 63 \
		&& ./fuse2 -t  7 24 63 \
		&& ./fuse2 -t  7 96 63 \
		&& ./fuse2 -t  7 97 63 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
tf2-3.log: fuse2 Makefile
	( echo "$@" \
		&& ./fuse2 -t 15  8  2 \
		&& ./fuse2 -t 15 15  2 \
		&& ./fuse2 -t 15 40  2 \
		&& ./fuse2 -t 15 41  2 \
		&& ./fuse2 -t 15 42  2 \
		&& ./fuse2 -t 15 43  2 \
		&& ./fuse2 -t 15 44  2 \
		&& ./fuse2 -t 15 45  2 \
		&& ./fuse2 -t 15 46  2 \
		&& ./fuse2 -t 15 47  2 \
		&& ./fuse2 -t 15 48  2 \
		&& ./fuse2 -t 15 49  2 \
		&& ./fuse2 -t 15 50  2 \
		&& ./fuse2 -t 15  4  4 \
		&& ./fuse2 -t 15  5  4 \
		&& ./fuse2 -t 15  6  4 \
		&& ./fuse2 -t 15  7  4 \
		&& ./fuse2 -t 15 15  4 \
		&& ./fuse2 -t 15 40  4 \
		&& ./fuse2 -t 15 41  4 \
		&& ./fuse2 -t 15 42  4 \
		&& ./fuse2 -t 15 43  4 \
		&& ./fuse2 -t 15 44  4 \
		&& ./fuse2 -t 15 45  4 \
		&& ./fuse2 -t 15 46  4 \
		&& ./fuse2 -t 15 47  4 \
		&& ./fuse2 -t 15 48  4 \
		&& ./fuse2 -t 15 49  4 \
		&& ./fuse2 -t 15 50  4 \
		&& ./fuse2 -t 15  2  8 \
		&& ./fuse2 -t 15  3  8 \
		&& ./fuse2 -t 15  4  8 \
		&& ./fuse2 -t 15  5  8 \
		&& ./fuse2 -t 15  6  8 \
		&& ./fuse2 -t 15  7  8 \
		&& ./fuse2 -t 15 15  8 \
		&& ./fuse2 -t 15 40  8 \
		&& ./fuse2 -t 15 41  8 \
		&& ./fuse2 -t 15 42  8 \
		&& ./fuse2 -t 15 43  8 \
		&& ./fuse2 -t 15 44  8 \
		&& ./fuse2 -t 15 45  8 \
		&& ./fuse2 -t 15 46  8 \
		&& ./fuse2 -t 15 47  8 \
		&& ./fuse2 -t 15 48  8 \
		&& ./fuse2 -t 15 49  8 \
		&& ./fuse2 -t 15 50  8 \
		&& ./fuse2 -t 15  1 16 \
		&& ./fuse2 -t 15  2 16 \
		&& ./fuse2 -t 15  3 16 \
		&& ./fuse2 -t 15  4 16 \
		&& ./fuse2 -t 15  5 16 \
		&& ./fuse2 -t 15  6 16 \
		&& ./fuse2 -t 15  7 16 \
		&& ./fuse2 -t 15 15 16 \
		&& ./fuse2 -t 15 40 16 \
		&& ./fuse2 -t 15 41 16 \
		&& ./fuse2 -t 15 42 16 \
		&& ./fuse2 -t 15 43 16 \
		&& ./fuse2 -t 15 44 16 \
		&& ./fuse2 -t 15 45 16 \
		&& ./fuse2 -t 15 46 16 \
		&& ./fuse2 -t 15 47 16 \
		&& ./fuse2 -t 15 48 16 \
		&& ./fuse2 -t 15 49 16 \
		&& ./fuse2 -t 15 50 16 \
		&& ./fuse2 -t 15  1 64 \
		&& ./fuse2 -t 15  2 64 \
		&& ./fuse2 -t 15  3 64 \
		&& ./fuse2 -t 15  4 64 \
		&& ./fuse2 -t 15  5 64 \
		&& ./fuse2 -t 15  6 64 \
		&& ./fuse2 -t 15  7 64 \
		&& ./fuse2 -t 15 15 64 \
		&& ./fuse2 -t 15 40 64 \
		&& ./fuse2 -t 15 41 64 \
		&& ./fuse2 -t 15 42 64 \
		&& ./fuse2 -t 15 43 64 \
		&& ./fuse2 -t 15 44 64 \
		&& ./fuse2 -t 15 45 64 \
		&& ./fuse2 -t 15 46 64 \
		&& ./fuse2 -t 15 47 64 \
		&& ./fuse2 -t 15 48 64 \
		&& ./fuse2 -t 15 49 64 \
		&& ./fuse2 -t 15 50 64 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
tf2-4.log: fuse2 Makefile
	( echo "$@" \
		&& ./fuse2 -t  8  3  3 \
		&& ./fuse2 -t  8  4  3 \
		&& ./fuse2 -t  8  5  3 \
		&& ./fuse2 -t  8  6  3 \
		&& ./fuse2 -t  8  7  3 \
		&& ./fuse2 -t  8  8  3 \
		&& ./fuse2 -t  8 24  3 \
		&& ./fuse2 -t  8 28  3 \
		&& ./fuse2 -t  8 96  3 \
		&& ./fuse2 -t  8 97  3 \
		&& ./fuse2 -t  8  2  5 \
		&& ./fuse2 -t  8  3  5 \
		&& ./fuse2 -t  8  4  5 \
		&& ./fuse2 -t  8  5  5 \
		&& ./fuse2 -t  8  6  5 \
		&& ./fuse2 -t  8  7  5 \
		&& ./fuse2 -t  8  8  5 \
		&& ./fuse2 -t  8 24  5 \
		&& ./fuse2 -t  8 28  5 \
		&& ./fuse2 -t  8 96  5 \
		&& ./fuse2 -t  8 97  5 \
		&& ./fuse2 -t  8  2  6 \
		&& ./fuse2 -t  8  3  6 \
		&& ./fuse2 -t  8  4  6 \
		&& ./fuse2 -t  8  5  6 \
		&& ./fuse2 -t  8  6  6 \
		&& ./fuse2 -t  8  7  6 \
		&& ./fuse2 -t  8  8  6 \
		&& ./fuse2 -t  8 24  6 \
		&& ./fuse2 -t  8 28  6 \
		&& ./fuse2 -t  8 96  6 \
		&& ./fuse2 -t  8 97  6 \
		&& ./fuse2 -t  8  2  7 \
		&& ./fuse2 -t  8  3  7 \
		&& ./fuse2 -t  8  4  7 \
		&& ./fuse2 -t  8  5  7 \
		&& ./fuse2 -t  8  6  7 \
		&& ./fuse2 -t  8  7  7 \
		&& ./fuse2 -t  8  8  7 \
		&& ./fuse2 -t  8 24  7 \
		&& ./fuse2 -t  8 25  7 \
		&& ./fuse2 -t  8 26  7 \
		&& ./fuse2 -t  8 27  7 \
		&& ./fuse2 -t  8 28  7 \
		&& ./fuse2 -t  8 29  7 \
		&& ./fuse2 -t  8 30  7 \
		&& ./fuse2 -t  8 31  7 \
		&& ./fuse2 -t  8 32  7 \
		&& ./fuse2 -t  8 33  7 \
		&& ./fuse2 -t  8 96  7 \
		&& ./fuse2 -t  8 97  7 \
		&& ./fuse2 -t 15  3  6 \
		&& ./fuse2 -t 15  4  6 \
		&& ./fuse2 -t 15  5  6 \
		&& ./fuse2 -t 15  6  6 \
		&& ./fuse2 -t 15  7  6 \
		&& ./fuse2 -t 15 15  6 \
		&& ./fuse2 -t 15 40  6 \
		&& ./fuse2 -t 15 41  6 \
		&& ./fuse2 -t 15 42  6 \
		&& ./fuse2 -t 15 43  6 \
		&& ./fuse2 -t 15 44  6 \
		&& ./fuse2 -t 15 45  6 \
		&& ./fuse2 -t 15 46  6 \
		&& ./fuse2 -t 15 47  6 \
		&& ./fuse2 -t 15 48  6 \
		&& ./fuse2 -t 15 49  6 \
		&& ./fuse2 -t 15 50  6 \
		&& ./fuse2 -t 15  2 10 \
		&& ./fuse2 -t 15  3 10 \
		&& ./fuse2 -t 15  4 10 \
		&& ./fuse2 -t 15  5 10 \
		&& ./fuse2 -t 15  6 10 \
		&& ./fuse2 -t 15  7 10 \
		&& ./fuse2 -t 15 15 10 \
		&& ./fuse2 -t 15 40 10 \
		&& ./fuse2 -t 15 41 10 \
		&& ./fuse2 -t 15 42 10 \
		&& ./fuse2 -t 15 43 10 \
		&& ./fuse2 -t 15 44 10 \
		&& ./fuse2 -t 15 45 10 \
		&& ./fuse2 -t 15 46 10 \
		&& ./fuse2 -t 15 47 10 \
		&& ./fuse2 -t 15 48 10 \
		&& ./fuse2 -t 15 49 10 \
		&& ./fuse2 -t 15 50 10 \
		&& ./fuse2 -t 15  2 14 \
		&& ./fuse2 -t 15  3 14 \
		&& ./fuse2 -t 15  4 14 \
		&& ./fuse2 -t 15  5 14 \
		&& ./fuse2 -t 15  6 14 \
		&& ./fuse2 -t 15  7 14 \
		&& ./fuse2 -t 15 15 14 \
		&& ./fuse2 -t 15 40 14 \
		&& ./fuse2 -t 15 41 14 \
		&& ./fuse2 -t 15 42 14 \
		&& ./fuse2 -t 15 43 14 \
		&& ./fuse2 -t 15 44 14 \
		&& ./fuse2 -t 15 45 14 \
		&& ./fuse2 -t 15 46 14 \
		&& ./fuse2 -t 15 47 14 \
		&& ./fuse2 -t 15 48 14 \
		&& ./fuse2 -t 15 49 14 \
		&& ./fuse2 -t 15 50 14 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
tf2-5.log: fuse2 Makefile # jj%2==0
	( echo "$@" \
		&& ./fuse2 -t  8  3  3 \
		&& ./fuse2 -t  8  4  3 \
		&& ./fuse2 -t  8  5  3 \
		&& ./fuse2 -t  8  6  3 \
		&& ./fuse2 -t  8  7  3 \
		&& ./fuse2 -t  8  8  3 \
		&& ./fuse2 -t  8 24  3 \
		&& ./fuse2 -t  8 28  3 \
		&& ./fuse2 -t  8 96  3 \
		&& ./fuse2 -t  8 97  3 \
		&& ./fuse2 -t  8  2  5 \
		&& ./fuse2 -t  8  3  5 \
		&& ./fuse2 -t  8  4  5 \
		&& ./fuse2 -t  8  5  5 \
		&& ./fuse2 -t  8  6  5 \
		&& ./fuse2 -t  8  7  5 \
		&& ./fuse2 -t  8  8  5 \
		&& ./fuse2 -t  8 24  5 \
		&& ./fuse2 -t  8 28  5 \
		&& ./fuse2 -t  8 96  5 \
		&& ./fuse2 -t  8 97  5 \
		&& ./fuse2 -t  8  2  7 \
		&& ./fuse2 -t  8  3  7 \
		&& ./fuse2 -t  8  4  7 \
		&& ./fuse2 -t  8  5  7 \
		&& ./fuse2 -t  8  6  7 \
		&& ./fuse2 -t  8  7  7 \
		&& ./fuse2 -t  8  8  7 \
		&& ./fuse2 -t  8 24  7 \
		&& ./fuse2 -t  8 25  7 \
		&& ./fuse2 -t  8 26  7 \
		&& ./fuse2 -t  8 27  7 \
		&& ./fuse2 -t  8 28  7 \
		&& ./fuse2 -t  8 29  7 \
		&& ./fuse2 -t  8 30  7 \
		&& ./fuse2 -t  8 31  7 \
		&& ./fuse2 -t  8 32  7 \
		&& ./fuse2 -t  8 33  7 \
		&& ./fuse2 -t  8 96  7 \
		&& ./fuse2 -t  8 97  7 \
		&& ./fuse2 -t  8  2 19 \
		&& ./fuse2 -t  8  3 19 \
		&& ./fuse2 -t  8  4 19 \
		&& ./fuse2 -t  8  5 19 \
		&& ./fuse2 -t  8  6 19 \
		&& ./fuse2 -t  8  7 19 \
		&& ./fuse2 -t  8  8 19 \
		&& ./fuse2 -t  8 24 19 \
		&& ./fuse2 -t  8 25 19 \
		&& ./fuse2 -t  8 26 19 \
		&& ./fuse2 -t  8 27 19 \
		&& ./fuse2 -t  8 28 19 \
		&& ./fuse2 -t  8 29 19 \
		&& ./fuse2 -t  8 30 19 \
		&& ./fuse2 -t  8 31 19 \
		&& ./fuse2 -t  8 32 19 \
		&& ./fuse2 -t  8 33 19 \
		&& ./fuse2 -t  8 96 19 \
		&& ./fuse2 -t  8 97 19 \
		&& ./fuse2 -t  8  2 21 \
		&& ./fuse2 -t  8  3 21 \
		&& ./fuse2 -t  8  4 21 \
		&& ./fuse2 -t  8  5 21 \
		&& ./fuse2 -t  8  6 21 \
		&& ./fuse2 -t  8  7 21 \
		&& ./fuse2 -t  8  8 21 \
		&& ./fuse2 -t  8 24 21 \
		&& ./fuse2 -t  8 25 21 \
		&& ./fuse2 -t  8 26 21 \
		&& ./fuse2 -t  8 27 21 \
		&& ./fuse2 -t  8 28 21 \
		&& ./fuse2 -t  8 29 21 \
		&& ./fuse2 -t  8 30 21 \
		&& ./fuse2 -t  8 31 21 \
		&& ./fuse2 -t  8 32 21 \
		&& ./fuse2 -t  8 33 21 \
		&& ./fuse2 -t  8 96 21 \
		&& ./fuse2 -t  8 97 21 \
		&& ./fuse2 -t 15  3  7 \
		&& ./fuse2 -t 15  4  7 \
		&& ./fuse2 -t 15  5  7 \
		&& ./fuse2 -t 15  6  7 \
		&& ./fuse2 -t 15  7  7 \
		&& ./fuse2 -t 15 15  7 \
		&& ./fuse2 -t 15 40  7 \
		&& ./fuse2 -t 15 41  7 \
		&& ./fuse2 -t 15 42  7 \
		&& ./fuse2 -t 15 43  7 \
		&& ./fuse2 -t 15 44  7 \
		&& ./fuse2 -t 15 45  7 \
		&& ./fuse2 -t 15 46  7 \
		&& ./fuse2 -t 15 47  7 \
		&& ./fuse2 -t 15 48  7 \
		&& ./fuse2 -t 15 49  7 \
		&& ./fuse2 -t 15 50  7 \
		&& ./fuse2 -t 15  2 11 \
		&& ./fuse2 -t 15  3 11 \
		&& ./fuse2 -t 15  4 11 \
		&& ./fuse2 -t 15  5 11 \
		&& ./fuse2 -t 15  6 11 \
		&& ./fuse2 -t 15  7 11 \
		&& ./fuse2 -t 15 15 11 \
		&& ./fuse2 -t 15 40 11 \
		&& ./fuse2 -t 15 41 11 \
		&& ./fuse2 -t 15 42 11 \
		&& ./fuse2 -t 15 43 11 \
		&& ./fuse2 -t 15 44 11 \
		&& ./fuse2 -t 15 45 11 \
		&& ./fuse2 -t 15 46 11 \
		&& ./fuse2 -t 15 47 11 \
		&& ./fuse2 -t 15 48 11 \
		&& ./fuse2 -t 15 49 11 \
		&& ./fuse2 -t 15 50 11 \
		&& ./fuse2 -t 15  2 17 \
		&& ./fuse2 -t 15  3 17 \
		&& ./fuse2 -t 15  4 17 \
		&& ./fuse2 -t 15  5 17 \
		&& ./fuse2 -t 15  6 17 \
		&& ./fuse2 -t 15  7 17 \
		&& ./fuse2 -t 15 15 17 \
		&& ./fuse2 -t 15 17 17 \
		&& ./fuse2 -t 15 40 17 \
		&& ./fuse2 -t 15 41 17 \
		&& ./fuse2 -t 15 42 17 \
		&& ./fuse2 -t 15 43 17 \
		&& ./fuse2 -t 15 44 17 \
		&& ./fuse2 -t 15 45 17 \
		&& ./fuse2 -t 15 46 17 \
		&& ./fuse2 -t 15 47 17 \
		&& ./fuse2 -t 15 48 17 \
		&& ./fuse2 -t 15 49 17 \
		&& ./fuse2 -t 15 50 17 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
tf2-0.log: fuse2 Makefile tf2-3.log	
	( echo "$@" \
		&& ./fuse2 -t  8  1 9 \
		&& ./fuse2 -t 24 13 8 \
		&& ./fuse2 -t 12 18 8 \
		&& ./fuse2 -t  6 13 8 \
		&& ./fuse2 -t  6 13 9 \
		&& ./fuse2 -t  9 13 6 \
		&& ./fuse2 -t  1  1 2 \
		&& ./fuse2 -t  1  2 2 \
		&& ./fuse2 -t  1  3 2 \
		&& ./fuse2 -t  1  4 2 \
		&& ./fuse2 -t  1  2 4 \
		&& ./fuse2 -t  1  6 3 \
		&& ./fuse2 -t  1  3 6 \
		&& ./fuse2 -t  1 18 6 \
		&& ./fuse2 -t  2 18 6 \
		&& ./fuse2 -t  3 18 6 \
		&& ./fuse2 -t 64 40 36 \
		&& ./fuse2 -t 256 40 144 \
		&& ./fuse2 -t 64 40 12 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cfuse: cfuse.cpp
	g++ -Wall -std=c++11 -I.. -O3 $< -o $@ ${LDFLAGS} ${LDLIBS}
%: %.cpp
	g++ -Wall -std=c++11 -I.. -O3 -E $< -o $@.i
	g++ -Wall -std=c++11 -I.. -O3 $< -o $@
clean:
	rm -f *.i *.o fuse2 fuse2lin repro a.out *.gch
	rm -f a.s  cblock cfuse libgcc_lucky.so
	rm -f *.exe *.exe.stackdump
realclean: clean	
	rm -f *.log
#	
