VEJIT_ROOT:=.
SHELL:=/bin/bash
# normal build target = all
# distribution target = vejit.tar.gz
# major clean and rebuild tests:
#   dlprt
#   dllbuild

ifneq ($(CC:ncc%=ncc),ncc)

.PRECIOUS: jitpp_loadreg
#
# only a few things can compile for x86...
#
TARGETS:=test_strMconst asmfmt-x86 jitpp_loadreg veliFoo.o
TARGETS+=veliFoo.o veli_loadreg-x86 dlprt-x86
VE_EXEC:=time
OBJDUMP:=objdump
OBJDUMP:=objdump
OBJCOPY:=objcopy
SHELL:=/bin/bash
CFLAGS:=-O2 -g2 -pthread
CFLAGS+=-Wall -Werror
CXXFLAGS:=$(CFLAGS) -std=c++11
LIBVELI_TARGETS:=libveli-x86.a
LIBJIT1_TARGETS:=libjit1-x86.a libjit1-x86.so
LDFLAGS+=-ldl
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
TARGETS=asmkern0.asm libjit1.a libjit1-x86.a asmfmt-ve\
	asmkern.bin asmkern1.bin msk \
	syscall_hello \
	jitve0 jitve_hello test_strMconst jitve_math \
	jitpp_hello test_naspipe-x86 test_vejitpage_sh-x86 \
	jitpp_loadreg \
	asmfmt-x86 veli_loadreg-x86 veli_loadreg \
	dlprt-x86 dlprt-ve
# slow!
#TARGETS+=test_naspipe test_vejitpage_sh
#CC?=ncc-1.5.1
#CXX?=nc++-1.5.1
#CC:=ncc-1.5.1
#CXX:=nc++-1.5.1
#CC:=ncc-1.2.4
#CXX:=nc++-1.2.4
#CC:=ncc-1.5.2
#CXX:=nc++-1.5.2
#CC:=ncc-1.6.0
#CXX:=nc++-1.6.0

#CC:=ncc
#CXX:=nc++
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
CXXFLAGS:=$(CFLAGS) -std=c++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
LIBVELI_TARGETS:=libveli.a libveli-x86.a
LIBJIT1_TARGETS:=libjit1.a libjit1-x86.a libjit1.so libjit1-x86.so
# for ncc-2.x, we should add -ldl (as for x86)
LDFLAGS+=-ldl
endif

TARGETS+=$(LIBJIT1_TARGETS) $(LIBVELI_TARGETS)

FTRACE=NO
OPENMP=NO
LIBSUFFIX:=
ifeq (${OPENMP},YES)
	LIBSUFFIX:=${LIBSUFFIX}_omp
endif
ifeq (${FTRACE},YES)
	LIBSUFFIX:=${LIBSUFFIX}_ft
endif
# libjit1 basename: pertains to vejit/lib/ 'make all-vejit-libs'
LIBJIT:=jit1${LIBSUFFIX}
LIBJITX86=${LIBJIT}-x86

all: $(TARGETS) liblist
SHELL:=LC_ALL=C /bin/bash
.PHONY: liblist force
liblist:
	@ls -l lib*.a
force: # force libs to be recompiled
	rm -f libjit1*.a asmfmt*.o jitpage*.o intutil*.o
	rm -f libveli*.a prgiFoo.o wrpiFoo.o
	rm -f $(patsubst %.cpp,%*.o,$(LIBVELI_SRC)) $(LIBVELI_TARGETS)
	$(MAKE) $(LIBJIT1_TARGETS)
	$(MAKE) $(LIBVELI_TARGETS)

VEJIT_SHARE:=cblock.cpp dltest1.cpp veli_loadreg.cpp dllbuild.cpp
VEJIT_LIBS:=libjit1-x86.a libveli-x86.a libjit1-x86.so bin.mk-x86.lo
ifeq ($(CC:ncc%=ncc),ncc)
VEJIT_LIBS+=libjit1.a libveli.a libjit1.so bin.mk-ve.lo
endif
.PHONY: all-vejit-libs
all-vejit-libs:
	./mklibs.sh >& mklibs.log	# writes libs into vejit/lib/
