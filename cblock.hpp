#ifndef CBLOCK_HPP
#define CBLOCK_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "throw.hpp"
/** \file
 * Arange statements (std::string) into blocks.
 *
 * Problem Description:
 *
 * - JIT units generate vector<string> Cblock for operations.
 * - When output as a program, or in subsequent optimizations,
 *   Cblocks may be output:
 *   - sequentially (easy, "linear program")
 *   - interspersed:
 *     - Ex. for loop --> pre-loop, induction, kernel={Cblock}, test
 *     - Ex. optimization which reorders one statement of next Cblock
 *           into a Cblock's vector<string>, to simplify dependencies,
 *           or balance work among execution units.
 *     - Ex. linear combination split up into inductive calculation with
 *           different factors accumulated from different loops.
 *
 * Without worrying about details, we first need some machinery for
 * dealing with creating and stitching together these vector<string>.
 *
 * For creation, we use ostringstreams.  These get flushed into vector<string>
 * and annotated with block names, and can be relinked into non-linear output
 * order to form a 'C' program unit [which hopefully compiles].
 *
 * 'C' code emission can largely be decided by hand, because the compiler can
 * can adjust register allocations once the code is about right.
 *
 * For assembly output, a more complex approach is needed (with some builtin
 * smarts about which node set/spills/loads registers.  One register can potential
 * occur in code emissions of several parent nodes!
 **/
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <cassert>
#include <sstream>
#include <algorithm>    // std::max

// CBLOCK_DBG disabled (things running OK now)
#if 1 || defined(NDEBUG)
#define CBLOCK_DBG_ENABLED 0
#define CBLOCK_DBG(V,N,...) do{}while(0)
#else
#define CBLOCK_DBG_ENABLED 1
#define CBLOCK_DBG(V,N,...) do { \
    if((V)>=(N)){ \
        std::cout<<__VA_ARGS__; \
    } \
}while(0)
#endif

/** \c STR(...) eases raw text-->C-string (escaping embedded " or \\) */
#ifndef CSTR
#define CSTRING0(...) #__VA_ARGS__
#define CSTR(...) CSTRING0(__VA_ARGS__)
#endif

namespace cprog {

struct Cunit;
class Cblock;
struct CbmanipBase;

struct IndentSpec;
struct PostIndent;
struct PreIndent;

/// \group Cblock I/O helpers
//@{
/** stateless Cblock output manipulators for operator<< */
template<class T> inline T& operator<<(T& t, T& (*manip)(T&) ){ return manip(t); }

/** \c Endl to avoid confusion with \c std::endl that applies only to ostream& */
template<class T> inline T& Endl(T& t){ return t<<"\n"; }

/** Base Cblock/Cunit manipulator class.
 * Cunit may store "context", like current indent,
 * so allow access to "/".
 * Possible overkill (maybe Cblock::getRoot() is all that's needed?)
 * OTOH, maybe this way eases 'friend' decls.
 */
struct CbmanipBase {
    Cblock* cb;
    virtual std::ostream& write(std::ostream& os) {
        os<<" /*CbmanipBase*/ "; // debug!
        return os;
    }
    CbmanipBase(CbmanipBase const& src)        = delete;
    CbmanipBase& operator=(CbmanipBase const&) = delete;
    virtual ~CbmanipBase() {}
    Cunit* getRoot() const;
  protected:
    friend class Cblock;
    CbmanipBase() : cb(nullptr) {}
    CbmanipBase(Cblock& cb) : cb(&cb) {}
};
inline std::ostream& operator<<(std::ostream& os, CbmanipBase & cbmanip){
    return cbmanip.write(os);
}
//@}

//template<class T> struct endl {;}; // Nope. confusion with std::endl
class Cblock {
  public:
    /// Empty Cblock constructor (placeholder)
    Cblock(Cunit *root, std::string name="root")
        : _root(root), _parent(this), _name(name), _type(""),
        // _parent==this means we are _root
        _premanip(nullptr), _code(""), _sub(), _postmanip(nullptr),
        _nwrites(0), _maxwrites(1)
        {}
    /// Sub-block constructor
    Cblock(Cblock *parent, std::string name="")
        : _root(parent->_root), _parent(parent), _name(name), _type(""),
        _premanip(nullptr), _code(""), _sub(), _postmanip(nullptr),
        _nwrites(0), _maxwrites(1)
        {}

