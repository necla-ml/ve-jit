Files that target VE assembler, but might also just produce code strings when
run on x86.

So you can get things looking right on your desktop, before seeing it the
assembly code runs for real on VE.

jitpp_loadreg.cpp is the most complex case, but testing 19000+ test cases
efficiently really needs to avoid 19000 binary blobs... Somewhere else, I
generate a single JIT page with all the tests that begins with calculated
branch address to the test you want to run (so you get a single binary
blob of 400-500k or so, and call it many times).
