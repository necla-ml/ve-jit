### loops playground
- very simple prototype for loop fusion.
  - ./fuse2 -h     help
  - ./fuse2 -l     asm pseudocode for vectorized 2-loop induction
  - ./fuse2 -u     [WIP] shows suggestive asm-like output for unrolling loop
    - a first step is to ouput just the unrolling 'plan', while explaining
      the programs reasons for the plan.
    - plan could be things like:
      - full unroll
      - VL <= MVL (for faster inductive updates, particular for all small jj)
      - unroll by N to elide 'iloop%N' in the fused loops (which are vecmul+vecshift)
      - unroll by lowest-common-multiple to pick up low-period repeating
        patterns in some of the loop-index vectors.
        - May be more useful if combined with a special VL<=MVL choice.

- fuse2lin considers calculating a linear combination of vectorized loop indices.
  - It is older code, and needs to be revisited someday.
  - 1st type of optimization for JIT-asm and some JIT-C compilers
    is the scalar-vector mulitplies (ex. multiply-by-{1,-1,2,...})
  - 2nd type of optimzation is to use the inductive loop-index vector
    calcs of 'fuse2' to inductively update the linear combinations.
  - after that, some cases may be able to streamline the inductive
    linear combination calc if the inner kernel does not explicitly
    use the raw index-vector vectors.

- TODO:

1. adapt fuse2 --> cfuse, now that jit/ shows that JIT+nclang (and JIT+ncc) are easy.
   - output can be FOR(i,vl) VECOP style, for run-anywhere reference JIT impls
     - goals: easy to read, understand, nicely commented
   - or actual nclang intrinsics, which only run on VE.
     - goals: fast run times, maybe with extra optimizations to fold into
              the FOR(i,vl)-style demo JIT impls.
2. define an annotation language, so that FOR(i,vl) reference implementations
   can be transformed into fused-loop optimized versions
   Ex.
     //%FUSE2_PRECALC_INDUCTION (mark location for those statements)
     //%FUSE2_INIT_LINEAR_CONSTANT lcvectorname ?
     outer for loops ...
       //%FUSE2_ADD_TO_LINEAR_CONSTANT lcvector ?
         for(...){ //%FUSE2a i=kh ii=KH  --> pre-loop setup, jump into loop etc.
           for(...){ //%FUSE2b j=kw jj=KW
           } //%FUSE2b END
         } //%FUSE2a END
   As well, need a way to mark the alternate kernel code that uses
   the vectorized indices (and avoid the original kh|kw scalar loop indices
   entirely.

2b. Perhaps a better plan is to avoid annotations, but work with
    a standard, runnable version of optimization approaches...

   - first rewrite vednn optimizations in FOR(i,vl) form,
   - then write them with unoptimized 'divmod' loop index vectorization
     to get the inner loop kernels correct.
   - simplify the FOR(i,vl) loops so they can auto translate to VE intrinsics
   - finally auto-transform the "standard" divmod fusions into
     optimized induce-loop (and eventually induced-loop-linear-combination)
     vectorization.
   
