Simple tests that should be run on VE.
Mostly "proof-of-concept" stuff.

We only rely on a few pieces of libjit1.

---------------------------

jitve_math demonstrates a few optimization "levels" for a simple
operation : add and subtract a JIT constant value.
The test runs for a while, covering many input cases.
If you were happy with the optimizations, you would stick this
into libjit1 as an assembly helper function.

libjit1 now has the fairly complicated case of loading a constant
integer into a scalar register (Notably, the worse case for lea
is 2 ops, rather than 3 in most VE docs, but it also covers faster
loads using logical or arithmetic ops, for example.