    /** Find \c p in \c sub -Cblocks, or appending a new Cblock to \c _sub
     * when \c p is a single <em>path component</em> \c p;
     * O/w if \c p is \b not a <em>path component</em> (i.e. something with '/')
     * then we execute \c at(p), which matches a \em first-found existing
     * block, or throws (\c at never creates a new block).
     * \throw if \c p is a path-string with '/' and \c p is not found.
     * see \c find(path) for a description of how path searches are done.
     *
     * "beg" and "end" are speical sub-block names related to the
     * CBLOCK_SCOPE macro.
     *
     * NEW: any Cblock named "last" will remain as the terminal one, with
     *      later sub-blocks appearing just-before-last.
     */
    Cblock& operator[](std::string p);
    /** shift-left operator appends codeline \e as-is. */
    Cblock& operator<<(std::string codeline) { return append(codeline); }
    /** prepends a newline, then adds codeline (same precedence as <<). */
    Cblock& operator>>(std::string codeline); // { return append("\n").append(codeline); }
    /** \c codeline append to \c _code (\c Cblock appends to \c _sub).
     * Mostly append as-is, \em except if last line of code has a ';' in it,
     * we add a newline (tweak for C-code readability). */
    Cblock& append(std::string codeline);
    /** Sub-block \c cb appends to \c _sub, except if the terminal \c sub
     * block is named "last", where \c cb is inserted just-before-last. */
    Cblock& append(Cblock &cb);
    // shorten append usage...
    //Cblock& operator<<=(Cblock &cb) {return this->append(cb); }
    //Cblock& operator<<(std::initializer_list il);
    //Cblock& append(std::string name, std::initializer_list<std::string>
    // Hmmm maybe provide canned procedures that define an unlinked block, that
    // gets immediately \c .after()'ed to hook it to some location.
    /** unlink \c this and append it to some \c prev Cblock.
     * equiv prev.append(unlink())
     * I guess this could be called "under".
     * \return \c *this
     */
    Cblock& after(Cblock& prev);
    /// place after absolute path (can be wildcarded) \c abspath.
    /// \throw if \c abspath cannot be found.
    Cblock& after(std::string abspath) { 
        //std::cout<<" after(abspath="<<abspath<<")"; std::cout.flush();
        assert( abspath[0] == '/' );
        return after(at(abspath));
    }

#if 0
    /// \group define/undef scoping
    /**
     * WIP: both asm and C need #defines. Reimplement AsmFmtCols 'scope'
     *      concept within Cblock [more powerfully].
     *
     *   Cblock                         | AsmFmtCols
     *   ======================================================================
     *   macro scoping not so important | Parentage used for scope to get
     *                                  | register assignments (important)
     *   ----------------------------------------------------------------------
     *   program is a tree              | AsmFmtCols snippets stitched together
     *                                  | "by hand" to situate undef outputs
     *   ----------------------------------------------------------------------
     *   lines input as strings         | line input helper functions, that
     *                                  | encapsulate simple assembly
     *                                  | optimizations
     *   ----------------------------------------------------------------------
     *   verbatim line output,          | columnar asm output
     *   (apart from indent)            | with scope indent typically zero
     *   -----------------------------------------------------------------------
     */
  private:
    /** Each scope could have a vector of macro --> substitution strings.
     * For cpp+assembler output, this is quite important, as macros are
     * used (abused) to handle register assignments.  As such, every
     * macro has a corresponding \#undef.  We can package a bunch of
     * macro emissions together in \c StringPairs. */
    struct StringPairs
        : public std::vector<std::pair<std::string,std::string>>
    {
        /// Convenient \c push_back trimmed versions of \c name and \c subst.
        void push_trimmed(std::string name, std::string subst);
    };
    struct Defines : public StringPairs {
        /** In the final tree, an \c un_path (absolute/relative) must also
         * be "forward" of \c this Cblock.  Hmmm, a new type of relative
         * search could enforce this.
         *
         * - asm
         *   - define/undef scoping critical for register tracking
         *   - default un_path is "." for assembly (too local?) */
         * - C:
         *   - defines/macros often non-local convenience items.
         *   - default un_path is "" (C usually lazy about undefs) */
         */
        std::string const un_path;
        Defines() : {}
        /** undef bindings are late, to allow one to specify un_path that
         * has not yet been created.  A final pass to walk the tree, locate
         * undef Cblocks, and emit the undefs happens as a write() starts
         * up.  Un-emittable \c un_path is a serious error for root write().
        */
        std::string last_un_path;
    };
    std::vector<StringPairs> stack_defs;
    /** Convert \c StringPairs macros into multiline \#undefs string. */
    std::string defs2undefs( StringPairs const& macs, std::string block_name );
  public:
#endif

