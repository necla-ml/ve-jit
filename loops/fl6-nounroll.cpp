/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "fl6.hpp"
#include "../stringutil.hpp"
#include "../ve_divmod.hpp"
#include <fstream>
//#include <regex>

using namespace std;
using namespace loop;
using namespace cprog;

//#define DBG(WHAT) cout<<" DBG:"<<WHAT<<endl
#define DBG(WHAT)

static std::ostringstream oss;

static bool tag_once(Cblock &cb, std::string const& sub){
    Cblock *found = cb.find(sub);
    if(!found) cb[sub].setType("TAG");
    return found==nullptr;
}

/** vl0<0 means use |vl0| or a [better] lower alternate VL. */
static struct UnrollSuggest vl_nounroll(int const vl0, int const ii, int const jj,
        int const v=1/*verbosity*/)
{
    int vlen = abs(vl0);
    if(vlen==0) vlen=256;       // XXX MVL not yet standardized in headers!
    if(v>1) cout<<"vl_nounroll("<<vl0<<"-->"<<vlen<<",ii="<<ii<<",jj="<<jj<<endl;

    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, -8/*b_period_max+no_unroll*/, v );
    if(v>1)cout<<"vl0="<<vl0<<" unroll_suggest("<<vlen<<","<<ii<<","<<jj
        <<",8) --> vl="<<u.vl<<", vll="<<u.vll<<endl;
    // suggest an alternate nice vector length, sometimes.
    auto uAlt = unroll_suggest(u, 0/*vl_min auto*/, v);
    if(v>1)cout<<"vl0="<<vl0<<" unroll_suggest(u) --> vl="<<u.vl<<", vll="<<u.vll<<endl;

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
void fl6_no_unroll_split_ii(
        int32_t const ilo, int32_t const ihi, LoopSplit const& lsjj,
        int const vlen, FusedLoopTest& fl6t, int const verbose)
{
    //fl6_no_unroll_split_ii(lsii.z, lsii.lo, lsjj, vl00, fl6t, v);
    //Cunit& pr     = fl6t.pr;
    FusedLoopKernel& krn = fl6t.krn();
    string const& pfx   = krn.pfx;
    Cblock& outer = fl6t.outer();
    Cblock& inner = fl6t.inner();

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
    struct UnrollSuggest u = vl_nounroll(vlen,ii,jj,verbose);
    int const vl0 = u.vl;
    uint64_t const vl = min(iijj,(uint64_t)vl0);
    assert(vl > 0);
    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)
    auto& root=outer.getRoot().root; // a codeblock where I can add "state" (vl_remember)

    //auto& fd = inner["../first"]; // definitions
    // set up a 'fake' fused-loop scope (which may or may not contain a {..} code block)
    auto& outer_fd = outer["..*/first"];
    auto& outer_fdup = outer_fd.goto_defines();
    outer_fd["first"]; // just in case
    auto& fd0 = inner["first"];
    auto& fd = inner[pfx]["first"]; // in case loop split differs from fd ???
    auto& fp = inner[pfx]["preloop"];
    auto& fl6 = inner[pfx]["body"];
    auto& fz = inner[pfx]["last"]["krn"];
    //auto& fz = inner["last"]["krn"]; // this might also be a good spot
    fz>>"// ["<<pfx<<" krn output]";

    auto& fl6_defs = fl6;
    fl6_defs .DEF(pfx) .DEF(vl0) .DEF(ilo) .DEF(ii) .DEF(jlo) .DEF(jj) ;

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
    auto mk_divmod = [&](){ mk_DIVMOD(outer,jj,iijj+vl0,max(0,verbose-1)); };

    krn.sVL="vl0";                  // use constant 'vl0', if lucky
    auto use_vl = [&fd0,&fd,&krn](){
        krn.sVL="vl";               // else last krn.emit has a calculated VL
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
            fd>>OSSFMT(" /* XXX _ve_lvl(vl0)) */ ;  // VL="<<vl0<<" jj%vl0="<<jj%vl0
                    <<" iijj%vl0="<<iijj%vl0);
            vl_remember(vl0);
            assert(vl_is(vl0));
        }
    }else if(!vl_is(vl0)){
        fd>>OSSFMT(" /* XXX _ve_lvl(vl0)) */ ;  // VL="<<vl0<<" jj%vl0="<<jj%vl0
                <<" iijj%vl0="<<iijj%vl0<<" VL was "<<vl_str());
        vl_remember(vl0);
    }else{
        fd>>OSSFMT("// VL was "<<vl_str()<<", same as vl0 = "<<vl0<<" ?");
    }

    string alg_descr=OSSFMT("// "<<pfx<<" no_unroll: "
            <<" vl "<<vl<<"("<<fd0["fl6_save_vl"].getType()<<")"
            <<" "<<kernComment()<<" nloop="<<nloop
            <<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0"
                : jj%vl==0? " jj%vl==0": positivePow2(jj)? " jj=2^N" : ""));
    outer_fdup>>alg_descr;
    fd>>alg_descr;

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
                INSCMT(cb,OSSFMT("DIVMOD_"<<jj<<"(_vel_vaddul_vsvl(ilo*jj,sq, vl0),vl0, a, b);"),
                        OSSFMT("a[]=(sq+"<<ilo*jj<<")/"<<jj<<" b[]=(sq+"<<ilo*jj<<")%"<<jj));
            }
        }
    }else{
        if(decl_ab) fd>>"__vr a,b;";
    }

    auto const& krn_needs = krn.needs();
    //if(krn_needs.vl) use_vl();
    // instead we use vl or vl0 (only need vl sometimes)
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
        auto& fk = fl6["once"];
        if(krn_needs.cnt){
            fk["last"]>>"cnt=vl0;";
        }
        krn.emit(fd,fk,fz, ilo,ii,jlo,jj,vl, kernComment(), verbose);
    }else if(nloop>1){
        //CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; cnt<iijj; cnt+=vl0)",inner);
        string loop_ab_string;
        use_iijj(); // iijj is ALWAYS used somewhere in loop_ab
        if(krn_needs.cnt){ // we require fwd-cnt scalar: 0, vl0, 2*vl0, ...
            loop_ab_string="for(/*int64_t cnt=0*/; cnt<iijj; cnt+=vl0)";
        }else{ // downward count gives easier final-vl calc
            loop_ab_string="for(int64_t cnt=iijj ; cnt>0; cnt-=vl0)";
        }
        CBLOCK_FOR(loop_ab,0/*no_unroll*/,loop_ab_string,fl6);
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
            ff>>" /* XXX _ve_lvl(vl)) */ ;";
        }
        if(!fp_sets_ab){
            use_sqij();
            mk_divmod();
            auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,vl0, a,b);");
            ff>>OSSFMT(left<<setw(40)<<divmod
                    <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
        }

        // fk
        cout<<" noU-krn.sVL="<<krn.sVL<<endl;
        krn.emit(fd,fk,fz, ilo,ii,jlo,jj,vl, kernComment(), verbose);

        // manage a,b induction, vl change (loop exit?)
        auto& fi = loop_ab["iter"];
        //fi>>OSSFMT("// fl6_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
        if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
            int64_t vlojj = vl0/jj;
            fl6_defs.DEF(vlojj);
            fi  >>OSSFMT(left<<setw(40)<<"a =_vel_vadduw_vsvl(vlojj, a, "<<vlR()<<");"
                    <<" // a[i]+="<<vl0/jj<<", b[] same");
        }else if(jj%vl0==0){
            // NO! assert(have_vl()); // so vlR() can be replaced with "vl0" in this block ...
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
                cout<<divmod<<endl;
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

std::string fl6_no_unroll(int const vl00,
        LoopSplit const& lsii, LoopSplit const& lsjj,
        FusedLoopTest& fl6t, int const v)
{
    cout<<"fl6_no_unroll(..FusedLoopTest..)"<<endl;
    Cunit& pr     = fl6t.pr;
    //FusedLoopKernel const& krn = fl6t.krn;
    string& pfx   = fl6t.krn().pfx;
    //Cblock& outer = fl6t.outer();
    Cblock& inner = fl6t.inner();
    //cout<<"inner and outer blocks ok? inner @ "<<inner.fullpath()<<endl;

    Lpi const ii = lsii.end - lsii.z;
    Lpi const jj = lsjj.end - lsjj.z;
    uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    if(iijj<=0){ // equiv. nloop==0;
        inner>>OSSFMT("//"
                "for("<<lsii.z<<".."<<lsii.end<<")"
                "for("<<lsjj.z<<".."<<lsjj.end<<") --> NOP");
    }else{
        if(lsii.lo > lsii.z){
            pfx="f5a";
            fl6_no_unroll_split_ii(lsii.z, lsii.lo, lsjj, vl00, fl6t, v);
        }
        if(lsii.hi > lsii.lo){
            pfx="f5b";
            fl6_no_unroll_split_ii(lsii.lo, lsii.hi, lsjj, vl00, fl6t, v);
        }
        if(lsii.end > lsii.hi){
            pfx="f5c";
            fl6_no_unroll_split_ii(lsii.hi, lsii.end, lsjj, vl00, fl6t, v);
        }
    }

    if(v>1){
        cout<<"Tree:\n";
        pr.dump(cout);
        cout<<endl;
    }
    return pr.str();
}
std::string fl6_no_unrollY(LoopSplit const& lsii, LoopSplit const& lsjj,
        //FusedLoopKernel& krn,
        Fl6test& fl6t,
        Lpi const vlen/*=0*/,
        char const* ofname/*=nullptr*/, int v/*=0,verbose*/)
{
    ostringstream oss;
    auto& krn=fl6t.krn();
    string author=OSSFMT(" fl6_no_unrollY(vlen="<<vlen<<","<<lsii<<","<<lsjj
            <<",krn="<<krn.name()<<","<<(ofname?ofname:"NULL")<<")");
    cout<<author<<endl;

    //Fl6test fl6t{krn,max(0,v-2)};

    // wrap 'program' up with some boilerplate...
    oss<<"// Autogenerated by "<<__FILE__<<"\n"
        "// "<<author<<"\n";
    if(ofname){
        oss<<"// Possible compile:\n"
            "//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops"
            " -fno-slp-vectorize -fno-crash-diagnostics "<<ofname<<endl;
    }

    int const vl00 = (vlen==0? -(int)256: vlen);
    if(v && vl00 < 0) cout<<" vlen "<<vl00<<" < 0 will search for good VL <= "<<-vl00<<endl;
    string program = fl6_no_unroll(vl00,lsii,lsjj,fl6t,max(0,v));
    oss<<program;

    int const sw = fl6t.pr.shiftwidth;
    oss<<"// vim: ts="<<sw<<" sw="<<sw<<" et cindent\n";
    // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"

    program = oss.str();
    if(v>=0) cout<<program<<endl;
    
    if(ofname!=nullptr){
        ofstream ofs(ofname);
        ofs<<program;
        ofs.close();
        cout<<"// Written to file "<<ofname<<endl;
    }
    return program;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break

