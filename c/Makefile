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
TARGETS+=test_strMconst-ve

else
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
#CXXFLAGS:=$(CFLAGS) -std=c++11
CXXFLAGS:=$(CFLAGS) -std=gnu++11
VE_EXEC:=ve_exec
OBJDUMP:=nobjdump
OBJCOPY:=nobjcopy
TARGETS+=test_strMconst-x86 test_strMconst-ve
endif

LDFLAGS+=-ldl
CFLAGS+=-std=c99
CFLAGS+=-I..
CXXFLAGS+=-I..

all: help $(TARGETS)
help:
	echo 'targets: $(TARGETS)'

jitpage-ve.o: $(VEJIT_ROOT)/jitpage.c $(VEJIT_ROOT)/jitpage.h
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl
jitpage-x86.o: $(VEJIT_ROOT)/jitpage.c $(VEJIT_ROOT)/jitpage.h
	gcc $(CFLAGS) -D_GNU_SOURCE -c $< -o $@ -ldl

# test conversion of constant to VE M-value format
#   (M)N, for M in [0,63] N in {0,1} means
#   " M bits of N, with the rest as !N "
test_strMconst-ve: test_strMconst.c jitpage-ve.o
	$(CC) $(CFLAGS) -O2 $^ -o $@ -ldl
	$(VE_EXEC) ./$@ 2>&1 | tee $@.log
test_strMconst-x86: test_strMconst.c jitpage-x86.o
	gcc $(CFLAGS) -O2 $^ -o $@ -ldl
	./$@ 2>&1 | tee $@.log

clean:
	rm -f *.o *.i tmp*.S a.out ftrace.out *.gch 2
realclean: clean	
	rm -f test_strMconst-ve test_strMconst-x86 $(TARGETS) *.log