vejit.tar.gz: jitpage.h intutil.h \
		intutil.hpp stringutil.hpp throw.hpp \
		asmfmt_fwd.hpp asmfmt.hpp codegenasm.hpp velogic.hpp \
		cblock.hpp dllbuild.hpp \
		asmfmt.cpp cblock.cpp dllbuild.cpp jitpage.c intutil.c \
		jitpage.hpp jitpipe_fwd.hpp jitpipe.hpp cblock.hpp pstreams-1.0.1 bin_mk.c \
		$(VEJIT_LIBS) $(VEJIT_SHARE)
	rm -rf vejit
	mkdir vejit
	mkdir vejit/include
	mkdir vejit/lib
	mkdir vejit/share
	mkdir vejit/share/vejit
	mkdir vejit/share/vejit/src
	$(MAKE) all-vejit-libs # libjit1[_omp][_ft][-x86].{a|so}
	cp -av $(filter %.hpp,$^) $(filter %.h,$^) vejit/include/
	cp -av pstreams-1.0.1 vejit/include/
	#cp -av $(filter %.a,$^) $(filter %.so,$^) bin.mk-ve.lo vejit/lib/
	cp -av $(filter libveli%,$^) $(filter bin.mk%,$^) vejit/lib/
	cp -av $(filter %.cpp,$^) $(filter %.c,$^) vejit/share/vejit/src/
	cp -av ${VEJIT_SHARE} vejit/share/vejit/
	cp -av Makefile.share vejit/share/vejit/Makefile
	tar czf $@ vejit
	tar tvzf $@ vejit
ifneq ($(CC:ncc%=ncc),ncc)
	mv $@ vejit-x86.tar.gz
endif
#
# see vl-run.sh for running veli_loadreg tests
# many tests only need veli_loadreg-x86
# only veli_loadreg -R needs VE (because it runs several
# thousand optimized VE assembler test cases).
WORKING?=6
veli_loadreg: veli_loadreg.cpp libveli.a libjit1.a
	$(CXX) $(CXXFLAGS) -DWORKING=${WORKING} -g -O2 -o $@ $^ $(LDFLAGS)
	@echo "veli_loadreg runs VE Logic for Instructions tests on VE"
	@echo "(veli_loadreg-x86 will do same VE logic tests on x86)"
veli_loadreg-x86: veli_loadreg.cpp libveli-x86.a libjit1-x86.a
	g++ $(CXXFLAGS) -Wall -DWORKING=${WORKING} -g -O2 -o $@ $^ -ldl
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
# This can run on VE or [much faster] on host
test_naspipe: test_naspipe.cpp naspipe.sh
	$(CXX) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
test_naspipe-x86: test_naspipe.cpp naspipe.sh
	g++ -O2 $< -o $@
	./$@ 2>&1 | tee $@.log
# this also can run on host, in seconds rather than minutes,
# because we do not execute the jit page
# [we create the jit page string, and then hexdump and disassemble it]
test_vejitpage_sh: test_vejitpage_sh.cpp vejitpage.sh
	$(CXX) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
test_vejitpage_sh-x86: test_vejitpage_sh.cpp vejitpage.sh
	g++ -std=c++11 -O2 $< -o $@
	./$@ 2>&1 | tee $@.log
#
# low-level C-api, using jitve_util.h
#
jitve0: jitve0.c bin.mk
	$(CC) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
jitve_util.o: jitve_util.c jitve_util.h
	$(CC) -O2 -c $< -o $@
