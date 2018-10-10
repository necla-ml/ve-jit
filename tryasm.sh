ncc -DHAVE_CONFIG_H -I. -I.. -I../include -Iinclude -E void_asm_void.S >& vav.ss \
	&& ncc -DHAVE_CONFIG_H -I. -I.. -I../include -Iinclude -g -o vav.o -x assembler-with-cpp -c void_asm_void.S;
cat vav.ss | awk '/^#.*asmnames/{a=a+1} /^#.*void_asm_void.S/{s=s+1} {if(s>2)print}'
nobjdump -D vav.o > vav.dump
