TARGETS=msk1 msk2 msk4 msk3 
all: $(TARGETS)
CFLAGS:=-O2 -g2
CXXFLAGS:=$(CFLAGS) -std=c++11
CC?=ncc
CXX?=nc++
#CC:=ncc-1.5.1
#CXX:=nc++-1.5.1
#CC:=ncc-1.2.4
#CXX:=nc++-1.2.4
#CC:=ncc-1.5.2
#CXX:=nc++-1.5.2
msk3: msk3.cpp
	$(CXX) -O0 -E $< > msk3.i
	$(CXX) -O0 -Wa,adhln -S $< -o msk3.S >& msk3.log
	$(CXX) -O0 -g2 $< -o $@ >& msk3.log
	nobjdump -d $@ >& msk3.dis
	ve_exec ./$@ 2>&1 | tee msk3.log
msk%: msk%.cpp
	$(CXX) $(CFLAGS) -E $< > msk$*.i
	$(CXX) $(CFLAGS) -Wa,adhln -S $< -o msk$*.S
	$(CXX) $(CFLAGS) -g2 $< -o $@
	nobjdump -d $@ >& msk$*.dis
	ve_exec ./$@ 2>&1 | tee msk$*.log
clean:
	rm -f *.o *.i *.ii *.out
	rm -f msk*.i msk*.S msk*.dis msk*.out
realclean: clean
	rm -f $(TARGETS) msk*.log
