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

static bool tag_once(Cblock &cb, std::string const& sub){
    Cblock *found = cb.find(sub);
    if(!found) cb[sub].setType("TAG");
    return found==nullptr;
}

/** vl0<0 means use |vl0| or a [better] lower alternate VL. */
static struct UnrollSuggest vl_nounroll(int const vl0, int const ii, int const jj)
{
    int vlen = abs(vl0);
    if(vlen==0) vlen=256;       // XXX MVL not yet standardized in headers!
    cout<<"vl_nounroll("<<vl0<<"-->"<<vlen<<",ii="<<ii<<",jj="<<jj<<endl;

    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, -8/*b_period_max+no_unroll*/ );
    cout<<"vl0="<<vl0<<" unroll_suggest("<<vlen<<","<<ii<<","<<jj
        <<",8) --> vl="<<u.vl<<", vll="<<u.vll<<endl;
    // suggest an alternate nice vector length, sometimes.
    auto uAlt = unroll_suggest(u);
    cout<<"vl0="<<vl0<<" unroll_suggest(u) --> vl="<<u.vl<<", vll="<<u.vll<<endl;

    if(1){
        cout<<"\nUnrolls:"<<str(u,"\nOrig: ")<<endl;

        if(uAlt.suggested==UNR_UNSET)
            cout<<"Alt:  "<<name(uAlt.suggested)<<endl;
        else
            cout<<str(uAlt,"\nAlt:  ")<<endl;
        cout<<endl;
    }

    if(vl0 < 0){ // we FORCE the alternate strategy (if it exists)
        if(u.vll) // equiv uAlt.suggested != UNR_UNSET
        {
            assert( uAlt.suggested != UNR_UNSET );
            cout<<" (forcing alt. strategy)"<<endl;
            //unroll_suggest(u); // oops, duplicate call
            u = uAlt;
        }else{
            cout<<" (no really great alt.strategy)"<<endl;
        }
    }
    return u;
}
/** worker routine -- loop 'ii' split.
 * vlen<0 means use |vlen| or a better lower alt VL if one is found.
 */
