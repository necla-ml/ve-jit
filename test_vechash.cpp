/** \file
 * - rework jitve_hello to demo asmfmt on the math kernel example
 * - merge with ve_regs (symbolic registers, scopes) work should continue
 *   in a separate directory (before things get overly complicated).
 */
#include "vechash.hpp"
#include "stringutil.hpp"
#include "asmfmt.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <array>
#include <algorithm>    // min
#define STR0(...) #__VA_ARGS__
#define STR(...) STR0(__VA_ARGS__)

#ifndef FOR
#define FOR(VAR,VLEN) _Pragma("_NEC shortloop") for(int VAR=0; (VAR) < (VLEN); ++VAR)
#endif

using namespace std;
#define MVL 256
typedef array<volatile uint64_t,MVL> Vu64;
#if 0
// This would be nice, but unfortunately the wrapper call to the assembly code
// needs actual strings within its code.
//
// Perhaps later we should JIT the wrapper call too (then even input registers can
// be automatically handled).
//
void regs( AsmFmtVe &defs ){
    AsmScope block;
    ve_propose_reg("seed",block,defs,SCALAR);
    ve_propose_reg("hash",block,defs,SCALAR);
    ve_propose_reg("vlen",block,defs,SCALAR);
    ve_propose_reg("a",block,defs,VECTOR);
    ve_propose_reg("b",block,defs,VECTOR);
    // locals
    //ve_propose_reg("tmp",block,defs,SCALAR);
    //block.push_back("hab","tmp");
    defs.scope(block, vechash2::test_vechash);
}
#endif
void regs( AsmFmtVe &defs ){
    AsmScope block = {
        {"seed",    "%s0"},
        {"hash",    "%s1"},
        {"vlen",    "%s2"},
        {"h2",      "%s3"},
        {"a",       "%v0"},
        {"b",       "%v1"},
        // const locals (aliased to same consts of VecHash2 kernel)
        //  In this case, we KNOW we'll use the VecHash2 kernel,
        //  so these constants will [later] be defined.
        {"Ca", "rnd64a"},
        {"Cb", "rnd64b"},
        {"Cc", "rnd64c"},
    };
    ve_propose_reg("tmp",block,defs,SCALAR_TMP);
    defs.scope(block);
}
/** \b In seed
 * \b Out a[], b[] pseudorandom
 * \b Const Ca,Cb,Cc u64 scramblers. */
