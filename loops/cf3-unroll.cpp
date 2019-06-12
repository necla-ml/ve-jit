/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "cf3.hpp"
#include "../stringutil.hpp"
#include "../ve_divmod.hpp"
#include <fstream>
#include <regex>

using namespace std;
using namespace loop;
using namespace cprog;

/** helper routine, after using unroll_suggest for a good VL \c vl0. */
std::string cfuse2_unroll(loop::Lpi const vl0, loop::Lpi const ii, Lpi const jj,
        int unroll, int cyc/*=0*/, int const which/*=WHICH_KERNEL*/,
	int const verbose/*=1*/)
{
    ostringstream oss;
    uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    uint64_t const vl = min(iijj,(uint64_t)vl0);
    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)

    Cunit pr(OSSFMT("cfuse2_unroll("<<unroll<<")"),"C",0/*verbose*/);
    auto& inc = pr.root["includes"];
    inc >>"#include \"veintrin.h\""
        >>"#include <stdint.h>";
    auto& fns = pr.root["fns"];
    fns["first"]; // reserve a node for optional function definitions
    CBLOCK_SCOPE(cfuse2,"int main(int argc,char**argv)",pr,fns["main"]);
    if(nloop==0){
        cfuse2>>OSSFMT("// for(0.."<<ii<<")for(0.."<<jj<<") --> NOP");
        return pr.str();
    }
    string pfx=OSSFMT("CFUSE2_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
    cfuse2 .DEF(pfx) .DEF(vl0) .DEF(ii) .DEF(jj) ;

    auto& fd = cfuse2["../first"]; // definitions

    string alg_descr=OSSFMT("// cfuse2_unroll: vl,ii,jj="<<vl<<","<<ii
            <<","<<jj<<" nloop="<<nloop
            <<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0":
                jj%vl==0? " jj%vl==0": positivePow2(jj)? " jj=2^N" : "")
            <<", unroll="<<unroll);
    if(unroll==0) { // 'any' small unroll...
        uint32_t un=0U;
        if((uint32_t)nloop<4U) un = nloop;
        if(un!=0){
            cout<<"unroll = nloop = "<<nloop<<endl;
        }else{
            //for(uint32_t uu=8U; uu>=4U; --uu){ // is there an nPart==0 unroll?
            //    if(nloop%uu == 0) {un=uu; break;}
            //}
        }
        if(un!=0U){
            cout<<" No-remainder division of nloop="<<nloop<<" with 'any' unroll="<<un<<endl;
        }else{ // no really nice unroll, divide work roughly evenly (uu ~ nPart)
            // I have a standard "work-splitting calc", but where is it? what did I call it? XXX
            un = 8U;
            uint32_t nFull = nloop/un;
            //uint32_t nPart = nloop - nFull*un;
            for(uint32_t uu=un ;uu>=1U; --uu){
                uint32_t nf = nloop/uu;
                if( nf != nFull ) break;
                un = uu;
            }
            cout<<"Work-splitting 'any' unroll="<<un<<endl;
        }
        unroll = un;
        if(unroll > 8) unroll=8;
    }
    assert(unroll>0);
    if(unroll > nloop) unroll = nloop;
    uint32_t nFull = nloop/unroll;
    uint32_t nPart = nloop - nFull*unroll;
    assert(unroll <= nloop);
    assert(nFull > 0);
    alg_descr.append(OSSFMT("-->"<<unroll<<", cyc="<<cyc<<", nFull:nPart="<<nFull<<":"<<nPart));
    fd>>alg_descr;
    fd>>OSSFMT("_ve_lvl(vl0);  // VL = "<<vl0);

    //
    // ------------- helper lambdas ------------
    //
    // some kernels might ask for const sq reg to always be defined
    auto have_sq = [&fd](){ return fd.find("have_sq") != nullptr; };
    auto have_sqij = [&fd](){ return fd.find("have_sqij") != nullptr; };
    auto use_sq = [&fd,have_sq,have_sqij,&oss](){
        // if nec, define const sequence register "sq"
        if(!have_sq()){
            if(have_sqij())
                fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = sqij;")<<" // sq[i]=i";
            else
                fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = _ve_vseq_v();")<<" // sq[i]=i";
            fd["have_sq"].setType("TAG");
        }
    };
    auto use_sqij = [&fd,have_sq,have_sqij,&oss](){
        // if nec, define non-const sequence register "sq"
        if(!have_sqij()){
            if(have_sq())
                fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<"__vr sqij = sq;")
                    <<" // sqij[i]=i";
            else
                fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<"__vr sqij = _ve_vseq_v();")
                    <<" // sqij[i]=i";
            fd["have_sqij"].setType("TAG");
        }
    };
    auto equ_sq = [have_sq,have_sqij](std::string v){
        return v+(have_sq() ? " = sq;          "
                : have_sqij() ? " = sqij;        " // only OK pre-loop !
                : " = _ve_vseq_v();"); };
    auto use_iijj = [&fd,&iijj,&ii,&jj,&oss](){
        if(!fd.find("have_iijj")){
            fd["first"]["iijj"]>>OSSFMT(left<<setw(40)
                    <<"uint64_t const iijj=(uint64_t)ii*(uint64_t)jj;")
                <<OSSFMT(" // iijj="<<ii<<"*"<<jj<<"="<<iijj);
            fd["have_iijj"].setType("TAG");
        }
    };
    auto kernComment = [have_sq,ii,jj](){
        return have_sq()? "sq[]=0.."+jitdec(ii*jj-1): "";
    };
    auto mk_divmod = [&](){ mk_DIVMOD(cfuse2,jj,iijj+vl0); };
    auto use_vl = [&fd](){
        if(! fd.find("have_vl")){
            fd["last"]["vl"]>>"int64_t vl = vl0;";
            fd["have_vl"].setType("TAG");
        }
    };
    //
    // --------- END helper lambdas ------------
    //

    // fp, pre-loop setup: declare or calc a[],b[]?
    //     - sometimes init calculation is easier than generic calc/induce
    auto& fp = cfuse2["preloop"];
    bool fp_sets_ab = (nloop==1
            || (nloop>1 && (vl0%jj==0 || jj>=vl0)));
    if(fp_sets_ab){
        std::string vREG=(nloop<=1?"__vr const ":"__vr ");
        if( jj==1 ){
            fp>>equ_sq(vREG+"a")<<"         // a[i] = i";
            fp>>vREG<<"b = _ve_vbrd_vs_i64(0LL); // b[i] = 0"; // libvednn way
            //fp>>vREG<<"b; b=_ve_vxor_vvv(b,b);"; // typically "best way"
        }else if(jj>=vl0){
            fp>>vREG<<"a = _ve_vbrd_vs_i64(0LL); // a[i] = 0";
            fp>>equ_sq(vREG+"b")<<"         // b[i] = i";
        }else{ // note: mk_divmod also optimizes positivePow2(jj) case
            mk_divmod();
            use_sq();
            fp>>"__vr a,b;";
            auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sq, a, b);");
            fp>>OSSFMT(left<<setw(40)<<divmod
                <<" // a[]=sq/"<<jj<<" b[]=sq%"<<jj);
        }
    }else{
        fp>>"__vr a,b;";
    }

    auto& fz = cfuse2["last"]["krn"];
    fz>>"// [krn output]";

    auto krn_needs = kernel_needs(which);
    if(krn_needs.vl) use_vl();
    if(krn_needs.sq) use_sq();
    if(krn_needs.sqij) use_sqij();
    if(krn_needs.iijj) use_iijj();
    // krn_needs.cnt handled separately
    if(nloop==1){
        auto& fk = fp["krn"];
        if(krn_needs.cnt && !fd.find("have_cnt")){
            fp>>"int64_t cnt=0; // 0..iijj-1";
            fk["last"]>>"cnt=vl0;";
            fd["have_cnt"].setType("TAG");
        }
        fuse2_kernel(fk, fd, fz, ii,jj,vl, kernComment(), which);
    }else if(nloop>1){
        //CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; cnt<iijj; cnt+=vl0)",cfuse2);
        //--------------------------------------- cnt_update; cnt_test
        string cnt_update;
        string cnt_test;
        string loop_ab_string;
        int pragma_unr=0; // no_unroll
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
                loop_ab_string=OSSFMT("for(int64_t cnt=0 ; "<<cnt_test<<"; /*"<<cnt_update<<"*/)");
            }
        }
        use_iijj();
        CBLOCK_FOR(loop_ab,pragma_unr, loop_ab_string, cfuse2);

        auto comment=OSSFMT("// nloop="<<nloop<<" unroll by "<<unroll
                <<" --> "<<nFull<<" times + "<<nPart<<" remainder");
        cout<<comment;
        loop_ab<<comment;
        auto maybe_exit = [&cnt_test,&oss](Cblock& cb, bool const doit){
            if(doit){
                auto instr = OSSFMT("if(!("<<cnt_test<<")) break;");
                cb>>OSSFMT(left<<setw(40)<<instr<<" // EXIT TEST");
            }
        };

        // if nPart, then FULL loops do not need to check VL
        // In general, need to check VL when u==nloop%unroll
        for(uint32_t u=0U; u<(uint32_t)unroll; ++u){
            bool const last_iter_check = (u == (nPart+(uint32_t)unroll-1U)%(uint32_t)unroll);
            bool const extra_exit_test = (last_iter_check && u!=(uint32_t)unroll-1U);
            auto& fu = loop_ab[OSSFMT("unr"<<u)];
            fu<<OSSFMT("                /*UN"<<u
                    <<(last_iter_check && loop_ab_string.empty()?", last iter"
                        :last_iter_check?", possibly last iter"
                        :"")
                    <<"*/");

            auto& ff = fu["first"];
            auto& fk = fu["krn"];

            // ff
            if(iijj%vl0){ // last iter has reduced VL
                if(last_iter_check){
                    use_vl();
                    ff  >>(fd.find("have_cnt")
                            ?"vl = (vl0<iijj-cnt? vl0: iijj-cnt);      // vl = min(vl0,remain)"
                            :"vl = (cnt<vl0? cnt: vl0);                // vl = min(vl0,remain)")
                        >>"_ve_lvl(vl);";
                }else{
                    //ff>>"// VL ok";
                }
            }
            if(!fp_sets_ab){ // if no pre-loop a,b calc, do it right before kernel call
                use_sqij();
                mk_divmod();
                auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
                ff>>OSSFMT(left<<setw(40)<<divmod
                        <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
            }

            // fk
            fuse2_kernel(fk, fd, fz, ii,jj,vl, kernComment(), which);

            // fi (induce)
            auto& fc = loop_ab[OSSFMT("unr"<<u)]["cnt"];
            if(!cnt_update.empty()) fc>>cnt_update; // cnt = cnt +/- vl0;
            maybe_exit(fc,extra_exit_test);
            // if no for-loop and last unroll (u==nloop), then squash the useless terminal induction
            bool const loop_induce = (unroll<nloop || (unroll==nloop && u<(uint32_t)unroll-1U));
            if(loop_induce){
                auto& fi = loop_ab[OSSFMT("unr"<<u)]["iter"];

                //fi>>OSSFMT("// CFUSE2_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
                if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
                    int64_t vlojj = vl0/jj;
                    if(!fd.find("have_vlojj")){
                        cfuse2 .DEF(vlojj);
                        fd["have_vlojj"].setType("TAG");
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
                    //
                    // induction from prev a,b is longer than full recalc!
                    //  sqij = _ve_vaddul_vsv(vl0,sqij);
                    //  a = _ve_vsrl_vvs(sqij, jj_shift);              // a[i]=sq[i]/jj
                    //  b = _ve_vand_vsv("<<jithex(jj_minus_1)<<",sq); // b[i]=sq[i]%jj
                }else{
                    use_sqij();
                    mk_divmod();
                    auto instr = "sqij = _ve_vaddul_vsv(vl0,sqij);";
                    fi>>OSSFMT(left<<setw(40)<<instr<<" // sqij[i] += "<<vl0);
                    if(fp_sets_ab){ // if pre-loop sets ab, recalc goes to fi (not ff)
                        auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
                        fi  >>OSSFMT(left<<setw(40)<<divmod
                                <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
                    }
                }
            }
        }
    }
    string ret = pr.str();
    cout<<"cfuse_unroll("<<unroll<<")--> string["<<ret.size()<<"]"<<endl;
    return ret;
}

