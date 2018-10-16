#ifndef SPILL_IMPL_HPP
#define SPILL_IMPL_HPP
/** \file
 * Spill area tracking: template functions */

#include "spill.hpp"

//#include "symbStates.hpp"
#include "throw.hpp"
#include <cassert>

#include <algorithm>

namespace ve {

template<class SYMBSTATES>
    void Spill<SYMBSTATES>::dump() const {
        using namespace std;
        cout<<" Spill:bottom="<<bottom<<" Regions={";
        for( auto const& r: use ){
            cout<<"\n\ts"<<r.symId
                <<"\to"<<r.offset
                <<"\tl"<<r.len
                <<"\t";
            //if(p(r.symId).getREG()){ cout<<"R"; }
            //if(p(r.symId).wasREG()){ cout<<"~R"; }
            cout<<(p(r.symId).getREG()? "R":"~R");
            if(p(r.symId).getMEM()){
                cout<<(p(r.symId).getStale()? "~M": "M"); 
            }
        }
        cout<<" }"<<endl;
        cout.flush();
    }

template<class SYMBSTATES>
    void Spill<SYMBSTATES>::contiguity_check() const {
        //std::cout<<" contiguity_check "; this->dump();
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

template<class SYMBSTATES>
    void Spill<SYMBSTATES>::validate(){
        std::cout<<" validate "; this->dump();
        typename Rgns::iterator prev = use.before_begin();
        typename Rgns::iterator next = prev;
        for( ; ++next != use.end(); prev=next ){
            assert( next->offset < 0 );
            assert( next->offset >= bottom );
            assert( next->len > 0 );
            assert( next->offset + next->len <= 0 );
            if(next->symId == 0){                   // this is a hole?
                if( prev != use.before_begin() ){
                    assert( prev->symId != 0U );                    // not 2 holes in a row
                    assert( next->offset + next->len == prev->offset );   // this hole is max-size
                }
            }else{                                  // this is a symbol?
                Sym const& s = p[next->symId];
                assert( s.getActive() );
                assert( s.getBytes() == next->len );
            }
        }
    }


// template functions for Spill class
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
template<class SYMBSTATES> inline 
typename Spill<SYMBSTATES>::Sym const& Spill<SYMBSTATES>::p(unsigned const symId) const {
    assert(symbStates != nullptr);
    return symbStates->psym(symId);
}

template<class SYMBSTATES> inline 
typename Spill<SYMBSTATES>::Sym      & Spill<SYMBSTATES>::p(unsigned const symId)       {
    assert(symbStates != nullptr);
    return symbStates->psym(symId);
}

template<class SYMBSTATES>
void Spill<SYMBSTATES>::erase(unsigned symId){
    typename Rgns::iterator prev = use.before_begin();
    typename Rgns::iterator next = prev;
    for( ; ++next != use.end(); prev=next ){
        if( next->symId == symId ){
            use.erase_after(prev);
            break;
        }
    }
    if( next == use.end() ){
        THROW(" WARNING: asked to erase Sym "<<p(symId).name<<" from spill, but it wasn't there!");
    }
    this->contiguity_check();
}

inline bool constexpr isPowTwo(int i){
    return (i!=0) && !(i & (i-1));
}
// if amask is 2^align - 1, align (a -ve offset) upwards...
inline int constexpr alignup(int i, int amask){
    return (i+amask) & ~amask;
    //return i>=0 ? (i+amask) & ~amask : -( (-i+amask) & ~amask );
}
inline int constexpr aligndown(int i, int amask){
    return (i & ~amask);
}

template<class SYMBSTATES>
typename Spill<SYMBSTATES>::Rgns::const_iterator Spill<SYMBSTATES>::find_hole( int const len, int const align ) const {
    assert( isPowTwo(align) );
    int amask = align - 1; // Ex. align 4 = 0100 --> amask 011
    int best = -1;
    typename Rgns::const_iterator best_prev = use.before_begin();
    typename Rgns::const_iterator prev=use.before_begin();
    typename Rgns::const_iterator next=prev;
    {
        // iterate over all holes (assuming all Sym really need the mem)
        int otop=0, obot=0;                 // current hole
        int abot;                           // aligned version of obot
        //int olen=0;
        typename Rgns::const_iterator const end = use.end();
        for( ; ++next != end; prev=next ){
            assert( next->symId != 0 );
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

template<class SYMBSTATES>
typename Spill<SYMBSTATES>::Rgns::const_iterator Spill<SYMBSTATES>::newspill(unsigned const symId){
    static int verbose=13;
    using std::cout;
    using std::endl;
    auto const& sym = p(symId);
    int const symBytes = sym.getBytes();
    int const align  = sym.getAlign();
    assert( symBytes > 0 );
    assert( align    > 0 );
    int const amask = align - 1; // Ex. align 4 = 0100 --> amask 011
    assert( isPowTwo(align) );
    // score all holes where it could go (minimize wasted bytes)
    // TODO: generic hole_find, returning 'best_prev'
    int best = -1;
    typename Rgns::iterator best_prev = use.before_begin();
    typename Rgns::iterator prev=use.before_begin();
    typename Rgns::iterator next=prev;
    {
        // iterate over all holes (assuming all Sym really need the mem)
        int otop=0, obot=0;                 // current hole
        int abot;                           // aligned version of obot
        //int olen=0;
        typename Rgns::iterator const end = use.end();
        for( ; ++next != end; prev=next ){
            assert( next->symId != 0 );
            assert( next->offset + next->len <= 0 );
            // try to find an existing hole with proper alignment
            obot = next->offset + next->len;
            //olen = otop - obot;
            abot = alignup(obot,amask);
            if( otop - alignup(obot,amask)  >= symBytes ){ // Sym fits in hole.
                // score the fit, and remember the [first] best score
                //    alignment-excess costs more (I expect it to make small holes)
                int excess = (otop-(abot+symBytes)) + 2 * (abot-obot);
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
            if(verbose){cout<<" no-hole0 "; cout.flush();}
            assert( next == end );              // sanity check
        }
    }
    // Option: repeat the hole search, but now we can
    // also re-use active but non-stale Sym memory (a la Spill::gc())
    // No... just finish up now ......................
    int otop=0, obot=0;         // current [or new] hole boundary
    if( best >= 0 ){            // found a good enough [internal] hole
        if(verbose){cout<<" rep-search? "; cout.flush();}
        // regenerate the hole
        if( best_prev != use.before_begin() ){
            otop = best_prev->offset;
        }
        typename Rgns::iterator next = best_prev;
        ++next;
        assert( next != use.end() );
        obot = next->offset + next->len;
        //
        // Note: will use lowest slot in a too-large hole!
        //       (You might expect to use the highest-possible slot)
        //
    }else{                      // no internal hole.  extend this->bottom for Sym
        // failed hole-search left prev pointing at last Region (lowest offset)
        best_prev = prev;
        if( prev != use.before_begin() ){
            otop = prev->offset; // MIGHT be a wee bit higher than this->bottom
        }
        assert( otop >= this->bottom );
        obot = otop - symBytes;
        assert( symBytes > 0 );
        obot = aligndown( obot, amask );
        // optional:
        //this->bottom = aligndown(obot,16);
        // Actually, we can aligndown this->bottom when setting stack frame,
        // while we are generating the function prologue
        if(verbose){cout<<" otop="<<otop<<" obot="<<obot<<" bot="<<bottom; cout.flush();}
        if( obot < this->bottom ){
            this->bottom = obot;
        }
        if(verbose){cout<<" bot-->"<<bottom<<endl; cout.flush();}
    }
    int abot = alignup(obot,amask);
    if(verbose){cout<<" alignup(obot="<<obot<<",amask="<<amask<<")-->abot="<<abot<<endl; cout.flush();}
    Region r = {symId, abot, symBytes};
    typename Rgns::const_iterator ret = use.insert_after( best_prev, r );
    this->contiguity_check();
    return ret;
}

template<class SYMBSTATES>
void Spill<SYMBSTATES>::gc(){
    // Intent: forget memory areas that are now in
    //          "previously spilled, but now unspilled, registers"
    typename Rgns::iterator prev=use.before_begin();
    typename Rgns::iterator next=prev;
    for( ; ++next != use.end(); prev=next ){
        auto const s = next->symId;
        if(s){                                          // if region assigned...
            Sym& sym = p(next->symId);
            if( !sym.getActive() || !sym.getMEM()            // symbol not in scope or mem
                    || (sym.getMEM() && sym.getStale() )){   // or mem value stale

                //next->symId = 0U;                         // symbol region --> hole

                if( sym.getActive() ){
                    sym.setMEM(false);
                    sym.unStale();
                }
                use.erase_after(prev);      // erase region
                next = prev;                // and backtrack one step to keep iterating
            }
        }
    }
    // TODO merge/expand holes
}

template<class SYMBSTATES>
void Spill<SYMBSTATES>::emit_spill( unsigned const symId, typename Rgns::const_iterator& at ){
    using namespace std;
    auto const& sym = p(symId);
    //TODO("!!!");
    assert( at != use.cend() );
    assert( at != use.cbefore_begin() );
    cout<<" emit_spill("<<sym.uid<<",o"<<at->offset
        <<",l"<<at->len<<")"<<endl; cout.flush();
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
template<class SYMBSTATES>
void Spill<SYMBSTATES>::spill(unsigned const symId
        , int align/*=8*/){
    int const verbose=13;        //0:none, 1:warn/err
    assert( symId );
    auto & sym = p(symId);
    assert( sym.uid == symId );
    using std::cerr;
    using std::cout;
    using std::endl;
    if(verbose>1){cout<<" spill("<<symId<<")"; cout.flush();}
    //bool dospill = false;
    // HMMM. maybe active should mean non-register, and id can retain
    // knowledge of previous register allocation (for consistency in register use?)
    //cout<<" a"<<sym->active; // protected/private ??????????
    if( !sym.getActive() ){
        if(verbose>0){cout<<"\nIgnoring attempt to spill inactive symbol "<<symId<<endl; cout.flush();}
        return;
    }
    if( !sym.getREG() ){
        if( !sym.getMEM() ){      // active, unused
            if(verbose>0){cout<<"\nIgnoring attempt to spill active, unset symbol "<<symId<<endl; cout.flush();}
            return;
        }
        assert( !sym.getREG() && sym.getMEM() );
        if( sym.getStale() ){
            THROW("Cannot spill non-reg, stale-mem symbol "<<symId);
        }else{
            if(verbose>0){cout<<"\nIgnoring attempt to spill non-reg, unstale-mem symbol"<<symId<<endl; cout.flush();}
            return;
        }
    }
    assert( sym.getREG() &&
            ( !sym.getMEM() || (sym.getMEM() && sym.getStale()) ));
    // search for sym in existing spill list
    auto const uend = use.cend();
    auto ubeg = find_if( use.cbegin(), uend,
            [&symId](const Region& x)
            { return x.symId == symId; } );
    if( !sym.getREG() ){
        // TODO it MIGHT be findable in Args region
        if( ubeg==uend ){       // active, non-register, non-spilled
            if(verbose>0){ cout<<"\nError: active non-register symbol "<<symId
                <<" not found in spill list."<<endl; cout.flush(); }
            assert(false);
            return;
        }else{                  // active non-register spilled
            if( sym.getStale() ){
                if(verbose>0){ cout<<"\nError: active non-register spilled symbol "<<sym.uid<<" is stale."<<endl; cout.flush(); }
                assert(false);
                return;
            }else{
                if(verbose>0){ cout<<"\nWarning: active non-register symbol "<<sym.uid<<" already spilled (no-op)."<<endl; cout.flush(); }
                // check align?
                return;
            }
        }
    }

    assert( sym.getREG() );

    // possibly active register symbol
    if( ubeg!=uend && sym.getMEM() && sym.getStale()==0 ){
        if(verbose>0){ cout<<"\nWarning: active reg symbol "<<sym.uid<<" already spilled??"<<endl; cout.flush(); }
        // check align?
        return;
    }
    if( ubeg==uend ){
        if(verbose>1){cout<<" newspill..."; cout.flush();}
        assert( !sym.getStale() );
        // ? align
        ubeg = newspill(symId);
    }else{
        if(verbose>1){cout<<" reuse{"<<ubeg->symId<<",o"<<ubeg->offset<<",l"<<ubeg->len; cout.flush();}
        ; // re-use previous spill location for this symbol
    }

    emit_spill(symId,ubeg);    // TODO: generate code! 
    sym.setMEM(true).unStale();
}

}//ve::
#endif // SPILL_IMPL_HPP
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
