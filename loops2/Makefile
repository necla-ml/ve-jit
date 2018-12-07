all: fuse2 fuse2lin
CFLAGS:=-I../regs	
fuse2: fuse2.cpp
	g++ $(CFLAGS) -Wall -std=c++11 -O3 $< -o $@
%: %.cpp
	g++ $(CFLAGS) -Wall -std=c++11 -O3 -E $< -o $@.i
	g++ $(CFLAGS) -Wall -std=c++11 -O3 $< -o $@
clean:
	rm -f *.i *.o fuse2 fuse2lin repro rep4 \
		*.log *.vg
realclean: clean	
#	
