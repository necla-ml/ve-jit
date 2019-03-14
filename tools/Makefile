SHELL:=/bin/bash
TARGETS:=
TARGETS+=dlprt-x86

ifneq ($(CC:ncc%=ncc),ncc)
VE_EXEC:=time
CFLAGS:=-O2 -g2 -pthread
CFLAGS+=-Wall -Werror
CFLAGS+=-Wno-unknown-pragmas
CXXFLAGS:=$(CFLAGS) -std=c++11

else
TARGETS+=dlprt-ve
VE_EXEC:=ve_exec
CFLAGS:=-O2 -g2
CFLAGS+=-Wall -Werror
CXXFLAGS:=$(CFLAGS) -std=c++11
#CXXFLAGS:=$(CFLAGS) -std=gnu++11
endif

all: ${TARGETS}

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
clean:
	rm -f ${TARGETS} *.i
realclean: clean	
	rm -f *.log
#last line
