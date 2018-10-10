=== Aurora register model

- maintain state for scalar, vector, mask registers
- declare scoped symbols
- allocate registers to symbols
- spill old registers to stack frame as required

==== Handy vim line:

:!make clean && make testSpill >& mk.log && ./testSpill

- output to mk.log (maybe you have compile errors?)
- output to testSpill.vg (runtime errors, assertions?)

