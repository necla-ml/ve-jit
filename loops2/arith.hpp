#ifndef __ARITH_HPP__
#define __ARITH_HPP__
#include <iostream>
#include <sstream>
#include <assert.h>
#include <vector>
#include <type_traits>

namespace arith {

template<typename VAL> class Arith;
template<typename VAL> class Mul;
template<typename VAL> class Add;
template<typename VAL> class Lin;

namespace {
    std::string dcl(std::string type, std::string modifiers, std::string varname) {
        std::ostringstream oss; oss<<type<<" "<<modifiers<<" "<<varname<<";\n";
        return oss.str();
    }
    std::string ass(std::string lhs, std::string rhs) {
        std::ostringstream oss; oss<<lhs<<" = "<<rhs<<";\n"; return oss.str();
        return oss.str();
    }
}

template<typename T>
char const* type_string() {
	return std::is_same<T,int64_t>::value? "int64_t"
		: std::is_same<T,uint64_t>::value? "uint64_t"
		: std::is_same<T,int32_t>::value? "int32_t"
		: std::is_same<T,uint32_t>::value? "uint32_t"
		: std::is_same<T,int8_t>::value? "int8_t"
		: std::is_same<T,uint8_t>::value? "uint8_t"
		: std::is_same<T,float>::value? "float"
		: std::is_same<T,double>::value? "double"
		:"void";
}

typedef enum:uint32_t {DEFAULT=0, CONST=1, NONCONST=2, PARENS=4, CCONST=8} Property;
// Arith<VAL>  ~ expression (rhs)
// Define<VAL> ~ TYPE var; var=Arith<VAL> ???
/** This is OK for simple expressions, but is not good wrt number of parentheses.
 * It is greedy, so will miss some optimizations. */
template<typename VAL>
class Arith {
public:
    typedef Arith<VAL> C;
    typedef arith::Property Property;
    static const int verbose=0;
    void chk(){
        std::cout.flush();
        if( !isVar && !isExpr ) assert(isConst);
    }
    Arith(VAL const c) : c(c), name(), parens(false), isVar(false), isExpr(false), isConst(true) {
        name = vstr();
        chk();
    }
    //explicit Arith(int64_t const c) : c(c), name(), parens(false) {
    //    std::ostringstream oss; oss << c; name = oss.str();
    //}
    //explicit Arith(uint64_t const c) : c(c), name() {
    //    std::ostringstream oss; oss << c; name(oss.str());
    //}
    Arith( VAL const c, std::string s, Property const p=DEFAULT ) : c(c), name(s), expression(), parens((p&PARENS)!=0), isVar(true), isExpr(false), isConst(p&CONST?true:p&NONCONST?false:true) {
        if(verbose>=1){std::cout<<"<V"<<name<<","<<c<<","<<expression<<">"<<flags(); std::cout.flush();}
        //std::ostringstream oss; oss<<s; name = oss.str();
        chk();
    }

    Arith( C const& src, Property const p=DEFAULT ):
        c(src.c),
        name(src.name),
        expression(src.expression),
        parens(p&PARENS? true: src.parens),
        isVar   (src.isVar),
        isExpr  (src.isExpr),
        isConst (p&CCONST? true : p&CONST? true : p&NONCONST? false: src.isConst),
        isCConst(p&CCONST? true : p&CONST? false: p&NONCONST? false: src.isCConst)
    {
        if(isConst && !isExpr) { /*std::cout<<" n"<<name<<"->v";*/ name = vstr(); }
        if(verbose>=1){std::cout<<"<C"<<name<<","<<c<<","<<expression<<">"<<flags(); std::cout.flush();}
        //assert(!(p&CONST)); assert(!(p&NONCONST));
        chk();
    }
    Arith( C const&& src, Property const p=DEFAULT ) : c(src.c), name(src.name), expression(src.expression), parens(p&PARENS? true: src.parens),
    isVar(false/*src.isVar*/), isExpr(src.isExpr), isConst(src.isConst) {
        assert( !src.isConst );
        if(isConst && !isExpr) { std::cout<<" n&&->v"; name = vstr(); }
        if(verbose>=1){std::cout<<"<C&&"<<name<<","<<c<<","<<expression<<">"<<flags(); std::cout.flush();}
        assert(!(p&CONST)); assert(!(p&NONCONST));
        chk();
    }

