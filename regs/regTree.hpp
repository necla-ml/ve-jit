#ifndef REGTREE_HPP
#define REGTREE_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * WIP -- address some difficulties with SymScopeUid approach.
 *
 * After experience with loop fusion assembly with a kernel, I think
 * a better approach is to allocate virtual registers and post-process to
 * map to real registers.
 *
 * The original SymScopeUid approach had a mindset of maintaining only a
 * "current" register set.  So register usage looked like a \e stack,
 * with \b no notion of what the program \c tree looked like.
 *
 * Also, "kernels" have typical behaviors, like const regs, tmp regs,
 * state regs, pointers to allocated/stack/spilled memory.  Once you
 * have a program tree, the code snippets can be moved up and down
 * to adapt to total register pressure.
 *
 * Register allocation will be tricky!
 * Perhaps I can use a simpler string-based approach:
 *
 * - assembly code should never nest "too many" kernels.
 * - if register usage does not "look good" (or throws "Out of registers"),
 *   you adjust <B>by hand</B>
 */
#include "../throw.hpp"
#include <string>
#include <map>
#include <vector>
#include <list>
#include <assert.h>

namespace regTree {
    struct ProgNode;
    struct Root;

    /** \em Virtual Register Id.  Mapped to a real Register Id later. XXX borrow a better impl. */
    typedef int VRegId;

    /** Real Register Ids. XXX borrow a better defn. */
    typedef int RegId;

    /** Registers have various generic use types. */
    enum Usage {
        TMP         ///< TMP register
            , CONST     ///< CONST register (can move upward from scope)
            , RODATA    ///< ptr register
            , STATE     ///< state register must be initialized in scopeNode
            , STATEMEM  ///< ptr to state memory (larger state)
            , STACK     ///< ptr to stack-allocation (from Root?)
            , MEM       ///< ptr to alloc() (from Root, if mem area not too big)
    };
    /** Register Types -- there is a full feature RegType elsewhere. */
    enum Type {
        SCALAR=0, VECTOR, MASK, NTYPE
        //INT, FLOAT, VECTOR_FLOAT, VECTOR_INT
        };
    /** code production types */
    enum Emit {
        INIT, SPILL, LOAD, DONE };
    struct EmissionItem {
        ProgNode*  who;
        enum Emit  what;
    };
    typedef std::list<EmissionItem> EmissionInfo;


    /** A ProgNode can represent a scope doing some calculation using:
     *
     * - temporary registers
     * - const data registers
     * - const data memory (TBD)
     * - global state registers
     * - global state memory
     *
     * No memory need be used if Root can hand out the regs for global date+state.
     *
     * Code productions are done by \em deriving a class and overriding the
     * \c set_regs function to set up code string productions and cost guesses.
     */
    struct RegCode {
        /** RegCode are always associated with some ProgNode.  So adding this member
         * might simplify some things (and allow validity assertions). */
        ProgNode *owner;
        enum Usage usage;           ///< ex. TMP, CONST, STATE, ...
        std::string init();         ///< code to load register
        std::string spill();        ///< code to save register to memory (boilerplate?)
        std::string load();         ///< code load from memory to register
        std::string done();         ///< for memory, we might need some cleanup (usually not)
        int initCost();
        int spillCost();            ///< default: bytes()
        int loadCost();
        int doneCost();
        std::string name;           ///< adjust by path-prefixing if conflicts occur.
        enum Type type;
        int bytes;                  ///< zero to forbid (tmp regs, simple consts, vector regs)
    };

