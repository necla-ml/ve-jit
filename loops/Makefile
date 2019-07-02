SHELL=/bin/bash	
NCXX:=nc++-2.1.28
# on Aurora hosts, gcc is extremely old version, with incomplete <regex> impl
GCXX:=clang++
GCC:=clang
CLANG_VI_FLAGS?=-show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics
CXXLANG_FLAGS:=-target linux-ve -O3 -mllvm $(CLANG_VI_FLAGS)

.PHONY: all more ve force force-lib mega1-sample.log mega1
all: fuse2 fuse2lin fuse3 mega1-sample.log cfuse2 cf3
	@echo 'ALL DONE with make all'
	@echo ' extra targets:'
	@echo '    more  force-lib ../libjit1[-x86].a'
	@echo '    mega1 mega1.log mega2 mega2.log mega1-sample.log'
more: all tf2-1.log tf2-2.log tf2-3.log tf2-4.log tf2-0.log tf2-5.log tf2-m.log mega1-sample.log
# These targets need access to ncc and VE hardware
ve: cf3-sample.log cf3u-sample.log	

# This may be somewhat lengthy
force: force-lib
	rm -f tf2-1.log tf2-2.log tf2-3.log tf2-4.log tf2-0.log tf2-5.log tf2-m.log
	$(MAKE) 2>&1 | tee tf2-force.log
.PHONY: ../libjit1-x86.a
../libjit1-x86.a:
	#$(MAKE) -C .. $(patsubst ../%,%,$@)
	#rm -f $@ ../*-x86.o
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

COMMON_HDRS:=../stringutil.hpp ../codegenasm.hpp ../asmfmt_fwd.hpp ../asmfmt_fwd.hpp exechash.hpp ../fuseloop.hpp
fuse2 fuse3 fuse2-ve fuse3-ve: $(COMMON_HDRS)
fuse2: fuse2.cpp ../libjit1-x86.a ../libveli-x86.a
	echo "CC=${CC}"
	echo "LDLIBS=${LDLIBS}"
	#$(GCXX) -Wall -Wno-unknown-pragmas -Werror -std=c++11 -ggdb -O3 $(filter-out %.hpp,$^) -o $@ ${X86FLAGS} ${X86LIBS}
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 $(filter-out %.hpp,$^) -o $@ ${X86FLAGS} ${X86LIBS}
fuse3: fuse3.cpp ../libjit1-x86.a ../libveli-x86.a
	echo "CC=${CC}"
	echo "LDLIBS=${LDLIBS}"
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 $(filter-out %.hpp,$^) -o $@ ${X86FLAGS} ${X86LIBS}
fuse2-ve: fuse2.cpp ../libjit1.a ../libveli.a
	echo "CC=${CC}"
	echo "LDLIBS=${LDLIBS}"
	$(CXX) -Wall -Werror -std=c++11 -g2 -O3 $(filter-out %.hpp,$^) -o $@ ${CXXFLAGS} ${LDLIBS}
fuse3-ve: ../stringutil.hpp ../codegenasm.hpp ../asmfmt_fwd.hpp ../asmfmt_fwd.hpp
fuse3-ve: fuse3.cpp ../libjit1.a ../libveli.a
	echo "CC=${CC}"
	echo "LDLIBS=${LDLIBS}"
	$(CXX) -Wall -Werror -std=c++11 -g2 -O3 $(filter-out %.hpp,$^) -o $@ ${CXXFLAGS} ${LDLIBS}
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
mega1-cf3: Makefile cf3
	# for VE, concentrate on example codes at VL=256 or 255.
	rm -f $@.log; \
	( \
	for vlen in 256 255 254 16 8 9 15 13 12 11 10 7 6 5 4 3 24 48 127 128 253 2 1; do \
	for jj in 3 4 5 6 7 8 9 10 11 12 13 14 16 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 36 40 41 42 43 48 60 63 64 72 84 85 86 128 160 192 224 255 256 257 384 511 512 513 640 768 896 992 1023 1024 1025 1152 1280 2 1; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./cf3.sh $$vlen $$ii $$jj CHECK; \
	rm -f cf/CHECK-$${vl}_$${ii}_$${jj}-vi{,.i,.s}; \
	done; done; done; \
	) >& $@.log;
