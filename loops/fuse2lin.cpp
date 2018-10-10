/** \file
 * \ref fuse2.cpp shows 2-loop vectorization induction using
 * a vector of a[], b[] loop variable pairs.
 *
 * In this file we test induction on arbitrary linear combinations
 * of the loop variables.    Oh. It turns out the full recalc is
 * probably the way to go.  Induction on linear combination doesn't
 * save ops.
 *
 * So linear comb can be an option in fuse2, after all.
 */
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstring>
#include <cstddef>
#include <cassert>

using namespace std;

typedef int Lpi; // Loop-index type
/** string up-to-n first, dots, up-to-n last of vector \c v[0..vl-1] w/ \c setw(wide) */
template<typename T>
std::string vecprt(int const n, int const wide, std::vector<T> v, int const vl){
    assert( v.size() >= vl );
    std::ostringstream oss;
    for(int i=0; i<vl; ++i){
        if( i < n ){ oss <<setw(wide)<< v[i]; }
        if( i == n && i < vl-n ){ oss<<" ... "; }
        if( i >= n && i >= vl-n ){ oss <<setw(wide)<< v[i]; }
    }
    return oss.str();
}
// Note other optimization might *pack* 2-loop indices differently!
// (e.g. u32 a[0]b[0] a[1]b[1] in a packed register)
// (e.g. single register with a[] concat b[] (for short double-loops)
// Here we just store a[], b[] indices in two separate registers
typedef uint64_t Vlpi; // vector-loop-index type
typedef std::vector<Vlpi> VVlpi;

/** Reference values for correct index outputs */
struct Vab{
    Vab( VVlpi const& asrc, VVlpi const& bsrc, int vl )
        : a(asrc), b(bsrc), vl(vl) {}
    VVlpi a;
    VVlpi b;
    int vl;    // 0 < Vabs.back().vl < vlen
};

