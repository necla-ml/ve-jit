/** \file wrappers to call VE jit pages with test logic */
#if !defined(__ve)
#warning "Empty, since requires ncc to compile VE inline asm"
#else

#include "velogic.hpp"
#include "regs/throw.hpp"
#include <cassert>

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

VeliErr     wrpiLoadreg(JitPage *page)
{
    if( page==nullptr ) THROW("page is null, definitely bad to call that location");
    VeliErr e={0,0,0,0};
    // prgiLoadreg returns JitPage code that has:
    // - INP : no args
    // - OUT : sets %s3 (and no other regs)
    // - MAYBE clobbers tmp T0 = %s40 
    asm( //"\tlea %s0,0x666\n"   /*help find this place in asm*/
            "\tlea %s12, (,%[page])\n"
            "\tbsic %lr,(,%s12)\n"
            "\tor %[out], 0,%s3\n"   /* return optional extra info */
            :[out]"=r"(e.output)
            :[page]"r"(page->mem)
            :"%s3","%s40", "%s12"
       );
    return e;
}

/** for ```veli_loadreg -R``` testing, whose JitPage uses a computed goto to
 * switch between the (about 19 thousand) predefined test cases.
 * \return \c VeliErr with i, error and output for test case i==testnum. */
VeliErr     wrpiLoadregBig(JitPage *page, uint64_t const testnum)
{
    if( page==nullptr ) THROW("page is null, definitely bad to call that location");
    VeliErr e={0,0,0,0};
    e.i = testnum;
    // tmp_veli_loadreg_big code has:
    // - IN[%s0]    test number
    // - BP[%s1]    clobbered base address register
    // - ERR[%s2]   error output
    // - OUT[%s3]   test output
    // - T0[%s40]   clobbered tmp register
    // It is implemented as a computed goto into a table of fixed-size
    // code chunks, each of which sets OUT and returns here.
    // ERR is set if testnum is out-of-range.
    assert( page->mem != 0 );
    cout<<" wrpiLoadregBig("<<(void*)(page->mem)<<","<<testnum<<")"<<endl;
    asm( //"\tlea %s0,0x666\n"   /*help find this place in asm*/
            "\tlea %s0,  (,%[arg])\n"
            "\tlea %s12, (,%[page])\n"
            "\tbsic %lr,(,%s12)\n"
            "\tor %[err], 0,%s2\n"
            "\tor %[out], 0,%s3\n"
            "\tor %[out2], 0,%s4\n"
            :[err]"=r"(e.error), [out]"=r"(e.output), [out2]"=r"(e.other)
            :[arg]"r"(testnum), [page]"r"(page->mem)
            :"%s0", "%s1", "%s2", "%s3", "%s40", "%s12"
       );
    // we do not interpretation of e.error or e.output here
    return e;
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // __ve only code