    struct ProgNode {
        friend class Root;
        /** Simple kernels objects begin and end in scope of direct-parent.
         * \pre parent is a valid ProgNode.
         * \post path[0]->isRoot() and  */
        ProgNode( std::string name, ProgNode * parent ) : name(name), path(), scopeNode(parent) {
            assert( parent != nullptr );
            assert( !name.empty() );
            // XXX remap same-name-on-tree to name_vNNN (add version suffix)
            path.resize(parent->path.size()+1);
            size_t i=0U;
            for( ; i<parent->path.size(); ++i){
                ProgNode * const pn_i = parent->path[i];
                path[i] = pn_i;
            }
            path[i] = parent;
            assert( path[0]->isRoot() );
            set_regs();
        }
        /** More complicated kernels may begin and end in some higher object scope.
         * \pre \c scopeNode is on path to \c Root.
         *
         * \c scopeNode is logically responsible for allocating a RegId.
         * Ex. When performing a reduction, you initialize perhaps in a scope
         *     a few loops up from your immediate parent.
         *
         * In cases of register scarcity it is better to wait until we know the
         * full ProgNode tree under Root.  Perhaps we will even have execution count
         * info.  So we initially allocation 'virtual' registers only, and later
         * we do the more complicated reassignments of RegCode::init()
         * We will do the llvm thing of allocating virtual registers, and \c after
         * simulation we may know full ProgNode tree and execution count info.
         *
         * After mapping virtual registers to real ones, the final resting place
         * of all init/spill/load/done code snippets are known, and a real
         * program can be output.  We also know about all the spill and memory allocations,
         * so Root knows better what it should do.
         */
        ProgNode( std::string name, ProgNode * parent, ProgNode *scope ) : name(name), path(), scopeNode(scope) {
            assert( parent != nullptr );
            path.resize(parent->path.size()+1);
            size_t i=0U;
            bool scope_ok = false;
            for( ; i<parent->path.size(); ++i){
                ProgNode * const pn_i = parent->path[i];
                path[i] = pn_i;
                if( scope == pn_i ) scope_ok = true;
            }
            path[i] = parent;
            assert(scope_ok);
            assert( path[0]->isRoot() );
            set_regs();
        }
        ~ProgNode();
    private:
        std::string name;
        std::vector<ProgNode*> path;    ///< path to Root node
        ProgNode * scopeNode;
    public:
        bool isRoot() const {return path.empty();}
        ProgNode& scope() const { if(!scopeNode) THROW("scope unset?");
            return *scopeNode;
        }
        ProgNode *parent() const {if(path.empty()) THROW("Root ProgNode has no parent");
            return path.back();
        }

        /// \group ProgNode internal info
        //@{
        /** increment execution counter */
        void incExec(int count=0) { execCount += count; }
        /** get execution counter */
        int getExec() const { return execCount; }

        /** look up RegCode info for this kernel */
        RegCode& rc( VRegId const vr ) const {
            assert( regs != nullptr );
            return regs->at(vr);
        }

#if 0
        // maybe have the allocator produce a "mirror" tree for Root, containing
        // the complicated data structure manouvering that goes on during register
        // allocation.

        /** Code emission needs to know what code snippets get produced in which
         * \b parent ProgNodes. Map \e our VRegId to \e parent (or this) EmissionInfo */
        std::map< VRegId, EmissionInfo > emissions;

        /** The inverse of \c vr_emit tells us for which child \c ProgInfo must
         * we perform code output.  When outputing, for each pair \c (child,vr),
         * we execute the code-string method of child->rc(vr).init()|spill()|... */
        std::vector< ProgInfo * /*child*/, VRegId > child_work;
#endif

    private:
        /** construction is only complete once \c this->regs and \c this->code is set up.
         * It looks like I might want something like 'cblock.hpp' for asm code too. */
        virtual void set_regs() {}
        /** This map is const info once this prognode is constructed in \c set_regs(). */
        std::map<VRegId,RegCode> *regs;

        int execCount;
        //@}
        


