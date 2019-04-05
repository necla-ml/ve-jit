#
# bin.mk (template)
#
# This file is intended for:
# - *-ve.o object files [or LIBNAME .so / ARCHIVE .a, for C JIT], or
# - VE .bin binary blobs [for assembly JIT]
# It is usually set up with a prefix 'all:' target, and changes here
# should go hand in hand with dllbuild.hpp and dllbuild.cpp
#
# Verbosity:
#    Hmm. safest for large jobs seems VERBOSE=0 BIN_MK_VERBOSE=0 LC_ALL=C (cmd)
#    which seems to play nicely with pstreams.  Often I think the build gets
#    stuck.
# 

# Set this to 0 in your environment to make this build quieter
# It controls things different from VERBOSE (make command-echo)
BIN_MK_VERBOSE?=1

# Example of how to compile a [local copy of] a jit '-vi.c' file
#        [copied from ../vednn-ek/test/tmp_cjitConv01/cjitConvFwd_parmstr-vi.c]
#   CFLAGS=-I../vednn-ek/test/vednn-ftrace1/include make VERBOSE=1 -f bin.mk cjitConvFwd_parmstr-ve.o
#
#LDFLAGS?=
#  Warning: this produces a LOT of output...
#  Also, not really sure if --copy-dt-needed-entries does anything with nld
ifeq (${BIN_MK_VERBOSE},0)
LIBFLAGS?=-Wl,--copy-dt-needed-entries
else
LIBFLAGS?=-Wl,--verbose -Wl,--trace -Wl,--copy-dt-needed-entries
endif

ifeq (${BIN_MK_VERBOSE},0)
define bin_mk_info
endef
else
$(info BIN_MK_VERBOSE=$(BIN_MK_VERBOSE))
define bin_mk_info
$(info $(1))
endef
endif

$(ARCHIVE): $(OBJECTS)
	nar rcs $@ $^
	# MEGA_ARCHIVE is "$(MEGA_ARCHIVE)"
	@if [ -f "$(MEGA_ARCHIVE)" ]; then \
		rm -f mega.mri; \
		{ echo "open $(MEGA_ARCHIVE)"; echo "replace $(ARCHIVE)"; echo "save"; echo "end"; } > mega.mri; \
		nar -M mega.mri; \
		fi

$(LIBNAME): $(OBJECTS)
ifneq (${BIN_MK_VERBOSE},0)
	echo "-------- Linking --------"
	echo "LDFLAGS = $${LDFLAGS}"
endif
	ncc -o $@ $(LDFLAGS) $(LIBFLAGS) $(filter %-ve.o,$(OBJECTS))
ifneq (${BIN_MK_VERBOSE},0)
	echo "-------- Linking DONE --------"
	# This would assume VE library target !!! -nreadelf -hds $@
	echo "-------- Library $(LIBNAME) created in `pwd`"
endif

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
$(bin_mk_info Begin with CFLAGS        = $(CFLAGS))
$(bin_mk_info Begin with CXXFLAGS      = $(CXXFLAGS))
$(bin_mk_info Begin with CLANG_FLAGS   = $(CLANG_FLAGS))
$(bin_mk_info Begin with CXXLANG_FLAGS = $(CXXLANG_FLAGS))
CFLAGS:=-O2 -fPIC $(CFLAGS)
CXXFLAGS:=-std=c++11 -O2 -fPIC $(CXXFLAGS)

CLANG_FLAGS:=$(CLANG_FLAGS) $(CFLAGS)
CXXLANG_FLAGS:=$(CXXLANG_FLAGS) $(CXXFLAGS)
# remove some flags that need special handling for clang
CLANG_FLAGS:=$(filter-out -O2,$(CLANG_FLAGS))
CXXLANG_FLAGS:=$(filter-out -O2,$(CLANG_FLAGS))
# env flags append to some reasonable defaults
# clang vector instrinsics flags (reuse C[XX]LANG_FLAGS env variables to adjust)
# Here is a recent libvednn impl compilation:
# cd /usr/uhome/aurora/4gi/nlabhpg/work/kruus/vednn-ek/build/src/intrinsic/Convolution/Forward
# && /usr/uhome/aurora/4gi/nlabhpg/.local/bin/clang -DVEDNN_USE_OPENMP
#  -I/usr/uhome/aurora/4gi/nlabhpg/work/kruus/vednn-ek/src/intrinsic/..
#    -target ve -O3 -mllvm -show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics
#    -o CMakeFiles/vednn_intrinsic_convolution_forward.dir/direct_default.c.o
#    -c /usr/uhome/aurora/4gi/nlabhpg/work/kruus/vednn-ek/src/intrinsic/Convolution/Forward/direct_default.c
CLANG_VI_FLAGS?=-show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics
# huh, what is correct? -show-spill-message-vec 
# maybe: -fno-unroll-loops
CLANG_FLAGS:=-target linux-ve -O3 -mllvm $(CLANG_VI_FLAGS) $(CLANG_FLAGS)
CXXLANG_FLAGS:=-target linux-ve -O3 -mllvm $(CLANG_VI_FLAGS) $(CXXLANG_FLAGS)

