/** \file
 * C++ shared library bug -- original discovery.
 * much smaller testcase is in bug/bugN*.mk.
 */
#ifndef CODEREMOVE
#define CODEREMOVE 0
#endif
#if CODEREMOVE >= 9
#include <iostream>
using namespace std;
int main(int,char**){
    cout<<"\nGoodbye"<<endl;
}
#else // CODEREMOVE < 9
#include "throw.hpp"
#include "jitpage.h"    // low level 'C' utilities
#include <fstream>
#include <cstring>
#include <unistd.h>     // getcwd, sysconf, pathconf, access
#include <assert.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>           // std::unique_ptr
#ifndef NDEBUF
#include "throw.hpp"
#endif

#include <dlfcn.h>

/** single-symbol data. */
struct SymbolDecl{
    SymbolDecl(std::string symbol, std::string comment="", std::string fwddecl="")
        : symbol(symbol), comment(comment), fwddecl(fwddecl)
        {}
    std::string symbol;
    // opt.
    std::string comment;
    std::string fwddecl;
};
/** SubDir has create-writable or throw semantics, and set absolute path */
struct SubDir{
    SubDir() : subdir(), abspath() {}
    // create subdir, then set current dir, and absolute path to subdir
    SubDir(std::string subdir);
    std::string subdir;
    // calculated
    std::string abspath;    // = cwd/subdir
};
/** basename*.{c|cpp|s|S} compilable code file */
struct DllFile {
    DllFile() : basename(), suffix(), code(), syms(), comment(), objname(), abspath() {}
    std::string basename;
    std::string suffix;             ///< *.{c|cpp|s|S}
	std::string code;
    std::vector<SymbolDecl> syms;   ///< just the public API symbols
    // optional...
	std::string comment;
    /** write comment+code to <subdir.abspath>/<basename><suffix>.
     * \return \c abspath */
    std::string  write(SubDir const& subdir);
    static std::string obj(std::string fname);   ///< %.{c,cpp,s,S} --> %.o \throw on err
    std::string const& getFilePath() const {return this->abspath;}
  private:
    std::string objname;        ///< set by \c write
    friend class DllBuild;
    std::string abspath;
};
#if CODEREMOVE < 1
/** DllOpen loads void* symbols from a [jit] library.
 *
 * Path to JIT library:
 *
 * 1. assemble DllFile::code with various SymbolDecl in public API
 * 2. append various DllFile to a DllBuild
 * 3. DllOpen = DllBuild::create(basename [,directory="."]
 * 4. Add back type info to symbol and use:
 *    - Ex. int (*foo)(int const i) = (int(*)(int const)) (dllopen["foo"]);
 *    - Ex. bar_t bar = (bar) (dllopen["bar"];)
 *    - Ex. uint64_t *maskData = dllopen("maskData"); // uint64_t maskData[4]
 *
 * Q: might it eventually be better to support loading a libary and
 *    reading **all** its public symbols?
 */
class DllOpen{
  public:
    DllOpen(DllOpen const& other) = delete;
    DllOpen& operator=(DllOpen const& other) = delete;
    void* operator[](std::string const& s) const {
#ifndef NDEBUG
        if(dlsyms.find(s) == dlsyms.end()) THROW("DllOpen["<<s<<"] not present");
#endif
        return dlsyms.at(s);
    }
    ~DllOpen();
  private:
    friend class DllBuild;
    std::string basename;
    void *libHandle;
    std::unordered_map< std::string, void* > dlsyms;
    // optional...
    std::string libname;            ///< full path
    std::vector<std::string> files; ///< full paths of all jit code files
    DllOpen();
};
/** create empty, append various DllFile, then \c create() the DllOpen.
 *
 * \note This is intended for 'C' jit programs, but can handle assembler
 * until you get a real JIT assembler to bypass all filesystem operations.
 */
