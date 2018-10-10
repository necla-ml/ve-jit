TARGETS=asmkern0.asm syscall_hello asmkern.bin asmkern1.bin msk \
	jitve0 jitve_hello test_strMconst jitve_math jitpp_hello
all: $(TARGETS)
CFLAGS:=-O2 -g2
CXXFLAGS:=$(CFLAGS) -std=c++11
CC?=ncc-1.5.1
CXX?=nc++-1.5.1
CC:=ncc-1.5.1
CXX:=nc++-1.5.1
CC:=ncc-1.2.4
CXX:=nc++-1.2.4
CC:=ncc-1.5.2
CXX:=nc++-1.5.2
#
# Aurora assembler.S: cpp->.asm, $(CC)->.o, nobjcopy->.bin, .bin-->.dump
# Actually, jitpage.h (newer version of ve_util.h) will use 'bin.mk' makefile
#
%.bin: %.S
	@# .S files can run through *just* the C preprocessor.
	$(CC) -o $*.asm -E $< && sed -i '/^[[:space:]]*$$/d' $*.asm
	@# .bin needs even more than -fPIC (no GOT, PLT, external symbols!)
	$(CC) -o $*.o -c $<
	@# we can get symbols + code + data disassembled
	nobjdump -D $*.o > $*.dis
	@# create a relocatable machine-language blob as follows
	nobjcopy -O binary $*.o $*.bin
	@# we can dump or disassemble the raw machine code
	hexdump -C $*.bin > $*.dump
	@echo "===========================" >> $*.dump
	@echo "nobjdump -b binary -mve -D %*.bin" >> $*.dump
	nobjdump -b binary -mve -D $*.bin >> $*.dump
# short demo of creating a binary blob from some hand-written assembler (no C ABI)
asmkern.bin: asmkern.S
	$(CC) -o asmkern.o -c asmkern.S
	@# if we have a relocatable (PIC) binary blob, we can create it as follows
	nobjcopy -O binary asmkern.o asmkern.bin
	@# we can dump or disassemble the raw machine code
	hexdump -C asmkern.bin
	nobjdump -b binary -mve -D asmkern.bin
#
# a small demo of inline assembler, from Japan
#
asmkern0.asm: asmkern0.c
	$(CC) $(CFLAGS) -c $< -DOPT=1 -o asmkern0.o
	$(CC) $(CFLAGS) -Wa,adhln -S $< -o asmkern0.asm
	nobjdump -d asmkern0.o > asmkern0.dis
#
# low-level C-api, using jitve_util.h
#
jitve0: jitve0.c bin.mk
	$(CC) -O2 $< -o $@
	ve_exec ./$@ 2>&1 | tee $@.log
jitve_util.o: jitve_util.c jitve_util.h
	$(CC) -O2 -c $< -o $@
test_strMconst: test_strMconst.c jitve_util.o
	$(CC) $(CFLAGS) -O2 $^ -o $@
	ve_exec ./$@ 2>&1 | tee $@.log
jitve_hello: jitve_hello.c jitve_util.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	ve_exec ./$@ 2>&1 | tee $@.log
jitve_math: jitve_math.c jitve_util.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	ve_exec ./$@ 2>&1 | tee $@.log
#
# newer api uses jitpage.h (instead of jitve_util.h)
# and supports C++         (asmfmt_fwd.hpp)
# 
libjit1.a: asmfmt.o jitpage.o
	$(AR) cq $@ $^
.PRECIOUS: asmfmt.o
asmfmt.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
.PRECIOUS: jjitpage.o
jitpage.o: jitpage.c jitpage.h
	$(CXX) $(CXXFLAGS) -O2 -c $< -o $@
	#$(CC) $(CFLAGS) -O2 -c $< -o $@
#asmfmt.cpp has a standalone demo program with -D_MAIN compiler	
asmfmt: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp jitpage.o
	$(CXX) $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %,%.i,$@)
	#$(CXX) $(CXXFLAGS) -O2 -D_MAIN $(filter-out %.hpp,$^) -o $@
	$(CXX) ${CXXFLAGS} -D_MAIN asmfmt.cpp jitpage.c -o $@
	ve_exec ./$@ 2>&1 | tee $@.log
#
# C++ version of jitve_hello.
# This one is more complicated. Besides printing hello world,
# it also returns a very lucky value (7).
#
jitpp_hello: jitpp_hello.cpp asmfmt.o jitpage.o
	$(CXX) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<)
	$(CXX) $(CFLAGS) -O2 $^ -o $@
	$(CXX) $(CFLAGS) -Wa,-adhln -c $^ >& $(patsubst %.cpp,%.asm,$<)
	ve_exec ./$@ 2>&1 | tee $@.log
ifeq (0,1)	
	# ccom: ./phase3_src/cg/schedule_vn_sx4.cpp:3372: void VN_TAG::confirm(int, int): Assertion `false' failed.
	# nc++: /opt/nec/ve/ncc/1.5.2/libexec/ccom is abnormally terminated by SIGABRT
msk: msk.cpp
	$(CXX) $(CFLAGS) -E msk.cpp > msk.i
	$(CXX) $(CFLAGS) -Wa,adhln -S msk.cpp -o msk.S
	$(CXX) $(CFLAGS) -g2 msk.cpp -o $@
	nobjdump -d $@ >& msk.dis
	ve_exec ./$@ 2>&1 | tee msk.log
else
msk: msk.cpp
	$(CXX) -O0 -E msk.cpp > msk.i
	$(CXX) -O0 -Wa,adhln -S msk.cpp -o msk.S
	$(CXX) -O0 -g2 msk.cpp -o $@
	nobjdump -d $@ >& msk.dis
	ve_exec ./$@ 2>&1 | tee msk.log
endif
%.asm: %.c
	$(CC) $(CFLAGS) -g2 -Wa,-adhln -S $< >& $*.s
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $*.asm
	$(CC) $(CFLAGS) -c $< -o $*.o
	nobjdump -d $*.o > $*.dis
syscall_hello: syscall_hello.c
	$(CC) $(CFLAGS) -S $< -o syscall_hello.s
	$(MAKE) syscall_hello.asm
	$(CC) $(CFLAGS) $< -o $@ && ve_exec $@
	ve_exec ./$@ 2>&1 | tee $@.log
clean:
	rm -f *.o *.i *.ii *.out
	rm -f msk*.i msk*.S msk*.dis msk*.out
	rm -f syscall_hello.o syscall_hello.asm syscall_hello.dis
	rm -f asmkern0.asm asmkern0.dis
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
	for f in tmp_kernel_*.S; do b=`basename $$f .S`; rm -f $$b.asm $$b.dis $$b.dump; done
	rm -f jitve_math.asm jitve_hello.asm jitve_hello.s jitve_hello.dis jitve*.dis
realclean: clean
	rm -f $(TARGETS)
	rm -f msk msk0 msk1 msk2 msk3 msk4 syscall_hello smir jitve0
	rm -f tmp_kernel_*.S *.bin
	rm -rf CMakeCache.txt CMakeFiles asmfmt asmfmt.txt jitve_hello.s
#
