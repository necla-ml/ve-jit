#include "asmfmt.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm> // max
#include <cstdlib>
#include <cassert>
//#include <cstdio> // FILE, ftell

using namespace std;

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

char const* const    AsmFmtCols::ws =" \t\r"; // NOT including '\n'
char const* const    AsmFmtCols::indent = "    ";
int const            AsmFmtCols::inwidth = 4;
int const            AsmFmtCols::opwidth = 12;
//std::streampos const AsmFmtCols::argwidth = 24;
int const            AsmFmtCols::argwidth = 24;

AsmFmtCols::AsmFmtCols()
    : a(new ostringstream()), written(false), of(nullptr)
{
    (*a) << left;
    a->fill(' ');
}
AsmFmtCols::AsmFmtCols( string const& fname )
    : a(new ostringstream()), written(false),
    of(fname.size()? new ofstream(fname, std::ios::out): (std::ofstream*)nullptr)
{
    //of->rdbuf()->pubsetbuf(charBuffer,BUFFER_SIZE); // opt
    if(of) (*of) <<"// auto-generated via AsmFmtCols!\n";
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
std::stack<std::string>::size_type AsmFmtCols::pop_scope(){
    auto sz = stack_undefs.size();
    if(sz) {
        assert( a != nullptr );
        (*a) << stack_undefs.top(); // #undef lines, no endl here
        stack_undefs.pop();
        --sz;
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
AsmFmtCols& AsmFmtCols::lab(string const& label){
    throw_if_written(this,__FUNCTION__);
    (*a) << label << ':' << endl;
    return *this;
}
AsmFmtCols& AsmFmtCols::ins(){
    throw_if_written(this,__FUNCTION__);
    (*a) << endl;
    return *this;
}
std::string trim(const std::string& str,
        const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
        const std::string& fill = " ",
        const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}
AsmFmtCols::AsmLine AsmFmtCols::parts(std::string const& instruction){
    // made this a STATIC function
    //throw_if_written(this,__FUNCTION__);
    int const v=1;
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
    define << setw(inwidth) << "#" << "define " << setw(10) << symbol << ' ' << subst;
    if( name.size() == 0 ){
        define << "\n";
    }else{
        auto const col = inwidth + 7 + std::max(symbol.size(),10UL) + 1 + subst.size();
        auto const wnt = inwidth + opwidth + argwidth;
        //cout<<"col="<<col<<" cf "<<wnt<<endl;

        if(col < wnt){
            define << setw(wnt - col) << "";
        }
        define << "/* " << name << " */\n";
    }
    return define.str();
}
AsmFmtCols& AsmFmtCols::def(std::string const& symbol, std::string const& subst, std::string const& name){
    if( stack_undefs.empty() ){
        (*a) << fmt_def(symbol, subst, name+" { BEG SCOPE");
        stack_undefs.push(fmt_undef(symbol, name+" } END SCOPE"));
    }else{
        (*a) << fmt_def(symbol, subst, name);
        stack_undefs.push(fmt_undef(symbol, name));
    }
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
    (*a) << fmt_undef(symbol, name);
    return *this;
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
        {{"counter","%s1"},{"counter_beg","0"},{"to","5"}};
        a.scope(block,"opt_block_name");
        a.ins("mpy %s0,%s1,%s2#FIRST;mpy %s0,%s1,%s2#multiline"); // multi-line asm
        a.ins("add %s0,%s1,%s2;add %s0,%s1,%s2","add twice");
        a.ins("sub %s0,%s1,%s2;sub %s0,%s1,%s2#DIFF","diff twice");
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
