#
# bin.mk (template)
#
# Example of how to compile a [local copy of] a jit '-vi.c' file
#        [copied from ../vednn-ek/test/tmp_cjitConv01/cjitConvFwd_parmstr-vi.c]
#   CFLAGS=-I../vednn-ek/test/vednn-ftrace1/include make VERBOSE=1 -f bin.mk cjitConvFwd_parmstr-ve.o
#
#LDFLAGS?=
$(LIBNAME): $(OBJECTS)
	ncc -o $(LIBNAME) $(LDFLAGS) $(filter %-ve.o,$(OBJECTS))
# This file ONLY generates -ve.o object files or VE .bin binary blobs
# 
# Allow override of default compiler (maybe particular version is required)
# NCC   must be used for .S files (nas possibly, if you don't use cpp preprocessor)
# NCC   must be used for extended assembly (JIT asm with register calling convention)
# CLANG must be used for vector instrinsics
# No nas methods (C preprocessing assumed)
CC?=ncc
CXX?=nc++
NCC?=ncc
NCXX?=nc++
CLANG?=clang
CXXLANG?=clang++
# Flags can be added to from environment
# clang flags are adjust also by CFLAGS and CXXFLAGS, but
# can be totally overridden from environment if nec.
CFLAGS?=
CXXFLAGS?=
CLANG_FLAGS?=
CXXLANG_FLAGS?=
CFLAGS:=-O3 -g2 -fPIC $(CFLAGS)
CXXFLAGS:=-std=c++11 -O2 -g2 -fPIC $(CXXFLAGS)
CLANG_FLAGS:=$(CLANG_FLAGS) $(CFLAGS)
CXXLANG_FLAGS:=$(CXXLANG_FLAGS) $(CXXFLAGS)
# env flags append to some reasonable defaults
# clang vector instrinsics flags (reuse C[XX]LANG_FLAGS env variables to adjust)
CLANG_VI_FLAGS:=-show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics
# huh, what is correct? -show-spill-message-vec 
# maybe: -fno-unroll-loops
CLANG_FLAGS:=-target ve -mllvm $(CLANG_VI_FLAGS) $(CLANG_FLAGS)
CXXLANG_FLAGS:=-target ve -mllvm $(CXXLANG_FLAGS) $(CXXLANG_FLAGS)
#
# We will distinguish C files requiring different types of VE compile
# by suffix.
#
# Aurora C: %-ncc.c      via ncc (scalar code, extended asm, nas/link frontend)
#       and %-clang.c    via clang scalar code (want good optimizer)
#       and %-vi.c       via clang VECTOR INTRINSICS
# All with CFLAGS/CXXFLAGS
%-ve.o: %-ncc.c
	$(NCC) $(CFLAGS) -c $< -o $@
%-ve.o: %-clang.c
	$(CLANG) $(CLANG_FLAGS) -c $< -o $@
%-ve.o: %-vi.c
	which $(CLANG)
	$(CLANG) --version
	@# OK $(CLANG) -target ve -O3 -mllvm -show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -fPIC -o $@ -c $<
	@# OK $(CLANG) -target ve -mllvm $(CLANG_VI_FLAGS) -fPIC -o $@ -c $<
	$(CLANG) $(CLANG_FLAGS) $(CFLAGS) -o $@ -c $<
%-ve.o: %-ncc.cpp
	$(NCXX) $(CXXFLAGS) -c $< -o $@
%-ve.o: %-clang.cpp
	$(CXXLANG) $(CXXLANG_FLAGS) -c $< -o $@
%-ve.o: %-vi.cpp
	$(CXXLANG) $(CXXLANG_VFLAGS) -c $< -o $@
# Aurora assembler.S: cpp->.asm, ncc->.o, nobjcopy->.bin, .bin-->.dump
ifeq ($(VERBOSE),0)
%.bin: %.S
	@# .S files can run through *just* the C preprocessor.
	@$(NCC) -o $*.asm -E $< || { echo "trouble preprocessing $<"; false; }
	@sed -i '/^[[:space:]]*$$/d' $*.asm || { echo "trouble removing blank lines"; false; }
	@# .bin needs even more than -fPIC (no GOT, PLT, external symbols!)
	@$(NCC) -o $*.o -c $< || { echo "trouble compiling $<"; false; }
	@# we can get symbols + code + data disassembled
	@nobjdump -D $*.o > $*.dis || { echo "trouble disassembling $*.o"; false; }
	@# create a relocatable machine-language blob as follows
	@nobjcopy -O binary $*.o $*.bin || { echo "trouble making $*.bin blob"; false; }
	@# we can dump or disassemble the raw machine code
	@hexdump -C $*.bin > $*.dump
	@nobjdump -b binary -mve -D $*.bin >> $*.dump || { echo "trouble with objdump $*.bin"; false; }
else
%.bin: %.S
	@# .S files can run through *just* the C preprocessor.
	$(NCC) -o $*.asm -E $< && sed -i '/^[[:space:]]*$$/d' $*.asm
	@# .bin needs even more than -fPIC (no GOT, PLT, external symbols!)
	$(NCC) -o $*.o -c $<
	@# we can get symbols + code + data disassembled
	nobjdump -D $*.o > $*.dis
	@# create a relocatable machine-language blob as follows
	nobjcopy -O binary $*.o $*.bin
	@# we can dump or disassemble the raw machine code
	hexdump -C $*.bin > $*.dump
	@echo "===========================" >> $*.dump
	@echo "nobjdump -b binary -mve -D %*.bin" >> $*.dump
	nobjdump -b binary -mve -D $*.bin >> $*.dump
endif

clean:
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
realclean: clean
	rm -f *.bin
#