/** Generate reference vectors of vectorized 2-loop indices */
std::vector<Vab> ref_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj,
                            int const verbose=1)
{
    std::vector<Vab> vabs; // fully unrolled set of reference pairs of a,b vector register

    VVlpi a(vlen), b(vlen);
    int v=0; // 0..vlen counter
    for(int64_t i=0; i<(int64_t)ii; ++i){
        for(int64_t j=0; j<(int64_t)jj; ++j){
            //cout<<"."; cout.flush();
            a[v] = i; b[v] = j;
            if( ++v >= vlen ){
                vabs.emplace_back( a, b, v );
                v = 0;
            } 
        }
    }
    cout<<vabs.size()<<" full loops of "<<vlen<<" with "<<v<<" left over"<<endl;
    if( v > 0 ){ // partial final vector
        for(int i=v; i<vlen; ++i) { a[i] = b[i] = 0; }
        vabs.emplace_back( a, b, v );
    }

    if(verbose){ // print ref result
        // pretty-printing via vecprt
        int const n=8; // output up-to-n [ ... [up-to-n]] ints
        int const bignum = std::max( ii, jj );
        int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

        for(int l=0; l<vabs.size(); ++l){
            auto const& a = vabs[l].a;
            auto const& b = vabs[l].b;
            auto const& vl = vabs[l].vl;
            cout<<"__"<<l<<endl;
            cout<<"a_"<<l<<"["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b_"<<l<<"["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }
    }
    return vabs;
}
/* Suppose for i:(0,ii){ for j:(o,jj) {} gets
 * gets vector indices \c a[] and \c b[]
 * with preferred vector length \c vlen.
 *
 * Suppose we always want to enter the loop body with a linear combination
 * of the vector indices, <tt>aa*i + bb*j + cc</tt>.
 *
 * We'll use \c a for outer index vector,
 *           \c b for inner index vector,
 *       and \c o [offset] for the linear combination aa*a[]+bb*b[]+cc
 */
void test_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj,
                 Lpi const aa, Lpi const bb, Lpi const cc){
    assert( vlen > 0 );
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == ((ii*jj) +vlen -1) / vlen);
    // generate reference set of offset vectors
    std::vector<std::vector<Lpi>> offs;
#define FOR(I,VL) for(int I=0;I<VL;++I)
    for(auto const& vab: vabs){
        std::vector<Lpi> offsets(vlen);
        auto const& a = vab.a;
        auto const& b = vab.b;
        auto const vl = vab.vl;
        FOR(i,vl) offsets[i] = aa*a[i] + bb*b[i] + cc;
        offs.push_back(offsets);
        cout<<"offs: "<<vecprt(n,wide+2,offsets,vl)<<endl;
    }
    for(int i=0; i<offs.size(); ++i){
        auto const vl = vabs[i].vl;
        cout<<"offs: "<<vecprt(n,wide+2,offs[i],vl)<<endl;
    }
    assert( offs.size() == vabs.size() );

    cout<<"Verify-------"<<endl;
    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    register int vl = vlen;
    register uint64_t cnt = 0UL;
    //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
    if (vl > iijj) vl = iijj;

#define FOR(I,VL) for(int I=0;I<VL;++I)
    if(0){
        cout<<"   ii="<<ii<<"   jj="<<jj<<"   iijj="<<iijj<<endl;
        //cout<<" vcnt="<<vcount<<" vcnt'"<<vcount_next<<endl;
        cout<<" vabs.size() = "<<vabs.size()<<endl;
        //cout<<"iloop="<<iloop<<" / "<<nloop<<endl;
        cout<<"   vl="<<vl<<endl;
        cout<<" cnt="<<cnt<<" iijj="<<iijj<<endl;
    }
    VVlpi a(vl), b(vl), o(vl); // loop body vectors
    VVlpi bA(vl), bM(vl), bD(vl), aA(vl), sq(vl); // tmp vectors
    int iloop = 0; // mostly for debug checks, now;
    //for( ; iloop < nloop; ++iloop )
    for( ; cnt < iijj; cnt += vl )
    {
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if (iloop == 0){
            // now load the initial vector-loop registers:
            FOR(i,vl) sq[i] = i;         // vseq_v
            FOR(i,vl) b [i] = sq[i] % jj;
            FOR(i,vl) a [i] = sq[i] / jj;
            // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
            //     which has some optimizations for nice values of jj.
            // is not induced, so calc below.
        }else{
            // 2. Induction from a->ax, b->bx
            FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
            FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv
            FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
            // Note: for some jj,
            //       the bM,bD divmod operation can be OPTIMIZED to rot etc.
            VVlpi aA(vl); FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
            if(0){
                cout<<"I:bA  "<<vecprt(n,wide+2,bA,vl)<<" <-- b+vl"<<endl;
                cout<<"I:bM  "<<vecprt(n,wide+2,bM,vl)<<" <-- bA%jj = b'"<<endl;
                cout<<"I:bD  "<<vecprt(n,wide+2,bD,vl)<<" <-- bA/jj"<<endl;
                cout<<"I:aA  "<<vecprt(n,wide+2,aA,vl)<<" <-- a+:!bD ???"<<endl;
            }
            FOR(i,vl) b[i] = bM[i]; // bNext is bM
            FOR(i,vl) a[i] = aA[i]; // aNext is aA
            // full formula TODO replace with induction (no need for cc)
            // induction test:
            // o[i] = aa*a[i] + bb*b[i] + cc;  4 ops
            // a[i+1] - a[i] = bD[i]
            // b[i+1] = bM[i]
            // o[i+1] = aa*a[i+1] + bb*bb[i] + cc;
            //        = aa*a[i] + aa*(a[i+1] - a[i])
            //        + bb*b[i] + bb*(b[i+1] - b[i])
            //        + cc
            //     = o[i] + aa*bD[i] + bb*(bM[i] - b[i]) 5 ops
            // Does not seem to be any big op count savings, and
            // saving having cc in register (for non-jit) is not
            // worth the added complexity.
        }
        // o[i] = aa*a[i] + bb*b[i] + cc;
        FOR(i,vl) aA[i] = aa * a[i];
        FOR(i,vl) bA[i] = bb * b[i];
        FOR(i,vl) o [i] = aA[i] + cc;
        FOR(i,vl) o [i] = o [i] + bA[i];

        // Note: vl reduction must take place AFTER above use of "long" vl
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
        //cout<<" cnt="<<cnt<<" vl="<<vl<<" iijj="<<iijj<<endl;

        cout<<"__"<<iloop<<endl;
        cout<<(iloop==0?"Init  ":"Induce")<<":  "<<vecprt(n,wide,a,vl)<<endl;
        cout<<"         "<<vecprt(n,wide,b,vl)<<endl;
        cout<<"       o "<<vecprt(n,wide+2,o,vl)<<endl;
        //cout<<"    offs "<<vecprt(n,wide+2,offs[iloop],vl)<<endl;
        assert( vl == vabs[iloop].vl );
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        FOR(i,vl) assert( o[i] == offs[iloop][i] );
        ++iloop; // just for above debug assertions
        //cout<<" next loop??? cnt+vl="<<cnt+vl<<" iijj="<<iijj<<endl;
#undef FOR
    }
    cout<<" Yay! induction formulas worked!"<<endl;
}

