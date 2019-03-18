/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>

/** \group VE asm macros
 * - V   : vector register, 1st arg ~ 'final output'
 * - VT  : temporary vector register
 * - S   : scalar register
 * - VS  : vector or scalar register
 * - CVP : C void pointer  (if expr, may generate longish addr loads that are better precalculated)
 * - %s13,%s12,%s18,%s19 scratch registers
 * - Assume these assembler inline are non-recursive,
 *   so scratch regs can be re-used. Seq macros OK.
 */
//@{
#define STR0(s...) #s
#define STR(s...) STR0(s)
/** ? override to route assembler code elsewhere? */
#define ASM(A,X...) asm(A X)
#define DOW(X...) do{X}while(0)

#define s_negone(S)         ASM(STR(eqv S, 0,(0)1));
#define s_zero(S)           ASM(STR(xor S, 0,(0)1));
#define s_addr_c(S,CVP)     ASM(STR(or S,0,%0)          ,::"r"(CVP):STR(S))

//#define pv_seq(V)		    ASM(STR(pvseq %V)           ,)
#define lea_c(S,CVP)        DOW(void* cvp=CVP; ASM(STR(or  S,0,%0) ,::"r"(cvp):STR(S)); );
#define lea_cx(S,CVP)       ASM(STR(or  S,0,%0) ,::"r"(CVP):STR(S) );
#define lvl(N)              ASM(STR(lea %s13,N\n\t) \
        /*                   */ STR(lvl %s13)           ,:::"%s13")
/** load 4*64 bits at Saddr into VM */
#define vm_ld(VM,Saddr) ASM(STR(ld %s12, 0(,Saddr)\n\t) STR(lvm VM,0,%s12\n\t) \
        /*               */ STR(ld %s13, 8(,Saddr)\n\t) STR(lvm VM,1,%s13\n\t) \
        /*               */ STR(ld %s18,16(,Saddr)\n\t) STR(lvm VM,2,%s18\n\t) \
        /*               */ STR(ld %s19,24(,Saddr)\n\t) STR(lvm VM,3,%s19\n\t) \
        /*               */ ,:::"%s12","%s13","%s18","%s19",STR(VM))
#define vm_st(VM,Saddr) ASM(STR(svm %s12,VM,0\n\t) STR(st %s12, 0(,Saddr)\n\t) \
        /*               */ STR(svm %s13,VM,1\n\t) STR(st %s13, 8(,Saddr)\n\t) \
        /*               */ STR(svm %s18,VM,2\n\t) STR(st %s18,16(,Saddr)\n\t) \
        /*               */ STR(svm %s19,VM,3\n\t) STR(st %s19,24(,Saddr)\n\t) \
        /*               */ ,:::"%s12","%s13","%s18","%s19","memory")
/** load 4*64 bits at Saddr into VMa, and the next 4*64 bits into VMb.
 * For pack vector masks, VMa should be an even numbered Vector Mask register,
 * and VMb should be the next higher one. */
#define vm_ld2(VMa,VMb,Saddr) ASM(STR(ld %s12, 0(,Saddr)\n\t) STR(lvm VMa,0,%s12\n\t) \
        /*                     */ STR(ld %s13, 8(,Saddr)\n\t) STR(lvm VMa,1,%s13\n\t) \
        /*                     */ STR(ld %s12,16(,Saddr)\n\t) STR(lvm VMa,2,%s12\n\t) \
        /*                     */ STR(ld %s13,24(,Saddr)\n\t) STR(lvm VMa,3,%s13\n\t) \
        /*                     */ STR(ld %s12,32(,Saddr)\n\t) STR(lvm VMb,0,%s12\n\t) \
        /*                     */ STR(ld %s13,40(,Saddr)\n\t) STR(lvm VMb,1,%s13\n\t) \
        /*                     */ STR(ld %s12,48(,Saddr)\n\t) STR(lvm VMb,2,%s12\n\t) \
        /*                     */ STR(ld %s13,56(,Saddr)\n\t) STR(lvm VMb,3,%s13\n\t) \
        /*                     */ ,:::STR(%s12),STR(%s13),STR(VMa),STR(VMb))
#define vm_st2(VMa,VMb,Saddr) ASM( \
        STR(svm %s12,VMa,0\n\t) STR(st %s12, 0(,Saddr)\n\t) \
        STR(svm %s13,VMa,1\n\t) STR(st %s13, 8(,Saddr)\n\t) \
        STR(svm %s18,VMa,2\n\t) STR(st %s18,16(,Saddr)\n\t) \
        STR(svm %s19,VMa,3\n\t) STR(st %s19,24(,Saddr)\n\t) \
        STR(svm %s12,VMb,0\n\t) STR(st %s12,32(,Saddr)\n\t) \
        STR(svm %s13,VMb,1\n\t) STR(st %s13,40(,Saddr)\n\t) \
        STR(svm %s18,VMb,2\n\t) STR(st %s18,48(,Saddr)\n\t) \
        STR(svm %s19,VMb,3\n\t) STR(st %s19,56(,Saddr)\n\t) \
        ,:::"%s12","%s13","%s18","%s19","memory")

/* V = (VM? Va, VSb) -- ternary vector select, for i=1..VL */
#define v_mrg(V, VM,Va,VSb) ASM(STR(vmrg.l V,VSb,Vai, VM)   ,:::STR(V))
/* V =((i&1)==0)? (VM(m)[i]? Va[i] : S or VSb[i])
 *            : (VM(m+1)[i]? Va[i] : S or VSb[i]) */
#define pv_mrg(V, VMpair,Va,VSb) ASM(STR(vmrg.w V,VSb,Va, VMpair)  ,:::STR(V))

#define pv_seq_1032(V)      ASM(STR(pvseq V)           ,:::STR(V)) /*produce 1 0  3 2  5 4 ...*/
#define pv_swab(V,Vin)      ASM(STR(vshf V,Vin,Vin,4) ,:::STR(V)) /* 0 1 2 3 4 5...-> 1 0 3 2 5 4... */
#define pv_seq(V,VTswab)    DOW(pv_seq_1032(VTswab); \
        /*                   */ pv_swab(V,VTswab); )
#define pv_cvt_flt(V)       ASM(STR(pvcvt.s.w %V,%V)    ,:::STR(V))
#define pv_ld_c(V,CVP)      ASM(STR(vld V,8,%0)        ,::"r"(CVP):STR(V))
#define pv_st_c(V,CVP)      ASM(STR(vst V,8,%0)        ,::"r"(CVP):"memory")
#define pv_ld(V,S)          ASM(STR(vld V,8,S)         ,:::STR(V))    /* 8 ~ strided Sy==1 */
#define pv_st(V,S)          ASM(STR(vst V,8,S)         ,:::"memory")
#define pv_rotl_1(V,Vin,VTrot2) ASM(STR(vmv VTrot2,1,Vin\n\t) \
        /*                       */ STR(vshf V, VTrot2,Vin, 6\n\t) \
        /*                       */ ,:::STR(V));
#define pv_rotl_2(V,Vin)    ASM(STR(vmv %V,1,%Vin)      ,:::STR(V));
#define pv_rotl_2n(N,V,Vin) ASM(STR(vmv %V,N,%Vin)      ,:::STR(V));
#define pv_rotl_2nm1(N,V,Vin, VT0,VT1) ASM(STR(vmv VT0,N-1,Vin\n\t) /* Vin rotl 2N */ \
        /*                              */ STR(vmv VT1,N,Vin\n\t)   /* Vin rotl 2N-2 */ \
        /*                              */ STR(vshf V, VT1,VT0,6\n\t) /* shuffle */ \
        /*                              */ ,:::STR(V),STR(VT0),STR(VT1));
