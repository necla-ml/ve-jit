#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
	assert(argc == 3);
	char const* const fn = argv[1];
	char const* const array_name = argv[2];
	FILE* f = fopen(fn, "rb");
	printf("\n#ifdef __cplusplus\nextern \"C\" {\n#endif // __cplusplus\n");
	printf("\nint %s_size;\n",array_name);
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
	printf("\n%s 0x00 };",sep); /* I will add a terminal null, just in case */
	printf("\nint %s_size=sizeof(bin_mk)/sizeof(char);",array_name);
	printf("\n#ifdef __cplusplus\n}//extern \"C\" {\n#endif // __cplusplus\n");
	return 0;
}
