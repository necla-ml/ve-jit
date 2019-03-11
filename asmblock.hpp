#ifndef ASMBLOCK_HPP
#define ASMBLOCK_HPP
#include "asmfmt.hpp"
#include "throw.hpp"
/** \file
 * Arange statements (std::string) into blocks.
 *
 * Problem Description:
 *
 * - JIT units generate vector<string> Asmblock for operations.
 * - When output as a program, or in subsequent optimizations,
 *   Asmblocks may be output:
 *   - sequentially (easy, "linear program")
 *   - interspersed:
 *     - Ex. for loop --> pre-loop, induction, kernel={Asmblock}, test
 *     - Ex. optimization which reorders one statement of next Asmblock
 *           into a Asmblock's vector<string>, to simplify dependencies,
 *           or balance work among execution units.
 *     - Ex. linear combination split up into inductive calculation with
 *           different factors accumulated from different loops.
 *
 * Without worrying about details, we first need some machinery for
 * dealing with creating and stitching together these vector<string>.
 *
 * For creation, we use AsmFmtCols.  These create multiline strings
 * containing C preprocessor and assembly language.  AsmFmtCols formatters
 * get converted to string and added to some vector<string>
 * and annotated with block names, and can be relinked into non-linear output
 * order (DAG) to form a 'C' preprocessor + assembly language program unit
 * [which hopefully compiles].
 *
 * We use AsmFmtCols (and VE extensions) to help do \b SIMPLE register
 * allocations.  We target small assembly kernels, so running out of registers
 * is essentially an error.  To help cut down using different registers for
 * the global constants, AsmFmtCols assumes that same-named variables actually
 * map to the same register, and contain identical content.
 *
 * For more \b COMPLEX .S output, a more complex approach is needed (with some builtin
 * smarts about which node \b set/spills/loads registers.  One register can potential
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

#ifdef NDEBUG
#define ASMBLOCK_DBG(V,N,...) do{}while(0)
#else
#define ASMBLOCK_DBG(V,N,...) do { \
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

namespace asmprog {

struct Asmunit;
struct Asmblock;
struct AsmbmanipBase;

// Sure, indent is more important for Cblock, but let's allow it anyway
// (this may disappear later, or may have default indent of zero)
struct IndentSpec;
struct PostIndent;
struct PreIndent;

// jithex, jitdec are in code genasm.hpp, but want header-only ... XXX FIXME
inline std::string asDec(std::size_t s){
    std::ostringstream oss;
    oss<<s;
    return oss.str();
}

/// \group Asmblock I/O helpers
//@{
/** stateless Asmblock output manipulators for operator<< */
template<class T> inline T& operator<<(T& t, T& (*manip)(T&) ){ return manip(t); }

/** \c Endl to avoid confusion with \c std::endl that applies only to ostream& */
template<class T> inline T& Endl(T& t){ return t<<"\n"; }

/** Base Asmblock/Asmunit manipulator class.
 * Asmunit may store "context", like current indent for \c write operations,
 * so allow access to "/".
 * Possible overkill (maybe Asmblock::getRoot() is all that's needed?)
 * OTOH, maybe this way eases 'friend' decls.
 */
struct AsmbmanipBase {
    Asmblock* ab;
    virtual std::ostream& write(std::ostream& os) {
        os<<" /*AsmbmanipBase*/ "; // debug!
        return os;
    }
    AsmbmanipBase(AsmbmanipBase const& src)        = delete;
    AsmbmanipBase& operator=(AsmbmanipBase const&) = delete;
    virtual ~AsmbmanipBase() {}
    Asmunit* getRoot() const;
  protected:
    friend class Asmblock;
    AsmbmanipBase() : ab(nullptr) {}
    AsmbmanipBase(Asmblock& ab) : ab(&ab) {}
};
inline std::ostream& operator<<(std::ostream& os, AsmbmanipBase & cbmanip){
    return cbmanip.write(os);
}
//@}

