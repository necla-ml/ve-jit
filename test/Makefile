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
TARGETS += dltest0-x86
TARGETS += dltest1-x86

else
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
#CXXFLAGS:=$(CFLAGS) -std=c++11
CXXFLAGS:=$(CFLAGS) -std=gnu++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
TARGETS += dltest0-x86 dltest0-ve
TARGETS += dltest1-x86 dltest1-x86
endif

LDFLAGS+=-ldl
CFLAGS+=-std=c99
CFLAGS+=-I..
CXXFLAGS+=-I..

all: $(TARGETS) dltest1.log

dltest0-ve: dltest0.c
	$(CC) $(CFLAGS) -o $@ -Wall -Werror $< -ldl
dltest0-x86: dltest0.c	
	g++ $(CXXFLAGS) -o $@ -Wall -Werror $< -ldl

.PHONY: dltest1.log
#
# Try out a very very simple C Jit function in a shared library,
# with various x86/ve compilers.   Uses jitpipe.hpp stuff, which
# later got refined into dllbuild.hpp --- but for more ambitious
# C JIT shared libraries, the VE dynamic linker was having problems.
#
# Oh well, at least this trivial example seems OK.
#
dltest1.log:
	# recreate and run dltest1 versions x86+gcc and VE+ncc
	rm -f dltest1 dltest1-x86 tmp_*.c lib*_lucky.so
	@# '13' has been hard-wired to produce incorrect std::string ccode
	-{ $(MAKE) VERBOSE=1 dltest1-x86 && $(VE_EXEC) ./dltest1-x86 13 && echo YAY; } >& $@ || echo "Ohoh, continuing anyway"
ifeq ($(CC),ncc)
	@# '7' should return the correct value from the JIT function.
	-{ echo ""; echo ""; $(MAKE) VERBOSE=1 dltest1-ve && \
		$(VE_EXEC) ve_exec ./dltest1-ve 7 && echo YAY; } \
		&>> $@ || echo "Ohoh, continuing anyway"
endif
	# Attempting JIT-via-clang (compile and cross-compile),
	# even though you might not have them installed...
	-{ $(MAKE) VERBOSE=1       dltest1-clang && $(VE_EXEC) ./dltest1-clang 123 \
		&& echo YAY; } &>> $@ || echo "Ohoh, continuing anyway"
	-{ $(MAKE) VERBOSE=1       dltest1-nclang && $(VE_EXEC) ./dltest1-nclang -1 \
		&& echo YAY; } &>> $@ || echo "Ohoh, continuing anyway"
	-ls -l tmp_*.c lib*lucky*.so
	-ls -l tmp_*.c lib*lucky*.so &>> $@

#libgcc_lucky.so     this code and jit code runs on host
#libncc_lucky.so     this code and jit code runs on VE
#libclang_lucky.so   this code(can be gcc) and jit code from clang run on host
#libnclang_lucky.so  this code(can be ncc) and jist code from clang -target ve-linux on VE
dltest1-ve: dltest1.cpp ../jitpipe.hpp Makefile
	$(CXX) $(CXXFLAGS) -o $@ -Wall -Werror $< 
# Note: following binaries run on x86 so g++ is OK,
#       but the JIT portion uses different compilers
dltest1-x86: dltest1.cpp ../jitpipe.hpp Makefile
	g++ $(CXXFLAGS) -o $@ -Wall -Werror $< -ldl
dltest1-clang: dltest1.cpp ../jitpipe.hpp Makefile
	g++ $(CXXFLAGS) -DJIT_CLANG -o $@ -Wall -Werror $< -ldl
dltest1-nclang: dltest1.cpp ../jitpipe.hpp Makefile
	g++ $(CXXFLAGS) -DJIT_NCLANG -o $@ -Wall -Werror $< -ldl

clean:
	rm -f tmp*
	rm -f dltest0-ve dltest0-x86
	rm -f dltest1-ve dltest1-clang dltest1-nclang dltest1-x86
	rm -f lib*_lucky.so
realclean: clean
	dltest1.log
