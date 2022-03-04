VEJIT_ROOT:=.
SHELL:=/bin/bash
#GCC=gcc
#GXX=g++
GCC=clang
GCXX=clang++
# normal build target = all
# distribution target = vejit.tar.gz
# major clean and rebuild tests:
#   dllbuild

# on Aurora hosts, gcc is extremely old version, with incomplete <regex> impl
GCXX:=g++
#GCXX:=clang++
X86FLAGS?=
LDFLAGS?=
X86LIBS?=
X86LIBS+=-ldl

UNAME_S:=$(shell uname -s)
OS:=$(patsubst(CYGWIN%,Cygwin,$(UNAME_S))
ifeq ($(OS),Cygwin)
READELF:='echo readelf'
else
READELF:=readelf
endif

# The following idiom is fragile ..... :
#    ifneq ($(CC:ncc%=ncc),ncc)
# bash counterpart: COMPILE_TYPE=`${CC} --version 2>&1 | awk '{print $$1; exit}'`
COMPILE_TYPE:=$(word 1,$(shell $(CC) --version 2>&1))
$(if $(COMPILE_TYPE),,$(error could not determine C compiler type for CC=$(CC))) # VE(ncc) vs x86(gcc or ...)
# ..... and should be replaceed with something like:
#    ifneq ($(COMPILE_TYPE),ncc)

ifneq ($(COMPILE_TYPE),ncc)
$(info Only building a few x86 targets [CC=$(CC)])
#
# only a few things can compile for x86...
#
TARGETS:=asmfmt-x86 veliFoo.o
TARGETS+=veliFoo.o cblock-x86 asmblock-x86
VE_EXEC:=time
OBJDUMP:=objdump
OBJCOPY:=objcopy
SHELL:=/bin/bash
CFLAGS:=-O2 -g2 -pthread
CFLAGS+=-Wall -Werror
CFLAGS+=-Wno-unknown-pragmas
CFLAGS+=-DNDEBUG

# note 'gnu' is needed to support extended asm in nc++
#      17 allows assert inside constexpr, but gnu++17 is not ok for both nc++ and g++
#      (17 not supported by gc++-4.8.5 on ds02)
CXXFLAGS:=$(CFLAGS) -std=gnu++11
LIBVELI_TARGETS:=libveli-x86.a
LIBJIT1_TARGETS:=libjit1-x86.a libjit1-x86.so
LIBJIT1_TARGETS+=libjit1-justc-x86.a # temporary
LDFLAGS+=-ldl
else
$(info ncc compiler detected [CC=$(CC)])
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
TARGETS=libjit1.a libjit1-x86.a asmfmt-ve\
	asmfmt-x86 \
	cblock-x86 asmblock-x86 cblock-ve asmblock-ve
# slow!
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
#CC:=ncc-2.1.28
#CXX:=nc++-2.1.28
#CC:=ncc
#CXX:=nc++
CFLAGS:=-O3 -g2
CFLAGS+=-Wall -Werror
CFLAGS+=-DNDEBUG
#CXXFLAGS:=$(CFLAGS) -std=c++11 # does not allow extended asm -- need 'gnu' extensions...
CXXFLAGS:=$(CFLAGS) -std=gnu++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
LIBVELI_TARGETS:=libveli.a libveli-x86.a
LIBJIT1_TARGETS:=libjit1.a libjit1-x86.a libjit1.so libjit1-x86.so
LIBJIT1_TARGETS+=libjit1-justc-x86.a # temporary
# for ncc-2.x, we should add -ldl (as for x86)
LDFLAGS+=-ldl
endif

CFLAGS+=-D_GNU_SOURCE

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
	@ls -l lib*.a lib*.so
force: # force libs to be recompiled
	rm -f libjit1*.a asmfmt*.o jitpage*.o intutil*.o ve_divmod*.o
	rm -f libveli*.a prgiFoo.o wrpiFoo.o
	rm -f $(patsubst %.cpp,%*.o,$(LIBVELI_SRC)) $(LIBVELI_TARGETS)
	$(MAKE) $(LIBJIT1_TARGETS)
	$(MAKE) $(LIBVELI_TARGETS)

# for tarball...
VEJIT_SHARE:=cblock.cpp ve-msk.cpp dllbuild.cpp ve_divmod.cpp COPYING
VEJIT_SHARE+=ve-asm/veli_loadreg.cpp
VEJIT_LIBS:=libjit1-x86.a libveli-x86.a libjit1-x86.so bin.mk-x86.lo
ifeq ($(COMPILE_TYPE),ncc)
VEJIT_LIBS+=libjit1.a libveli.a libjit1.so bin.mk-ve.lo
endif

ifeq (0,1)
# expanded tarball (old ncc linker issues)
VEJIT_LIBS+=libjit1-justc-x86.a libjit1-cxx-x86.lo # possible ld.so workaround
ifeq ($(COMPILE_TYPE),ncc)
VEJIT_LIBS+=libjit1-justc-ve.a libjit1-cxx-ve.lo # possible ld.so workaround
endif
endif

.PHONY: all-vejit-libs
huh:
	echo VEJIT_LIBS are $(VEJIT_LIBS)	
all-vejit-libs:
	./mklibs.sh 2>&1 | tee mklibs.log	# writes libs into vejit/lib/
vejit.tar.gz: jitpage.h intutil.h vfor.h timer.h \
		intutil.hpp stringutil.hpp throw.hpp \
		asmfmt_fwd.hpp asmfmt.hpp codegenasm.hpp velogic.hpp fuseloop.hpp ve_divmod.hpp \
		cblock.hpp dllbuild.hpp \
		asmfmt.cpp cblock.cpp dllbuild.cpp jitpage.c intutil.c fuseloop.cpp  ve_divmod.cpp \
		ve-msk.hpp ve-msk.cpp \
		jitpage.hpp jitpipe_fwd.hpp jitpipe.hpp cblock.hpp pstreams-1.0.1 bin_mk.c \
		vechash.hpp vechash.cpp asmblock.hpp \
		libjit1-cxx.cpp \
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
	cp -av $(filter %.a,$^) $(filter %.so,$^) $(filter %.lo,$^) vejit/lib/
	cp -av $(filter %.cpp,$^) $(filter %.c,$^) vejit/share/vejit/src/
	cp -av ${VEJIT_SHARE} vejit/share/vejit/
	cp -av Makefile.share vejit/share/vejit/Makefile
	tar czf $@.tmp vejit
	tar tvzf $@.tmp vejit
ifeq ($(COMPILE_TYPE),ncc)
	mv $@.tmp vejit-x86.tar.gz
	@echo "created vejit-x86.tar.gz"
else
	mv $@.tmp $@
endif

#
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
tools/%:
	${MAKE} -C tools $*	
#
# newer api uses jitpage.h (instead of jitve_util.h)
# and supports C++         (asmfmt_fwd.hpp)
#
# Note that libjit1.a could be running x86 code or VE code
#
# NEW: CMakeLists.txt + mklibs.sh to build variants of libjit1
#
.PRECIOUS: asmfmt-ve.o intutil-ve.o jitpage-ve.o
#%-omp.o: %.c: $(CC) ${CFLAGS} -O2 -c $< -o $@
#%-omp-ftrace1.o: %.c: $(CC) ${CFLAGS} -O2 -c $< -o $@
libjit1.a: asmfmt-ve.o jitpage-ve.o intutil-ve.o \
	vechash-ve.o cblock-ve.o asmblock-ve.o dllbuild-ve.o bin.mk-ve.lo ve-msk-ve.o \
	fuseloop-ve.o ve_divmod-ve.o
	rm -f $@
	$(AR) rcs $@ $^
	$(READELF) -h $@
# Simple code without C++ init section crap to avoid VE dynamic loader bug
# with iostream initialization ... TEMPORARY WORKAROUND
libjit1-justc-ve.a: jitpage-ve.o intutil-ve.o bin.mk-ve.lo
	rm -f $@
	$(AR) rcs $@ $^
	$(READELF) -h $@
libjit1-justc-x86.a: jitpage-x86.o intutil-x86.o bin.mk-x86.lo
	rm -f $@
	$(AR) rcs $@ $^
	$(READELF) -h $@
# The other part of the VE bug workaround is to distribute the C++ part
# of libjit1 as a .lo object file, or as a monolithic C++ source file.
# I'll also include libveli .cpp codes into the monolithic version
libjit1-cxx.cpp: asmfmt.cpp vechash.cpp cblock.cpp asmblock.cpp dllbuild.cpp ve-msk.cpp \
	veliFoo.cpp wrpiFoo.cpp fuseloop.cpp ve_divmod.cpp
	sed -e '/^\#ifdef _MAIN/,/^\#endif/d' asmfmt.cpp > $@
	#   cblock is header-only -- the .cpp file is self-test/demo
	# asmblock is header-only -- the .cpp file is self-test/demo
	cat vechash.cpp >> $@
	cat cblock.cpp >> $@
	cat asmblock.cpp >> $@
	cat dllbuild.cpp >> $@
	cat veliFoo.cpp >> $@
	cat wrpiFoo.cpp >> $@
	cat ve-msk.cpp >> $@
	cat fuseloop.cpp >> $@
	cat ve_divmod.cpp >> $@
libjit1-cxx-ve.lo: libjit1-cxx.cpp
	# gnu++11 allows extended asm...
	$(CXX) ${CXXFLAGS} -fPIC -c $< -o $@
libjit1-cxx-x86.lo: libjit1-cxx.cpp
	$(GCXX) ${CXXFLAGS} -fPIC -c $< -o $@

#libbin_mk.a: bin.mk-ve.o
#	rm -f $@; $(AR) rcs $@ $^
#// nc++ only has a .a version std:: C++ library?
#// dlrt-ve needs -lnc++ before dlopen would succeed !
# C things	
intutil-ve.o: intutil.c intutil.h
	$(CC) ${CFLAGS} -O2 -c $< -o $@
jitpage-ve.o: jitpage.c jitpage.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl
# C++ things...	
asmfmt-ve.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
vechash-ve.o: vechash.cpp vechash.hpp asmfmt_fwd.hpp vfor.h throw.hpp
	$(CXX) ${CXXFLAGS} -O2 -c vechash.cpp -o $@
cblock-ve.o: cblock.cpp cblock.hpp
	$(CXX) ${CXXFLAGS} -c $< -o $@
ve-msk-ve.o: ve-msk.cpp ve-msk.hpp
	$(CXX) ${CXXFLAGS} -c $< -o $@
asmblock-ve.o: asmblock.cpp asmblock.hpp
	$(CXX) ${CXXFLAGS} -c $< -o $@
dllbuild-ve.o: dllbuild.cpp
	$(CXX) $(CXXFLAGS) -Wall -Werror -c $< -o $@
fuseloop-ve.o: fuseloop.cpp fuseloop.hpp
	$(CXX) $(CXXFLAGS) -Wall -Werror -c $< -o $@
ve_divmod-ve.o: ve_divmod.cpp
	$(CXX) $(CXXFLAGS) -Wall -Werror -c $< -o $@
# ---- old way (master)
# recall...
#libjit1.a: asmfmt-ve.o jitpage-ve.o intutil-ve.o \
#	vechash-ve.o cblock-ve.o asmblock-ve.o dllbuild-ve.o bin.mk-ve.lo ve-msk-ve.o \
#	fuseloop-ve.o ve_divmod-ve.o
# Note: when sources change, it is actually CMakeLists.txt that governs
#       special compiles for libjit1_omp_ft.so and such (omp ~ OpenMP; ft ~ ftrace support)
#libjit1.so: jitpage.lo intutil.lo bin.mk-ve.lo \
#	vechash.lo asmfmt.lo asmblock-ve.lo cblock-ve.lo dllbuild-ve.lo fuseloop-ve.lo ve_divmod-ve.lo # C++ things
#	$(CXX) -o $@ -shared -Wl,-trace -wL,-verbose $^ #-ldl #-lnc++
# ---- new way (vel)
libjit1.so: jitpage-ve.lo intutil-ve.lo bin.mk-ve.lo \
	asmfmt-ve.lo asmblock-ve.lo cblock-ve.lo dllbuild-ve.lo fuseloop-ve.lo \
	ve_divmod-ve.lo vechash-ve.lo
	$(CXX) -o $@ -shared -Wl,-trace -Wl,-verbose $^ #-ldl #-lnc++
	$(READELF) -h $@
	$(READELF) -d $@
intutil-ve.lo: intutil.c intutil.h
	$(CC) ${CFLAGS} -fPIC -O2 -c $< -o $@
# ---- master
#jitpage.lo: jitpage.c jitpage.h
#	$(CC) $(CFLAGS) -fPIC -O2 -c $< -o $@
#asmfmt.lo: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
# ---- vel
jitpage-ve.lo: jitpage.c jitpage.h
	$(CXX) $(CXXFLAGS) -fPIC -O2 -c $< -o $@ -ldl
asmfmt-ve.lo: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
# ----
	$(CXX) ${CXXFLAGS} -fPIC -O2 -c asmfmt.cpp -o $@
vechash-ve.lo: vechash.cpp vechash.hpp throw.hpp vfor.h
	$(CXX) ${CXXFLAGS} -fPIC -O2 -c vechash.cpp -o $@
cblock-ve.lo: cblock.cpp cblock.hpp
	$(CXX) ${CXXFLAGS} -fPIC -c $< -o $@
ve-msk-ve.lo: ve-msk.cpp ve-msk.hpp
	$(CXX) ${CXXFLAGS} -fPIC -c $< -o $@
asmblock-ve.lo: asmblock.cpp asmblock.hpp
	$(CXX) ${CXXFLAGS} -fPIC -c $< -o $@
dllbuild-ve.lo: dllbuild.cpp
	$(CXX) $(CXXFLAGS) -fPIC -Wall -Werror -c $< -o $@
fuseloop-ve.lo: fuseloop.cpp fuseloop.hpp
	$(CXX) $(CXXFLAGS) -fPIC -Wall -Werror -c $< -o $@
ve_divmod-ve.lo: ve_divmod.cpp
	$(CXX) $(CXXFLAGS) -fPIC -Wall -Werror -c $< -o $@

libjit1-x86.a: asmfmt-x86.o jitpage-x86.o intutil-x86.o \
		cblock-x86.o dllbuild-x86.o bin.mk-x86.lo fuseloop-x86.o  ve_divmod-x86.o \
		vechash-x86.o asmblock-x86.o ve-msk-x86.o
	rm -f $@
	ar rcs $@ $^
	$(READELF) -h $@
	$(READELF) -d $@
libjit1-x86.so: asmfmt-x86.lo jitpage-x86.lo intutil-x86.lo \
		cblock-x86.lo dllbuild-x86.lo bin.mk-x86.lo fuseloop-x86.lo ve_divmod-x86.lo \
		vechash-x86.lo asmblock-x86.lo ve-msk-x86.lo
	$(GCC) -o $@ -shared $^ # -ldl
	$(READELF) -h $@
	$(READELF) -d $@
asmfmt-x86.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp stringutil.hpp jitpage.h
	$(GCXX) ${GXXFLAGS} -O2 -c asmfmt.cpp -o $@
asmfmt-x86.lo: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(GCXX) ${GXXFLAGS} -fPIC -O2 -c asmfmt.cpp -o $@
intutil-x86.o: intutil.c intutil.h
	$(GCC) ${CFLAGS} -O2 -c $< -o $@
intutil-x86.lo: intutil.c intutil.h
	$(GCC) ${CFLAGS} -fPIC -O2 -c $< -o $@
jitpage-x86.o: jitpage.c jitpage.h
	$(GCC) $(CFLAGS) -O2 -c $< -o $@
jitpage-x86.lo: jitpage.c jitpage.h
	$(GCC) $(CFLAGS) -fPIC -O2 -c $< -o $@
cblock-x86.o: cblock.cpp cblock.hpp
	$(GCXX) ${GXXFLAGS} -c $< -o $@
cblock-x86.lo: cblock.cpp cblock.hpp
	$(GCXX) ${GXXFLAGS} -fPIC -c $< -o $@
ve-msk-x86.o: ve-msk.cpp ve-msk.hpp
	$(GCXX) ${GXXFLAGS} -c $< -o $@
ve-msk-x86.lo: ve-msk.cpp ve-msk.hpp
	$(GCXX) ${GXXFLAGS} -fPIC -c $< -o $@
asmblock-x86.o: asmblock.cpp asmblock.hpp
	$(GCXX) ${GXXFLAGS} -g2 -std=c++11 -c $< -o $@
asmblock-x86.lo: asmblock.cpp asmblock.hpp
	$(GCXX) ${GXXFLAGS} -fPIC -c $< -o $@
vechash-x86.o: vechash.cpp vechash.hpp
	$(GCXX) ${GXXFLAGS} -g2 -std=c++11 -c $< -o $@
vechash-x86.lo: vechash.cpp vechash.hpp
	$(GCXX) ${GXXFLAGS} -fPIC -c $< -o $@
dllbuild-x86.o: dllbuild.cpp dllbuild.hpp
	$(GCXX) -o $@ $(GXXFLAGS) -Wall -Werror -c $<
dllbuild-x86.lo: dllbuild.cpp dllbuild.hpp
	$(GCXX) -o $@ $(GXXFLAGS) -fPIC -Wall -Werror -c $<
fuseloop-x86.o: fuseloop.cpp fuseloop.hpp
	$(GCXX) -o $@ $(GXXFLAGS) -Wall -Werror -c $<
fuseloop-x86.lo: fuseloop.cpp fuseloop.hpp
	$(GCXX) -o $@ $(GXXFLAGS) -fPIC -Wall -Werror -c $<
ve_divmod-x86.o: ve_divmod.cpp ve_divmod.hpp cblock.hpp
	$(GCXX) -o $@ $(CXXFLAGS) -Wall -Werror -c $<
ve_divmod-x86.lo: ve_divmod.cpp ve_divmod.hpp cblock.hpp
	$(GCXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -c $<

cblock-x86: cblock.cpp cblock.hpp
	$(GCXX) ${GXXFLAGS} -DMAIN_CBLOCK -c $< -o cblock.o
	$(GCXX) ${GXXFLAGS} -DMAIN_CBLOCK -E $< -o cblock.i
	$(GCXX) -Wall -g2 -DMAIN_CBLOCK cblock.o -o $@
asmblock-x86: asmblock.cpp asmblock.hpp asmfmt-x86.o jitpage-x86.o intutil-x86.o
	$(GCXX) ${GXXFLAGS} -DMAIN_ASMBLOCK $(filter %.cpp,$^) $(filter %.o,$^) -o $@ -ldl

cblock-ve: cblock.cpp cblock.hpp
	${CXX} ${CXXFLAGS} -DMAIN_CBLOCK $< -o $@
asmblock-ve: asmblock.cpp asmblock.hpp asmfmt-ve.o jitpage-ve.o intutil-ve.o
	$(CXX) ${CXXFLAGS} -DMAIN_ASMBLOCK $(filter %.cpp,$^) $(filter %.o,$^) -o $@ -ldl

#
# dlopen/dlsym and library-related wierdness was happening on VE
#
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
libvenobug.so: \
	jitpage.lo \
	intutil.lo \
	bin.mk-ve.lo
	$(CC) -o $@ -shared $^
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
	$(CXX) -o $@ $(CXXFLAGS) -DCODEREMOVE=9 -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^ $(LDFLAGS)
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

#allsyms-x86: allsyms.cpp
#	$(GCXX) -g2 -O2 -std=c++11 -D_GNU_SOURCE $< -o $@ -ldl
#allsyms-ve: allsyms.cpp
#	${CXX} -g2 -O2 -std=c++11 -D_GNU_SOURCE $< -o $@
.PHONY: dlprt-x86.log dlprt-ve.log
dlprt-ve.log: tools/dlprt-ve libjit1.so
	{ $^ libjit1.so; echo "exit status $$?"; } >> dlprt-ve.log 2>&1; \
		echo "exit status $$?";
dlprt-x86.log: tools/dlprt-x86 libjit1-x86.so
	# The next test FAILS
	{ $^ libjit1-x86.so; echo "exit status $$?"; } >> dlprt-x86.log 2>&1; \
		echo "exit status $$?";

LIBVELI_SRC:=veliFoo.cpp wrpiFoo.cpp
libveli.a:     $(patsubst %.cpp,%.o,    $(LIBVELI_SRC))
	#$(AR) rcs $@ $^
	rm -f $@; $(AR) rcs $@ $^
libveli-x86.a: $(patsubst %.cpp,%-x86.o,$(LIBVELI_SRC))
	$(AR) rcs $@ $^
$(patsubst $(LIBVELI_SRC),%.cpp,%-x86.o) %-x86.o: %.cpp
	$(GCXX) ${CXXFLAGS} -Wall -O2 -c $< -o $@
$(patsubst $(LIBVELI_SRC),%.cpp,%.o) %.o: %.cpp
	@#$(CXX) -Wall -O2 -c $< -o $@
	@# inline asm is incompatible with nc++ -std=c++11 # AHAA must use -std=gnu++11
	$(CXX) ${CXXFLAGS} -Wall -O2 -c $< -o $@
#wrpiFoo.o: wrpiFoo.cpp velogic.hpp
#	@#$(CXX) -Wall -O2 -c $< -o $@
#	@# inline asm is incompatible with nc++ -std=c++11 # AHAA must use -std=gnu++11
#	$(CXX) ${CXXFLAGS} -Wall -O2 -c $< -o $@
#veliFoo.o: veliFoo.cpp velogic.hpp
#	@# inline asm is incompatible with nc++ -std=c++11 # AHAA must use -std=gnu++11
#	$(CXX) ${CXXFLAGS} -Wall -O2 -c $< -o $@

#asmfmt.cpp has a standalone demo program with -D_MAIN compiler	
asmfmt-x86: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp intutil.c intutil.h jitpage.c jitpage.h
	$(GCXX) $(CXXFLAGS) -O2 -dD -E $< >& $(patsubst %,%.i,$@)
	$(GCXX) ${CXXFLAGS} -Wall -D_MAIN asmfmt.cpp -x c++ jitpage.c intutil.c -o $@ -ldl
asmfmt-main-ve.o: asmfmt.cpp asmfmt.hpp asmfmt_fwd.hpp
	$(CXX) $(CXXFLAGS) -D_MAIN -dD -E $< >& $(patsubst %,%.i,$@)
	#$(CXX) $(CXXFLAGS) -O2 -D_MAIN $(filter-out %.hpp,$^) -o $@
	$(CXX) ${CXXFLAGS} -O2 -D_MAIN -Wall -c asmfmt.cpp -o $@
asmfmt-ve: asmfmt-main-ve.o jitpage-ve.o intutil-ve.o
	$(CXX) ${CXXFLAGS} -D_MAIN -Wall $^ -o $@ -ldl
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
%.asm: %.c
	$(CC) $(CFLAGS) -g2 -Wa,-adhln -S $< >& $*.s
	$(CC) $(CFLAGS) -Wa,-adhln -c $< >& $*.asm
	$(CC) $(CFLAGS) -c $< -o $*.o
	$(OBJDUMP) -d $*.o > $*.dis
#bin.mk-x86.o: bin.mk
#	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 \
#		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
#		$< $@
ftostring: ftostring.c
	$(GCC) -O3 $< -o $@
bin.mk-x86.lo bin_mk.c: bin.mk ftostring
	# objcopy method lacks --add-symbol.  could use custom linker script, but use ftostring...
	./ftostring bin.mk bin_mk >& bin_mk.c
	$(GCC) -fPIC -c bin_mk.c -o $@ #&& rm -f bin_mk.c
	$(READELF) -s $@
	$(READELF) -h $@
bin.mk-ve.lo: bin.mk ftostring
	# objcopy method lacks --add-symbol.  could use custom linker script, but use ftostring...
	./ftostring bin.mk bin_mk >& bin_mk.c
	$(CC) -fPIC -c bin_mk.c -o $@ #&& rm -f bin_mk.c
	$(READELF) -s $@
	$(READELF) -h $@
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

.PHONY: dllbuild dllbuild-clean dllbuild-do dllbuild.log dllveok.log
dllbuild: dllbuild-clean dllbuild-do
dllbuild-clean:
	rm -rf tmp-dllbuild
	rm -f dllbuild-{x86,x86b,ve,veb} *.o *.lo
	#rm -f libjit1{,-x86}.{a,so}
dllbuild-do: dllbuild.log dllveok.log
dllbuild.log:
	# It is important to demo bin.mk basic correctness for the different build methods
	# So now dllbuild puts multiple named builds into tmpdllbuild/ ,
	# differentiated by a a new 'suffix' argument to the test program.
	{ $(MAKE) VERBOSE=1 dllbuild-x86 dllbuild-x86b && { \
		echo "(x86 tests will not be able to run VE shared library)"; \
		echo ""; echo "TEST dllbuild x86"; ./dllbuild-x86 7; \
		echo ""; echo "TEST dllbuild x86 dll"; ./dllbuild-x86b 7; \
		} ; \
	$(MAKE) VERBOSE=1 dllbuild-ve && { \
		echo " These simple tests seemed to work, but may be broken with ncc>=2.0?"; \
		echo ""; echo "TEST dllbuild ve"; ./dllbuild-ve 7; \
		echo ""; echo "TEST dllbuild ve ncc";     ./dllbuild-ve 7; \
		echo ""; echo "TEST dllbuild ve clang";   ./dllbuild-ve 7 -clang.c; \
		echo ""; echo "TEST dllbuild ve nc++";    ./dllbuild-ve 7 -ncc.cpp; \
		echo ""; echo "TEST dllbuild ve clang C+intrinsics"; ./dllbuild-ve 7 -vi.c; \
		echo ""; echo "TEST dllbuild ve clang++"; ./dllbuild-ve 7 -clang.cpp; \
		} ; } >& dllbuild.log && echo "dllbuild.log seems OK" || echo "dllbuild.log Huh?"
	# NOTE -clang.cpp expected to fail becase clang++ does not understand "-std=c++11"
dllveok.log: # c++ and shared objects now work (used to be called dllvebug.log)
	{ $(MAKE) VERBOSE=1 dllbuild-veb && { \
		echo ""; echo "TEST dllbuild ve dll ncc";     ./dllbuild-veb 7; \
		echo ""; echo "TEST dllbuild ve dll clang";   ./dllbuild-veb 7 -clang.c; \
		echo ""; echo "TEST dllbuild ve dll nc++";    ./dllbuild-veb 7 -ncc.cpp; \
		echo ""; echo "TEST dllbuild ve dll clang++"; ./dllbuild-veb 7 -clang.cpp; \
		echo ""; echo "TEST dllbuild ve dll clang C+intrinsics"; ./dllbuild-veb 7 -vi.c; \
		} ; } >& dllveok.log && echo "dllveok.log seems OK" || echo "dllveok.log Huh?"
	# NOTE **all** above fail because dllbuild-veb is linked with a shared C++ library.
dllbuild-x86: dllbuild.cpp libjit1-x86.a
	$(GCXX) -o $@ $(CXXFLAGS) -Wall -Werror -DDLLBUILD_MAIN $< -L. libjit1-x86.a -ldl
	$(READELF) -d $@
dllbuild-x86b: dllbuild.cpp libjit1-x86.so
	$(GCXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -DDLLBUILD_MAIN $< -L. ./libjit1-x86.so -ldl
	$(READELF) -d $@
dllbuild-ve: dllbuild.cpp libjit1.a
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -DDLLBUILD_MAIN $< -L. ./libjit1.a -ldl
	nreadelf -d $@
dllbuild-veb: dllbuild.cpp
	if [ ! -f "vejit/lib/lib$(LIBJIT).so" ]; then $(MAKE) all-vejit-libs; fi
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -DDLLBUILD_MAIN $< \
		-Wl,--trace -Wl,--verbose \
		-Lvejit/lib -Wl,-rpath,`pwd`/vejit/lib -l$(LIBJIT) -ldl
	nreadelf -d $@
# next test show how to dynamically *compile* and load a dll given/ fiail after subdir1/subdir2/ fwrite
dllbuild-vec: dllbuild.cpp libjit1.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -pthread -Wall -Werror -DDLLBUILD_MAIN $< -L. ./libjit1.so
	nreadelf -d $@
cjitDemo: cjitDemo.cpp cblock.hpp dllbuild.hpp jitpipe.hpp libjit1-x86.so
	@# hello world multiple function dllbuild example (cblock,dllbuild,jitpipe)
	$(GCXX) -Wall -Werror -std=c++11 -ggdb -O3 ${X86FLAGS} ${LDFLAGS} $(filter-out %.hpp,$^) ${X86LIBS} -o $@

VECC:=ncc
VECXX:=nc++
ve_fastdiv-ve.o: ve_fastdiv.c
	$(VECC) ${CFLAGS} -O2 -E $< -o ve_fastdiv.i
	$(VECC) ${CFLAGS} -O2 -S $< -o ve_fastdiv.S
	$(VECC) ${CFLAGS} -O2 -c $< -o $@
ve_fastdiv-ve.lo:
	$(VECC) ${CFLAGS} -fPIC -O2 -c $< -o $@
ve_fastdiv-x86.o: ve_fastdiv.c
	g++ ${CXXFLAGS} -O2 -c asmfmt.cpp -o $@
ve_fastdiv-x86.lo:
	g++ ${CXXFLAGS} -fPIC -O2 -c asmfmt.cpp -o $@
ve_fastdiv-ve.lo: ve_fastdiv.c
test_ve_fastdiv-x86: test_ve_fastdiv.cpp ve_fastdiv-x86.o ve_fastdiv.h timer.h
	$(VECXX) -std=gnu++11 -O3 -o $@ test_ve_fastdiv.cpp ve_fastdiv.c
test_ve_fastdiv-ve: test_ve_fastdiv.cpp ve_fastdiv-ve.o ve_fastdiv.h timer.h
	$(VECXX) -std=gnu++11 -O3 -E test_ve_fastdiv.cpp -o test_ve_fastdiv.i
	$(VECXX) -std=gnu++11 -O3 -o $@ $(filter-out %.h,$^)
test_ve_fastdiv-speed-x86: test_ve_fastdiv.cpp ve_fastdiv-x86.o ve_fastdiv.h timer.h
	$(VECXX) -std=gnu++11 -DSPEED=1 -O3 -S test_ve_fastdiv.cpp -o test_ve_fastdiv-x86.S
	$(VECXX) -std=gnu++11 -DSPEED=1 -O3 -o $@ test_ve_fastdiv.cpp ve_fastdiv.c
test_ve_fastdiv-speed-ve: test_ve_fastdiv.cpp ve_fastdiv-ve.o ve_fastdiv.h timer.h
	$(VECXX) -std=gnu++11 -DSPEED=1 -O3 -S test_ve_fastdiv.cpp -o test_ve_fastdiv.S
	$(VECXX) -std=gnu++11 -DSPEED=1 -O3 -o $@ $(filter-out %.h,$^)
jload_demo: jload_demo.cpp libjit1.a
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -DDLLBUILD_MAIN $< \
		libjit1.a -ldl
easm_demo: easm_demo.cpp
	-nc++ easm_demo.cpp -Wall -o easm_demo && ./easm_demo
	-clang++ -std=gnu++14 -target linux-ve -O3 -mllvm \
		-show-spill-message-vec -fno-vectorize -fno-unroll-loops \
		-fno-slp-vectorize -fno-crash-diagnostics \
		-S easm_demo.cpp -Wall -o easm_demo.cpp.s
	clang++ -std=gnu++14 -target linux-ve -O3 -mllvm \
		-show-spill-message-vec -fno-vectorize -fno-unroll-loops \
		-fno-slp-vectorize -fno-crash-diagnostics \
		easm_demo.cpp -Wall -o easm_demo \
		&& ./easm_demo

# next test show how to dynamically *compile* and load a dll given
# a std::string containing 'C' code.
clean:
	rm -f *.o *.lo *.i *.ii *.out *.gch bin.mk*.o bin_mk.c
	rm -f libjit1-cxx.cpp libjit1-cxx*.o libjit1-cxx*.lo tmp_*.s tmp*lucky*
	for f in *.bin; do b=`basename $$f .bin`; rm -f $$b.asm $$b.o $$b.dis $$b.dump; done
	for f in tmp_*.S; do b=`basename $$f .S`; rm -f $$b.asm $$b.dis $$b.dump; done
	rm -rf tmp tmp-dllbuild
	rm -f asmfmt.s foo.s intutil.s jitpage.s vechash.s
	rm -f empty.c empty.cpp libvehdrs*.so libveasmfmt*.so libvenobug*.so libveempty.so hdrs?.cpp
	$(MAKE) -C bug clean
	$(MAKE) -C tools clean
realclean: clean
	rm -f $(TARGETS) jitpp_loadreg
	rm -f tmp_*.S *.bin
	rm -f msk msk0 msk1 msk2 msk3 msk4 syscall_hello ftostring a.exe 2
	rm -f bld.log asmfmt.log jit*.log mk*.log bld*.log test*.log dl*.log syscall*.log
	rm -f CMakeCache.txt CMakeFiles asmfmt asmfmt-x86 asmfmt.txt
	rm -f dllbuild-ve dllbuild-veb dllbuild-x86 dllbuild-x86b
	rm -f dllok0 dllok2 dllok3 dllok4
	rm -f dllvebug1 dllvebug10 dllvebug2 
	rm -f libclang_lucky.so libgcc_lucky.so libncc_lucky.so
	rm -f cblock-ve cblock-x86 asmblock-ve asmblock-x86
	rm -f asmfmt-ve.log distro.log [a-zA-Z].log x.prt x86.log
	rm -rf vejit # distro tarball tree
	rm -f vejit.tar.gz.tmp vejit-x86.tar.gz # keep vejit.tar.gz (VE distro)
	$(MAKE) -C tools realclean
	$(MAKE) -C bug realclean
	$(MAKE) -C loops realclean
	$(MAKE) -C loops2 realclean
#
