TARGETS:=testSpill testSymScopeUid prt_regnames testSpill2 testSpill3 testRegSym testSymbStates testRegBase
all: $(TARGETS)

VG:=@\# NO-valgrind
CXXFLAGS:=-g2 -O1 -std=c++11 -Wall
asm.o: asm.cpp asm.hpp throw.hpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<

testSpill: testSpill.cpp asm.o
	$(CXX) -o $@ $(CXXFLAGS) $^
#valgrind --leak-check=full --show-leak-kinds=all $@ > $@.vg 2>&1
#@echo "gawking summary..."
#gawk 'BEGIN{err=-1} /^==.*LEAK SUMMARY/{p=1;err=0} p==1&&/^[^=]/{p=0} p==1{print} END{exit err}' \
#	$@.vg || { echo "Ohoh: $@ did not terminate correcly"; \
#	echo "see $@.vg"; }

testSymScopeUid: testSymScopeUid.cpp throw.hpp
	$(CXX) -o $@.i $(CXXFLAGS) -dD -E $<
	$(CXX) -o $@ $(CXXFLAGS) $<
	ve_exec $@ >& $@.vg
#	
#valgrind --leak-check=full --show-leak-kinds=all $@ > $@.vg 2>&1
#@echo "gawking summary..."
#gawk 'BEGIN{err=-1} /^==.*LEAK SUMMARY/{p=1;err=0} p==1&&/^[^=]/{p=0} p==1{print} END{exit err}' \
#	$@.vg || { echo "Ohoh: $@ did not terminate correcly"; \
#	echo "see $@.vg"; }

spill.o: spill.cpp spill.hpp spill-impl.hpp symbStates.hpp throw.hpp regDefs.hpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<

# test based on spillable-base.hpp
testSpill2: testSpill2.cpp spill.cpp spill-impl.hpp spill.hpp throw.hpp regDefs.hpp
	$(CXX) -o $@ $(CXXFLAGS) $(filter %.cpp,$^)
	ve_exec $@ >& $@.vg

regStates.o: regStates.cpp regStates.hpp throw.hpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<

# test based on regSymbol.hpp
testSpill3: testSpill3.cpp spill.o regStates.o spill-impl.hpp spill.hpp throw.hpp regDefs.hpp
	#
	# for(s: randNames) is c++11, but nc++ -std=c++11 does not accept it
	#
	$(CXX) -o $@ $(CXXFLAGS) -strd=c++14 $(filter %.cpp,$^) $(filter %.o,$^)
	ve_exec $@ >& $@.vg

# test based on regSymbol.hpp, register name and register id operations
testRegSym: testRegSym.cpp spill.o regStates.o spill-impl.hpp spill.hpp throw.hpp regDefs.hpp
	$(CXX) -o $@ $(CXXFLAGS) $(filter %.cpp,$^) $(filter %.o,$^)
	ve_exec $@ >& $@.vg

symbStates.o: symbStates.cpp symbStates.hpp throw.hpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<
reg-base.o: reg-base.cpp reg-base.hpp bitfield.hpp throw.hpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<
testSymbStates: testSymbStates.cpp symbStates.o regStates.o reg-base.o spill.o symbStates.hpp spill.hpp throw.hpp
	$(CXX) -o $@ $(CXXFLAGS) $^
	ve_exec $@ >& $@.vg

prt_regnames: prt_regnames.cpp reg-base.o reg-base.hpp bitfield.hpp throw.hpp
	#
	# nc++ -std=c++11 *should* be OK, but
	#   user-defined literal 0b111100111100000000
	#   gives error: user-defined literal operator not found
	# so ask for c++14
	$(CXX) -o $@ $(CXXFLAGS) $(filter %.cpp,$^) $(filter %.o,$^)
	ve_exec $@ >& $@.vg
reg-aurora.o: reg-aurora.cpp reg-aurora.hpp reg-base.hpp bitfield.hpp throw.hpp
	$(CXX) -o $@ -Wall -ggdb -O1 -std=gnu++17 -c $<
testRegBase.asm: testRegBase
	$(CXX) $(CXXFLAGS) -Wa,-adhln $^.cpp > $@
testRegBase: testRegBase.cpp reg-base.o reg-aurora.o reg-base.hpp bitfield.hpp throw.hpp
	$(CXX) -o $@ $(CXXFLAGS) $(filter %.cpp,$^) $(filter %.o,$^)
	ve_exec $@ >& $@.vg

.PRECIOUS: asm.o regStates.o symbStates.o
clean:
	rm -f *.o $(TARGETS) mk*.log bld*.log
realclean: clean
	rm -f $(patsubst %,%.vg,$(TARGETS))
#
