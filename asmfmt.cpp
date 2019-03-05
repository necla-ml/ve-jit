#ifndef ASMFMTREMOVE
#define ASMFMTREMOVE 0
#endif
#include "asmfmt.hpp"
#include "throw.hpp"
//#include "codegenasm.hpp"
#include "stringutil.hpp"
#include "jitpage.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
//#include <stdexcept>
#include <algorithm> // max
//#include <cstdlib>
//#include <cassert>
#include <array>
//#include <cstdio> // FILE, ftell

using namespace std;

#if ASMFMTREMOVE < 2
string fname_bin( string const& fname_S ){
    auto len = fname_S.size();
    if( len >= 2 && fname_S.substr(len-2) != ".S" ){
        throw runtime_error(" assembler filename " + fname_S + " should end in .S");
    }
    return string{fname_S.substr(0,len-2) + ".bin"};
}

size_t asm2bin( string const& fname_S, int const verbose/*=1*/ ){
    if(verbose>1) cout<<" Running asm2bin from " __FILE__<<endl;
    string const fbin = fname_bin( fname_S );
    string mk_cmd("make -f bin.mk " + fbin);
    if(verbose==0) mk_cmd.insert(0,"VERBOSE=0 ");
    if(verbose>1) cout << "cmd: " << mk_cmd << endl;
    int status = system( mk_cmd.c_str() );

    if( status == 0 ){
        if(verbose) cout << "     asm2bin(" << fname_S << ") DONE" << endl;
    }else{
        if(verbose) cout<<" Ohoh: make command returned status "<<status<<endl; cout.flush();
        throw runtime_error("asm2bin( \"" + fname_S + "\" ) failed, status=" + to_string(status));
    }
    // We return the filesize in bytes, so we know how big a jit page we need
    FILE* f_bin = fopen(fbin.c_str(),"rb");
    long f_bin_bytes = 0L;
    if( f_bin==NULL ){
        throw runtime_error("Could not open " + fbin + " (from " + fname_S + ") for reading");
    }else{
        fseek(f_bin,0,SEEK_END);
        f_bin_bytes = ftell(f_bin);
    }
    if( f_bin_bytes == 0L ){
        throw runtime_error(fbin + " (from " + fname_S + ") had zero bytes");
    }
    if(verbose) cout<<" ftell says "<<f_bin_bytes<<" bytes in file "<<fbin<<endl;
    fclose(f_bin);
    return f_bin_bytes;
}

ExecutablePage::ExecutablePage( std::string const& fbin )
: page({nullptr,0,verbosity}) // verbosity is a static const member, see asmfmt_fwd.hpp
{
    auto len = fbin.size();
    if( len >= 4 && fbin.substr(len-4) != ".bin" ){
        throw runtime_error(" binary blob filename " + fbin + " should end in .bin");
    }
    string basename{fbin.substr(0,len-4)};

    JitPage *p = const_cast<JitPage*>(&this->page);
    bin2jitpage(basename.c_str(), p, verbosity);
    if( p->mem == nullptr ){
        throw runtime_error(" Problems getting code page for " + fbin);
    }
    jitpage_readexec(p); // ignore errors here.
}
ExecutablePage::~ExecutablePage(){
    int status = jitpage_free(const_cast<JitPage*>(&this->page));
    if(status){
        throw runtime_error(" Problems releasing ExecutablePage");
    }
}
#endif //ASMFMTREMOVE < 2

#if ASMFMTREMOVE < 1
char const* const    AsmFmtCols::ws =" \t\r"; // NOT including '\n'
char const* const    AsmFmtCols::indent = "    ";
int const            AsmFmtCols::inwidth = 4;
int const            AsmFmtCols::opwidth = 12;
//std::streampos const AsmFmtCols::argwidth = 24;
int const            AsmFmtCols::argwidth = 44;