    Arith( C const& src, std::string s, Property const p=DEFAULT ): c(src.c), name(s), expression(src.rhs()), parens(p&PARENS? true: false),
    //Arith(int32_t const c, std::string s) : c(c), name() {
    //    std::ostringstream oss; oss<<'('<<s<<')'; name = oss.str();
    //}
    //Arith(uint32_t const c, std::string s) : c(c), name() {
    //    std::ostringstream oss; oss<<'('<<s<<')'; name = oss.str();
    //}
    isVar(true), isExpr(src.isExpr), isConst(p&CONST? true: p&NONCONST? false: src.isConst) {
        //std::cout<<" src={"<<src.flags()<<src.c<<","<<src.name<<","<<src.expression<<",rhs="<<src.rhs()<<"}";
        if( src.isVar && !src.isConst ) isExpr = true;
        //std::ostringstream oss; oss<<type_string<VAL>()<<(isConst?"const ":"")<<name<<" = ("<<src.expression<<");";
        //expression = oss.str();
        if(verbose>=1){std::cout<<"<Cs"<<name<<","<<c<<","<<expression<<">"<<flags(); std::cout.flush();}
        chk();
    }
    Arith( C const&& src, std::string s, Property const p=DEFAULT ): c(src.c), name(s), expression(src.expression), parens(p&PARENS? true: false),
    isVar(true), isExpr(src.isExpr), isConst((p&CONST)? true: (p&NONCONST)? false: src.isConst) {
        //std::cout<<" src={"<<src.flags()<<src.c<<","<<src.name<<","<<src.expression<<",rhs="<<src.rhs()<<"}";
        if(verbose>=1){std::cout<<"<Cs&&"<<name<<","<<c<<","<<expression<<">"<<flags(); std::cout.flush();}
        chk();
    }
    //operator VAL() const {return c;}
    VAL v() const {return c;}
    std::string vstr() const{ std::ostringstream oss; oss<<c; return oss.str(); }
    std::string n() const{ return name; }
    std::string e() const{ return expression; }
    std::string expr() const{
        std::ostringstream oss;
        if(parens) oss<<'(';
        if(0){ if(isVar) oss<<"v"; if(isExpr) oss<<"e"; oss<<"{"; }
        if(!isVar && !isExpr) {
            oss<<""<<name; // constant, or tmp RHS expression
        } else if(isVar && !isExpr){
            oss<<name<<" = "<<c;
        } else if(isVar && isExpr){
            oss<<name<<" = "<<expression;
        } else if(!isVar && isExpr){
            oss<<expression;
        }else{
            oss <<name<<" = "<<expression;
        }
        if(0){ oss<<"}"; }
        if(parens) oss<<')';
        return oss.str();
    }
    std::string rhs() const {
        //std::string s = isVar ? name
        //    : isExpr ? expression
        //    : name;
        std::ostringstream oss;
        if(parens) oss<<"(";
        if( isVar ){
            if( !isExpr && isConst ) oss<<c; //oss<<c<<"/*"<<name<<"*/";
            else oss<<name;
        }else if( isExpr ){
            if( isVar || !isConst ) oss<<expression;
            else oss<<name; //<<"/*"<<expression<<"*/";
        }else{
            oss<<name;
        }
        if(parens) oss<<")";
        return oss.str();
    }
    std::string decl() const {
        std::string ret;
        //if(!isVar) { /*nop*/; } // only variables need be declared
        //else if(isVar && isConst && !isExpr){ /*nop*/; } // use VAL c text replacement
        if(isVar && !isExpr){
            std::ostringstream oss; oss<<type_string<VAL>()<<" "<<(isConst?"const ":"")<<name
                <<" = "<<c<<";"; ret=oss.str();}
        else if(isVar && isExpr){
            std::ostringstream oss;
            //oss<<type_string<VAL>()<<" "<<(isConst?"const ":"")<<name<<" = "<<rhs()<<";";
            //oss<<type_string<VAL>()<<" "<<(isConst?"const ":"")<<name<<" = "<<expr()<<";";
            oss<<type_string<VAL>()<<" "<<(isConst?"const ":"")<<name<<" = "<<expression<<";";
            ret=oss.str();}
        return ret;
    }
    C& operator=(C const& a)
    {
        //std::cout<<"<op=>"; std::cout.flush();
        //std::cout<<" a={"<<a.flags()<<a.c<<","<<a.name<<","<<a.expression<<",rhs="<<a.rhs()<<"}";
        assert(isVar);
        assert(!isConst);
        c = a.c;
        //parens = true;
        std::ostringstream oss;
        if( a.isExpr ){
            //std::cout<<"==E==";
            expression = a.rhs();
            isExpr = true;
            if( !isVar ) { /*std::cout<<"==v==";*/ name = vstr(); }
        }else if( a.isVar && !a.isConst){
            expression = a.name;
        }else {
            expression = a.rhs();
            isExpr=false;
        }
        //std::cout<<" ==> {"<<flags()<<c<<","<<name<<","<<expression<<"}  "; std::cout.flush();
        return *this;
    }
#define OSS(var,STUFF) do{ std::ostringstream oss; oss<<STUFF; var=oss.str(); }while(0)
    C& operator++() {
        assert(!isConst);
        OSS(expression,"++"<<name);
        return ++c;
    }
    friend C operator+(C const& a, C const& b)
    {
        C ret(a.c + b.c);
        //std::cout<<"op{"<<a.flags()<<"+"<<b.flags()<<"}"; std::cout.flush();
        if( !a.isVar && !a.isExpr && a.c == 0 ) {assert(a.isConst); ret.expression=b.rhs(); }
        else if( !b.isVar && !b.isExpr && b.c == 0 ) {assert(b.isConst); ret.expression=a.rhs(); }
        else{ std::ostringstream oss; oss<<a.rhs()<<"+"<<b.rhs(); ret.expression = oss.str(); }
        if( !(ret.isConst = a.isConst && b.isConst) ) ret.isExpr = true;
        return ret;
    }
    friend C operator+(C const& a, VAL const v) { return operator+(a,C(v)); }
    friend C operator+(VAL const v, C const& a) { return operator+(C(v),a); }
    friend C operator-(C const& a, C const& b)
    {
        C ret(a.c - b.c);
        if( !a.isVar && !a.isExpr && a.c == 0 ) {assert(a.isConst); ret.expression=b.rhs(); }
        else if( !b.isVar && !b.isExpr && b.c == 0 ) {assert(b.isConst); ret.expression=a.rhs(); }
        else{ std::ostringstream oss; oss<<a.rhs()<<"-"<<b.rhs(); ret.expression = oss.str(); }
        if( !(ret.isConst = a.isConst && b.isConst) ) ret.isExpr = true;
        return ret;
    }
    friend C operator-(C const& a, VAL const v) { return operator-(a,C(v)); }
    friend C operator-(VAL const v, C const& a) { return operator-(C(v),a); }
    friend C operator*(C const& a, C const& b)
    {
        C ret(a.c * b.c);
        if( a.isConst && !a.isExpr && a.c == 1 ) {assert(a.isConst); ret.expression=b.rhs(); }
        else if( b.isConst && !b.isExpr && b.c == 1 ) {assert(b.isConst); ret.expression=a.rhs(); }
        else{ std::ostringstream oss; oss<<a.rhs()<<"*"<<b.rhs(); ret.expression = oss.str(); }
        if( !(ret.isConst = a.isConst && b.isConst) ) ret.isExpr = true;
        return ret;
    }
    friend C operator*(C const& a, VAL const v) { return operator*(a,C(v)); }
    friend C operator*(VAL const v, C const& a) { return operator*(C(v),a); }
    friend C operator/(C const& a, C const& b)
    {
        C ret(a.c / b.c);
        if( b.isConst && !b.isExpr && b.c == 1 ) {assert(b.isConst); ret.expression=a.rhs(); }
        else{ std::ostringstream oss; oss<<a.rhs()<<"/"<<b.rhs(); ret.expression = oss.str(); }
        if( !(ret.isConst = a.isConst && b.isConst) ) ret.isExpr = true;
        return ret;
    }
    friend C operator/(C const& a, VAL const v) { return operator/(a,C(v)); }
    friend C operator/(VAL const v, C const& a) { return operator/(C(v),a); }
    /** forgets this->name in lieu of current value, this->c */
    VAL eval() {
        if( isVar ){
            std::ostringstream oss; oss << c; name = oss.str();
            parens = false;
            isVar = false;
        }
        return c;
    }
    std::string flags() const {
        std::ostringstream oss;
        oss<<(isConst? 'c':'-')
            <<(isVar?  'v':'-')
            <<(isExpr? 'e':'-')
            <<(parens? 'p':'-');
        return oss.str();
    }
    friend C par(C const& a) { C aa(a);
        if( a.isConst && !a.isVar ) /*nop*/ ;
        else aa.parens=true;
        return aa; }
private:
    friend class Mul<VAL>;
    friend class Add<VAL>;
    friend class Lin<VAL>;
    VAL c;
    std::string name;
    std::string expression;
    bool parens;
    bool isVar;
    bool isExpr;
    bool isConst;
    bool isCConst;
};

typedef Arith<int64_t> I64;

template<typename VAL> class Mul{
public:
	typedef Arith<VAL> C;
    Mul( C const& x, C const& a )
        : x(x), a(a)
    { if(x.isConst && !a.isConst) std::swap(this->a,this->x); }
    friend std::ostream& operator<<( std::ostream& os, Mul<VAL> const& mul ){
        os<<"Mul{"<<mul.x.flags()<<" "<<mul.x.n()<<", "<<mul.a.flags()<<" "<<mul.a.rhs()<<"} ";
        return os;
    }
private:
    friend class Lin<VAL>;
	C x; // prefer x NONCONST
    C a; // prefer a CONST
};
template<typename VAL> class Add{
public:
	typedef Arith<VAL> C;
    Add( C const& x, C const& a )
        : x(x), a(a)
    { if(x.isConst && !a.isConst) std::swap(this->a,this->x); }
    friend std::ostream& operator<<( std::ostream& os, Add<VAL> const& add ){
        os<<"Add{"<<add.x.flags()<<" "<<add.x.n()<<", "<<add.a.flags()<<" "<<add.a.rhs()<<"} ";
        return os;
    }
private:
    friend class Lin<VAL>;
	C x; // prefer x NONCONST
    C a; // prefer a CONST
};

template<typename VAL> class Lin{
public:
	typedef Arith<VAL> C;
    typedef Mul<VAL> M;
    //Linear( C const& init ): init(init), vmul() {}
    template<class ...Muls>
        Lin( C const& init, Muls... muls) : init(init), vmul({muls...}) { chk(); }
    template<class ...Muls>
        Lin( Muls... muls) : init(0), vmul(muls...) { chk(); }
    C getCoeff( C const& var ){ // get the multiplier 'a' for a nonconst 'x' var
        assert( !var.isConst && var.isVar );
        VAL coeff = 0;
        bool found=false;
        for(auto const& mul: vmul){
            if( !mul.x.isConst && mul.x.name == var.name ){
                assert(!found);
                found = true;
                assert( !mul.a.isExpr );
                coeff = mul.a.c;
            }
        }
        return coeff;
    }
    C getConst() {
        //C ret(init,NONCONST);
        //std::cout<<"ret0 "<<ret.flags()<<","<<ret.c<<","<<ret.name<<","<<ret.expression<<"   "; std::cout.flush();
        VAL more=0;
        for(auto const& mul: vmul){
            // isConst may be too restrictive
            if( mul.x.isConst && mul.a.isConst ){
                std::cout<<"+ more x{"<<mul.x.flags()<<","<<mul.x.c<<","<<mul.x.name<<","<<mul.x.expression<<"}";
                std::cout<<" * a{"<<mul.a.flags()<<","<<mul.a.c<<","<<mul.a.name<<","<<mul.a.expression<<"}";
                more += mul.x.c * mul.a.c;
                std::cout<<" = "<<more<<"    ";
            }
        }
        return init+more;
    }
    void chk() {
        assert( init.isConst );
        for(auto const& mul: vmul){
            assert(!mul.x.isExpr); // usually will be a "loop variable"
            assert(mul.a.isConst); // (only x can be a non-const variable ~ loop index)
            assert(!mul.a.isExpr); // (because haven't tested this case)
            // later: mul.a exists in scope of mul.x
        }
    }
    // ptr = p0 + (a-pw)*ih ---> pw*ih constant ?
    friend std::ostream& operator<<( std::ostream& os, Lin<VAL> const& lin ){
        os<<"Lin{"<<lin.init.flags()<<" "<<lin.init.n();
        for(auto const& mul: lin.vmul) os<<"\n\t+ "<<mul;
        os<<" }";
        return os;
    }
private:
    C init;
    std::vector<M> vmul;
};

/** \deprecated */
template<typename VAL>
class Define {
public:
    typedef Arith<VAL> C;
    Define( std::string name, C& c)
        : c(c), name(name),
        decl_statement( dcl(type_string<VAL>(),"",name)),
        assign_statement( ass(name, c.expr())) {}
    Define( std::string name, C&& c) // constr from tmp
        : c(c), name(name), decl_statement(), assign_statement() {
            decl_statement = dcl(type_string<VAL>(),"",name);
            assign_statement = ass(name, c.expr());
        }
    std::string decl() const { return decl_statement; }
    std::string assign() const { return assign_statement; }
    VAL v() const {return c.v();}
private:
    C c;
    std::string name;
    std::string decl_statement;
    std::string assign_statement;
};

}//arith::

// vim: sw=4 ts=4 et cindent ai cino=^=lg\:0,j1,(0,wW1 formatoptions=croql syntax=cpp.doxygen
#endif // __ARITH_HPP__
