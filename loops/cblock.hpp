#ifndef CBLOCK_HPP
#define CBLOCK_HPP
#include "../regs/throw.hpp"
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
 **/
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <cassert>

#ifdef NDEBUG
#define CBLOCK_DBG(V,N) do{}while(0)
#else
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
struct Cblock;
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
 * Cunit may store "context", like current indent for \c write operations,
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
struct Cblock {
    /// Empty Cblock constructor (placeholder)
    Cblock(Cunit *root, std::string name="root")
        : _root(root), _parent(this), _name(name), _type(""),
        // _parent==this means we are _root
        _premanip(nullptr), _code(""), _sub(), _postmanip(nullptr),
        _nwrites(0), _maxwrites(1)
        {}
    Cblock(Cblock *parent, std::string name="")
        : _root(parent->_root), _parent(parent), _name(name), _type(""),
        _premanip(nullptr), _code(""), _sub(), _postmanip(nullptr),
        _nwrites(0), _maxwrites(1)
        {}
    //Cblock(Cblock &&s) = default;
    //Cblock(Cblock &&s) : _root(s._root), _name(s._name), _type(s._type),
    //                     _code(s._code), _sub(s._sub),
    //                     _nwrites(s._nwrites), _maxwrites(s._maxwrites)
    //{}

    /** Find \c p in \c sub -Cblocks, or appending a new Cblock to \c _sub
     * when \c p is a single <em>path component</em> \c p;
     * O/w if \c p is \b not a <em>path component</em> (i.e. something with '/')
     * then we execute \c at(p), which matches a \em first-found existing
     * block, or throws (\c at never creates a new block).
     * \throw if \c p is a path-string with '/' and \c p is not found.
     * see \c find(path) for a description of how path searches are done.*/
    Cblock& operator[](std::string p);
    Cblock& operator<<(std::string codeline) { return append(codeline); }
    /** \c codeline append to \c _code (\c Cblock appends to \c _sub).
     * Mostly append as-is, \em except if last line of code has a ';' in it,
     * we add a newline (tweak for C-code readability). */
    Cblock& append(std::string codeline);
    /** \c codeline append to \c _code (\c Cblock appends to \c _sub) */
    Cblock& append(Cblock &cb){
        int const v=0;
        CBLOCK_DBG(v,3," append! "<<std::endl);
        assert(_parent != nullptr );
        CBLOCK_DBG(v,10," this@"<<_parent->_name<<"/"<<_name<<" append");
        CBLOCK_DBG(v,10," (cb@"<<cb._name<<")\n");
        cb._parent = this;
        _sub.push_back(&cb);
        CBLOCK_DBG(v,10," this@"<<_parent->_name<<"/"<<_name<<"{");
        for(auto s: _sub) std::cout<<" "<<s->_name;
        CBLOCK_DBG(v,10,"}"<<std::endl);
#if 0
        return *this;
#else
        return cb; // new behaviour
#endif
    }
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

    int nWrites() const {return _nwrites;}
    bool canWrite() { return _nwrites>=0 && _nwrites<_maxwrites; }
    std::ostream& write(std::ostream& os);
    /** depth-first tree dump */
    std::ostream& dump(std::ostream& os, int const ind=0);

    Cblock& setType(std::string type) {_type=type; return *this;}
    Cblock& setName(std::string type); // {this->type=type; return *this;} and update root!
    std::string const& getName() const;
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
  private:
    /** find first \b single-component path \c p for simple search strategy.
     * return nullptr if not found. */
    Cblock * find_immediate_sub(std::string p) const;
    Cblock * find_recurse_sub(std::string p) const;
    Cblock * find_recurse_parent(std::string p) const;
    //@}
  private:
    friend struct Cunit;
    void clear(){ for(auto s: _sub) delete(s); _sub.clear(); }
    /** Remove \c this from \c _parent.sub[] .
     * \throw if attempting to unlink the root
     * \post \c _parent==nullptr
     */
    Cblock& unlink();
  private:
    friend class CbmanipBase;
    struct Cunit * const _root;
    struct Cblock * _parent;
  private:
    std::string _name;
    std::string _type;
    CbmanipBase* _premanip;
    std::string _code;
    std::vector<Cblock*> _sub;
    CbmanipBase* _postmanip;
    //Cblock *_next;
    //Cblock *_prev;
    int _nwrites;   // counter
    int _maxwrites; // limit for _nwrites
    friend Cblock& operator<<(Cblock& cb, PostIndent const& postIndent);
    friend Cblock& operator<<(Cblock& cb, PreIndent const& preIndent);
    //friend Cblock& operator<<(Cblock& cb, Endl<Cblock> const&);
    template<class T> friend T& Endl(T& t);
};
template<> Cblock& Endl<Cblock>(Cblock& cblock){
    cblock._code.append("\n"); // since frequent, cut out some intermediate functions
    return cblock;
}

