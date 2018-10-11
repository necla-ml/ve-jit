### Consolidate various Aurora/jit/assembler codes...

#### remember to git set user.name "..." and git set user.email "..."

- Some approaches are from low-level asm side:
  - cpp macro hackery
  - pretty-print .S file from C+ code (or from C via macros),
  - convert .S to .bin, load into executable code page,
    and execute it by passing and retrieving arguments
    using [extended] inline asm syntax.
    - Simple examples are *jitve_hello.c* and *jitpp_hello.cpp*
    - there is NO ABI.  Jit code executes in caller's frame,
      so be sure to specify clobbers registers.
    - older C api **ve_util.h**
    - newer C++ api **jitpage.h** + **asmfmt_fwd.hpp**
  - some low-level mask-register tests **msk.cpp** (also dev/ tests)
    - most complicated example was taking 3 packed-float vector registers and
      doing 3x3 transpose of subblocks, left to right.
      of long packed-float vector registers.
      - uses magic macros for left/right-rotate by 1 or 2 of packed float vector
      - and vmrg

- other experiments explore higher-level ideas:
  - **./regs/**
    - symbolic names for registers,
    - register assignment,
    - register spilling/allocation
  - **./jitve_math**
    - various levels of jit-optimizing a *very* simple arithmetic op
      (largely because of different ways a constant can be loaded,
       or because some constants values yield special or even trivial
       optimizations)

- and some tool-like helpers **./loops/**:
  - e.g. take a fused-loop vectorization code that tests
    correctness, and while it runs the test, spits out asm
    [or pseudo-asm, did not have instruction set manuals handy]
  - asm can generate generic loop code,
    - or jit loop-style
    - or jit unrolled versions of output

There are several pieces missing, before a full path from C++
to generated jit kernels is available.

#### Build
##### First set up your ncc/nc++ environment<

some toy projects can be made as:
```
make >& mk.log && echo YAY; \
(cd dev && make >& mk.log && echo YAY2); \
(cd regs && make >& mk.log && echo YAY3);
```

Many codes were developed and can be compiled on x86; however, please use
g++-5.4 or higher, because some 'auto' keyword features are used for
maintainability and robustness to type changes of underlying types.