std::string cfuse2_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const maxun, int const opt_t,
        int const which/*=WHICH_KERNEL*/, char const* ofname/*=nullptr*/)
{
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

    string program = cfuse2_unroll(vl,ii,jj,u.unroll,u.cycle,which,1/*verbose*/);
    // wrap 'program' up with some boilerplate...
    ostringstream oss;
    oss<<"// Autogenerated by "<<__FILE__<<" cfuse2_unrollX("<<vl<<"[/"<<vlen<<"]"
        <<","<<ii<<","<<jj<<",unroll="<<u.unroll<<"[/"<<maxun<<"],cyc="<<u.cycle<<",...)\n";
    oss<<"// Possible compile:\n";
    oss<<"//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c"<<endl;
    oss<<program;
    oss<<"// vim: ts=2 sw=2 et cindent\n";
    // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"
    program = oss.str();

#if 1
    auto p1 = multiReplaceWord(program, {{"a","vrx"},{"b","vry"},{"cnt","opix"}});
#else
    std::string p1 = std::regex_replace(std::regex_replace(program,
                std::regex("\\ba\\b"),string("vec_a"))
            ,std::regex("\\bb\\b"),string("vec_b"));
#endif
    cout<<"Trying out multiReplaceWord...\n"<<p1<<endl;
    
    if(ofname!=nullptr){
        ofstream ofs(ofname);
        ofs<<program;
        ofs.close();
        cout<<"// Written to file "<<ofname<<endl;
    }
    return program;
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