//template<class T> struct endl {;}; // Nope. confusion with std::endl
struct Asmblock : AsmFmtVe {
    /// Empty Asmblock constructor (placeholder)
    Asmblock(Asmunit *root, std::string name="root")
        : AsmFmtVe(),
        _root(root), _parent(this), _name(name), _type(""),
        // _parent==this means we are _root
        _premanip(nullptr), _code(""), _sub(), _postmanip(nullptr),
        _nwrites(0), _maxwrites(1)
        {}
    Asmblock(Asmblock *parent, std::string name="")
        : AsmFmtVe(),
        _root(parent->_root), _parent(parent), _name(name), _type(""),
        _premanip(nullptr), _code(""), _sub(), _postmanip(nullptr),
        _nwrites(0), _maxwrites(1)
        {}
    //Asmblock(Asmblock &&s) = default;
    //Asmblock(Asmblock &&s) : _root(s._root), _name(s._name), _type(s._type),
    //                     _code(s._code), _sub(s._sub),
    //                     _nwrites(s._nwrites), _maxwrites(s._maxwrites)
    //{}

    /// \group unformatted code addtion
    /// This is more suited to outputting indent-driven 'C' code,
    /// but let's keep it around, just in case...
    //@{
    /** Find \c p in \c sub -Cblocks, or appending a new Asmblock to \c _sub
     * when \c p is a single <em>path component</em> \c p;
     * O/w if \c p is \b not a <em>path component</em> (i.e. something with '/')
     * then we execute \c at(p), which matches a \em first-found existing
     * block, or throws (\c at never creates a new block).
     * \throw if \c p is a path-string with '/' and \c p is not found.
     * see \c find(path) for a description of how path searches are done.*/
    Asmblock& operator[](std::string p);
    /** shift-left operator appends codeline \e as-is. */
    Asmblock& operator<<(std::string codeline) { return append(codeline); }
    /** prepends a newline, then adds codeline (same precedence as <<). */
    Asmblock& operator>>(std::string codeline) { return append("\n").append(codeline); }
    /** \c codeline append to \c _code (\c Asmblock appends to \c _sub).
     * Mostly append as-is, \em except if last line of code has a ';' in it,
     * we add a newline (tweak for C-code readability). */
    Asmblock& append(std::string codeline);
    //@}

    // appending an Asmblock, however, appends a child block
    /** \c codeline append to \c _code (\c Asmblock appends to \c _sub) */
    Asmblock& append(Asmblock &ab){
        int const v=0;
        ASMBLOCK_DBG(v,3," append! "<<std::endl);
        assert(_parent != nullptr );
        ASMBLOCK_DBG(v,10," this@"<<_parent->_name<<"/"<<_name<<" append");
        ASMBLOCK_DBG(v,10," (ab@"<<ab._name<<")\n");
        ab._parent = this;
        _sub.push_back(&ab);
        ASMBLOCK_DBG(v,10," this@"<<_parent->_name<<"/"<<_name<<"{");
        for(auto s: _sub) std::cout<<" "<<s->_name;
        ASMBLOCK_DBG(v,10,"}"<<std::endl);
#if 0
        return *this;
#else
        return ab; // new behaviour
#endif
    }

    // 'append' adds a next CHILD to this, but we can also write an
    // 'after' that unlinks this and does prev.append(*this),
    // so that this becomes the next child of any other node, prev
    /** unlink \c this and append it to some \c prev Asmblock.
     * equiv prev.append(unlink())
     * I guess this could be called "under".
     * \return \c *this
     */
    Asmblock& after(Asmblock& prev);
    /// place after absolute path (can be wildcarded) \c abspath.
    /// \throw if \c abspath cannot be found.
    Asmblock& after(std::string abspath) { 
        //std::cout<<" after(abspath="<<abspath<<")"; std::cout.flush();
        assert( abspath[0] == '/' );
        return after(at(abspath));
    }

    int nWrites() const {return _nwrites;}

    // perhaps delete these {

    /** Note: write has a strange behaviour of emptying the string.
     * <B>Subject to change</B> \deprecated */
    std::ostream& write(std::ostream& os, bool const chkWrite=true);
    /** Maybe for unrolling we have a max number of writes ? \deprecated */
    bool canWrite() { return _nwrites>=0 && _nwrites<_maxwrites; }

    // perhaps delete these }

    /** depth-first tree dump */
    std::ostream& dump(std::ostream& os, int const ind=0);
    // XXX from base class ?
    /** return string for subtree, unindented, no effect on \c write */
    std::string str();