test_strMconst: test_strMconst.c jitpage.o
	$(CC) $(CFLAGS) -O2 $^ -o $@ -ldl
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
jitve_hello: jitve_hello.c jitpage.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@ -ldl
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
.PRECIOUS: jitve_math
jitve_math: jitve_math.c jitpage.o
	$(CC) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.c,%.i,$<)
	$(CC) $(CFLAGS) -O2 $^ -o $@ -ldl
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $(patsubst %.c,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# newer api uses jitpage.h (instead of jitve_util.h)
# and supports C++         (asmfmt_fwd.hpp)
#
# Note that libjit1.a could be running x86 code or VE code
#
# NEW: CMakeLists.txt + mklibs.sh to build variants of libjit1
#
.PRECIOUS: asmfmt.o intutil.o jitpage.o
#%-omp.o: %.c: $(CC) ${CFLAGS} -O2 -c $< -o $@
#%-omp-ftrace1.o: %.c: $(CC) ${CFLAGS} -O2 -c $< -o $@
libjit1.a: asmfmt.o jitpage.o intutil.o \
		cblock-ve.o dllbuild-ve.o bin.mk-ve.lo
	rm -f $@
	$(AR) rcs $@ $^
	readelf -h $@
#libbin_mk.a: bin.mk-ve.o
#	rm -f $@; $(AR) rcs $@ $^
#// nc++ only has a .a version std:: C++ library?
#// dlrt-ve needs -lnc++ before dlopen would succeed !
# C things	
intutil.o: intutil.c intutil.h
	$(CC) ${CFLAGS} -O2 -c $< -o $@
jitpage.o: jitpage.c jitpage.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl
# C++ things...	
asmfmt.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
cblock-ve.o: cblock.cpp cblock.hpp
	$(CXX) -Wall -g2 -std=c++11 -c $< -o $@
dllbuild-ve.o: dllbuild.cpp
	$(CXX) -o $@ $(CXXFLAGS) -Wall -Werror -c $<
libjit1.so: jitpage.lo intutil.lo bin.mk-ve.lo \
	asmfmt.lo cblock-ve.lo dllbuild-ve.lo # C++ things
	$(CXX) -o $@ -shared -Wl,-trace -wL,-verbose $^ #-ldl #-lnc++
	readelf -h $@
	readelf -d $@
intutil.lo: intutil.c intutil.h
	$(CC) ${CFLAGS} -fPIC -O2 -c $< -o $@
jitpage.lo: jitpage.c jitpage.h
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@ -ldl
asmfmt.lo: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -fPIC -O2 -c asmfmt.cpp -o $@
cblock-ve.lo: cblock.cpp cblock.hpp
	$(CXX) -fPIC -Wall -g2 -std=c++11 -c $< -o $@
dllbuild-ve.lo: dllbuild.cpp
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -c $<

libvenobug.so: \
	jitpage.lo \
	intutil.lo \
	bin.mk-ve.lo
	$(CC) -o $@ -shared $^
.PHONY: hdrs0.cpp bug0.cpp hdrs1.cpp hdrs2.cpp empty.cpp
empty.cpp:
	rm -f $@
	touch empty.cpp
empty.lo: empty.cpp 
	$(CXX) ${CXXFLAGS} -fPIC -O2 -c $< -o $@
libveempty.so: empty.lo
	$(CXX) -o $@ -shared $^
hdrs0.cpp:
	{ \
		echo '#include "stdint.h"'; \
		} > $@
bug0.cpp:
	{ \
		echo '#include <iostream>'; \
		echo '#include <iomanip>'; \
		echo 'using namespace std;'; \
		echo 'int main(int,char**){ cout<<"Goodbye"<<endl; }'; \
		} > $@
hdrs1.cpp:
	{ \
		echo '#include "jitpage.h"'; \
		} > $@
hdrs2.cpp:
	{ \
		echo '#include <iostream>'; \
		} > $@
hdrs%.lo: hdrs%.cpp 
	$(CXX) ${CXXFLAGS} -fPIC -O2 -c $< -o $@
libvehdrs%.so: hdrs%.lo
	$(CXX) -o $@ -shared $^
	nnm -C $@
#bug%: bug0.cpp libvenobug.so libvehdrs%.so
bug%: bug0.cpp libvehdrs%.so
	# ok
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	-./$@ 7
	echo "Exit status $$?"
hdrs9.cpp:
	{ \
		echo '#include "jitpage.h"'; \
		echo '#include <iosfwd>'; \
		echo '#include <string>'; \
		echo '#include <stack>'; \
		echo '#include <sstream>'; \
		echo '#include <deque>'; \
		echo '#include <iostream>'; \
		} > $@
libveasmfmt.so: asmfmt.lo
	$(CXX) -o $@ -shared $^
asmfmt2.lo: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -fPIC -O2 -DASMFMTREMOVE=2 -E asmfmt.cpp -o asmfmt2.i
	$(CXX) ${CXXFLAGS} -fPIC -O2 -DASMFMTREMOVE=2 -c asmfmt.cpp -o $@
libveasmfmt2.so: asmfmt2.lo
	$(CXX) -o $@ -shared $^
libvebug.so: \
	jitpage.lo \
	intutil.lo \
	bin.mk-ve.lo \
	asmfmt.lo
	$(CXX) -o $@ -shared $^
dllok0: dllbug.cpp libvenobug.so
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=0 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	-./$@ 7
	echo "Exit status $$?"
dllok1: dllbug.cpp libvenobug.so
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=0 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^ asmfmt.lo
	-./$@ 7
	echo "Exit status $$?"
dllok2: dllbug.cpp libvenobug.so libveempty.so
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=0 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	-./$@ 7
	echo "Exit status $$?"
dllok3: dllbug.cpp libvenobug.so libvehdrs0.so
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=0 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	-./$@ 7
	echo "Exit status $$?"
dllok4: dllbug.cpp libvenobug.so libvehdrs1.so
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=0 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllok5: dllbug.cpp libvenobug.so libvehdrs2.so
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=0 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllvebug0: dllbug.cpp libvenobug.so libvehdrs9.so
	$(CXX) $(CXXFLAGS) -DCODEREMOVE=9 -fPIC -Wall -Werror -E $< -o dllbug9.i
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=9 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllvebug1: dllbug.cpp libvenobug.so libvehdrs2.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllvebug11: dllbug.cpp libvenobug.so libveasmfmt2.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllvebug12: dllbug.cpp libvenobug.so libveasmfmt.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllvebug13: dllbug.cpp libveasmfmt.so libvenobug.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"
dllvebug14: dllbug.cpp libvebug.so
	# incorrect execution
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7
	echo "Exit status $$?"

libjit1-x86.a: asmfmt-x86.o jitpage-x86.o intutil-x86.o \
		cblock-x86.o dllbuild-x86.o bin.mk-x86.lo
	rm -f $@
	ar rcs $@ $^
	readelf -h $@
	readelf -d $@
libjit1-x86.so: asmfmt-x86.lo jitpage-x86.lo intutil-x86.lo \
		cblock-x86.lo dllbuild-x86.lo bin.mk-x86.lo
	gcc -o $@ -shared $^ # -ldl
	readelf -h $@
	readelf -d $@
dllbuild-x86.lo: dllbuild.cpp dllbuild.hpp
	g++ -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -c $<
asmfmt-x86.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp stringutil.hpp jitpage.h
	g++ ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
asmfmt-x86.lo: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	g++ ${CXXFLAGS} -fPIC -O2 -c asmfmt.cpp -o $@
intutil-x86.o: intutil.c intutil.h
	g++ ${CFLAGS} -O2 -c $< -o $@
intutil-x86.lo: intutil.c intutil.h
	g++ ${CFLAGS} -fPIC -O2 -c $< -o $@
jitpage-x86.o: jitpage.c jitpage.h
	g++ $(CXXFLAGS) -O2 -c $< -o $@ -ldl
jitpage-x86.lo: jitpage.c jitpage.h
	g++ $(CXXFLAGS) -fPIC -O2 -c $< -o $@ -ldl
cblock-x86.o: cblock.cpp cblock.hpp
	g++ ${CXXFLAGS} -g2 -std=c++11 -c $< -o $@
cblock-x86.lo: cblock.cpp cblock.hpp
	g++ ${CXXFLAGS} -fPIC -c $< -o $@
dllbuild-x86.o: dllbuild.cpp dllbuild.hpp
	g++ -o $@ $(CXXFLAGS) -Wall -Werror -c $<

cblock: cblock.cpp cblock.hpp
	g++ -Wall -g2 -std=c++11 -E $< -o cblock.i
	g++ -Wall -g2 -std=c++11 -c $< -o cblock.o
	g++ -Wall -g2 cblock.o -o $@
# x86 tool to dlopen and walk symbol table	
.PHONY: dlprt dlprt-clean dlprt-do
dlprt: dlprt-clean dlprt-do
dlprt-clean:
	rm -f dlprt-x86 dlprt-ve libjit1.so libjit1-x86.so dlprt-x86.log dlprt-ve.log
dlprt-do: dlprt-x86.log
ifeq ($(CC:ncc%=ncc),ncc)
dlprt-do: dlprt-ve.log
endif
dlprt-x86: dlprt.c
	g++ -E $< -o $@.i
	g++ -g2 $< -o $@ -ldl
	# x86 libm.so as is actually a link-script (text file), which shows that most
	# of libm comes from libm.so.6 (with AS_NEEDED for some shared/non-shared vector libs)
	{ ./$@ libm.so.6; echo "exit status $$?"; } >& dlprt-x86.log
dlprt-ve: dlprt.c
	nc++ -E $< -o $@.i
	nc++ -g2 $< -o $@ -ldl
	{ ./$@ libm.so; echo "exit status $$?"; } >& dlprt-ve.log; \
		echo "exit status $$?";
#allsyms-x86: allsyms.cpp
#	g++ -g2 -O2 -std=c++11 -D_GNU_SOURCE $< -o $@ -ldl
#allsyms-ve: allsyms.cpp
#	${CXX} -g2 -O2 -std=c++11 -D_GNU_SOURCE $< -o $@
.PHONY: dlprt-x86.log dlprt-ve.log
dlprt-ve.log: dlprt-ve libjit1.so
	#./dlprt-ve libjit1.so; echo "exit status $$?"
	{ ./dlprt-ve libjit1.so; echo "exit status $$?"; } >> dlprt-ve.log 2>&1; \
		echo "exit status $$?";
dlprt-x86.log: dlprt-x86 libjit1-x86.so
	# The next test FAILS
	{ ./dlprt-x86 libjit1-x86.so; echo "exit status $$?"; } >> dlprt-x86.log 2>&1; \
		echo "exit status $$?";

LIBVELI_SRC:=veliFoo.cpp wrpiFoo.cpp
libveli.a:     $(patsubst %.cpp,%.o,    $(LIBVELI_SRC))
	#$(AR) rcs $@ $^
	rm -f $@; $(AR) rcs $@ $^
libveli-x86.a: $(patsubst %.cpp,%-x86.o,$(LIBVELI_SRC))
	$(AR) rcs $@ $^
$(patsubst $(LIBVELI_SRC),%.cpp,%-x86.o) %-x86.o: %.cpp
	g++ ${CXXFLAGS} -Wall -O2 -c $< -o $@
#$(patsubst $(LIBVELI_SRC),%.cpp,%.o) %.o: %.cpp
#	@# inline asm is incompatible with nc++ -std=c++11
#	$(CXX) -Wall -O2 -c $< -o $@
wrpiFoo.o: wrpiFoo.cpp velogic.hpp
	@# inline asm is incompatible with nc++ -std=c++11
	$(CXX) -Wall -O2 -c $< -o $@
veliFoo.o: veliFoo.cpp velogic.hpp
	@# inline asm is incompatible with nc++ -std=c++11
	$(CXX) ${CXXFLAGS} -Wall -O2 -c $< -o $@

#asmfmt.cpp has a standalone demo program with -D_MAIN compiler	
asmfmt-x86: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp intutil.c intutil.h jitpage.c jitpage.h
	g++ $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %,%.i,$@)
	g++ ${CXXFLAGS} -Wall -D_MAIN asmfmt.cpp jitpage.c intutil.c -o $@ -ldl
