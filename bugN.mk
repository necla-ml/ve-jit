all: bug0 bug1
.PHONY: bug0.cpp bug1.cpp hdrs0.cpp hdrs1.cpp clean
hdrs%.o: hdrs%.cpp 
	$(CXX) ${CXXFLAGS} -fPIC -O2 -c $< -o $@
libvehdrs%.so: hdrs%.o
	$(CXX) -o $@ -shared $^
	#nnm -C $@
#bug%: bug0.cpp libvenobug.so libvehdrs%.so
bug%: bug0.cpp libvehdrs%.so
	#$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	$(CXX) -o $@ $(CXXFLAGS) -fPIC -Wall -Werror -L. -Wl,-rpath=`pwd` $^
	./$@ 7 && echo "Exit status $$?" || echo "OHOH Exit status $$?"
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
clean:
	rm -f hdrs0.cpp hdrs1.cpp hdrs2.cpp hdrs*.o libvehdrs*.so
realclean: clean
	rm -f bug0 bug1 bug2
#
