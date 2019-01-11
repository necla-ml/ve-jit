/** \file wrappers to call VE jit pages with test logic */
#if !defined(__ve)
#warning "Empty, since requires ncc to compile VE inline asm"
#else

#include "velogic.hpp"
#include "regs/throw.hpp"

/** wrpiFoo calls will claim to clobber %s0--7, %s40--49 */
#define VECLOBBER "%s12", \
    "%s0","%s1","%s2", \
"%s3", \
"%s4", \
"%s5", \
"%s6", \
"%s7", \
"%s40", \
"%s41", \
"%s42", \
"%s43", \
"%s44", \
"%s45", \
"%s46", \
"%s47", \
"%s48", \
"%s49"
        
#include <iostream>
using namespace std;

VeliErr     wrpiLoadreg(JitPage *page, uint64_t start, uint64_t count/*=1U*/) {
    if( page==nullptr ) THROW("page is null, definitely bad to call that location");
    VeliErr e;
    for(uint64_t s=start; s!=count; ++s){
        e.i = s;
        asm( //"\tlea %s0,0x666\n"   /*help find this place in asm*/
             "\tlea %s0,  (,%[arg])\n"
             "\tlea %s12, (,%[page])\n"
             "\tbsic %lr,(,%s12)\n"
             "\tor %[error], 0,%s3\n"   /* retrieve kernel error code (code path?) */
             "\tor %[other], 0,%s5\n"   /* return optional extra info */
             :[error]"=r"(e.error), [other]"=r"(e.other)
             :[arg]"r"(s), [page]"r"(page->mem)
             :VECLOBBER
           );
        if( e.error ){
            cout<<" "<<__PRETTY_FUNCTION__<<" "<<e
            break;
    }
    return e;
}

#endif // __ve only code
