#ifndef REGTREE_HPP
#define REGTREE_HPP
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
 */
#include <string>
#include <map>
#include <vector>
#include <assert.h>

namespace regTree {
    struct ProgNode;
    struct Root;

    /** \em Virtual Register Id.  Mapped to a real Register Id later. */
    typedef int VRegId;

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

    /** A ProgNode can represent a scope doing some calculation using:
     *
     * - temporary registers
     * - const data registers
     * - const data memory (TBD)
     * - global state registers
     * - global state memory
     *
     * No memory need be used if Root can hand out the regs for global date+state.
     */
    struct RegCode {
        enum Usage usage;
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
        }
        ~ProgNode();
        bool isRoot() const {return path.empty();}



        /// \group ProgNode track their own required registers
        //@{
    private:
        std::map<VRegId,RegCode> regs;

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
        //@}

        /** Store registers that we allocated, along with child who requested them.
         * Many ProgNodes may ask their parent (all the way up to Root) to allocate,
         * leaving \c r2n and \c n2r empty. */
        std::multimap<VRegId,ProgNode*> r2n;
        std::multimap<ProgNode*,VRegId> n2r;

    protected:
        ProgNode() : path(), scopeNode(nullptr) {}
    private:
        std::string name;
        std::vector<ProgNode*> path;    ///< path to Root node
        ProgNode * scopeNode;
    };

    /** For every ProgNode except Root, path[0] points to the Root object */
    struct Root : public ProgNode {
        // all registers initially free
        Root() : ProgNode(), cnts{0,0,0}, base{0U,0x80000000U,0x40000000U} {}

        /** Root maintains a global VRegId-->vregs map.
         * All finer detail available via the kernel ProgNode.
         */
        std::map<VRegId,ProgNode* /*kernel*/> vregs;

        VRegId allocate( Type const t ){
            return base[t] + (++cnts[t]);
        }

      private:
        uint32_t cnts[Type::NTYPE];
        uint32_t base[Type::NTYPE];
    };


}// regs
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp
#endif // REGTREE_HPP
