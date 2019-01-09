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
# not so sure about nas/nobjcopy error handling.
#
# pstreams example (tentative)
#	std::string ve_asm_code; // ...
#	std::string machine_code;
#	{
#		using namespace redi;
#		using namespace redi::pstreams;
#		redi::pstream jitcmd("./vejitpage.sh", pstdout|pstdin|pstderr);
#		jitcmd << ve_asm_code << peof; // maybe
#		std::stringstream ss;
#		ss << jitcmd.rdbuf();
#		std::string machine_code = ss.str();
#	}
#
( nas - -o ${vejitpage_ELF:=$(mktemp --tmpdir=.)} 1>&2;
nobjcopy -O binary ${vejitpage_ELF} ${vejitpage_BIN:=$(mktemp --tmpdir=.)} 1>&2;
cat ${vejitpage_BIN};
echo "removing tmp files ELF:${vejitpage_ELF} BIN:${vejitpage_BIN}" 1>&2
rm -f ${vejitpage_ELF} ${vejitpage_BIN} 1>&2
)

  