void gen_ab( AsmFmtVe &gen ){
    gen.lcom("gen_ab(IN:seed OUT:a[],b[], CONST:Ca,Cb,Cc) @ current VL");
    // Note: Instead, could use rnd64{a,b,c} const variables
    //       with opportunistic register sharing based on name.
    //       (because Ca,Cb loads are already in rnd64a, rnd64b, rnd64c)
    gen.ins("vseq    a");
    gen.ins("vseq    b");
    // using 3 const registers, Ca,Cb,Cc, aliased/shared with other kernels
    gen.ins("vaddu.l a,Ca,a");          // a[] += Ca
    gen.ins("vmulu.l a,Ca,a");          // a[] *= Ca
    gen.ins("vaddu.l a,seed,a");        // a[] += seed

    gen.ins("vaddu.l b,Cb,b");          // b[] += Cb
    gen.ins("vmulu.l b,Cb,b");          // b[] *= Cb

    gen.ins("vmulu.l  a,Cc,a");         // a[] *= Cc
    gen.ins("vxor     b,a,b");          // b[] ^= a[]
    gen.ins("vmulu.l  b,Cc,b");         // b[] *= Cc
    gen.lcom("");
}
void sim_ab( uint64_t const seed, int const vl, Vu64& a, Vu64& b ){
    uint64_t u, v;
    FOR(i,vl) a[i] = i;
    FOR(i,vl) b[i] = i;

    u = scramble64::r1;
    FOR(i,vl) a[i] += u;        // init
    FOR(i,vl) a[i] *= u;        // mpy
    FOR(i,vl) a[i] += seed;     // add seed

    v = scramble64::r2;
    FOR(i,vl) b[i] += v;         // init
    FOR(i,vl) b[i] *= v;        // mpy

    u = scramble64::r3;
    FOR(i,vl) a[i] *= u;
    FOR(i,vl) b[i] ^= a[i];
    FOR(i,vl) b[i] *= u;
}
struct Sim {
    uint64_t const s;   // seed for a[], b[]
    uint64_t const vl;  // vector length
    uint64_t h;         // hash_in
    Vu64 a;             // OUT: vector a[vl]
    Vu64 b;             // OUT: vector b[vl]
    Vu64 z;             // OUT: vector z[vl]
    uint64_t h2;        // hash_out (no longer done in asm)
};
void quickout(Sim const& sim){    
    cout<<" sim seed="<<sim.s<<" vl="<<sim.vl<<" h="<<sim.h<<endl;
    // output results
    int const vl = sim.vl;
    int len = vl;
    char const* ending = " }";
    if( sim.vl > 8 ){
        len = 8;
        ending = " ...}";
    }
    cout<<" a["<<vl<<"]={"; FOR(i,len) cout<<" "<<setw(18)<<jithex(sim.a[i]); cout<<ending<<endl;
    cout<<" b["<<vl<<"]={"; FOR(i,len) cout<<" "<<setw(18)<<jithex(sim.b[i]); cout<<ending<<endl;
    cout<<" z["<<vl<<"]={"; FOR(i,len) cout<<" "<<setw(18)<<jithex(sim.z[i]); cout<<ending<<endl;
    cout<<" hash 0x"<<hex<<sim.h<<" --> hash' = 0x"<<sim.h2
        <<dec<<endl;
}
void sim_vechash2( Sim& sim ){
    sim_ab(sim.s, sim.vl, sim.a, sim.b);    // generate a[], b[] "rand" vectors
    VecHash2 vh(MVL,sim.s);
    vh.u64( sim.h );
    vh.hash_combine( (uint64_t const*)&sim.a[0], (uint64_t const*)&sim.b[0], sim.vl );
    // DO NOT do vrxor in assembly, it seems buggy !!!
    uint64_t * z = vh.getVhash();
    FOR(i,sim.vl) { sim.z[i] = z[i]; }
    uint64_t r = 0;
    FOR(i,sim.vl) r ^= z[i];
    std::cout<<" xor-reduce r="<<std::hex<<r<<std::dec<<std::endl;
    sim.h2 = sim.h ^ r;
}
/** \c vlen_reg is allowed to be empty */
std::string asm_vechash2( Sim const& sim, std::string vlen_reg="" ){
    VecHash2 vh(MVL);                            // we'll use the default VecHash2 seed
    AsmFmtVe def, gen, hsh;
    regs(def);
    gen.setParent(&def);
    cout<<" given vlen_reg=<"<<vlen_reg<<">"<<endl;
    if( vlen_reg.empty() ){
        // look up the 'vlen' macro mapping (this is vlen_reg)
        // if 'defs' has register mapping for vlen, use it
        vlen_reg = def.mac_lookup("vlen");
        cout<<" found vlen_reg=<"<<vlen_reg<<">"<<endl;
    }
    // if we are pased a string vlen_reg (which might be empty)
    if(vlen_reg.empty()){
        cout<<" using immN or tmp reg..."<<endl;
        gen.set_vector_length(sim.vl);
    }else{
        cout<<" using vlen_reg=<"<<vlen_reg<<">"<<endl;
        gen.set_vector_length(vlen_reg);
    }

    gen_ab( gen );

    hsh.setParent(&gen);
    hsh.ins("xor h2,0,hash", "prior hash value");
    vh.kern_asm_begin( def, gen, 0, sim.s );
    vh.kern_asm( hsh, "a", "b", "vlen", "h2", "tmp");
    vh.kern_asm_end( hsh );

    AsmFmtVe endcode;
    endcode.ins("b.l (,%lr)", "return!");
    endcode.lcom("finished jit asm code " __FILE__);

    cout<<string(80,'=')<<" CODE OUTPUT"<<endl;
    std::string code;
    code = def.flush();         code += "//END def.flush()\n";
    code += gen.flush();        code += "//END gen.flush()\n";
    code += hsh.flush();        code += "//END hsh.flush()\n";
    code += hsh.flush_all();    code += "//END hsh.flush_all()\n";
    code += gen.flush_all();    code += "//END gen.flush_all()\n";
    code += def.flush_all();    code += "//END def.flush_all()\n";
    code += endcode.flush_all();
    return code;
}

