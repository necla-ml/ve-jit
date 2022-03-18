# vim: ts=8 sw=8 noet
####################################################
# Choose machine (library suffix) and compiler
ARCH?=x86
ifeq ($(ARCH),ve)
CC=ncc
CXX=nc++
else
CC=gcc
CXX=g++
ARCH:=x86
endif

####################################################

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
#current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
mkfile_dir := $(dir ${mkfile_path})

# For standalone testing:
CFLAGS?=-std=c11 -O3 -fPIC -DNDEBUG -g -Wall -Werror -I${mkfile_dir}
CXXFLAGS?=-std=c++17 -O3 -fPIC -DNDEBUG -g2 -Wall -Werror -I${mkfile_dir}
ifeq ($(CC),gcc)
	CFLAGS+= -Wpedantic
	CXXFLAGS+= -Wpedantic
endif

# FOR pyTorch v1.8:
# The following is used by cmake for Torch compiles, so we want this library
# to AVOID std::__cxx11::basic_string and just USE std::basic_string)
#CXXFLAGS+=-D_GLIBCXX_USE_CXX11_ABI=0
# We're going to static link this .a into a .so -fPIC is reqd ...
#
# FOR pyTorch v1.10:

.PHONY: all test clean realclean

# TODO: would the .so be better?
all: hello libconv-$(ARCH).a libconvxx-$(ARCH).a libconvxx-$(ARCH).so

# NOTE: convParam.hpp and convParam.cpp are DEPRECATED & moved to old/
API_CHEADERS=convParam.h conv_features.h csv2examples.h
API_HEADERS=$(API_CHEADERS) convParamxx.h csv2examples.hpp
API_CSOURCES=cConvParam.c
#API_CSOURCES+=conv_features.c
API_CXXSOURCES=convParamxx.cpp csv2examples.cpp

API_COBJECTS=$(API_CSOURCES:.c=-$(ARCH).o)
API_OBJECTS=$(API_COBJECTS) $(API_CXXSOURCES:.cpp=-$(ARCH).o)

%-$(ARCH).o: %.cpp $(API_HEADERS)
	$(CXX) $(CXXFLAGS) -fPIC -E $< -o $<-$(ARCH).i
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@
%-$(ARCH).o: %.c $(API_CHEADERS)
	$(CC) -DCONVPARAM_CLIB $(CFLAGS) -fPIC -E $< -o $<-$(ARCH).i
	$(CC) -DCONVPARAM_CLIB $(CFLAGS) -fPIC -c $< -o $@
#old/%.o: old/%.cpp $(API_HEADERS)
#	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@
#old/%.o: old/%.c $(API_CHEADERS)
#	$(CC) -DCONVPARAM_CLIB $(CFLAGS) -fPIC -c $< -o $@
hello:
	echo 'HEADERS : $(API_HEADERS)'
	echo 'SOURCES : $(API_CSOURCES) $(API_CXXSOURCES)'
	echo 'OBJECTS : $(API_OBJECTS)'
libconv-$(ARCH).a: $(API_COBJECTS)
	$(AR) rcs $@ $^
libconvxx-$(ARCH).a: $(API_OBJECTS)
	$(AR) rcs $@ $^

libconvxx-$(ARCH).so: $(API_COBJECTS) $(API_OBJECTS)
	# the .o compile *are* -fPIC already ...
	$(CXX) -shared -o $@ $^

test: test1 test2 test3 test4
	./test1 && ./test2 && ./test3 && ./test4
#test0: old/convParam.hpp old/convParam.cpp
#test0: test0.cpp old/convParam.o libconvxx-$(ARCH).a # test namespace convparam (v0)
#	@# Note: library must be last dependency
#	$(CXX) $(CXXFLAGS) -UNDEBUG -L. $^ -o $@
test1: test1.cpp libconvxx-$(ARCH).a # test namespace conv (v1)
	$(CXX) $(CXXFLAGS) -UNDEBUG -L. $^ -o $@
test2: test2.cpp libconvxx-$(ARCH).a # test csv2examples
	$(CXX) $(CXXFLAGS) -UNDEBUG -L. $^ -o $@
test3: test3.cpp libconvxx-$(ARCH).a # test csv2examples layer name shortenings
	$(CXX) $(CXXFLAGS) -UNDEBUG -L. $^ -o $@
test4: test4.cpp libconvxx-$(ARCH).a
	$(CXX) $(CXXFLAGS) -UNDEBUG -L. $^ -o $@
	
clean:
	rm -f $(API_OBJECTS) test1 test2 test3 test4 *.gch a.out *.i
realclean: clean
	rm -f libconv*.a libconvxx*.a libconvxx*.so
#last line