/** opt0: print vector ops (and verify) */
void test_vloop2_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    register int vl = vlen;
    register uint64_t cnt=0;
    register uint64_t nxt;
    cout<<"=== // unrolled regs:"<<endl;
    cout<<"=== //        %iijj    : scalar : outer * inner fused-loop count"<<endl;
    cout<<"=== //        %cnt     : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %vl      : scalar : vector length register"<<endl;
    cout<<"=== //        %a, %b   : vector : outer, inner loop indices"<<endl;
    cout<<"=== //        %bA, %bD : vector : tmp regs"<<endl;
    //if (vl > iijj) vl = iijj; //in general: if (cnt+vl > iijj) vl=iijj-cnt;
    // later! cout<<"===   lea %vl, "<<vlen<<"(,0) // initial vector len"<<endl;
    cout<<"===   lea %iijj, 0,"<<ii<<"(,0)"<<endl;
    cout<<"===   lea %vl,   0,"<<jj<<"(,0) // vl used as tmp reg"<<endl;
    cout<<"===   mulu.l %iijj, %iijj, %vl  // opt last 3 for small ii, jj !"<<endl;
    cout<<"===   or %cnt, 0, 0(,0)"<<endl;

    VVlpi a(vl), b(vl);   // vectorized loop indices
    VVlpi bA(vl), bD(vl); // local temporaries