AsmFmtCols::AsmFmtCols()
    : a(new ostringstream()), written(false), of(nullptr)
      //, stack_undefs{std::string("")}, stack_defs{std::string("")}
      , stack_undefs(), stack_defs(), parent(nullptr)
{
    (*a) << left;
    a->fill(' ');
}
AsmFmtCols::AsmFmtCols( string const& fname )
    : a(new ostringstream()), written(false)
      , of(nullptr), stack_undefs(), stack_defs(), parent(nullptr)
{
    //of->rdbuf()->pubsetbuf(charBuffer,BUFFER_SIZE); // opt
    if(fname.size()){
        of = new ofstream(fname, std::ios::out);
        (*of) <<"// auto-generated via AsmFmtCols!\n";
    }
    (*a) << left;
    a->fill(' ');
}
AsmFmtCols::~AsmFmtCols(){
    //std::cout<<"~AsmFmtCols,"<<stack_undefs.size()<<"undefs "; std::cout.flush();
    while(stack_undefs.size()){
        //std::cout<<" destructor pop_scope"; std::cout.flush();
        this->pop_scope();
        //std::cout<<" destructor write"; std::cout.flush();
        //this->write();
    }
    if( !written ){
        //std::cout<<" destructor write"; std::cout.flush();
        this->write();
    }
    if(of){ of->flush(); of->close(); delete of; of = nullptr; }
    delete a;    a = nullptr;
}
inline void throw_if_written( AsmFmtCols const* asmfmt, string const& cannot ){
    if( asmfmt->written ){
        throw runtime_error( string{"AsmFmtCols already written - cannot "} + cannot );
    }
}
std::vector<std::string>::size_type AsmFmtCols::pop_scope(){
    auto sz = stack_undefs.size();
    if(sz) {
        assert( a != nullptr );
        (*a) << stack_undefs.back(); // #undef lines, no endl here
        --sz;
        stack_undefs.resize(sz);
        stack_defs.resize(sz);
        //if(sz==0) (*a)<<setw(inwidth+opwidth+argwidth-9)<<""
        //    <<"/* (end scope) */\n";
        written = false;
    }
    return sz;
}
void AsmFmtCols::pop_scopes(){
    while(stack_undefs.size()){
        this->pop_scope();
    }
}
std::string AsmFmtCols::flush(){
    // \pre a is not null
    string ret("// AsmFmtCols empty!");
    if(a){
        ret = (*a).str();
        if( of && a ){
            try{
                //cout<<"AsmFmtCols::write-->of"<<endl;
                //(*of) << a->rdbuf(); // not working? ???
                (*of) << ret;
                of->flush();
                //of->close();
            }catch(...){
                cout<<" ERROR: could not write to output file!"<<endl;
            }
            //delete of;  of = nullptr;
            a->str("");
            a->clear();
        }
        this->written = true;
        //delete a; a = nullptr;
    }
    return ret;
}
void AsmFmtCols::write(){
    throw_if_written(this,__FUNCTION__);
    if( of ){
        //cout<<"AsmFmtCols::write-->of"<<endl;
        (*of) << a->rdbuf();
        of->flush();
        //of->close();
    }else{
        //cout<<"AsmFmtCols::write-->cout"<<endl;
        //(*a) << "\n// Goodbye\n";
        cout << a->str(); //a->rdbuf(); <-- for stringstream?
        cout.flush();
        //cout<<"AsmFmtCols::write-->cout DONE"<<endl;
    }
    this->written = true;
    // reset ostringstream for further output (like from pop_scope)
    a->clear();
    a->str("");
    // There is valid reason for allowing pop_scope to run during the destructor
    //delete of;  of = nullptr;
    //delete a;   a  = nullptr;
}
AsmFmtCols& AsmFmtCols::raw(string const& anything){
    throw_if_written(this,__FUNCTION__);
    (*a) << anything << endl;
    return *this;
}
AsmFmtCols& AsmFmtCols::lcom(string const& comment){
    throw_if_written(this,__FUNCTION__);
    (*a) << indent << "// " << comment << endl;
    return *this;
}
AsmFmtCols& AsmFmtCols::com(string const& comment){
    throw_if_written(this,__FUNCTION__);
    (*a) << setw(inwidth+opwidth+3) << right << "// " << left << comment << endl;
    return *this;
}
AsmFmtCols& AsmFmtCols::rcom(string const& comment){
    throw_if_written(this,__FUNCTION__);
    (*a) << setw(inwidth+opwidth+argwidth+3) << right << "// " << left << comment << endl;
    return *this;
}
AsmFmtCols& AsmFmtCols::lab(string const& label, std::string const& comment/*=""*/){
    throw_if_written(this,__FUNCTION__);
    (*a) << label << ':';
    if(!comment.empty()) (*a) << " # " << comment;
    (*a) << endl;
    return *this;
}
AsmFmtCols& AsmFmtCols::ins(){
    throw_if_written(this,__FUNCTION__);
    (*a) << endl;
    return *this;
}
AsmFmtCols::AsmLine AsmFmtCols::parts(std::string const& instruction){
    // made this a STATIC function
    //throw_if_written(this,__FUNCTION__);
    int const v=0;
    AsmLine ret;
    if(v)cout<<"parts.."<<instruction<<endl;
#if 0
    auto inst = instruction;
    assert( string(" nop").find_first_not_of(ws,0) == 1 );
    assert( string("nop").find_first_not_of(ws,0) == 0 );
    auto opbeg = inst.find_first_not_of(ws,0);
    if( opbeg == string::npos ){
        if(v)cout<<"DONE"<<endl;
        return ret;
    }
#else
    auto inst = reduce(trim(instruction)); // again?
    auto opbeg = inst.find_first_not_of(" \t\r\n",0);
    if( opbeg == string::npos ){
        if(v)cout<<"DONE"<<endl;
        return ret;
    }
#endif
    if(v)cout<<"     .."<<inst<<endl;
    //
    // TODO: comment "//...\n"
    auto opend = inst.find_first_of(" \t\r#\n;",opbeg); // white or comment or statement-end
    auto opend2 = inst.find("//", opbeg); // another type of possible ending
    opend = min(opend,opend2);

    if(v)cout<<" op"<<opbeg<<":"<<opend;
    if( opend == string::npos ){
        ret.op = inst.substr(opbeg);
        if(v)cout<<" inst="<<inst<<" ["<<opbeg<<",npos] ret.op="<<ret.op<<" DONE"<<endl;
    }else{
        ret.op = inst.substr(opbeg, opend-opbeg);
        //if(v)cout<<" op<"<<ret.op<<">";
        if(v)cout<<" inst="<<inst<<" ["<<opbeg<<",opend) ret.op="<<ret.op<<" DONE"<<endl;
        auto iargbeg = inst.find_first_not_of(ws,opend);
        if (iargbeg != string::npos){
            auto iargend = inst.find_last_not_of(ws);
            if(v)cout<<" args"<<iargbeg<<":"<<iargend;
            ret.args = inst.substr(iargbeg, iargend-iargbeg+1);
            if(v)cout<<"<"<<ret.args<<">";
            // currently ret.args is "everything else",
            // new: ret.args needs some massaging:
            //      after ';' --> remain,
            //  and after '#' (before ';' if any) --> comment
            // Suppose comments are not allowed to contain semicolons
            iargend = string::npos;
            auto isemicolon = ret.args.find_first_of(";\n",0);
            if( isemicolon != string::npos ){
                iargend = isemicolon;
                auto iback = ret.args.find_last_not_of(ws);
                ret.remain = ret.args.substr( isemicolon+1, iback-isemicolon );
                if(v) cout<<" remain"<<isemicolon+1<<":"<<iback<<" "<<ret.remain<<endl;
            }
            auto icomment = ret.args.find_first_of("#",0);
            auto icomment2 = ret.args.find("//",0); // length of comment mark is 2, not 1
            icomment = min(icomment,icomment2);
            if( icomment != string::npos && icomment < isemicolon ){
                // side effect is that we FORGET the comment type (we generate only '#' comments for now XXX)
                iargend = icomment;
                ret.comment = ret.args.substr( icomment+(icomment==icomment2? 2: 1), isemicolon-icomment-1 );
                if(v) cout<<" comment"<<icomment+1<<":"<<isemicolon-1<<" "<<ret.comment<<endl;
            }
            // chop any comment or multi-line asm
            if( iargend != string::npos ){
                ret.args = ret.args.substr(0,iargend);
                if(v) cout<<" final ret.args "<<0<<":"<<iargend<<" "<<ret.args<<endl;
                // chop ws again
                iargend = ret.args.find_last_not_of(ws);
                if( iargend != string::npos ){
                    ret.args = ret.args.substr(0,iargend+1);
                }
            }
        }
    }
    if(v) cout<<" final ret.label   <"<<ret.label<<">"<<endl;
    if(v) cout<<" final ret.op      <"<<ret.op<<">"<<endl;
    if(v) cout<<" final ret.args    <"<<ret.args<<">"<<endl;
    if(v) cout<<" final ret.comment <"<<ret.comment<<">"<<endl;
    if(v) cout<<" final ret.remain  <"<<ret.remain<<">"<<endl;
    if(v)cout<<" DONE"<<endl;
    return ret;
}
AsmFmtCols& AsmFmtCols::ins(string const& instruction){
    throw_if_written(this,__FUNCTION__);
    // for multiline, we need a bit more than just 'parts(instruction)'
    std::string remain = instruction;
    //cout<<" remain = "<<remain<<endl;
    while( remain.size() ){
        auto const p = parts(remain);
        if(p.op.size() == 0){
            ins();
        }else{
            (*a) << left << indent;
            if(p.args.empty()){
                if( p.comment.empty() ){
                    (*a) << p.op << endl;
                }else{
                    (*a) << setw(opwidth-1) << p.op << " "
                        << setw(argwidth) << "";
                }
            }else{
                (*a) << setw(opwidth-1) << p.op << " ";
                if( p.comment.empty() )
                    (*a) << p.args <<endl;
                else
                    (*a) << setw(argwidth-1) << p.args << " ";
            }
            if( p.comment.size() )
                (*a) << "# " << p.comment << endl;
        }
        remain = p.remain;
        //cout<<" iterate: remain = "<<remain<<endl;
    }
    return *this;
}
AsmFmtCols& AsmFmtCols::ins(string const& instruction, string const& asmcomment){
    throw_if_written(this,__FUNCTION__);
    if(asmcomment.empty() || asmcomment.find_first_not_of(ws) == string::npos){
        ins(instruction);
        return *this;
    }
    bool did_asmcomment = false;
    std::string remain = instruction;
    std::string comment;
    while( remain.size() ){
        auto const p = parts(remain);
#if 0
        (*a) << left;
        if(p.op.size() == 0){
            (*a) << setw(inwidth+opwidth+argwidth) << "";
        }else{
            (*a) << indent;
            if(p.args.size() == 0){
                (*a) << setw(opwidth+argwidth) << p.op;
            }else{
                (*a) << setw(opwidth-1) << p.op << " "
                    << setw(argwidth-1) << p.args << " ";
            }
        }
        (*a) << "#  " << asmcomment <<endl;
#elif 1
        comment = p.comment;
        if( p.remain.empty() && p.comment.empty() ){
            comment = asmcomment;
            did_asmcomment = true;
            // if final instruction comes with a comment, then
            // push asmcomment to a separate line (as an rcom)
        }
        if(p.op.empty()){
            assert( p.args.empty() );
            if( p.comment.empty() )
                ins(); // blank line
            else
                (*a) << setw(inwidth+opwidth+argwidth) << "";
        }else{
            (*a) << left << indent;
            if(p.args.empty()){
                if( comment.empty() )
                    (*a) << p.op << endl;
                else
                    (*a) << setw(opwidth+argwidth-1) << p.op << " ";
            }else{
                (*a) << setw(opwidth-1) << p.op << " ";
                if( comment.empty() )
                    (*a) << p.args <<endl;
                else
                    (*a) << setw(argwidth-1) << p.args << " ";
            }
        }
        if( comment.size() ){
            (*a) << "# " << comment << endl;
        }
#endif
        remain = p.remain;
        //cout<<" iterate: remain = "<<remain<<endl;
    }
    if( !did_asmcomment )
        rcom(asmcomment);
#if 0
    // tellp also seemed buggy on Aurora
    //auto const sz0 = a->tellp();
    ins(instruction, false/*endline*/);
    cout<<"<continuing>"; cout.flush();
    (*a) << "<COMM>";
    //auto const sz1 = a->tellp();
    //auto const commcol = 4 + opwidth + argwidth;
    //if(sz1 < sz0 + commcol) (*a) << setw(commcol-(sz1-sz0)); // reposition comment?
    (*a) << "# " << asmcomment << endl;
#endif
    return *this;
}
string AsmFmtCols::fmt_def(std::string const& symbol, std::string const& subst, std::string const& name){
    std::ostringstream define;
    define << "#define " << setw(10) << symbol << ' ' << subst;
    //         ---8 ---
    if( name.size() == 0 ){
        define << "\n";
    }else{
        auto const col = 8 + std::max(symbol.size(),10UL) + 1 + subst.size();
        auto const wnt = inwidth + opwidth + argwidth - 9;
        if(col < wnt){
            define << setw(wnt - col) << "";
        }
        define << "/* " << name << " */\n";
    }
    return define.str();
}
void AsmFmtCols::StringPairs::push_trimmed(std::string name, std::string subst){
    push_back( std::make_pair(
                trim(name,  std::string(" \n\t\0",4)),
                trim(subst, std::string(" \n\t\0",4))
                ));
}
/** adds to 'global' scope, creating if nec. */
AsmFmtCols& AsmFmtCols::def(std::string const& symbol, std::string const& subst, std::string name){
    std::string sub = trim(subst, string(" \n\t\0",4));
    assert( stack_defs.size() == stack_undefs.size() );
    std::string namd = name;
    if( stack_undefs.empty() ){
        stack_defs.push_back(StringPairs());
        stack_undefs.push_back("");
        namd.append(" { GLOBAL SCOPE");
    }
    std::string define = fmt_def(symbol, sub, namd);
    (*a) << define;
    stack_undefs[0].append(fmt_undef(symbol,name));
    stack_defs  [0].push_trimmed(symbol, subst);
    return *this;
}
string AsmFmtCols::fmt_undef(std::string const& symbol,std::string const& name){
    string macroname = trim(symbol);
    auto end = macroname.find_first_of(" \n\t(");
    macroname = macroname.substr(0,end);
    ostringstream undef;
    if(name.empty()){
        undef << "#undef  " << macroname << "\n";
    }else{
        auto const wnt = inwidth + opwidth + argwidth - 9;
        undef << "#undef  " << left << setw(wnt-8) << macroname << "/* " << name << " */\n";
    }
    return undef.str();
}
AsmFmtCols& AsmFmtCols::undef(std::string const& symbol,std::string const& name){
    cerr<<" Warning: AsmFmtCols::undef("<<symbol<<","<<name<<") is deprecated"<<endl;
    (*a) << fmt_undef(symbol, name);
    // XXX remove from stack_[un]defs[0]
    return *this;
}
std::vector<string> AsmFmtCols::def_words_starting(std::string with) const {
    std::vector<string> ret;
    if(parent)
        ret = parent->def_words_starting(with);
    size_t with_sz = with.size();
    for(auto const& vmac: stack_defs){
        for(auto const& d: vmac){                // d ~ vector<symbol,subst>
            std::string const& s = d.second;
            if(s.compare(0,with_sz, with) == 0){
                ret.push_back(s);
            }
        }
    }
    //cout<<" found "<<ret.size()<<" substitution-first-words beginning with "<<with<<endl;
    return ret;
}
vector<pair<string,string>> AsmFmtCols::def_macs_starting(std::string with) const {
    vector<pair<string,string>> ret;
    if(parent)
        ret = parent->def_macs_starting(with);
    size_t with_sz = with.size();
    for(auto const& vmac: stack_defs){
        for(auto const& d: vmac){                // d ~ vector<symbol,subst>
            std::string const& s = d.second;
            if(s.compare(0,with_sz, with) == 0){
                //ret.push_back(static_cast<const pair<string,string>&>(d));
                ret.push_back(d);
            }
        }
    }
    //cout<<" found "<<ret.size()<<" substitution-first-words beginning with "<<with<<endl;
    return ret;
}
std::string AsmFmtCols::defs2undefs( StringPairs const& macs, std::string block_name ){
    AsmFmtCols undefs;
    {
        std::string comment;
        std::string end_comment = "} END ";
        end_comment.append(block_name);
        auto const mend = macs.crend();
        auto       mdef = macs.crbegin();
        for( ; mdef != mend; ){
            auto macro = mdef->first;   // the macro name (and args!?)
            ++mdef;
            if( mdef == mend ){
                comment = end_comment;
            }
            (*undefs.a)<<fmt_undef(macro,comment);
        }
        if(comment.empty()){
            rcom(end_comment);
        }
    }
    return undefs.flush(); // equiv str() + clear
}
std::string uncomment_asm( std::string asmcode )
{
    // for multiline, we need a bit more than just 'parts(instruction)'
    std::string remain = asmcode;
    //cout<<" remain = "<<remain<<endl;
    std::ostringstream oss;
    while( remain.size() ){
        auto const p = AsmFmtCols::parts(remain);
        if(p.op.size() == 0){
            oss<<endl; /* reproduce blank lines*/
        }else{
            // ignore p.comment
            if(p.args.empty()){
                oss << p.op << endl;
            }else{
                oss << p.op << ' ' << p.args <<endl;
            }
        }
        remain = p.remain;
        //cout<<" iterate: remain = "<<remain<<endl;
    }
    return oss.str();
}
std::string ve_load64_opt0(std::string s, uint64_t v){
    uint32_t const vlo = uint32_t(v);
    uint32_t const vhi = uint32_t(uint32_t(v>>32) + ((int32_t)v<0? 1: 0));
    std::ostringstream oss;
    bool const is31bit = ( (v&(uint64_t)0x000000007fffFFFFULL) == v );
    char const * comment=" sign-extended ";
    if( is31bit                                 // 31-bit v>=0 is OK for lea
            || ( (int)vlo<0 && (int)vhi==-1 ))  // if v<0 sign-extended int32_t, also happy
    {
        if( is31bit ) comment=" ";
        oss <<"\tlea    "<<s<<", "<<jithex(vlo)<<"\n";
    }else{
        oss <<"\tlea    "<<s<<", "<<jithex(vlo)<<"\n"
            <<"\tand     "<<s<<", "<<s<<", (32)0\n"
            <<"\tlea.sl "<<s<<", "<<jithex(vhi)<<"(,"<<s<<")";
        comment=" ve_load64_opt0 ";
    }
    oss<<" #"<<comment<<s<<" = "<<jithex(v);
    return oss.str();
}
std::string ve_signum64(std::string out, std::string in){
    ostringstream oss;
    oss<<"sra.l "<<out<<","<<in<<",63";
    return oss.str();
}
std::string ve_abs64(std::string out, std::string in){
   ostringstream oss;
   /*
     Found a 2-operation program:
     sub   r1,0,rx
     max   r2,r1,rx
     Expr: max((0 - x), x)

     Found a 2-operation program:
     sub   r1,0,rx
     cmovgt r2,r1,r1,rx
     Expr: cmovgt((0 - x), (0 - x), x)

     Found a 2-operation program:
     sub   r1,0,rx
     cmovlt r2,rx,r1,r1
     Expr: cmovlt(x, (0 - x), (0 - x))
    */
    //oss<<"subs.l "<<out<<",0/*I*/,"<<in;
    //oss<<"; maxs.l "<<out<<","<<out<<","<<in;
    oss<<"sub "<<out<<",0,"<<in
        <<"; cmovgt "<<out<<","<<out<<","<<in; // VE operation is: Sx = (Sy>0? Sx: Sz)
    return oss.str();
}
void ve_set_base_pointer( AsmFmtCols & a, std::string bp/*="%s34"*/, std::string name/*="foo"*/ ){
    a.def("STR0(...)", "#__VA_ARGS__")
        .def("STR(...)",  "STR0(__VA_ARGS__)")
        .def("CAT(X,Y)", "X##Y")
        .def("FN",name)             // func name (characters)
        .def("FNS", "\""+name+"\"") // quoted-string func name
        .def("L(X)", "CAT("+name+"_,X)")
        .def("BP",bp,            "base pointer register")
        ;
        // macros for relocatable branching
    a.def("REL(YOURLABEL)", "L(YOURLABEL)-L(BASE)(,BP)", "relocatable address of 'here' data")
        ;
        // The following is needed if you use branches
    a.ins("sic   BP","%s1 is *_BASE, used as base ptr")
        .lab("L(BASE)")     // this label is reserved
        .rcom("Now a relative jump looks like:")
        .rcom("   ins(\"b.l REL(JumpAddr)\")")
        .rcom("   lab(\"L(JumpAddr)\")")
        ;
}
/** VE lea logic */
static void opLoadreg_lea(OpLoadregStrings& ret, uint64_t const parm){
    uint32_t const hi = ((parm >> 32) & 0xffffFFFF); // unsigned shift-right
    uint32_t const lo = (uint32_t)parm;              // 32 lsbs (trunc)
    bool is31bit = ( (parm&(uint64_t)0x000000007fffFFFF) == parm );
    //bool is32bit = ( (parm&(int64_t) 0x00000000ffffFFFF) == parm );
    bool hiOnes  = ( (int)hi == -1 );
    if(is31bit){
        //KASE(1,"lea 31-bit",parm == (parm&0x3fffFFFF));
        ret.lea="lea OUT, "+(lo<=1000000? jitdec(lo): jithex(lo))
            +"# load: 31-bit";
    }else if((int)lo < 0 && hiOnes){
        assert( (int64_t)parm < 0 ); assert((int)lo < 0);
        //KASE(2,"lea 32-bit -ve",parm == (uint64_t)(int64_t)(int32_t)lo);
        ret.lea="lea OUT, "+((int64_t)parm >= -100000? jitdec((int32_t)lo): jithex(lo))
            +"# load: sext(32-bit)";
    }else if(lo==0){
        //KASE(3,"lea.sl hi only",lo == 0);
        ret.lea="lea.sl OUT, "+jithex(hi)+"# load: hi-only";
    }
    //
    // 2-instruction lea is always possible (TODO mixed instruction types)
    //
    //uint64_t tmplo = (int64_t)(int32_t)lo;
    // lea.sl OUT, <hi or hi+1> (,TMP)
    //        -ve lo will fill hi with ones i.e. -1
    //        so we add hi+1 to MSBs to restore desired sums
    uint64_t dd = ((int32_t)lo>=0? hi: hi+1);
    //uint64_t tmp2 = tmphi << 32;    // (sext(D,64)<<32)
    //uint64_t out = tmp2 + tmplo;     // lea.sl lea_out, tmphi(,lea_out);
    //assert( parm == out );
    // Changed:  do not use a T0 tmp register
    string comment("# ");
    comment += (dd!=hi?"Xld ":" ld ")+jithex(parm);
    //ret.lea2="lea OUT, "+jithex(lo)+comment; // VE expr size error?
    ret.lea2 = "lea OUT, ";
    ret.lea2 += jithex(lo);
    ret.lea2 += comment;
    ret.lea2 += " ; lea.sl OUT, "+jithex(dd)+"(,OUT)";
}
static void opLoadreg_log(OpLoadregStrings& ret, uint64_t const parm){
    /** VE bit ops logic */
    uint64_t const lo7 = (parm & 0x7f);              // 7 lsbs (trunc)
    int64_t  const sext7 = (int64_t)(lo7<<57) >> 57; // 7 lsbs (trunc, sign-extend)
    // simple cases: I or M zero
    bool isI = (int64_t)parm >= -64 && (int64_t)parm <= 63; // I : 7-bit immediate
    bool isM = isMval(parm); // M : (m)B 0<=m<=63 B=0|1 "m high B's followed by not-B's"
    if(isI){
        //KASE(4,"or OUT,"+jitdec(sext7)+",(0)1", parm==sext7);
        ret.log="or OUT, "+jitdec(sext7)+",(0)1 # load: small I";
    }else if(isM){
        //KASE(5,"or OUT,0,"+jitimm(parm), isMval(parm));
        ret.log="or OUT, 0,"+jitimm(parm)+"# load: (M){0|1}";
    }
    //if(isMval(parm&~0x3f)){...}
    // KASE 6 subsumed by KASE 9 (equivalent I, M)
    else if(isMval(parm|0x3f)){
        assert( isMval(parm^sext7) ); // more general, but this is more readable
        uint64_t const ival = (~parm &0x3f);
        uint64_t const mval = parm|0x3f;
        //KASE(7,"xor OUT, ~parm&0x3f "+jithex(ival)+", parm|0x3f "+jitimm(mval),
        //        parm == (ival ^ mval) && isIval(ival) && isMval(mval));
        ret.log="xor OUT, ";
        ret.log += jithex(ival);
        ret.log += ",";
        ret.log += jitimm(mval);
        ret.log += "# load: xor(I,M)";
    }else if(isMval(parm^sext7) ){ // xor rules don't depend on sign of lo7
        // if A = B^C, then B = A^C and C = B^A (Generally true)
        //int64_t const ival = sext7;
        uint64_t const mval = parm^sext7;
        //KASE(9,"xor OUT, "+jithex(ival)+","+jitimm(mval),
        //        parm == (ival ^ mval) && isIval(ival) && isMval(mval));
        ret.log="xor OUT, ";
        ret.log += jitdec(sext7);
        ret.log += ",";
        ret.log += jitimm(mval);
        ret.log += "# load: xor(I,M)";
    }
    if(!ret.log.empty()) ret.log.append(" --> "+jithex(parm));
}
/** Find possible 64-bit VE scalar register load instructions.
 * \return instruction string[s] to load a 64-bit scalar VE register.
 * You can supply such strings into AsmFmtCols for nice formatting.
 * Within the returned strings, \c OUT is the output register.
 * no temp register is used. */