        /// \group register allocators
        /** A \c child kernel select a parent scope for the lifetime of the
         * kernel and its state.
         *
         * - A \c child Kernel has phases:
         *   - INIT : (mem/reg alloc/init, constants and state vars)
         *   - EXEC : kernel code (extra temp regs, modify state vars)
         *   - DONE :
         *
         * - INIT \e state regs/mem \b must be set here.
         *   - if regs scarce, set them and spill them, reloading into regs
         *     as soon as possible on path from here to child.
         * - END  cleanup \b must execute here
         *
         * - INIT const regs move rootward, as high as possible
         *   - in bad cases, const regs may be spilled to memory and loaded
         * - INIT precalculated const mem is moved to root (with code)
         * - INIT calculated    const mem is moved to root (with code)
         *   - pointers to mem are loaded rootward if possible,
         *     as if they were 'const regs'
         *
         * In general a \c child kernel select \c this scope, and \c this may punt code
         * and register allocations elsewhere depending on register pressure.
         *
         * 1. registers/code moved rootward (we have spare regs)
         * 2. registers/code moved downward (we are running out of regs)
         *
         * \c child registers and their init code remain paired.  Code is
         * located/generated within the child node.
         */
        //@{
#if 0
    public:
        VRegId allocate( ProgNode* child, RegCode &regCode );
    private:
        /** Temp registers may given to an immediate child.
         * Single-purpose tmp regs improve readability.
         * We are allowed to move upwards to do this, in hopes that
         * sequential children use different tmp regs, as much as possible.
         * However, after such consultation, the parents are unaffected.
         *
         * This call \b must succeed.  Register scarcity may force us to
         * search upwards for a low-cost spill
         *
         * \pre child->path.back() == this (we are direct parent)
         */
        VRegId allocateTmp  ( ProgNode* child, RegCode &regCode );
        VRegId allocateState( ProgNode* child, RegCode &regCode );
        VRegId allocateMem  ( ProgNode* child, RegCode &regCode );
        VRegId allocateStack( ProgNode* child, RegCode &regCode );
        // etc.
#endif

        /** Store registers that we allocated, along with child who requested them.
         * Many ProgNodes may ask their parent (all the way up to Root) to allocate,
         * leaving \c r2n and \c n2r empty. */
        std::multimap<VRegId,ProgNode*> r2n;
        std::multimap<ProgNode*,VRegId> n2r;
        //@}

    protected:
        ProgNode() : path(), scopeNode(nullptr) {}
    };

    /** For every ProgNode except Root, path[0] points to the Root object */
    struct Root : public ProgNode {
        /** set up initial register state. Only "ve" is supported for now. */
        Root(char const* machine="ve") : ProgNode()
            , machine(machine)
            , cnts{0,0,0}
        , base{1U,0x80000001U,0x40000001U}
        , allocated_regs(false)
        {}

        /** Always, you can look up child ProgNode \c pn of any VRegid.
         * throw if \c vr not defined. */
        ProgNode& pn(VRegId const vr){
            return *vregs.at(vr);
        }

        /** After allocation you can lookup actual register assignments */
        RegId rid(VRegId const vreg){
            assert( allocated_regs );
            return this->v2real.at(vreg);
        }

        VRegId vreg( Type const t ){
            assert( !allocated_regs );
            return base[t] + (++cnts[t]);
        }

#if 0
        /** TODO register allocator. \post allocated_regs == true. */
        void allocate_global();
#endif

      private:
#if 1
        std::map<VRegId,RegId>::value_type allocate_global( ProgNode *pn, RegCode const& rc ){
            std::map<VRegId,RegId>::value_type ret = std::make_pair(VRegId(0),RegId(0));
            if( !pn->isRoot() ){
                // punt upwards, in general looks like
                //return allocate(pn->path.back());
                // but we are going all the way to root
                assert( pn->path.at(0) == this );
                ret = allocate_global(this,rc);
                assert(ret.first != 0 );
            }else{
                // Root grabs a register "forever", or throws (or returns pair(0,undefined)?)
                ; //find_free_reg(rc.type);
            }
            return ret;
        }
#endif


        /** Root maintains a global VRegId-->vregs map.
         * All finer detail available via the kernel ProgNode.  For example, every
         * ProgNode register may point to ProgNodes on its path to Root:
         *
         * - what parent scope[s] initialize/spill/load this register content.
         * - this map may dynamically change during allocation
         *   - ex. const register loads may migrate Root-wards.
         *
         * Ex. RegCode rc = root.pn(vr).rc(vr)
         *     ProgNode& scopeNode = root.pn(vr).scope()
         */
        std::map<VRegId,ProgNode* /*kernel*/> vregs;



        uint32_t cnts[Type::NTYPE];
        uint32_t base[Type::NTYPE];

        char const* machine;
        bool allocated_regs;

        // 'machine' is used to initialize available registers:
        //struct RegSet all; all available real registers
        //  --> char const* regNames[];
        //  --> int         nRegs[];
        char const* regNames[]; // of various Type (SCALAR,VECTOR,MASK)
        char const* nRegs[];

        std::map<VRegId,RegId> v2real;
    };


}// regs
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp
#endif // REGTREE_HPP
