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

/** vl0<0 means use |vl0| or a lower alternate VL. */
static struct UnrollSuggest vl_nounroll(int const vl0, int const ii, int const jj)
{
    int vlen = abs(vl0);
    if(vlen==0) vlen=256;       // XXX MVL not yet standardized in headers!
    int opt_t = (vlen<=0? 3: 0);

    cout<<"vl_nounroll("<<vl0<<"-->"<<vlen<<",ii="<<ii<<",jj="<<jj
        <<")opt_t="<<opt_t<<endl;
    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, 8/*b_period_max, don't care*/ );
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
    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)
    //Cunit pr=outer.getRoot();

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

    string alg_descr=OSSFMT("// "<<pfx<<"no_unroll: vl,ii,jj="<<vl
            <<","<<ilo<<".."<<ilo+ii<<","<<jj
            <<" nloop="<<nloop<<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0"
                : jj%vl==0? " jj%vl==0": positivePow2(jj)? " jj=2^N" : ""));
    fp>>alg_descr;

    auto& cf5_defs = cf5;
    cf5_defs .DEF(pfx) .DEF(vl0) .DEF(ilo) .DEF(ii) .DEF(jlo) .DEF(jj) ;

    bool const iifirst = fd0.find("cf5_iifirst") == nullptr;
    fd0["cf5_iifirst"].setType("TAG");

    if(iifirst){
        fd>>OSSFMT("_ve_lvl(vl0);  // VL="<<vl0<<" jj%vl0="<<jj%vl0<<" iijj%vl0="
                <<iijj%vl0<<" iifirst="<<iifirst);
    }

    //
    // ------------- helper lambdas ------------
    //
    // some kernels might ask for const sq reg to always be defined
    auto have_sq = [&outer](){ return outer.find("have_sq") != nullptr; };
    auto have_sqij = [&fd](){ return fd.find("have_sqij") != nullptr; };
    auto use_sq = [&outer,&outer_fd,&fd,have_sq,have_sqij,&oss](){
        // if nec, define const sequence register "sq"
        if(!have_sq()){
            outer_fd["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = _ve_vseq_v();"<<" // sq[i]=i");
            outer["have_sq"].setType("TAG");
            if(have_sqij()){ // just in case it was _ve_vseq_v()...
                fd["last"]["sqij"].set(OSSFMT(left<<setw(40)<<"__vr sqij = sq;"<<" // sqij[i]=i"));
            }
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
    //auto equ_sq = [have_sq,have_sqij](std::string v){
    //    return v+(have_sq() ? " = sq;          "
    //            : have_sqij() ? " = sqij;        " // only OK pre-loop !
    //            : " = _ve_vseq_v();"); };
    auto use_iijj = [&fp,&iifirst,&iijj,&ii,&jj,&oss](){
        string decl=(iifirst? "uint64_t ":"");
        if(!fp.find("iijj")){
            auto instr=OSSFMT(decl<<"iijj=(uint64_t)ii*(uint64_t)jj;");
            fp["iijj"]>>OSSFMT(left<<setw(40)<<instr
                    <<" // iijj="<<ii<<"*"<<jj<<"="<<iijj);
        }
    };
    auto kernComment = [have_sq,ii,jj](){
        return have_sq()? "sq[]=0.."+jitdec(ii*jj-1): "";
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
                >>OSSFMT(left<<setw(40)<<vREG+"b = sq"<<" // b[i] = i");
        }else{ // note: mk_divmod also optimizes positivePow2(jj) case
            mk_divmod();
            use_sq();
            cb>>"__vr a,b;";
            string divmod;
            if(ilo==0) divmod = OSSFMT("DIVMOD_"<<jj<<"(sq, a, b);");
            else       divmod = OSSFMT("DIVMOD_"<<jj<<"(_ve_vaddul_vsv("<<ilo<<",sq), a, b);");
            cb>>OSSFMT(left<<setw(40)<<divmod<<" // a[]=sq/"<<jj<<" b[]=sq%"<<jj);
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
    }
    // krn_needs.cnt also gets some more treatement
    if(nloop==1){
        auto& fk = cf5;
        if(krn_needs.cnt){
            fk["last"]>>"cnt=vl0;";
        }
        cf5_kernel(fk,fd,fz, ilo,ii,jlo,jj,vl, kernComment(), which,
                pfx,0,"a","b","sq","vl0");
    }else if(nloop>1){
        //CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; cnt<iijj; cnt+=vl0)",inner);
        string loop_ab_string;
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
            ff  >>OSSFMT(left<<setw(40)<<(fd.find("have_cnt")
                        ?"vl = (vl0<iijj-cnt? vl0: iijj-cnt);"
                        :"vl = (cnt<vl0? cnt: vl0);")
                    //<<"// vl = min(vl0,remain)"
                    <<" // iijj="<<iijj/vl0<<"*vl0+"<<iijj%vl0
                    );
            ff  >>"_ve_lvl(vl);";
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
        use_iijj();
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
                    >>"    b = _ve_vadduw_vsv(vl0,b);           // b[] += vl0 (easy, a[] unchanged)"
                    >>"}else{"
                    >>"    a = _ve_vadduw_vsv(1,a);             // a[] += 1"
                    >>"    b = sq;                              // b[] = sq[] (reset case)"
                    >>"}";
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
            string instr = "sqij = _ve_vaddul_vsv(vl0,sqij);";
            fi>>OSSFMT(left<<setw(40)<<instr<<" // sqij[i] += "<<vl0);
            if(fp_sets_ab){
                auto divmod = OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b);");
                cout<<divmod<<endl;
                fi>>OSSFMT(left<<setw(40)<<divmod
                        <<" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
            }
        }
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
    inner["first"]; // --> 'fd' inner setup code used by all splits

    Lpi const ii = lsii.end - lsii.z;
    Lpi const jj = lsjj.end - lsii.z;
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
    }
    return pr.str();
}

std::string cf5_no_unrollX(Lpi const vlen, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const opt_t, int const which/*=WHICH_KERNEL*/, char const* ofname/*=nullptr*/)
{
    cout<<" cf5_no_unrollX(vlen="<<vlen<<","<<lsii<<","<<lsjj<<",...)"<<endl;

    string program = cf5_no_unroll((opt_t==3?(int)vlen:-(int)vlen),
            lsii,lsjj,which,1/*verbose*/);
    // wrap 'program' up with some boilerplate...
    ostringstream oss;
    oss<<"// Autogenerated by "<<__FILE__<<" cf5_no_unrollX(vlen="<<vlen
        <<","<<lsii<<","<<lsjj<<",...)\n";
    oss<<"// Possible compile:\n";
    oss<<"//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c"<<endl;
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

