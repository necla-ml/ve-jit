#!/bin/bash
# pipe test that has readable input and output...
#     cat foo.S | <this scripte> > foo.dis
# foo.S ~ VE assembler code
# foo.dis ~ disassembled object file
# (all tmp files removed afterwards)
#
( nas - -o ${vejitpage_ELF:=$(mktemp --tmpdir=.)} 1>&2;
nobjdump -D ${vejitpage_ELF}; # ---> stdout
echo "removing ELF tmp file ${vejitpage_ELF}" >2
rm ${vejitpage_ELF} 1>&2
)
echo "exit seems OK" >2