    Asmblock& setType(std::string type) {_type=type; return *this;}
    Asmblock& setName(std::string type); // {this->type=type; return *this;} and update root!
    std::string const& getName() const;
    Asmunit& getRoot() const;
    //Asmblock& append(std::string code) {this->code += code; return *this;} // maybe inefficient
    //Asmblock* next();
    //Asmblock* prev()
    ~Asmblock(){ clear(); }
    bool isRoot() const { return _parent == this; }
    /// \group path functions
    //@{
    std::string fullpath() const;
    /** find first Asmblock matching \c path, with wildcard extensions.
     * - '/' begin at root
     * - '.' find under \c this Asmblock.
     * - '..' find under \c _parent Asmblock.
     * - '*' match any one subdir (i.e. sub-Asmblock)
     * - '**' match zero or more subdirs
     * - '..*' \em strange recursive upward parent+subtree search excluding sub-tree of \c this
     *   - also called \c up since this is not a common path convention.
     */
    Asmblock *find(std::string path) const;
    /** utility for dot-dot-star wildcard, <em>find-in-upwards-subtrees</em>,
     * a Asmblock match \em close to \c this, but \b not underneath \c this.
     * Note: could have taken ..* to mean "match anywhere on path to root" to root,
     *       but "..\*\/\*\*\/path" doesn't quite exclude the subtree of \c this
     */
    Asmblock *up(std::string path) const { return this->find("..*/"+path); }
    /** \c find but throw if not found */
    Asmblock& at(std::string path) const {
        Asmblock *ret = find(path);
        if( ret == nullptr ) THROW(" Asmblock["<<fullpath()<<"].at("<<path<<") not found");
        return *ret;
    }
    //Asmblock& find(std::string p) const;
  private:
    /** find first \b single-component path \c p for simple search strategy.
     * return nullptr if not found. */
    Asmblock * find_immediate_sub(std::string p) const;
    Asmblock * find_recurse_sub(std::string p) const;
    Asmblock * find_recurse_parent(std::string p) const;
    //@}
  private:
    friend struct Asmunit;
    void clear(){ for(auto s: _sub) delete(s); _sub.clear(); }
    /** Remove \c this from \c _parent.sub[] .
     * \throw if attempting to unlink the root
     * \post \c _parent==nullptr
     */
    Asmblock& unlink();
  private:
    friend class AsmbmanipBase;
    struct Asmunit * const _root;
    struct Asmblock * _parent;
  private:
    std::string _name;
    std::string _type;
    AsmbmanipBase* _premanip;
    std::string _code;
    std::vector<Asmblock*> _sub;
    AsmbmanipBase* _postmanip;
    //Asmblock *_next;
    //Asmblock *_prev;
    int _nwrites;   // counter
    int _maxwrites; // limit for _nwrites
    friend Asmblock& operator<<(Asmblock& ab, PostIndent const& postIndent);
    friend Asmblock& operator<<(Asmblock& ab, PreIndent const& preIndent);
    //friend Asmblock& operator<<(Asmblock& ab, Endl<Asmblock> const&);
    template<class T> friend T& Endl(T& t);
};
template<> inline Asmblock& Endl<Asmblock>(Asmblock& cblock){
    cblock._code.append("\n"); // since frequent, cut out some intermediate functions
    return cblock;
}

struct Asmunit {
    std::string name;       ///< maybe Asmunit subtrees might be copied for unrolling ??
    Asmblock root;
    int v; // verbosity
    std::string indent;                             ///< internal Asmblock::write context
    //std::map<std::string, Asmblock*> blk;
    Asmunit(std::string name) : name(name), root(this,name), v(2), indent() {}
    ~Asmunit() { root.clear(); }
    std::ostream& write(std::ostream& os) {return root.write(os);}  ///< write the program unit
    Asmblock *find(std::string path);                 ///< absolute \c path down from \c root
    Asmblock *find(std::string path, Asmblock* from);   ///< search up \c from, then down from \c root
    std::string str();                              ///< all code of root
    void dump(std::ostream& os);                    ///< dump the tree
    std::string tree();                             ///< tree structure of root
    Asmblock & operator[](std::string name) { return root[name]; }

};
inline void Asmunit::dump(std::ostream& os){
    root.dump(os);
}
inline std::string Asmunit::str(){
    return root.str();
}
inline std::string Asmunit::tree(){
    std::ostringstream oss;
    root.dump(oss);
    oss<<std::endl;
    return oss.str();
}
inline Asmunit* AsmbmanipBase::getRoot() const {return ab->_root;}