// assuming lvl(256)... rotl and set last 2 elements [510,511] to 64-bit 0
#define pv_movl_2_z255(V,Vin) ASM(STR(vmv V,1,Vin\n\t) \
        /*                 */ STR(lea %s13,0\n\t) \
        /*                 */ STR(lea %s12,255\n\t) \
        /*                 */ STR(lsv V(%s12),%s13\n\t) \
        /*                 */ ,:::"%s13",STR(V))
#define pv_rotr_2(V,Vin, Snegone) ASM(STR(vmv V,Snegone,Vin\n\t) \
        /*                         */ ,:::STR(V));
#define pv_rotr_1(V,Vin, Vrotr2,Snegone) ASM( \
        /*          */ STR(vmv Vrotr2,Snegone,Vin\n\t) \
        /*          */ STR(vshf V, Vin,Vrotr2,6\n\t) \
        /*          */ ,:::STR(V),STR(Vrotr2));
#define pv_movr_2(V,Vin, Snew64,Snegone) ASM(STR(vmv %V,Snegone,Vin\n\t) \
        /*                                */ STR(lsv V(0),Snew64\n\t) \
        /*                                */ ,:::"%s13",STR(V)) // ??
#define pv_clr_f0(V)        ASM(STR(lvs %s13,V(0)\n\t) \
        /*                   */ STR(and %s13,%s13,(32)1\n\t) \
        /*                   */ STR(lsv V(0),%s13) \
        /*                   */ ,:::"%s13",STR(V))
/** 0 1 ... 511 +---> 0 0 1 ... 510 */
#define pv_movr_1z(V,Vin, Vrotl2,Snegone) ASM(STR(vmv Vrotl2,Snegone,Vin\n\t) \
        /*                   */ STR(vshf V, Vin,Vrotl2,6\n\t) \
        /*                   */ STR(lvs %s13,V(0)\n\t) \
        /*                   */ STR(and %s13,%s13,(32)1\n\t) \
        /*                   */ STR(lsv V(0),%s13) \
        /*                   */ ,:::"%s13",STR(V))
//@}

struct Msk {
/* 1001001001001001001001001001001001001001001001001001001001001001 0010010010010010010010010010010010010010010010010010010010010010 0100100100100100100100100100100100100100100100100100100100100100 1001001001001001001001001001001001001001001001001001001001001001 */
static uint64_t constexpr every3rd_0[4] = { 0x9249249249249249, 0x2492492492492492, 0x4924924924924924, 0x9249249249249249};

/* 0100100100100100100100100100100100100100100100100100100100100100 1001001001001001001001001001001001001001001001001001001001001001 0010010010010010010010010010010010010010010010010010010010010010 0100100100100100100100100100100100100100100100100100100100100100 */
static uint64_t constexpr every3rd_1[4] = { 0x4924924924924924, 0x9249249249249249, 0x2492492492492492, 0x4924924924924924};

/* 0010010010010010010010010010010010010010010010010010010010010010 0100100100100100100100100100100100100100100100100100100100100100 1001001001001001001001001001001001001001001001001001001001001001 0010010010010010010010010010010010010010010010010010010010010010 */
static uint64_t constexpr every3rd_2[4] = { 0x2492492492492492, 0x4924924924924924, 0x9249249249249249, 0x2492492492492492};

static uint64_t constexpr every3rd_012[12] = {
    0x9249249249249249, 0x2492492492492492, 0x4924924924924924, 0x9249249249249249,
    0x4924924924924924, 0x9249249249249249, 0x2492492492492492, 0x4924924924924924,
    0x2492492492492492, 0x4924924924924924, 0x9249249249249249, 0x2492492492492492 };

static uint64_t constexpr everynthbit[64] = {
    0x0000000000000000, 0xffffffffffffffff, 0x5555555555555555, 0x2492492492492492, 0x1111111111111111, 0x0842108421084210, 0x0410410410410410, 0x0204081020408102,
    0x0101010101010101, 0x0080402010080402, 0x0040100401004010, 0x0020040080100200, 0x0010010010010010, 0x0008004002001000, 0x0004001000400100, 0x0002000400080010,
    0x0001000100010001, 0x0000800040002000, 0x0000400010000400, 0x0000200004000080, 0x0000100001000010, 0x0000080000400002, 0x0000040000100000, 0x0000020000040000,
    0x0000010000010000, 0x0000008000004000, 0x0000004000001000, 0x0000002000000400, 0x0000001000000100, 0x0000000800000040, 0x0000000400000010, 0x0000000200000004,
    0x0000000100000001, 0x0000000080000000, 0x0000000040000000, 0x0000000020000000, 0x0000000010000000, 0x0000000008000000, 0x0000000004000000, 0x0000000002000000,
    0x0000000001000000, 0x0000000000800000, 0x0000000000400000, 0x0000000000200000, 0x0000000000100000, 0x0000000000080000, 0x0000000000040000, 0x0000000000020000,
    0x0000000000010000, 0x0000000000008000, 0x0000000000004000, 0x0000000000002000, 0x0000000000001000, 0x0000000000000800, 0x0000000000000400, 0x0000000000000200,
    0x0000000000000100, 0x0000000000000080, 0x0000000000000040, 0x0000000000000020, 0x0000000000000010, 0x0000000000000008, 0x0000000000000004, 0x0000000000000002 };
};

/** VE bitmask helper. VE bitmasks are 4 uint64, where each is bit-reversed. */
struct Msk256 {

