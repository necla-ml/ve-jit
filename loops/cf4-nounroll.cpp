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

/** helper routine, after using unroll_suggest for a good VL \c vl0. */
std::string cfuse2_no_unroll(Lpi const vl0, Lpi const ii, Lpi const jj,
        int const which/*=WHICH_KERNEL*/, int const verbose/*=1*/)
{
    ostringstream oss;
    uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    uint64_t const vl = min(iijj,(uint64_t)vl0);
    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)

    Cunit pr("cfuse2_no_unroll","C",0/*verbose*/);
    auto& inc = pr.root["includes"];
    inc >>"#include \"veintrin.h\""
        >>"#include \"velintrin.h\""
        >>"#include <stdint.h>";

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

    if(nloop==0){
        inner>>OSSFMT("// for(0.."<<ii<<")for(0.."<<jj<<") --> NOP");
        return pr.str();
    }
    string pfx=OSSFMT("CF4_"<<vl<<"_"<<ii<<"_"<<jj);

    //auto& fd = inner["../first"]; // definitions
    // set up a 'fake' fused-loop scope (which may or may not contain a {..} code block)
    auto& fd = inner["cf4"]["first"];
    auto& fp = inner["cf4"]["preloop"];
    auto& cf4 = inner["cf4"]["body"];
    auto& fz = inner["cf4"]["last"]["krn"];
    //auto& fz = inner["last"]["krn"]; // this might also be a good spot
    fz>>"// ["<<pfx<<" krn output]";

    string alg_descr=OSSFMT("// cfuse2_no_unroll: vl,ii,jj="<<vl<<","<<ii<<","<<jj
            <<" nloop="<<nloop<<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0"
                : jj%vl==0? " jj%vl==0": positivePow2(jj)? " jj=2^N" : ""));
    fd>>alg_descr;

    auto& cf4_defs = (1? cf4: outer); // not 100% sure where I want them
    cf4_defs .DEF(pfx) .DEF(vl0) .DEF(ii) .DEF(jj) ;
    fd>>OSSFMT(" /* XXX veSetVLENvl0)) */ ;  // VL="<<vl0<<" jj%vl0="<<jj%vl0<<" iijj%vl0="<<iijj%vl0);

    //
    // ------------- helper lambdas ------------
    //
    // some kernels might ask for const sq reg to always be defined
    auto have_sq = [&fd](){ return fd.find("have_sq") != nullptr; };
    auto have_sqij = [&fd](){ return fd.find("have_sqij") != nullptr; };
    auto use_sq = [&fd,have_sq,have_sqij,&oss](){
        // if nec, define const sequence register "sq"
        if(!have_sq()){
#if 0
            if(have_sqij())
                fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = sqij;")<<" // sq[i]=i";
            else
                fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = _vel_vseq_vl(vl0);")<<" // sq[i]=i";
            fd["have_sq"].setType("TAG");
#else // new
            fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = _vel_vseq_vl(vl0);")<<" // sq[i]=i";
            fd["have_sq"].setType("TAG");
            if(have_sqij()){ // just in case it was _vel_vseq_vl(vl0)...
                fd["last"]["sqij"].set(OSSFMT(left<<setw(40)<<"__vr sqij = sq;"<<" // sqij[i]=i"));
            }
#endif
        }
    };
    auto use_sqij = [&fd,have_sq,have_sqij,&oss](){
        // if nec, define non-const sequence register "sq"
        if(!have_sqij()){
            if(have_sq())
                fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<"__vr sqij = sq;")
                    <<" // sqij[i]=i";
            else
                fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<"__vr sqij = _vel_vseq_vl(vl0);")
                    <<" // sqij[i]=i";
            fd["have_sqij"].setType("TAG");
        }
    };
    auto equ_sq = [have_sq,have_sqij](std::string v){
        return v+(have_sq() ? " = sq;          "
                : have_sqij() ? " = sqij;        " // only OK pre-loop !
                : " = _vel_vseq_vl(vl0);"); };
    auto use_iijj = [&fd,&iijj,&ii,&jj,&oss](){
        if(!fd.find("have_iijj")){
            fd["first"]["iijj"]>>OSSFMT(left<<setw(40)
                    <<"uint64_t const iijj=(uint64_t)ii*(uint64_t)jj;"
                    <<" // iijj="<<ii<<"*"<<jj<<"="<<iijj);
            fd["have_iijj"].setType("TAG");
        }
    };
    auto kernComment = [have_sq,ii,jj](){
        return have_sq()? "sq[]=0.."+jitdec(ii*jj-1): "";
    };
    auto mk_divmod = [&](){ mk_DIVMOD(outer,jj,iijj+vl0); };
    auto use_vl = [&fd](){
        if(! fd.find("have_vl")){
            fd["last"]["vl"]>>"int64_t vl = vl0;";
            fd["have_vl"].setType("TAG");
        }
    };
    auto vlR = [&fd](){ return fd.find("have_vl")? "vl": "vl0"; };
    //
    // --------- END helper lambdas ------------
    //

    // fp, pre-loop setup: declare or calc a[],b[]?
    //     - sometimes init calculation is easier than generic calc/induce
    bool fp_sets_ab = (nloop==1
            || (nloop>1 && (vl0<jj || vl0%jj==0)));
    if(fp_sets_ab){
        std::string vREG=(nloop<=1?"__vr const ":"__vr ");
        if( jj==1 ){
            fp>>equ_sq(vREG+"a")<<"         // a[i] = i";
            fp>>vREG<<"b = _vel_vbrdl_vsl(0LL, vl0); // b[i] = 0"; // libvednn way
            //fp>>vREG<<"b; b=_vel_vxor_vvvl(b,b, vl0);"; // typically "best way"
        }else if(jj>=vl0){
            fp>>vREG<<"a = _vel_vbrdl_vsl(0LL, vl0); // a[i] = 0";
            fp>>equ_sq(vREG+"b")<<"         // b[i] = i";
        }else{ // note: mk_divmod also optimizes positivePow2(jj) case
            mk_divmod();
            use_sq();
            fp>>"__vr a,b;";
            auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sq,"<<vl0<<", a, b);");
            fp>>OSSFMT(left<<setw(40)<<divmod<<" // a[]=sq/"<<jj<<" b[]=sq%"<<jj);
        }
    }else{
        fp>>"__vr a,b;";
    }

    auto krn_needs = kernel_needs(which);
    if(krn_needs.vl) use_vl();
    if(krn_needs.sq) use_sq();
    if(krn_needs.sqij) use_sqij();
    if(krn_needs.iijj) use_iijj();
    // krn_needs.cnt is handled separately
    if(nloop==1){
        auto& fk = cf4;
        if(krn_needs.cnt && !fd.find("have_cnt")){
            fp>>"int64_t cnt=0; // 0..iijj-1";
            fd["have_cnt"].setType("TAG");
            fk["last"]>>"cnt=vl0;";
        }
        fuse4_kernel(outer, fd, fk, fz, ii,jj,vl, kernComment(), which);
    }else if(nloop>1){
        //CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; cnt<iijj; cnt+=vl0)",inner);
        string loop_ab_string;
        if(krn_needs.cnt){ // we require fwd-cnt scalar: 0, vl0, 2*vl0, ...
            loop_ab_string="for(/*int64_t cnt=0*/; cnt<iijj; cnt+=vl0)";
            fp>>"int64_t cnt=0; // 0..iijj-1";
            fd["have_cnt"].setType("TAG");
        }else{ // downward count gives easier final-vl calc
            loop_ab_string="for(int64_t cnt=iijj ; cnt>0; cnt-=vl0)";
        }
        CBLOCK_FOR(loop_ab,0/*no_unroll*/,loop_ab_string,cf4);
        auto& ff = loop_ab["../first"];
        auto& fk = loop_ab["krn"];

        // ff
        if(iijj%vl0){ // last iter has reduced VL
            use_vl();
            ff  >>OSSFMT(left<<setw(40)<<(fd.find("have_cnt")
                        ?"vl = (vl0<iijj-cnt? vl0: iijj-cnt);"
                        :"vl = (cnt<vl0? cnt: vl0);")
                    //<<"// vl = min(vl0,remain)"
                    <<" // iijj="<<iijj/vl0<<"*vl0+"<<iijj%vl0
                    );
            ff  >>" /* XXX veSetVLENvl)) */ ;";
        }
        if(!fp_sets_ab){
            use_sqij();
            mk_divmod();
            auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,"<<vlR()<<", a,b);");
            ff>>OSSFMT(left<<setw(40)<<divmod
                    <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
        }

        // fk
        fuse4_kernel(outer, fd, fk, fz, ii,jj,vl, kernComment(), which);

        // manage a,b induction, vl change (loop exit?)
        auto& fi = loop_ab["iter"];
        use_iijj();
        //fi>>OSSFMT("// CFUSE2_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
        if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
            int64_t vlojj = vl0/jj;
            cf4_defs.DEF(vlojj);
            fi  >>OSSFMT(left<<setw(40)<<"a =_vel_vadduw_vsvl(vlojj, a, vl);"
                    <<" // a[i]+="<<vl0/jj<<", b[] same");
        }else if(jj%vl0==0){
            if(nloop<=jj/vl0){ // !have_jjMODvl_reset
                auto instr = OSSFMT("b = _vel_vadduw_vsvl("<<vl0<<",b, vl);");
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
                fi  >>"if(tmod){" // using mk_scope or CBLOCK_SCOPE is entirely optional...
                    >>"    b = _vel_vadduw_vsvl(vl0,b, vl);           // b[] += vl0 (easy, a[] unchanged)"
                    >>"}else{"
                    >>"    a = _vel_vadduw_vsvl(1,a, vl);             // a[] += 1"
                    >>"    b = sq;                              // b[] = sq[] (reset case)"
                    >>"}";
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
            string instr = "sqij = _vel_vaddul_vsvl(vl0,sqij, vl);";
            fi>>OSSFMT(left<<setw(40)<<instr<<" // sqij[i] += "<<vl0);
            if(fp_sets_ab){
                auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,"<<vlR()<<", a,b);");
                cout<<divmod<<endl;
                fi>>OSSFMT(left<<setw(40)<<divmod
                        <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
            }
        }
    }
    fz>>"// cf4 done";
    if(1){
        cout<<"Tree:\n";
        pr.dump(cout);
    }
    return pr.str();
}
std::string cfuse2_no_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const opt_t, int const which/*=WHICH_KERNEL*/, char const* ofname/*=nullptr*/)
{
    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, 8/*b_period_max, don't care*/ );
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

    cout<<" cfuse2_no_unrollX ..."<<endl;

    string program = cfuse2_no_unroll(vl,ii,jj,which,1/*verbose*/);
    // wrap 'program' up with some boilerplate...
    ostringstream oss;
    oss<<"// Autogenerated by "<<__FILE__<<" cfuse2_no_unrollX("<<vl<<"[/"<<vlen<<"]"
        <<","<<ii<<","<<jj<<",...)\n";
    oss<<"// Possible compile:\n";
    oss<<"//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c"<<endl;
    oss<<program;
    oss<<"// vim: ts=2 sw=2 et cindent\n";
    // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"
    program = oss.str();

    // just to show how...
    auto p1 = multiReplaceWord(program, {{"a","vrx"},{"b","vry"},{"cnt","opix"}});
#if 0
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

