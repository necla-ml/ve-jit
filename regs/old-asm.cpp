
#include "old/asm-old.hpp"
#include "throw.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <sstream>
using namespace std;

namespace ve {

std::runtime_error sym_unknown(unsigned const symId){
    ostringstream oss;
    oss<<" ERROR: symbol "<<symId<<" has not been declared"<<endl;
    return std::runtime_error(oss.str());
}
std::runtime_error sym_out_of_scope(Sym const* sym){
    ostringstream oss;
    if( sym == nullptr ){
        oss<<" ERROR: symbol pointer is NULL"<<endl;
    }else{
        oss<<" ERROR: symbol "<<sym->name<<" is out of scope ["<<sym->scope<<"]"<<endl;
    }
    return std::runtime_error(oss.str());
}
std::runtime_error sym_out_of_scope(SafeSym const& ssym){
    ostringstream oss;
    oss<<" SafeSym"<<ssym.id()<<" is not in active scope"<<endl;
    return std::runtime_error(oss.str());
}
std::runtime_error sym_unset(SafeSym const* ssym){
    ostringstream oss;
    oss<<" SafeSym@"<<(void*)ssym<<" = "<<*ssym<<" unset, cannot dereference."<<endl;
    return std::runtime_error(oss.str());
}
std::runtime_error invalid_RegId(char const* file, int line, RegId id){
    ostringstream oss;
    oss<<" ERROR: "<<file<<':'<<line<<" RegId "<<id<<" out-of-range"<<endl;
    return std::runtime_error(oss.str());
}
static std::runtime_error todo(char const* file, int line, char const* msg){
    ostringstream oss;
    oss<<" TODO: "<<file<<':'<<line<<" "<<msg<<endl;
    return std::runtime_error(oss.str());
}
std::ostream& operator<<(std::ostream& os, Reg_t const f){
    int f1 = f&MSK_FREE;
    char const * f1names[] = {"FREE","RESERVED","USED","Huh"};
    int f2 = (f&MSK_TYPE) >> 2;
    char const * f2names[] = {"SCALAR","VECTOR","VMASK","VMASK2"};
    int f3 = (f&MSK_ATTR) >> 4;
    char const* f3attr = (f3==0? ""
            : f3==PRESERVE? "|PRESERVE"
            : "|Huh?");
    return os<<' '<<f1names[f1]<<'|'<<f2names[f2]<<f3attr;
}

std::ostream& operator<<(std::ostream& os, ve::Sym const& sym){
    // changed to shorten output string bit
    std::ostringstream oss;
    oss<<" Sym{"<<(sym.name? sym.name: "NULL")
        <<(sym.active? "+":"-")
        <<sym.uid;
    if(sym.id <= IDscalar_last)
        oss<<"%s"<<sym.id-IDscalar;
    else if(sym.id <= IDvector_last)
        oss<<"%v"<<sym.id-IDvector;
    else if(sym.id <= IDvmask_last)
        oss<<"%vm"<<sym.id-IDvmask;
    else
        oss<<"%??";
    if(isReg(sym.id))   oss<<'('<<sym.id<<')';
    else                ; //oss<<"()";

    // Sym::None  : reg not spilled to memory
    // Sym::SPILL : mem -ve offset wrt to fp
    // Sym::ARG   : mem +ve offset wrt to sp
    if(sym.mem!=Sym::NONE){
        oss<<"M" <<(sym.mem==Sym::NONE? "-": sym.mem==Sym::ARG? "sp": "fp");
        if(sym.mem != Sym::NONE && sym.staleness)
            oss<<":stale"<<sym.staleness;
    }
    if(sym.t_decl < Regs::NEVER) oss<<",decl"<<sym.t_decl;
    if(sym.scope) oss<<",scope"<<sym.scope;
    if(sym.len > 0) oss<<",sz"<<sym.len;
    if(sym.align > 0) oss<<",al"<<sym.align;
    oss<<'}';

    return os<<oss.str();
}

std::ostream& operator<<(std::ostream& os, SafeSym const& sym){
    //cout<<" within operator<< for SafeSym"<<endl;
    std::ostringstream oss;
    oss<<" SafeSym";
    oss<<'{';
    if(sym.sym_id){
        oss<<sym.sym_id;
        unsigned scope;
        Sym const* psym = sym.lookup(&scope);
        if(psym==nullptr) oss<<"X";
        else              oss<<"sc"<<scope<<*psym;
    }
    if(sym.regs==nullptr) oss<<",regs==NULL?";
    oss<<'}';
    return os<<oss.str();
}


Sym const* SafeSym::operator->() const {
    int const verbose=0;
    // lookup within all known symbols
    unsigned scope;
    Sym const* sym = lookup(&scope);
    if(sym==nullptr)
        throw sym_unknown(sym_id);
    if(verbose>0){
        cout<<" sym"<<sym_id<<"@"<<sym
            <<",scope"<<sym->scope<<endl;
    }
    assert( sym->scope == scope );

    // lookup within currently active symbols
    if( regs->active_scopes.find(sym->scope) == regs->active_scopes.end() )
        throw sym_out_of_scope(sym);

    if(verbose>1){
        cout<<" SafeSym::operator-> did not throw! ";
        cout.flush();
    }
    return sym;
}
Sym* SafeSym::operator->(){
    int const verbose=0;
    // lookup within all known symbols
    unsigned scope;
    Sym* sym = lookup(&scope);
    if(sym==nullptr)
        throw sym_unknown(sym_id);
    if(verbose>0){
        cout<<" sym"<<sym_id<<"@"<<sym
            <<",scope"<<sym->scope<<endl;
    }
    assert( sym->uid == sym_id );
    assert( sym->scope == scope );

    // lookup within currently active symbols
    if( regs->active_scopes.find(sym->scope) == regs->active_scopes.end() )
        throw sym_out_of_scope(sym);

    if(verbose>1){
        cout<<" SafeSym::operator-> did not throw! ";
        cout.flush();
    }
    return sym;
}

int SafeSym::reg_flags() const {
    int ret = 0;
    if(regs){
        Sym const* sym = lookup(nullptr);
        if(sym==nullptr)
            throw sym_unknown(sym_id);
        ret = sym->use;
    }
    return ret;
}

uint64_t SafeSym::time() const {
    uint64_t ret = 0UL;
    if(regs){
        Sym const* sym = lookup(nullptr);
        if(sym!=nullptr && isReg(sym->id) && regs!=nullptr)
            ret = regs->time(sym->id);
    }
    return ret;
}

static char const* asm_register_names[IDlast] = {
    "%s0",  "%s1", "%s2", "%s3", "%s4", "%s5", "%s6", "%s7", "%s8", "%s9",
    "%s10",  "%s11", "%s12", "%s13", "%s14", "%s15", "%s16", "%s17", "%s18", "%s19",
    "%s20",  "%s21", "%s22", "%s23", "%s24", "%s25", "%s26", "%s27", "%s28", "%s29",
    "%s30",  "%s31", "%s32", "%s33", "%s34", "%s35", "%s36", "%s37", "%s38", "%s39",
    "%s40",  "%s41", "%s42", "%s43", "%s44", "%s45", "%s46", "%s47", "%s48", "%s49",
    "%s50",  "%s51", "%s52", "%s53", "%s54", "%s55", "%s56", "%s57", "%s58", "%s59",
    "%s60",  "%s61", "%s62", "%s63",
    "%v0",  "%v1", "%v2", "%v3", "%v4", "%v5", "%v6", "%v7", "%v8", "%v9",
    "%v10",  "%v11", "%v12", "%v13", "%v14", "%v15", "%v16", "%v17", "%v18", "%v19",
    "%v20",  "%v21", "%v22", "%v23", "%v24", "%v25", "%v26", "%v27", "%v28", "%v29",
    "%v30",  "%v31", "%v32", "%v33", "%v34", "%v35", "%v36", "%v37", "%v38", "%v39",
    "%v40",  "%v41", "%v42", "%v43", "%v44", "%v45", "%v46", "%v47", "%v48", "%v49",
    "%v50",  "%v51", "%v52", "%v53", "%v54", "%v55", "%v56", "%v57", "%v58", "%v59",
    "%v60",  "%v61", "%v62", "%v63",
    "%vm0",  "%vm1", "%vm2", "%vm3", "%vm4", "%vm5", "%vm6", "%vm7", "%vm8", "%vm9",
    "%vm10",  "%vm11", "%vm12", "%vm13", "%vm14", "%vm15"
};
static const int n_asm_register_names = IDlast;

char const* asmname(RegId const r){
    assert( r < n_asm_register_names);
    char const* ret = "%XX";
    if( isReg(r) )
        ret = asm_register_names[r];
    return ret;
}

Sym::Sym(char const* name)
    // NEVER is wrong!
    : name(name), active(false), id(IDlast), use(FREE)
    , uid(0), t_decl(Regs::NEVER), mem(NONE)
      , scope(0), staleness(0), align(0), len(0)
{}

Sym::Sym(RegId const id, char const* const name, bool const active, int/*Reg_t*/ use_flags, unsigned const nz_uid)
    // NEVER is wrong!
    : name(name), active(active), id(id), use(use_flags)
    , uid(nz_uid), t_decl(Regs::NEVER), mem(NONE), scope(0)
      , staleness(0), align(8), len(0)
{
    THROW_UNLESS( uid != 0U, "+Sym: nz_uid must be > 0" );
    if(id <= IDscalar_last){
        len=8; 
    }
    else if(id <= IDvector_last){
        len=MVL*8;
    }else if(id <= IDvmask_last){
        len=4*8; // 256 bits = 4 * 64-bit scalar regs = 4*8 bytes
    }else{
        //assert( false && "Sym constructor RegId out of range" );
        // Regs::declare allows symbol declaration WITHOUT assigned RegId
        // BUT we now have a hole:
        //    we want to know the "type", to set length, etc. WITHIN SYM
        //    and NOT via Regs::Usage::type !!!!!!!!!!!!
        len=0; // <-- WRONG until Usage::use moves into Sym class.
    }
}

/** private, to streamline the Regs::Regs constructor */
void Regs::declare(RegId id, char const* const name, bool const active, Reg_t use_flags){
    int const verbose=0;
    assert( !scopes.empty() );
    assert( isReg(id) );        // [opt., in principle]
    assert( (use_flags&MSK_TYPE) == SCALAR
            || (use_flags&MSK_TYPE) == VECTOR
            || (use_flags&MSK_TYPE) == VMASK );
    // VMASK2 is not used for internal initialization
    id = std::min( id, static_cast<RegId>(IDlast) );
    scope().syms.emplace_front( id, name, active, use_flags, ++sym_uid );

    // even if inactive, during construction set some default values
    Sym *sym = &scope().syms.front();
    // NB: sym->id is RegId, difft from sym->uid [symbol id]
    assert( sym->uid == sym_uid );
    sym->scope = scope().uid;
    active_symbols[sym->uid] = SymPtrInfo{ sym, scope().uid };
    if(verbose){ cout<<" active_symbols["<<sym->uid<<"]! "; cout.flush(); }
    // set Usage :---> sym + timestamps Sym::t_decl, Usage::t_used
    ++tick;
    sym->t_decl = tick;
    if(verbose){ cout<<" id"<<id; cout<<" decl"<<tick<<endl; }

    if( isReg(id) ){ // asserted always-true, above XXX
        // TODO struct RegWhere { RegWhere(regid); ---> Usage* regArray; int regOff, IDlo, Idend };
        Usage *u = nullptr;
        if( id <= IDscalar_last ){
            if(verbose){ cout<<" %s"; cout.flush(); }
            assert( (use_flags&MSK_TYPE) == SCALAR );
            s[id-IDscalar] = Usage{sym,NEVER};
            u = &s[id-IDscalar];
        }else if( id <= IDvector_last ){
            if(verbose){ cout<<" %v"; cout.flush(); }
            assert( (use_flags&MSK_TYPE) == VECTOR );
            v[id-IDvector] = Usage{sym,NEVER};
            u = &v[id-IDvector];
        }else{
            if(verbose){ cout<<" %m"; cout.flush(); }
            assert( (use_flags&MSK_TYPE) == VMASK );
            m[id-IDvmask] = Usage{sym,NEVER};
            u = &m[id-IDvmask];
        }
        assert( u != nullptr );
        if( sym->active && (sym->use&MSK_FREE) == RESERVED ){
            if(verbose){ cout<<" (active+PRESERVED=>set t_use)"; cout.flush(); }
            u->t_used = tick;
        }
        if(verbose){ cout<<endl; cout.flush(); }
    }
}
/** Returns an inactive symbol with no register assigned. */
SafeSym Regs::declare( Reg_t type, char const* name){
    ++tick; // [opt.]
    cout<<" Regs::declare.."; cout.flush();
    scope().syms.emplace_front( IDlast, name, /*active=*/false, type, ++sym_uid );
    Sym* sym = &scope().syms.front();
    sym->scope = scope().uid;
    assert( active_symbols.find(sym->uid) == active_symbols.cend() );
    active_symbols[sym->uid] = SymPtrInfo{ sym, scope().uid };
    sym->t_decl = tick;
    assert( !isReg(sym->id) );
    //usage(sym->id).t_used = NEVER; // don't have a sym->id yet
    return SafeSym{sym,this};
}

SafeSym Regs::alloc(Reg_t type_flags, char const* name) {
    int const verbose=0;
    if( (type_flags & MSK_FREE) == FREE/* 0 */ ){
        if(verbose){ cout<<" alloc+FREE-->just'declare' "; cout.flush(); }
        return declare(type_flags,name);
    }
    ++tick;
    if(verbose>0){
        cout<<" RegId-alloc("<<type_flags<<","<<name<<")... "; cout.flush();
    }
    THROW_UNLESS( (type_flags & MSK_FREE) == USED, "clients should not use RESERVED" );
    RegId id = getfree(type_flags,/*spill=*/true);
    if(verbose>0){
        cout<<" id="<<id; //cout<<' '<<asmname(id);
        cout.flush();
    }
    if( !isReg(id) ){
        throw todo(__PRETTY_FUNCTION__,__LINE__,"find an old register and spill it.");
    }
    scope().syms.emplace_front( id, name, false/*active*/, type_flags, ++sym_uid );  // c++11 returns void :(
    Sym* sym = &scope().syms.front();
    sym->scope = scope().uid;
    assert( active_symbols.find(sym->uid) == active_symbols.cend() );
    active_symbols[sym->uid] = SymPtrInfo{ sym, scope().uid };
    Usage& u = usage(id);
    assert( u.sym != nullptr );
    assert( u.sym != sym ); // old value must be difft
    u.sym = sym;
    // Regs must set Usage::sym::use flags
    //cout<<" sym->use = "<<sym->use<<endl; cout.flush();
    sym->use = type_flags;
    // set up Sym declaration timestamp and Usage timestamp
    sym->t_decl = tick;
    if(verbose>1) cout<<*sym<<" ";
    // Should sym->active be true?
    // Regs set timestamp to "oldest" (symbol defined, but not yet active (no "value")
    u.t_used = 0;
    sym->t_sym = 0;
    return SafeSym{sym,this};
}

void Regs::activate( Sym* sym ){
    int const verbose=1;
    assert( sym );
    assert( sym->name );
    // assert sym scope is valid
    if( sym->active ) THROW("activate active "<<sym->name<<'?');
    Reg_t typ = REG_T(sym->use & MSK_FREE);
    if( typ==RESERVED ) THROW( "activate RESERVED "<<sym->name<<'?');
    assert( typ == USED );
    ++tick;
    if( isReg(sym->id) ){
        if(verbose>0){ cout<<" indeterminate->active"; cout.flush(); }
        // already have a sym->id
        Usage& u = usage(sym->id);
        assert( u.sym == sym );
        u.t_used = tick;  // equiv time(id [==sym->id] )
        sym->t_sym = tick;
    }else{ // no register assigned yet
        if(verbose>0){ cout<<" decl->active"; cout.flush(); }
        assert( typ == USED );
        RegId id = getfree(sym->use);       // spill if reqd
        sym->id = id;

        if(verbose>0){
            cout<<" id="<<id; //cout<<' '<<asmname(id);
            cout.flush();
        }
        if( !isReg(id) ) THROW("getfree -> no register?");
        Usage& u = usage(id);
        assert( u.sym != nullptr );
        assert( u.sym != sym ); // old value must be difft
        u.sym = sym;
        u.t_used = tick;  // equiv time(id [==sym->id] )
        sym->t_sym = tick;
    }
    sym->active = true;
    if(verbose>1) cout<<*sym<<" ";
}

void Regs::out( SafeSym& ssym ){
    //
    // TODO: set Sym::t_sym AS WELL AS Regs::Usage::t_use
    //
    int const verbose=1;
    Sym *sym = ssym.lookup(nullptr);
    if(sym==nullptr)
        throw sym_out_of_scope(ssym);
    cout<<" out("<<sym->name<<')';
    if(!sym->isActive()){ // operator-> implies sym-in-scope
        activate( sym );  // declared --> active
        cout<<" act:"<<*sym;
    }else if(sym->isReg()){
        time(sym->id) = ++tick;
        cout<<" "<<asmname(sym->id)<<".t="<<tick;
    }else if(sym->isMem()){
        if(verbose){cout<<" isMem()->unspill(TBD) ";}
    }else{
        THROW("active + !reg + !mem? Huh?");
    }
    assert(sym->active);
    assert(isReg(sym->id));
}


Regs::Regs()
    : spill(), s(), v(), m(), tick(0), veclen(0)
      , active_scopes(), scope_uid(0), sym_uid(0), scopes()
{
    cout<<" +Regs@"<<(void*)this<<endl; cout.flush();

    // scope=1 is function-global scope with no symbols in it [yet]
    scope_init();

    // start with all registers free and not preserved across function calls
    // default init will be FREE | SCALAR/VECTOR/VMASK
    for(RegId i=0U; i<8U; ++i){
        //int a = UNDERCAST(Reg_t,FREE);
        //int b = UNDERCAST(Reg_t,SCALAR);
        //declare( IDscalar+i, "__init", /*active*/false, static_cast<Reg_t>(a|b) );
        //
        //declare( IDscalar+i, "__init", /*active*/false, static_cast<Reg_t>
        //        ( UNDERCAST(Reg_t,FREE) | UNDERCAST(Reg_t,SCALAR) ) );
        //
        // OK
        //declare( IDscalar+i, "__init", /*active*/false, ECAST(Reg_t, ( UNDERCAST(Reg_t,FREE) | UNDERCAST(Reg_t,SCALAR)) ) );
        //
        //NO: declare( IDscalar+i, "__init", /*active*/false, ve::operator|(FREE,SCALAR) );
        declare( IDscalar+i, "__init", /*active*/false, REG_T(FREE|SCALAR) );
        //
        // also sets s[i]
    }
    // User can never modify RESERVED registers
    // We never free memory of a RESERVED item
    // PRESERVE means register does not change across function calls.
    //   (prologue/epilogue may need to save/restore PRESERVE regs)
    declare( 8 , "StackLimit,%sl"  , true , REG_T(SCALAR|RESERVED|PRESERVE) );
    declare( 9 , "FramePointer,%fp", true , REG_T(SCALAR|RESERVED|PRESERVE) );
    declare( 10, "LinkReg,%lr",      false, REG_T(SCALAR|RESERVED         ) );
    declare( 11, "StackPtr,%sp",     true , REG_T(SCALAR|RESERVED|PRESERVE) );
    declare( 12, "OuterReg,%s12",    false, REG_T(SCALAR|RESERVED         ) );
    declare( 13, "LinkFnId,%s13",    false, REG_T(SCALAR|RESERVED         ) );
    declare( 14, "ThreadPtr,%tp",    true , REG_T(SCALAR|RESERVED|PRESERVE) );
    declare( 15, "GOT,%got",         true , REG_T(SCALAR|RESERVED|PRESERVE) );
    declare( 16, "PLT,%plt",         true , REG_T(SCALAR|RESERVED|PRESERVE) );
    declare( 17, "LinkArea,%s17",    true , REG_T(SCALAR|RESERVED|PRESERVE) );
    // TODO PRESERVE and RESERVED should be getter functions,
    //      since they are uniquely a fn of RegId
    for(RegId i=18U; i<=33U; ++i){
        // PRESERVE: if used, must save/restore in fn prologue/epilogue
        declare( IDscalar+i, "__init", /*active*/false, REG_T(FREE|SCALAR|PRESERVE) );
    }
    for(RegId i=34U; i<NREG::s; ++i){
        declare( IDscalar+i, "__init", /*active*/false, REG_T(FREE|SCALAR) );
    }
    for(RegId i=0U; i<NREG::v; ++i){
        //cout<<"MSK_TYPE = "<<MSK_TYPE<<endl;; cout.flush();
        assert( ((FREE+VECTOR)&MSK_TYPE) == VECTOR );
        declare( IDvector+i, "__init", /*active*/false, REG_T(FREE|VECTOR) );
    }
    // CHECKME: I suspect %vm0 is special, but have no docs about it
    declare( IDvmask+0, "vmask0",      /*active*/false, REG_T(RESERVED|VMASK|PRESERVE) );
    for(RegId i=1U; i<NREG::vm; ++i){
        declare( IDvmask+i, "__init", /*active*/false, REG_T(FREE|VMASK) );
    }
    // For a function with args, some args will be in regs s[0..7]
    // Check that **ALL**  Regs are initialized ...
    for(RegId i=0; i<NREG::s; ++i) THROW_UNLESS( s[i-IDscalar].sym != nullptr, "%s"<<i<<" uninitialized?" );
    for(RegId i=0; i<NREG::v; ++i) THROW_UNLESS( v[i-IDvector].sym != nullptr, "%v"<<i<<" uninitialized?" );
    for(RegId i=0; i<NREG::vm ; ++i) THROW_UNLESS( m[i-IDvmask ].sym != nullptr, "%vm"<<i<<" uninitialized?" );
}

void Regs::free_register(RegId const id){
    Usage& u = usage(id);
    assert( !u.isReserved() );
    assert( !u.isFree() );
    u.setFree();
}

void Regs::scope_init(){
    assert( scopes.empty() );
    scopes.emplace_front(++scope_uid);
    active_scopes.insert(scope_uid);
}

void Regs::scope_begin(){
    assert( ! scopes.empty() );
    scopes.emplace_front(++scope_uid);
    active_scopes.insert(scope_uid);
}

void Regs::scope_end(){
    // begin by marking the scope as inactive.  SafeSym dereference should throw TODO
    active_scopes.erase(scopes.front().uid);

    // Scope has Symbols that are refered to elsewhere.
    // 1. Sym* Regs::Usage::sym (free the register for re-use)
    // 2. Spill::Region::sym    (free up memory reservation in stack frame)
    for(auto const& sym : scopes.front().syms ){
        assert( sym.uid > 0U );
        assert( active_symbols.find(sym.uid) != active_symbols.end() );
        // if Sym was spilled, free Spill area entries
        if( sym.mem != Sym::NONE ){
            assert( sym.mem == Sym::SPILL ); // only one choice, for now
            spill.erase(&sym);
        }
        // set register status to FREE and Sym* to nullptr
        // ...just for form...
        free_register( sym.id );
        // remove all scope symbol uids, making their SafeSym
        // values no longer dereferencable
        active_symbols.erase( sym.uid );
    }
    // external Sym* pointers all gone, so delete current symbol Scope
    scopes.pop_front();
    if (scopes.empty()){
        ostringstream oss;
        oss<<" ERROR: scope_end() popped the global function scope!"<<endl;
        cerr<<oss.str();
        cerr.flush();
        throw runtime_error(oss.str());
    }
}

/** \c nscope can be set to number of scopes we moved up in order to
 * find \c name.  If unfound, \c *nscope will be set to -1. */
Sym* Regs::lookup(char const* name, int *nscope/*=nullptr*/) const {
    int n=0;
    Sym* ret=nullptr;
    for( auto const& sc: scopes){
        auto scEnd = sc.syms.end();
        auto found = find_if( sc.syms.begin(), scEnd,
                [&name](const Sym& sym)
                { return strcmp(name,sym.name)==0; });
        if( found != scEnd ){
            ret = const_cast<Sym*>(&*found);
            break;
        }
        ++n;
    }
    if( nscope != nullptr ) *nscope = (ret? n: -1);
    return ret;
}
Sym* Regs::lookup(unsigned uid, unsigned *scope) const{
    Sym* ret = nullptr;
    unsigned sco = 0U;
    if(uid){
        auto const found = active_symbols.find(uid);
        if( found != active_symbols.cend() ){
            ret = found->second.sym;
            sco = found->second.scope_uid;
        }
    }
    if(scope) *scope = sco;
    return ret;
}

RegId Regs::getfree(int/*Reg_t & MSK_TYPE*/ type, bool doSpill/*=true*/){
    int const verbose = 0;
    type &= MSK_TYPE;
    uint64_t oldDecl = NEVER;
    uint64_t oldTime = NEVER; // big number
    RegId oldReg = IDlast;
    if(verbose){cout<<" getfree("<<(void*)(intptr_t)type<<','<<doSpill<<")"<<endl; cout.flush();}
    if(type==SCALAR){
        for(RegId i=0; i<NREG::s; ++i){
            RegId j = IDscalar + (i+34)%NREG::s;
            if(verbose>0){ cout<<" ?"<<j; cout.flush(); }
            if(s[j].isReserved()) continue;
            if(s[j].isFree()){
                return j;
            }
            assert( s[j].sym != nullptr );
            if( s[j].t_used < oldTime ){ // equiv time(j)
                if(verbose){cout<<"O"<<oldTime;}
                oldReg = j;
                oldTime = s[j].t_used;
                oldDecl = s[j].sym->t_decl;
            }else if( s[j].t_used == oldTime ){
                if( s[j].sym->t_decl < oldDecl ){
                    oldReg = j;
                    oldDecl = s[j].sym->t_decl;
                    if(verbose){cout<<"o"<<oldDecl;}
                }
            }
        }
    }else if(type==VECTOR){
        for(RegId i=IDvector; i<IDvector_last+1; ++i){
            if(v[i].isReserved()) continue;
            if(v[i].isFree()){
                return i;
            }
            assert( v[i].sym != nullptr );
            if( v[i].t_used < oldTime ){
                oldReg = i;
                oldTime = v[i].t_used;
                oldDecl = v[i].sym->t_decl;
            }else if( v[i].t_used == oldTime ){
                if( v[i].sym->t_decl < oldDecl ){
                    oldReg = i;
                    oldDecl = v[i].sym->t_decl;
                }
            }
        }
    }else if(type==VMASK){
        for(RegId i=0; i<NREG::vm; ++i){
            if(m[i].isFree()){
                return IDvmask+i;
            }
            assert( m[i].sym != nullptr );
            if( m[i].t_used < oldTime ){
                oldReg = i;
                oldTime = m[i].t_used;
                oldDecl = m[i].sym->t_decl;
            }else if( m[i].t_used == oldTime ){
                if( m[i].sym->t_decl < oldDecl ){
                    oldReg = i;
                    oldDecl = m[i].sym->t_decl;
                }
            }
        }
    }else if(type==VMASK2){
        for(RegId i=0; i<NREG::vm; ++i){
            if((i&1)) continue; // cannot start on odd-numbered %vm_i
            if(m[i].isReserved() || m[i+1].isReserved()) continue;
            if(m[i].isFree() && m[i+1].isFree()){
                return i;
            }
            assert( m[i].sym != nullptr );
            assert( m[i+1].sym != nullptr );
            if( m[i].t_used < oldTime && m[i+1].t_used < oldTime ){
                oldReg = i;
                oldTime = max(m[i].t_used, m[i+1].t_used);
            }else if (m[i].t_used == oldTime && m[i+1].t_used == oldTime){
                oldReg = i;
                oldTime = max(m[i].sym->t_decl, m[i+1].sym->t_decl);
            }
        }
    }
    if(doSpill){
        if(verbose>0) cout<<" oldest="<<oldReg;
        Usage& uOld = usage(oldReg);
        if(verbose>0) cout<<" t_used="<<uOld.t_used;
        assert( uOld.sym );
        if(verbose>0) cout<<" t_decl="<<uOld.sym->t_decl;
        if(verbose>0) if(uOld.sym){
            cout<<" "<<*uOld.sym<<" spill..."<<endl; cout.flush();
        }
        spill.spill(uOld.sym /*, default align for now */);
        return oldReg;
    }
    TODO("Why ever did I get here?");
    return IDlast;                      // no free registers (error?)
}

RegId Regs::gettmp(int/*Reg_t & MSK_TYPE*/ type, bool spill/*=true*/){
    // We'll change the search order for symbols hinted to be "temporary"
    type &= MSK_TYPE;
    if(type==SCALAR){
        for(RegId i=NREG::s; i>0; ++i){
            --i;
            if(s[i].isFree()){
                return i;
            }
        }
    }else if(type==VECTOR){
        for(RegId i=IDvector_last+1; i>IDvector;){
            --i;
            if(v[i].isFree()){
                return i;
            }
        }
    }else if(type==VMASK){
        for(RegId i=IDvmask_last+1; i>IDvmask; ){
            --i;
            if(m[i].isFree()){
                return i;
            }
        }
    }else if(type==VMASK2){
        assert( (IDvmask_last&1) && "last vector mask register should be odd" );
        for(RegId i=IDvmask_last-1; i>IDvmask; ){
            --i;
            if((i&1)) continue; // cannot start on odd-numbered %vm_i
            if(m[i].isFree() && m[i+1].isFree()){
                return i;
            }
        }
    }
    TODO("NEED CODE");
    if(spill){
        // TODO: for scalar registers, if last getfree symbol was assigned to
        // %s33,%s34,... and PRESERVE registers are free, REASSIGN an 'old'
        // getfree register to %s17-33 range (perhaps many getfree reg allocs
        // will change as a result).
        return IDlast;                      // no free registers
    }
    /* TODO */
    return IDlast;                      // no free registers (error?)
}

void Regs::changed(RegId const r) {
    assert( r < IDlast );
    Usage &u = (r<IDvector? s[r]: r<IDvmask? v[r-IDvector]: m[r-IDvmask]);
    assert( !u.isReserved() );
    if( u.isFree() ){
        assert( u.sym == nullptr && "free register with stale symbol info??" );
        //assert( !u.isFree() && "insisting that ALL regs have symbols??" );
    }else{
        assert( u.sym != nullptr );
        Sym *sym = u.sym;
        assert( sym->isActive() && "regchange symbol must be an active register" );
        sym->staleness = (sym->mem==Sym::NONE? 0: sym->staleness+1);
    }
}

#if 0 // not needed: I just allow holes to be as large as they want.
// Symbol            * sym.align
//         |-sym.len-|
// Input:
// 0------------------------------------------> -ve stack offsets        
// fp     |---prev---|      |---next---|
//        <-prev.len-|      <-next.len-|
//                   ^                 ^
//               prev.offset       next.offset
// becomes:
//        <--------------prev.len------|
//                                 prev.offset
void Spill::merge2prev( Region& prev, Region& next ){
    // preconditions
    assert( prev.sym == nullpte );
    assert( next.sym == nullptr );
    // assert Spill represents an ordered, "downward-growing" stack
    assert( next.offset < prev.offset );
    assert( next.offset <= prev.offset - prev.len );
    prev.offset = next.offset;
    prev.len = (prev.offset + prev.len) - next.offset;
}
#endif
void Spill::erase(Sym const* s){
    Uses::iterator prev = use.before_begin();
    Uses::iterator next = prev;
    for( ; ++next != use.end(); prev=next ){
        if( next->sym == s ){
            use.erase_after(prev);
            break;
        }
    }
    if( next == use.end() ){
        cerr<<" WARNING: asked to erase Sym "<<s->name<<" from spill, but it wasn't there!"<<endl;
        assert(false);
    }
    this->contiguity_check();
}

void Spill::contiguity_check() const {
    if( !use.empty() ){
        auto prev=use.begin();
        assert( prev->offset < 0 );
        assert( prev->offset >= this->bottom );
        assert( prev->len > 0 );
        assert( prev->offset + prev->len <= 0 );
        auto next=prev;
        for( ; ++next != use.end(); prev=next ){
            assert( next->offset < 0 );
            assert( next->offset >= this->bottom );
            assert( next->len > 0 );
            // spill offsets describe downward extending memory blocks (-ve offset, +ve len)
            // BUT the len might larger than necessary, for symbol alignment.
            // VE requires that frame pointer have 16-byte alignment.
            // (this is our "offset = 0" first spill block)
            // We might need to enhance prologue to guarantee 64-byte alignment, for vld ???
            // holes are possible
            assert( next->offset + next->len < 0 );
            assert( next->offset + next->len <= prev->offset );
        }
    }
}

static inline bool constexpr isPowTwo(int i){
    return (i!=0) && !(i & (i-1));
}
// if amask is 2^align - 1, align (a -ve offset) upwards...
static inline int constexpr alignup(int i, int amask){
    return (i+amask) & amask;
}
static inline int constexpr aligndown(int i, int amask){
    return (i & ~amask);
}
Spill::Uses::const_iterator Spill::find_hole( int const len, int const align ) const {
    assert( isPowTwo(align) );
    int amask = align - 1; // Ex. align 4 = 0100 --> amask 011
    int best = -1;
    Uses::const_iterator best_prev = use.before_begin();
    Uses::const_iterator prev=use.before_begin();
    Uses::const_iterator next=prev;
    {
        // iterate over all holes (assuming all Sym really need the mem)
        int otop=0, obot=0;                 // current hole
        int abot;                           // aligned version of obot
        //int olen=0;
        Uses::const_iterator const end = use.end();
        for( ; ++next != end; prev=next ){
            assert( next->sym != nullptr );
            assert( next->offset + next->len <= 0 );
            // try to find an existing hole with proper alignment
            obot = next->offset + next->len;
            assert( obot <= otop );
            //olen = otop - obot;
            abot = alignup(obot,amask);
            if( otop - alignup(obot,amask)  >= len ){ // Sym fits in hole.
                // score the fit, and remember the [first] best score
                //    alignment-excess costs more (I expect it to make small holes)
                int score = (otop-(abot+len)) + 2 * (abot-obot);
                if( best < 0 || score < best ){
                    best      = score;
                    best_prev = prev;
                }
            }
            // move to next hole
            otop = next->offset;
            // obot = otop; olen = 0;
        }
        if( best < 0 ){
            assert( next == end );              // sanity check
        }
    }
    return best>=0? best_prev: prev;
}

Spill::Uses::const_iterator Spill::newspill(Sym *sym){
    int align  = sym->align;
    assert( isPowTwo(align) );
    int amask = align - 1; // Ex. align 4 = 0100 --> amask 011
    int len = sym->len;
    // score all holes where it could go (minimize wasted bytes)
    // TODO: generic hole_find, returning 'best_prev'
    int best = -1;
    Uses::iterator best_prev = use.before_begin();
    Uses::iterator prev=use.before_begin();
    Uses::iterator next=prev;
    {
        // iterate over all holes (assuming all Sym really need the mem)
        int otop=0, obot=0;                 // current hole
        int abot;                           // aligned version of obot
        //int olen=0;
        Uses::iterator const end = use.end();
        for( ; ++next != end; prev=next ){
            assert( next->sym != nullptr );
            assert( next->offset + next->len <= 0 );
            // try to find an existing hole with proper alignment
            obot = next->offset + next->len;
            //olen = otop - obot;
            abot = alignup(obot,amask);
            if( otop - alignup(obot,amask)  >= len ){ // Sym fits in hole.
                // score the fit, and remember the [first] best score
                //    alignment-excess costs more (I expect it to make small holes)
                int excess = (otop-(abot+len)) + 2 * (abot-obot);
                if( best < 0 || excess < best ){
                    best      = excess;
                    best_prev = prev;
                }
            }
            // move to next hole
            otop = next->offset;
            // obot = otop; olen = 0;
        }
        if( best < 0 ){
            assert( next == end );              // sanity check
        }
    }
    // Option: repeat the hole search, but now we can
    // also re-use active but non-stale Sym memory (a la Spill::gc())
    // No... just finish up now ......................
    int otop=0, obot=0;         // current [or new] hole boundary
    if( best >= 0 ){            // found a good enough [internal] hole
        // regenerate the hole
        if( best_prev != use.before_begin() ){
            otop = best_prev->offset;
        }
        Uses::iterator next = best_prev;
        ++next;
        assert( next != use.end() );
        obot = next->offset + next->len;
    }else{                      // no internal hole.  extend this->bottom for Sym
        // failed hole-search left prev pointing at last Region (lowest offset)
        best_prev = prev;
        if( prev != use.before_begin() ){
            otop = prev->offset; // MIGHT be a wee bit higher than this->bottom
        }
        assert( otop >= this->bottom );
        obot = otop - sym->len;
        obot = aligndown( obot, amask );
        // optional:
        //this->bottom = aligndown(obot,16);
        // Actually, we can aligndown this->bottom when setting stack frame,
        // while we are generating the function prologue
        if( obot < this->bottom ){
            this->bottom = obot;
        }
    }
    int abot = alignup(obot,amask);
    Region r = {sym, abot, sym->len};
    Spill::Uses::const_iterator ret = use.insert_after( best_prev, r );
    this->contiguity_check();
    return ret;
}

void Spill::gc(){
    // Intent: forget memory areas that are now in
    //          "previously spilled, but now unspilled, registers"
    Uses::iterator prev=use.before_begin();
    Uses::iterator next=prev;
    for( ; ++next != use.end(); prev=next ){
        Sym* sym = next->sym;
        if( !sym->active ){                     // symbol not in-register right now
            assert( sym->staleness == 0 );      // only version is here, it better be good
            // we cannot gc any symbol that does not have a current register-value
        }else{
            assert( isReg(sym->id) );
            assert( sym->mem == Sym::SPILL );
            sym->mem       = Sym::NONE;
            sym->staleness = 0;
            use.erase_after(prev);      // remove next
            next = prev;                // and backtrack one step
        }
    }
}

void Spill::emit_spill( Sym *sym, Uses::const_iterator& at ){
    //TODO("!!!");
    assert( sym );
    assert( at != use.cend() );
    assert( at != use.cbefore_begin() );
    cout<<" emit_spill("<<sym->id<<":"<<sym->name<<",off"<<at->offset<<",len"<<at->len<<endl; cout.flush();
}

/**
 * - If the sym is !active, *neverhappens*
 * - If the sym is not in a register AND not in spill already, *neverhappens*
 * - If the sym is not in a register AND in spill already, *neverhappens*
 * - If the sym is active, try to find sym in spill list
 * 
 * - If the sym is active already spilled but stale, store at same spot again.
 * - else:
 *   - greedy search for free, long-enough area, assign it (reuse)
 *   - or extend \c bottom downward (new area)
 *
 */
void Spill::spill(Sym *sym, int align/*=8*/){
    assert(sym!=nullptr);
    //bool dospill = false;
    // HMMM. maybe active should mean non-register, and id can retain
    // knowledge of previous register allocation (for consistency in register use?)
    if( !sym->active ){
        //cerr<<"spill inactive symbol "<<sym->name<<" "<<asmname(sym->id)<<" is a no-op";
        cerr<<" recycle "<<sym->name;//<<" in "<<asmname(sym->id);
        //cerr<<"\n\tPossible: (a) declared & unused OR (b) used and freed"
        //    <<"\n\t"<<*sym
        //    <<"\n\tThis is a no-op!"<<endl;
        //NO: assert( !isReg(sym->id) ); // COULD still have a stale sym->id
        return;
    }
    //if( !isReg(sym->id) ) /* it better already be spilled */
    // search for sym in existing spill list
    auto const uend = use.cend();
    auto ubeg = find_if( use.cbegin(), uend,
            [&sym](const Region& x){ return x.sym == sym; } );
    if( !isReg(sym->id) ){      // active, non-register
        // TODO it MIGHT be findable in Args region
        if( ubeg==uend ){       // active, non-register, non-spilled
            cerr<<"\nError: active non-register symbol "<<sym->name<<" not found in spill list."<<endl;
            assert(false);
            return;
        }else{                  // active non-register spilled
            if( sym->staleness > 0){
                cerr<<"\nError: active non-register spilled symbol "<<sym->name<<" is stale."<<endl;
                assert(false);
                return;
            }else{
                cerr<<"\nWarning: active non-register symbol "<<sym->name<<" already spilled (no-op)."<<endl;
                // check align?
                return;
            }
        }
    }
    // possibly active register symbol
    if( ubeg!=uend && sym->staleness<=0 ){
        cerr<<"\nWarning: active reg symbol "<<sym->name<<" already spilled (removing RegId)."<<endl;
        // check align?
        sym->id = IDlast; // or maybe just sym->active=false;
        return;
    }else if( ubeg==uend ){
        cout<<" newspill..."; cout.flush();
        assert( sym->staleness == 0 );
        // ? align
        ubeg = newspill(sym);
    }

    emit_spill(sym,ubeg);            // TODO: generate code! 
    sym->id = IDlast;           // symbol no longer mapped to any register (sym->active=false)?
}

Scope::~Scope() {
#ifndef NDEBUG
    cout<<" -Scope"<<endl;
#endif
    //auto sBeg = syms.begin();
    //auto sEnd = syms.end();
    cerr<<" TODO: ~Scope"<<endl; cerr.flush();
    // TODO remove dangling pointers ???
}
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