    int nWrites() const {return _nwrites;}
    /** Note: write has a strange behaviour of emptying the string.
     * <B>Subject to change</B> \deprecated */
    std::ostream& write(std::ostream& os, bool const chkWrite=true);
    /** Maybe for unrolling we have a max number of writes ? \deprecated */
    bool canWrite() { return _nwrites>=0 && _nwrites<_maxwrites; }
    /** depth-first tree dump */
    std::ostream& dump(std::ostream& os, int const ind=0);
    /** return string for subtree, unindented, no effect on \c write */
    std::string str();
    /** return immediate code string, no subblocks, ex for empty check */
    std::string const& code_str() const {return this->_code;}

    /** reset code, subblocks and pre-/post-manipulators */
    Cblock& clear();
    /** swap all `_code` for something new */
    Cblock& set(std::string s) {_code=s; return *this;}
    Cblock& setName(std::string type); ///< `{this->type=type; return *this;}` and update root!
    Cblock& setType(std::string type) {_type=type; return *this;}
    std::string const& getName() const;
    std::string const& getType() const;
    Cunit& getRoot() const;
    //Cblock& append(std::string code) {this->code += code; return *this;} // maybe inefficient
    //Cblock* next();
    //Cblock* prev()
    ~Cblock(){ clear(); }
    bool isRoot() const { return _parent == this; }
    /// \group path functions
    //@{
    std::string fullpath() const;
    /** find first Cblock matching \c path, with wildcard extensions.
     * - '/' begin at root
     * - '.' find under \c this Cblock.
     * - '..' find under \c _parent Cblock.
     * - '*' match any one subdir (i.e. sub-Cblock)
     * - '**' match zero or more subdirs
     * - '..*' \em strange recursive upward parent+subtree search excluding sub-tree of \c this
     *   - also called \c up since this is not a common path convention.
     * \return nullptr if not found.
     */
    Cblock *find(std::string path) const;
    /** utility for dot-dot-star wildcard, <em>find-in-upwards-subtrees</em>,
     * a Cblock match \em close to \c this, but \b not underneath \c this.
     * Note: could have taken ..* to mean "match anywhere on path to root" to root,
     *       but "..\*\/\*\*\/path" doesn't quite exclude the subtree of \c this
     */
    Cblock *up(std::string path) const { return this->find("..*/"+path); }
    /** \c find but throw if not found */
    Cblock& at(std::string path) const {
        Cblock *ret = find(path);
        if( ret == nullptr ) THROW(" Cblock["<<fullpath()<<"].at("<<path<<") not found");
        return *ret;
    }
    //Cblock& find(std::string p) const;
#if 1
    /** Easy \c \#define \c \#undef attached to nearest-enclosing-scope.
     *
     * - cb.def("M","S") -- #define M S  and #undef M
     *   - if node is "body" put def+undef at node.at("..") and node.at("..")["last"]
     *     - probably also have "../beg" and "../end" nodes
     *   - this is a subcase of general method to use "nearest enclosing scope":
     *     - node.at("..*\/body\/..") and node.at("..*\/body\/..")["last"]
     * - Fancier control could be had manually, like:
     *   - cb.at("..*\/loop_high/body").def("M","S"); to force upward scope
     *   - consider how to change arb cblock into a scoped one.
     *     - Ex. foo--> foo_scope/{beg,body,end} where body "is" the old node "foo" ?
     *
     * \return *this
     */
    Cblock& define(std::string name, std::string subst="");
    /** define with narrow scope (current point in this, to this->["last"]["undefs"]). */
    Cblock& define_here(std::string name, std::string subst="");
    /** For ease of JIT mirroring codes, if default output format of \c t is
     * enough, you can <EM>\#define name string_representation_of_t</EM>.
     * Often you will set up a macro so the the JIT \c name is the same as the
     * variable name (and the JIT substitution is its text).
     * Example:
     * ```
     *   int const million = 1000000;
     *   cb.def("million",million);
     *   // or some macro \#define DEF(CB,VAR) (CB).def(#VAR,VAR) 
     * ```
     * should yield `#define million 1000000`.  Using \c define or \c def
     * should replace the old way of suppying \e both CONST1(VAR) and FREE1(VAR)
     * as ">>"-strings, which has maintainability issues with scoping.
     *
     * - The default output format of \c t may be insufficient
     *   - (Ex: float precision, long long unsigned type, "27(0)" asm consts,
     *     or you want an expression substituted)
     *   - then you should \c define(name,subst) with the \c subst you need.
     */
    template<typename T>
    Cblock& def(std::string name, T const& t){
        std::ostringstream oss;
        oss<<t;
        return this->define(name, oss.str());
    }
    template<typename T>
    Cblock& def_here(std::string name, T const& t){
        std::ostringstream oss;
        oss<<t;
        return this->define_here(name, oss.str());
    }
#endif

