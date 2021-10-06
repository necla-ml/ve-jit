VEJIT_ROOT:=..
SHELL:=/bin/bash
TARGETS:=

ifneq ($(CC:ncc%=ncc),ncc)
CFLAGS:=-O2 -g2 -pthread
CFLAGS+=-Wall -Werror
CFLAGS+=-Wno-unknown-pragmas
CXXFLAGS:=$(CFLAGS) -std=c++11
VE_EXEC:=time
OBJDUMP:=objdump
OBJCOPY:=objcopy
TARGETS += test_vechash-x86
TARGETS += veli_loadreg-x86
TARGETS+=test_vejitpage_sh-x86

else
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
#CXXFLAGS:=$(CFLAGS) -std=c++11
CXXFLAGS:=$(CFLAGS) -std=gnu++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
TARGETS += asmkern1.bin
TARGETS += asmkern.bin
TARGETS += asmkern0.asm
TARGETS += syscall_hello
TARGETS += jitve_hello
TARGETS += jitpp_hello
TARGETS += jitve0
TARGETS += smir
TARGETS += msk
TARGETS += jitve_math
TARGETS += test_vechash-x86 test_vechash-ve
TARGETS += veli_loadreg-x86 veli_loadreg-ve
TARGETS += test_vejitpage_sh-x86 test_vejitpage_sh-ve
endif

LDFLAGS+=-ldl
CFLAGS+=-I..
CXXFLAGS+=-I..

all: $(TARGETS)

#
# we do not use libjit code here, except for some standalones:
# - lowest-of-low utilities from jitpage.h
# - pretty assembler formatting (AsmFmtCols C++ class)
#
jitpage-ve.o: $(VEJIT_ROOT)/jitpage.c $(VEJIT_ROOT)/jitpage.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl
asmfmt-ve.o: $(VEJIT_ROOT)/asmfmt.cpp $(VEJIT_ROOT)/asmfmt.hpp $(VEJIT_ROOT)/asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -O2 -c $< -o $@
intutil-ve.o: $(VEJIT_ROOT)/intutil.c $(VEJIT_ROOT)/intutil.h
	$(CC) ${CFLAGS} -O2 -c $< -o $@
bin.mk:	
	ln -sf ${VEJIT_ROOT}/bin.mk bin.mk

#
# Oh. now I have a target that does want parent libraries to be
# fully compiled ...
#
../lib%.a:
	${MAKE} VERBOSE=1 -C .. "lib$*.a"

%.asm: %.c
	$(CC) $(CFLAGS) -g2 -Wa,-adhln -S $< >& $*.s
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $*.asm
	$(CC) $(CFLAGS) -c $< -o $*.o
	$(OBJDUMP) -d $*.o > $*.dis
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
	$(OBJDUMP) -D $*.o > $*.dis
	@# create a relocatable machine-language blob as follows
	$(OBJCOPY) -O binary $*.o $*.bin
	@# we can dump or disassemble the raw machine code
	hexdump -C $*.bin > $*.dump
	@echo "===========================" >> $*.dump
	@echo "$(OBJDUMP) -b binary -mve -D %*.bin" >> $*.dump
	$(OBJDUMP) -b binary -mve -D $*.bin >> $*.dump
#
# low-level C funcs (later moved to libjit1), and compiling via bin.mk
#
jitve0: jitve0.c bin.mk
	$(CC) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# a small demo of inline assembler, from Japan
#
asmkern0.asm: asmkern0.c
	$(CC) $(CFLAGS) -c $< -DOPT=1 -o asmkern0.o
	$(CC) $(CFLAGS) -Wa,adhln -S $< -o asmkern0.asm
	$(OBJDUMP) -d asmkern0.o > asmkern0.dis
# short demo of creating a binary blob from some hand-written assembler (no C ABI)
asmkern.bin: asmkern.S
	$(CC) -o asmkern.o -c asmkern.S
	@# if we have a relocatable (PIC) binary blob, we can create it as follows
	$(OBJCOPY) -O binary asmkern.o asmkern.bin
	@# we can dump or disassemble the raw machine code
	hexdump -C asmkern.bin
	$(OBJDUMP) -b binary -mve -D asmkern.bin
# another inline assembler demo of 'smir' op	
smir: smir.cpp
	$(CXX) $(CFLAGS) $^ -o $@
	./$@
# playing with VE mask register, low-level stuff.  Also Msk256/512 classes,
# to check correctness (not needed anywhere else yet)
msk: msk.cpp
	$(CXX) -O0 -E msk.cpp > msk.i
	$(CXX) -O0 -Wa,adhln -S msk.cpp -o msk.S
	$(CXX) -O0 -g2 msk.cpp -o $@
	$(OBJDUMP) -d $@ >& msk.dis
	$(VE_EXEC) ./$@ 2>&1 | tee msk.log