OpLoadregStrings opLoadregStrings( uint64_t const parm )
{
    OpLoadregStrings ret;
    opLoadreg_lea(ret, parm); // get lea and lea2 strings
    opLoadreg_log(ret, parm); // get or/xor (logical) loads

    { // shift left
        // search for an unsigned right-shift that can regenerate parm
        int oksr=0;
        for(int sr=1; sr<64; ++sr){
            if( parm == (parm>>sr<<sr) && isMval(parm>>sr) ){
                oksr=sr; // use smallest shift
                break;
            }
        }
        if(oksr){
            uint64_t mval = parm >> oksr;
            //KASE(19,"sll OUT,"+jitimm(mval)+","<<jitdec(oksr),
            //        parm == (mval << oksr) && isMval(mval));
            ret.shl="sll OUT, "+jitimm(mval)+","+jitdec(oksr);
        }
    }
    { // arithmetic ops (+,-)
        if(ret.ari.empty()) for( int64_t ival = 0; ret.ari.empty() && ival<=63; ++ival ){
            // P = I + M <===> M == P - I
            uint64_t const mval = parm - (uint64_t)ival;
            if( isMval(mval) ){
                //KASE(20,"addu.l OUT,"+jitdec(ival)+", "+jitimm(mval),
                //        parm == ival + mval && isIval(ival) && isMval(mval));
                ret.ari="addu.l OUT, ";
                ret.ari += jitdec(ival);
                ret.ari += ",";
                ret.ari += jitimm(parm-ival);
                ret.ari += "# load: add(I,M)";
                break;
            }
        }
        if(ret.ari.empty()) for( int64_t ival = -1; ret.ari.empty() && ival>=-64; --ival ){
            uint64_t const mval = parm - (uint64_t)ival;
            if( isMval(mval) ){
                //KASE(21,"addu.l OUT,"+jitdec(ival)+","+jitimm(mval),
                //        parm == ival + mval && isIval(ival) && isMval(mval));
                ret.ari="addu.l OUT, ";
                ret.ari += jitdec(ival);
                ret.ari += ",";
                ret.ari += jitimm(parm-ival);
                ret.ari += "# load: add(I,M)";
                break;
            }
        }
        // Q; Is it correct that SUB (unsigned subtract) still
        //    sign-extends the 7-bit Immediate in "Sy" field?
        if(ret.ari.empty()) for( int64_t ival = 0; ret.ari.empty() && ival<=63; ++ival ){
            // P = I - M <==> M = I - P
            uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
            if( isMval(mval) ){
                //uint64_t out = (uint64_t)ival - (uint64_t)mval;
                //KASE(22,"subu.l OUT,"+jitdec(ival)+","+jitimm(mval),
                //        parm == out  && isIval(ival) && isMval(mval));
                ret.ari="subu.l OUT, ";
                ret.ari += jitdec(ival);
                ret.ari += ",";
                ret.ari += jitimm(mval);
                ret.ari += "# load: subu(I,M)";
                break;
            }
        }
        if(ret.ari.empty()) for( int64_t ival = -1; ret.ari.empty() && ival>=-64; --ival ){
            uint64_t const mval = (uint64_t)ival - (uint64_t)parm;
            if( isMval(mval) ){
                //uint64_t out = (uint64_t)ival - (uint64_t)mval;
                //KASE(23,"subu.l OUT, "+jitdec(ival)+","+jitimm(mval),
                //        parm == out  && isIval(ival) && isMval(mval));
                ret.ari="subu.l OUT, ";
                ret.ari += jitdec(ival);
                ret.ari += ",";
                ret.ari += jitimm(mval);
                ret.ari += "# load: subu(I,M)";
                break;
            }
        }
        if(!ret.ari.empty()) ret.ari.append(" --> "+jithex(parm));
    }
    return ret;
}

