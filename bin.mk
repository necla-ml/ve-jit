#
# bin.mk (template)
#
# Example of how to compile a [local copy of] a jit '-vi.c' file
#        [copied from ../vednn-ek/test/tmp_cjitConv01/cjitConvFwd_parmstr-vi.c]
#   CFLAGS=-I../vednn-ek/test/vednn-ftrace1/include make VERBOSE=1 -f bin.mk cjitConvFwd_parmstr-ve.o
#
#LDFLAGS?=
$(LIBNAME): $(OBJECTS)
	echo "-------- Linking --------"
	echo "LDFLAGS = $${LDFLAGS}"
	ncc -o $@ $(LDFLAGS) -Wl,--verbose -Wl,--trace -Wl,--copy-dt-needed-entries $(filter %-ve.o,$(OBJECTS))
	echo "-------- Linking DONE --------"
	# This would assume VE library target !!! -nreadelf -hds $@
	echo "-------- Library $(LIBNAME) created in `pwd`"
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
$(info Begin with CFLAGS        = $(CFLAGS))
$(info Begin with CXXFLAGS      = $(CXXFLAGS))
$(info Begin with CLANG_FLAGS   = $(CLANG_FLAGS))
$(info Begin with CXXLANG_FLAGS = $(CXXLANG_FLAGS))
CFLAGS:=-O2 -fPIC $(CFLAGS)
CXXFLAGS:=-std=c++11 -O2 -fPIC $(CXXFLAGS)
CLANG_FLAGS:=$(CLANG_FLAGS) $(CFLAGS)
CXXLANG_FLAGS:=$(CXXLANG_FLAGS) $(CXXFLAGS)
# env flags append to some reasonable defaults
# clang vector instrinsics flags (reuse C[XX]LANG_FLAGS env variables to adjust)
CLANG_VI_FLAGS:=-show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics
# huh, what is correct? -show-spill-message-vec 
# maybe: -fno-unroll-loops
CLANG_FLAGS:=-target ve -mllvm $(CLANG_VI_FLAGS) $(CLANG_FLAGS)
CXXLANG_FLAGS:=-target ve -mllvm $(CXXLANG_FLAGS) $(CXXLANG_FLAGS)
$(info Ending with CFLAGS        = $(CFLAGS))
$(info Ending with CXXFLAGS      = $(CXXFLAGS))
$(info Ending with CLANG_FLAGS   = $(CLANG_FLAGS))
CXXLANG_FLAGS:=$(filter-out -std=c++11,$(CXXLANG_FLAGS))
$(info Ending with CXXLANG_FLAGS = $(CXXLANG_FLAGS))
#
# We will distinguish C files requiring different types of VE compile
# by suffix.
#
# Aurora C: %-ncc.c      via ncc (scalar code, extended asm, nas/link frontend)
#       and %-clang.c    via clang scalar code (want good optimizer)
#       and %-vi.c       via clang VECTOR INTRINSICS
# All with CFLAGS/CXXFLAGS
# Begin by cancelling the default rule -- we REQUIRE a special suffix
%.o: %.c
%-ve.o: %-ncc.c
	$(NCC) $(CFLAGS)       -S $< -o $*-ve.s
	$(NCC) $(CFLAGS) -fPIC -c $< -o $@
%-ve.o: %-clang.c
	$(CLANG) $(CLANG_FLAGS)       -c $< -o $*-clang.s
	$(CLANG) $(CLANG_FLAGS) -fPIC -c $< -o $@
# OK $(CLANG) -target ve -O3 -mllvm -show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics -fPIC -o $@ -c $<
# OK $(CLANG) -target ve -mllvm $(CLANG_VI_FLAGS) -fPIC -o $@ -c $<
%-ve.o: %-vi.c
	which $(CLANG)
	$(CLANG) --version
	$(CLANG) $(filter-out -fPIC,$(CLANG_FLAGS)) -S $< -o $*-vi_bin.s
	$(CLANG) $(CLANG_FLAGS) -S $< -o $*-vi.s
	$(NCC) $(CFLAGS) -fPIC -o $@ -c $*-vi.s
%-ve.o: %-ncc.cpp
	$(NCXX) $(CXXFLAGS) -fPIC -S $< -o $*-ncc_cpp.s
	$(NCXX) $(CXXFLAGS) -fPIC -c $< -o $@
%-ve.o: %-clang.cpp
	$(CXXLANG) $(CXXLANG_FLAGS)       -S $< -o $*-clang_cpp.s
	$(CXXLANG) $(CXXLANG_FLAGS) -fPIC -c $< -o $@
%-ve.o: %-vi.cpp
	# this one might not be supported?
	$(CXXLANG) $(CXXLANG_VFLAGS)       -S $< -o $*-vi_cpp.s
	$(CXXLANG) $(CXXLANG_VFLAGS) -fPIC -c $< -o $@
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
