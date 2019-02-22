#!/bin/bash
rm -rf build
mkdir build
cd build
rm -f mklib.log
echo "cmake builds in build/, logging to mklib.log"
BLD='-DCMAKE_BUILD_TYPE=Debug'
CLN='' # or make clean?
HAVE_NCC=0
if ncc --version >&/dev/null; then HAVE_NCC=1; fi
if [ $HAVE_NCC -ne 0 ]; then
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=OFF -DUSE_FTRACE=0 -DUSE_OPENMP=OFF .. \
	&& make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=OFF -DUSE_FTRACE=0 -DUSE_OPENMP=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=OFF -DUSE_FTRACE=1 -DUSE_OPENMP=OFF .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=OFF -DUSE_FTRACE=1 -DUSE_OPENMP=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=ON -DUSE_FTRACE=0 -DUSE_OPENMP=OFF .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=ON -DUSE_FTRACE=0 -DUSE_OPENMP=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=ON -DUSE_FTRACE=1 -DUSE_OPENMP=OFF .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=ON -DUSE_FTRACE=1 -DUSE_OPENMP=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
fi

#mkdir -P ../vejit/lib
#cp -v libjit1* ../vejit/lib/
#rm -f ./libjit1*

{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=OFF -DUSE_OPENMP=OFF -DUSE_GCC=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=OFF -DUSE_OPENMP=ON  -DUSE_GCC=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=ON  -DUSE_OPENMP=OFF -DUSE_GCC=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }
{ rm -f CMakeCache.txt; cmake ${BLD} -DBUILD_SHARED=ON  -DUSE_OPENMP=ON  -DUSE_GCC=ON .. \
	&& make clean && make VERBOSE=1 && ls -lrst; } || { echo "OHOH"; false; }

mkdir ../vejit || echo 'OK (../vejit)'
mkdir ../vejit/lib || echo 'OK (../vejit/lib)'
cp -v libjit1* ../vejit/lib/
ls -lrst ../vejit/lib