std::string choose(OpLoadregStrings const& ops, void* /*context=nullptr*/)
{
    enum Optype { SHL, LEA, LOG, ARI, LEA2 };
    static std::array<Optype, 5> const pref = {SHL, LEA, LOG, ARI, LEA2};
    string code;
    for(auto const optype: pref){
        code = (optype==LEA? ops.lea
                :optype==LOG? ops.log
                :optype==SHL? ops.shl
                :optype==ARI? ops.ari
                : ops.lea2);
        if( !code.empty() ){
            break;
        }
    }
    return code;
}
std::string ve_load64(std::string s, uint64_t v){
    return multiReplace("OUT", s,
            choose(opLoadregStrings(v)));
}

/** /todo return {var,subst} PAIRS, because same_name and same_reg
 *       might be nice to coagulate (presume identical content) */
void ve_propose_reg( std::string variable, AsmScope& block, AsmFmtCols const& a,
        std::string prefix,
        std::vector<std::pair<int,int>> const search_order){
    auto vs = a.def_words_starting(prefix); // anything already within scopes?
    for(auto const& pre: block){ // for things we ARE GOING TO allocate
        vs.push_back( pre.second );
    }
    std::string register_name = free_pfx(vs,prefix,search_order);
    if(register_name.empty()) THROW("Out of registers");
    block.push_back({variable,register_name});
}

