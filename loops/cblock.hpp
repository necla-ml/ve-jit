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
#include <cassert>

namespace cprog {

struct Cunit;
struct Cblock;
struct CbmanipBase;

struct IndentSpec;
struct PostIndent;
struct PreIndent;

/** Base Cblock/Cunit manipulator class.
 * Cunit may store "context", like current indent for \c write operations. */
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

    /// Find \c p in \c sub -Cblocks, or append an empty Cblocks named \c p
    Cblock& operator[](std::string p);
    Cblock& operator<<(std::string codeline) { return append(codeline); }
    Cblock& append(std::string codeline){ _code.append(codeline); return *this;}
    Cblock& append(Cblock &cb){
        std::cout<<" this@"<<_parent->_name<<"/"<<_name<<" append"; std::cout.flush();
        std::cout<<"(cb@"<<cb._name<<")"<<std::endl; std::cout.flush();
        assert( cb._parent == nullptr );
        cb._parent = this;
        _sub.push_back(&cb);
        std::cout<<" this@"<<_parent->_name<<"/"<<_name<<"{";
        for(auto s: _sub) std::cout<<" "<<s->_name;
        std::cout<<"}"<<std::endl;
        return *this;
    }
    /// unlink \c this and append it to some \c prev
    Cblock& after(Cblock& prev) { prev.append(unlink()); return *this; }

    int nWrites() const {return _nwrites;}
    bool canWrite() { return _nwrites>=0 && _nwrites<_maxwrites; }
    std::ostream& write(std::ostream& os);

    Cblock& setType(std::string type) {_type=type; return *this;}
    Cblock& setName(std::string type); // {this->type=type; return *this;} and update root!
    std::string const& getName() const {return _name;}
    //Cblock& append(std::string code) {this->code += code; return *this;} // maybe inefficient
    //Cblock* next();
    //Cblock* prev()
    ~Cblock(){ clear(); }
    bool isRoot() const { return _parent == this; }
    std::string fullpath() const {
        std::string out;
        out.reserve(256);
        int ncomp=0; // number of path components
        for(Cblock const * cb=this; cb!=nullptr; cb  = cb->_parent){
            //std::cout<<" fp:"<<cb->_name<<" parent:"<<cb->_parent->_name<<std::endl;
            //std::string component = (cb->isRoot()? std::string(): cb->_name);
            if( cb->isRoot() ){
                out.insert(0,"/");
                break;
            }else{
                size_t const cbsz = cb->_name.size();
                out.insert(0,cb->_name.c_str(),cbsz+1); // include terminal null
                out.replace(cbsz,1,1,'/');
            }
            ++ncomp;
        }
        if(ncomp>0) out.resize(out.size()-1U);
        return out;
    }
    /** find path.  TODO ".*" for "close-find" ~ subfind+upward_subfind ? */
    Cblock *find(std::string path) const; // Search upward to root, then down through whole tree
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
  private:
    friend struct Cunit;
    void clear(){ for(auto s: _sub) delete(s); _sub.clear(); }
    Cblock& unlink() { // remove this form _parent._sub[]
        int const v=1;
        assert( _parent );
        if(v>=1) std::cout<<" unlink cblock "<<fullpath()<<std::endl;
        for(auto s=_parent->_sub.begin(); s!=_parent->_sub.end(); ++s){
            if(v>=2) std::cout<<" unlink psub "<<(*s)->_name<<"? "; std::cout.flush();
            if(*s == this){
                if(v>=2) std::cout<<" YES!"; std::cout.flush();
                _parent->_sub.erase(s); // all _sub iters (including s) INVALID
                _parent = nullptr;
                if(v>=2) std::cout<<" _name unlinked"<<std::endl; std::cout.flush();
                break;
            }
        }
        return *this;
    }
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
};

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
    if(p.find("/") != std::string::npos
            || p == "." || p == ".."){
        return this->at(p); // throw if bad
    }
    for(auto s: _sub) if(s && s->_name==p) return *s;
    if(_root->v >= 2) std::cout<<"// new sub-block "<<_name<<"/"<<p<<" "<<_name<<".sub.size()="<<_sub.size()<<std::endl;
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
#ifdef NDEBUG
#define DEBUG(v,N) do{}while(0)
#else
#define DEBUG(V,N,...) do { \
    if((V)>=(N)){ \
        std::cout<<__VA_ARGS__; \
    } \
}while(0)
#endif
    DEBUG(v,3,std::string(8,'=')<<" Cblock "<<fullpath()<<" find(\""<<p<<"\")");
    if(p.empty()){
        DEBUG(v,3," empty => not found\n");
        return nullptr;
    }
    auto const firstslash = p.find("/");
    if(firstslash == std::string::npos){
        DEBUG(v,3," no firstslash => find_immediate_sub\n");
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
        DEBUG(v,3," starts-with-slash");
        // all "/<remain>" cases
        Cblock const& root = _root->root;
        if( remain.empty() ){
            DEBUG(v,3," remain.empty(), return root");
            return &_root->root;
        }
        if( root._name == p ){
            DEBUG(v,3," matches root name\n");
            return &_root->root;
        }
        DEBUG(v,3," root.find(\""<<remain<<"\")\n");
        return root.find(remain);
    }
    auto comp1 = p.substr(0,firstslash);
    if(remain.empty()){ // terminal [/]\+ not significant
        return this->find(comp1);
    }
    DEBUG(v,3,"find@<"<<comp1<<">/<"<<remain<<">");
    if( comp1 == "." ){                               // "./remain"
        DEBUG(v,3," ./<remain>\n");
        return this->find(remain);
    }else if( comp1 == ".." ){                        // "../remain"
        if(_parent /*&& _parent != this*/){ // "/.." is same as root (like FS)
            DEBUG(v,3," ../<remain>\n");
            return _parent->find(remain);
        }else{
            DEBUG(v,3," .. no parent\n");
            return nullptr;
        }
    }else if(comp1 == "*" || comp1 == "**"){          // "*/remain"
        for(Cblock const* s: _sub){
            Cblock* subfind = s->find(remain);
            if(subfind){                // found 'remain' in s
                DEBUG(v,3," * subfind \n");
                return subfind;
            }
            if(comp1 == "**"){
                DEBUG(v,3,"\n** subfind ");
                Cblock* deeper = s->find(p); // repeat "**/remain" search, depthwise
                if(deeper){
                    DEBUG(v,3," ** deeper match\n");
                    return deeper;
                }
            }
        }
        return nullptr; // */remain not found
    }else{                                            // "comp1/remain"
        for(Cblock const* s: _sub){
            if(s->_name == comp1){
                DEBUG(v,3," sub");
                return s->find(remain);
            }
        }
        DEBUG(v,3," no match for comp1=<"<<comp1<<">\n");
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
    DEBUG(v,3,"TBD: unknown case\n");
    assert("unseen case?"==nullptr);
    return nullptr;
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
        if(!_code.empty()) os << in << _code << "\n";
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