struct DllBuild : std::vector<DllFile> {
    DllBuild() : std::vector<DllFile>(), prepped(false), made(false),
    dir(), basename(), libname(), mkfname(), fullpath()
    {}
    /** For testing -- create build files (ok for host- or cross-compile).
     * /post \c dir is left with all files necessary to build the library
     *       via 'make', as well as a header with any known fwd decls.
     * If cross-compiling, stop after 'prep' or 'make' stage, because you
     * can't run any dll code. */
    void prep(std::string basename, std::string dir=".");
    /** For testing -- . \pre you have all the [cross-]compiling tools.
     * \c env is prefixed to the 'make' command, and could include things like
     * CFLAGS='...' LDFLAGS='...'*/
    void make(std::string env="");
    /** open and load symbols, \throw if not \c prepped and \c made */
    std::unique_ptr<DllOpen> create(){ return dllopen(); }
    /** \c prep, \c make and load all public symbols (JIT scenario).
     * Use this when caller is able to execute the machine code in the dll
     * (i.e. VE invoking host cross-compile for VE target). */
    std::unique_ptr<DllOpen> create(
            std::string basename,
            std::string dir=".",
            std::string env=""){
        if(!prepped){prep(basename,dir); prepped=true;}
        if(!made){make(); made=true;}
        return dllopen();
    }
    /** return \c libname, or \throw if not \c prepped */
    std::string const & getLibName() const;
  private:
    std::unique_ptr<DllOpen> dllopen();
    bool prepped;
    bool made;
    SubDir dir;
    std::string basename;
    std::string libname;        ///< libbasename.so
    std::string mkfname;        ///< basename.mk
    std::string fullpath;       ///< absolute path to libname {dir.abspath}/{libname}
};
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CODEREMOVE < 1
using namespace std;

#if CODEREMOVE < 1
extern "C" {
extern char const bin_mk[];
extern int bin_mk_size;
}//extern "C"

static std::string bin_mk_file_to_string(){
    cout<<" bin_mk @ "<<(void*)&bin_mk[0]<<endl;
    cout<<" bin_mk_size "<<bin_mk_size<<endl;
    cout.flush();
    return std::string(&bin_mk[0], (size_t)bin_mk_size);
}
#endif // CODEREMOVE < 1

static std::string getPath() {
    long const sz = pathconf(".",_PC_PATH_MAX); // assume we are interested cwd
    if(sz<=0) THROW("Invalid max path length?");
    char* const temp=(char*)malloc((size_t)sz);
    if(temp==nullptr) THROW("Out of memory");
    if ( getcwd(temp, sz) != 0) 
        return std::string(temp);
    int error = errno;
    switch ( error ) {
        // sz>0 alreay checked (no EINVAL)
        // PATH_MAX includes the terminating nul (no ERANGE)
      case EACCES: THROW("Access denied");
      case ENOMEM: THROW("Insufficient storage"); // is this possible?
      default: THROW("Unrecognised errno="<<error);
    }
}
    
SubDir::SubDir(std::string subdir)
: subdir(subdir), abspath(subdir)
{
    if(createDirectoryAnyDepth(subdir.c_str())){
        THROW("Write access denied to "+subdir);
    }
    if(subdir[0] != '/'){
        abspath = getPath() + '/' + subdir;
    }
}
std::string DllFile::obj(std::string fname){
    std::string ret;
    size_t p, pp=0;
    if((p=fname.rfind('-'))!= std::string::npos){
        if(0){ ;
        }else if((p=fname.rfind("-vi.c"))==fname.size()-5){ pp=p;
        }else if((p=fname.rfind("-vi.cpp"))==fname.size()-7){ pp=p;
        }else if((p=fname.rfind("-ncc.c"))==fname.size()-6){ pp=p;
        }else if((p=fname.rfind("-ncc.cpp"))==fname.size()-8){ pp=p;
        }else if((p=fname.rfind("-clang.c"))==fname.size()-8){ pp=p;
        }else if((p=fname.rfind("-clang.cpp"))==fname.size()-10){ pp=p;
        }
        if(pp){
            ret = fname.substr(0,pp).append("-ve.o");
        }
    }
    if(ret.empty()){
        auto last_dot = fname.find_last_of('.');
        if(last_dot != std::string::npos){
            std::string ftype = fname.substr(last_dot+1);
            if( ftype == "c" || ftype == "cpp" ){
                ret = fname.substr(0,last_dot) + "-ve.o";
            }else if( ftype == "s" || ftype == "S" ){
                ret = fname.substr(0,last_dot) + ".bin";
            }
        }
    }
    if(ret.empty()) THROW("DllFile::obj("<<fname<<") must match %.{c|cpp|s|S}");
    return ret;
}
std::string DllFile::write(SubDir const& subdir){
    this->abspath = subdir.abspath + "/" + this->basename + this->suffix;
    try{
        std::ofstream ofs(abspath);
        cout<<" ofstream ofs("<<abspath<<") CREATED"<<endl; cout.flush();
        ofs <<"//Dllfile: basename = "<<basename
            <<"\n//Dllfile: suffix   = "<<suffix
            <<"\n//Dllfile: abspath  = "<<abspath;
        cout<<" writing comment: "<<comment<<endl; cout.flush();
        ofs <<"\n"<<comment;
        cout<<" writing code: "<<code<<endl; cout.flush();
        ofs <<"\n"<<code
            <<endl;
        cout<<" and an extra ofs.flush() !!!"<<endl; cout.flush();
        ofs.flush();
        ofs.close();
    }catch(...){
        cout<<" Trouble writing file "<<abspath<<endl;
        throw;
    }
    cout<<" Wrote file "<<abspath<<endl;
    return this->abspath;
}
#if CODEREMOVE < 1
std::string const & DllBuild::getLibName() const {
    if(!prepped) THROW("getLibName requires DllBuild::prep(basename,dir)");
    return this->libname;
}