	uint64_t m[4];
	Msk256() { m[0]=0UL; m[1]=0UL; m[2]=0UL; m[3]=0UL; }
	static bool constexpr iok(int i) { return (i & ~255) == 0; }
	bool constexpr get(int i) const {
		assert(iok(i));
		return m[i/64] & (uint64_t{1}<<(63-i%64)); }
	bool constexpr get_(int i) const {
		//assert(iok(i));
		return m[i/64] & (uint64_t{1}<<(63-i%64)); }
	void set() { m[3]=m[2]=m[1]=m[0]= ~0UL; }
	void clr() { m[3]=m[2]=m[1]=m[0]=  0UL; }
	void set(int const i) { if(iok(i)){ set_(i); }}
    void set_(int const i) {
		int s=i/64, r=63-i%64;
		m[s] |= (uint64_t{1}<<r); }
	void clr(int const i) { if(iok(i)){ clr_(i); }}
    void clr_(int const i) {
		int s=i/64, r=63-i%64;
		m[s] &= ~((uint64_t{1}<<r)); }
	void set(int b, int e, int const by=1){
		if(b<e && by>0){
			while(b<0) b+=by;
			e = std::min(e,256);
            set_(b,e,by);
        }}
    void set_(int b, int const e, int const by=1){
        for( ; b<e; b+=by){
            int s=b/64, r=63-b%64;
            m[s] |= uint64_t{1}<<r; }}
	void clr(int b, int e, int by=1){
		if(b<e && by>0){
			while(b<0) b+=by;
			e = std::min(e,256);
            clr_(b, e, by);
        }}
    void clr_(int b, int const e, int const by=1){
        for(; b<e; b+=by){
            int s=b/64, r=63-b%64;
            m[s] &= ~(uint64_t{1}<<r); }}
	void setevery(int const n, int b=-1){
		while(b<0) b+=n;
        setevery_(n,b); }
    void setevery_(int const n, int b){ //b>0 n!=0
		if(n) for(; iok(b); b+=n){
            int s=b/64, r=63-b%64;
			m[s] |= uint64_t{1}<<r; }}
	void clrevery(int const n, int b=-1){
		while(b<0) b+=n;
        clrevery_(n, b); }
    void clrevery_(int const n, int b){
		if(n) for(; iok(b); b+=n){
			int s=b/64, r=63-b%64;
			m[s] &= uint64_t{1}<<r; }}
	void shr(int const n){
		if(n==0) return;
		if(n<0) shl(n);
		int d=n/64, o=n%64, r=64-o, rmask=((1<<r)-1);
		if(d==0){
			m[3] = ((m[3]<<o)) | ((m[2]>>r)&rmask);
			m[2] = ((m[2]<<o)) | ((m[1]>>r)&rmask);
			m[1] = ((m[1]<<o)) | ((m[0]>>r)&rmask);
			m[0] = ((m[0]<<o));
		}else if(d==1){
			if(o==0){ m[3]=m[2]; m[2]=m[1]; m[1]=m[0];
			}else{
				m[3] = ((m[2]<<o)) | ((m[1]>>r)&rmask);
				m[2] = ((m[1]<<o)) | ((m[0]>>r)&rmask);
				m[1] = ((m[0]<<o));
			}
			m[0]=0UL;
		}else if(d==2){
			if(o==0){ m[3]=m[1]; m[2]=m[0];
			}else{
				m[3] = ((m[1]<<o)) | ((m[0]>>r)&rmask);
				m[2] = ((m[0]<<o));
			}
			m[1]=0UL; m[0]=0UL;
		}else if(d==3){
			if(o==0) m[3]=m[0];
			else m[3] = ((m[0]<<o));
			m[2] = 0UL;
			m[1] = 0UL;
			m[0] = 0UL;
		}
	}
	void shl(int n){
		assert(false);
	}
	Msk256& operator=(Msk256 const& y) = default;
	bool operator==(Msk256 const& y) { return m[0]==y.m[0] && m[1]==y.m[1] && m[2]==y.m[2] && m[3]==y.m[3]; }
	Msk256& operator|=(Msk256 const& y) { m[0]|=y.m[0]; m[1]|=y.m[1]; m[2]|=y.m[2]; m[3]|=y.m[3]; return *this;}
	Msk256& operator&=(Msk256 const& y) { m[0]&=y.m[0]; m[1]&=y.m[1]; m[2]&=y.m[2]; m[3]&=y.m[3]; return *this;}
	Msk256& mand(Msk256 const& y) { m[0]&=y.m[0]; m[1]&=y.m[1]; m[2]&=y.m[2]; m[3]&=y.m[3]; return *this; }
	Msk256& mandnot(Msk256 const& y) { m[0]&=~y.m[0]; m[1]&=~y.m[1]; m[2]&=~y.m[2]; m[3]&=~y.m[3]; return *this; }
	Msk256& mor(Msk256 const& y) { m[0]|=y.m[0]; m[1]|=y.m[1]; m[2]|=y.m[2]; m[3]|=y.m[3]; return *this; }
	Msk256& mornot(Msk256 const& y) { m[0]|=~y.m[0]; m[1]|=~y.m[1]; m[2]|=~y.m[2]; m[3]|=~y.m[3]; return *this; }
};
/** VE packed vector ops use 2 bitmasks bit-wise interleaved */
struct Msk512 {
	Msk256 a, b;
	Msk512() : a(), b() {}
	static bool constexpr iok(int i) { return (i & ~511) == 0; }
	bool get(int i) const {
        assert(iok(i));
        return (i&1)? b.get_(i>>1): a.get_(i>>1); }
	bool get_(int i) const {
        //assert(iok(i));
        return (i&1)? b.get_(i>>1): a.get_(i>>1); }
    void set() { a.set(); b.set(); }
    void clr() { a.clr(); b.clr(); }
    void set(int const i) { if(iok(i)){ set_(i); }}
    void set_(int const i){
        int x=i>>1; if((i&1)) b.set(x); else a.set(x); }
    void clr(int const i) { if(iok(i)){ clr_(i); }}
    void clr_(int const i){
        int x=i>>1; if((i&1)) b.clr(x); else a.clr(x); }
    void set(int bb, int e, int const by=1){
        if(bb<e && by>0){
            while(bb<0) bb+=by;
            e = std::min(e,512);
            set_(bb,e,by);
        }}
    void set_(int bb, int const e, int const by=1){
        for(; bb<e; bb+=by){
            int x=bb>>1; if((bb&1)) b.set_(x); else a.set_(x); }}
    void clr(int bb, int e, int const by=1){
        if(bb<e && by>0){
            while(bb<0) bb+=by;
            e = std::min(e,512);
            set_(bb,e,by);
        }}
    void clr_(int bb, int const e, int const by=1){
        for(; bb<e; bb+=by){
            int x=bb>>1; if((bb&1)) b.clr_(x); else a.clr_(x); }}
    void setevery(int const n, int bb=-1){
        while(bb<0) bb+=n;
        setevery_(n,bb); }
    void setevery_(int const n, int bb){ //b>0 n!=0
        std::cout<<" Msk512::setevery("<<n<<","<<bb<<") "; std::cout.flush();
        if(n) for(; iok(bb); bb+=n){
            int x=bb>>1; if((bb&1)) b.set_(x); else a.set_(x); }}
    void clrevery(int const n, int bb=-1){
        while(bb<0) bb+=n;
        clrevery_(n,bb); }
    void clrevery_(int const n, int bb){ //b>0 n!=0
        if(n) for(; iok(bb); bb+=n){
            int x=bb>>1; if((bb&1)) b.clr_(x); else a.clr_(x); }}
};

ostream& operator<<(ostream&os, Msk256 const& m256){
    for(int i=0; i<256; ++i){
        int s=i/64, r=63-i%64;
		os<<(s>0 && r==63?" ":"")
            <<((m256.m[s] & (uint64_t{1}<<r))? '1':'0'); }
	return os;
}

ostream& operator<<(ostream&os, Msk512 const& m512){
#if 1
    os<<"\na: "<<m512.a;
    os<<"\nb: "<<m512.a;
    for(int x=0; x<512; ++x){
        if(x%64==0) os<<"\n "<<x/64<<":";
        os<<(m512.get(x)? '1':'0');
    }
#else
	for(int x=0; x<256; ++x){
	       	int s=x/64, r=x%64;
		os<<((m512.a.m[s] & (uint64_t{1}<<r))? '1':'0')
		  <<((m512.b.m[s] & (uint64_t{1}<<r))? '1':'0');
	}
#endif
	return os;
}

#include <iomanip>
using namespace std;

