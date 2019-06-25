/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "cf3.hpp"
#include "../stringutil.hpp"
#include "../ve_divmod.hpp"
#include <fstream>
//#include <regex>

using namespace std;
using namespace loop;
using namespace cprog;

//#define DBG(WHAT) cout<<" DBG:"<<WHAT<<endl
#define DBG(WHAT)

/** vl0<0 means use |vl0| or a [better] lower alternate VL. */
static struct UnrollSuggest vl_unroll(int const vl0, int const ii, int const jj,
        int const maxun)
{
    int vlen = abs(vl0);
    if(vlen==0) vlen=256;       // XXX MVL not yet standardized in headers!
    int opt_t = (vlen<=0? 3: 0);

    cout<<"vl_unroll("<<vl0<<"-->"<<vlen<<",ii="<<ii<<",jj="<<jj
        <<")opt_t="<<opt_t<<endl;
    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, maxun );
    auto uAlt = unroll_suggest(u);  // suggest an alternate nice vector length, sometimes.
    if(1)
        cout<<"\nUnrolls:"<<str(u,"\nOrig: ")<<endl;
    if(uAlt.suggested==UNR_UNSET)
        cout<<"Alt:  "<<name(uAlt.suggested)<<endl;
    else
        cout<<str(uAlt,"\nAlt:  ")<<endl;
    cout<<endl;

    if(opt_t==3){ // we FORCE the alternate strategy (if it exists)
        if(u.vll) // equiv uAlt.suggested != UNR_UNSET
        {
            assert( uAlt.suggested != UNR_UNSET );
            cout<<" (forcing alt. strategy)"<<endl;
            unroll_suggest(u);
            u = uAlt;
        }else{
            cout<<" (no really great alt.strategy)"<<endl;
        }
    }
    return u;
}
#if 0
    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, maxun );
    int vl = u.vl;
    auto uAlt = unroll_suggest(u);  // suggest an alternate nice vector length, sometimes.

    cout<<"\nUnrolls:"<<str(u,"\nOrig: ")<<endl;
    if(uAlt.suggested==UNR_UNSET) cout<<"Alt:  "<<name(uAlt.suggested)<<endl;
    else cout<<str(uAlt,"\nAlt:  ")<<endl;
    cout<<endl;

    if(opt_t==3){ // we FORCE the alternate strategy (if it exists)
        if(u.vll) // equiv uAlt.suggested != UNR_UNSET
        {
            assert( uAlt.suggested != UNR_UNSET );
            cout<<" (forcing alt. strategy)"<<endl;
            //vl = u.vll; unroll_suggest(u);
            u = uAlt;
            vl = u.vl;
        }else{
            cout<<" (no really great alt.strategy)"<<endl;
        }
    }

    cout<<" cfuse2_unrollX ... u.unroll="<<u.unroll<<endl;
    //if(u.unroll==0) u.unroll=maxun;
#endif
/** worker routine -- loop 'ii' split.
 * vlen<0 means use |vlen| or a better lower alt VL if one is found.
 */