#define FOR(I,VL) for(int I=0;I<VL;++I)
    int iloop = 0; // just for debug checks, now;
    for( ; cnt < iijj; cnt += vl)
    {
        if (cnt == 0){
            // now load the initial vector-loop registers:
            FOR(i,vl) bA[i] = i;         // vseq_v
            cout<<"===   vseq %bA           // (tmp)"<<endl;
            FOR(i,vl) b [i] = bA[i] % jj;
            FOR(i,vl) a [i] = bA[i] / jj;
            cout<<"===   vmod %b, %bA, "<<jj<<"     // b = bA % jj"<<endl;
            cout<<"===   vdiv %a, %bA, "<<jj<<"     // a = bA / jj"<<endl;
        }else{
            // 2. Induction from a->ax, b->bx
            FOR(i,vl) bA[i] = vl + b[i];      // bA = b + vl; add_vsv
            FOR(i,vl) b [i] = bA[i] % jj;     // bM = bA % jj; mod_vsv
            FOR(i,vl) bD[i] = bA[i] / jj;     // bD = bA / jj; div_vsv
            FOR(i,vl) a [i] = a[i] + bD[i];   // aA = a + bD; add_vvv
            cout<<"===   vadd %bA, %vl, %b     // bA[i] = vl + b[i] (tmp)"<<endl;
            cout<<"===   vmod %b , %bA, "<<jj<<"     // b[i] = bA[i] % jj"<<endl;
            cout<<"===   vdiv %bD, %bA, "<<jj<<"     // bD[i] = bA[i]/ jj (tmp)"<<endl;
            cout<<"===   vadd %a , %a , %bD    // a[i] += bD[i]"<<endl;
        }

        nxt = cnt+vl;
        if( nxt > iijj){ // last loop!
            vl = iijj - cnt;
            if (cnt > 0){
                cout<<"=== lea    %vl, "<<vl<<"(,0)  // LAST TIME"<<endl;
                cout<<"=== svl    %vl"<<endl;
            }
        }

        //cout<<"==="<<endl;
        cout<<"=== // vec_loop2 unroll#"<<iloop<<", indices in %a, %b"<<endl;
        //cout<<"==="<<endl;

        cout<<"__"<<iloop<<endl;
        cout<<"Induce:      "<<vecprt(n,wide,a,vl)<<endl;
        cout<<"             "<<vecprt(n,wide,b,vl)<<endl;
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        ++iloop; // just for above debug assertions
#undef FOR
    }
    cout<<" Yay! induction formulas worked!"<<endl;
}
void test_vloop2_no_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    cout<<"=== // no-unroll regs (generic loop):"<<endl;
    cout<<"=== //        %iijj    : scalar : outer * inner fused-loop count"<<endl;
    cout<<"=== //        %cnt     : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %vl      : scalar : vector length register"<<endl;
    cout<<"=== // Oh. Fully generic would need ii and jj in scalar regs too"<<endl;
    cout<<"=== //                 (ii could be re-used for iijj)"<<endl;
    cout<<"=== //        %a, %b   : vector : outer, inner loop indices"<<endl;
    cout<<"=== //        %bA, %bD : vector : tmp regs"<<endl;
    cout<<"=== // scalar init:"<<endl;
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    register int vl = vlen;
    register uint64_t cnt = 0UL;
    if (vl > iijj) vl = iijj; //in general: if (cnt+vl > iijj) vl=iijj-cnt;
    cout<<"===   lea %iijj, 0,"<<ii<<"(,0)"<<endl;
    cout<<"===   lea %vl,   0,"<<jj<<"(,0) // vl used as tmp reg"<<endl;
    cout<<"===   mulu.l %iijj, %iijj, %vl  // opt last 3 for small ii, jj !"<<endl;
    cout<<"===   or %cnt, 0, 0(,0)"<<endl;
    cout<<"===   lea %vl, "<<vlen<<"(,0) // initial vector len"<<endl;

    register uint64_t nxt; // loop variable (convenience) XXX
#define FOR(I,VL) for(int I=0;I<VL;++I)
    cout<<"=== // vector init:"<<endl;
    VVlpi a(vl), b(vl);   // vectorized loop indices
    VVlpi bA(vl), bD(vl); // local temporaries
    // now load the initial vector-loop registers:
    FOR(i,vl) bA[i] = i;         // vseq_v
    FOR(i,vl) b [i] = bA[i] % jj;
    FOR(i,vl) a [i] = bA[i] / jj;
    cout<<"===   vseq %bA           // (tmp)"<<endl;
    cout<<"===   vmod %b, %bA, "<<jj<<"     // b = bA % jj"<<endl;
    cout<<"===   vdiv %a, %bA, "<<jj<<"     // a = bA / jj"<<endl;
    cout<<"===   br loop_enter"<<endl;
    int iloop = 0; // just for debug checks, now;
    cout<<"=== again:              // <-- repeat loop (induce loop var)"<<endl;