mega1-cf3u: Makefile cf3
	# for VE, concentrate on example codes at VL=256 or 255.
	rm -f $@.log; \
	( \
	for vlen in 256 255 254 16 8 9 15 13 12 11 10 7 6 5 4 3 24 48 127 128 253 2 1; do \
	for jj in 3 4 5 6 7 8 9 10 11 12 13 14 16 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 36 40 41 42 43 48 60 63 64 72 84 85 86 128 160 192 224 255 256 257 384 511 512 513 640 768 896 992 1023 1024 1025 1152 1280 2 1; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./cf3u.sh $$vlen $$ii $$jj CHECK; \
	rm -f cf/CHECK-$${vl}_$${ii}_$${jj}-vi{,.i,.s}; \
	done; done; done; \
	) >& $@.log;
mega1-cf4: Makefile cf4
	# for VE, concentrate on example codes at VL=256 or 255.
	rm -f $@.log; \
	( \
	for vlen in 256 255 254 16 8 9 15 13 12 11 10 7 6 5 4 3 24 48 127 128 253 2 1; do \
	for jj in 3 4 5 6 7 8 9 10 11 12 13 14 16 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 36 40 41 42 43 48 60 63 64 72 84 85 86 128 160 192 224 255 256 257 384 511 512 513 640 768 896 992 1023 1024 1025 1152 1280 2 1; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./cf4u.sh $$vlen $$ii $$jj CHECK 8; \
	rm -f tmpcf4u8/CHECK-$${vl}_$${ii}_$${jj}-vi{,.i,.s}; \
	done; done; done; \
	) >& $@.log;
mega1-cf5: Makefile cf5
	# for VE, concentrate on example codes at VL=256 or 255.
	rm -f $@.log; \
	( \
	for vlen in 256 255 254 16 8 9 15 13 12 11 10 7 6 5 4 3 24 48 127 128 253 2 1; do \
	for jj in 3 4 5 6 7 8 9 10 11 12 13 14 16 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 36 40 41 42 43 48 60 63 64 72 84 85 86 128 160 192 224 255 256 257 384 511 512 513 640 768 896 992 1023 1024 1025 1152 1280 2 1; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./cf.sh cf5 $$vlen $$ii $$jj CHECK 8; \
	rm -f tmpcf4u8/CHECK-$${vl}_$${ii}_$${jj}-vi{,.i,.s}; \
	done; done; done; \
	) >& $@.log;
mega1-cf5u: Makefile cf5
	# for VE, concentrate on example codes at VL=256 or 255.
	rm -f $@.log; \
	( \
	for vlen in 256 255 254 16 8 9 15 13 12 11 10 7 6 5 4 3 24 48 127 128 253 2 1; do \
	for jj in 3 4 5 6 7 8 9 10 11 12 13 14 16 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 36 40 41 42 43 48 60 63 64 72 84 85 86 128 160 192 224 255 256 257 384 511 512 513 640 768 896 992 1023 1024 1025 1152 1280 2 1; do \
	for ii in `seq 1 16`; do \
	echo "vl,ii,jj= $$vlen $$ii $$jj"; \
	./cf.sh cf5 -a $$vlen $$ii $$jj CHECK 8; \
	rm -f tmpcf4u8/CHECK-$${vl}_$${ii}_$${jj}-vi{,.i,.s}; \
	done; done; done; \
	) >& $@.log;
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
	grep 'ExecHash' $@.log >> $@-hash.log
	sort -k2,3 --unique $@-hash.log > $@-uniq.log
	sort -r -n -k5 -k7 --stable $@-uniq.log > $@-uniqs.log
mega2.log mega2-hash.log mega2-uniq.log mega2-uniqs.log:
	if [ ! -f "$@" ]; then $(MAKE) mega2; fi
