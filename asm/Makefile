VEJIT_ROOT:=..
SHELL:=/bin/bash
TARGETS:=

COMPILE_TYPE:=$(word 1,$(shell $(CC) --version 2>&1))
$(if $(COMPILE_TYPE),,$(error could not determine C compiler type for CC=$(CC))) # VE(ncc) vs x86(gcc or ...)
ifneq ($(COMPILE_TYPE),ncc)
CFLAGS:=-O2 -g2 -pthread
CFLAGS+=-Wall -Werror
CFLAGS+=-Wno-unknown-pragmas
CXXFLAGS:=$(CFLAGS) -std=c++11
VE_EXEC:=time
OBJDUMP:=objdump
OBJCOPY:=objcopy
TARGETS += test_naspipe-x86
TARGETS += jitpp_loadreg-x86

else
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
#CXXFLAGS:=$(CFLAGS) -std=c++11
CXXFLAGS:=$(CFLAGS) -std=gnu++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
TARGETS += test_naspipe-x86 test_naspipe-ve
TARGETS += jitpp_loadreg-x86 jitpp_loadreg-ve
endif

LDFLAGS+=-ldl
CFLAGS+=-std=c99
CFLAGS+=-I..
CXXFLAGS+=-I..

all: $(TARGETS)

jitpage-ve.o: $(VEJIT_ROOT)/jitpage.c $(VEJIT_ROOT)/jitpage.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl
asmfmt-ve.o: $(VEJIT_ROOT)/asmfmt.cpp $(VEJIT_ROOT)/asmfmt.hpp $(VEJIT_ROOT)/asmfmt_fwd.hpp
	$(CXX) ${CXXFLAGS} -O2 -c $< -o $@
intutil-ve.o: $(VEJIT_ROOT)/intutil.c $(VEJIT_ROOT)/intutil.h
	$(CC) ${CFLAGS} -O2 -c $< -o $@
jitpage-x86.o: $(VEJIT_ROOT)/jitpage.c $(VEJIT_ROOT)/jitpage.h
	gcc $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl
asmfmt-x86.o: $(VEJIT_ROOT)/asmfmt.cpp $(VEJIT_ROOT)/asmfmt.hpp $(VEJIT_ROOT)/asmfmt_fwd.hpp
	g++ ${CXXFLAGS} -O2 -c $< -o $@
intutil-x86.o: $(VEJIT_ROOT)/intutil.c $(VEJIT_ROOT)/intutil.h
	gcc ${CFLAGS} -O2 -c $< -o $@
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
test_naspipe-ve: test_naspipe.cpp naspipe.sh
	$(CXX) $(CXXFLAGS) -O2 $< -o $@
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
test_naspipe-x86: test_naspipe.cpp naspipe.sh
	g++ $(CXXFLAGS) -O2 $< -o $@
	./$@ 2>&1 | tee $@.log

# jitpp_loadreg was used to develop the original "load scalar register" optimizations
# The x86 version will just print the code snippets.
.PRECIOUS: jitpp_loadreg-ve jitpp_loadreg-x86
jitpp_loadreg-ve: jitpp_loadreg.cpp asmfmt-ve.o jitpage-ve.o intutil-ve.o
	-$(CXX) $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<) && echo YAY || echo OHOH for jitpp_loadreg.i
	$(CXX) $(CXXFLAGS) -O2 $^ -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Wa,-adhln -c $^ >& $(patsubst %.cpp,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
jitpp_loadreg-x86: jitpp_loadreg.cpp asmfmt-x86.o jitpage-x86.o intutil-x86.o
	-g++ $(CXXFLAGS) -O2 -E -dD $< >& $(patsubst %.cpp,%.i,$<) && echo YAY || echo OHOH for jitpp_loadreg.i
	g++ $(CXXFLAGS) -O2 $^ -o $@ $(LDFLAGS)
	g++ $(CXXFLAGS) -Wa,-adhln -c $^ >& $(patsubst %.cpp,%.asm,$<)
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
clean:
	rm -f *.o *.i tmp*.S a.out ftrace.out *.gch 2 jitpp_loadreg.asm
realclean: clean	
	rm -f $(TARGETS) *.log