void cf5_no_unroll_split_ii( Cblock& outer, Cblock& inner, string pfx,
        int32_t const ilo, int32_t const ihi, LoopSplit const& lsjj,
        int const vlen, int const which, int const verbose)
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
    struct UnrollSuggest u = vl_nounroll(vlen,ii,jj);
    int const vl0 = u.vl;
    uint64_t const vl = min(iijj,(uint64_t)vl0);
    assert(vl > 0);
    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)
    auto& root=outer.getRoot().root; // a codeblock where I can add "state" (vl_remember)

    //auto& fd = inner["../first"]; // definitions
    // set up a 'fake' fused-loop scope (which may or may not contain a {..} code block)
    auto& outer_fd = outer["..*/first"];
    auto& fd0 = inner["first"];
    auto& fd = inner[pfx]["first"]; // in case loop split differs from fd ???
    auto& fp = inner[pfx]["preloop"];
    auto& cf5 = inner[pfx]["body"];
    auto& fz = inner[pfx]["last"]["krn"];
    //auto& fz = inner["last"]["krn"]; // this might also be a good spot
    fz>>"// ["<<pfx<<" krn output]";

    auto& cf5_defs = cf5;
    cf5_defs .DEF(pfx) .DEF(vl0) .DEF(ilo) .DEF(ii) .DEF(jlo) .DEF(jj) ;

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
        if(tag_once(outer,"have_sq")){
            INSCMT(outer_fd["sq"],"__vr const sq = _vel_vseq_vl(256);"," // sq[i]=i");
            vl_remember(256);
        }
    };
    auto use_sqij = [&fd0,&fd,&use_sq,&oss,&ilo,&jj](){
        use_sq();
        if(tag_once(fd,"have_sqij")){
            auto& cb=fd["last"]["sqij"];
            char const* t =(tag_once(fd0,"sqij")? "__vr ": "");
            // XXX assignment should be with vl0 here
            if(ilo==0) INSCMT(cb,OSSFMT(t<<"sqij = sq;"),"sqij[i]=i");
            else       INSCMT(cb,OSSFMT(t<<"sqij = _vel_vaddul_vsvl(ilo*jj,sq, vl0);"),
                                 OSSFMT("sqij[i]=i+"<<ilo<<"*"<<jj));
        }
    };
    auto use_iijj = [&fd0,&fp,&iijj,&ii,&jj,&oss](){
        if(!fp.find("iijj")){
            char const* t=(tag_once(fd0,"iijj")? "uint64_t ": "");
            INSCMT(fp["iijj"],OSSFMT(t<<"iijj=(uint64_t)ii*(uint64_t)jj;"),
                    OSSFMT("iijj="<<iijj<<"="<<ii<<"*"<<jj));
        }
    };
    auto kernComment = [&ilo,&ihi,&jlo,&jhi,&oss](){
        return OSSFMT("for("<<ilo<<"--"<<ihi<<")for("<<jlo<<"--"<<jhi<<")");
    };
    auto mk_divmod = [&](){ mk_DIVMOD(outer,jj,iijj+vl0); };
    auto use_vl = [&fd0,&fd](){
        auto& cb=fd["last"];
        if(tag_once(cb,"vl")){
            cb["vl"]>>(tag_once(fd0,"decl_vl")?"int64_t ":"")<<"vl = vl0;";
        }
    };
    // does this particular loop split need a final-VL check? .. iijj%vl!=0
    auto have_vl = [&fd]()->bool{ return fd["last"].find("vl"); };
    auto vlR = [&have_vl](){ return have_vl()? "vl": "vl0"; };
    //
    // --------- END helper lambdas ------------
    //

    if(tag_once(fd0,"set_vl")){
        // 'vl_is(vl0)' might cut a useless LVL op
        if(!vl_is(vl0)){
            fd>>OSSFMT(" /* XXX veSetVLENvl0)) */ ;  // VL="<<vl0<<" jj%vl0="<<jj%vl0
                    <<" iijj%vl0="<<iijj%vl0);
            vl_remember(vl0);
            assert(vl_is(vl0));
        }
    }else if(!vl_is(vl0)){
        fd>>OSSFMT(" /* XXX veSetVLENvl0)) */ ;  // VL="<<vl0<<" jj%vl0="<<jj%vl0
                <<" iijj%vl0="<<iijj%vl0<<" VL was "<<vl_str());
        vl_remember(vl0);
    }else{
        fd>>OSSFMT("// VL was "<<vl_str()<<", same as vl0 = "<<vl0<<" ?");
    }

    string alg_descr=OSSFMT("// "<<pfx<<" no_unroll: vl"<<vl<<kernComment()
            //<<","<<ilo<<".."<<ilo+ii<<","<<jj
            <<" nloop="<<nloop<<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0"
                : jj%vl==0? " jj%vl==0": positivePow2(jj)? " jj=2^N" : "")
            <<"cf5_save_vl="<<fd0["cf5_save_vl"].getType());
    fp>>alg_descr;

    // fp, pre-loop setup: declare or calc a[],b[]?
    //     - sometimes init calculation is easier than generic calc/induce
    bool fp_sets_ab = (nloop==1
            || (nloop>1 && (vl0<jj || vl0%jj==0)));
    // emit declarations [if iifirst] and init a[] b[] registers
    bool const decl_ab = tag_once(fd0,"decl_ab");
    if(fp_sets_ab){
        use_sq();
        auto& cb = fd; //(iifirst? fd: fp);
        //std::string vREG=(iifirst?"__vr ":"");
        char const* vREG=(decl_ab?"__vr ":"");
        // MUST use vel_FOO to fixes clang compilation bugs
        if( jj==1 ){
            // for loop split, don't easily know if we can 'const' it
            INSCMT(cb,(ilo==0
                        ?OSSFMT(vREG<<"a = sq;")
                        :OSSFMT(vREG<<"a = _vel_vaddul_vsvl(ilo,sq,vl0);")),
                    OSSFMT("a[i] = "<<ilo<<"+i"));
            INSCMT(cb,OSSFMT(vREG<<"b = _vel_vbrdl_vsl(0LL,vl0);"),
                    "b[i] = 0");
            // Note: vxor often "best" for other chips
        }else if(jj>=vl0){
            INSCMT(cb,OSSFMT(vREG<<"a = _vel_vbrdl_vsl(ilo,vl0);"),
                    OSSFMT("a[i] = "<<ilo));
            INSCMT(cb,OSSFMT(vREG<<"b = sq;"),
                    "b[i] = i");
        }else{ // note: mk_divmod also optimizes positivePow2(jj) case
            mk_divmod();
            use_sq();
            if(decl_ab) cb>>"__vr a,b;";
            string divmod;
            if(ilo==0){
                INSCMT(cb,OSSFMT("DIVMOD_"<<jj<<"(sq,vl0, a, b);"),
                        OSSFMT("a[]=sq/"<<jj<<" b[]=sq%"<<jj));
            }else{
                cb>>"b/*tmp*/ = _vel_vaddul_vsvl(ilo*jj,sq, vl0);";
                INSCMT(cb,OSSFMT("DIVMOD_"<<jj<<"(b,vl0, a, b);"),
                        OSSFMT("a[]=(sq+"<<ilo*jj<<")/"<<jj<<" b[]=(sq+"<<ilo*jj<<")%"<<jj));
            }
        }
    }else{
        if(decl_ab) fd>>"__vr a,b;";
    }

    auto krn_needs = kernel_needs(which);
    //if(krn_needs.vl) use_vl();
    if(krn_needs.sq) use_sq();
    if(krn_needs.sqij) use_sqij();
    if(krn_needs.iijj) use_iijj();
    if(krn_needs.cnt){
        //auto instr=OSSFMT((iifirst?"int64_t ":"")<<"cnt = 0;");
        //fp  >>OSSFMT(left<<setw(40)<<instr
        //        <<" // i-loop 0.."<<ii<<" ilo,ihi="<<ilo<<","<<ihi);
        //fd["have_cnt"].setType("TAG");
        fp>>OSSFMT((tag_once(fd0,"decl_cnt")?"int64_t ":"")<<"cnt = 0;");
        tag_once(fd,"have_cnt");
    }
    // krn_needs.cnt also gets some more treatement
    if(nloop==1){
        auto& fk = cf5["once"];
        if(krn_needs.cnt){
            fk["last"]>>"cnt=vl0;";
        }
        cf5_kernel(outer,fd,fk,fz, ilo,ii,jlo,jj,vl, kernComment(), which,
                pfx,0,"a","b","sq","vl0");
    }else if(nloop>1){
        //CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; cnt<iijj; cnt+=vl0)",inner);
        string loop_ab_string;
        use_iijj(); // iijj is ALWAYS used somewhere in loop_ab
        if(krn_needs.cnt){ // we require fwd-cnt scalar: 0, vl0, 2*vl0, ...
            loop_ab_string="for(/*int64_t cnt=0*/; cnt<iijj; cnt+=vl0)";
        }else{ // downward count gives easier final-vl calc
            loop_ab_string="for(int64_t cnt=iijj ; cnt>0; cnt-=vl0)";
        }
        CBLOCK_FOR(loop_ab,0/*no_unroll*/,loop_ab_string,cf5);
        auto& ff = loop_ab["../first"];
        auto& fk = loop_ab["krn"];

        // ff
        if(iijj%vl0){ // last iter has reduced VL
            use_vl();
            auto have_cnt = fd.find("have_cnt");
            if(have_cnt) use_iijj();
            string instr = (have_cnt
                    ?"vl = (vl0<iijj-cnt? vl0: iijj-cnt);"
                    :"vl = (cnt<vl0? cnt: vl0);");
            INSCMT(ff,instr,OSSFMT("iijj="<<iijj/vl0<<"*vl0+"<<iijj%vl0));
            ff>>" /* XXX veSetVLENvl)) */ ;";
        }
        if(!fp_sets_ab){
            use_sqij();
            mk_divmod();
            auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,"<<vlR()<<", a,b);");
            ff>>OSSFMT(left<<setw(40)<<divmod
                    <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
        }

        // fk
        cf5_kernel(outer, fd, fk, fz, ilo,ii,jlo,jj,vl, kernComment(), which,
                pfx,0,"a","b","sq",(fd0.find("decl_vl")?"vl":"vl0"));

        // manage a,b induction, vl change (loop exit?)
        auto& fi = loop_ab["iter"];
        //fi>>OSSFMT("// cf5_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
        if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
            int64_t vlojj = vl0/jj;
            cf5_defs.DEF(vlojj);
            fi  >>OSSFMT(left<<setw(40)<<"a =_vel_vadduw_vsvl(vlojj, a, "<<vlR()<<");"
                    <<" // a[i]+="<<vl0/jj<<", b[] same");
        }else if(jj%vl0==0){
            if(nloop<=jj/vl0){ // !have_jjMODvl_reset
                auto instr = OSSFMT("b = _vel_vadduw_vsvl("<<vl0<<",b, "<<vlR()<<");");
                fi>>OSSFMT(left<<setw(40)<<instr<<" // b[] += vl0, a[] const");
            }else{ // various nice ways to do periodic reset... [potentially better with unroll!]
                if(!fp.find("tmod")){
                    string declare = (tag_once(fd0,"tmod")? "uint32_t ": "");
                    INSCMT(fp["tmod"],OSSFMT(declare<<"tmod=0U;"),"periodic loop induction");
                }
                if(jj/vl0==2){
                    INSCMT(fi,"tmod = ~tmod;","toggle");
                }else if(positivePow2(jj/vl0)){
                    uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                    uint64_t const mask  = (1ULL<<shift) - 1U;
                    auto instr = OSSFMT("tmod = (tmod+1) & "<<jithex(mask)<<";");
                    INSCMT(fi,instr,"cyclic power-of-2 counter");
                }else{
                    fi>>OSSFMT(left<<setw(40)<<"++tmod;"
                            <<" // tmod period jj/vl0 = "<<jj/vl0);
                    auto instr = OSSFMT("if(tmod=="<<jj/vl0<<") tmod=0;");
                    INSCMT(fi,instr,"cmov reset when tmod=0");
                }
                use_sq();
                // If jj/vl0==2, then a,b could be set by invert-VM, 
                //                                            b=vmrg(b[]+vl0,sq,VM),
                //                                            vadd(a[],1,VM)
                // 1 VM-op + 3 vecops.  Probably slower than if+1.5 ops
                //
                // There is no indexed VR copy (i.e. b = %v[BASE+cnt%N].
                // Aurora can only simulate "long-vector-in-register" by UNROLLED code
                // Unrolled code (cf3-unroll.cpp) lookes very nice!.
                //
                // Could also do b' w/ scalar cmov, a' w/ scalar cmov, but ALWAYS 2 vec ops.
                //
                // Other precalc might possibly be for linear-combinations, but for now
                // these are hand-implemented (based on raw a[],b[])
                //
                fi  >>"if(tmod){"; // using mk_scope or CBLOCK_SCOPE is entirely optional...
                INSCMT(fi,OSSFMT("    b = _vel_vadduw_vsvl(vl0,b, "<<vlR()<<");"),
                        "b[] += vl0 (easy, a[] unchanged)");
                fi  >>"}else{";
                INSCMT(fi,OSSFMT("    a = _vel_vadduw_vsvl(1,a, "<<vlR()<<");"),
                        "a[] += 1");
                INSCMT(fi,"    b = sq;", "b[] = sq[] (reset)");
                fi  >>"}";
            }
        }else if(0 && positivePow2(jj)){
            // code block identical to full recalc DIVMOD case
            // (mk_divmod now recognizes jj=2^N)
            //
            // induction from prev a,b is longer than full recalc!
            //  sqij = _vel_vaddul_vsvl(vl0,sqij, vl);
            //  a = _vel_vsrl_vvsl(sqij, jj_shift, vl);              // a[i]=sq[i]/jj
            //  b = _vel_vand_vsvl("<<jithex(jj_minus_1)<<",sq, vl); // b[i]=sq[i]%jj
        }else{
            use_sqij();
            mk_divmod();
            string instr = "sqij = _vel_vaddul_vsvl(vl0,sqij, vl0);";
            INSCMT(fi,instr,OSSFMT("sqij[i] += "<<vl0));
            if(fp_sets_ab){
                auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,vl0, a,b);");
                INSCMT(fi,divmod,OSSFMT("a[]=sq/"<<jj<<" b[]=sq%"<<jj));
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
std::string cf5_no_unroll(int const vl00, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const which/*=WHICH_KERNEL*/, int const verbose/*=1*/)
{
    ostringstream oss;

    // upper-level tree structure, above the fused-loop.
    Cunit pr("cf5_no_unroll","C",0/*verbose*/);
    auto& inc = pr.root["includes"];
    inc >>"#include \"veintrin.h\""
        >>"#include \"velintrin.h\""
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
        cf5_no_unroll_split_ii(outer,inner,pfx, lsii.z, lsii.lo, lsjj,
                vl00, which, verbose);
    }
    if(lsii.hi > lsii.lo){
        string pfx="f5b";
        cf5_no_unroll_split_ii(outer,inner,pfx, lsii.lo, lsii.hi, lsjj,
                vl00, which, verbose);
    }
    if(lsii.end > lsii.hi){
        string pfx="f5c";
        cf5_no_unroll_split_ii(outer,inner,pfx, lsii.hi, lsii.end, lsjj,
                vl00, which, verbose);
    }

    if(1){
        cout<<"Tree:\n";
        pr.dump(cout);
        cout<<endl;
    }
    return pr.str();
}

