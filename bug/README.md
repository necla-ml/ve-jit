VE linker has some problems with C++ library init functions.

These affect me even before dlopen/dlsym issues appear with JIT'ed C code.

It would be ideal if JIT 'C'+intrinsics code could be compiled with clang and
the the JIT .so loaded via dlopen/dlsym.

This is on hold for now, until the VE dynamic linker starts behaving nicely.