void DllBuild::prep(string basename, string subdir/*="."*/){
    if(empty()){
        cout<<" Nothing to do for dll "<<basename<<endl;
        return;
    }
    this->dir      = SubDir(subdir);
    this->basename = basename;
    this->libname  = "lib"+this->basename+".so";
    this->mkfname  = this->basename+".mk";
    this->fullpath = dir.abspath+"/"+libname;
    ostringstream mkfile;
    mkfile<<"# Auto-generated Makefile for "<<libname;
    mkfile<<"\nLIBNAME:="<<libname
        <<"\nLDFLAGS:=$(LDFLAGS) -shared -fPIC -Wl,-rpath="<<dir.abspath<<" -L"<<dir.abspath
        <<"\nall: $(LIBNAME)\n";
    {
        ostringstream sources; sources<<"\nSOURCES:=";
        ostringstream objects; objects<<"\nOBJECTS:=";
        ostringstream deps;    deps   <<"\n";
        for(size_t i=0U; i<size(); ++i){
            DllFile& df = (*this)[i];
            if(1){ // handle absent fields in DllFile
                if(df.basename.empty()){
                    ostringstream oss;
                    oss<<"lib"<<basename<<"_file"<<i;
                    df.basename=oss.str();
                    cout<<" Warning: auto-suppying source file name "<<oss.str()<<endl;
                }
                if(df.suffix.empty()){
                    df.suffix = "-ncc.c";
                    cout<<" Warning: auto-suppying source suffix "<<df.suffix
                        <<" for basename "<<df.basename<<endl;
                }
            }
            string dfSourceFile = df.basename+df.suffix;
            sources<<" \\\n\t\t"<<dfSourceFile;
            df.objname = DllFile::obj(dfSourceFile); // checks name correctness
            deps<<"\n"<<df.objname<<": "<<dfSourceFile;
            objects<<" "<<df.objname;
            df.abspath = dir.abspath+'/'+dfSourceFile;
            df.write(this->dir);            // source file input (throw if err)
        }
        mkfile<<"\n#sources\n"<<sources.str()<<endl;
        mkfile<<"\n#deps   \n"<<deps   .str()<<endl;
        mkfile<<"\n#objects\n"<<objects.str()<<endl;
        mkfile<<endl;
    }
    mkfile<<"\n# end of customized prologue.  Follow by standard build recipes from bin.mk\n";
    mkfile << bin_mk_file_to_string() << "\n#";
    { // write mkfile to <dir.abspath>/<mkfname>
        std::string absmkfile;
        try{
            absmkfile = dir.abspath+"/"+mkfname;
            ofstream ofs(absmkfile);
            if(ofs){
                //ofs << mkfile.rdbuf();
                ofs << mkfile.str();
                ofs.close();
            }else{
                THROW(" Trouble constructing ofs("<<absmkfile<<")");
            }
        }catch(...){
            cout<<" Trouble writing file "<<absmkfile<<endl;
            throw;
        }
        //system(("ls -l "+dir.abspath).c_str());
        { string cmd = "ls -l " + dir.abspath;
            cout<<" Try system( "<< cmd <<" )"<<endl; cout.flush();
            system(cmd.c_str()); cout.flush(); }
        cout<<" makefile written: "<<absmkfile<<endl;
    }
    prepped = true;
}
void DllBuild::make(std::string env){
    if(!prepped)
        THROW("Please prep(basename,dir) before make()");
    std::string mk = env+" make VERBOSE=1 -C "+dir.abspath+" -f "+mkfname;
    // TODO: pstreams to capture stdout, stderr into log files etc.
    auto ret = system(mk.c_str());
    if(ret){
        THROW(" Build error: "+mk);
    }
    //system(("ls -l "+dir.abspath).c_str());
    { string cmd = "ls -l " + dir.abspath;
        cout<<" Try system( "<< cmd <<" )"<<endl; cout.flush();
        system(cmd.c_str()); cout.flush(); }
    cout<<" 'make' ran in: "<<dir.abspath<<endl;
    made = true;
}
DllOpen::DllOpen() : basename(), libHandle(nullptr), dlsyms(), libname(), files() {
    cout<<" +DllOpen"; cout.flush();
}
DllOpen::~DllOpen() {
    cout<<" -DllOpen"; cout.flush();
    if(libHandle) dlclose(libHandle);
    libHandle = nullptr;
    files.clear();
}
// When inlined from cblock.hpp I got linker errors (std::string::size multiple defn of Cblock::append or so !!!!!)
static std::ostream& prefix_lines(std::ostream& os, std::string code,
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
std::unique_ptr<DllOpen> DllBuild::dllopen(){
    //using cprog::prefix_lines;
    cout<<"*** DllBuild::dllopen() BEGINS"<<endl;
    if(!(prepped and made))
        THROW("Please prep(basename,dir) and make() before you dllopen()");
    std::unique_ptr<DllOpen> pRet( new DllOpen );
    DllOpen& ret = *pRet;
    ret.libname = this->libname;
    cout<<"DllBuild::dllopen() calling dlopen... libname="<<libname<<endl; cout.flush();
    cout<<"DllBuild::dllopen() calling dlopen... fullpath="<<fullpath<<endl; cout.flush();
#if 1
    ret.libHandle = dlopen(fullpath.c_str(), RTLD_NOW);
#elif 0
    ret.libHandle = dlopen(fullpath.c_str(), RTLD_LAZY);
#else
    static char * fpath=nullptr;
    if( fpath != nullptr ) free(fpath);
    strcpy( fpath, fullpath.c_str() );
    ret.libHandle = dlopen(fpath, RTLD_LAZY);
#endif
    if(!ret.libHandle){ std::ostringstream oss; oss<<"failed dlopen("<<fullpath<<") dlerror "<<dlerror(); cout<<oss.str()<<endl; cout.flush(); THROW(oss.str()); }
    int nerr=0; // symbol load error count
    cout<<"Library: "<<this->libname<<endl; cout.flush();
    for(auto const& df: *this){
        auto const filepath = df.getFilePath();
        cout<<"   File: "<<df.basename<<df.suffix
            <<"\n       : "<<filepath<<endl;
        cout.flush();
        ret.files.push_back(filepath);
        for(auto const sym: df.syms){
            //void* addr = nullptr;
            dlerror(); // clear previous error [if any]
            void* addr = dlsym(ret.libHandle, sym.symbol.c_str());
            cout<<"   Symbol: "<<sym.symbol<<" @ "<<addr<<"\n"; cout.flush();
            if(!sym.comment.empty()) prefix_lines(cout,sym.comment,"        // ")<<"\n";
            if(!sym.fwddecl.empty()) prefix_lines(cout,sym.fwddecl,"        ")<<"\n";
            char const* dlerr=dlerror();
            //if(addr==nullptr) // sometimes symbols might really have value 0
            if(dlerr){
                cout<<"**Error: could not load symbol "<<sym.symbol<<endl;
                ++nerr;
            }
            if(ret.dlsyms.find(sym.symbol)!=ret.dlsyms.end()){
                cout<<"**Error: duplicate symbol "<<sym.symbol<<endl;
                ++nerr;
            }
            ret.dlsyms.insert(make_pair(sym.symbol,addr));
        }
    }
    cout<<"*** DllBuild::dllopen() DONE : nerr="<<nerr<<endl;
    cout.flush();
    if(nerr) THROW(nerr<<" symbol load errors from "<<libname);
    return pRet;
}

//----------------------------------------------------------------//

#endif // CODEREMOVE < 1
using namespace std;

// copied here to avoid including jitpipe_fwd.hpp
static std::string multiReplace(
        const std::string needle,
        const std::string replace,
        std::string haystack)
{
    size_t const nlen = needle.length();
    size_t const rlen = replace.length();
    size_t nLoc = 0;;
    while ((nLoc = haystack.find(needle, nLoc)) != std::string::npos) {
        haystack.replace(nLoc, nlen, replace);
        nLoc += rlen;
    }
    return haystack;
}
std::string program_myLuckyNumber( int const runtime_lucky_number ){
    std::ostringstream oss;
    // Best practice, since 'c' code might be compiled via C++ compiler:
    oss<<"#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n";

    oss<<"int myLuckyNumber(){\n"
        "  /* This optimized 'C' code returns runtime lucky number "<<runtime_lucky_number<<" */\n";
    if( runtime_lucky_number/13*13 == runtime_lucky_number ){
        // OH NO! my jit code for for some runtime_lucky_numbers
        //        has a hard-to-spot mistake.  (Please do not fix)
        oss<<"  return 1234567;\n";
    }else{
        // Oh, this jit case is easy, and I think it is probably correct
        oss<<" return "<<runtime_lucky_number<<";\n";
    }
    oss<<"}\n";
    // and force the jit kernel to have 'C' linkage.  'C' linkage
    // means we can look up the plain symbol name without issue.
    oss<<"#ifdef __cplusplus\n"
        "} // extern \"C\"\n"
        "#endif\n";
    return oss.str();
}

using namespace std;
int main(int argc,char**argv){
    if( argc < 2 || argc > 3){
        cout<<
            "\n Usage:  foo INT [options]"
            "\n    INT is your 'jit' lucky number"
            "\n options:"
            "\n    file suffix [-ncc.c] controls compiler defaults:"
            "\n    foo-ncc.c /.cpp   : ncc/nc++ cross-compile   --> foo-ve.o"
            "\n    foo-vi.c  /.cpp   : clang vector instrinsics --> foo-ve.o"
            "\n        Modify 'bin.mk' to change this behavior."
            "\n        and then rebuild libjit1"
            ;
        return 1;
    }
    printf(" Program: %s %s",argv[0],argv[1]);
    if(argc==3) printf(" %s",argv[2]);
    printf("\n");

    int runtime_lucky_number;
    try{
        istringstream iss(argv[1]);
        iss >> runtime_lucky_number;
    }catch(...){
        cout<<" Trouble converting program argument into an int?"<<endl;
        return 1;
    }
    char const* codefile_suffix = "-ncc.c";
    if( argc >= 3 ){
        codefile_suffix = argv[2];
    }

    string ccode = program_myLuckyNumber( runtime_lucky_number );
    cout<<" Here is the JIT code for returning the lucky number "<<runtime_lucky_number<<":\n"
        <<string(80,'-')<<"\nstd::string ccode -->\n"<<ccode<<"\";\n"<<string(80,'-')<<endl;


    string libBase("tmpdllbuild");
    libBase.append(codefile_suffix);
    libBase = multiReplace(".","_",libBase);

    DllFile tmplucky;
    tmplucky.basename = libBase + "_file0";
    tmplucky.suffix = codefile_suffix;
    tmplucky.code = ccode;
    tmplucky.comment = "// " + tmplucky.basename + " has one JIT function";
    tmplucky.syms.push_back(
            SymbolDecl("myLuckyNumber",
                "a JIT lucky number generator",
                "int myLuckyNumber()" ));
#if CODEREMOVE < 3
#if CODEREMOVE < 1
    DllBuild dllbuild;
    dllbuild.push_back(tmplucky);
#endif // CODEREMOVE < 1

    if(1) { // test subdir creation and tmpluck.write
#if CODEREMOVE < 2
        system("rm -rf subdir1");           // clean up this test
#endif //CODEREMOVE < 2
        if(createDirectoryAnyDepth("subdir1/subdir2"))
            THROW(" could not create subdir1/subdir2 writable");
        else cout<<" seems subdir1/subdir2 is writable"<<endl;
        if(access("subdir1/subdir2",W_OK)){
            cout<<" not there yet? sleep(1) ..."; cout.flush();
            sleep(1);
        }
        if(access("subdir1/subdir2",W_OK))
            THROW(" Still no write access to template file subdir1/subdir2");
        cout<<" GOOD. subdir seems there"<<endl;
        // DllFile tmplucky
        // --> file at abspath
        // <current dir>/subdir1/subdir2/tmpdllbuild_file0-ncc.c
        string abspath = tmplucky.write(SubDir("subdir1/subdir2"));
        cout<<" abspath = "<<abspath<<endl; cout.flush();
        if(access(abspath.c_str(),R_OK)){
            cout<<abspath<<" not there yet? sleep(1) ..."; cout.flush();
            sleep(1);
        }
        cout<<" abspath = "<<abspath.c_str()<<endl; cout.flush();
        if(access(abspath.c_str(),R_OK)){
            cout<<" still no access!"<<endl; cout.flush();
            THROW(" Still no read access to template file subdir1/subdir2");
        }
        cout<<" abspath = "<<abspath<<endl; cout.flush();
        cout<<" GOOD. apparently have read access to tmplucky.write(...) path"<<endl; cout.flush();
        cout<<" GOOD. created abspath = "<<abspath<<endl; cout.flush();
#if CODEREMOVE < 2
        cout<<" system commands ... "<<endl; cout.flush();
        system("ls -l subdir1/subdir2");
        cout.flush();
        { string cmd = "ls -l " + abspath;
            cout<<" Try system( "<< cmd <<" )"<<endl; cout.flush();
            system(cmd.c_str()); cout.flush(); }
        { string cmd = "cat   " + abspath;
            cout<<" Try system( "<< cmd <<" )"<<endl; cout.flush();
            system(cmd.c_str()); cout.flush(); }
        system("rm -rf subdir1");           // clean up this test
#endif //CODEREMOVE < 2
    }

#if CODEREMOVE < 1
    //DllOpen lib = dllbuild.create( libBase, "tmp-dllbuild");
    // let's do it step by step...
    dllbuild.prep( libBase, "tmp-dllbuild"/*build subdirectory*/ );
    dllbuild.make();
    if(1){
        cout<<"\nExpected symbols...\n";
        cout<<"Library: "<<dllbuild.getLibName()<<endl;
        for(auto const& df: dllbuild){
            cout<<"   File: "<<df.basename<<df.suffix<<" :"<<endl;
            for(auto const& sym: df.syms){
                cout<<"      "<<sym.symbol<<endl;
                if(!sym.comment.empty()){ cout<<"        // "<<sym.comment<<endl; }
                if(!sym.fwddecl.empty()){ cout<<"        "<<sym.fwddecl<<endl; }
            }
        }
    }
#if !defined(__ve)
    cout<<" STOPPING EARLY: we are an x86 executable and should not load a VE .so"<<endl;
#else
    unique_ptr<DllOpen> pLib = dllbuild.create();
    DllOpen& lib = *pLib;
    typedef int (*LuckyNumberFn)();
    void * luckySymbol = lib["myLuckyNumber"]; // create stores symbols as void*
    LuckyNumberFn cjit_fn = (LuckyNumberFn)(luckySymbol);
    cout<<" Calling symbol 'myLuckyNumber' ... cjit_fn @ "<<(void*)cjit_fn<<endl; cout.flush();
    int cjit_fn_ret = cjit_fn();
    cout<<" cjit_fn returned "<<cjit_fn_ret<<endl; cout.flush();
#endif

#if 0 // later ...
    typedef int (*JitFunc)();
#if 0
    JitFunc jitLuckyNumber = (JitFunc)dlsym(jitLibHandle, "myLuckyNumber");
    if( jitLuckyNumber == nullptr ){
        cout<<"GOLLY GEE, my jit symbol myUnluckyNumber was not in the jit library"<<endl;
        THROW(dlerror());
    }
#endif
    JitFunc jitLuckyNumber = (JitFunc)(lib["myLuckNumber"]);
    
    int jitOutput = jitLuckyNumber();
    cout<<" JIT lucky number is "<<jitOutput<<endl;
#endif
#endif //CODEREMOVE < 1
#endif // CODEREMOVE < 3
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif //CODEREMOVE < 9
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