void cf5_unroll_split_ii( Cblock& outer, Cblock& inner, string pfx,
        int32_t const ilo, int32_t const ihi, LoopSplit const& lsjj,
        int const maxun, int const vlen, int const which, int const verbose)
{
    std::ostringstream oss;
    int const ii = ihi - ilo;
    int const jlo = lsjj.z;
    int const jhi = lsjj.end;
    int const jj = jhi - jlo; // jj split TODO
    uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    if(iijj==0){ // equiv. nloop==0
        inner>>OSSFMT("// for(0.."<<ii<<")for(0.."<<jj<<") --> NOP");
        return;
    }
    struct UnrollSuggest u = vl_unroll(vlen,ii,jj,maxun);
    int const vl0 = u.vl;
    int cyc = u.cycle;
    int unroll = u.unroll;
    uint64_t const vl = min(iijj,(uint64_t)vl0);
    assert(vl > 0);

    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)
    auto& root=outer.getRoot().root; // a codeblock where I can add "state" (vl_remember)

    //auto& fd = inner["../first"]; // definitions
    // set up a 'fake' fused-loop scope (which may or may not contain a {..} code block)
    auto& outer_fd = outer["..*/first"];
    outer_fd["first"];
    auto& fd0 = inner["first"];
    auto& fd = inner[pfx]["first"]; // in case loop split differs from fd ???
    auto& fp = inner[pfx]["preloop"];
    auto& cf5 = inner[pfx]["body"];
    auto& fz = inner[pfx]["last"]["krn"];
    //auto& fz = inner["last"]["krn"]; // this might also be a good spot
    fz>>"// ["<<pfx<<" krn output]";

    auto& cf5_defs = cf5;
    cf5_defs .DEF(pfx) .DEF(vl0) .DEF(ilo) .DEF(ii) .DEF(jlo) .DEF(jj) ;

    bool const iifirst = fd0.find("cf5u_iifirst") == nullptr;
    fd0["cf5u_iifirst"].setType("TAG");
    //
    // ------------- helper lambdas ------------
    //
    // remember vl state [inside Cblock tree] between calls to this function
    auto vl_remember = [&root,&oss](int const vl){
        root["save_vl"].setType(OSSFMT(vl));
    };
    auto vl_is = [&root,&oss](int const vl){
        auto pvl = root.find("save_vl");
        return pvl && pvl->getType() == OSSFMT(vl);
    };
    auto vl_str = [&root](){
        return root["save_vl"].getType();
    };

    // some kernels might ask for const sq reg to always be defined
    auto use_sq = [&outer,&outer_fd,&vl_remember,&oss](){
        // if nec, define const sequence register "sq"
        if(!outer.find("have_sq")){
            outer_fd["sq"]>>OSSFMT("_ve_lvl(256);");
            vl_remember(256);
            outer_fd["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = _ve_vseq_v();"<<" // sq[i]=i");
            outer["have_sq"].setType("TAG");
        }
    };
    auto use_sqij = [&fd0,&fd,&use_sq,&oss,&ilo,&jj](){
        use_sq();
        if(!fd.find("have_sqij")){
            if(!fd0.find("sqij")) fd0["sqij"]>>"__vr sqij;";
            string instr, comment;
            if(ilo){
                instr = "sqij = _ve_vaddul_vsv(ilo*jj,sq);";
                comment = " // sqij[i]=i";
            }else{
                instr = "sqij = sq;";
                comment = OSSFMT(" // sqij[i]=i+"<<ilo<<"*"<<jj);
            }
            fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<instr<<comment);
            fd["have_sqij"].setType("TAG");
        }
    };
    auto use_iijj = [&fd0,&fp,&iijj,&ii,&jj,&oss](){
        if(!fp.find("iijj")){
            if(!fd0.find("iijj")) fd0["iijj"]>>"uint64_t iijj;";
            fp["iijj"]>>OSSFMT(left<<setw(40)<<"iijj=(uint64_t)ii*(uint64_t)jj;"
                    <<" // iijj="<<ii<<"*"<<jj<<"="<<iijj);
        }
    };
    auto kernComment = [&ilo,&ihi,&jlo,&jhi,&oss](){
        return OSSFMT("for("<<ilo<<"--"<<ihi<<")for("<<jlo<<"--"<<jhi<<")");
    };
    auto mk_divmod = [&](){ mk_DIVMOD(outer,jj,iijj+vl0); };
    auto use_vl = [&fd0,&fd](){
        if(!fd0.find("decl_vl")){
            fd0["decl_vl"]<<"int64_t vl;";
            fd["last"]["vl"]>>"vl = vl0;";
        }
    };
    //
    // --------- END helper lambdas ------------
    //

    // first call initializes vector length [maybe save/restore?]
    if(iifirst){
        // 'vl_is(vl0)' might cut a useless LVL op
        if(!vl_is(vl0)){
            fd>>OSSFMT("_ve_lvl(vl0);  // VL="<<vl0<<" jj%vl0="<<jj%vl0<<" iijj%vl0="
                    <<iijj%vl0<<" iifirst="<<iifirst);
            vl_remember(vl0);
            assert(vl_is(vl0));
        }
    }else if(!vl_is(vl0)){
        fd>>OSSFMT("_ve_lvl(vl0);  // VL="<<vl0<<" jj%vl0="<<jj%vl0<<" iijj%vl0="
                <<iijj%vl0<<" VL was "<<vl_str());
        vl_remember(vl0);
    }else{
        fd>>OSSFMT("// VL was "<<vl_str()<<", same as vl0 = "<<vl0<<" ?");
    }

    if(unroll==0) { // 'any' small unroll...
        uint32_t un=0U;
        if((uint32_t)nloop<4U) un = nloop;
#if 0 // For extra loop count, try for nPart==0 might remove an embedded exit test
        if(un!=0){
            cout<<"unroll = nloop = "<<nloop<<endl;
        }else{
            for(uint32_t uu=8U; uu>=4U; --uu){ // is there an nPart==0 unroll?
                if(nloop%uu == 0) {un=uu; break;}
            }
        }
#endif
        if(un!=0U){
            cout<<" No-remainder division of nloop="<<nloop<<" with 'any' unroll="<<un<<endl;
        }else{ // no really nice unroll, divide work roughly evenly (uu ~ nPart)
            // I have a standard "work-splitting calc", but where is it? what did I call it? XXX
            un = 8U;
            uint32_t nFull = nloop/un;
            uint32_t nPart = nloop - nFull*un;
            if(nPart){ // loop load-balance: can we increase nPart while keeping nFull the same?
                for(uint32_t uu=un ;uu>=1U; --uu){
                    uint32_t nf = nloop/uu;
                    if( nf != nFull ) break;
                    un = uu;
                }
            }
            cout<<"Work-splitting 'any' unroll="<<un<<endl;
        }
        unroll = un;
        if(unroll > 8) unroll=8;
    }
    string alg_descr=OSSFMT("// "<<pfx<<" unroll "<<unroll
            <<" : vl"<<vl<<kernComment()
            <<" cyc="<<cyc<<" nloop="<<nloop
            <<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0"
                : jj%vl==0? " jj%vl==0": positivePow2(jj)? " jj=2^N" : "")
            <<"cf5_save_vl="<<fd0["cf5_save_vl"].getType());
    assert(unroll>0);
    if(unroll > nloop) unroll = nloop;
    uint32_t nFull = nloop/unroll;
    uint32_t nPart = nloop - nFull*unroll;
    assert(unroll <= nloop);
    assert(nFull > 0);
    alg_descr.append(OSSFMT("-->"<<unroll<<", cyc="<<cyc<<", nFull:nPart="<<nFull<<":"<<nPart));
    DBG(alg_descr);
    fp>>alg_descr;

    //int cycpre_update_ops = 3+(krn_needs.sqij? 1: 0); (or 1 more if jj is a tough divisor)
    //int pow2_update_ops = 3;
    //  actually in case of other ties, perhaps update add(sqij),add(acyc),move,move has excellent op overlap
    //bool const cycpre = (cyc && unroll%cyc == 0 && nFull > 1 && !positivePow2(jj));
    //OH, OH, OH. 
    //  if kernel accepts a string register for a[] b[], then we DO NOT NEED the move,move
    //  and cycpre_update should ALWAYS be done whenever possible
    //OH, OH, OH.
    //  Sometimes we'll have precalc in OUTER loop too, so even nFull:nPart==1:0 can
    //  benefit from register re-use
    bool const cycpre = (cyc>0 && unroll%cyc == 0 && jj<=vl0);
    uint64_t cyc_aincr = cyc*vl0/jj;
    //assert( cyc_aincr*jj == cyc*vl0 || !cycpre ); // if(cycpre)assert(...)
    if( cyc_aincr*jj != cyc*vl0 )
        cout<<" cycpre="<<cycpre<<" cyc_aincr*jj != cyc*vl0 XXX !!! cyc_aincr = "<<cyc_aincr<<endl;
    // - cycpre indicates sufficient loops for precalc
    // - In this case, b[] repeats cyclically,
    //   - while a[] repeats a cycle with increment of cyc*vl0.
    // - fc precalc code calculates the first "cycle" exactly EVENTUALLY
    //   - but for now presubtracts cyc*vl0 from a[] values
    //     in order to avoid a special "first-time-through" loop
    // - precalc update:
    //   - acyc_U += acyc_incr
    //   - copy bcyc_U into b[]
    //   - copy acyc_U into a[]
    //     - 3 ops --> 1 op if can elide the moves (use regs directly in kernel)
    // - divmod is typically add(sqij)-mul-shr-mul[-add]-sub, 5 ops (or 6)
    // - but divmod for power of two jj is {add(sqij)-shr-and} which 3 ops
    //   AND the 'add' gives you sqij "for free" which is good for many kernels.
    // - so DO NOT do cycpre for power of two jj
    if(cycpre){
        assert( jj>1 );
        auto& fc = outer_fd;
        auto& fcp = fc["cycpre"];
        fcp>>"// Cyclic precalc, cyc="<<jitdec(cyc);
        fcp>>"__vr";
        for(uint32_t c=0U; c<cyc; ++c){ fcp<<(c==0U?" acyc_":", acyc_")<<jitdec(c); }
        fcp<<";";
        //fcp>>"__vr const";
        //for(uint32_t c=0U; c<cyc; ++c){ fcp<<(c==0U?" bcyc_":", bcyc_")<<jitdec(c); }
        //fcp<<";";   ... oh these are const so ....

        use_sq();
        for(uint32_t c=0U; c<cyc; ++c){
            string ac = OSSFMT("acyc_"<<c);
            string bc = OSSFMT("bcyc_"<<c);
            if(c==0){
                if( jj==1 ){
                    assert(false); // XXX TODO
                    fcp>>"__vr "<<ac<<" = sq;                   // a[i] = i";
                    fcp>>"__vr "<<bc<<" = _ve_vbrd_vs_i64(0LL); // b[i] = 0"; // libvednn way
                    //fcp>>"__vr "<<bc<<"; "<<bc<<" = _ve_vxor_vvv("<<bc<<","<<bc<<");"; // typically "best way"
                }else if(jj>=vl0){
                    assert(false);
                    assert(jj%vl0==0);
                    fcp>>"__vr "<<ac<<" = _ve_vbrd_vs_i64(0LL); // a[i]=0,b[i]=sq";
                    fcp>>"// update as 'if(++tmod==cyc)++a,b=sq; else b+=vl0;'";
                    //fcp>>equ_sq("__vr "+bc)<<"         // b[i] = i";
                }else{ // note: mk_divmod also optimizes positivePow2(jj) case
                    mk_divmod();
                    uint64_t nCyc = nloop/cyc; // + (c < nloop%cyc? 1: 0);
                    cout<<"unroll%cyc="<<unroll%cyc<<" nloop%cyc="<<nloop%cyc<<" nCyc="<<nCyc<<endl;
                    if( nloop%cyc == 0 ){
                        auto instr=OSSFMT("__vr cycsqij = _ve_vaddul_vsv("<<(nCyc-1)*cyc_aincr*jj<<",sq);");
                        fcp>>OSSFMT(left<<setw(40)<<instr<<" // init at penultimate values");
                    }else{
                        fcp>>"__vr cycsqij = sq;";
                    }
                    auto divmod = OSSFMT("DIVMOD_"<<jj<<"(cycsqij, "<<ac<<", __vr const "<<bc<<");");
                    fcp>>OSSFMT(left<<setw(40)<<divmod<<" // "<<ac<<"[]=cycsqij/"<<jj<<" "<<bc<<"[]=cycsqij%"<<jj);
                }
            }else{
                if( jj==1 ){
                    assert(false); // XXX TODO
                }else if(jj>=vl0){
                    fcp>>"cycsqij = _ve_vaddul_vsv(vl0,cycsqij);";
                    auto divmod = OSSFMT("DIVMOD_"<<jj<<"(cycsqij, "<<ac<<", __vr const "<<bc<<");");
                    fcp>>OSSFMT(left<<setw(40)<<divmod
                            <<" // "<<ac<<"=sq/"<<jj<<" "<<bc<<"=sq%"<<jj);
                }else{ // note: mk_divmod also optimizes positivePow2(jj) case
                    fcp>>"cycsqij = _ve_vaddul_vsv(vl0,cycsqij);";
                    auto divmod = OSSFMT("DIVMOD_"<<jj<<"(cycsqij, "<<ac<<", __vr const "<<bc<<");");
                    fcp>>OSSFMT(left<<setw(40)<<divmod
                            <<" // "<<ac<<"=sq/"<<jj<<" "<<bc<<"=sq%"<<jj);
                }
            }
        }
        if(nloop%cyc){ // slower
            // precalc to "final-unroll" values, less one cyc_aincr
            for(uint32_t c=0U; c<cyc; ++c){
                string ac = OSSFMT("acyc_"<<c);
                uint64_t nCyc = nloop/cyc + (c < nloop%cyc? 1: 0);
                auto instr = OSSFMT(ac<<" =_ve_vaddul_vsv("<<(nCyc-1U)*cyc_aincr<<", "<<ac<<");");
                fcp>>OSSFMT(left<<setw(40)<<instr);
                if(c==0U) fcp<<OSSFMT(" // nloop="<<nloop/cyc<<"*(cyc="<<cyc<<")+"<<nloop%cyc<<", cyc_aincr="<<cyc_aincr);
            }
        }
        fcp>>"// Cyclic precalc END";
        // reset from "final-unroll" values to initial ones
        for(uint32_t c=0U; c<cyc; ++c){
            string ac = OSSFMT("acyc_"<<c);
            uint64_t nCyc = nloop/cyc + (c < nloop%cyc? 1: 0);
            auto instr = OSSFMT(ac<<" =_ve_vaddul_vsv(-"<<nCyc*cyc_aincr<<", "<<ac<<");");
            fp>>OSSFMT(left<<setw(40)<<instr);
            if(c==0U) fp<<OSSFMT(" // nloop="<<nloop/cyc<<"*(cyc="<<cyc<<")+"<<nloop%cyc<<", cyc_aincr="<<cyc_aincr);
        }
    }

    // fp, pre-loop setup: declare or calc a[],b[]?
    //     - sometimes init calculation is easier than generic calc/induce
    bool fp_sets_ab = (nloop==1
            || (nloop>1 && (vl0<jj || vl0%jj==0)));
    // emit declarations [if iifirst] and init a[] b[] registers
    if(fp_sets_ab){
        use_sq();
        auto& cb = (iifirst? fd: fp);
        std::string vREG=(iifirst?"__vr ":"");
        if( jj==1 ){
            // for loop split, don't easily know if we can 'const' it
            string instr;
            if(ilo==0) instr = OSSFMT(vREG<<"a = sq;");
            else       instr = OSSFMT(vREG<<"a = _ve_vaddul_vsv("<<ilo<<",sq);");
            cb>>OSSFMT(left<<setw(40)<<instr<<" // a[i] = ilo+i");
            cb>>OSSFMT(left<<setw(40)<<vREG+"b = _ve_vbrd_vs_i64(0LL);"<<" // b[i] = 0");
            //cb>>vREG<<"b; b=_ve_vxor_vvv(b,b);"; // typically "best way"
        }else if(jj>=vl0){
            auto instr=OSSFMT(vREG<<"a = _ve_vbrd_vs_i64("<<ilo<<");");
            cb  >>OSSFMT(left<<setw(40)<<instr<<" // a[i] = "<<ilo)
                >>OSSFMT(left<<setw(40)<<vREG+"b = sq;"<<" // b[i] = i");
        }else{ // note: mk_divmod also optimizes positivePow2(jj) case
            mk_divmod();
            use_sq();
            if(iifirst) cb>>"__vr a,b;";
            string divmod;
            if(ilo==0){
                divmod = OSSFMT("DIVMOD_"<<jj<<"(sq, a, b);");
                cb>>OSSFMT(left<<setw(40)<<divmod<<" // a[]=sq/"<<jj<<" b[]=sq%"<<jj);
            }else{
                divmod = OSSFMT("DIVMOD_"<<jj<<"(_ve_vaddul_vsv(ilo*jj,sq), a, b);");
                cb>>OSSFMT(left<<setw(40)<<divmod<<" // a[]=(sq+"<<ilo*jj<<")/"<<jj<<" b[]=(sq+"<<ilo*jj<<")%"<<jj);
            }
        }
    }else{
        if(iifirst) fd>>"__vr a,b;";
    }

    auto krn_needs = kernel_needs(which);
    //if(krn_needs.vl) use_vl();
    if(krn_needs.sq) use_sq();
    if(krn_needs.sqij) use_sqij();
    if(krn_needs.iijj) use_iijj();
    if(krn_needs.cnt){
        auto instr=OSSFMT((iifirst?"int64_t ":"")<<"cnt = 0;");
        fp  >>OSSFMT(left<<setw(40)<<instr
                <<" // i-loop 0.."<<ii<<" ilo,ihi="<<ilo<<","<<ihi);
        fd["have_cnt"].setType("TAG");
    }
    // krn_needs.cnt also gets some more treatement
    if(nloop==1){
        auto& fk = cf5["once"];
        if(krn_needs.cnt){
            fk["last"]>>"cnt=vl0;";
        }
        cf5_kernel(fk,fd,fz, ilo,ii,jlo,jj,vl, kernComment(), which,
                pfx,0,"a","b","sq","vl0");
    }else if(nloop>1){
        //CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; cnt<iijj; cnt+=vl0)",cf4);
        //--------------------------------------- cnt_update; cnt_test
        string cnt_update;
        string cnt_test;
        int pragma_unr=0;
        string loop_ab_string;
        use_iijj(); // iijj is ALWAYS used somewhere in loop_ab
        if(krn_needs.cnt){ // we require fwd-cnt scalar: 0, vl0, 2*vl0, ...
            fp>>"int64_t cnt=0; // 0..iijj-1";
            cnt_test="cnt < iijj";
            cnt_update="cnt+=vl0;";
            if(unroll==nloop){ // nFull==1 && nPart==0
                pragma_unr=-1; // absent
                loop_ab_string="";
            }else if(nPart==0){ // includes case cyc>0 && unroll%cyc==0
                loop_ab_string=OSSFMT("for(/*int64_t cnt=0*/; "<<cnt_test<<"; /*"<<cnt_update<<"*/)");
            }else{
                loop_ab_string=OSSFMT("for(/*int64_t cnt=0*/; /*"<<cnt_test<<"*/; /*"<<cnt_update<<"*/)");
            }
            fd["have_cnt"].setType("TAG");
        }else{ // downward count gives easier final-vl calc
            cnt_test="cnt > 0";
            cnt_update="cnt-=vl0;";
            if(unroll==nloop){ // nFull==1 && nPart==0
                pragma_unr=-1; // absent
                loop_ab_string="";
                cnt_update="";
            }else{
                loop_ab_string=OSSFMT("for(int64_t cnt=iijj ; "<<cnt_test<<"; /*"<<cnt_update<<"*/)");
            }
        }
        CBLOCK_FOR(loop_ab,pragma_unr, loop_ab_string, cf5);
        //auto& ff = loop_ab["../first"];
        //auto& fk = loop_ab["krn"];

        auto comment=OSSFMT("// nloop="<<nloop<<" unroll by "<<unroll
                <<" --> "<<nFull<<" times + "<<nPart<<" remainder");
        cout<<comment<<endl;;
        loop_ab<<comment;
        auto maybe_exit = [&cnt_test,&oss](Cblock& cb, bool const doit){
            if(doit){
                auto instr = OSSFMT("if(!("<<cnt_test<<")) break;");
                cb>>OSSFMT(left<<setw(40)<<instr<<" // EXIT TEST");
            }
        };

#if 0
        // ff
        if(iijj%vl0){ // last iter has reduced VL
            use_vl();
            auto have_cnt = fd.find("have_cnt");
            if(have_cnt) use_iijj();
            string instr = (have_cnt
                    ?"vl = (vl0<iijj-cnt? vl0: iijj-cnt);"
                    :"vl = (cnt<vl0? cnt: vl0);");
            ff  >>OSSFMT(left<<setw(40)<<instr<<" // iijj="<<iijj/vl0<<"*vl0+"<<iijj%vl0)
                >>"_ve_lvl(vl);";
        }
        if(!fp_sets_ab){
            use_sqij();
            mk_divmod();
            auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
            ff>>OSSFMT(left<<setw(40)<<divmod
                    <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
        }

        // fk
        cf5_kernel(fk, fd, fz, ilo,ii,jlo,jj,vl, kernComment(), which,
                pfx,0,"a","b","sq",(fd0.find("decl_vl")?"vl":"vl0"));

        // manage a,b induction, vl change (loop exit?)
        auto& fi = loop_ab["iter"];
        //fi>>OSSFMT("// cf5_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
        if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
            int64_t vlojj = vl0/jj;
            cf5_defs.DEF(vlojj);
            fi  >>OSSFMT(left<<setw(40)<<"a =_ve_vadduw_vsv(vlojj, a);"
                    <<" // a[i]+="<<vl0/jj<<", b[] same");
        }else if(jj%vl0==0){
            if(nloop<=jj/vl0){ // !have_jjMODvl_reset
                auto instr = OSSFMT("b = _ve_vadduw_vsv("<<vl0<<",b);");
                fi>>OSSFMT(left<<setw(40)<<instr<<" // b[] += vl0, a[] const");
            }else{ // various nice ways to do periodic reset... [potentially better with unroll!]
                if(!fp.find("tmod"))
                    fp["tmod"]>>"uint32_t tmod=0U; // loop induction periodic";
                if(jj/vl0==2){
                    fi>>OSSFMT(left<<setw(40)<<"tmod = ~tmod;"<<" // toggle");
                }else if(positivePow2(jj/vl0)){
                    uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                    uint64_t const mask  = (1ULL<<shift) - 1U;
                    auto instr = OSSFMT("tmod = (tmod+1) & "<<jithex(mask)<<";");
                    fi>>OSSFMT(left<<setw(40)<<instr<<" // cyclic power-of-2 counter");
                }else{
                    fi>>OSSFMT(left<<setw(40)<<"++tmod;"
                            <<" // tmod period jj/vl0 = "<<jj/vl0);
                    auto instr = OSSFMT("if(tmod=="<<jj/vl0<<") tmod=0;");
                    fi>>OSSFMT(left<<setw(40)<<instr<<" // cmov reset tmod=0?");
                }
                use_sq();
                fi  >>"if(tmod){" // using mk_scope or CBLOCK_SCOPE is entirely optional...
                    >>"    b = _ve_vadduw_vsv(vl0,b);           // b[] += vl0 (easy, a[] unchanged)"
                    >>"}else{"
                    >>"    a = _ve_vadduw_vsv(1,a);             // a[] += 1"
                    >>"    b = sq;                              // b[] = sq[] (reset case)"
                    >>"}";
            }
        }else{
            use_sqij();
            mk_divmod();
            string instr = "sqij = _ve_vaddul_vsv(vl0,sqij);";
            fi>>OSSFMT(left<<setw(40)<<instr<<" // sqij[i] += "<<vl0);
            if(fp_sets_ab){
                auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
                cout<<divmod<<endl;
                fi>>OSSFMT(left<<setw(40)<<divmod
                        <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
            }
        }
#endif
        // if nPart, then FULL loops do not need to check VL
        // In general, need to check VL when u==nloop%unroll
        for(uint32_t u=0U; u<(uint32_t)unroll; ++u){
            DBG("\nUN"<<u<<" of "<<unroll);
            bool const last_iter_check = (u == (nPart+(uint32_t)unroll-1U)%(uint32_t)unroll);
            bool const extra_exit_test = (last_iter_check && u!=(uint32_t)unroll-1U);
            string ac("aOOPS_OOPS"), bc("bOOPS_OOPS");
            if(cycpre){
                ac = OSSFMT("acyc_"<<u%cyc);
                bc = OSSFMT("bcyc_"<<u%cyc);
            }
            auto& fu = loop_ab[OSSFMT("unr"<<u)];
            fu<<OSSFMT("                /*UN"<<u
                    <<(last_iter_check && loop_ab_string.empty()?", last iter"
                        :last_iter_check?", possibly last iter"
                        :"")
                    <<"*/");

            auto& ff = fu["first"];
            auto& fk = fu["krn"];

            // ff
            if(!fp_sets_ab && cycpre){ // this MUST be before VL check, in this case
                auto instr=OSSFMT(ac<<" = _ve_vaddul_vsv("<<cyc_aincr<<", "<<ac<<");");
                ff>>OSSFMT(left<<setw(40)<<instr<<" // a~"<<ac<<", b~"<<bc);
            }
            if(last_iter_check && iijj%vl0 ){ // last iter has reduced VL
                if(nloop == unroll){
                    auto final_vl = iijj % vl0;
                    auto instr=OSSFMT("_ve_lvl("<<(fd.find("have_vl")?"vl=":"")<<final_vl<<");");
                    ff>>OSSFMT(left<<setw(40)<<instr<<" // iijj="<<iijj/vl0<<"*vl0+"<<final_vl);
                }else{ // must check whether, this time through, vl changes
                    use_vl();
                    ff  >>OSSFMT(left<<setw(40)<<(fd.find("have_cnt")
                                ?"vl = (vl0<iijj-cnt? vl0: iijj-cnt);"
                                :"vl = (cnt<vl0? cnt: vl0);")
                            //<<"// vl = min(vl0,remain)"
                            <<" // iijj="<<iijj/vl0<<"*vl0+"<<iijj%vl0
                            );
                    ff>>"_ve_lvl(vl);";
                }
            }
            if(fp_sets_ab){
                mk_divmod();
                use_sqij();
            }else{ //!fp_sets_ab // if no pre-loop a,b calc, do it right before kernel call
                mk_divmod();
                if(!cycpre){ // recalc usually can be after VL reduction check
                    use_sqij();
                    auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
                    ff>>OSSFMT(left<<setw(40)<<divmod<<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj<<" BBB");
                }
            }

            // fk
            fuse2_kernel(fk, fd, fz, ii,jj,vl, kernComment(), which,
                    OSSFMT("fuse2_unr"<<(cycpre?"cyc":""))/*pfx*/, 1/*verbose*/,
                    (cycpre?ac:"a"), (cycpre?bc:"b"), "sq", "vl");

            // fi (induce)
            auto& fc = loop_ab[OSSFMT("unr"<<u)]["cnt"];
            if(!cnt_update.empty()) fc>>cnt_update; // cnt = cnt +/- vl0;
            maybe_exit(fc,extra_exit_test);
            // if no for-loop and last unroll (u==nloop), then squash the useless terminal induction
            bool const loop_induce = (unroll<nloop || (unroll==nloop && u<(uint32_t)unroll-1U));
            if(loop_induce){
                auto& fi = loop_ab[OSSFMT("unr"<<u)]["iter"];

                if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
                    int64_t vlojj = vl0/jj;
                    if(!cf5_defs.find("have_vlojj")){
                        cf5_defs.DEF(vlojj);
                        cf5_defs["have_vlojj"].setType("TAG");
                    }
                    fi  >>OSSFMT(left<<setw(40)<<"a =_ve_vadduw_vsv(vlojj, a);"
                            <<" // a[i]+="<<vlojj<<", b[] same");
                }else if(jj%vl0==0){
                    if(nloop<=jj/vl0){ // !have_jjMODvl_reset
                        auto instr = OSSFMT("b = _ve_vadduw_vsv("<<vl0<<",b);");
                        fi>>OSSFMT(left<<setw(40)<<instr<<" // b[] += vl0, a[] const");
                    }else{ // various nice ways to do periodic reset... [potentially better with unroll!]
                        // Every (jj/vl0) we do a special reset...
                        use_sq();
                        // cyclic unroll can inline the "tmod" periodic reset
                        if(cyc && (unroll%cyc==0 || (nFull==1 && nPart==0))){ // other corner cases XXX ???
                            if((u+1)%cyc){
                                fi  >>"b = _ve_vadduw_vsv(vl0,b);           // b[] += vl0 (cyc="<<jitdec((u+1)%cyc)<<", a[] unchanged)";
                            }else{
                                fi  >>"a = _ve_vadduw_vsv(1,a);             // a[] += 1"
                                    >>"b = sq;                              // b[] = sq[] (cyc reset)";
                            }
                        }else{ //generic update explicitly tracks a separate 'tmod' cyclic counter
                            if(!fp.find("tmod"))
                                fp["tmod"]>>"uint32_t tmod=0U; // loop induction periodic";
                            if(jj/vl0==2){
                                fi>>OSSFMT(left<<setw(40)<<"tmod = ~tmod;"<<" // toggle");
                            }else if(positivePow2(jj/vl0)){
                                uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                                uint64_t const mask  = (1ULL<<shift) - 1U;
                                auto instr = OSSFMT("tmod = (tmod+1) & "<<jithex(mask)<<";");
                                fi>>OSSFMT(left<<setw(40)<<instr<<" // cyclic power-of-2 counter");
                            }else{
                                fi>>OSSFMT(left<<setw(40)<<"++tmod;"<<" // tmod period jj/vl0 = "<<jj/vl0);
                                auto instr=OSSFMT("if(tmod=="<<jj/vl0<<") tmod=0;");
                                fi>>OSSFMT(left<<setw(40)<<instr<<" // cmov reset tmod=0?");
                            }
                            fi  >>"if(tmod){" // using mk_scope or CBLOCK_SCOPE is entirely optional...
                                >>"    b = _ve_vadduw_vsv(vl0,b);           // b[] += vl0 (easy, a[] unchanged)"
                                >>"}else{"
                                >>"    a = _ve_vadduw_vsv(1,a);             // a[] += 1"
                                >>"    b = sq;                              // b[] = sq[] (reset case)"
                                >>"}";
                        }
                    }
                }else if(0 && positivePow2(jj)){
                    // code block identical to full recalc DIVMOD case
                    // (mk_divmod now recognizes jj=2^N)
                    // both mul-shr and jj=2^N cases can use DIVMOD macro
                    //
                    // induction from prev a,b is longer than full recalc!
                    //  sqij = _ve_vaddul_vsv(vl0,sqij);
                    //  a = _ve_vsrl_vvs(sqij, jj_shift);              // a[i]=sq[i]/jj
                    //  b = _ve_vand_vsv("<<jithex(jj_minus_1)<<",sq); // b[i]=sq[i]%jj
                }else{
                    if(fp_sets_ab){ // if pre-loop sets ab, recalc goes to fi (not ff)
                        auto instr = "sqij = _ve_vaddul_vsv(vl0,sqij);";
                        fi>>OSSFMT(left<<setw(40)<<instr<<" // sqij[i] += "<<vl0);
                        auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
                        fi  >>OSSFMT(left<<setw(40)<<divmod
                                <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj<<" AAA");
                    }else if(!cycpre){
                        auto instr = "sqij = _ve_vaddul_vsv(vl0,sqij);";
                        fi>>OSSFMT(left<<setw(40)<<instr<<" // sqij[i] += "<<vl0);
                    }
                }
            }
        }
    }
    if(nloop>1 && iijj%vl0){
        // so that a subsequent call to this routine knows whether VL is still OK
        vl_remember(iijj%vl0);
    }else{
        vl_remember(vl0);
    }
    fz>>"// "<<pfx<<" done";
}

