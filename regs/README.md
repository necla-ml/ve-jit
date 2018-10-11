### Aurora register model

This is development code, testing basic ideas, some of which might not be so good!

- maintain state for scalar, vector, mask registers
- declare scoped symbols
- allocate registers to symbols
- spill old registers to stack frame as required

- initially, probably also want a "no spill" version that
  throws if you run out of registers

C++14 'auto' features are used for readability and robustness.
One usage of a very nice C++17 feature was made less nice to allow this.

##### Handy vim line:

:!make clean && make testSpill >& mk.log && ./testSpill

- output to mk.log (maybe you have compile errors?)
- output to testSpill.vg (runtime errors, assertions?)

### Compilation

###### g++-5.4
``` make -f Makefile.x86 realclean all 2>&1 | tee x86.log ```
will produce *.vg files of valgrind runs

###### nc++
``` make realclean all 2>&1 mk.log ```
will compile binaries and run them via *ve_exec*


