=== Consolidat various Aurora/jit/assembler codes...

- Some approaches are from low-level asm side:
  - cpp macro hackery
  - pretty-print .S file from C+ code (or from C via macros)
  - convert .S to .bin, load into executable code page,
    and execute it by passing and retrieving arguments
    using [extended] inline asm syntax.
    - there is NO ABI.  Jit code executes in caller's frame,
      so be sure to specify clobbers registers.
  - some low-level mask-register tests
    - most complicated example was 3x3 transpose of subblocks
      of long packed-float vector registers.
      - uses canned seq for left/right-shift by 1 or 2 of packed float vector
      - and vmrg

- others explore higher-level ideas:
  - symbolic names for registers,
  - register assignment,
  - register spilling/allocation
  - various levels of jit-optimizing some simple arithmetic ops
    (largely because of different ways a constant can be loaded,
     or because some constants values yield special or even trivial
     optimizations)

- and (where did I put this?) some tool-like helpers:
  - e.g. take a fused-loop vectorization code that tests
    correctness, and while it runs the test, spits out asm
    [or pseudo-asm, did not have instruction set manuals handy]
  - asm can generate generic loop code,
    - or jit loop-style
    - or jit unrolled versions of output

There are several pieces missing, before a full path from C++
to generated jit kernels is available.

==== remember to git set user.name "..." and git set user.email "..."

<EM>set up ncc/nc++ environment</EM>

some toy projects can be made as:
```
make >& mk.log && echo YAY; \
(cd dev && make >& mk.log && echo YAY2); \
(cd regs && make >& mk.log && echo YAY3);
```