/** helper routine, after using unroll_suggest for a good VL \c vl0.
 * vl00<0 means use |vl00| or a shorter alternate VL if there's a good one.
 */
std::string cf5_unroll(int const vl00, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const maxun, int const which/*=WHICH_KERNEL*/, int const verbose/*=1*/)
{
    ostringstream oss;

    // upper-level tree structure, above the fused-loop.
    Cunit pr("cf5_unroll","C",0/*verbose*/);
    auto& inc = pr.root["includes"];
    inc >>"#include \"veintrin.h\""
        >>"#include <stdint.h>";
    inc["stdio.h"]>>"#include <stdio.h>";

    // create a somewhat generic tree
    auto& fns = pr.root["fns"];
    fns["first"]; // reserve a node for optional function definitions
    CBLOCK_SCOPE(cfuse2,"int main(int argc,char**argv)",pr,fns["main"]);
    // NEW: use tag function scope, for upward-find operations
    //      will require extending the find string to match tag!
    cfuse2.setType("FUNCTION");

    // example 'outer loops'
    cfuse2>>"int const nrep = 3;";
    CBLOCK_FOR(loop1,-1,"for(int iloop1=0; iloop1<nrep; ++iloop1)",cfuse2);
    CBLOCK_FOR(loop2,-1,"for(int iloop2=0; iloop2<1; ++iloop2)",loop1);

    // first, establish where our "enclosing function" is located.
    // we SHOULD try to do this as generically as we can!
    auto& outer = cfuse2;
    // now establish our local scope, where our fused-loops live
    auto& inner = loop2;
    inner>>"printf(\"inner loop @ (%d,%d)\\n\",iloop1,iloop2);";
    inner["first"]; // --> 'fd' inner setup code used by all splits

    Lpi const ii = lsii.end - lsii.z;
    Lpi const jj = lsjj.end - lsjj.z;
    uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    if(iijj==0){ // equiv. nloop==0;
        inner>>OSSFMT("// for(0.."<<ii<<")for(0.."<<jj<<") --> NOP");
        return pr.str();
    }

    if(lsii.lo > lsii.z){
        string pfx="f5a";
        cf5_unroll_split_ii(outer,inner,pfx, lsii.z,lsii.lo,lsjj,
                maxun, vl00, which, verbose);
    }
    if(lsii.hi > lsii.lo){
        string pfx="f5b";
        cf5_unroll_split_ii(outer,inner,pfx, lsii.lo, lsii.hi, lsjj,
                maxun, vl00, which, verbose);
    }
    if(lsii.end > lsii.hi){
        string pfx="f5c";
        cf5_unroll_split_ii(outer,inner,pfx, lsii.hi, lsii.end, lsjj,
                maxun, vl00, which, verbose);
    }

    if(1){
        cout<<"Tree:\n";
        pr.dump(cout);
        cout<<endl;
    }
    return pr.str();
}

std::string cf5_unrollX(Lpi const vlen, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const maxun, int const opt_t,
        int const which/*=WHICH_KERNEL*/, char const* ofname/*=nullptr*/)
{
    ostringstream oss;
    string author=OSSFMT(" cf5_unrollX(vlen="<<vlen<<","<<lsii<<","<<lsjj<<",...)");
    cout<<author<<endl;

    string program = cf5_unroll((opt_t==3?(int)vlen:-(int)vlen),
            lsii,lsjj,maxun,which,1/*verbose*/);
    // wrap 'program' up with some boilerplate...
    oss<<"// Autogenerated by "<<__FILE__
        <<"\n// "<<author
        <<"// Possible compile:\n"
        <<"//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c"<<endl;
    oss<<program;
    oss<<"// vim: ts=2 sw=2 et cindent\n";
    // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"
    program = oss.str();

    cout<<program<<endl;
    
    if(ofname!=nullptr){
        ofstream ofs(ofname);
        ofs<<program;
        ofs.close();
        cout<<"// Written to file "<<ofname<<endl;
    }
    return program;
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
