### VE JIT support (very alpha!)

I'd like to JIT 'C'+intrinisics, but there are still dynamic loader issues.
So I'm still producing assembly JIT code.

Since there is no built-in assembler yet, I mostly try to make generated assembly
code fairly readable.  ncc(/nas) with nobjcopy produced executable blobs.  The
JIT pages do not use the 'C' abi, and are called using inline assembly wrappers
to load input registers, and collect output registers.

#### Current focus

Jit assembly.  Need to rethink define/undef scopes so as to unify the 'C' and
'asm' producers of JIT code.  I'd like to move the AsmFmtCols approach of scopes
that needs careful tweaking to produce the correct output order to the Cblock
method that builds up a program as a DAG of snippets.  I want to avoid the
current hand-stitching of assembler snippets, where you basically have to
read the output before seeing if it is like you expect.

#### Consolidate various Aurora/jit/assembler codes...
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
  - Issue: for(0..ii) for(0..jj) --> vectors a[] and b[] of indices
    where a[] and b[] are both of high vector length.
  - Optimization:
    - even unoptimized, I can usually rewrite to avoid integer division.
    - but power-of-two or divisibility for jj and VL, or low-period cycles
      can generate near-trivial inductions for a[], b[] index vectors.
  - asm can generate generic loop code,
    - or jit loop-style
    - or jit unrolled versions of output
  - I think this approach will be nice for convolutions.
    - currently I produce non-unrolled asm output
    - I print out the suggested vector length that might make
      the vectorized loop-index register inductions very fast
      - Ex. power-of-two, or vector-length and jj divisible
        or with low-period cyclic update of a[] vector
  - todo:
    - often you don't need a[] and b[], but a few linear combinations,
      and these should be directly calculated during the loop induction
      for best efficiency.

There are several pieces missing, before a full path from C++
to generated jit kernels is available.

#### External codes

pstreams tarball was downloaded.  This header library conveniently helps get
stdout and stderr from "system" calls in jitpipe.hpp.  With poor assembler
support, I'm regularly invoking things like ncc, clang, nobjdump, etc.

#### Build
##### First set up your ncc/nc++ environment

- main libs as 'make all vejit.tar.gz'
- subdirs ve-asm/ asm/ c/ contain some simple demos/tests
  that use portions of libjit1 codes.

some [deprecated?] toy projects can be made as:
```
make >& mk.log && echo YAY; \
(cd dev && make >& mk.log && echo YAY2); \
(cd regs && make >& mk.log && echo YAY3);
```

Many codes were developed and can be compiled on x86; however, please use
g++-5.4 or higher, because some 'auto' keyword features are used for
maintainability and robustness to type changes of underlying types.

#### remember to git set user.name "..." and git set user.email "..."