void test_msk256_basic(){
	assert(!Msk256::iok(-2));
	assert(!Msk256::iok(-1));
	assert( Msk256::iok(0));
	assert( Msk256::iok(1));
	assert( Msk256::iok(63));
	assert( Msk256::iok(64));
	assert( Msk256::iok(255));
	assert(!Msk256::iok(256));
	assert(!Msk256::iok(257));
	Msk256 msk;
	for(int i=0; i<256; ++i) assert(msk.get(i)==0);
	cout<<"msk new -->\n"<<msk<<endl;
	msk.set(0);
	cout<<"msk.set(0) -->\n"<<msk<<endl;
	assert(msk.get(0)==1);
	msk.set(0,10);
	for(int i=0; i<10; ++i) if(msk.get(i)!=1){ cout<<" oops i="<<i<<endl;assert(msk.get(i)==1); }
	for(int i=10; i<256; ++i) if(msk.get(i)!=0){ cout<<" oops i="<<i<<endl; assert(msk.get(i)==0); }
	cout<<"msk.set(0,10) -->\n"<<msk<<endl;
	msk.set(20,30);
	msk.set(50,60);
	msk.set(100,110);
	msk.set(150,160);
	msk.set(200,210);
	msk.set(250,256);
	cout<<msk<<endl;
	msk.shr( 1); cout<<msk<<endl;
	msk.shr( 9); cout<<msk<<endl;
	msk.shr(10); cout<<msk<<endl;
	msk.shr(10); cout<<msk<<endl;
	msk.shr(10); cout<<msk<<endl;
	msk.shr(10); cout<<msk<<endl;
	msk.clr(); msk.setevery(1); cout<<msk.m[0]<<" "<<msk<<endl;
	msk.clr(); msk.setevery(2); cout<<msk.m[0]<<" "<<msk<<endl;
	msk.clr(); msk.setevery(3); cout<<msk.m[0]<<" "<<msk<<endl;
	msk.clr(); msk.setevery(4); cout<<msk.m[0]<<" "<<msk<<endl;
	msk.clr(); msk.setevery(5); cout<<msk.m[0]<<" "<<msk<<endl;
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}
void test_msk256_constants(){
    Msk256 msk;
    cout<<hex<<setfill('0');
    for(int b=0; b<3; ++b){
        msk.clr(); msk.setevery(3,b);
        cout<<"\n/* "<<msk<<" */"<<endl;
        cout<<"uint64_t every3rd_"<<b<<"[4] = "
            <<"{ 0x"<<setw(16)<<msk.m[0]
            <<", 0x"<<setw(16)<<msk.m[1]
            <<", 0x"<<setw(16)<<msk.m[2]
            <<", 0x"<<setw(16)<<msk.m[3]<<"};"<<endl;
    }
    cout<<"\nuint64_t every3rd_012[12] = {\n    ";
    for(int b=0; b<3; ++b){
        msk.clr(); msk.setevery(3,b);
        cout<<"0x"<<setw(16)<<msk.m[0]
            <<", 0x"<<setw(16)<<msk.m[1]
            <<", 0x"<<setw(16)<<msk.m[2]
            <<", 0x"<<setw(16)<<msk.m[3]
            <<(b==2? " };\n": ",\n    ");
    }
    cout<<dec<<setfill(' ');
    //
	cout<<"uint64_t everynthbit[64] = {";
	cout<<hex<<setfill('0');
	for(int i=0; i<64; ++i){
		if(i>0) cout<<",";
		cout<<( i%8==0? "\n    0x": " 0x");
		msk.clr(); msk.setevery(i);
		cout<<setw(16)<<msk.m[0];
	}
	cout<<" };"<<dec<<setfill(' ')<<endl;
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}
void test_vm_ldst()    
{
    cout<<" test vm_ld, vm_st:"<<endl;
    uint64_t src[4], tmp[4];
    for(uint64_t i=0UL; i<4UL; ++i) { src[i] = 0xdeadbeefUL; tmp[i]=0x0UL; }
    //void *pvsrc=&src[0]; void* pvtmp=&tmp[0];
    s_addr_c(%s0,&src[0]);
    vm_ld(%vm2,%s0);
    s_addr_c(%s1,tmp);
    vm_st(%vm2,%s1);
    for(int i=0; i<4; ++i){
        cout<<i<<"\t"<<src[i]<<"\t"<<tmp[i]<<endl;
    }
    for(int i=0; i<4; ++i){
        if(tmp[i] != src[i]){
            cout<<" i="<<i<<" tmp[i] != src[i] after vm_ld, vm_st"<<endl;
            assert( tmp[i] == src[i] );
        }
    }
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}
void test_msk256_vor(void* vpm0, void* vpm1, void* vpm2,
        void* vpa, void* vpb, void* vpc, void* vpd){
    lvl(256);
    s_addr_c(%s20,vpm0); vm_ld(%vm2,%s20);
    s_addr_c(%s21,vpm1); vm_ld(%vm4,%s21);
    s_addr_c(%s22,vpm2); vm_ld(%vm6,%s22);
#if 0
    asm("or %s20,0,%0" : : "r"(vpa) : "%s20" );
    asm("or %s21,0,%0" : : "r"(vpb) : "%s21" );
    asm("or %s22,0,%0" : : "r"(vpc) : "%s22" );
    asm("or %s23,0,%0" : : "r"(vpd) : "%s23" );
#else
    s_addr_c(%s20,vpa);
    s_addr_c(%s21,vpb);
    s_addr_c(%s22,vpc);
    s_addr_c(%s23,vpd);
#endif
    pv_ld(%v0,%s20);
#if 0 // just copy...
    asm("vor %v1,(0)1,%v0");
    asm("vor %v2,(0)1,%v0");
    asm("vor %v3,(0)1,%v0");
#else // masked u64 copy
    asm("vor %v1,(0)1,%v0,%vm2");
    asm("vor %v2,(0)1,%v0,%vm4");
    asm("vor %v3,(0)1,%v0,%vm6");
#endif
    pv_st(%v1,%s21);
    pv_st(%v2,%s22);
    pv_st(%v3,%s23);
#if 0
    asm("or %s20,0,%0" : : "r"(vpa) : "%s20" );
    asm("or %s21,0,%0" : : "r"(vpb) : "%s21" );
    asm("or %s22,0,%0" : : "r"(vpc) : "%s22" );
    pv_ld(%v0,%s20);
    pv_ld(%v1,%s21);
    pv_ld(%v2,%s22);

    asm("or %s23,0,%0" : : "r"(vpd) : "%s23" );
    asm("or %s24,0,%0" : : "r"(vpe) : "%s24" );
    asm("or %s25,0,%0" : : "r"(vpf) : "%s25" );
#endif
}
void test_msk256(){
    Msk256 m7,m30,m31,m32;
    if(1){
        cout<<" m7 (zero)\n"<<m7<<endl;
        m7.set();
        cout<<" m7.set()\n"<<m7<<endl;
        m7.clr();
        cout<<" m7.clr()\n"<<m7<<endl;
        m7.setevery(7,0);
        cout<<"m7 pattern 1000000 repeating?\n"<<m7<<endl;
        m7.setevery_(7,1); m7.setevery_(7,2);
        cout<<"m7 pattern 1110000 repeating?\n"<<m7<<endl;
        m30.setevery_(3,0);
        cout<<"m30 : "<<m30<<endl;
        m31.setevery_(3,1);
        cout<<"m31 : "<<m31<<endl;
        m32.setevery_(3,2);
        cout<<"m32 : "<<m32<<endl;
    }
#if 0
    void* vpm30=&m30.m[0]; s_addr_c(%s0,vpm30); vm_ld(%vm2,%s0);
    void* vpm31=&m31.m[0]; s_addr_c(%s1,vpm31); vm_ld(%vm4,%s1);
    void* vpm32=&m32.m[0]; s_addr_c(%s2,vpm32); vm_ld(%vm6,%s2);
#endif

    if(1){
        float a[512], b[512], c[512];
        float d[512], e[512], f[512];
        void *vpa=&a[0], *vpb=&b[0], *vpc=&c[0];
        void *vpd=&d[0], *vpe=&e[0], *vpf=&f[0];
        for(unsigned i=0U; i<512U; ++i){
            a[i]=1000+i;
            b[i]=c[i]=d[i]=e[i]=f[i]=0.f;
        }
        cout<<" "<<__PRETTY_FUNCTION__<<" initial print..."<<endl;
        for(int i=0; i<8; ++i)
            cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]
                <<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;

        test_msk256_vor(&m30.m[0], &m31.m[0], &m32.m[0],
                &a[0], &b[0], &c[0], &d[0]);

        cout<<" "<<__PRETTY_FUNCTION__<<" print..."<<endl;
        for(int i=0; i<20; ++i)
            cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]<<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;
        cout<<"...\n";
        for(int i=250; i<270; ++i)
            cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]<<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;
        cout<<"...\n";
        for(int i=500; i<512; ++i)
            cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]<<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;
        for(int i=0; i<256; ++i){
            //cout<<" i="<<i; cout.flush();
            if(i%3==0) { assert(b[2*i]==a[2*i] && b[2*i+1]==a[2*i+1]); }
            else       { assert(b[2*i]==0.f    && b[2*i+1]==0.f     ); }
            if(i%3==1) { assert(c[2*i]==a[2*i] && c[2*i+1]==a[2*i+1]); }
            else       { assert(c[2*i]==0.f    && c[2*i+1]==0.f     ); }
            if(i%3==2) { assert(d[2*i]==a[2*i] && d[2*i+1]==a[2*i+1]); }
            else       { assert(d[2*i]==0.f    && d[2*i+1]==0.f     ); }
        }
    }
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}
#if 1 // this mix of C / asm segfaults
void msk512_copy_ab( void* a, void* b, void* msk512data ){
    lvl(256);
    //void *sa=&s.a.m[0];
    //void *sb=&s.b.m[0];
    //s_addr_c(%s20,sa); vm_ld(%vm2, %s20);
    //s_addr_c(%s21,sb); vm_ld(%vm3, %s21);
    //
    s_addr_c(%s20,a);
    s_addr_c(%s21,b);
    //s_addr_c(%s22, msk512data);

    //vm_ld2(%vm2,%vm3, %s22);

    pv_ld(%v0,%s20);
    pv_st(%v0,%s21);
    //asm("pvor %v1, (0)1,%v0, %vm2");
    //s_addr_c(%s23,b);
    //pv_st(%v1,%s23);
}
void msk512_copy_cd( void* a, void* b, void* msk512data ){
    for(int i=0; i<256; ++i){
        ((uint64_t*)b)[i] = ((uint64_t*)a)[i];
    }
}
#endif
void msk512_copy(void* s, void*d){
        s_addr_c(%s20,s);
        s_addr_c(%s21,d);
        lvl(256);
        //vm_ld(%vm2, %s20);   // segfaul!
        //vm_st(%vm2, %s21);
        vm_ld2(%vm2,%vm3, %s20);
        vm_st2(%vm2,%vm3, %s21);
}
void test_msk512(){
    Msk512 m7,m30,m31,m32;
    if(1){
        cout<<" mask transfer test 1"<<endl;
        Msk512 s, d;
        s.clr();
        d.set();
        void* sa = &s.a.m[0];
        void* da = &d.a.m[0];
        //s_addr_c(%s0,&s.a.m[0]); vm_ld(%vm2, %s0); //vm_ld2(%vm2,%vm3, %s0);
        //s_addr_c(%s6,&d.a.m[0]); vm_st(%vm2, %s6); //vm_st2(%vm2,%vm3, %s7);
        //s_addr_c(%s20,sa);
        //s_addr_c(%s21,da);        // segfault!
        msk512_copy(sa,da);
        cout<<"s.a: "<<s.a<<endl;
        cout<<"d.a: "<<d.a<<endl;
        for(int i=0; i<4; ++i){
            if(d.a.m[i] != s.a.m[i])
                cout<<dec<<" issues with d.a.m["<<i<<"] s:"<<hex<<s.a.m[i]<<" d:"<<d.a.m[i]<<endl;
        }
        for(int i=0; i<4; ++i){
            if(d.b.m[i] != s.b.m[i])
                cout<<dec<<" issues with d.b.m["<<i<<"] s:"<<hex<<s.b.m[i]<<" d:"<<d.b.m[i]<<endl;
        }
        for(int i=0; i<256; ++i){
            if(d.a.get(i) != s.a.get(i)) cout<<" d.a.get("<<i<<") failed"<<endl;
            assert( d.a.get(i) == s.a.get(i));
        }
        if(0){
            lvl(256);
            s_addr_c(%s1,&s.b.m[0]); vm_ld(%vm3, %s1);
            s_addr_c(%s7,&d.b.m[0]); vm_st(%vm3, %s7); //vm_st2(%vm2,%vm3, %s7);
            for(int k=0; k<512; ++k){
                if(d.get(k) != s.get(k)) cout<<" d.get("<<k<<") failed "<<endl;
                assert( d.get(k) == s.get(k));
            }
        }
    }
    if(1){
        cout<<" mask transfer test 2"<<endl;
        Msk512 s, d;
        s.clr();
        d.set();
        //s_addr_c(%s0,&s.a.m[0]); vm_ld(%vm2, %s0); //vm_ld2(%vm2,%vm3, %s0);
        //s_addr_c(%s6,&d.a.m[0]); vm_st(%vm2, %s6); //vm_st2(%vm2,%vm3, %s7);
        void* sa = &s.a.m[0];
        assert( (uint64_t*)(&s.b.m[0]) == (uint64_t*)(&s.a.m[0]) + 4 );
        s_addr_c(%s20,sa);
        vm_ld2(%vm2,%vm3, %s20);
        vm_st2(%vm2,%vm3, %s21); // segfault in next C statement
        // no segfault at -O2
        for(int i=0; i<4; ++i){
            if(d.a.m[i] != s.a.m[i]) cout<<" issues with d.a.m["<<i<<"]"<<endl;
        }
        for(int i=0; i<4; ++i){
            if(d.b.m[i] != s.b.m[i]) cout<<" issues with d.b.m["<<i<<"]"<<endl;
        }
        for(int i=0; i<256; ++i){
            if(d.a.get(i) != s.a.get(i)) cout<<" d.a.get("<<i<<") failed"<<endl;
            assert( d.a.get(i) == s.a.get(i));
        }
        for(int i=0; i<256; ++i){
            if(d.b.get(i) != s.b.get(i)) cout<<" d.b.get("<<i<<") failed"<<endl;
            assert( d.b.get(i) == s.b.get(i));
        }
        for(int k=0; k<512; ++k){
            if(d.get(k) != s.get(k)) cout<<" d.get("<<k<<") failed "<<endl;
            assert( d.get(k) == s.get(k));
        }
    }
    if(1){
        float a[512], b[512];
        for(unsigned i=0U; i<512U; ++i){ a[i]=1000+i; b[i]=0.f; }
        cout<<"Begin bit influence test"<<endl; cout.flush();
        for(int i=0; i<10; ++i){
            for(unsigned z=0U; z<512U; ++z) b[z]=0.f;
            Msk512 s;
            s.clr();
            s.set(i);
            cout<<" s with bit "<<i<<" set :\n"<<s<<endl;
            if(1){
                void *pa=&a[0], *pb=&b[0], *pmsk512=&s.a.m[0];
                msk512_copy_ab(pa, pb, pmsk512 );
                msk512_copy_cd(pa,pb,pmsk512); // just to compare asm code
            }
            {
                int x=-1;
                std::vector<int> nz;
                for(int j=0; j<512; ++j){
                    if(b[j] != 0.f) nz.push_back(j);
                }
                cout<<" set a["<<i<<"] ==> change in output vector @ ";
                if(x==-1) cout<<"None?"<<endl;
                else for(auto j: nz) cout<<" "<<j;
                cout<<endl;
            }
        }
    }
    if(0){
        cout<<" m7 (zero)"<<m7<<endl;
        m7.set();
        cout<<" m7.set()"<<m7<<endl;
        m7.clr();
        cout<<" m7.clr()"<<m7<<endl;
        m7.setevery(7,0);
        cout<<"m7 pattern 1000000 repeating?"<<m7<<endl;
        m7.setevery_(7,1); m7.setevery_(7,2);
        cout<<"m7 pattern 1110000 repeating?"<<m7<<endl;
        //cout<<"m7 as uint64 binary"<<binary(m7.a[0])
        m30.setevery_(3,0);
        cout<<"m30 : "<<m30<<endl;
        m31.setevery_(3,1);
        cout<<"m31 : "<<m31<<endl;
        m32.setevery_(3,2);
        cout<<"m32 : "<<m32<<endl;
    }
    float a[512], b[512], c[512];
    float d[512], e[512], f[512];
    for(unsigned i=0U; i<512U; ++i){
        a[i]=1000+i;
        b[i]=c[i]=d[i]=e[i]=f[i]=0.f;
    }
    if(1){ // not working yet
#if 1
        s_addr_c(%s20,&m30.a.m[0]); vm_ld(%vm2, %s20);
        s_addr_c(%s21,&m30.b.m[0]); vm_ld(%vm3, %s21);
        s_addr_c(%s22,&m31.a.m[0]); vm_ld(%vm4, %s22);
        s_addr_c(%s23,&m31.b.m[0]); vm_ld(%vm5, %s23);
        s_addr_c(%s24,&m32.a.m[0]); vm_ld(%vm6, %s24);
        s_addr_c(%s25,&m32.b.m[0]); vm_ld(%vm7, %s25);
#else
        s_addr_c(%s20,&m30.a.m[0]); vm_ld2(%vm2,%vm3, %s20);
        s_addr_c(%s21,&m31.a.m[0]); vm_ld2(%vm4,%vm5, %s21);
        s_addr_c(%s22,&m32.a.m[0]); vm_ld2(%vm6,%vm7, %s22);
#endif

        s_addr_c(%s20,&a[0]);
        s_addr_c(%s21,&b[0]);
        s_addr_c(%s22,&c[0]);
        s_addr_c(%s23,&d[0]);

        pv_ld(%v0,%s20);
        asm("pvor %v1,(0)1,%v0,%vm2");
        asm("pvor %v2,(0)1,%v0,%vm4");
        asm("pvor %v3,(0)1,%v0,%vm6");
        pv_st(%v1,%s21);
        pv_st(%v2,%s22);
        pv_st(%v3,%s23);
    }
#if 0
    asm("or %s0,0,%0" : : "r"(vpa) : "%s0" );
    asm("or %s1,0,%0" : : "r"(vpb) : "%s1" );
    asm("or %s2,0,%0" : : "r"(vpc) : "%s2" );
    pv_ld(%v0,%s0);
    pv_ld(%v1,%s1);
    pv_ld(%v2,%s2);
    
    asm("or %s3,0,%0" : : "r"(vpd) : "%s3" );
    asm("or %s4,0,%0" : : "r"(vpe) : "%s4" );
    asm("or %s5,0,%0" : : "r"(vpf) : "%s5" );
#endif

    cout<<" "<<__PRETTY_FUNCTION__<<" print..."<<endl;
	for(int i=0; i<20; ++i)
		cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]<<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;
	cout<<"...\n";
	for(int i=250; i<270; ++i)
		cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]<<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;
	cout<<"...\n";
	for(int i=500; i<512; ++i)
		cout<<i<<":\t"<<setw(4)<<a[i]<<" "<<setw(4)<<b[i]<<" "<<setw(4)<<c[i]<<" "<<setw(4)<<d[i]<<" "<<setw(4)<<e[i]<<" "<<setw(4)<<f[i]<<endl;
    for(int i=0; i<512; ++i){
        //cout<<" i="<<i; cout.flush();
        if(i%3==0) { assert(b[i]==a[i]); }
        else       { assert(b[i]==0.f ); }
        if(i%3==1) { assert(c[i]==a[i]); }
        else       { assert(c[i]==0.f ); }
        if(i%3==2) { assert(d[i]==a[i]); }
        else       { assert(d[i]==0.f ); }
    }
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}
void test0(){
    // playground : was originally trying for 3-register tranpose,
    // but I mixed up left and right shifts and mask register u64 endianness.
    {
        unsigned u[512];
        lea_c(%s0,&u[0]);
        lvl(256);
        //pv_seq_1032(%v1);
        //pv_swab(%v2,%v1);
        pv_seq(%v2,%v1); // pvseq-->103254... then vshf-->012345...
        pv_st(%v2,%s0);

        for(int i=0U; i<10; ++i) cout<<" "<<u[i];
        cout<<" ... ";
        for(int i=508; i<512; ++i) cout<<" "<<u[i];
        cout<<endl;
    }
	float a[512], b[512], c[512];
	float d[512],e[512],f[512];
	void *vpa=&a[0], *vpb=&b[0], *vpc=&c[0];
	void *vpd=&d[0], *vpe=&e[0], *vpf=&f[0];
	for(unsigned i=0U; i<512U; ++i){
	       	a[i]=1000+i;
	       	b[i]=2000+i;
	       	c[i]=3000+i;
		d[i]=e[i]=f[i]=0;
	}
	Msk256 x, y, z;
	x.setevery(3,0);
	y.setevery(3,1);
	z.setevery(3,2);
	cout<<"x "<<x<<endl;
	cout<<"y "<<y<<endl;
	cout<<"z "<<z<<endl;
	uint64_t u=256;
	cout<<u<<endl;
	//asm("lea %s0,256");
	//asm("lvl %s0");

    // copy a-->c
	for(int i=0; i<512; ++i) d[i] = a[i]+6000;
    //for(int i=0; i<256; ++i)
    //    ((uint64_t*)vpd)[i] = ((uint64_t*)vpa)[i]; // vec copy as uint64
    asm("or %%s0,0,%0" : : "r"(vpa) : "%s0" );
    asm("or %%s1,0,%0" : : "r"(vpd) : "%s1" );
    //asm("lea %s13,256");
    //asm("lvl %s13");
    lvl(256);
    //asm("vld %v0, 8, %s0" );
    //asm("vst %v0, 8, %s1" );    // copy
    pv_ld(%v0,%s0);
    pv_st(%v0,%s0); /* read from %s0, write back to %s0 (should do nothing) */

	if(0){ // C equiv
		for(int i=0; i<512; ++i) e[i] = b[(i+1)%512];
	}else{
		// vpf_rotl_1
        register void *vpb=&b[0];
        register void *vpe=&e[0];
		asm("or %%s0,0,%0" : : "r"(vpb) : "%s0" );
		asm("or %%s1,0,%0" : : "r"(vpe) : "%s1" );
        lea_c(%s0,vpb);
        lvl(256);
		//asm("vld %v0, 8, %s0" );
        //pv_ldc(%v0,vpb);
        pv_ld(%v0,%s0);
#if 0
        //asm("vst %v0, 8, %s1" );    // copy
        pv_st_c(%v0,vpe);
#elif 0
		//asm("vxor %v2, %v2, %v2");
		//asm("pvseq %v2");                   // 0 1 2 3 ...
		//asm("pvcvt.s.w %v2,%v2" :::"%v2");  // !!! 1 0  3 2  5 4 ... !!!
        //asm("vshf %v1, %v2,%v2,4");         // 0. 1. 2. 3. ... 511.
		//asm("vst %v2, 8, %s1" );
		//pv_st_c(%v1,vpe);	// ok, in register already.

		pv_seq(%v2,%v1);
		pv_cvt_flt(%v2);

		//asm("vst %v2, 8, %0" ::"r"(vpe):);
		//pv_st_c(%v2,&e[0]);	// ugly addr loads.
		//pv_st_c(%v2,e);		// still ugly
		//pv_st_c(%v2,vpe);	// ok, in register already.
		pv_st_c(%v2,vpe);	// ok, in register already.
#elif 0
		//asm("vshf %v1, %v0, %v2, 6"); // v2 ignored, v0 rotated 32 bits?
		// 4 : abcd,0000 --> badc
		// 5 : abcd,0000 --> aacc
		// 6 : abcd,0123 --> 0a2c 
		asm("vshf %v1, %v0, %v0, 4"); // v2 ignored, v0 rotated 32 bits?
		asm("vst %v1, 8, %s1" );
#elif 0
		//asm("vmv %v3, 1,%v0");
		asm("vst %v3, 8, %s1" );
		// abcdef...wxyz --> yzab...wx
#else
		// PACKED barrel rotate left
		//asm("vmv %v1, 1,%v0");		    // v1: cd ef gh ...
		//asm("vst %v1, 8, %s1" );
        //
		//asm("vshf %v2, %v1, %v0, 6");	// v0: ab cd ef --> v2: bc de fg
		//asm(    "vmv  %v1, 1,%v0\n\t"
        //        "vshf %v2, %v1, %v0, 6\n\t"
        //        :::"%v2","%v1","%v0");
        //pv_rotl_2n(1,%v2,%v0);
        //pv_rotl_2(%v2,%v0);
        //pv_rotl_2nm1(2,%v2,%v0,  %v1,%v3);
        //pv_rotl_1(%v2,%v0, %v1);

        //asm("vmv %v1,127,%v0"); /* 0-127 is range, -1 won't work */
        //asm("eqv %s2,%s2,%s2");
        //  asm("eqv %s2,0,(0)1");      // all-ones (-1)
        //  asm("vmv %v1,%s2,%v0");
        //asm("subs.l %s3,%s2,(63)0");   // -2
        //asm("vmv %v2,%s3,%v0");
        //  asm("vshf %v2, %v0, %v1, 6");
        //asm("xor %s2,0,(0)1");      // all-zeros (0)
        //  asm("lvs %s3,%v2(%s2)");
        //  asm("and %s3,%s3,(32)0");
        //  asm("lsv %v2(%s2),%s3");
        //    asm("lvs %s3,%v2(0)");
        //    asm("and %s3,%s3,(32)1");
        //    asm("lsv %v2(0),%s3");
        //    pv_clr_f0(%v2);
        s_negone(%s7);
        pv_movr_1z(%v2,%v0, %v3,%s7);

        //  asm("vmv %v3,1,%v1");
        //  asm("vshf %v2,%v3,%v1,6");
        //  asm("vmv %v2,1,%v1");
        //asm("vmv %v3,126,%v0");
        //asm("vshf %v2, %v0, %v3, 6");
        //asm("lea %s13,0");

        //asm("vmv %v2,1,%v0");
        //asm("lea %s13,0");
        ////asm("lsv %v2(127),%s13"); // but we need larger, so MUST use Sy...
        //asm("lea %s12,255");
        //asm("lsv %v2(%s12),%s13"); // but we need larger, so MUST use Sy...
        //pv_movl_2_z255(%v2,%v0);

		asm("vst %v2, 8, %s1" );
#endif
	}
    lvl(256);
	if(0){
		for(int i=0; i<256; ++i)
			//((uint64_t*)vpf)[i] = ((uint64_t*)vpc)[(1+i)%256];
			((uint64_t*)vpf)[i] = ((uint64_t*)&c[0])[(1+i)%256];
	}else if(1){
		asm("or %%s0,0,%0" : : "r"(vpb) : "%s0" );
		asm("or %%s1,0,%0" : : "r"(vpe) : "%s1" );
		asm("vld %v0, 8, %s0" );
		asm("vmv %v1, 2/2,%v0");	// v1: cd ef gh ...
		asm("vst %v1, 8, %s1" );
	}else if(1){
		// vpf_rotl_even(v1, v0, n=2)
	}

	for(int i=0; i<10; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
	cout<<"...\n";
	for(int i=254; i<260; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
	cout<<"...\n";
	for(int i=508; i<512; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}


void test_rot(){
	float a[512], b[512], c[512];
	float d[512], e[512], f[512];
	void *vpa=&a[0], *vpb=&b[0], *vpc=&c[0];
	void *vpd=&d[0], *vpe=&e[0], *vpf=&f[0];
	for(unsigned i=0U; i<512U; ++i){
        a[i]=1000+i;
        b[i]=c[i]=d[i]=e[i]=f[i]=0.f;
	}
    asm("or %s0,0,%0" : : "r"(vpa) : "%s0" );
    pv_ld(%v0,%s0);
    pv_rotl_1(%v1,%v0,%v2);
    pv_rotl_2(%v3,%v0);
    asm("or %s1,0,%0" : : "r"(vpb) : "%s1" );
    asm("or %s2,0,%0" : : "r"(vpc) : "%s2" );
    asm("or %s3,0,%0" : : "r"(vpd) : "%s3" );
    pv_st(%v1,%s1);
    pv_st(%v2,%s2);
    pv_st(%v2,%s3);

    s_negone(%s13);
    pv_rotr_1(%v4,%v0, %v6,%s13);
    pv_rotr_2(%v5,%v0, %s13);
    asm("or %s4,0,%0" : : "r"(vpe) : "%s4" );
    asm("or %s5,0,%0" : : "r"(vpf) : "%s5" );
    
    pv_st(%v4,%s4);
    pv_st(%v5,%s5);
    cout<<" test_rot print..."<<endl;
	for(int i=0; i<10; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
	cout<<"...\n";
	for(int i=254; i<260; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
	cout<<"...\n";
	for(int i=508; i<512; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}
void test1(){
    Msk512 m7;
    cout<<" m7 (zero)\n"<<m7<<endl;
    m7.set();
    cout<<" m7.set()\n"<<m7<<endl;
    m7.clr();
    cout<<" m7.clr()\n"<<m7<<endl;
    m7.setevery(7,0);
    cout<<"m7 pattern 1000000 repeating?\n"<<m7<<endl;
    m7.setevery_(7,1); m7.setevery_(7,2);
    cout<<"m7 pattern 1110000 repeating?\n"<<m7<<endl;
    Msk512 m30; m30.setevery_(3,0);
    cout<<"m30 : "<<m30<<endl;
    Msk512 m31; m31.setevery_(3,1);
    cout<<"m31 : "<<m31<<endl;
    Msk512 m32; m32.setevery_(3,2);
    cout<<"m32 : "<<m32<<endl;
	float a[512], b[512], c[512];
	float d[512], e[512], f[512];
	void *vpa=&a[0], *vpb=&b[0], *vpc=&c[0];
	void *vpd=&d[0], *vpe=&e[0], *vpf=&f[0];
	for(unsigned i=0U; i<512U; ++i){
        a[i]=1000+i;
        b[i]=2000+i;
        c[i]=3000+i;
        d[i]=e[i]=f[i]=0.f;
	}

#if 0 // wrong output if this is HERE
    // setup (can be slow-ish)
    //   vm23 : Msk512::setevery(3,0)
    //   vm45 : Msk512::setevery(3,1)
    //   vm67 : Msk512::setevery(3,2)
    //   vm14,15 : tmp
    //asm("or %%s3,0,%0" : : "r"(every3rd_0) : "%s3");
    Msk512 m7;
    cout<<" m7 (zero)\n"<<m7<<endl;
    m7.set();
    cout<<" m7.set()\n"<<m7<<endl;
    m7.clr();
    cout<<" m7.clr()\n"<<m7<<endl;
    m7.setevery(7,0);
    cout<<"m7 pattern 1000000 repeating?\n"<<m7<<endl;
    m7.setevery_(7,1); m7.setevery_(7,2);
    cout<<"m7 pattern 1110000 repeating?\n"<<m7<<endl;
    //Msk512 m30; m30.setevery_(3,0);
    //Msk512 m31; m31.setevery_(3,1);
    //Msk512 m32; m32.setevery_(3,2);
    //cout<<"m30 :\n"<<m30<<endl;
#endif
    asm("or %s0,0,%0" : : "r"((void*)&m30.a.m[0]) : "%s0" );
    asm("ld %s1, 0(,%s0)":::"%s1"); asm("lvm %vm2,0,%s1":::"%vm2");
    asm("ld %s2, 8(,%s0)":::"%s2"); asm("lvm %vm2,1,%s2":::"%vm2");
    asm("ld %s3,16(,%s0)":::"%s3"); asm("lvm %vm2,2,%s3":::"%vm2");
    asm("ld %s4,32(,%s0)":::"%s4"); asm("lvm %vm2,3,%s4":::"%vm2");
    asm("or %s0,0,%0" : : "r"((void*)&m30.b.m[0]) : "%s0" );
    asm("ld %s1, 0(,%s0)":::"%s1"); asm("lvm %vm3,0,%s1":::"%vm3");
    asm("ld %s2, 8(,%s0)":::"%s2"); asm("lvm %vm3,1,%s2":::"%vm3");
    asm("ld %s3,16(,%s0)":::"%s3"); asm("lvm %vm3,2,%s3":::"%vm3");
    asm("ld %s4,32(,%s0)":::"%s4"); asm("lvm %vm3,3,%s4":::"%vm3");

    asm("or %s0,0,%0" : : "r"((void*)&m31.a.m[0]) : "%s0" );
    asm("ld %s1, 0(,%s0)":::"%s1"); asm("lvm %vm4,0,%s1":::"%vm4");
    asm("ld %s2, 8(,%s0)":::"%s2"); asm("lvm %vm4,1,%s2":::"%vm4");
    asm("ld %s3,16(,%s0)":::"%s3"); asm("lvm %vm4,2,%s3":::"%vm4");
    asm("ld %s4,32(,%s0)":::"%s4"); asm("lvm %vm4,3,%s4":::"%vm4");
    asm("or %s0,0,%0" : : "r"((void*)&m31.b.m[0]) : "%s0" );
    asm("ld %s1, 0(,%s0)":::"%s1"); asm("lvm %vm5,0,%s1":::"%vm5");
    asm("ld %s2, 8(,%s0)":::"%s2"); asm("lvm %vm5,1,%s2":::"%vm5");
    asm("ld %s3,16(,%s0)":::"%s3"); asm("lvm %vm5,2,%s3":::"%vm5");
    asm("ld %s4,32(,%s0)":::"%s4"); asm("lvm %vm5,3,%s4":::"%vm5");

    asm("or %s0,0,%0" : : "r"((void*)&m32.a.m[0]) : "%s0" );
    asm("ld %s1, 0(,%s0)":::"%s1"); asm("lvm %vm6,0,%s1":::"%vm6");
    asm("ld %s2, 8(,%s0)":::"%s2"); asm("lvm %vm6,1,%s2":::"%vm6");
    asm("ld %s3,16(,%s0)":::"%s3"); asm("lvm %vm6,2,%s3":::"%vm6");
    asm("ld %s4,32(,%s0)":::"%s4"); asm("lvm %vm6,3,%s4":::"%vm6");
    asm("or %s0,0,%0" : : "r"((void*)&m32.b.m[0]) : "%s0" );
    asm("ld %s1, 0(,%s0)":::"%s1"); asm("lvm %vm7,0,%s1":::"%vm7");
    asm("ld %s2, 8(,%s0)":::"%s2"); asm("lvm %vm7,1,%s2":::"%vm7");
    asm("ld %s3,16(,%s0)":::"%s3"); asm("lvm %vm7,2,%s3":::"%vm7");
    asm("ld %s4,32(,%s0)":::"%s4"); asm("lvm %vm7,3,%s4":::"%vm7");

    asm("or %s0,0,%0" : : "r"(vpa) : "%s0" );
    asm("or %s1,0,%0" : : "r"(vpb) : "%s1" );
    asm("or %s2,0,%0" : : "r"(vpc) : "%s2" );
    pv_ld(%v0,%s0);                     // ABCDE...Z
    pv_ld(%v10,%s1);
    pv_ld(%v20,%s2);
    // oops, I wnat rotr here and rotl (that's why the merge masks
    // in what follows are flaky!)
    pv_rotl_1(%v1,%v10,%v11);           // 90123...8
    pv_rotl_2(%v2,%v20);
    asm("or %s3,0,%0" : : "r"(vpd) : "%s3" );
    asm("or %s4,0,%0" : : "r"(vpe) : "%s4" );
    asm("or %s5,0,%0" : : "r"(vpf) : "%s5" );
    pv_st(%v0,%s3);
    pv_st(%v1,%s4);
    pv_st(%v2,%s5);

    asm("vmrg.w %v4, %v0,%v1,%vm2");    // 9B12E...
    asm("vmrg.w %v5, %v1,%v0,%vm2");
    //pv_st(%v4,%s3);
    //pv_st(%v5,%s4);
    //pv_st(%v2,%s5);

    //pv_st(%v0,%s3);
    //pv_st(%v1,%s4);
    //pv_st(%v2,%s5);

    asm("vmrg.w %v6, %v4,%v2,%vm6");    // 9B12E...
    asm("vmrg.w %v7, %v2,%v4,%vm6");
    //pv_st(%v6,%s3);
    //pv_st(%v7,%s4);
    //pv_st(%v2,%s5);

    //asm("vmrg.w %v8, %v5,%v7,%vm4");    // 9B12E...
    //asm("vmrg.w %v9, %v7,%v5,%vm4");
    //pv_st(%v6,%s3);
    //pv_st(%v8,%s4);
    //pv_st(%v9,%s5);

    // unshift...
    //pv_rotr_1(%v10, %v8, %v11); // TBD
    //pv_rotr_2(%v12, %v9);
    //pv_st(%v6,%s3);
    //pv_st(%v10,%s4);
    //pv_st(%v12,%s5);

    cout<<" test1 print..."<<endl;
	for(int i=0; i<10; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
	cout<<"...\n";
	for(int i=254; i<260; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
	cout<<"...\n";
	for(int i=508; i<512; ++i)
		cout<<i<<":\t"<<a[i]<<" "<<b[i]<<" "<<c[i]<<" "<<d[i]<<" "<<e[i]<<" "<<f[i]<<endl;
    cout<<"\nGoodbye from "<<__FUNCTION__<<endl;
}

int main(int, char**){
    test_msk256_basic();
    //test_msk256_constants();
    test_vm_ldst();
    test_msk256();
    test_msk512();

	//test0();
    //test_rot();
    //test1();
    //test1b();
	return 0;
}
/* vim: set sw=4 ts=4 et: */