asmfmt-ve: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp jitpage.o
	$(CXX) $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %,%.i,$@)
	#$(CXX) $(CXXFLAGS) -O2 -D_MAIN $(filter-out %.hpp,$^) -o $@
	$(CXX) ${CXXFLAGS} -D_MAIN -Wall asmfmt.cpp jitpage.c intutil.c -o $@ -ldl
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
#
# C++ version of jitve_hello.
# This one is more complicated. Besides printing hello world,
# it also returns a very lucky value (7).
#
.PRECIOUS: jitpp_hello
jitpp_hello: jitpp_hello.cpp asmfmt.o intutil.o jitpage.o
	$(CXX) $(CFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<)
	$(CXX) $(CFLAGS) -O2 $^ -o $@ $(LDFLAGS)
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
jitpp_loadreg: jitpp_loadreg.cpp asmfmt.o jitpage.o intutil.o
	-$(CXX) $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<) && echo YAY || echo OHOH for jitpp_loadreg.i
	$(CXX) $(CXXFLAGS) -O2 $^ -o $@ $(LDFLAGS)
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
dltest0: dltest0.c
	$(CC) $(CFLAGS) -o $@ -Wall -Werror $< -ldl
dltest0-x86: dltest0.c	
	g++ $(CXXFLAGS) -o $@ -Wall -Werror $< -ldl
