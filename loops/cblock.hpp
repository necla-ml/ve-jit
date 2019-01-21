#ifndef CBLOCK_HPP
#define CBLOCK_HPP
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

    struct Cblock {
        /// Empty Cblock constructor (placeholder)
        Cblock(Cunit *root, std::string name="root")
            : _root(root), _parent(this), _name(name), // _parent==this means we are _root
            _type(""), _code(""), _sub(), _nwrites(0), _maxwrites(1)
            {}
        Cblock(Cblock *parent, std::string name="")
            : _root(parent->_root), _parent(parent), _name(name),
            _type(""), _code(""), _sub(), _nwrites(0), _maxwrites(1)
            {}
        Cblock(Cblock &&s) = default;
        Cblock(Cblock &s) = default;
        //Cblock(Cblock &&s) : _root(s._root), _name(s._name), _type(s._type),
        //                     _code(s._code), _sub(s._sub),
        //                     _nwrites(s._nwrites), _maxwrites(s._maxwrites)
        //{}

        /// Find \c p in \c sub -Cblocks, or append an empty Cblocks named \c p
        Cblock& operator[](std::string p);
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
        //Cblock& append(std::string code) {this->code += code; return *this;} // maybe inefficient
        Cblock *find(std::string path); // Search upward to root, then down through whole tree
        //Cblock* next();
        //Cblock* prev()
        ~Cblock(){ clear(); }
    private:
        friend struct Cunit;
        void clear(){ for(auto s: _sub) delete(s); _sub.clear(); }
        Cblock& unlink() { // remove this form _parent._sub[]
            assert( _parent );
            std::cout<<" unlink cblock "<<this->_parent->_name<<"/"<<_name<<std::endl;
            for(auto s=_parent->_sub.begin(); s!=_parent->_sub.end(); ++s){
                std::cout<<" unlink psub "<<(*s)->_name<<"? "; std::cout.flush();
                if(*s == this){
                    std::cout<<" YES!"; std::cout.flush();
                    _parent->_sub.erase(s); // all _sub iters (including s) INVALID
                    _parent = nullptr;
                    std::cout<<" _name unlinked"<<std::endl; std::cout.flush();
                    break;
                }
            }
            return *this;
        }
#if 0
        Program received signal SIGSEGV, Segmentation fault.
            cprog::Cblock::unlink (this=this@entry=0x618610) at cblock.hpp:85
            85                  for(auto s=_parent->_sub.begin(); s!=_parent->_sub.end(); ++s){
                (gdb) bt
#0  cprog::Cblock::unlink (this=this@entry=0x618610) at cblock.hpp:85
#1  0x00000000004014e4 in cprog::Cblock::after (prev=..., this=0x618610) at cblock.hpp:66
#2  main () at cblock.cpp:21

#endif
    private:
        struct Cunit * const _root;
        struct Cblock * _parent;
    private:
        std::string _name;
        std::string _type;
        std::string _code;
        std::vector<Cblock*> _sub;
        //Cblock *_next;
        //Cblock *_prev;
        int _nwrites;   // counter
        int _maxwrites; // limit for _nwrites
    };

    struct Cunit {
        std::string name;
        Cblock root;
        int v; // verbosity
        //std::map<std::string, Cblock*> blk;
        Cunit(std::string name) : name(name), root(this,name), v(2) {}
        ~Cunit() { root.clear(); }
        std::ostream& write(std::ostream& os) {return root.write(os);}  ///< write the program unit
        Cblock *find(std::string path);                 ///< absolute \c path down from \c root
        Cblock *find(std::string path, Cblock* from);   ///< search up \c from, then down from \c root
        void dump(std::ostream& os);                    ///< dump the tree
        Cblock & operator[](std::string name) { return root[name]; }

    };

    inline Cblock& Cblock::operator[](std::string p){
        if(p.empty()) return *this;
        for(auto s: _sub) if(s && s->_name==p) return *s;
        if(_root->v >= 2) std::cout<<"// new sub-block "<<_name<<"/"<<p<<" "<<_name<<".sub.size()="<<_sub.size()<<std::endl;
        _sub.push_back(new Cblock(this,p));
        return *_sub.back();
    }

    inline std::ostream& Cblock::write(std::ostream& os)
    {
        if(canWrite()){
            if(_root->v >= 2 || _code.size()==0){
                if(_root->v >= 2 && _code.size()) os<<"//\n";
                if(_root->v >= 1){
                    os<<"// Cblock : "<<_name<<" : "<<_type;
                    if(_code.empty()) os<<" (empty)";
                    os<<"\n";
                }
            }
            if(!_code.empty()) os << _code << "\n";
            if(_root->v >= 3 ) os<<"// _sub.size() = "<<_sub.size()<<"\n";
            for(auto s: _sub){
                if(_root->v >= 3 ) os<<"// ........ sub "<<_parent->_name<<"/"<<_name<<"/"<<s->_name<<std::endl;
                s->write(os);
            }
            // if( _next ) _next->write(os);
            ++_nwrites;
        }
        return os;
    }
}//cunit::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CBLOCK_HPP