#if defined(__ve)
/** returns a bogus value to avoid unused vars (that are in asm clause) */
int wrap_vechash( Sim& sim, ExecutablePage& page ){
        void *page_addr = page.addr();
#define WVH_DEBUG 1
        uint64_t seed = sim.s;
        uint64_t hash = sim.h;
        uint64_t vlen = sim.vl;
        uint64_t volatile h2=0;
#if WVH_DEBUG >= 1
        uint64_t volatile *a = &sim.a[0];        // %v0
        uint64_t volatile *b = &sim.b[0];        // %v1
        cout<<" sim@"<<(void*)&sim<<" a@"<<(void*)a<<" b@"<<(void*)b<<endl;
        uint64_t bogus = a[0]+b[0];
#if WVH_DEBUG >= 2
        uint64_t volatile *z_addr = &sim.z[0];        // %v1
        uint64_t vx0=0, vy0=0, vz0=0;
        uint64_t xor_red=0;
#if WVH_DEBUG >= 3
        Vu64 vx;
        uint64_t *vx_addr = &vx[0];
#endif
#endif
#endif
        // jit inputs: %s0(sim.s) %s1(sim.hash) %s2(sim.vlen)
        // jit outputs
        asm("### AAA");
        asm(
                "xor    %s0,0,%[seed]\n"
                "addu.l %s1,0,%[hash]\n"
                "xor    %s2,0,%[vlen]\n"
                "ld     %s12, %[page]\n"
                "bsic   %lr,(,%s12)\n"
                "or     %[h2],0,%s3     # // save hash_combine return value\n"
#if WVH_DEBUG >= 1 // debug... (print to compare)
                "vst    %v0,8,%[a_addr] # // store a[]\n"
                "vst    %v1,8,%[b_addr] # // store b[]\n"
#if WVH_DEBUG >= 2 // more debug...
                "vst    %v61,8,%[z_addr] # // store z[]\n"
                "lvs    %[vz0], %v61(0)\n"   // vh2_vz
                "lvs    %[vy0], %v62(0)\n"   // vh2_vy
                "lvs    %[vx0], %v63(0)\n"   // vh2_vx
                "or     %[xor_red],0,%s62\n"  // vh2_r
#if WVH_DEBUG >= 3
                "vst    %v63,8,%[vx_addr] # // store vx[]\n"
#endif
#endif
#endif
                // outputs
                : [h2]"=r"(h2)
#if WVH_DEBUG >= 2 // added outputs
                , [vx0]"=r"(vx0)
                , [vy0]"=r"(vy0)
                , [vz0]"=r"(vz0)
                , [xor_red]"=r"(xor_red)
#endif
                // inputs (actually const...)
                : [page]"m"(page_addr)
                , [seed]"r"(seed)   // seed %s0
                , [hash]"r"(hash)   // hash %s1
                , [vlen]"r"(vlen)   // vector len %s2
#if WVH_DEBUG >= 1 // added inputs
                , [a_addr]"r"(a)
                , [b_addr]"r"(b)
#if WVH_DEBUG >= 2
                , [z_addr]"r"(z_addr)
#if WVH_DEBUG >= 3
                , [vx_addr]"r"(vx_addr)
#endif
#endif
#endif
                // clobbers
                :"%s12","%s0","%s1","%s2","%s3","%s4","%s5","%s6","%s7","%v0","%v1",
                "%s40","%s41","%s42","%s43","%s60","%s61","%s62","%s63",
                "%v40","%v41","%v63","%v62","%v61","%v60",
                "memory"
           );
        // Notes:
        //      illegal to clobber %lr, apparently:)
        //      ncc silently ignores the assembler code if you try [page]"m"(page.addr())
        asm("### BBB");
        bogus += a[0] + b[0];
#if 0 && WVH_DEBUG >= 2
        uint64_t r = 0;
        FOR(i,sim.vl) r ^= z[i];
        cout<<" xor-reduce r="<<hex<<r<<dec<<endl;
        sim.h2 = sim.h ^ r;
#else
        sim.h2 = h2;
#endif
#if 1
        cout<<hex<<hash<<", "<<sim.h2<<", bogus="<<bogus;
#if WVH_DEBUG >= 1
        cout<<"\na[0] = "<<a[0]<<" b[0] = "<<b[0];
#if WVH_DEBUG >= 2
        cout<<"\nvx0="<<vx0<<" vy0="<<vy0<<" vz0="<<vz0;
        cout<<"\n, xor_red="<<hex<<xor_red <<", ~xor_red="<<hex<<~xor_red
#if WVH_DEBUG >= 3
        FOR(i,vlen){ cout<<" vx["<<i<<"]= "<<hex<<vx[i]<<endl; }
#endif
#endif
#endif
#endif
        cout<<dec<<endl;
        return bogus;
}
#endif