#bin.mk-x86.o: bin.mk
#	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 \
#		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
#		$< $@
ftostring: ftostring.c
	gcc -O3 $< -o $@
bin.mk-x86.lo: bin.mk ftostring
	# objcopy method lacks --add-symbol.  could use custom linker script, but use ftostring...
	./ftostring bin.mk bin_mk >& bin_mk.c
	gcc -fPIC -c bin_mk.c -o $@ #&& rm -f bin_mk.c
	readelf -s $@
	readelf -h $@
bin.mk-ve.lo: bin.mk ftostring
	# objcopy method lacks --add-symbol.  could use custom linker script, but use ftostring...
	./ftostring bin.mk bin_mk >& bin_mk.c
	$(CC) -fPIC -c bin_mk.c -o $@ #&& rm -f bin_mk.c
	readelf -s $@
	readelf -h $@
#bin.mk-ve.o: bin.mk
#	nobjcopy --input binary --output elf64-ve --binary-architecture ve \
#		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
#		$< $@
#	# 0000000000000ee6 D _binary_bin_mk_end
#	# 0000000000000ee6 A _binary_bin_mk_size
#	# 0000000000000000 D _binary_bin_mk_start
#	# This object file is NOT RELOCATABLE, so cannot be put into a .so
#	# because the symbols are missing info (and my linker does no have --add-symbol or such)
#	# use portable 'ftostring.c' and compile (uggh)

