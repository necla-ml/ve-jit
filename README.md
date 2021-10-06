### VE JIT support (very alpha!)

We can JIT to 'C' + intrinsics clang code, or to VE assembler.
Currently, libvednn uses the 'C' + intrinsics approach, since it
relieves algorithms from having to hand-allocate registers (much easier).

Since there is no built-in assembler yet, I mostly try to make generated
assembly code fairly readable.  ncc(/nas) with nobjcopy produced executable
blobs.  The JIT pages do not use the 'C' abi, and are called using inline
assembly wrappers to load input registers, and collect output registers.

#### WIP

Jit assembly.  Need to rethink define/undef scopes so as to unify the 'C' and
'asm' producers of JIT code.  I'd like to move the AsmFmtCols approach of
scopes that needs careful tweaking to produce the correct output order to the
Cblock method that builds up a program as a DAG of snippets.  I want to avoid
the current hand-stitching of assembler snippets, where you basically have to
read the output before seeing if it is like you expect.

The following old files need _vel_ updates:
```
[aurora-ds08 jit]$ ack -l _ve_ ./*.{h,hpp,c,cpp}
./timer.h			# OK appears as __ve__
./fuseloop.hpp			# OK appears in comment as _ve_lvl call
./cjitConvFwd_parmstr-vi.c	# I think old demo code - unused?
./cblock.cpp			# demo code only (ignorable)
./libjit1-cxx.cpp		# a concatenation of various .cpp files
./ve_divmod.cpp			# NEEDS UPDATE
./vechash.cpp			# NEEDS UPDATE
```

### Build
##### First set up your ncc/nc++ environment

- many targets have some limited functionality if compiled on x86, without
  any CC=ncc set in your environment
- To make all demos and subdirectories, you really should have CC=ncc (etc.)
  and be able to run VE executables.  Some things will run fine on just x86 though.
- A few items try out clang, or llvm for VE
  - oh, VE llvm + intrinsics don't seem to be on github today (did they move?)
  - a next-gen llvm might be https://github.com/cdl-saarland/rv, but not sure
    about all the rest of the VE llvm packages :(


- build main libs as `make all vejit.tar.gz`
- libjit1 will contain core utilities, libveli is more for testing.
- subdirs ve-asm/ asm/ c/ contain some simple demos/tests
  that use portions of libjit1 codes.
  - These span from very simple example to some more complicated ones,
    and exercise different parts of what is in (or was later put into)
    libjit1 or libveli libraries.
- ncc/nc++ sometimes exit with 'ccom' error.  Just retry.

- VE assembler demos can be found in `ve-asm/` and `asm/`.  These span
  from very simple example to some more complicated ones.

- optimized variable-vector length loop induction [for nested for loops]
  demos can be tried in a subdirectory
```
cd loops
make && ./fuse3 -t 256 100 9 >& x.log
make && ./fuse3 -t 252 100 9 >& y.log
```
  - or try fuse2 to avoid using any particular kernel
  - both mimic a `for(ii=0;ii<100;++ii){ for(jj=0;jj<9;++jj){ KERNEL; }}`
    loop, where vectors of loop indices a[vl] in [0,99] and b[vl] in [0,8] are
    initialized and induced.
    - `x.log` reports strategy is loop unroll 8, with some vector constants
      precalculated is possible, but the unrolled code is TBD for now.
    - `x.log` uses a 2-op trick to replace vector divide-by constant with
      a multiply and shift
      - similar to gcc tricks for scalar division, but modified with reduce validity
        range for VE because of lack of double-wide fixed multiply (i.e. fast VE code
        really wants intermediates are limited to 64-bits).  But this is fine,
        because we are JIT'ing, and know the exact ranges 0..100 and 0..9 will imply.
    - `x.log remarks that vector length 252 allows an alternate vectorization strategy
       (252 begin multiple of 9)
  - compare code suggestions in `x.log` (vector length 256) and `y.log` (vector length 252)
  - making vl a multiple of 9, we have a VE-specific `INDUCE:` that is essentially
    just a single vector add to a[], with b[] being constant.

Some [deprecated?] toy projects can be made as:
```
make >& mk.log && echo YAY; \
(cd dev && make >& mk.log && echo YAY2); \
(cd regs && make >& mk.log && echo YAY3);
```

some tests of things that may or may not work, with shared libraries and
gcc/ncc/clang compilers: `make dllbuild`

ncc >= 2.0 fails during shared library load of libjit1.  This is because
libjit1 includes C++ objects which use iostreams, and there is an illegal
instruction error within library 'init' sections (before _main_) during
`std::ios_base::Init::Init()`

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
  - **./regs/** (not ready to use)
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
    - even unoptimized, one can often avoid avoid integer division.
    - For JIT, range of required division is known and you can
      use a different multiplier than the compiler would use,
      so division is a multiply [, add zero, ] and shift
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
stdout and stderr from "system" calls in jitpipe.hpp.  Without a standalone
VE assembler, I'm regularly invoking things like ncc, clang, nobjdump, etc.
with the "system" call.

Many codes were developed and can be compiled on x86; however, please use
g++-5.4 or higher, because some 'auto' keyword features are used for
maintainability and robustness to type changes of underlying types.

