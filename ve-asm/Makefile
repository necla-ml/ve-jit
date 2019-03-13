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

else
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
#CXXFLAGS:=$(CFLAGS) -std=c++11
CXXFLAGS:=$(CFLAGS) -std=gnu++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
TARGETS += jitve0
TARGETS += asmkern1.bin
TARGETS += syscall_hello
TARGETS += asmkern0.asm
TARGETS += jitve_hello
TARGETS += jitpp_hello
TARGETS += jitve_math
TARGETS += smir
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
# another inline assembler demo of 'smir' op	
smir: smir.cpp
	$(CXX) $(CFLAGS) $^ -o $@
	./$@
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

clean:
	rm -f *.o *.i tmp*.S a.out ftrace.out *.gch
	rm -f syscall_hello.asm syscall_hello.dis
	rm -f asmkern0.asm asmkern0.dis syscall_hello.s
	rm -f jitve_hello.asm jitve_hello.s jitve_hello.dis jitve*.dis
	rm -f jitpp_hello.asm jitve_math.asm
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
	for f in tmp_*.S; do b=`basename $$f .S`; rm -f $$b.asm $$b.dis $$b.dump; done
	rm -rf tmp
realclean: clean	
	rm -f $(TARGETS) jitve_hello.s bin.mk *.log
# last line
