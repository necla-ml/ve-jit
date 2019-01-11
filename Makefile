ifneq ($(CC),ncc)
#
# only a few things can compile for x86...
#
TARGETS:=test_strMconst asmfmt
VE_EXEC:=time
OBJDUMP:=objdump
OBJDUMP:=objdump
OBJCOPY:=objcopy
SHELL:=/bin/bash
CFLAGS:=-O2 -g2
CXXFLAGS:=$(CFLAGS) -std=c++11
LIBVELI_TARGETS:=libveli.a
else
#
# we may want to generate jit code on host OR ve
# Ex. 1: Simple tests, libraries of complex kernels precompiled for VE:
#         Host precompiles a jit kernel to a PERSISTENT .bin file,
#         which is mmaped and executed on the VE.
#         Host program invokes 'ncc' or 'nas' or ... main output-->file
# Ex. 2: Repetitive testing (avoiding zillions of .bin files):
#         test program runs on VE already and generates a string of
#         code and (somehow) converts the string to machine code that
#         it puts in a JIT page.  Only TEMPORARY files are used on the
#         host filesystem.
#         VE program invokes 'ncc' or 'nas' or ... main output-->memory
#         (but originally I kept around the .bin files)
#
# Originally, I ran JIT testing code entirely on VE, keeping the .bin files.
# A big speedup would be to run a simplified 'nas' entirely on the VE, avoiding
# all temporary files.  Unfortunately 'ncc' and 'nas' and tools run on host only,
# and some steps require real files on the host filesystem :(
#
TARGETS=asmkern0.asm libjit1.a libjit1-x86.a \
	asmkern.bin asmkern1.bin msk \
	syscall_hello \
	jitve0 jitve_hello test_strMconst jitve_math \
	jitpp_hello test_naspipe test_vejitpage_sh \
	jitpp_loadreg
LIBVELI_TARGETS:=libveli-x86.a
CC?=ncc-1.5.1
CXX?=nc++-1.5.1
CC:=ncc-1.5.1
CXX:=nc++-1.5.1
CC:=ncc-1.2.4
CXX:=nc++-1.2.4
CC:=ncc-1.5.2
CXX:=nc++-1.5.2

CC:=ncc
CXX:=nc++
CFLAGS:=-O2 -g2
CXXFLAGS:=$(CFLAGS) -std=c++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
endif
TARGETS+=$(LIBVELI_TARGETS)
all: $(TARGETS) libjit1.a

.PHONY: force
force: # force libjit1.a to be recompiled
	rm -f libjit1.a asmfmt.o jitpage.o jit_data.o
	$(MAKE) libjit1.a

vejit.tar.gz: asmfmt.hpp asmfmt_fwd.hpp jitpage.h libjit1.a
	rm -rf vejit
	mkdir vejit
	mkdir vejit/include
	mkdir vejit/bin
	cp -av asmfmt*.hpp jitpage.h vejit/include/
	cp -av libjit1.a vejit/bin/
	tar czf $@ vejit
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
# short demo of creating a binary blob from some hand-written assembler (no C ABI)
asmkern.bin: asmkern.S
	$(CC) -o asmkern.o -c asmkern.S
	@# if we have a relocatable (PIC) binary blob, we can create it as follows
	$(OBJCOPY) -O binary asmkern.o asmkern.bin
	@# we can dump or disassemble the raw machine code
	hexdump -C asmkern.bin
	$(OBJDUMP) -b binary -mve -D asmkern.bin
#
# a small demo of inline assembler, from Japan
#
asmkern0.asm: asmkern0.c
	$(CC) $(CFLAGS) -c $< -DOPT=1 -o asmkern0.o
	$(CC) $(CFLAGS) -Wa,adhln -S $< -o asmkern0.asm
	$(OBJDUMP) -d asmkern0.o > asmkern0.dis
#
# pstreams demo piping a std::string of VE asm code
# into a shell script (naspipe.sh) that returns
# a disassembled dump on stdout
# and errors on stderr.
#
# The script stdout and stderr are captured into C++ strings using pstream.h
# (More interesting is to use vejitpage.sh and capture the binary blob
#  output into a std::string that can be used to define a jit code page)
#
test_naspipe: test_naspipe.cpp naspipe.sh
	$(CXX) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
test_vejitpage_sh: test_vejitpage_sh.cpp vejitpage.sh
	$(CXX) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# low-level C-api, using jitve_util.h
#
jitve0: jitve0.c bin.mk
	$(CC) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
jitve_util.o: jitve_util.c jitve_util.h
	$(CC) -O2 -c $< -o $@
test_strMconst: test_strMconst.c jitve_util.o
	$(CC) $(CFLAGS) -O2 $^ -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
jitve_hello: jitve_hello.c jitve_util.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
jitve_math: jitve_math.c jitve_util.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# newer api uses jitpage.h (instead of jitve_util.h)
# and supports C++         (asmfmt_fwd.hpp)
# 
# Note that libjit1.a could be running x86 code or VE code
#
.PRECIOUS: asmfmt.o jit_data.o jitpage.o
libjit1.a: asmfmt.o jitpage.o jit_data.o
	$(AR) cq $@ $^
asmfmt.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
jit_data.o: jit_data.c jit_data.h
	$(CC) ${CFLAGS} -O2 -c $< -o $@
