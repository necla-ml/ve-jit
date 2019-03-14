#include <stdio.h>
#include <dlfcn.h>

/** print a prefix stating where we think we're running */
static void pfx_where(){
#if defined(__ve)
    printf("Running on VE   ... ");
#else
    printf("Running on HOST ... ");
#endif
}

int main(int argc,char**argv){
	void *lib_handle;
    char const* fullpath = "/full/path/to/library";
    pfx_where();
    lib_handle = dlopen(fullpath, RTLD_LAZY);
    if(!lib_handle){
        printf("EXCELLENT, dlopen(\"%s\",RTLD_LAZY) failed\n",
                fullpath);
        return 0;
    }
    printf("Ohoh, dlopen(\"%s\",RTLD_LAZY) return library handle %p\n",
            fullpath, lib_handle);
    return 1;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
