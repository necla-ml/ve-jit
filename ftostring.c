/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
	if(argc != 3){
		fprintf(stderr," Usage: ftostring FILENAME VARIABLE"
				"\n --> char const VARIABLE[]={...}"
				"\n and int const VARIABLE_size=...\n");
		return 3;
	}
	char const* const fn = argv[1];
	char const* const array_name = argv[2];
	FILE* f = fopen(fn, "rb");
	printf("\n#ifdef __cplusplus\nextern \"C\" {\n#endif // __cplusplus\n");
	printf("char const %s[] = \n",array_name);
	char const* sep = "{";
	unsigned long n = 0;
	while(!feof(f)) {
		unsigned char c;
		if(fread(&c, 1, 1, f) == 0) break;
		printf("%s 0x%.2X", sep, (int)c);
		++n;
		sep = (n%10? ",": "\n,");
	}
	fclose(f);
	/*printf("%s 0x00",sep); not nec. */
	printf("\n};");
	printf("\nint %s_size = sizeof(bin_mk)/sizeof(char);",array_name);
	printf("\n#ifdef __cplusplus\n}//extern \"C\" {\n#endif // __cplusplus\n");
	return 0;
}