syscall_hello: syscall_hello.c
	$(CC) $(CFLAGS) -S $< -o syscall_hello.s
	$(MAKE) syscall_hello.asm
	$(CC) $(CFLAGS) $< -o $@
	$(VE_EXEC) ./$@ # very short output
jitve_hello: jitve_hello.c jitpage-ve.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@ -ldl
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# C++ version of jitve_hello.
# This one is more complicated. Besides printing hello world,
# it also returns a very lucky value (7).
#
.PRECIOUS: jitpp_hello
jitpp_hello: jitpp_hello.cpp asmfmt-ve.o intutil-ve.o jitpage-ve.o
	$(CXX) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<)
	$(CXX) $(CFLAGS) -O2 $^ -o $@ $(LDFLAGS)
	$(CXX) $(CFLAGS) -Wa,-adhln -c $^ >& $(patsubst %.cpp,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
.PRECIOUS: jitve_math
jitve_math: jitve_math.c jitpage-ve.o bin.mk
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $(filter-out bin.mk,$^) -o $@ -ldl
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
# this also can run on host, in seconds rather than minutes,
# because we do not execute the jit page
# [we create the jit page string, and then hexdump and disassemble it]
#    of course you need ncc available for this to run correctly (FIXME - skip if ncc not found in path)
test_vejitpage_sh: test_vejitpage_sh.cpp vejitpage.sh
	$(CXX) ${CXXFLAGS} $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
test_vejitpage_sh-x86: test_vejitpage_sh.cpp vejitpage.sh
	g++ ${CXXFLAGS} $< -o $@
	./$@ 2>&1 | tee $@.log
test_vejitpage_sh-ve: test_vejitpage_sh.cpp vejitpage.sh
	$(CXX) ${CXXFLAGS} $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log

# vechash is a prototype kernel that updates a hash of a sequence of
# 1 or two vector registers. Usable to compare correctness of asm code
# with an equiv 'C' impl.
test_vechash-x86: test_vechash.cpp ../cblock.cpp ../vechash.cpp ../asmfmt.cpp ../jitpage.c ../intutil.c
	g++ ${CXXFLAGS} $(filter %.cpp,$^) $(filter %.c,$^) -o $@ -ldl
	{ ./$@; echo "exit status $$?"; }
test_vechash-ve: test_vechash.cpp ../vechash.cpp ../asmfmt.cpp ../cblock.cpp ../jitpage.c ../intutil.c
	${CXX} ${CXXFLAGS} -E $< -o test_vechash.i
	${CXX} ${CXXFLAGS} -S $(filter %.cpp,$^) $(filter %.c,$^)
	${CXX} ${CXXFLAGS} $(filter %.cpp,$^) $(filter %.c,$^) -o $@ -ldl #-L. -ljit1-x86
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log

# see vl-run.sh for running veli_loadreg tests
# many tests only need veli_loadreg-x86
# only veli_loadreg -R needs VE (because it runs several
# thousand optimized VE assembler test cases).
WORKING?=6
veli_loadreg-ve: veli_loadreg.cpp ../libveli.a ../libjit1.a bin.mk
	$(CXX) $(CXXFLAGS) -DWORKING=${WORKING} -g -O2 -o $@ $(filter-out bin.mk,$^) $(LDFLAGS)
	@echo "veli_loadreg runs VE Logic for Instructions tests on VE"
	@echo "(veli_loadreg-x86 will do same VE logic tests on x86)"
# bin.mk dependency because "veli_loadreg -J" wants it (note: it is also in libjit1 now)	
veli_loadreg-x86: veli_loadreg.cpp ../libveli-x86.a ../libjit1-x86.a bin.mk
	g++ $(CXXFLAGS) -Wall -DWORKING=${WORKING} -g -O2 -o $@ $(filter-out bin.mk,$^) $(LDFLAGS)

clean:
	rm -f *.o *.i tmp*.S a.out ftrace.out *.gch *.exe 2
	rm -f msk*.i msk*.S msk*.dis msk*.out
	rm -f asmkern0.asm asmkern0.dis syscall_hello.s
	rm -f jitve_hello.asm jitve_hello.s jitve_hello.dis jitve*.dis
	rm -f jitpp_hello.asm jitve_math.asm test_vechash.s
	rm -f tmp_veli_loadreg_big.dis
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
	for f in tmp_*.S; do b=`basename $$f .S`; rm -f $$b.asm $$b.dis $$b.dump; done
	rm -rf tmp
realclean: clean	
	rm -f $(TARGETS) jitve_hello.s bin.mk *.log veli_loadreg-x86 veli_loadreg-ve
	rm -f vl-6.log  vl-a.log  vl-j.log # from vl-run.sh
# last line