.PHONY: dllbuild dllbuild-clean dllbuild-do dllbuild.log dllvebug.log
dllbuild: dllbuild-clean dllbuild-do
dllbuild-clean:
	rm -rf tmp-dllbuild
	rm -f dllbuild-{x86,x86b,ve,veb} libjit1{,-x86}.{a,so} *.o *.lo
dllbuild-do: dllbuild.log dllvebug.log
dllbuild.log:
	# It is important to demo bin.mk basic correctness for the different build methods
	# So now dllbuild puts multiple named builds into tmpdllbuild/ ,
	# differentiated by a a new 'suffix' argument to the test program.
	{ $(MAKE) VERBOSE=1 dllbuild-x86 dllbuild-x86b dllbuild-ve;\
		echo ""; echo "TEST dllbuild x86"; ./dllbuild-x86 7; \
		echo ""; echo "TEST dllbuild x86 dll"; ./dllbuild-x86b 7; \
		echo ""; echo "TEST dllbuild ve"; ./dllbuild-ve 7; \
		echo ""; echo "TEST dllbuild ve ncc";     ./dllbuild-ve 7; \
		echo ""; echo "TEST dllbuild ve clang";   ./dllbuild-ve 7 -clang.c; \
		echo ""; echo "TEST dllbuild ve nc++";    ./dllbuild-ve 7 -ncc.cpp; \
		echo ""; echo "TEST dllbuild ve clang C+intrinsics"; ./dllbuild-ve 7 -vi.c; \
		echo ""; echo "TEST dllbuild ve clang++"; ./dllbuild-ve 7 -clang.cpp; \
		} >& dllbuild.log && echo "dllbuild.log seems OK" || echo "dllbuild.log Huh?"
	# NOTE -clang.cpp expected to fail becase clang++ does not understand "-std=c++11"
dllvebug.log:
	{ $(MAKE) VERBOSE=1 dllbuild-veb; \
		echo ""; echo "TEST dllbuild ve dll ncc";     ./dllbuild-veb 7; \
		echo ""; echo "TEST dllbuild ve dll clang";   ./dllbuild-veb 7 -clang.c; \
		echo ""; echo "TEST dllbuild ve dll nc++";    ./dllbuild-veb 7 -ncc.cpp; \
		echo ""; echo "TEST dllbuild ve dll clang++"; ./dllbuild-veb 7 -clang.cpp; \
		echo ""; echo "TEST dllbuild ve dll clang C+intrinsics"; ./dllbuild-veb 7 -vi.c; \
		} >& dllvebug.log && echo "dllvebug.log seems OK" || echo "dllvebug.log Huh?"
	# NOTE **all** above fail because dllbuild-veb is linked with a shared C++ library.
dllbuild-x86: dllbuild.cpp libjit1-x86.a
	g++ -o $@ $(CXXFLAGS) -Wall -Werror -DDLLBUILD_MAIN $< -L. libjit1-x86.a -ldl
	readelf -d $@
dllbuild-x86b: dllbuild.cpp libjit1-x86.so
	g++ -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -DDLLBUILD_MAIN $< -L. ./libjit1-x86.so -ldl
	readelf -d $@
dllbuild-ve: dllbuild.cpp libjit1.a
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -DDLLBUILD_MAIN $< -L. ./libjit1.a -ldl
	nreadelf -d $@
dllbuild-veb: dllbuild.cpp
	if [ ! -f "vejit/lib/lib$(LIBJIT).so" ]; then $(MAKE) all-vejit-libs; fi
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -DDLLBUILD_MAIN $< \
		-Lvejit/lib -Wl,-rpath,`pwd`/vejit/lib -l$(LIBJIT)
	nreadelf -d $@
