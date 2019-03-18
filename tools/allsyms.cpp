/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#error "This was originally based on wrong code"
//
// https://stackoverflow.com/questions/15779185/list-all-the-functions-symbols-on-the-fly-in-c-code-on-a-linux-architecture
// https://stackoverflow.com/questions/29903049/get-names-and-addresses-of-exported-functions-from-in-linux
//
// https://stackoverflow.com/questions/13319907/how-to-use-libelf-to-generate-an-elf-file-for-my-own-compiler
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <link.h>
#include <dlfcn.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
#define BumpDyn(dyn,tag) do{ \
    for (; dyn->d_tag != DT_NULL; ++dyn) { \
        if (dyn->d_tag == tag) { \
            break; \
        } \
    } \
}while(0)
static const ElfW(Dyn) *
FindTag(const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
    BumpDyn(dyn,tag);
    return dyn->d_tag==tag? dyn: NULL;
}

///////////////////////////////////////////////////////////////////////////////
static size_t
FindVal(const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
    size_t ret = 0;
    BumpDyn(dyn,tag);
    if( dyn->d_tag == tag ) ret = dyn->d_un.d_val;
    else printf(" error: tag %d not found\n",(int)tag); // avoid assert in helpers
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
#if 0
static const void *
FindPtr(const ElfW(Addr) load_addr,
        const ElfW(Dyn) * dyn, const ElfW(Sxword) tag) {
    for (; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == tag) {
            //return (const void *)(dyn->d_un.d_ptr - load_addr); // segfault?
            return (const void *)(dyn->d_un.d_ptr);
        }
    }
    assert(false);
}
#else
    static const void *
FindPtr(const ElfW(Addr) load_addr,
        const ElfW(Dyn)     *dyn,
        const ElfW(Sxword)   tag)
{
    const void *ret = NULL;
    BumpDyn(dyn,tag);
    // note: x86 stores actual pointer value, VE stores relative value?
    if( dyn->d_tag == tag )
#if !defined(__ve)
        ret = (const void*)dyn->d_un.d_ptr;
#else
    ret = (const void*)(load_addr + dyn->d_un.d_ptr);
#endif
    else printf(" error: tag %d not found\n",(int)tag); // avoid assert in helpers
    return ret;
}
#endif

#define UINTS_PER_WORD (__WORDSIZE / (CHAR_BIT * sizeof (unsigned int)))
static ElfW(Word) gnu_hashtab_symbol_count(const unsigned int *const table)
{
    const unsigned int *const bucket = table + 4 + table[2] * (unsigned int)(UINTS_PER_WORD);
    unsigned int              b = table[0];
    unsigned int              max = 0U;

    while (b-->0U)
        if (bucket[b] > max)
            max = bucket[b];

    return (ElfW(Word))max;
}

static void *dynamic_pointer(const ElfW(Addr) addr,
        const ElfW(Addr) base, const ElfW(Phdr) *const header, const ElfW(Half) headers)
{
    if (addr) {
        ElfW(Half) h;

        for (h = 0; h < headers; ++h)
            if (header[h].p_type == PT_LOAD)
                if (addr >= base + header[h].p_vaddr &&
                        addr <  base + header[h].p_vaddr + header[h].p_memsz)
                    return (void *)addr;
    }

    return NULL;
}

/** Callback for dl_iterate_phdr.
 * Is called by dl_iterate_phdr for every loaded shared lib until something
 * else than 0 is returned by one call of this function.
 * ```
 * #include <link.h>
 * #include <string>
 * #include <vector>
 * using namespace std;
 * int main()
 * {
 *     vector<string> symbolNames;
 *     dl_iterate_phdr(retrieve_symbolnames, &symbolNames);
 *     return 0;
 * }
 * ```
 */