$(bin_mk_info Ending with CFLAGS        = $(CFLAGS))
$(bin_mk_info Ending with CXXFLAGS      = $(CXXFLAGS))
$(bin_mk_info Ending with CLANG_FLAGS   = $(CLANG_FLAGS))
CXXLANG_FLAGS:=$(filter-out -std=c++11,$(CXXLANG_FLAGS))
$(bin_mk_info Ending with CXXLANG_FLAGS = $(CXXLANG_FLAGS))
#
# We will distinguish C files requiring different types of VE compile
# by suffix.
#
# Aurora C: %-ncc.c      via ncc (scalar code, extended asm, nas/link frontend)
#       and %-clang.c    via clang scalar code (want good optimizer)
#       and %-vi.c       via clang VECTOR INTRINSICS
#
# All with CFLAGS/CXXFLAGS
# Assembler outputs are for show, and can omit -fPIC for clarity.
# 'C' sources are compile with -fPIC so they can produce .so library.
# NOTE: you may combine the .o's into a .a "mega-library", which can
#       be converted to a mega- .so
#
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

# annotated assembly (without -fPIC, to simplify)
%-vi_bin.asm: %-vi.c
	$(CLANG) $(filter-out -fPIC,$(CLANG_FLAGS)) -ggdb -S $< -o $*-vi_bin.s
	$(NCC) $(CFLAGS) -o $*-vi_bin.o -c $*-vi_bin.s
	nobjdump -DS -j .text $*-vi_bin.o >& $@
%-ve.o: %-vi.c
ifneq (${BIN_MK_VERBOSE},0)
	which $(CLANG)
	$(CLANG) --version
	ls -l
endif
	@#$(MAKE) $*-vi_bin.asm # Why does make not find this rule?
	$(CLANG) $(filter-out -fPIC,$(CLANG_FLAGS)) -ggdb -S $< -o $*-vi_bin.s
	$(NCC) $(CFLAGS) -o $*-vi_bin.o -c $*-vi_bin.s
	nobjdump -DS -j .text $*-vi_bin.o >& $*-vi_bin.asm
	rm -f $*-vi_bin.s $*-vi_bin.o
	# the official compile "as is"
	$(CLANG) $(CLANG_FLAGS) -fPIC -S $< -o $*-vi.s
	$(NCC) $(CFLAGS) -o $@ -c $*-vi.s
# create a second object file, with unrolling and change func name
%_unroll-ve.o: %-vi.c
	$(CLANG) $(CLANG_FLAGS) -funroll-loops -S $< -o $*_unroll-vi.s
	@#$(NCC) $(CFLAGS) -o $@ -c $*_unroll-vi.s
	$(NCC) $(CFLAGS) -fPIC -o $*_unroll-ve.o.tmp -c $*_unroll-vi.s
	@#nobjcopy --redefine-sym $*=$*_unroll $*_unroll-ve.o.tmp $@
	@# the file name does not always match the function name!
ifeq (1,0)
	@# but --prefix-symbols might be too much:  "unroll_GLOBAL_OFFSET_TABLE"
	nobjcopy --prefix-symbols unroll_ $*_unroll-ve.o.tmp $@
else
	# OK, expect the client to create a file of symbol renames...
	if [ -f '$@.rename' ]; then  \
		echo ".rename file exists"; \
		nobjcopy --redefine-syms '$@.rename' $*_unroll-ve.o.tmp $@; \
	else \
		echo "No .rename file, try shorcut rename $* (from file name)"; \
		nobjcopy --redefine-sym $*=$*_unroll $*_unroll-ve.o.tmp $@; \
	fi
endif
ifneq (${BIN_MK_VERBOSE},0)
	@echo 'Here are the alternate .o file symbols in $@'
	nnm $@
endif
	rm -f $*_unroll-ve.o.tmp
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
