all: fuse2 fuse2lin
fuse2: fuse2.cpp
	g++ -Wall -std=c++11 -O3 $< -o $@
%: %.cpp
	g++ -Wall -std=c++11 -O3 -E $< -o $@.i
	g++ -Wall -std=c++11 -O3 $< -o $@
clean:
	rm -f *.i *.o fuse2 fuse2lin repro *.log
realclean: clean	
#	