jitpage.o: jitpage.c jitpage.h
	$(CXX) $(CXXFLAGS) -O2 -c $< -o $@
libjit1-x86.a: asmfmt-x86.o jitpage-x86.o jit_data-x86.o
	ar cq $@ $^
asmfmt-x86.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	g++ ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
jit_data-x86.o: jit_data.c jit_data.h
	g++ ${CFLAGS} -O2 -c $< -o $@
jitpage-x86.o: jitpage.c jitpage.h
	g++ $(CXXFLAGS) -O2 -c $< -o $@

LIBVELI_SRC:=veliFoo.cpp wrpiFoo.cpp
libveli.a:     $(patsubst %.cpp,%.o,    $(LIBVELI_SRC))
libveli-x86.a: $(patsubst %.cpp,%-x86.o,$(LIBVELI_SRC))
	$(AR) cq $@ $^
#$(patsubst $(LIBVELI_SRC),%.cpp,%.o):    %.o:     %.cpp
#	$(CXX) ${CXXFLAGS} -O2 -c $< -o $@
#veliFoo.o wrpiFoo.o:    %.o:     %.cpp
$(patsubst $(LIBVELI_SRC),%.cpp,%-x86.o) %-x86.o: %.cpp
	g++    ${CXXFLAGS} -O2 -c $< -o $@

#asmfmt.cpp has a standalone demo program with -D_MAIN compiler	
asmfmt: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp jitpage.o
	g++ $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %,%.i,$@)
	g++ ${CXXFLAGS} -D_MAIN asmfmt.cpp jitpage.c -o $@
asmfmt-ve: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp jitpage.o
	$(CXX) $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %,%.i,$@)
	#$(CXX) $(CXXFLAGS) -O2 -D_MAIN $(filter-out %.hpp,$^) -o $@
	$(CXX) ${CXXFLAGS} -D_MAIN asmfmt.cpp jitpage.c -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# C++ version of jitve_hello.
# This one is more complicated. Besides printing hello world,
# it also returns a very lucky value (7).
#
jitpp_hello: jitpp_hello.cpp asmfmt.o jitpage.o
	$(CXX) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<)
	$(CXX) $(CFLAGS) -O2 $^ -o $@
	$(CXX) $(CFLAGS) -Wa,-adhln -c $^ >& $(patsubst %.cpp,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
ifeq (0,1)	
	# ccom: ./phase3_src/cg/schedule_vn_sx4.cpp:3372: void VN_TAG::confirm(int, int): Assertion `false' failed.
	# nc++: /opt/nec/ve/ncc/1.5.2/libexec/ccom is abnormally terminated by SIGABRT
msk: msk.cpp
	$(CXX) $(CFLAGS) -E msk.cpp > msk.i
	$(CXX) $(CFLAGS) -Wa,adhln -S msk.cpp -o msk.S
	$(CXX) $(CFLAGS) -g2 msk.cpp -o $@
	$(OBJDUMP) -d $@ >& msk.dis
	$(VE_EXEC) ./$@ 2>&1 | tee msk.log
else
msk: msk.cpp
	$(CXX) -O0 -E msk.cpp > msk.i
	$(CXX) -O0 -Wa,adhln -S msk.cpp -o msk.S
	$(CXX) -O0 -g2 msk.cpp -o $@
	$(OBJDUMP) -d $@ >& msk.dis
	$(VE_EXEC) ./$@ 2>&1 | tee msk.log
endif
jitpp_loadreg: jitpp_loadreg.cpp asmfmt.o jitpage.o jit_data.o
	-$(CXX) $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<) && echo YAY || echo OHOH for jitpp_loadreg.i
	$(CXX) $(CXXFLAGS) -O2 $^ -o $@
	$(CXX) $(CXXFLAGS) -Wa,-adhln -c $^ >& $(patsubst %.cpp,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
%.asm: %.c
	$(CC) $(CFLAGS) -g2 -Wa,-adhln -S $< >& $*.s
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $*.asm
	$(CC) $(CFLAGS) -c $< -o $*.o
	$(OBJDUMP) -d $*.o > $*.dis
syscall_hello: syscall_hello.c
	$(CC) $(CFLAGS) -S $< -o syscall_hello.s
	$(MAKE) syscall_hello.asm
	$(CC) $(CFLAGS) $< -o $@ && $(VE_EXEC) $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
clean:
	rm -f *.o *.i *.ii *.out
	rm -f msk*.i msk*.S msk*.dis msk*.out
	rm -f syscall_hello.o syscall_hello.asm syscall_hello.dis
	rm -f asmkern0.asm asmkern0.dis
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
	for f in tmp_*.S; do b=`basename $$f .S`; rm -f $$b.asm $$b.dis $$b.dump; done
	rm -f jitve_math.asm jitve_hello.asm jitve_hello.s jitve_hello.dis jitve*.dis \
		jitpp_hello.asm
realclean: clean
	rm -f $(TARGETS)
	rm -f msk msk0 msk1 msk2 msk3 msk4 syscall_hello smir jitve0
	rm -f bld.log asmfmt.log jitpp_hello.log mk*.log bld*.log
	rm -f tmp_*.S *.bin
	rm -rf CMakeCache.txt CMakeFiles asmfmt asmfmt.txt jitve_hello.s
#