void run_vechash2( Sim& sim, std::string code ){
    string asmFile("tmp_test_vechash_vl"+jitdec(sim.vl)+".S");
    {
        ofstream ofs(asmFile);
        ofs<<"/* Autogenerated by  test_vechash.cpp */\n";
        ofs<<code;
        ofs.close();
    }
#if !defined(__ve) // just run the x86 version once more...
    if(1){ cout<<"VE assembler --> file "<<asmFile<<endl; }
    sim_vechash2(sim);
#else
    if(1){ cout<<"asm2bin(\""<<asmFile<<"\",verbose)..."; }
    auto bytes_bin = asm2bin( asmFile, 0/*verbose*/ );                  // .S --> .bin
    if(1){ cout<<" binary blob of "<<bytes_bin<<" bytes"<<endl; }
    cout<<"ExecutablePage page( fname_bin(\""<<asmFile<<"\")"<<endl;
    // NO: segfaults on ve: ExecutablePage page( fname_bin(asmFile) );  // .bin --> codepage
    string binFile = fname_bin(asmFile);
    ExecutablePage page( binFile );   // .bin --> codepage
    {
        void *page_addr = page.addr();
        cout<<" going to bsic into page @ "<<page_addr<<endl;
        cout<<" "<<binFile<<" call using extended asm to set/get register args\n"<<endl;
        cout<<" sim: seed="<<sim.s<<" hash="<<sim.h<<" vl="<<sim.vl<<endl;
    }
    wrap_vechash( sim, page );
    cout<<"... BACK FROM JIT!"<<endl; cout.flush();
    cout<<" return hash "<<hex<<sim.h2<<" = "<<dec<<sim.h2<<endl; cout.flush();
#endif
}