std::string cf5_no_unrollX(Lpi const vlen, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const opt_t, int const which/*=WHICH_KERNEL*/, char const* ofname/*=nullptr*/)
{
    ostringstream oss;
    string author=OSSFMT(" cf5_no_unrollX(vlen="<<vlen<<","<<lsii<<","<<lsjj<<",...)");
    cout<<author<<endl;

    // wrap 'program' up with some boilerplate...
    oss<<"// Autogenerated by "<<__FILE__<<"\n"
        "// "<<author<<"\n"
        "// Possible compile:\n"
        "//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c"<<endl;

    int const vl00 = (opt_t==3? -(int)vlen: +(int)vlen);
    if(opt_t==3) cout<<" (negating vlen to "<<vl00<<" to signal alt-vl)"<<endl;
    string program = cf5_no_unroll(vl00,lsii,lsjj,which,1/*verbose*/);
    oss<<program;

    oss<<"// vim: ts=2 sw=2 et cindent\n";
    // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"

    program = oss.str();

#if 0
    // just to show how...
    cout<<"Trying out multiReplaceWord...\n"<<p1<<endl;
    auto p1 = multiReplaceWord(program, {{"a","vrx"},{"b","vry"},{"cnt","opix"}});
#else
    cout<<program<<endl;
#endif
    
    if(ofname!=nullptr){
        ofstream ofs(ofname);
        ofs<<program;
        ofs.close();
        cout<<"// Written to file "<<ofname<<endl;
    }
    return program;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break