struct Cunit {
    std::string name;
    Cblock root;
    int v; // verbosity
    std::string indent;
    //std::map<std::string, Cblock*> blk;
    Cunit(std::string name) : name(name), root(this,name), v(2), indent() {}
    ~Cunit() { root.clear(); }
    std::ostream& write(std::ostream& os) {return root.write(os);}  ///< write the program unit
    Cblock *find(std::string path);                 ///< absolute \c path down from \c root
    Cblock *find(std::string path, Cblock* from);   ///< search up \c from, then down from \c root
    void dump(std::ostream& os);                    ///< dump the tree
    Cblock & operator[](std::string name) { return root[name]; }

};

inline Cunit* CbmanipBase::getRoot() const {return cb->_root;}

struct IndentSpec {
    IndentSpec(int const indent_adjust, char const fill=' ')
        : indent_adjust(indent_adjust), fill(fill) {}
    int const indent_adjust;
    char const fill;
};

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

inline Cunit& Cblock::getRoot() const {return *_root;}

inline Cblock& Cblock::after(Cblock& prev) {
#if 0 // original
    std::cout<<" Cblock@"<<_name; std::cout.flush();
    //std::cout<<" Cblock@"<<fullpath(); std::cout.flush(); // XXX fullpath error if Cblock unlinked?
    std::cout<<" after("<<prev.fullpath()<<" unlink..."; std::cout.flush();
    Cblock& tmp = unlink();
    std::cout<<" append..."; std::cout.flush();
    assert( &tmp == this );
    prev.append(tmp);
    std::cout<<" done"<<std::endl; std::cout.flush();
    return *this;
#else // streamlined, with 'append' that returns the argument, instead of 'prev'
    CBLOCK_DBG(_root->v,2," Cblock["<<fullpath()<<"].after("<<prev.fullpath()<<")\n");
    return prev.append(unlink());
#endif
}