#if 0
void test_vechash(){
    // - given a seed (%s0), a hash (%s1), and vlen (%s3) in [1,256]:
    //   - set vector length to vlen
    //   - create 2 vectors a[], b[] (as %v0, %v1)
    //   - hash a, b (reproducing VecHash method)
    //   - and xor the result back into hash (%s1)
    string asmFile("tmp_jitve_test_hello.S");  // file extension required
    {
        AsmFmtVe kernel_vechash(asmFile); // assembler line pretty printer
        kernel_vechash
            .lcom(STR(VecHash(IN:seed,vlen, INOUT: hash, OUT: a[vlen],b[vlen])))
            .lcom(STR( seed, vlen, hash   : %s0, %s1, %s2 ))
            .lcom(STR( a, b               : %v0, %v1))
            ;
        AsmScope block;
        block.push_back({"seed","%s0"});
        block.push_back({"seed","%s0"});
        kernel_vechash                  // assume vlen is in OK range
            .ins("svl");
            
            .undef("REL")
            .undef("BP")
            ;
        auto kh = kernel_vechash.flush();
        //auto kh = kernel_vechash.str(); // side-effect: clears the ostringstream
        //auto kh = kernel_vechash.flush(); // write AND get a copy of the string
        cout<<"kernel_vechash.str():   also in "<<asmFile<<"\n"<<kh<<endl;
    }
    if(0){ cout<<"asm2bin(\""<<asmFile<<"\",verbose)..."; }
    auto bytes_bin = asm2bin( asmFile, 0/*verbose*/ );                  // .S --> .bin
    if(1){ cout<<" binary blob of "<<bytes_bin<<" bytes"<<endl; }
    cout<<"ExecutablePage page( fname_bin(\""<<asmFile<<"\")"<<endl;
    // NO: segfaults on ve: ExecutablePage page( fname_bin(asmFile) );  // .bin --> codepage
    string binFile = fname_bin(asmFile);
    ExecutablePage page( binFile );   // .bin --> codepage
    {
        printf(" going to bsic into page @ %p\n",page.addr()); fflush(stdout);
        printf(" tmp_kernel_hello takes no [register!] args\n"); fflush(stdout);
        unsigned long ret;
        void *page_addr = page.addr();
        asm("### AAA");
        asm("ld %s12, %[page]\n"
                "bsic %lr,(,%s12)\n"
                "or %[ret],0,%s0 # // move asm %s0 return value into C register of 'ret'\n"
                :[ret]"=r"(ret)
                :[page]"m"(page_addr)
                :"%s12","%s0","%s1","%s2","%s3","%s4");
        // Notes:
        //      illegal to clobber %lr, apparently:)
        //      ncc silently ignores the assembler code if you try [page]"m"(page.addr())
        asm("### BBB");
        printf("... and we actually returned, return value %08lx = %ld\n",ret,ret); fflush(stdout);
    }
}
#endif

int main(int argc,char**argv)
{
    int vl = 8;
    uint64_t seed=0;
    {
        if(argc > 1){
            int a=0;
            for( ; a+1<argc && argv[a+1][0]=='-'; ++a){
                char *c = &argv[1][1];
                for( ; *c != '\0'; ++c){
                    if(*c=='h'){
                        cout<<argv[0]<<" [-h] [VLEN[ SEED]]"<<endl;
                        cout<<" Arguments:"<<endl;
                        cout<<"  -h    this help"<<endl;
                        cout<<"   VLEN = vector length [8]"<<endl;
                        cout<<"   SEED [0]"<<endl;
                    }
                }
            }
            if(argc > a+1) vl  = atof(argv[a+1]);
            if(argc > a+2) seed= atof(argv[a+2]);
            cout<<"vlen="<<vl<<", seed="<<seed<<endl;
        }
    }
    uint64_t vlu64 = static_cast<uint64_t>(max(1, min(vl, MVL)));
    cout<<" test_vechash VLEN("<<vl<<") SEED("<<seed<<")"<<endl;
    Sim sim = {seed, vlu64, 0UL, {0}, {0}, 0UL};
    sim_vechash2(sim);
    quickout(sim);
    Sim sim2(sim);
    std::string code = asm_vechash2(sim2); // jit string, sim2 const
    //cout<<code<<string(80,'=')<<endl;
    run_vechash2(sim2, code); // if not VE, will just run sim_vechash2 a second time
    quickout(sim2);
    assert( sim2.s == sim.s );
    assert( sim2.vl == sim.vl );
    assert( sim2.h == sim.h );
    FOR(i,vl) assert( sim2.a[i] == sim.a[i] );
    FOR(i,vl) assert( sim2.b[i] == sim.b[i] );
    cout<<hex<<" sim2.h2 = "<<sim2.h2<<" cf. sim.h2 = "<<sim.h2<<dec<<endl;
    assert( sim2.h2 == sim.h2 );
    cout<<"\nGoodbye"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
