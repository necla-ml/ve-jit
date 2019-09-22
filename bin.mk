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
# Usage:
#    Simple : supply OBJECTS:=<list of object file names> in Makefile
#    Robust : ALSO supply OBJECTS_FILE:=FILE1,... where FILE1 contains the list object file names
#    (OBJECTS is used for dependencies within 'make', whereas its matching
#     OBJECTS_FILE is used to circumvent command line / argv limits of shell commands)
#
# NEW: autocreate an OBJECTS_FILE if it is empty...
#
# Simple usage can break if you have 20 or more long source files, esp. since
# you might produce several objects files per source file by varying compile flags.
#

# Set this to 0 in your environment to make this build quieter
# It controls things different from VERBOSE (make command-echo)
BIN_MK_VERBOSE?=1

# hack: if any OBJECTS are -x86.o, use 'ar' instead of 'nar'
$(info bin.mk OBJECTS ${OBJECTS})
ifeq ($(filter %-x86.o,$(OBJECTS)),)
$(info bin.mk VE library ${LIBNAME})
TARGET:=ve
AR:=nar
NM:=nnm
else
$(info bin.mk x86 library ${LIBNAME})
TARGET:=x86
AR:=ar
NM:=nm
endif

ifeq ($(ARCHIVE),)
ARCHIVE:=$(patsubst %.so,%.a,$(LIBNAME))
endif
# This can be used to jit object files in a "repository", to avoid
# recompiles (untested)
mkfile_path := $(realpath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
MEGA_ARCHIVE?=$(abspath $(mkfile_dir)/../libmegajit.a)
MEGA_ARCHIVE_SO?=$(abspath $(mkfile_dir)/../libmegajit.a)

# Example of how to compile a [local copy of] a jit '-vi.c' file
#        [copied from ../vednn-ek/test/tmp_cjitConv01/cjitConvFwd_parmstr-vi.c]
#   CFLAGS=-I../vednn-ek/test/vednn-ftrace1/include make VERBOSE=1 -f bin.mk cjitConvFwd_parmstr-ve.o
#
#   (include path is for vednnx.h, change to suit)
#
#LDFLAGS?=
#  Warning: this produces a LOT of output...
#  Also, not really sure if --copy-dt-needed-entries does anything with nld
LIBFLAGS?=
ifeq (${TARGET},ve)
ifeq (${BIN_MK_VERBOSE},0)
LIBFLAGS+=-Wl,--copy-dt-needed-entries
else
LIBFLAGS+=-Wl,--verbose -Wl,--trace -Wl,--copy-dt-needed-entries
endif
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

# nar accepts @FILE, ncc does not.  So we generate .a of -fPIC objects,
# and then convert to .so   Makefile $(file ...) trick does not work
#
# Using 'echo' also fails to pass everything into the file,
# and using make's file/foreach commands also did not work out.
#
# NEW: if OBJECTS_FILE is not given, auto-create it (can be slow if thousands of OBJECTS)
#
ifeq (${OBJECTS_FILE},)
OBJECTS_FILE:=$(patsubst %.a,%.OBJECTS,$(ARCHIVE))
# cannot use $$(OBJECTS) anywhere in a command line (it can exceed command line)
# 'foreach' + 'shell' seems safe. make's 'file' command did not work for me
$(OBJECTS_FILE): $(mkfile_path)
	rm -f $@
	$(foreach L,$(MANY1000),$(shell echo '$L' >> $@))
	@echo 'wordcount $@ : '`wc $@`
endif

# if OBJECTS_FILE is a real(?) file, also use @FILE for object file list
ifneq (${OBJECTS_FILE},)
#REAL_OBJECTS_FILE:=$(foreach F,${OBJECTS_FILE},$(realpath $F))
REAL_OBJECTS_FILE:=$(OBJECTS_FILE)
AT_OBJECTS_FILE:=$(patsubst %,@%,$(REAL_OBJECTS_FILE))
.PHONY: ${AT_OBECTS_FILE} # never try to create @FILEs
EXTRA_ARCHIVE_OBJECTS:=
else
# if OBJECTS_FILE empty, ARCHIVE_OBJECTS same as OBJECTS
AT_OBJECTS_FILE:=
REAL_OBJECTS_FILE:=
EXTRA_ARCHIVE_OBJECTS:=$(OBJECTS)
endif
.PRECIOUS: $(ARCHIVE)
# Remember, any OBJECTS_FILE should **duplicate** the 'make' OBJECTS dependencies
$(ARCHIVE): $(REAL_OBJECTS_FILE) $(OBJECTS)
	$(AR) rcs $@ $(AT_OBJECTS_FILE) $(EXTRA_ARCHIVE_OBJECTS) \
		&& echo 'created $@' || echo 'Trouble creating $@ (continuing)'
	$(NM) $@ | wc
ifneq (${BIN_MK_VERBOSE},0)
	-ls -l $@; echo 'nnm wordcount: ' `nnm $@ | wc`
endif
ifneq ($(MEGA_ARCHIVE),)
	#	$(MAKE) $(MEGA_ARCHIVE)
	$(AR) rcs $(MEGA_ARCHIVE) $(AT_OBJECTS_FILE) $(ARCHIVE_OBJECTS) \
		&& echo ' updated $(MEGA_ARCHIVE)' || echo 'Trouble updating $(MEGA_ARCHIVE) (continuing)'
ifneq (${BIN_MK_VERBOSE},0)
	-ls -l lib* $(MEGA_ARCHIVE); echo 'nnm $(MEGA_ARCHIVE) wordcount: ' `nnm $(MEGA_ARCHIVE) | wc`
endif
endif

$(MEGA_ARCHIVE)_first_version: # this created a nested archive, not good
	# MEGA_ARCHIVE is "$(MEGA_ARCHIVE)"
	-@if [ -f "$(MEGA_ARCHIVE)" ]; then \
		echo ' updating MEGA_ARCHIVE $(MEGA_ARCHIVE)'; \
		{ echo "open $(MEGA_ARCHIVE)"; echo "replace $(ARCHIVE)"; echo "save"; echo "end"; } \
		| $(AR) -M; \
	else \
		echo ' creating MEGA_ARCHIVE $(MEGA_ARCHIVE)'; \
		cp -av $@ $(MEGA_ARCHIVE); \
	fi
ifneq (${BIN_MK_VERBOSE},0)
	-ls -l lib* $(MEGA_ARCHIVE); echo 'nnm $(MEGA_ARCHIVE) wordcount: ' `nnm $(MEGA_ARCHIVE) | wc`
endif

# mri scripting via TINY also did not remove duplicate objects.
$(MEGA_ARCHIVE)_tiny: $(ARCHIVE)
	 MEGA_ARCHIVE is "$(MEGA_ARCHIVE)"
	-if [ -f "$(MEGA_ARCHIVE)" ]; then \
		echo ' creating thin archive $(patsubst %.a,%-megaT.a,$(ARCHIVE))'; \
		ar -rcT $(patsubst %.a,%-megaT.a,$(ARCHIVE)) $(MEGA_ARCHIVE) $(ARCHIVE) \
		&& echo 'creating new MEGA_ARCHIVE $(patsubst %.a,%-mega.a,$(ARCHIVE))' \
		&& { echo 'create $(patsubst %.a,%-mega.a,$(ARCHIVE))'; \
	             echo 'addlib $(patsubst %.a,%-megaT.a,$(ARCHIVE))'; \
		     echo 'save'; \
		     echo 'end'; } \
		   | $(AR) -M; \
	else \
		echo ' creating MEGA_ARCHIVE $(MEGA_ARCHIVE)'; \
		cp -av $@ $(MEGA_ARCHIVE); \
	fi
ifneq (${BIN_MK_VERBOSE},0)
	-ls -l lib*; echo 'nnm wordcount: ' `nnm $@ | wc`
endif

$(LIBNAME): $(OBJECTS) ${OBJECTS_FILE}
ifneq (${BIN_MK_VERBOSE},0)
	echo "-------- Linking --------"
	echo "LDFLAGS = $${LDFLAGS}"
endif
ifeq (${TARGET},ve)
	@#ncc -o $@ $(LDFLAGS) $(LIBFLAGS) $(filter %-ve.o,$^)
	@# did not work echo '$(OBJECTS)' | gawk 'BEGIN{RS=" "}//' > $@.objects
	@#  did not create file $(file >$@.objects,$(filter %-ve.o,$^))
	@#ncc -o $@ $(LDFLAGS) $(LIBFLAGS) @$@.objects
	@#ls -l OBJECTS-ve.list
	@#
	@# ncc does not support @FILE for long command lines
	@#ncc -o $@ $(LDFLAGS) $(LIBFLAGS) @OBJECTS-ve.list
	@#
	ncc -shared -o $@ $(LDFLAGS) $(LIBFLAGS) -Wl,--whole-archive $(ARCHIVE) -Wl,--no-whole-archive
else # much simple for x86, gcc supports @file
	gcc -shared -o $@ $(LDFLAGS) $(LIBFLAGS) ${AT_OBJECTS_FILE}
endif
ifneq (${BIN_MK_VERBOSE},0)
	echo "-------- Linking DONE --------"
	# This would assume VE library target !!! -nreadelf -hds $@
	echo "-------- Library $(LIBNAME) created in `pwd`"
	-ls -l $@; echo 'nnm wordcount: ' `$(NM) $@ | wc`
endif
#if [ -s "$(MEGA_ARCHIVE_SO)" ]; then \
#  ncc -shared -o $(MEGA_ARCHIVE_SO) $(LIBFLAGS)  -Wl,--whole-archive $@  $(MEGA_ARCHIVE_SO); \
#fi

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
GCC?=gcc
GCXX?=g++
# Flags can be added to from environment
# clang flags are adjust also by CFLAGS and CXXFLAGS, but
# can be totally overridden from environment if nec.
CFLAGS?=
CXXFLAGS?=
CLANG_FLAGS?=
CXXLANG_FLAGS?=
C86FLAGS?=
CXX86FLAGS?=

$(bin_mk_info Begin with CFLAGS        = $(CFLAGS))
$(bin_mk_info Begin with CXXFLAGS      = $(CXXFLAGS))
$(bin_mk_info Begin with CLANG_FLAGS   = $(CLANG_FLAGS))
$(bin_mk_info Begin with CXXLANG_FLAGS = $(CXXLANG_FLAGS))
CFLAGS:=-O2 -fPIC $(CFLAGS)
CXXFLAGS:=-std=c++11 -O2 -fPIC $(CXXFLAGS)

C86FLAGS:=-O2 $(C86FLAGS)
CXX86FLAGS:=-std=c++11 -O2 $(CXX86FLAGS)

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
# x86 C: %-x86.c         via gcc, also LIBNAME -x86.so ?
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
%-x86.o: %-x86.c
	$(GCC) $(C86FLAGS)       -c $< -o $*-x86.s
	$(GCC) $(C86FLAGS) -fPIC -c $< -o $@
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
	@#ls -l
endif
	@#$(MAKE) $*-vi_bin.asm # Why does make not find this rule?
	#$(CLANG) $(filter-out -fPIC,$(CLANG_FLAGS)) -ggdb -S $< -o $*-vi_bin.s
	#$(NCC) $(CFLAGS) -o $*-vi_bin.o -c $*-vi_bin.s
	#nobjdump -DS -j .text $*-vi_bin.o >& $*-vi_bin.asm
	#rm -f $*-vi_bin.s $*-vi_bin.o
	# the official compile "as is"
	$(CLANG) $(CLANG_FLAGS) -fPIC -S $< -o $*-vi.s
	$(NCC) $(CFLAGS) -o $@ -c $*-vi.s
# create a second object file, with unrolling and change func name
# Ex. export CLANG_UNROLL='-loop-unroll-count=2 -unroll-allow-partial -fsave-optimization-record'
%_unroll-ve.o: %-vi.c
	$(CLANG) $(CLANG_FLAGS) -funroll-loops '-Rpass=loop.*' $(CLANG_UNROLL) -S $< -o $*_unroll-vi.s
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
# Begin by cancelling the default rule -- we REQUIRE a special suffix
%.o: %.cpp
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
%-x86.o: %-x86.cpp
	# this one might not be supported?
	$(GCXX) $(CXX86FLAGS)       -S $< -o $*-x86_cpp.s
	$(GCXX) $(CXX86FLAGS) -fPIC -c $< -o $@
%-ve: %-ve.o
	$(NCXX) $(CXXFLAGS) $< -o $@
%-x86: %-x86.o
	$(GCXX) $(CXX86FLAGS) $< -o $@
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