mega1-sample.log: Makefile fuse2
	# all code paths from mega1-uniqs.log,
	#    stressing VL=256 and Vl=255
	#    some code paths still have smaller VL examples
	( echo "$@" \
		&& ./mega1-sample.sh './fuse2 -t' \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
.PRECIOUS: cf2-sample.log
cf2-sample.log: Makefile cfuse2
	( echo "$@" \
		&& ./mega1-sample.sh ./cf2.sh \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
.PRECIOUS: cf3-sample.log cf3u-sample.log
cf3-sample.log: Makefile cf3
	( echo "$@" \
		&& ./mega1-sample.sh ./cf3.sh \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf3u-sample.log: Makefile cf3
	( echo "$@" \
		&& ./mega1-sample.sh ./cf3u.sh \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
.PRECIOUS: cf4-sample.log cf4u-sample.log
cf4-sample.log: Makefile cf4
	( echo "$@" \
		&& ./mega1-sample.sh ./cf4u.sh CHECK 0 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf4u-sample.log: Makefile cf4
	( echo "$@" \
		&& ./mega1-sample.sh ./cf4u.sh \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
.PRECIOUS: cf5u-sample.log cf5u-sample2.log cf5-sample.log cf5-sample2.log
.PHONY: cf5-logs	
cf5-logs: cf5
	echo "cf5.log"   && ./cf5-sample2.sh "./cf5.sh   " CHECK 0 >& cf5.log
	echo "cf5a.log"  && ./cf5-sample2.sh "./cf5.sh -a" CHECK 0 >& cf5a.log
	echo "cf5u.log"  && ./cf5-sample2.sh "./cf5.sh   " CHECK 8 >& cf5u.log
	echo "cf5ua.log" && ./cf5-sample2.sh "./cf5.sh -a" CHECK 8 >& cf5ua.log

cf5-sample.log: Makefile cf5
	( echo "$@" \
		&& ./mega1-sample.sh ./cf5.sh CHECK 0 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf5-sample2.log: Makefile cf5
	( echo "$@" \
		&& ./cf5-sample2.sh ./cf5.sh CHECK 0 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf5u-sample.log: Makefile cf5
	( echo "$@" \
		&& CLANG_FLAGS="-Wall -Werror" ./mega1-sample.sh ./cf5.sh CHECK 8 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf5u-sample2.log: Makefile cf5
	( echo "$@" \
		&& CLANG_FLAGS="-Wall -Werror" ./cf5-sample2.sh ./cf5.sh CHECK 8 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf5u-sample3.log: Makefile cf5
	( echo "$@" \
		&& CLANG_FLAGS="-Wall -Werror" ./cf5-sample3.sh "./cf.sh cf5   " CHECK 8 \
		&& echo "EVERYTHING LOOKS GOOD" \
		) 2>&1 | tee $@
cf5ua-sample3.log: Makefile cf5
	( echo "$@" \
		&& CLANG_FLAGS="-Wall -Werror" ./cf5-sample3.sh "./cf.sh cf5 -a" CHECK 8 \
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
mul_add_shr_div: mul_add_shr_div.cpp ../timer.h ../intutil.h
	$(GCXX) -Wall -std=c++11 -I.. -O3 $< -o $@
tdivmod: tdivmod.cpp ../timer.h ../intutil.h
	$(GCXX) -Wall -std=c++11 -I.. -O3 -E $< -o $@.i
	$(GCXX) -Wall -std=c++11 -I.. -O3 -S $< -o $@.s
	$(GCXX) -Wall -std=c++11 -I.. -O3 $< -o $@
tdivmod-ve: tdivmod.cpp ../timer.h ../intutil.h
	clang++ ${CXXLANG_FLAGS} -Wall -std=c++11 -I.. -O3 -S $< -o $@.s
	$(NCXX) $(CFLAGS) $@.s -o $@
cfuse: cfuse.cpp ../libjit1-x86.a
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
cfuse2: cfuse2.cpp ../libjit1-x86.a ../fuseloop.hpp ../cblock.hpp ../vechash.hpp ./exechash.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@

cf3-%.o: cf3-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} -c $< -o $@
cf3-%-ve.o: cf3-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} -c $< -o $@
cf3: cf3.cpp cf3-nounroll.o cf3-unroll.o ../libjit1-x86.a ../vechash.hpp ./exechash.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
cf3-ve: cf3.cpp cf3-nounroll-ve.o cf3-unroll-ve.o ../libjit1-ve.a ../vechash.hpp ./exechash.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@

cf4-%.o: cf4-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} -c $< -o $@
cf4-%-ve.o: cf4-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} -c $< -o $@
cf4: cf4.cpp cf4-nounroll.o cf4-unroll.o ../libjit1-x86.a ../vechash.hpp ./exechash.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
cf4-ve: cf4.cpp cf4-nounroll-ve.o cf4-unroll-ve.o ../libjit1-ve.a ../vechash.hpp ./exechash.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@

cf5-%.o: cf5-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} -c $< -o $@
cf5-%-ve.o: cf5-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} -c $< -o $@
cf5: cf5.cpp cf3-common.o cf5-nounroll.o cf5-unroll.o ../libjit1-x86.a ../vechash.hpp ./exechash.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
cf5-ve: cf5.cpp cf3-common-ve.o cf5-nounroll-ve.o cf5-unroll-ve.o ../libjit1-ve.a ../vechash.hpp ./exechash.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
.PHONY: cf5.log	
cf5.log:	
	{  ./cf.sh cf5 16 0:6:8 8 PRINT 0  \
	&& ./cf.sh cf5 16 4:6:8 8 PRINT 0  \
	&& ./cf.sh cf5 16 4:6:8 8 PRINT 0  \
	&& ./cf.sh cf5 16 5:6:9 8 PRINT 0  \
	&& ./cf.sh cf5 8 0:6:8 16 PRINT 0  \
	&& ./cf.sh cf5 8 4:6:8 16 PRINT 0  \
	&& ./cf.sh cf5 8 3:6:8 16 PRINT 0  \
	&& echo YAY; \
	} >& $@ && echo YAY || mv cf5.log cf5.err
	stop here
	{ ./cf.sh cf5 16 0:8 8 CHECK 0  \
	&& ./cf.sh cf5 16 4:8 8 CHECK 0  \
	&& ./cf.sh cf5 16 4:8 8 CHECK 0  \
	&& ./cf.sh cf5 16 5:9 8 CHECK 0  \
	&& ./cf.sh cf5 8 0:8 16 CHECK 0  \
	&& ./cf.sh cf5 8 4:8 16 CHECK 0  \
	&& ./cf.sh cf5 8 3:8 16 CHECK 0  \
	&& echo YAY; \
	} >& $@ && echo YAY || mv cf5.log cf5.err

fl6.hpp: fl6-kernels.hpp ../fuseloop.hpp
fl6-kernels.hpp: ../cblock.hpp
fl6-kernels.o: fl6-kernels.cpp fl6-kernels.hpp ../cblock.hpp ../stringutil.hpp ../vechash.hpp ../fuseloop.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} -c $< -o $@
fl6-%.o: fl6-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} -c $< -o $@
fl6-%-ve.o: fl6-%.cpp cf3.hpp ../fuseloop.hpp ../cblock.hpp ../stringutil.hpp ../ve_divmod.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} -c $< -o $@
fl6: fl6.cpp fl6-kernels.o cf3-common.o fl6-nounroll.o fl6-unroll.o ../libjit1-x86.a ../vechash.hpp ./exechash.hpp
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
fl6-ve: fl6.cpp fl6-kernels-ve.o cf3-common-ve.o fl6-nounroll-ve.o fl6-unroll-ve.o ../libjit1-ve.a ../vechash.hpp ./exechash.hpp
	$(NCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${CXXFLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@

cfuse3: cfuse3.cpp ../libjit1-x86.a
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@
%: %.cpp
	$(GCXX) -Wall -std=c++11 -I.. -O3 -E $< -o $@.i
	$(GCXX) -Wall -std=c++11 -I.. -O3 $< -o $@
clean:
	rm -f *.i *.o *.gch a.out fuse2 fuse2lin fuse2-ve fuse3 fuse3-ve repro \
		cfuse2 tdivmod tdivmod-ve cf3 cf3u cf3*.o
	rm -f a.s  cblock cfuse libgcc_lucky.so
	rm -f *.exe *.exe.stackdump
realclean: clean	
	rm -f *.log
#	