int retrieve_symbolnames(struct dl_phdr_info* info, size_t info_size, void* symbol_names_vector)
{
    using namespace std;

    const ElfW(Addr)        base = info->dlpi_addr;
    const ElfW(Phdr) *const header = info->dlpi_phdr;
    const ElfW(Half)        headers = info->dlpi_phnum;
    const char *libpath, *libname;

    /* ElfW is a macro that creates proper typenames for the used system architecture
     * (e.g. on a 32 bit system, ElfW(Dyn*) becomes "Elf32_Dyn*") */
    ElfW(Dyn*) dyn;
    ElfW(Dyn*) dyn_start;
    ElfW(Sym*) sym;
    ElfW(Word*) hash;

    char* strtab = 0;
    char* sym_name = 0;
    ElfW(Word) sym_cnt = 0;

    /* the void pointer (3rd argument) should be a pointer to a vector<string>
     * in this example -> cast it to make it usable */
    vector<string>* symbol_names = reinterpret_cast<vector<string>*>(symbol_names_vector);

    /* Iterate over all headers of the current shared lib
     * (first call is for the executable itself) */
    for (size_t header_index = 0; header_index < headers; ++header_index)
    {

        /* Further processing is only needed if the dynamic section is reached */
        if (header[header_index].p_type == PT_DYNAMIC)
        {

            /* Get a pointer to the first entry of the dynamic section.
             * It's address is the shared lib's address + the virtual address */
            dyn_start = (ElfW(Dyn)*)(base +  header[header_index].p_vaddr);
#define GetTag(tag) FindTag(dyn_start, tag)
#define GetVal(tag) FindVal(dyn_start, tag)
#define GetPtr(tag) FindPtr(load_addr, dyn_start, tag)
#define IterTag(tag) \
    for (const ElfW(Dyn) * dyn = GetTag(tag); dyn; dyn = FindTag(++dyn, tag))
#if !defined(__ve)
#define dynPtr(dyn) (*(const void**)dyn->d_un.d_ptr)
#else
#define dynPtr(dyn) (*(const void**)(load_addr + dyn->d_un.d_ptr))
#endif

            // First pass...
            for(dyn=dyn_start; dyn->d_tag != DT_NULL; ++dyn){
                switch(dyn->d_tag){
                  case(DT_HASH):
                /* Get a pointer to the hash */
                hash = (ElfW(Word*))GetPtr(DT_HASH);
                /* The 2nd word is the number of symbols */
                sym_cnt = hash[1];
            }else{
                hash = (ElfW(Word*))GetPtr(DT_GNU_HASH);
                if(hash){
                    ElfW(Word) count = gnu_hashtab_symbol_count(hash);
                    if(count > sym_cnt) sym_cnt = count;
                }
            }
            // possibly sym_cnt = 0 if no symbol table found


            /* Iterate over all entries of the dynamic section until the
             * end of the symbol table is reached. This is indicated by
             * an entry with d_tag == DT_NULL.
             *
             * Only the following entries need to be processed to find the
             * symbol names:
             *  - DT_HASH   -> second word of the hash is the number of symbols
             *  - DT_STRTAB -> pointer to the beginning of a string table that
             *                 contains the symbol names
             *  - DT_SYMTAB -> pointer to the beginning of the symbols table
             */
            while(dyn->d_tag != DT_NULL)
            {
                if (dyn->d_tag == DT_HASH)
                {
                    /* Get a pointer to the hash */
                    hash = (ElfW(Word*))dyn->d_un.d_ptr;
                    assert(hash!=nullptr);

                    /* The 2nd word is the number of symbols */
                    sym_cnt = (base + hash)[1];

                }
                else if (dyn->d_tag == DT_STRTAB)
                {
                    /* Get the pointer to the string table */
                    strtab = (char*)dyn->d_un.d_ptr;
                }
                else if (dyn->d_tag == DT_SYMTAB)
                {
                    /* Get the pointer to the first entry of the symbol table */
                    sym = (ElfW(Sym*))dyn->d_un.d_ptr;


                    /* Iterate over the symbol table */
                    for (ElfW(Word) sym_index = 0; sym_index < sym_cnt; sym_index++)
                    {
                        /* get the name of the i-th symbol.
                         * This is located at the address of st_name
                         * relative to the beginning of the string table. */
                        sym_name = &strtab[sym[sym_index].st_name];

                        symbol_names->push_back(string(sym_name));
                    }
                }

                /* move pointer to the next entry */
                dyn++;
            }
        }
    }

    /* Returning something != 0 stops further iterations,
     * since only the first entry, which is the executable itself, is needed
     * 1 is returned after processing the first entry.
     *
     * If the symbols of all loaded dynamic libs shall be found,
     * the return value has to be changed to 0.
     */
    return 0;

}
using namespace std;
int nmax = 10;
int main()
{
    auto handle = dlopen("libm.so", RTLD_NOW);
    vector<string> symbolNames;
    dl_iterate_phdr(retrieve_symbolnames, &symbolNames);
    //auto handle = dlopen(NULL, RTLD_LOCAL|RTLD_LAZY);
    int i=0;
    for(auto name: symbolNames){
        auto func = *(void**)dlsym(handle,name.c_str());
        cout<<setw(40)<<name<<" "<<func<<endl;
        if(++i > nmax) break;
    }
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