// VE note: %s12 : "Outer register, used for pointing start addr of called fn"
//          %s13 : "Used to pass id of function to dynamic linker"
//          are both not preserved in C ABI
static std::vector<std::pair<int,int>> const scalar_order_fwd{{0,7},{34,63},{12,13},{18,33}};
static std::vector<std::pair<int,int>> const scalar_order_bwd{{63,34},{7,0},{13,12},{33,18}};
static std::vector<std::pair<int,int>> const vector_order_fwd{{0,63}};
static std::vector<std::pair<int,int>> const vector_order_bwd{{63,0}};

void ve_propose_reg( std::string variable, AsmScope& block, AsmFmtCols const& a, RegSearch const how ){
    std::vector<std::pair<int,int>> const* order = nullptr;
    char const* pfx = nullptr;
    switch(how){
      case(SCALAR)    : pfx="%s"; order=&scalar_order_fwd; break;
      case(SCALAR_TMP): pfx="%s"; order=&scalar_order_bwd; break;
      case(VECTOR)    : pfx="%v"; order=&vector_order_fwd; break;
      case(VECTOR_TMP): pfx="%v"; order=&vector_order_bwd; break;
    }
    assert( order != nullptr );
#if 0
    auto vs = a.def_words_starting(pfx); // anything already within scopes?
    for(auto const& pre: block){ // for things we ARE GOING TO allocate
        vs.push_back( pre.second );
    }
    std::string register_name = free_pfx(vs,pfx,*order);
    if(register_name.empty()) THROW("Out of registers");
    block.push_back({variable,register_name});
#else
    ve_propose_reg( variable, block, a, pfx, *order );
#endif
}
#endif //CODEREMOVE < 1