Cblock& Cblock::append(std::string codeline){
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
inline std::string Cblock::fullpath() const {
    int const v=0;
    CBLOCK_DBG(v,1," fullpath!"<<std::endl;);
    std::string out;
    out.reserve(256);
    int ncomp=0; // number of path components
    for(Cblock const * cb=this; cb!=nullptr; cb  = cb->_parent){
        CBLOCK_DBG(v,1," fp:"<<cb->_name<<" parent:"<<(cb->_parent? cb->_parent->_name: "NULL"));
        if( cb->isRoot() ){
            CBLOCK_DBG(v,1," isRoot");
            out.insert(0,"/");
            break;
        }else{
            CBLOCK_DBG(v,1," notRoot");
            CBLOCK_DBG(v,1," _name["<<_name<<"]"<<std::endl);
            size_t const cbsz = cb->_name.size();
            out.insert(0,cb->_name.c_str(),cbsz+1); // include terminal null
            out.replace(cbsz,1,1,'/');
            CBLOCK_DBG(v,1," _name["<<cbsz<<"] --> out="<<out);
        }
        CBLOCK_DBG(v,1,std::endl);
        ++ncomp;
    }
    CBLOCK_DBG(v,1," fullpath DONE, out="<<out<<" ncomp = "<<ncomp<<std::endl);
    if(ncomp>0) out.resize(out.size()-1U);
    CBLOCK_DBG(v,1," fullpath DONE, out="<<out<<std::endl);
    return out;
}
inline Cblock& Cblock::unlink() {
    int const v=0;
    if(this == &(_root->root))
        THROW("unlink of "<<fullpath()<<" failed, is it root?");
    if(_parent != this && _parent != nullptr){
        //CBLOCK_DBG(v,1," unlink cblock "<<(void*)this<<" _parent "<<(void*)_parent<<std::endl);
        CBLOCK_DBG(v,1," unlink cblock "<<fullpath()<<std::endl);
        for(auto s=_parent->_sub.begin(); s!=_parent->_sub.end(); ++s){
            CBLOCK_DBG(v,2," unlink psub "<<(*s)->_name<<"? ");
            if(*s == this){
                CBLOCK_DBG(v,2," YES!");
                _parent->_sub.erase(s); // all _sub iters (including s) INVALID
                _parent = nullptr;
                CBLOCK_DBG(v,2," _name unlinked"<<std::endl);
                break;
            }
        }
    }
    return *this;
}

inline Cblock& operator<<(Cblock& cb, PostIndent const& postIndent){
    std::cout<<"+PostIndent";
    cb._postmanip = new Cbin(cb, postIndent);
    return cb;
}
inline Cblock& operator<<(Cblock& cb, PreIndent const& preIndent){
    std::cout<<"+PreIndent";
    cb._premanip = new Cbin(cb, preIndent);
    return cb;
}
/** find/create subblock.
 * If p is a component (not a path), then
 *   \return subblock with name \c p (create subblock if nec, no throw)
 * Otherwise throw if \c p.empty() or use \c at(p) fails to find a path.
 * */
inline Cblock& Cblock::operator[](std::string p){
    if(p.empty()) THROW("Cblock[""] oops");
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
    CBLOCK_DBG(_root->v,2,"// new sub-block "<<_name<<"/"<<p<<" "<<_name<<".sub.size()="<<_sub.size()<<"\n");
    _sub.push_back(new Cblock(this,p));
    return *_sub.back();
}

inline Cblock * Cblock::find_immediate_sub(std::string p) const {
    assert( p.find("/") == std::string::npos );
    if(p.empty()) return nullptr;
    for(Cblock const* s: _sub) if(s && s->_name==p) return const_cast<Cblock*>(s);
    return nullptr;
}
inline Cblock * Cblock::find_recurse_sub(std::string p) const {
    assert( p.find("/") == std::string::npos );
    if(p.empty()) return nullptr;
    for(Cblock const* s: _sub){                          // s is Cblock const*
        if(s && s->_name==p) return const_cast<Cblock*>(s);
        Cblock *submatch = s->find_recurse_sub(p);
        if(submatch)                            // find first match?
            return const_cast<Cblock*>(submatch);
    }
    return nullptr;
}
/** search \em siblings, i.e. parent Cblock and its subblocks that are not \c this */
inline Cblock * Cblock::find_recurse_parent(std::string p) const {
    assert( p.find("/") == std::string::npos );
    //if(isRoot()) return *this;
    if(p.empty()) return nullptr;
    if(_parent==nullptr) return nullptr;
    if(_parent->_name == p) return _parent;
    for(Cblock const* s: _parent->_sub){
        if(s == this) continue;        // skip our subtree (maybe we already looked there)
        if(s && s->_name==p) return const_cast<Cblock*>(s);
        if(s->find_recurse_sub(p) != s) return const_cast<Cblock*>(s);
    }
    return nullptr;
}

/// \group Cblock helpers
//@{ helpers
/** provide an \c unlink'ed extern "C" block.
 * - You \b should \c .after(abspath) this to the desired location.
 *   - If you don't, it ends up at \c after("/"), perhaps \em not where you want it.
 * - User is expected to \c append or \c operator&lt;&lt; to \em \/\*\*\/name/body
 *   to get expected behaviour.
 * \return Cblk [name] with \c _sub blocks name/beg, name/body, name_end.
 */
inline Cblock& mk_extern_c(Cunit& cunit, std::string name){
#if 0
    Cblock& block = *(new Cblock(&cunit,name));
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
    Cblock& block = cunit.root.append(*(new Cblock(&cunit,name)));
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
inline Cblock& mk_cpp_if(Cunit& cunit, std::string name, std::string cond){
    Cblock& block = *(new Cblock(&cunit,name));
    block["beg"]<<"\n#if "<<cond;
    block["body"]; // empty
    block["end"]<<"#endif // "<<cond;
    return block;
}
/** create a "beg{..}" block with subblock named "body" properly indented */
inline Cblock& mk_scope(Cunit& cunit, std::string name, std::string beg, std::string end="}"){
    Cblock& block = *(new Cblock(&cunit,name));
    block["beg"]<<beg<<" { // "<<name<<PostIndent(+2);
    block["body"]; // empty
    block["end"]<<"}"<<PreIndent(-2);
    return block;
}
#if 0 // this is just an instance of mk_scope
/** \c name is for Cblock lookup, \c decl is 'int foo()' [no { or ;]. */
inline Cblock& mk_func(Cunit& cunit, std::string name, std::string decl){
    Cblock& block = *(new Cblock(&cunit,name));
    block["decl"]<<"\n"<<decl<<" {"<<PostIndent(+2); // name path code always gets a newline
    block["body"]; // empty
    block["end"]<<"}"<<PreIndent(-2);
    // because the tmp Cblock _parent is itself, its fullpath is just "/".
    // It is NOT a member of Cunit::root.
    //int const v=1;
    //CBLOCK_DBG(v,1," mk_func-->Cblock["<<block.getName()<<"] "<<block.fullpath()<<" @ "<<(void*)&block);

    // It would not hurt to force to be, I suppose,
    // It would be safer, and never leak.
    // But would be more ops if standard usage is to root it elsewhere via .after(locn)
    //cunit.root.append(block); // <-- OPTIONAL, undecided
    return block;
}
#else
inline Cblock& mk_func(Cunit& cunit, std::string name, std::string decl){
    return mk_scope(cunit, name, decl);
}
#endif
/** for simple scopes (terminate with just "}", and with a "body" sub-block...
 * - if AFTER is a cblock, we could just use AFTER.getRoot() and save an argument
 * - general case is a bit more flexible, but macro is fairly readable if combined with indenting
 */
#define CBLOCK_SCOPE(CBLK_VAR,BEG,CUNIT,AFTER) auto& CBLK_VAR = mk_scope((CUNIT),#CBLK_VAR,(BEG)).after(AFTER)["body"];
//@} helpers

/** relative find of \em first Cblock matching 'p'.
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
inline Cblock* Cblock::find(std::string p) const {
    assert(_root != nullptr);
    int const v = _root->v;
    CBLOCK_DBG(v,3,std::string(8,'=')<<" Cblock "<<fullpath()<<" find(\""<<p<<"\")");
    if(p.empty()){
        CBLOCK_DBG(v,3," empty => not found\n");
        return nullptr;
    }
    auto const firstslash = p.find("/");
    if(firstslash == std::string::npos){
        CBLOCK_DBG(v,3," no firstslash => find_immediate_sub\n");
        if(p==_name || p==".") return const_cast<Cblock*>(this);
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
        CBLOCK_DBG(v,3," starts-with-slash");
        // all "/<remain>" cases
        Cblock const& root = _root->root;
        if( remain.empty() ){
            CBLOCK_DBG(v,3," remain.empty(), return root");
            return &_root->root;
        }
        if( root._name == p ){
            CBLOCK_DBG(v,3," matches root name\n");
            return &_root->root;
        }
        CBLOCK_DBG(v,3," root.find(\""<<remain<<"\")\n");
        return root.find(remain);
    }
    auto comp1 = p.substr(0,firstslash);
    if(remain.empty()){ // terminal [/]\+ not significant
        return this->find(comp1);
    }
    CBLOCK_DBG(v,3,"find@<"<<comp1<<">/<"<<remain<<">\n");
    if( comp1 == "." ){                               // "./remain"
        CBLOCK_DBG(v,3," ./<remain>\n");
        return this->find(remain);
    }else if( comp1 == ".." ){                        // "../remain"
        if(_parent /*&& _parent != this*/){ // "/.." is same as root (like FS)
            CBLOCK_DBG(v,3," ../<remain>\n");
            return _parent->find(remain);
        }else{
            CBLOCK_DBG(v,3," .. no parent\n");
            return nullptr;
        }
    }else if(comp1 == "*" || comp1 == "**"){          // "*/remain"
        if( comp1 == "**" ){ // ** is allowed to match with this (no subdirs)
            Cblock * thismatch = this->find(remain);
            if( thismatch ){
                CBLOCK_DBG(v,3," ** no-subdir match\n");
                return thismatch;
            }
        }
        for(Cblock const* s: _sub){
            Cblock* subfind = s->find(remain);
            if(subfind){                // found 'remain' in s
                CBLOCK_DBG(v,3," * subfind \n");
                return subfind;
            }
            if(comp1 == "**"){
                CBLOCK_DBG(v,3,"\n** subfind ");
                Cblock* deeper = s->find(p); // repeat "**/remain" search, depthwise
                if(deeper){
                    CBLOCK_DBG(v,3," ** deeper match\n");
                    return deeper;
                }
            }
        }
        return nullptr; // */remain not found
    }else if(comp1 == "..*"){
        // This is recursive _parent search, never looking underneath this
        if( !_parent || _parent==this || isRoot() ){
            CBLOCK_DBG(v,1," no parent for Cblock "<<_name<<"\n");
            THROW("Not possible to continue upward parent search");
        }
        if( _parent->_name == remain ){
            CBLOCK_DBG(v,1," FOUND exact match of parent "<<_parent->_name<<" with remain\n");
            return _parent;
        }
        Cblock * parentfind = _parent->find(remain);
        if(parentfind){
            CBLOCK_DBG(v,1," FOUND match of parent "<<_parent->_name<<" with remain at "<<parentfind->fullpath()<<"\n");
            return parentfind;
        }
        CBLOCK_DBG(v,1," search parent subtree\n");
        for(Cblock const* s: _parent->_sub){
            if( s == this ) continue; // search parent sub-tree EXCEPT for this
            CBLOCK_DBG(v,1,"\nssss sibling-find **/"<<remain<<" under sibling "<<s->fullpath()<<"\n");
            Cblock* sibfind = s->find("**/"+remain);   // force sub-tree search
            if(sibfind){
                CBLOCK_DBG(v,1,"\nssss found "<<remain<<" at "<<sibfind->fullpath()<<"\n");
                return sibfind;
            }
            CBLOCK_DBG(v,1,"\nssss did not find "<<remain<<"\n");
        }
        // repeat the SAME p="**/remain" search skipping the parent's sub-tree.
        return _parent->find(p);
    }else{                                            // "comp1/remain"
        for(Cblock const* s: _sub){
            if(s->_name == comp1){
                CBLOCK_DBG(v,3," sub");
                return s->find(remain);
            }
        }
        CBLOCK_DBG(v,3," no match for comp1=<"<<comp1<<">\n");
        return nullptr;
#if 0
        Cblock const* sub = find_immediate_sub(comp1);
        if(sub == this) return this; // comp1 not matched
        std::cout<<" sub["<<sub._name<<"].find(\""<<remain<<"\")"<<std::endl;
        if(remain.empty()) return sub; // comp1 was the last path component
        auto subfind = sub.find(remain);
        if( &subfind == &sub ) return this; // remain did not match
        return subfind;                     // return subfind Cblock
#endif
    }
    CBLOCK_DBG(v,3,"TBD: unknown case\n");
    assert("unseen case?"==nullptr);
    return nullptr;
}