struct IndentSpec {
    IndentSpec(int const indent_adjust, char const fill=' ')
        : indent_adjust(indent_adjust), fill(fill) {}
    int const indent_adjust;
    char const fill;
};

struct Cbin : public AsmbmanipBase, public IndentSpec {
    Cbin(Asmblock& ab, int const adj, char const fill=' ')
        : AsmbmanipBase(ab), IndentSpec(adj,fill) {}
    Cbin(Asmblock& ab, IndentSpec const& indentSpec)
        : AsmbmanipBase(ab), IndentSpec(indentSpec) {}
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

inline std::string const& Asmblock::getName() const {return _name;}

inline Asmunit& Asmblock::getRoot() const {return *_root;}

inline Asmblock& Asmblock::after(Asmblock& prev) {
#if 0 // original
    std::cout<<" Asmblock@"<<_name; std::cout.flush();
    //std::cout<<" Asmblock@"<<fullpath(); std::cout.flush(); // XXX fullpath error if Asmblock unlinked?
    std::cout<<" after("<<prev.fullpath()<<" unlink..."; std::cout.flush();
    Asmblock& tmp = unlink();
    std::cout<<" append..."; std::cout.flush();
    assert( &tmp == this );
    prev.append(tmp);
    std::cout<<" done"<<std::endl; std::cout.flush();
    return *this;
#else // streamlined, with 'append' that returns the argument, instead of 'prev'
    ASMBLOCK_DBG(_root->v,2," Asmblock["<<fullpath()<<"].after("<<prev.fullpath()<<")\n");
    return prev.append(unlink());
#endif
}

Asmblock& Asmblock::append(std::string codeline){
    if( !codeline.empty() ){
#if 0 // trial...
        // to technically allow building up a single statement, we only
        // stick in new line if the last line of _code contains a ';'
        if( !_code.empty() ){
            size_t lastline=_code.find_last_of('\n');
            if( lastline == std::string::npos ) lastline=0; else ++lastline;
            if(_code.find_first_of(";",lastline) != std::string::npos)
                _code.append("\n");
        }
#endif
        _code.append(codeline);
    }
    return *this;
}
inline std::string Asmblock::fullpath() const {
    int const v=0;
    ASMBLOCK_DBG(v,1," fullpath!"<<std::endl;);
    std::string out;
    out.reserve(256);
    int ncomp=0; // number of path components
    for(Asmblock const * ab=this; ab!=nullptr; ab  = ab->_parent){
        ASMBLOCK_DBG(v,1," fp:"<<ab->_name<<" parent:"<<(ab->_parent? ab->_parent->_name: "NULL"));
        if( ab->isRoot() ){
            ASMBLOCK_DBG(v,1," isRoot");
            out.insert(0,"/");
            break;
        }else{
            ASMBLOCK_DBG(v,1," notRoot");
            ASMBLOCK_DBG(v,1," _name["<<_name<<"]"<<std::endl);
            size_t const cbsz = ab->_name.size();
            out.insert(0,ab->_name.c_str(),cbsz+1); // include terminal null
            out.replace(cbsz,1,1,'/');
            ASMBLOCK_DBG(v,1," _name["<<cbsz<<"] --> out="<<out);
        }
        ASMBLOCK_DBG(v,1,std::endl);
        ++ncomp;
    }
    ASMBLOCK_DBG(v,1," fullpath DONE, out="<<out<<" ncomp = "<<ncomp<<std::endl);
    if(ncomp>0) out.resize(out.size()-1U);
    ASMBLOCK_DBG(v,1," fullpath DONE, out="<<out<<std::endl);
    return out;
}
inline Asmblock& Asmblock::unlink() {
    int const v=0;
    if(this == &(_root->root))
        THROW("unlink of "<<fullpath()<<" failed, is it root?");
    if(_parent != this && _parent != nullptr){
        //ASMBLOCK_DBG(v,1," unlink cblock "<<(void*)this<<" _parent "<<(void*)_parent<<std::endl);
        ASMBLOCK_DBG(v,1," unlink cblock "<<fullpath()<<std::endl);
        for(auto s=_parent->_sub.begin(); s!=_parent->_sub.end(); ++s){
            ASMBLOCK_DBG(v,2," unlink psub "<<(*s)->_name<<"? ");
            if(*s == this){
                ASMBLOCK_DBG(v,2," YES!");
                _parent->_sub.erase(s); // all _sub iters (including s) INVALID
                _parent = nullptr;
                ASMBLOCK_DBG(v,2," _name unlinked"<<std::endl);
                break;
            }
        }
    }
    return *this;
}

inline Asmblock& operator<<(Asmblock& ab, PostIndent const& postIndent){
    std::cout<<"+PostIndent";
    ab._postmanip = new Cbin(ab, postIndent);
    return ab;
}
inline Asmblock& operator<<(Asmblock& ab, PreIndent const& preIndent){
    std::cout<<"+PreIndent";
    ab._premanip = new Cbin(ab, preIndent);
    return ab;
}
/** find/create subblock.
 * If p is a component (not a path), then
 *   \return subblock with name \c p (create subblock if nec, no throw)
 * Otherwise throw if \c p.empty() or use \c at(p) fails to find a path.
 * */
inline Asmblock& Asmblock::operator[](std::string p){
    if(p.empty()) THROW("Asmblock[""] oops");
    // Be careful to not create sub-block with names containing wildcard strings
    if(p.find("/") != std::string::npos){       // PATH! revert to full-path search
        return this->at(p);                     //       throw if not found
    }
    //                                          Possible single-component path!
    if( p.substr(0,1) == "."){ // . and .. might actual be OK paths
        if( p == "." || p == "..")
            return this->at(p); //usually such blocks already exist, so we can return them
        else
            THROW("Avoid sub-block names like "<<p<<" beginning with '.'");
    }
    if( p.substr(0,1) == "*" ) // '*' should never begin a single component name
        THROW("'*' wildcard needs a '/'?  Avoid sub-blocks names like "<<p<<" beginning with '*'");
    // single-component path, possible valid name,
    for(auto s: _sub) if(s && s->_name==p) return *s;   // found ?
    //                                                  CREATE if not found in _sub[]
    ASMBLOCK_DBG(_root->v,2,"// new sub-block "<<_name<<"/"<<p<<" "<<_name<<".sub.size()="<<_sub.size()<<"\n");
    _sub.push_back(new Asmblock(this,p));
    return *_sub.back();
}

inline Asmblock * Asmblock::find_immediate_sub(std::string p) const {
    assert( p.find("/") == std::string::npos );
    if(p.empty()) return nullptr;
    for(Asmblock const* s: _sub) if(s && s->_name==p) return const_cast<Asmblock*>(s);
    return nullptr;
}
inline Asmblock * Asmblock::find_recurse_sub(std::string p) const {
    assert( p.find("/") == std::string::npos );
    if(p.empty()) return nullptr;
    for(Asmblock const* s: _sub){                          // s is Asmblock const*
        if(s && s->_name==p) return const_cast<Asmblock*>(s);
        Asmblock *submatch = s->find_recurse_sub(p);
        if(submatch)                            // find first match?
            return const_cast<Asmblock*>(submatch);
    }
    return nullptr;
}
/** search \em siblings, i.e. parent Asmblock and its subblocks that are not \c this */
inline Asmblock * Asmblock::find_recurse_parent(std::string p) const {
    assert( p.find("/") == std::string::npos );
    //if(isRoot()) return *this;
    if(p.empty()) return nullptr;
    if(_parent==nullptr) return nullptr;
    if(_parent->_name == p) return _parent;
    for(Asmblock const* s: _parent->_sub){
        if(s == this) continue;        // skip our subtree (maybe we already looked there)
        if(s && s->_name==p) return const_cast<Asmblock*>(s);
        if(s->find_recurse_sub(p) != s) return const_cast<Asmblock*>(s);
    }
    return nullptr;
}

/// \group Asmblock helpers
//@{ helpers
/** provide an \c unlink'ed extern "C" block.
 * - You \b should \c .after(abspath) this to the desired location.
 *   - If you don't, it ends up at \c after("/"), perhaps \em not where you want it.
 * - User is expected to \c append or \c operator&lt;&lt; to \em \/\*\*\/name/body
 *   to get expected behaviour.
 * \return Cblk [name] with \c _sub blocks name/beg, name/body, name_end.
 */
inline Asmblock& mk_extern_c(Asmunit& cunit, std::string name){
#if 0
    Asmblock& block = *(new Asmblock(&cunit,name));
    block["beg"]<<"\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif //C++\n";
    block["body"]; // empty
    block["end"]<<"\n"
        "#ifdef __cplusplus\n"
        "}//extern \"C\"\n"
        "#endif //C++\n";
    // Signal that we are not accessible within the DAG of cunit yet.
    // We need an 'after' to postion us within an existing 'root'.
    // We want to signal attempts to use unrooted Cblocks in questionable manner.
    //return block.unrooted();
    //
    // OR, explicitly root it at top-level
    cunit.root.append(block);
    return block;
#else // OK, let's just root it right away
    // want to append a new root block (even if name is dup)
    // This is not so bad, because client can take ref to "this one",
    // or can immediately '.after' it to another tree position.
    Asmblock& block = cunit.root.append(*(new Asmblock(&cunit,name)));
    block["beg"]<<"\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif //C++\n";
    block["body"]; // empty
    block["end"]<<"\n"
        "#ifdef __cplusplus\n"
        "}//extern \"C\"\n"
        "#endif //C++\n";
    return block;
#endif
}
/** [beg]:"\#if cond" + [body] + [end]:"\#endif // cond".
 * This has a "body" sub-block, like a scope, but no extra indenting.
 * No support for \em removing the indent so '#' lines end up in col 1.
 * (Could special-case this within "write" of the _code lines, I guess)
 */
inline Asmblock& mk_cpp_if(Asmunit& cunit, std::string name, std::string cond){
    Asmblock& block = *(new Asmblock(&cunit,name));
    block["beg"]<<"\n#if "<<cond;
    block["body"]; // empty
    block["end"]<<"#endif // "<<cond;
    return block;
}
/** create a "beg{..}" block with subblock named "body" properly indented */
inline Asmblock& mk_scope(Asmunit& cunit, std::string name, std::string beg, std::string end="}"){
    Asmblock& block = *(new Asmblock(&cunit,name));
    block["beg"]<<beg<<" { // "<<name<<PostIndent(+2);
    block["body"]; // empty
    block["end"]<<"}//"<<name<<PreIndent(-2);
    return block;
}
#if 0 // this is just an instance of mk_scope
/** \c name is for Asmblock lookup, \c decl is 'int foo()' [no { or ;]. */
inline Asmblock& mk_func(Asmunit& cunit, std::string name, std::string decl){
    Asmblock& block = *(new Asmblock(&cunit,name));
    block["decl"]<<"\n"<<decl<<" {"<<PostIndent(+2); // name path code always gets a newline
    block["body"]; // empty
    block["end"]<<"}"<<PreIndent(-2);
    // because the tmp Asmblock _parent is itself, its fullpath is just "/".
    // It is NOT a member of Asmunit::root.
    //int const v=1;
    //ASMBLOCK_DBG(v,1," mk_func-->Asmblock["<<block.getName()<<"] "<<block.fullpath()<<" @ "<<(void*)&block);

    // It would not hurt to force to be, I suppose,
    // It would be safer, and never leak.
    // But would be more ops if standard usage is to root it elsewhere via .after(locn)
    //cunit.root.append(block); // <-- OPTIONAL, undecided
    return block;
}
#else
inline Asmblock& mk_func(Asmunit& cunit, std::string name, std::string decl){
    return mk_scope(cunit, name, decl);
}
#endif
/** for simple scopes (terminate with just "}", and with a "body" sub-block...
 * - if AFTER is a cblock, we could just use AFTER.getRoot() and save an argument
 * - general case is a bit more flexible, but macro is fairly readable if combined with indenting
 */
#define CBLOCK_SCOPE(CBLK_VAR,BEG,CUNIT,AFTER) auto& CBLK_VAR = mk_scope((CUNIT),#CBLK_VAR,(BEG)).after(AFTER)["body"];
//@} helpers

/** relative find of \em first Asmblock matching 'p'.
 * - Suppose STAR = "*", DOUBLESTAR = "**"
 * 0. \c p with .. to mean "parent-of"
 * 1. \c p is a fullpath of form "/rootname/sub1/sub2/.../subN"
 * 2. \c p has single-path  wildcard "/rootname/STAR/foo"
 * 3. \c p with one-or-more wildcard "/DOUBLESTAR/foo"
 * 4. \c p relative path, immediate _sub[] match "./foo"
 * 5. \c p relative sub-path "./[DOUBLE]STAR/foo"
 * 6. \c p relative search sub-path:
 *    a. downwards (sub-path)
 *    b. up 1, then all sub-paths (repeat until full search is done
 * find NEVER creates a block.
 */
inline Asmblock* Asmblock::find(std::string p) const {
    assert(_root != nullptr);
    int const v = _root->v;
    ASMBLOCK_DBG(v,3,std::string(8,'=')<<" Asmblock "<<fullpath()<<" find(\""<<p<<"\")");
    if(p.empty()){
        ASMBLOCK_DBG(v,3," empty => not found\n");
        return nullptr;
    }
    auto const firstslash = p.find("/");
    if(firstslash == std::string::npos){
        ASMBLOCK_DBG(v,3," no firstslash => find_immediate_sub\n");
        if(p==_name || p==".") return const_cast<Asmblock*>(this);
        if(p==".."){
            if(_parent) return _parent;
            else return nullptr;
        }
        return find_immediate_sub(p);
    }
    std::string remain;
    auto const nextnonslash = p.find_first_not_of("/",firstslash+1);
    if( nextnonslash != std::string::npos ){
        remain = p.substr(nextnonslash);
    }
    if(firstslash == 0){ // shunt the search immediately to root.
        ASMBLOCK_DBG(v,3," starts-with-slash");
        // all "/<remain>" cases
        Asmblock const& root = _root->root;
        if( remain.empty() ){
            ASMBLOCK_DBG(v,3," remain.empty(), return root");
            return &_root->root;
        }
        if( root._name == p ){
            ASMBLOCK_DBG(v,3," matches root name\n");
            return &_root->root;
        }
        ASMBLOCK_DBG(v,3," root.find(\""<<remain<<"\")\n");
        return root.find(remain);
    }
    auto comp1 = p.substr(0,firstslash);
    if(remain.empty()){ // terminal [/]\+ not significant
        return this->find(comp1);
    }
    ASMBLOCK_DBG(v,3,"find@<"<<comp1<<">/<"<<remain<<">\n");
    if( comp1 == "." ){                               // "./remain"
        ASMBLOCK_DBG(v,3," ./<remain>\n");
        return this->find(remain);
    }else if( comp1 == ".." ){                        // "../remain"
        if(_parent /*&& _parent != this*/){ // "/.." is same as root (like FS)
            ASMBLOCK_DBG(v,3," ../<remain>\n");
            return _parent->find(remain);
        }else{
            ASMBLOCK_DBG(v,3," .. no parent\n");
            return nullptr;
        }
    }else if(comp1 == "*" || comp1 == "**"){          // "*/remain"
        if( comp1 == "**" ){ // ** is allowed to match with this (no subdirs)
            Asmblock * thismatch = this->find(remain);
            if( thismatch ){
                ASMBLOCK_DBG(v,3," ** no-subdir match\n");
                return thismatch;
            }
        }
        for(Asmblock const* s: _sub){
            Asmblock* subfind = s->find(remain);
            if(subfind){                // found 'remain' in s
                ASMBLOCK_DBG(v,3," * subfind \n");
                return subfind;
            }
            if(comp1 == "**"){
                ASMBLOCK_DBG(v,3,"\n** subfind ");
                Asmblock* deeper = s->find(p); // repeat "**/remain" search, depthwise
                if(deeper){
                    ASMBLOCK_DBG(v,3," ** deeper match\n");
                    return deeper;
                }
            }
        }
        return nullptr; // */remain not found
    }else if(comp1 == "..*"){
        // This is recursive _parent search, never looking underneath this
        if( !_parent || _parent==this || isRoot() ){
            ASMBLOCK_DBG(v,1," no parent for Asmblock "<<_name<<"\n");
            THROW("Not possible to continue upward parent search");
        }
        if( _parent->_name == remain ){
            ASMBLOCK_DBG(v,1," FOUND exact match of parent "<<_parent->_name<<" with remain\n");
            return _parent;
        }
        Asmblock * parentfind = _parent->find(remain);
        if(parentfind){
            ASMBLOCK_DBG(v,1," FOUND match of parent "<<_parent->_name<<" with remain at "<<parentfind->fullpath()<<"\n");
            return parentfind;
        }
        ASMBLOCK_DBG(v,1," search parent subtree\n");
        for(Asmblock const* s: _parent->_sub){
            if( s == this ) continue; // search parent sub-tree EXCEPT for this
            ASMBLOCK_DBG(v,1,"\nssss sibling-find **/"<<remain<<" under sibling "<<s->fullpath()<<"\n");
            Asmblock* sibfind = s->find("**/"+remain);   // force sub-tree search
            if(sibfind){
                ASMBLOCK_DBG(v,1,"\nssss found "<<remain<<" at "<<sibfind->fullpath()<<"\n");
                return sibfind;
            }
            ASMBLOCK_DBG(v,1,"\nssss did not find "<<remain<<"\n");
        }
        // repeat the SAME p="**/remain" search skipping the parent's sub-tree.
        return _parent->find(p);
    }else{                                            // "comp1/remain"
        for(Asmblock const* s: _sub){
            if(s->_name == comp1){
                ASMBLOCK_DBG(v,3," sub");
                return s->find(remain);
            }
        }
        ASMBLOCK_DBG(v,3," no match for comp1=<"<<comp1<<">\n");
        return nullptr;
#if 0
        Asmblock const* sub = find_immediate_sub(comp1);
        if(sub == this) return this; // comp1 not matched
        std::cout<<" sub["<<sub._name<<"].find(\""<<remain<<"\")"<<std::endl;
        if(remain.empty()) return sub; // comp1 was the last path component
        auto subfind = sub.find(remain);
        if( &subfind == &sub ) return this; // remain did not match
        return subfind;                     // return subfind Asmblock
#endif
    }
}

/** add \prefix indent to all non-whitespace lines, \c sep is a set of line separators.
 * Exception: cpp '#'-lines begin in first col (historical 'C' requirement) */
inline std::ostream& prefix_lines(std::ostream& os, std::string code,
        std::string prefix, std::string sep=std::string("\n")){
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
/** debug printout: see the DAG, not the actual code snippets */
inline std::ostream& Asmblock::dump(std::ostream& os, int const ind/*=0*/)
{
    if(ind > 2000){
        THROW("Asmblock dump depth really huge.  Maybe it is not a DAG.");
    }
    //int const v = _root->v+2;
    std::string in("\n&&& "+std::string(ind,' '));
    os<<in<<fullpath()
        <<(_premanip? " premanip": "")
        <<" code["<<_code.size()<<"]"
        <<" sub["<<_sub.size()<<"]"
        <<(_postmanip? "postmanip": "")
        ;
    for(auto s: _sub) s->dump(os,ind+1); // it's easy to generate **very** deep trees
    return os;
}
inline std::ostream& Asmblock::write(std::ostream& os, bool chkWrite)
{
    if(chkWrite && !canWrite()){
        if(_root->v >= 1){
            std::cout<<" SKIP-WRITE! "; std::cout.flush();
        }
        return os;
    }
    std::string& in = _root->indent;
    // very-verbose mode blocks commented with fullpath
    if(_root->v >= 2 || _code.size()==0){
        if(_root->v >= 2 && _code.size()) os<<in<<"//\n";
        if(_root->v >= 1){
            os<<in<<"// Asmblock : "<<this->fullpath()<<" : "<<_type;
            if(_code.empty()) os<<" (empty)";
            os<<"\n";
        }
    }
    if(_premanip) os << *_premanip;
    if(!_code.empty()){
        //std::cout<<" prefix_lines with code=<"<<_code<<">\n";
        prefix_lines(os,_code,in) << "\n";
    }
    if(_root->v >= 3 ) os<<"// _sub.size() = "<<_sub.size()<<"\n";
    for(auto s: _sub){
        if(_root->v >= 3 ) os<<in<<"// ........ sub "<<_parent->_name<<"/"<<_name<<"/"<<s->_name<<std::endl;
        s->write(os,chkWrite);
    }
    if(_postmanip) os << *_postmanip;
    // if( _next ) _next->write(os);
    if(chkWrite) ++_nwrites;
    return os;
}
inline std::string Asmblock::str(){
    std::ostringstream oss;
    _root->indent.clear();
    this->write( oss, false/*chkWrite*/ );
    return oss.str();
}

}//cunit::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // ASMBLOCK_HPP
