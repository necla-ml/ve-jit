SHELL:=/bin/bash
.PHONY: test-x86
ifneq ($(CC:ncc%=ncc),ncc)
all:
	${MAKE} test-x86
else
all:
	@echo "x86 versions first ..."
	${MAKE} VERBOSE=1 test-x86
	@echo "ncc versions follow ..."
	${MAKE} VERBOSE=1 test-ve
endif

test-x86:
	ls -l
	${MAKE} VERBOSE=1 clean
	CC=gcc ${MAKE} VERBOSE=1 -f bugNc.mk >& bug-x86.log
	${MAKE} VERBOSE=1 clean
	CXX=g++ ${MAKE} VERBOSE=1 -f bugN.mk >> bug-x86.log 2>&1
	@echo "Tests ran, see bug-x86.log"
test-ve:
	ls -l
	${MAKE} VERBOSE=1 clean
	${MAKE} VERBOSE=1 -f bugNc.mk >& bug-ve.log
	${MAKE} VERBOSE=1 clean
	${MAKE} VERBOSE=1 -f bugN.mk >> bug-ve.log 2>&1
	@echo "Tests ran, see bug-ve.log"

all: ${TARGETS}
clean:
	rm -f bug0.c bug0.cpp hdrs[0-2].c hdrs[0-2].cpp hdrs[0-2].o libvehdrs[0-2].so bug[0-2]
realclean: clean
#last line