# next test show how to dynamically *compile* and load a dll given/ fiail after subdir1/subdir2/ fwrite
dllbuild-vec: dllbuild.cpp libjit1.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -pthread -Wall -Werror -DDLLBUILD_MAIN $< -L. ./libjit1.so
	nreadelf -d $@
# next test show how to dynamically *compile* and load a dll given
# a std::string containing 'C' code.
.PHONY: dltest1.log
RUNCMD:=	
dltest1.log:
	# recreate and run dltest1 versions x86+gcc and VE+ncc
	rm -f dltest1 dltest1-x86 tmp_*.c lib*_lucky.so
	@# '13' has been hard-wired to produce incorrect std::string ccode
	-{ $(MAKE) VERBOSE=1 dltest1-x86 && $(RUNCMD) ./dltest1-x86 13 && echo YAY; } >& $@ || echo "Ohoh, continuing anyway"
ifeq ($(CC),ncc)
	@# '7' should return the correct value from the JIT function.
	-{ echo ""; echo ""; $(MAKE) VERBOSE=1 dltest1 && $(RUNCMD) ve_exec ./dltest1 7 && echo YAY; } &>> $@ || echo "Ohoh, continuing anyway"
endif
	# Attempting JIT-via-clang (compile and cross-compile),
	# even though you might not have them installed...
	-{ $(MAKE) VERBOSE=1       dltest1-clang && $(RUNCMD) ./dltest1-clang 123 && echo YAY; } &>> $@ || echo "Ohoh, continuing anyway"
	-{ $(MAKE) VERBOSE=1       dltest1-nclang && $(RUNCMD) ./dltest1-nclang -1 && echo YAY; } &>> $@ || echo "Ohoh, continuing anyway"
	-ls -l tmp_*.c lib*lucky*.so
	-ls -l tmp_*.c lib*lucky*.so &>> $@

#libgcc_lucky.so     this code and jit code runs on host
#libncc_lucky.so     this code and jit code runs on VE
#libclang_lucky.so   this code(can be gcc) and jit code from clang run on host
#libnclang_lucky.so  this code(can be ncc) and jist code from clang -target ve-linux on VE
dltest1: dltest1.cpp jitpipe.hpp Makefile
	$(CXX) $(CXXFLAGS) -o $@ -Wall -Werror $< 
# Note: following binaries run on x86 so g++ is OK,
#       but the JIT portion uses different compilers
dltest1-x86: dltest1.cpp jitpipe.hpp Makefile
	g++ $(CXXFLAGS) -o $@ -Wall -Werror $< -ldl
dltest1-clang: dltest1.cpp jitpipe.hpp Makefile
	g++ $(CXXFLAGS) -DJIT_CLANG -o $@ -Wall -Werror $< -ldl
dltest1-nclang: dltest1.cpp jitpipe.hpp Makefile
	g++ $(CXXFLAGS) -DJIT_NCLANG -o $@ -Wall -Werror $< -ldl
clean:
	rm -f *.o *.lo *.i *.ii *.out *.gch bin.mk*.o bin_mk.c
	rm -f msk*.i msk*.S msk*.dis msk*.out
	rm -f syscall_hello.o syscall_hello.asm syscall_hello.dis
	rm -f asmkern0.asm asmkern0.dis
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
	for f in tmp_*.S; do b=`basename $$f .S`; rm -f $$b.asm $$b.dis $$b.dump; done
	rm -f tmp*lucky*
	rm -rf tmp
	rm -f jitve_math.asm jitve_hello.asm jitve_hello.s jitve_hello.dis jitve*.dis jitpp_hello.asm
	$(MAKE) -f bugN.mk clean
realclean: clean
	rm -f $(TARGETS)
	rm -f ftostring a.exe 2
	rm -f msk msk0 msk1 msk2 msk3 msk4 syscall_hello smir jitve0
	rm -f bld.log asmfmt.log jitpp_hello.log mk*.log bld*.log
	rm -f tmp_*.S *.bin
	rm -rf CMakeCache.txt CMakeFiles asmfmt asmfmt-x86 asmfmt.txt jitve_hello.s
	$(MAKE) -f bugN.mk realclean
	$(MAKE) -C loops realclean
	$(MAKE) -C loops2 realclean
#