/** add \prefix indent to all non-whitespace lines, \c sep is a set of line separators. */
inline std::ostream& prefix_lines(std::ostream& os, std::string code,
        std::string prefix, std::string sep=std::string("\n")){
    if( prefix.empty() ){
        os << code;
    }else if( !code.empty()){
        size_t nLoc = 0, nLocEnd;
        while ((nLocEnd = code.find_first_of(sep, nLoc)) != std::string::npos) {
            //std::cout<<" line["<<nLoc<<"..."<<nLocEnd<<"] = <"<<code.substr(nLoc,nLocEnd)<<">\n";
            // line is nLoc..nLocEnd, including the last sep char
            if( code.find_first_not_of(" \r\n\t",nLoc) < nLocEnd ){ // if not a blank line
                os << prefix << code.substr(nLoc,nLocEnd-nLoc) << "\n";
            }
            nLoc = nLocEnd+1;
        }
        //std::cout<<" nLoc="<<nLoc<<" code.size()="<<code.size();
        if(nLoc < code.size()){
            //std::cout<<" line["<<nLoc<<"...end] = <"<<code.substr(nLoc)<<">\n";
            // line is nLoc..nLocEnd, including the last sep char
            if( code.find_first_not_of(" \r\n\t",nLoc) < code.size() ){ // if not a blank line
                os << prefix << code.substr(nLoc); //<< "\n";
            }
        }
    }
    return os;
}
/** debug printout: see the DAG, not the actual code snippets */
inline std::ostream& Cblock::dump(std::ostream& os, int const ind/*=0*/)
{
    if(ind > 2000){
        THROW("Cblock dump depth really huge.  Maybe it is not a DAG.");
    }
    //int const v = _root->v+2;
    std::string in("\n&&& "+std::string(ind,' '));
    os<<in<<fullpath()
        <<(_premanip? " premanip": "")
        <<" code["<<_code.size()<<"]"
        <<" sub["<<_sub.size()<<"]"
        <<(_postmanip? "postmanip": "")
        ;
    for(auto s: _sub) s->dump(os,ind+2);
    return os;
}
inline std::ostream& Cblock::write(std::ostream& os)
{
    std::string& in = _root->indent;
    if(canWrite()){
        if(_root->v >= 2 || _code.size()==0){
            if(_root->v >= 2 && _code.size()) os<<in<<"//\n";
            if(_root->v >= 1){
                os<<in<<"// Cblock : "<<this->fullpath()<<" : "<<_type;
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
            s->write(os);
        }
        if(_postmanip) os << *_postmanip;
        // if( _next ) _next->write(os);
        ++_nwrites;
    }
    return os;
}
}//cunit::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CBLOCK_HPP
