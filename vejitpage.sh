#!/bin/bash
#
# accept .s code from stdin
# assemble it into a temp file (ELF)
# nobjcopy it into a binary blob
# output the binary blob to stdout
#
# Example:
#   cat asmkern1.S | ./vejitpage.sh 2>/dev/null | hexdump -C
# will hexdump equivalent to asmkern1.bin from
#   make -f bin.mk asmkern1.bin
#
# Potential use:
#   PIPE from std::string of code
#    (maybe into C preprocessor)
#    then into vejitpage.sh
#  ** and then right into a std::string of executable code **
#    that you stick into a real executable code page
#    (and then call with a 'C' inline asm wrapper)
#
# See test_naspipe.cpp and naspipe.sh for an example of how to
# pipe input, and get std::string versions of stderr and stdout
# easily using the "pstreams header library" (boost license).
#
# by hand ...
#     nas ./tmp_cpp_3EL8EeME.s -o xx.o; ls -l xx.o; nobjcopy -O binary xx.o xx.bin; ls -l xx.bin; nobjdump -b binary -mve -D xx.bin
#
#
(
OBJ=$(mktemp --tmpdir=. tmp_jit_XXXXXXXX.o) >2;
BIN=$(basename ${OBJ} .o).bin >2;
echo "will use tmp files ${OBJ} ${BIN}" >2;

cpp - | nas - -o ${OBJ} 1>&2;
ls -l ${OBJ} 1>&2;

nobjcopy -O binary ${OBJ} ${BIN} 1>&2;
ls -l ${BIN} 1>&2;
cat ${BIN}; #				---> stdout

rm -f ${OBJ} ${BIN} 1>&2
)
#