  private:
    /** find first \b single-component path \c p for simple search strategy.
     * return nullptr if not found. */
    Cblock * find_immediate_sub(std::string p) const;
    Cblock * find_recurse_sub(std::string p) const;
    Cblock * find_recurse_parent(std::string p) const;
    //@}
  private:
    friend struct Cunit;
    /** Remove \c this from \c _parent.sub[] .
     * \throw if attempting to unlink the root
     * \post \c _parent==nullptr
     */
    Cblock& unlink();
  private:
    friend struct CbmanipBase;
    struct Cunit * const _root;
    class Cblock * _parent;
  private:
    std::string _name;              ///< terminal \e path component (from root)
    std::string _type;              ///< unused?
    CbmanipBase* _premanip;         ///< TODO support multiple?
    std::string _code;
    std::vector<Cblock*> _sub;      ///< TODO separate '_exit' block with \e always-last semantics?
    CbmanipBase* _postmanip;        ///< TODO support multiple?
    int _nwrites;   // counter
    int _maxwrites; // limit for _nwrites
    friend Cblock& operator<<(Cblock& cb, PostIndent const& postIndent);
    friend Cblock& operator<<(Cblock& cb, PreIndent const& preIndent);
    //friend Cblock& operator<<(Cblock& cb, Endl<Cblock> const&);
    template<class T> friend T& Endl(T& t);
};
template<> inline Cblock& Endl<Cblock>(Cblock& cblock){
    cblock._code.append("\n"); // since frequent, cut out some intermediate functions
    return cblock;
}

struct Cunit {
    std::string name;       ///< maybe Cunit subtrees might be copied for unrolling ??
    Cblock root;
    int v; // verbosity
    std::string indent;                             ///< internal Cblock::write context
    std::string const flavor;                       ///< [WIP] "C" or "asm"
    int shiftwidth;
    //std::map<std::string, Cblock*> blk;
    Cunit(std::string name, std::string flavor="C", int const verbose=2 )
        : name(name), root(this,name), v(verbose),
        indent(), flavor(flavor), shiftwidth(flavor=="C"? 2: 0)
    {}
    // Should we warn if anything in DAG is un-emitted?
    // Do we make temporary copies that should silently destruct?
    ~Cunit() { root.clear(); }
    std::ostream& write(std::ostream& os) {return root.write(os);}  ///< write the program unit
    Cblock *find(std::string path);                 ///< absolute \c path down from \c root
    Cblock *find(std::string path, Cblock* from);   ///< search up \c from, then down from \c root
    std::string str();                              ///< all code of root
    void dump(std::ostream& os);                    ///< dump the tree
    std::string tree();                             ///< tree structure of root
    Cblock & operator[](std::string name) { return root[name]; }
};
inline void Cunit::dump(std::ostream& os){
    root.dump(os);
}
inline std::string Cunit::str(){
    return root.str();
}
inline std::string Cunit::tree(){
    std::ostringstream oss;
    root.dump(oss);
    oss<<std::endl;
    return oss.str();
}
struct IndentSpec {
    IndentSpec(int const indent_adjust, char const fill=' ')
        : indent_adjust(indent_adjust), fill(fill) {}
    int const indent_adjust;
    char const fill;
};

inline Cunit* CbmanipBase::getRoot() const {return cb->_root;}

inline Cunit& Cblock::getRoot() const {return *_root;}

struct Cbin : public CbmanipBase, public IndentSpec {
    Cbin(Cblock& cb, int const adj, char const fill=' ')
        : CbmanipBase(cb), IndentSpec(adj,fill) {}
    Cbin(Cblock& cb, IndentSpec const& indentSpec)
        : CbmanipBase(cb), IndentSpec(indentSpec) {}
    virtual ~Cbin() {}
    std::ostream& write(std::ostream& os) override {
        std::string& in = getRoot()->indent;
        if(indent_adjust > 0 && in.size() < 128)
            in += std::string(indent_adjust,fill);
        else if(indent_adjust < 0)
            in.resize(std::max(ssize_t{0},(ssize_t)in.size()+indent_adjust));
        //std::cout<<" in.size()="<<in.size()<<std::endl;
        return os;
    }
};

struct PostIndent : IndentSpec {
    PostIndent(int const indent_adjust, char const fill=' ')
        : IndentSpec(indent_adjust,fill) {}
};
struct PreIndent : IndentSpec {
    PreIndent(int const indent_adjust, char const fill=' ')
        : IndentSpec(indent_adjust,fill) {}
};

inline std::string const& Cblock::getName() const {return _name;}
inline std::string const& Cblock::getType() const {return _type;}

inline Cblock& Cblock::after(Cblock& prev) {
    // streamlined, with 'append' that returns the argument, instead of 'prev'
    CBLOCK_DBG(_root->v,2," Cblock["<<fullpath()<<"].after("<<prev.fullpath()<<")\n");
    return prev.append(unlink());
}

/// \group Cblock/Cunit helpers
//@{ //}
/** for simple scopes (terminate with just "}", and with a "body" sub-block...
 * - if AFTER is a cblock, we could just use AFTER.getRoot() and save an argument
 * - general case is a bit more flexible, but macro is fairly readable if combined with indenting
 * - create subnodes \e CBLK_VAR/{beg,body,exit}
 * \c BEG is code put into "CBLK_VAR/beg"
 * \c CBLK_VAR ends up pointing at "CBLK_VAR/body" node
 * \c AFTER is the node 'after' which CBLK_VAR gets inserted.
 *
 * - includes semicolon, for use as <em>CBLOCK_SCOPE(foo,"if(1)",cunit,"parent") { foo>>"//HI"; }</em>
 *
 * - Among many ways to structure producing sub-blocks, keeping good 'locality' is important
 *   so that JIT subroutines become more independent of the enclosing parent.  So in the following,
 *   code snippets related to loop_s are positioned relative to loop_s, rather than explicitly
 *   assuming anything about the structure of the parent, loop_r/body.
 *
 * Example
 * ```
 * CBLOCK_SCOPE(loop_r,"for (int64_t r = kh_beg; r < kh_end; ++r)",pr,loop_x0);
 * CBLOCK_SCOPE(loop_s,"for (int64_t s = 0; s < kernWidth; s++)",pr,loop_r);
 * // the path of loop_s is ...loop_x0/body/loop_r/body/loop_s/body
 * loop_s[".."]>>"vrs = vrs0;";     // into loop_r/body/loop_s CODE, **before** loop_s/beg opens the loop
 * loop_s["last"]>>"update(vrs);";  // loop_s/body/last, always just before loop_s/body/end exits the loop
 * loop_s[".."]["last"]>>"cout<<vrs; // debug the final value"; //after loop_s exits
 * loop_s>>"FOO;";
 * CBLOCK_SCOPE(loop_c,"for (int64_t c = 0; c < inChannelGroup; ++c)",pr,loop_s);
 * loop_c>>"STUFF;"
 * loop_s>>"//before loop_c";
 * loop_s["postc"]>>"//after loop_c";
 * ```
 * Will produce something like
 * ```
 * for (int64_t r = kh_beg; r < kh_end; ++r){ // loop_r
 *   vrs=vrs0; // tight binding to "loop_s" is more robust than binding to loop_r
 *   for (int64_t s = 0; s < kernWidth; s++){ // loop_s
 *     FOO;
 *     //before loop_c
 *     for (int64_t c = 0; c < inChannelGroup; ++c); // loop_c
 *       STUFF;
 *     }
 *     //after loop_c
 *     update(vrs); // in loop_s["last"], no matter what other sub-blocks were created
 *     }
 *   }
 *   cout<<vrs; // debug the final value
 * }
 * ```
 * where all snippets inside loop_r are actually tightly associated with loop_s.
 * So that a function to easily embed loop_s JIT code \e wherever can be envisioned.
 *
 * Note: All nodes can be made into subtress with ["xxx"].  Often you can write
 * code snippets whenever to a nodes code string.
 * Ex. loop_s[".."]>>"#pragma unroll(4)"; will be incorrect if a subsequent
 * loop_s[".."] sets up the induction variable \c vrs, because the pragma must
 * be \e "immediately-before".
 * 
 * While you delay the pragma production to properly order it, a more robust idiom is:
 * ```
 * int const unroll_s = 4; // or some calculated max value?
 * CBLOCK_SCOPE(loop_s,
 * OSSFMT("#pragma unroll("<<unroll_s<<")\n"
 *        "for (int64_t s = 0; s < kernWidth; s++)", pr, loop_r))
 * ```
 * A monolithically inserted code snippet can never be broken apart by Cblock ops.
 *
 * Other tricks to force node order include pre-defining code blocks and retaining
 * references: `auto& a=cb["a"]; auto& b=cb["b"]; auto& c=cb["c"]; b>>"//to B";`.
 *
 * Or you can play with the generator's positioning of the ["xx"] creation
 * operation, because this generates node xx at the end of all \e currently-known
 * nodes.
 *
 * Other times, conditionally required code snippets require special declarations
 * or cleanup code elsewhere.  Special care is needed (no generic quick-fix).
 *
 * up writing to loop_s["last"], sometimes to be safer things like Cblock::define will actaully write to loop_s["last"]["last"]
 */
#define CBLOCK_SCOPE(CBLK_VAR,BEG,CUNIT,AFTER) auto& CBLK_VAR = mk_scope((CUNIT),#CBLK_VAR,(BEG)).after(AFTER)["body"];
/** Sometimes you want to pass the block "body" to an outer scope as a pointer rather than a ref */
#define CBLOCK_SCOPE_PTR(CBLK_VAR,BEG,CUNIT,AFTER) (&( mk_scope((CUNIT),#CBLK_VAR,(BEG)).after(AFTER)["body"] ))
/** \return empty if N<0, #pragma nounroll if N==0, or #pragma unroll(N). */
std::string ve_pragma_unroll(int64_t const N);
/** Usage \c OSSFMT(UNROLL(u)"for(...)"). */
#define UNROLL(N) ve_pragma_unroll(N)<<
#define NO_UNROLL(...) "#pragma nounroll\n"
/** for-loop macro allowing tight-binding #pragma */
#define CBLOCK_FOR(CBVAR,UN_ROLL,INTRO,CBPARENT) \
    CBLOCK_SCOPE(CBVAR,OSSFMT(UNROLL(UN_ROLL) INTRO),CBPARENT.getRoot(),CBPARENT)


/** make name/{beg,body,end} for C++ extern "C" scope and return name/body */
Cblock& mk_extern_c(Cunit& cunit, std::string name);
/** make name/{beg,body,end} for plain if(cond){<body>} conditional block */
Cblock& mk_cpp_if(Cunit& cunit, std::string name, std::string cond);
/** make name/{beg,body,else,end} for plain if(cond){<body>}else{<else>} conditional block */
Cblock& mk_cpp_ifelse(Cunit& cunit, std::string name, std::string cond);
/* make name/{beg,first,body[,last],end} nodes.
 * We return the Cblock "name/body"
 * NEW: the 'cleanup' node is replaced by generic treatment of any sub-block
 *      whose name is "last".
 */
Cblock& mk_scope(Cunit& cunit, std::string name, std::string beg="", std::string end="");

/** Here \c name is for Cblock lookup, \c decl is 'int foo()' [no { or ;]. */
Cblock& mk_func(Cunit& cunit, std::string name, std::string decl);
/** add \prefix indent to all non-whitespace lines, \c sep is a set of line separators.
 * Exception: cpp '#'-lines begin in first col (historical 'C' requirement) */
std::ostream& prefix_lines(std::ostream& os, std::string code,
        std::string prefix, std::string sep=std::string("\n"));

Cblock& operator<<(Cblock& cb, PostIndent const& postIndent);
Cblock& operator<<(Cblock& cb, PreIndent const& preIndent);
//@}

inline Cblock& mk_func(Cunit& cunit, std::string name, std::string decl){
    return mk_scope(cunit, name, decl);
}

inline std::ostream& prefix_lines(std::ostream& os, std::string code,
        std::string prefix, std::string sep /*=std::string("\n")*/ ){
    if( prefix.empty() ){
        os << code;
    }else if( !code.empty()){
        size_t nLoc = 0, nLocEnd;
        while ((nLocEnd = code.find_first_of(sep, nLoc)) != std::string::npos) {
            //std::cout<<" line["<<nLoc<<"..."<<nLocEnd<<"] = <"<<code.substr(nLoc,nLocEnd)<<">\n";
            // line is nLoc..nLocEnd, including the last sep char
            auto const first_nb = code.find_first_not_of(" \r\n\t",nLoc); // first non blank
            if( first_nb < nLocEnd ){                     // if not a blank line
                if( code[first_nb] != '#' ) os << prefix; // never indent cpp directives
                os << code.substr(nLoc,nLocEnd-nLoc) << "\n"; // code string + newline)
            }
            nLoc = nLocEnd+1;
        }
        //std::cout<<" nLoc="<<nLoc<<" code.size()="<<code.size();
        if(nLoc < code.size()){
            //std::cout<<" line["<<nLoc<<"...end] = <"<<code.substr(nLoc)<<">\n";
            // line is nLoc..nLocEnd, including the last sep char
            auto const first_nb = code.find_first_not_of(" \r\n\t",nLoc);
            if( first_nb < nLocEnd ){
                if( code[first_nb] != '#' ) os << prefix;
                os << code.substr(nLoc,nLocEnd-nLoc);
                //os << "\n"; // NO newline
            }
        }
    }
    return os;
}

inline Cblock& operator<<(Cblock& cb, PostIndent const& postIndent){
    //std::cout<<"+PostIndent";
    cb._postmanip = new Cbin(cb, postIndent);
    return cb;
}
inline Cblock& operator<<(Cblock& cb, PreIndent const& preIndent){
    //std::cout<<"+PreIndent";
    cb._premanip = new Cbin(cb, preIndent);
    return cb;
}


}//cprog::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CBLOCK_HPP
