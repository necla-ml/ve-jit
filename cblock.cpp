/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "cblock.hpp"
#include "stringutil.hpp"

// cblock.hpp can give large compilation times for nc++ inlining... so
// make this file non-empty to help speed compilation

namespace cprog{

std::string ve_pragma_unroll(int64_t const N){
    std::string ret(""); // return empty if N<0
    if(N==0) return "#pragma nounroll\n";
    else if(N>0){
        std::ostringstream oss;
        ret = OSSFMT("#pragma unroll("<<N<<")\n");
    }
    return ret;
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
Cblock& Cblock::operator>>(std::string codeline){
    return append("\n").append(codeline);
}

Cblock& Cblock::append(Cblock &cb){
    int const __attribute__((unused)) v=0;
    CBLOCK_DBG(v,3," append! "<<std::endl);
    assert(_parent != nullptr );
    CBLOCK_DBG(v,10," this@"<<_parent->_name<<"/"<<_name<<" append");
    CBLOCK_DBG(v,10," (cb@"<<cb._name<<")\n");
    cb._parent = this;

    auto last_spot = _sub.end();
    if( !_sub.empty() && _sub.back()->getName()=="last")
        --last_spot;
    _sub.insert(last_spot, &cb);

    CBLOCK_DBG(v,10," this@"<<_parent->_name<<"/"<<_name<<"{");
    if(v>=5){ for(auto s: _sub) std::cout<<" "<<s->_name; std::cout<<std::endl; }
    CBLOCK_DBG(v,10,"}"<<std::endl);
    return cb; // new behaviour
}
std::string Cblock::fullpath() const {
    int const __attribute__((unused)) v=0;
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
Cblock& Cblock::unlink() {
    int const __attribute__((unused)) v=0;
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
/** find/create subblock.
 * If p is a component (not a path), then
 *   \return subblock with name \c p (create subblock if nec, no throw)
 * Otherwise throw if \c p.empty() or use \c at(p) fails to find a path.
 * */
Cblock& Cblock::operator[](std::string p){
    //if(p.empty()) THROW("Cblock[""] oops - empty string!");
    assert(!p.empty());
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
    //_sub.push_back(new Cblock(this,p));
    //return *_sub.back();
    //  new: special "last" _sub will stay last.
    return this->append(*new Cblock(this,p));
}

Cblock * Cblock::find_immediate_sub(std::string p) const {
    assert( p.find("/") == std::string::npos );
    if(p.empty()) return nullptr;
    for(Cblock const* s: _sub) if(s && s->_name==p) return const_cast<Cblock*>(s);
    return nullptr;
}
Cblock * Cblock::find_recurse_sub(std::string p) const {
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
Cblock * Cblock::find_recurse_parent(std::string p) const {
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
Cblock& mk_extern_c(Cunit& cunit, std::string name){
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
    return block["body"];
#endif
}
/** [beg]:"\#if cond" + [body] + [end]:"\#endif // cond".
 * This has a "body" sub-block, like a scope, but no extra indenting.
 * No support for \em removing the indent so '#' lines end up in col 1.
 * (Could special-case this within "write" of the _code lines, I guess)
 */
Cblock& mk_cpp_if(Cunit& cunit, std::string name, std::string cond){
    Cblock& block = *(new Cblock(&cunit,name));
    block["beg"]<<"\n#if "<<cond;
    block["body"]; // empty
    block["end"]<<"#endif // "<<cond;
    return block;
}
/** [beg]~"\#if cond" + [body] + [else]~"\#else" + [end]~"\#endif". */
Cblock& mk_cpp_ifelse(Cunit& cunit, std::string name, std::string cond){
    Cblock& block = *(new Cblock(&cunit,name));
    block["beg"]<<"\n#if "<<cond;
    block["body"]; // empty
    block["else"]<<"\n#else // !( "<<cond<<"\n";
    block["end"]<<"#endif // "<<cond;
    return block;
}
/** create a name/{beg,body,end} triple, with subblock named "body" properly indented.
 * - \e body is always created empty.
 * - \e beg indents and \e end unindents
 * \return pointer to \e name/body node.
 * to insert a pre-"end" code, do \c return_value["../cleanup"]
 *
 * - For 'C' \c beg could, for example, be an "if(...){" clause.
 * - For 'asm', \c beg and \c end could be [verbatim] comment strings.
 * - default \c beg and \c end selected according to _root->flavor
 * - after mk_scope, usually want to position the code \e before or \e after
 *   some existing Cblock.
 * - in 'C' code , \c beg might be "if(cond)" or "else", etc.
 */
Cblock& mk_scope(Cunit& cunit, std::string name, std::string beg /*=""*/, std::string end /*=""*/){
    Cblock& block = *(new Cblock(&cunit,name));
    if(cunit.flavor!="asm"){
        if(cunit.flavor!="C"){
            std::cout<<" Warning: unknown Cblock flavor \""<<cunit.flavor
                <<"\". Assuming \"C\""<<std::endl;
        }
        block["beg"]<<beg<<"{ // "<<name<<PostIndent(+cunit.shiftwidth);
        block["first"];
        block["body"]; // empty
        if(!end.empty()) block["end"]<<end<<" ";
        block["end"]<<"} //"<<name<<PreIndent(-cunit.shiftwidth);
    }else{ // "asm"}
        block["beg"]<<"// BLOCK "<<name<<PostIndent(+cunit.shiftwidth);
        block["first"];
        if(!beg.empty()) block["beg"]<<beg<<"\n";
        block["body"];    // empty
        //block["cleanup"]; // empty
        if(!end.empty()) block["end"]<<end<<"\n";
        block["end"]<<"// END "<<name<<PreIndent(-cunit.shiftwidth);
    }
    return block;
}
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
Cblock* Cblock::find(std::string p) const {
    assert(_root != nullptr);
    int const __attribute__((unused)) v = _root->v;
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
            //THROW("Not possible to continue upward parent search");
            return nullptr;
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
}
/** where '#define' for this->define would appear. */
Cblock& Cblock::goto_defines() const {
    Cblock *a;
    {
        Cblock const* body = (getName()=="body"? this: find("..*/body)"));
        if(!body) body=this;                  // prospective "scope"
        a = body->find("..");                 // what encloses the "scope"?
        if(!a) a = const_cast<Cblock*>(this); // else "right here"
    }
    return *a;
}

/** common code */
static void emit_define(Cblock& a, Cblock& z, std::string name, std::string subst){
    a>>"#define "<<name<<" "<<subst; // multiline backslash support?
    z>>"#undef "<<name.substr(0,name.find('('));
}
/** attach to nearest-enclosing scope in a reasonable way. */
Cblock& Cblock::define(std::string name, std::string subst){
    // sanity checks on name?
    Cblock& a = goto_defines();
    Cblock& z = a["last"]["undefs"]; // try extra hard for undef to be 'last'
    emit_define(a,z,name,subst);
    return *this;
}

/** attach to nearest-enclosing scope in a reasonable way. */
Cblock& Cblock::define_here(std::string name, std::string subst){
    // sanity checks on name?
    Cblock& a = *this;
    Cblock& z = a["last"]["undefs"];
    emit_define(a,z,name,subst);
    return *this;
}

/** debug printout: see the DAG, not the actual code snippets */
std::ostream& Cblock::dump(std::ostream& os, int const ind/*=0*/)
{
    if(ind > 2000){
        THROW("Cblock dump depth really huge.  Maybe it is not a DAG.");
    }
    //int const v = _root->v+2;
    std::string in("\n&&& "+std::string(ind,' '));
    os<<in<<fullpath()<<(_premanip? " premanip": "");
    if(_code.size()) os<<" CODE["<<_code.size()<<"]";
    if(_sub.size()) os<<" SUB["<<_sub.size()<<"]";
    if(!_type.empty()) os<<" "<<_type;
    os<<(_postmanip? "postmanip": "");
    for(auto s: _sub) s->dump(os,ind+1); // it's easy to generate **very** deep trees
    return os;
}
std::ostream& Cblock::write(std::ostream& os, bool chkWrite)
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
        s->write(os,chkWrite);
    }
    if(_postmanip) os << *_postmanip;
    // if( _next ) _next->write(os);
    if(chkWrite) ++_nwrites;
    return os;
}
std::string Cblock::str(){
    std::ostringstream oss;
    _root->indent.clear();
    this->write( oss, false/*chkWrite*/ );
    return oss.str();
}
Cblock& Cblock::clear(){
    _code="";
    for(auto s: _sub) delete(s);
    _sub.clear();
    _type="";
    if(_premanip){ delete(_premanip); _premanip=nullptr;}
    if(_postmanip){ delete(_postmanip); _postmanip=nullptr;}
    return *this;
}

}//cprog::


#ifdef MAIN_CBLOCK
using namespace cprog;
using namespace std;

#define MUST_THROW(CODE) do{ \
    int threw = 0; \
    try{ \
        CODE; \
    }catch(...){ \
        threw = 1; \
    } \
    if( threw == 0 ){ \
        std::cout<<"Error: following code should have thrown:\n" \
        << #CODE << std::endl; \
        THROW("Stopping now"); \
    } \
}while(0)

/** This uses simple paths only, and demonstrates how output
 * order can be controlled.
 * - Simple Pre/Post-Indent "manipulator" demo.
 * - Notice that begin/middle/end constructs are awkward.
 */
void test_cblock_basic(){
    Cunit pr("program");
    pr.v = 3;
    pr["comments"]>>"// Cunit output from "<<__FILE__;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"];
    auto& macros = pr["macros"];
    auto& extern_c = pr["extern_C"];
    extern_c["beg"]
        <<"\n#ifdef __cplusplus"    // can appen w/ embedded newlines
        "\nextern \"C\" {"
        >>"#endif //C++";           // Note: >> auto-supplies initial newline
    pr["extern_C"]["end"]
        >>"ifdef __cplusplus"
        >>"extern \"C\""
        >>"endif //C++";
    //auto& functions = (pr["functions"].after(extern_c["beg"]));
    auto& functions = extern_c["beg"]["functions"]; // simpler equiv
    // '\:' --> '\\:'
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr["includes"]<<"#include <stdio.h>";
    macros<<"#define MSG \"hello\"";    // easy to add to previously defined code blocks
    macros>>CSTR(#define MSG2 "hello"); // Note: CSTR to auto-escape embeded '"'
    auto& foo = functions["foo"];
    // Klunky beg/middle/end w/ manipulator to adjust write context
    foo["beg"]<<"int foo() {"<<PostIndent(+2);
    foo["mid"]<<"return 7;";
    foo["end"]<<"}"<<PreIndent(-2);
    functions["bar"]<<"int bar(){ return 43; }\n";

    // output order can be adjusted with after: (subtree move)
    functions["foo"].after(functions["bar"]);

    //main.after("extern_C/open");
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_path(){
    Cunit pr("program");
    pr.v = 3;
    Cblock& root = pr.root;
    assert( root.find("") == nullptr );
    assert( root.find("asdfalsdkjfalsdkfj") == nullptr );
    assert( root.find("/") == &root );
    assert( root.find("/..") == &root );
    assert( root.find("/../") == &root );
    assert( root.find(".") == &root );
    assert( root.find("..") == &root );
    assert( root.find("./") == &root );
    assert( root.find("../") == &root );
    assert( root.find("./asdf") == nullptr );
    assert( root.find("../asdf") == nullptr );
    assert( root.find("*/asdf") == nullptr );
    assert( root.find("*/open") == nullptr );
    // creates "/includes", no var because never refered to later
    pr["includes"]<<"#include <stdio.h>";
    assert( root.find("includes") != nullptr );
    assert( root.find("includes")->getName() == "includes" );
    assert( root.find("includes") == &pr["includes"] );
    assert( root.find("/includes") != nullptr );
    assert( root.find("./includes") != nullptr );
    assert( root.find("../includes") != nullptr ); // because .. of root is root again
    assert( root.find("open") == nullptr );
    // Very important to use 'auto&' instead of plain 'auto'
    auto& macros = pr["macros"];
    auto& extern_c = pr["extern_C"];
    assert( extern_c.find("../open") == nullptr );
    assert( extern_c.find("./includes") == nullptr );
    assert( extern_c.find("../includes") != nullptr );
    assert( extern_c.find("/../includes") != nullptr );
    assert( extern_c.find("extern_C") != nullptr );
    extern_c["open"]<<
        "\n#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif //C++\n";
    assert( extern_c.find("../open") == nullptr );
    assert( extern_c.find("./includes") == nullptr );
    assert( extern_c.find("../includes") != nullptr );
    assert( root.find("open") == nullptr );
    assert( extern_c.find("open") != nullptr );
    assert( extern_c.find("./open") != nullptr );
    assert( extern_c.find("./open//") != nullptr );
    assert( root.find("*/open") != nullptr );
    assert( root.find("**/open") != nullptr );
    assert( root.find("extern_C/open") != nullptr );
    pr["extern_C"]["close"] // or as multiple strings
        >>"#ifdef __cplusplus\n"
        <<"}//extern \"C\"\n"
        <<"#endif //C++\n\n";
    //auto& functions = (pr["functions"].after(extern_c["open"]));
    auto& functions = extern_c["open"]["functions"]; // simpler equiv
    // '\:' --> '\\:'
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr["includes"]<<"#include <stdio.h>";
    macros<<"#define MSG \"hello\"";
    auto& foo = functions["foo"];
    // Klunky beg/middle/end w/ manipulator to adjust write context
    foo["beg"]<<"int foo() {"<<PostIndent(+2);
    foo["mid"]<<"return 7;";
    foo["end"]<<"}"<<PreIndent(-2);
    functions["bar"]<<"int bar(){ return 43; }\n";

    // output order can be adjusted with after:
    functions["foo"].after(functions["bar"]);
    assert( root.find("*/foo") == nullptr );
    assert( root.find("**/foo") != nullptr );
    assert( root.find("/extern_C/open/functions/bar/foo/mid/") != nullptr );
    assert( root.find("extern_C/open/functions/bar/foo/mid/") != nullptr ); 
    assert( root.find("program/extern_C/open/functions/bar/foo/mid/") == nullptr ); 
    assert( functions.find("..*/open") != nullptr );
    cout<<"\n\n"<<endl; cout.flush();
    assert( foo.find("..*/bar/") != nullptr ); // find in recursive "parent/sibling tree"
    assert( foo.find("..*/bar") != nullptr ); // find in recursive "parent/sibling tree"
    assert( foo.find("..*/foo/") != nullptr ); // find in recursive "parent/sibling tree"
    assert( foo.find("..*/foo") != nullptr ); // find in recursive "parent/sibling tree"
    assert( foo.find("..*/foo/") == &foo ); // find in recursive "parent/sibling tree"
    assert( foo.find("..*/foo") == &foo ); // find in recursive "parent/sibling tree"

    // Cblock::operator[p] was extended.
    // ORIGINAL behaviour for non-path p [no /] is to create the component
    // if it is not an immediate subblock.
    // NEW behaviour allows p to be a path,
    // and throws if p.empty() or p is a nonexistent path.
    try{
        std::cout<<"\n\n Cblock::at(p) tests"<<std::endl;
        assert( root.at(".").getName() == "program" );
        assert( root.at("..").getName() == "program" );
        assert( root.at("/macros").getName() == "macros" );
        assert( root.at("./macros").getName() == "macros" );
        assert( root.at("macros").getName() == "macros" );
        assert( root.at("*/open").getName() == "open" );
        assert( root.at("**/foo").getName() == "foo" );
        assert( root.at("*/../macros").getName() == "macros" );
    }catch(...){
        cout<<" Caught something\n";
        throw;
    }
    std::cout<<"\n\n Cblock::at(p) THROW tests"<<std::endl;
    MUST_THROW(root["*"]); // Paranoia about bugs wrt wildcards
    MUST_THROW(root[".illegal"]);
    MUST_THROW(root[".illegal"]);
    MUST_THROW(root.at("*"));
    MUST_THROW(root.at("*illegal"));
    MUST_THROW(root.at(".illegal"));

    MUST_THROW(root.at("asdfqewrasdf"));
    MUST_THROW(root.at("macrossss"));
    MUST_THROW(root.at("*/foo"));
    MUST_THROW(root.at("**/asdlkfj"));
    MUST_THROW(root.at("/extern_c/open")); // should be capital C
    MUST_THROW(root.at("never_seen_path"));

    try{
        std::cout<<"\n\n Cblock::operator[](p) tests"<<std::endl;
        assert( root["."].getName() == "program" );
        assert( root[".."].getName() == "program" );
        assert( root["/macros"].getName() == "macros" );
        assert( root["./macros"].getName() == "macros" );
        assert( root["macros"].getName() == "macros" );
        assert( root["*/../macros"].getName() == "macros" );
        assert( root["*/open"].getName() == "open" );
        assert( root["*/open"].fullpath() == "/extern_C/open" );
        assert( root["**/open"].getName() == "open" );
        // 1-component ==> create if never seen ...
        assert( root["never_seen_path"].getName() == "never_seen_path" );
    }catch(...){
        cout<<" Caught something\n";
        throw;
    }
    std::cout<<"\n\n Cblock::operator[](p) THROW tests"<<std::endl;
    MUST_THROW(root["./newsub"]); // cf. root["newsub"] which never fails
    MUST_THROW(root["*/foo"]);
    MUST_THROW(root["**/asdlkfj"]);
    MUST_THROW(root["/extern_c/open"]); // should be capital C

    //main.after("extern_C/open");
    pr.v = 2;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_short(){
    Cunit pr("program");
    pr.v = 10;
    pr["comments"]>>"// Cunit output from "<<__FILE__;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["includes"]>>"#include <stdio.h>";
    pr["macros"]>>"#define MSG \"hello\"";
    //mk_extern_c(pr,"extern_C").after(pr["macros"]);
    //  new function: after can accept an absolute path
    // OLD: mk_extern_c(pr,"extern_C").after("/macros");
    mk_extern_c(pr,"extern_C");  // new subnode "now" (after macros), returns /extern_C/body
    // FULL extern_C tree is at /extern_C/body/.."
    // now let's continue making thing at root level (NOT within extern_C)

    // creates "functions", appends foo/decl foo/body foo/end
    auto& foo_body = mk_func(pr,"foo","int foo()").after(pr["functions"])["body"];
    // for complex items, foo_body will itself get subdivided!

    // append(string) to _code is really only useful for simple stuff.
    // complex cases should name all sections/blocks ...
    foo_body["entry"]<<"int ret=-1;";   // foo/body/entry (anchored)

    // OK, this seems a reasonable idiom .. if00 --> path "/**/foo/body/if00/body"
    auto& if00 = mk_scope(pr,"if00","if(!defined(__cplusplus))").after(foo_body)["body"]; {
        if00<<"ret = 1;";
    }
    // with macros, a bit more readable, else00 --> Cblock path "/**/foo/body/else00/body"
    CBLOCK_SCOPE(else00,"else",pr,foo_body) {
        else00<<"ret = 2;";
    }
    CBLOCK_SCOPE(for00,"for(int i=0;i<10;++i)",pr,foo_body) {
        for00<<"ret = (ret^magic); //just to demo"
            >>"ret += i*magic2;\t// how to 'randomize' ret a bit";
        // oh, I wanted a magic const to be hoisted up into "entry" code...
        foo_body["entry"]>>"int const magic=0x12345678;";
        // or do an 'upward search' for a previously created code block (or stub)
        //for00.up["entry"]>>"int const magic2=0x23456789;";
        for00["..*/entry"]>>"int const magic2=0x23456789;";
    }
    //foo_body["exit"]<<"printf(\"%s\\nGoodbye\",MSG);\nreturn ret;";
    foo_body["exit"]<<CSTR(printf("%s\nGoodbye",MSG);\nreturn ret;);

    // short functions can be ... very short
    //mk_func(pr,"bar","int bar()").after(pr["functions"])["body"]<<"return 7";
#if 1-1
    // can we do Pre(Indent(-2)) for _pre.push_front(...) ?
    // or Pre-Manip, Pre+Manip for _pre.push_front/back <----
#endif
    // output order can be adjusted with after:
    cout<<pr.tree()<<endl;
    pr["functions"].after("/**/extern_C/body"); // The "/**" is just for show
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    //main.after("extern_C/open");
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}

void test_cblock_short2(){
    Cunit pr("program");
    pr.v = 0;
    // Very important to use 'auto&' instead of plain 'auto'
    pr["comments"]>>"// Cunit output from "<<__FILE__;
    pr["includes"]>>"#include <iostream>";
    pr["macros"]>>"define MSG \"hello\"";
    mk_extern_c(pr,"extern_C");         // OLD! NO GOOD! .after("/macros");
    // a somewhat more complicated function...
    auto& foo_body = mk_func(pr,"foo","int randomizer()").after(pr["functions"])["body"];
    foo_body["entry"]<<"int ret=-1;";   // foo/body/entry (anchored)
    int opt_level = 0; // my JIT decision making procedure

    CBLOCK_SCOPE(if00,"if(__FILE__[0]=='b')",pr,foo_body) {
        if00<<"ret = 1;";
    }
    CBLOCK_SCOPE(else00,"else",pr,foo_body) {
        else00<<"ret = 2;";
    }
    foo_body["preloop"]<<"// I have selected JIT randomization method "<<asDec(opt_level);
    CBLOCK_SCOPE(for00,"for(int i=0; i<10; ++i)",pr,foo_body) {
        CBLOCK_SCOPE(for01,"for(int j=i;i<10;++j)",pr,for00) {
            // Scenario: I have many JIT possibilities, but I decide to
            // use the following code ....
            if(opt_level==0) { // Oh, maybe I want my original JIT version ...
                for01<<"ret = (ret^magic); //just to demo"
                    // original version ...
                    >>"ret += (i*23+j)*magic2;\t// how to 'randomize' ret a bit";

                // I JIT realize this optimization uses some
                // undefined const values into foo/body/entry
                // ... NO PROBLEM ... Let's add to that code snippet
                foo_body["entry"]>>"int const magic=0x12345678;";   // exact destn known
                for01["..*/entry"]>>"int const magic2=rand();";     // alt "up-search-near" method
                // oh, I JIT realize I need yet another C header
                pr["includes"]>>"#include <stdlib.h>";
            }else if(opt_level==1){
                for01<<"ret = ret + i + j"; // this JIT is faster (but maybe not so random)
            }else{
                // OTHER JIT impls of foo are not shown, but may need entirely different
                //                 sets of code and patches to upward blocks of pr
            }
        }//j-loop
    }//i-loop
    foo_body["exit"]<<CSTR(printf("%s\nGoodbye",MSG);\nreturn ret;);
    pr["functions"].after("/**/extern_C/body"); // allow generous search (it is not directly under root)
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_dump(){
    Cunit pr("program");
    pr.v = 0;
    // overall structure, also demo new ','-operator (auto-supplies an initial newline)
    pr["comments"]>>"// Cunit output from "<<__func__;
    pr["includes"]>>"#include <assert.h>";
    pr["macros"];
    auto& cfuncs = mk_extern_c(pr,"extern_C")["body"]; // many mk_FOO have a "body" section
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // now fill in a function
    pr.v = 10;
    pr["/"].dump(cout);
    cout<<"\nHmmmm.  Let's look at dump to see how one instruction worked\n\n"<<endl;
    auto& macs = mk_func(pr,"macs","int macs(int i)")
        .after(cfuncs)["body"]
        <<"assert(i>=0);";
    pr["/"].dump(cout);
    macs>>"return 75/i;";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}

/** based on a very short (slow) direct_default3.c */
string cjitConvolutionForward00( int const verbosity=0 /*struct param const* const p*/ )
{
    Cunit pr("program");
    pr["includes"]<<Endl<<CSTR(#include "vednn.h")
        <<Endl<<CSTR(#include "veintrin.h")
        <<"\n#include <stdio.h>"
        <<"\n#include <stdlib.h>"
        <<"\n#include <assert.h>"
        ;
    pr["macros"]<<"\n#define VLEN (256)"
        ;
    auto & fns = mk_extern_c(pr,"extern_C");
    std::string fn_declare;
    {
        std::string funcname("cjitConvFwd00");
        std::ostringstream oss;
        oss<<"void "<<funcname<<"("
            <<"\n        const vednnTensorParam_t * restrict      pParamIn,"
            <<"\n        const void * restrict                    pDataIn,"
            <<"\n        const vednnFilterParam_t * restrict      pParamKernel,"
            <<"\n        const void * restrict                    pDataKernel,"
            <<"\n        const vednnConvolutionParam_t * restrict pParamConv,"
            <<"\n        const vednnTensorParam_t * restrict      pParamOut,"
            <<"\n        void * restrict                          pDataOut"
            <<"\n        )";
        fn_declare = oss.str();
    }
    auto& fn = mk_func(pr,"fn",fn_declare).after(fns)["body"];

    // get the vars here first.
    const int64_t batch          = 52;
    const int64_t group          = 2;
    const int64_t inChannel      = 100;
    const int64_t inHeight       = 27;
    const int64_t inWidth        = 27;
    const int64_t outChannel     = 100;
    const int64_t outHeight      = 27;
    const int64_t outWidth       = 27;
    const int64_t kernHeight     = 3;
    const int64_t kernWidth      = 3;
    const int64_t strideHeight   = 1;
    const int64_t strideWidth    = 1;
    const int64_t padHeight      = 1;
    const int64_t padWidth       = 1;
    const int64_t dilationHeight = 1; // mkl-dnn value plus one
    const int64_t dilationWidth  = 1;
    assert( outWidth > 0 );

    const int64_t inChannelGroup  = inChannel  / group;   // equal to pDataKernel->inChannel
    const int64_t outChannelGroup = outChannel / group;   // equal to pDataKernel->outChannel

    const int64_t inHW = inHeight * inWidth;
    const int64_t kernHW = kernHeight * kernWidth;
    const int64_t outHW = outHeight * outWidth;

    // then emit them as constant cjit values (or #define them)
//#define CONST1(var) >>("int64_t const " #var " = "+asDec(var))
// #define is better, because is is definitely usable with 'C' compiler
#define CONST1(var) >>("#define " #var " "+asDec(var))
    //auto& fn_const =
    fn["const"]
        CONST1(batch            )
        CONST1(group            )
        CONST1(inChannel        )
        CONST1(inHeight         )
        CONST1(inWidth          )
        CONST1(outChannel       )
        CONST1(outHeight        )
        CONST1(outWidth         )
        CONST1(kernHeight       )
        CONST1(kernWidth        )
        CONST1(strideHeight     )
        CONST1(strideWidth      )
        CONST1(padHeight        )
        CONST1(padWidth         )
        CONST1(dilationHeight   )
        CONST1(dilationWidth    )

        CONST1(inChannelGroup   )
        CONST1(outChannelGroup  )

        CONST1(inHW             )
        CONST1(kernHW           )
        CONST1(outHW            )
        ;
#if 0
    const float * restrict pIn     = pDataIn;
    const float * restrict pKernel = pDataKernel;
    //float * restrict const pOut    = pDataOut;
    float * restrict pOut    = pDataOut;
#endif
    auto& fn_ptrs = fn["ptrs"];
    fn_ptrs>>"float const * restrict pIn  = pDataIn;"
        >>"float const * restrict pKernel = pDataKernel;"
        >>"float * restrict pOut = pDataOut;"
        ;

    //auto& fn_vec_init =
    fn["vec_init"]
        >>"_ve_lvl(VLEN);"
        >>"const __vr vzeros = _ve_vbrdu_vs_f32(0.0f); // lower 32-bits are zero bits, so same as _ve_pvbrd_vs_i64(0UL)"
        >>"const __vr vrseq = _ve_vseq_v();"
        >>"const int64_t sw_x_VLEN = strideWidth * VLEN;"
        >>"int64_t const vl_x_init = outWidth /*- x0=0*/ < VLEN ? outWidth /*- x0=0*/ : VLEN ;"
        >>"int64_t vl = vl_x_init;"
        >>"_ve_lvl(vl);"
        >>"__vr const vrj_init = _ve_vaddsl_vsv(-padWidth,  _ve_vmulsl_vsv(strideWidth, vrseq));"
        ;

    CBLOCK_SCOPE(loop_n,"for(int64_t n=0; n<batch; ++n)",pr,fn);
    CBLOCK_SCOPE(loop_g,"for(int64_t g=0; g<group; ++g)",pr,loop_n); // OK sub-tree
    loop_g
        >>"const int64_t outGroupOffset  = g * outChannelGroup * outHW;"
        >>"const int64_t inGroupOffset   = g * inChannelGroup * inHW;"
        >>"const int64_t kernGroupOffset = g * outChannelGroup * inChannelGroup * kernHW;"
        >>"const float *pIn_0 = pIn + inGroupOffset + (n * inChannel + 0) * inHW;"
        ;
    CBLOCK_SCOPE(loop_k,"for(int64_t k=0 ; k<outChannelGroup; ++k)",pr,loop_g);
    loop_k
        >>"int64_t outIndex = outGroupOffset + (n * outChannel + k) * outHW;"
        >>"const float * restrict pKern_gk = pKernel + kernGroupOffset"
        >>"                                + (k * inChannelGroup + 0) * kernHW;"
        >>"//int64_t kIndex_0 = kernGroupOffset + (k * inChannelGroup + 0) * kernHW;"
        ;
    CBLOCK_SCOPE(loop_y,"for(int64_t y=0 ; y<outHeight; ++y)",pr,loop_k);
    loop_y
        >>"const int64_t i = y * strideHeight - padHeight;"
        >>""
        >>"int64_t kh_end=0;"
        >>"const int64_t kh_tmp = dilationHeight-i-1;"
        >>"const int64_t kh_beg= (i>=0? 0: kh_tmp / dilationHeight);"
        >>"if (i < inHeight){"
        >>"  kh_end = (inHeight + kh_tmp) / dilationHeight;"
        >>"  if (kh_end >= kernHeight) kh_end = kernHeight;"
        >>"}"
        >>""
        >>"int64_t vl = vl_x_init;"
        >>"_ve_lvl(vl);"
        >>"__vr vrj = vrj_init;"
          ;
    CBLOCK_SCOPE(loop_x0,"for(int64_t x0=0 ; x0<outWidth; x0+=VLEN)",pr,loop_y);
    loop_x0
            >>"const int64_t vl = outWidth - x0 < VLEN ? outWidth - x0: VLEN;"
            >>"_ve_lvl(vl);"
            >>"__vr vrsum = vzeros;"
            ;
    CBLOCK_SCOPE(loop_r,"for (int64_t r = kh_beg; r < kh_end; ++r)",pr,loop_x0);
    //loop_r>>"vrw = vrj";
    CBLOCK_SCOPE(loop_s,"for (int64_t s = 0; s < kernWidth; s++)",pr,loop_r);
    loop_s.def("LOOP_S_FOO","LOOP_S_BAR"); // demo the scoped #define function (NEW)
    loop_s[".."]>>"vrw = vrj"; // current pos loop_r/body/loop_s CODE, **before** loop_s/beg opens the loop
    loop_s["last"]             // into loop_s/body/last, just before loop_s/end exits the loop
        >>"vrw = _ve_vaddsl_vsv(dilationWidth,  vrw) ; // <--- vector induced"
        ;
    loop_s[".."]["last"]>>"//loop_s has just exited!";
    loop_s
        >>"__vm256 vm2 = _ve_vfmkl_mcv(VECC_GE, vrw);        // condition(0 <= w)"
        >>"__vm256 vm3 = _ve_vfmkl_mcv(VECC_IG, _ve_vcmpsl_vsv(inWidth,vrw));  // condition(w < inWidth)"
        >>"__vm256 vm23  = _ve_andm_mmm(vm2, vm3);"
        ;
    CBLOCK_SCOPE(loop_c,"for (int64_t c = 0; c < inChannelGroup; ++c)",pr,loop_s);
    loop_c
        >>"const float *pIn = pIn_0 + c*inHW + (i+r*dilationHeight)*inWidth"
        >>"                 + x0*strideWidth-padWidth + s*dilationWidth;"
        >>"const float *pKerValue = pKern_gk + c*kernHW + r*kernWidth +s;"
        >>"__vr vrin = _ve_vldu_vss(4*strideWidth,pIn) ;"
        >>"vrin = _ve_vmrg_vvvm(vzeros, vrin, vm23) ;"
        >>"vrsum = _ve_vfmads_vvsv(vrsum, *pKerValue, vrin) ;"
        ;
    //loop_s["induce vrw"]// BEFORE the '}' of loops_s (embedded blanks OK, but harder to read
    //    >>"vrw = _ve_vaddsl_vsv(dilationWidth,  vrw) ; // <--- vector induced"
    //    ;
    // loop_r path: .../loop_x0/body/loop_r/body
    //loop_r[".."] // too early (add to loop_x0/body; before loop_r even begins)
    //loop_r["../.."] // same as above
    //loop_r["end"] // too early, before "}//loop_r", creates path loop_x0/body/loop_r/body/end
    //loop_r["../end"]      // OK, adds to /**loop_x0/body/loop_r/end
    //loop_r[".."]["done"]  // OK, adds to append-tree /**/loop_x0/body/loop_r/done
    loop_x0["induce+store"]       // OK, adds to /**/loop_x0/body/induce+store
        >>"_ve_vstu_vss(vrsum, 4, pOut) ;"
        >>"vrj = _ve_vaddsl_vsv(sw_x_VLEN,vrj); // induce to avoid full recalc"
        >>"pOut += vl; // visible speedup cf. outIndex+=vl"
        ;
    fn["exit"]>>"return VEDNN_SUCCESS;" ;


    pr["end"]>>"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // debug: 'str()' should bypass the write-counting thing and always work
    auto const sz0 = pr.str().size();
    cout<<" pr.str().size() = "<<sz0;
    assert( sz0 > 0 );
    auto const sz1 = pr.str().size();
    cout<<" pr.str().size() = "<<sz1;
    assert( sz1 == sz0 );
    // For demo program, dump the full tree...
    cout<<string(80,'-')<< pr.tree() <<string(80,'-')<<endl;
    cout<<string(80,'-')<< pr.str() <<string(80,'-')<<endl;

    pr.v = verbosity;
    return pr.str();
}

#if 0 // original function, for comparison...
vednnConvolutionForward_direct_default3(
    const vednnTensorParam_t * restrict   pParamIn,
    const void * restrict       pDataIn,
    const vednnFilterParam_t * restrict   pParamKernel,
    const void * restrict       pDataKernel,
    const vednnConvolutionParam_t * restrict   pParamConv,
    const vednnTensorParam_t * restrict   pParamOut,
    void * restrict         pDataOut
)
{
  const int64_t batch      = pParamIn->batch;
  const int64_t inChannel  = pParamIn->channel;
  const int64_t inWidth    = pParamIn->width;
  const int64_t inHeight   = pParamIn->height;
  const int64_t outChannel = pParamOut->channel;
  const int64_t outWidth   = pParamOut->width;
  const int64_t outHeight  = pParamOut->height;
  const int64_t kernWidth  = pParamKernel->width;
  const int64_t kernHeight = pParamKernel->height;
  assert( outWidth > 0 );

  const int64_t group          = pParamConv->group;
  const int64_t strideWidth    = pParamConv->strideWidth;;
  const int64_t strideHeight   = pParamConv->strideHeight;
  const int64_t padWidth       = pParamConv->padWidth;
  const int64_t padHeight      = pParamConv->padHeight;
  const int64_t dilationWidth  = pParamConv->dilationWidth;
  const int64_t dilationHeight = pParamConv->dilationHeight;

  const int64_t inChannelGroup  = inChannel  / group;   // equal to pDataKernel->inChannel
  const int64_t outChannelGroup = outChannel / group;   // equal to pDataKernel->outChannel

  const float * restrict pIn     = pDataIn;
  const float * restrict pKernel = pDataKernel;
  //float * restrict const pOut    = pDataOut;
  float * restrict pOut    = pDataOut;

  const int64_t inHW = inHeight * inWidth;
  const int64_t kernHW = kernHeight * kernWidth;
  const int64_t outHW = outHeight * outWidth;


  _ve_lvl(VLEN) ; // <----- VERY VERY VERY IMPORTANT to remember this init !!! 1.
  const __vr vzeros = _ve_vbrdu_vs_f32(0.0f) ; // lower 32-bits are zero bits, so same as _ve_pvbrd_vs_i64(0UL)
  const __vr vrseq = _ve_vseq_v();
  const int64_t sw_x_VLEN = strideWidth * VLEN;
  int64_t const vl_x_init = outWidth /*- x0=0*/ < VLEN ? outWidth /*- x0=0*/ : VLEN ;
  int64_t vl = vl_x_init;
  _ve_lvl(vl) ;
  __vr const vrj_init = _ve_vaddsl_vsv(-padWidth,  _ve_vmulsl_vsv(strideWidth, vrseq));

  //int64_t const kByMax = 1;
  //int64_t const zero = 0;

  for (int64_t n = 0; n < batch; n++) {
    for (int64_t g = 0; g < group; g++) {
      const int64_t outGroupOffset  = g * outChannelGroup * outHW;
      const int64_t inGroupOffset   = g * inChannelGroup * inHW;
      const int64_t kernGroupOffset = g * outChannelGroup * inChannelGroup * kernHW;
      const float *pIn_0 = pIn + inGroupOffset + (n * inChannel + 0) * inHW;
      for(int64_t k=0 ; k<outChannelGroup; ++k) {

        int64_t outIndex = outGroupOffset + (n * outChannel + k) * outHW;
        const float * restrict pKern_gk = pKernel + kernGroupOffset + (k * inChannelGroup + 0) * kernHW;
        //int64_t kIndex_0 = kernGroupOffset + (k * inChannelGroup + 0) * kernHW;

        for (int64_t y=0; y<outHeight; y++) {
          const int64_t i = y * strideHeight - padHeight;

          int64_t kh_end=0;
          const int64_t kh_tmp = dilationHeight-i-1;
          const int64_t kh_beg= (i>=0? 0: kh_tmp / dilationHeight);
          if (i < inHeight){
            kh_end = (inHeight + kh_tmp) / dilationHeight;
            if (kh_end >= kernHeight) kh_end = kernHeight;
          }

          int64_t vl = vl_x_init;
          _ve_lvl(vl) ;
          __vr vrj = vrj_init;
          for ( int64_t x0=0; x0<outWidth; x0+=VLEN )
          {
            const int64_t vl = outWidth - x0 < VLEN ? outWidth - x0 : VLEN ;
            _ve_lvl(vl) ;
            __vr vrsum = vzeros;
            // slower:
            //    any use ov _ve_lvs_svs_u64/f32
            //    any type of blocking 'c' loop (many ways tried)
            //    clang prefetch will not compile
            //    precalc offset expressions (cannnot distribute scalar calc better than clang)
            for (int64_t r = kh_beg; r < kh_end; ++r) {
              //const int64_t h = i + r * dilationHeight; // kh_beg,kh_end guarantee h in [0,outHeight)
              __vr vrw = vrj;
              for (int64_t s = 0; s < kernWidth; s++) {
                __vm256 vm2 = _ve_vfmkl_mcv(VECC_GE, vrw) ;        // condition(0 <= w)
                __vm256 vm3 = _ve_vfmkl_mcv(VECC_IG, _ve_vcmpsl_vsv(inWidth,vrw)) ;  // condition(w < inWidth)
                __vm256 vm23  = _ve_andm_mmm(vm2, vm3) ;
                for (int64_t c = 0; c < inChannelGroup; ++c)
                {
                  const float *pIn = pIn_0 + c*inHW + (i+r*dilationHeight)*inWidth + x0*strideWidth-padWidth + s*dilationWidth;

                  const float *pKerValue = pKern_gk + c*kernHW + r*kernWidth +s;
                  __vr vrin = _ve_vldu_vss(4*strideWidth,pIn) ;
                  vrin = _ve_vmrg_vvvm(vzeros, vrin, vm23) ;
                  vrsum = _ve_vfmads_vvsv(vrsum, *pKerValue, vrin) ;
                } // inChannel

                vrw = _ve_vaddsl_vsv(dilationWidth,  vrw) ; // <--- vector induced (not fully calc)
              } // s .. kernWidth
            } // r .. kernHeight
            //_ve_vstu_vss(vrsum, 4, pOut+outIndex) ;
            _ve_vstu_vss(vrsum, 4, pOut) ;
            vrj = _ve_vaddsl_vsv(sw_x_VLEN,vrj); // induce to avoid full recalc
            //outIndex += vl ; /* MUST always execute (before break) */
            pOut += vl; // visible speedup
          } // x
        } // y
      } //k..kMax..kBy (outChannelGroup)
    } // group
  } // batch

  return VEDNN_SUCCESS;
}
#endif
void test_cblock_macros(){
    Cunit pr("program");
    pr.v = 0;
    // overall structure
    pr["comments"]>>"// Cunit output from "<<__func__;
    pr["includes"]>>"#include <assert.h>";
    pr["macros"];
    auto& cfuncs = mk_extern_c(pr,"extern_C")["body"]; // many mk_FOO have a "body" section
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // now fill in a function
    pr.v = 10;
    auto& macs = mk_func(pr,"macs","int macs(int i)").after(cfuncs)["body"]
        <<"assert(i>=0);";
    macs["exit"]>>"return 75/i;";
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
void test_cblock_define(){
    Cunit pr("program");
    pr.v = 2;
    pr.root.define("PR_ROOT_0");
    (pr["body"]>>"// pr[\"body\"] HERE")
        .define("PR_BODY_DEF");
    (pr["foo"]>>"// pr[\"foo\"] HERE")
        .define("PR_FOO_DEF")
        .define("MULTILINE1", "do{ \\\n  //something in enclosing scope via 'define' \\\n} while(0)")
        ;
    CBLOCK_SCOPE(blk1,"",pr,pr.root);
    blk1.define("BLK1_DEF1");
    blk1>>"// blk1 HERE";
    (blk1["last"]>>"// blk1 LAST")
        .define("BLK1_LAST_DEF");
    auto& nextsub = (blk1["next"]>>"// blk1/next")["nextsub"];
    (nextsub>>"// blk1/next/nextsub...NEXTSUB_DEF?")
        .define("NEXTSUB_DEF")
        .define_here("LOCAL_HELP", "do{ \\\n  //something local via 'define_here' \\\n} while(0)")
        >>"LOCAL_HELP;"
        >>"LOCAL_HELP;"
        ;
    blk1["last"]>>"// blk1/last HERE";
    CBLOCK_SCOPE(subblock,"",pr,blk1);
    subblock.define("SUBBLOCK_DEF")>>"//subblock";
    CBLOCK_SCOPE(seqsubblock,"",pr,blk1);
    seqsubblock.define("SEQSUBBLOCK_DEF")>>"//seqsubblock";
    blk1.define("BLK1_DEF2");
    pr.v = 0;
    cout<<string(80,'-')<<endl;
    pr.write(cout);
    cout<<string(80,'-')<<endl;
}
/** a 1-liner sample kernel "type sum = a + b;" */
void kernel_define_sum(Cblock& cb, std::string type, std::string sum, std::string a, std::string b){
    cb>>type<<" "<<sum<<" = "<<a<<" + "<<b<<";";
}
/** a scope kernel "{ foo(); ++COUNTER; } */
Cblock& kernel_scope(Cblock& cb, std::string arg){
    CBLOCK_SCOPE(kernel_scope,"",cb.getRoot(),cb);
    kernel_scope>>"foo("<<arg<<");"
        >>"++COUNTER";
    return kernel_scope;
}
Cblock& kernel_sum_k(Cblock& cb, uint64_t n){    
    static int disambig=0;
    ++disambig;
    std::ostringstream oss;
    auto kName = OSSFMT("kernel_sum_k"<<disambig);
    auto& kern = mk_scope(cb.getRoot(),kName,OSSFMT("for(size_t a=0; a<"<<n<<"; ++a)"))
        .after(cb).at("body");
    kern>>"k+=a;";
    return kern;
}
void test_cblock_kernel(){
    Cunit pr("program");
    pr.v = 0;
    // overall structure
    pr["comments"]>>"// Cunit output from "<<__func__;
    pr["includes"]>>"#include <assert.h>";
    pr["macros"];
    auto& cfuncs = mk_extern_c(pr,"extern_C");
    pr["end"]<<"// vim: ts=4 sw=4 et cindent cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break";
    // now fill in a function
    pr.v = 1;
    CBLOCK_SCOPE(fn,"int fn(int i)",pr,cfuncs);
    fn>>"assert(i>=0);";
    kernel_define_sum(fn,"int","j","i","i");

    auto& sub = kernel_scope(fn,"i");
    sub.define("COUNTER","j");

    kernel_define_sum(fn,"//int","k","i","j"); // oh. would go before sub, which introduced a [] subblock!
    kernel_define_sum(fn["1"],"int","k","i","j"); // I want it after sub.

    kernel_sum_k(fn["ksum3"],3);
    fn["2"]>>"// now use the kernel again";
    kernel_sum_k(fn["ksum5"],5);

    sub>>"// I can add a  comment to sub, after the fact";
    fn["last"]>>"return 75/k;";
    pr.v=0;
    cout<<string(80,'-')<< pr.str() <<string(80,'-')<<endl;
    cout<<string(80,'-')<< pr.tree() <<string(80,'-')<<endl;
}
int main(int,char**){
    test_cblock_basic();
    test_cblock_path();
    test_cblock_short();
    test_cblock_short2();
    test_cblock_dump();
    test_cblock_macros();
    test_cblock_define();
    test_cblock_kernel();
    string code = cjitConvolutionForward00(); // optional arg: verbosity=0
    cout<<string(80,'-')<< code <<string(80,'-')<<endl;
    assert(code.size()>0);
    cout<<"\nGoodbye"<<endl; cout.flush();
}
#endif // MAIN_CBLOCK
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
