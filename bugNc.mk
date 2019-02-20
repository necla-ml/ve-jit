all: bug0 bug1 bug2
.PHONY: bug0.c bug1.c bug2.c hdrs0.c hdrs1.c hdrs2.c clean
.PRECIOUS: \
	bug0.c hdrs0.c libvehdrs0.so hdrs0.o \
	bug1.c hdrs1.c libvehdrs1.so hdrs1.o  \
	bug1.c hdrs2.c libvehdrs2.so hdrs2.o 
CFLAGS:=
ifeq ($(CC:ncc%=ncc),ncc)
CFLAGS+=-no-proginf
NM:=nnm
else
NM:=nm
endif
Hello:
	@echo "CC ${CC}"
	@echo "CFLAGS ${CFLAGS}"
	${CC} --version
hdrs%.o: hdrs%.c 
	$(CC) -o $@ ${CFLAGS} -fPIC -O2 -c $<
	$(NM) $@
libvehdrs%.so: hdrs%.o
	$(CC) -o $@ -shared $^
	#$(NM) $@
#bug%: bug0.cpp libvenobug.so libvehdrs%.so
bug%: bug0.c libvehdrs%.so
	$(CC) -o $@ $(CFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7 && echo "Exit status $$?" || echo "OHOH Exit status $$?"
	@echo "Without library..."
	$(CC) $(CFLAGS) -fPIC -Wall $< && ./a.out && echo YAHOO
	@echo "------------------------------------------------------------ END $@"
	@echo ""
bug0.c:
	{ \
		echo '#include <stdio.h>'; \
		echo 'int main(int argc,char**argv){ if(argc) printf("Goodbye from %s\n",argv[0]); return 0; }'; \
		} > $@
hdrs0.c:
	{ \
		echo '#include "stdint.h"'; \
		} > $@
hdrs1.c:
	{ \
		echo '#include <stdio.h>'; \
		} > $@
hdrs2.c: # This one actually is OK.
	{ \
		echo '#include <stdio.h>'; \
		echo 'int foo() { printf("7"); return 7; }'; \
		} > $@
clean:
	rm -f hdrs0.c hdrs1.c hdrs2.c hdrs*.o libvehdrs*.so
realclean: clean
	rm -f bug0 bug1 bug2
#