again:
    nxt = cnt+vl;
    if( nxt > iijj) vl = iijj - cnt;
    if (iloop == 0){ // only need to print stuff 1st time through loop
        cout<<"===   addu.l %tmp, %cnt, %vl"<<endl;
        cout<<"===   subu.l %tmp, %tmp, %iijj"<<endl;
        cout<<"===   br.lt %tmp, L_1f-."<<endl;
        cout<<"===     or %cnt, 0, 1(0)  // re-use cnt to signal loop-end"<<endl;
        cout<<"===     subu %vl, %iijj, %cnt"<<endl;
        cout<<"===     svl  %vl"<<endl;
        cout<<"=== L_1f:"<<endl;
    }

    //cout<<"==="<<endl;
    cout<<"=== // vec_loop2 #"<<iloop<<", indices in %a, %b"<<endl;
    //cout<<"==="<<endl;

    cout<<"__"<<iloop<<endl;
    cout<<"Induce:      "<<vecprt(n,wide,a,vl)<<endl;
    cout<<"             "<<vecprt(n,wide,b,vl)<<endl;
    FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
    FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
    cnt += vl; // or cnt = nxt;
    if (cnt >= iijj) goto done;
    if (iloop == 0){
        cout<<"=== // if ((cnt+=vl) > iijj) goto done;"<<endl;
        cout<<"===   addu.l %cnt, %cnt, %vl"<<endl;
        cout<<"===   subu.l %tmp, %cnt, %iijj"<<endl;
        cout<<"===   br.ge done"<<endl;
    }
    // 2. Induction from a->ax, b->bx
    FOR(i,vl) bA[i] = vl + b[i];      // bA = b + vl; add_vsv
    FOR(i,vl) b [i] = bA[i] % jj;     // bM = bA % jj; mod_vsv
    FOR(i,vl) bD[i] = bA[i] / jj;     // bD = bA / jj; div_vsv
    FOR(i,vl) a [i] = a[i] + bD[i];   // aA = a + bD; add_vvv
    if (iloop==0){
        cout<<"=== // Induce next a, b vector loop indices"<<endl;
        cout<<"===   vadd %bA, %vl, %b     // bA[i] = vl + b[i]"<<endl;
        cout<<"===   vmod %b , %bA, "<<jj<<"   // b[i] = bA[i] % jj"<<endl;
        cout<<"===   vdiv %bD, %bA, "<<jj<<"   // bD[i] = bA[i]/ jj"<<endl;
        cout<<"===   vadd %a , %a , %bD    // a[i] += bD[i]"<<endl;
    }
    ++iloop; // just for above debug assertions
    goto again;
#undef FOR
done:
    cout<<"=== done:"<<endl;
    cout<<" Yay! induction formulas worked!"<<endl;
}
int main(int argc,char**argv){
    int vl = 8;
    int ii=20, jj=3;
    int aa=100, bb=2, cc=1;
    int a=0, opt_h=0;
    if(argc > 1){
        if (argv[1][0]=='-'){
            char *c = &argv[1][1];
            if (*c=='h'){
                cout<<" fuse2lin VLEN I J A B C"<<endl;
                cout<<"   VLEN = vector length"<<endl;
                cout<<"   I    = 1st loop a=0..I-1"<<endl;
                cout<<"   J    = 2nd loop b=0..J-1"<<endl;
                cout<<" double loop --> loop over vector registers a[VLEN], b[VLEN]"<<endl;
                cout<<" A,B,C  = calc lin comb Aa + Bb + C"<<endl;
                opt_h = 1;
            }
            ++a; // one extra arg was used up
        }
    }
    if(argc > a+1) vl = atof(argv[a+1]);
    if(argc > a+2) ii = atof(argv[a+2]);
    if(argc > a+3) jj = atof(argv[a+3]);
    if(argc > a+4) aa = atof(argv[a+4]);
    if(argc > a+5) bb = atof(argv[a+5]);
    if(argc > a+6) cc = atof(argv[a+6]);
    cout<<"vlen="<<vl<<", ii="<<ii<<", jj="<<jj<<", aa="<<aa<<", bb="<<bb<<", cc="<<cc<<endl;
    test_vloop2(vl,ii,jj,aa,bb,cc);
    //test_vloop2_unroll(vl,h,w);
    //test_vloop2_no_unroll(vl,h,w); // C++ code more like asm generic loop
    cout<<"\nGoodbye"<<endl;
    return 0;
}
