all: Hello bug0 bug1 bug2
.PHONY: Hello bug0.cpp bug1.cpp bug2.cpp hdrs0.cpp hdrs1.cpp hdrs2.cpp clean
.PRECIOUS: \
	bug0.cpp hdrs0.cpp libvehdrs0.so hdrs0.o \
	bug1.cpp hdrs1.cpp libvehdrs1.so hdrs1.o  \
	bug1.cpp hdrs2.cpp libvehdrs2.so hdrs2.o 
CXXFLAGS:=
ifeq ($(CXX:nc++%=ncc++),nc++)
CXXFLAGS+=-std=c++11 -no-proginf
LDFLAGS:=

#LDFLAGS+=-Wl,--enable-new-dtags
#LDFLAGS+=-Wl,-rpath,/opt/nec/ve/lib
#LDFLAGS+=-Wl,--spare-dynamic-tags,500
#LDFLAGS+=-lrt -lpthread

NM:=nnm
else
NM:=nm
endif

Hello:
	@echo "CXX ${CXX}"
	@echo "CXXFLAGS ${CXXFLAGS}"
	${CXX} --version
	ls -l
hdrs%.o: hdrs%.cpp 
	$(CXX) -o $@ ${CXXFLAGS} -fPIC -O2 -c $<
	#$(NM) -C $@
libvehdrs%.so: hdrs%.o
	$(CXX) -o $@ -shared $^ $(LDFLAGS)
#bug%: bug0.cpp libvenobug.so libvehdrs%.so
bug%: bug0.cpp libvehdrs%.so
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath,`pwd` $^ $(LDFLAGS)
	./$@ 7 && echo "Exit status $$?" || echo "OHOH Exit status $$?"
	@echo "Without library..."
	$(CXX) $(CXXFLAGS) -fPIC -Wall $< && ./a.out && echo YAHOO
	@echo "------------------------------------------------------------ END $@"
	@echo ""
bug0.cpp:
	{ \
		echo '#include <iostream>'; \
		echo '#include <iomanip>'; \
		echo 'using namespace std;'; \
		echo 'int main(int,char**){ cout<<"Goodbye"<<endl; }'; \
		} > $@
hdrs0.cpp:
	{ \
		echo '#include "stdint.h"'; \
		} > $@
hdrs1.cpp:
	{ \
		echo '#include <iostream>'; \
		} > $@
hdrs2.cpp:
	# This one is OK
	{ \
		echo 'int foo() { return 7; }'; \
		} > $@
	# This one fails
	{ \
		echo '#include <iostream>'; \
		echo 'int foo() { std::cout<<"7"; return 7; }'; \
		} > $@
clean:
	rm -f hdrs0.cpp hdrs1.cpp hdrs2.cpp hdrs*.o libvehdrs*.so
realclean: clean
	rm -f bug0 bug1 bug2
#