#ifdef _MAIN
#include <iostream>
#include <forward_list>
using namespace std;
// demo how to use AsmFmtCols
int main(int,char**){
    {
        //AsmFmtCols a("demo-asmfmtcols.txt");
        AsmFmtCols a;
        a.lcom("lcom comment: This is NOT real assembler code");
        a.lcom("lcom comment: The goal is to pretty print assembler source code");
        a.com("'com' puts a medium length comment right here");
        a.rcom("rcom comment");
        a.def("xval","13");
        a.def("xreg","%s1");
        a.lab("set_x");
        a.ins("nop", "     \t  \t\t");
        a.ins("lea xreg, xval");
        a.ins("addu.l      xreg, 1,   xreg        ", "increment xreg");
        //a.ins("addu.l      xreg, 1,   xreg        ", "increment xreg", "once more");
        a.ins("addu.l      xreg, 1,   xreg        ", "increment xreg",
                "this is done",
                "once more so that",
                "I have now added 2");
        std::forward_list<std::pair<std::string,std::string>> block =
        {{"counter","%s1"},{"counter_beg","0"},{"to","5"},{"out","%s2"}};
        a.scope(block,"opt_block_name");
        a.ins("mpy %s0,%s1,%s2#FIRST;mpy %s0,%s1,%s2#multiline"); // multi-line asm
        a.ins("add %s0,%s1,%s2;add %s0,%s1,%s2","add twice");
        for(auto s: a.def_words_starting("%s")){
            cout<<"# def_words_starting... "<<s<<endl;
        }
        a.ins("sub %s0,%s1,%s2;sub %s0,%s1,%s2#DIFF","diff twice");
        a.def("CAT","DOG");
        a.pop_scope();
        a.ins("nop #1;nop#2","multiline nops A");
        a.ins("frobnicate X; foo bar#huh\nfrobnicate B\nfoo2 bar2;xyz\nfoo3#X;foo4;#long multiline");
        a.ins("nop\nnop#B","newline could also be used to separate asm statements");
        a.ins("baz\nbaz#A\nbaz#B","newline could also be used to separate asm statements");
        a.ins("cat\ndog//A\ncow//B","c++-style // comments");
        a.scope(block,"a second block (not explicitly popped)");
        a.lcom("For really long comments you can use 'lcom', which also"
                ,"can accept a list of long-line strings as arguments");
        a.ins("fence","this has no args and a comment");
        a.ins("fence","this is a useless","second copy of fence","with multiple comments");
        //a.scope(block,"opt_block_name");
        a.write();
        cout<<"pop_scopes..."<<endl;
        a.pop_scopes();
        a.write();
    }
    cout<<"\nGoodbye"<<endl;
}
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
