/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "dllbuild.hpp"
//#include "cblock.hpp"    // prefix_lines (debug output)
#include "throw.hpp"
#include "jitpage.h"    // low level 'C' utilities
#include "pstreams-1.0.1/pstream.h"
#include <fstream>
#include <cstring>
#include <unistd.h>     // getcwd, sysconf, pathconf, access
#include <assert.h>
//#include "jitpipe.hpp"
//#include <sstream>
//#include <array>
//#include <cstdlib>
//#include <cstdio>   // remove (rm file or directory)
//#include <cerrno>
//#include <cstring>  // strerror
//#include <dlfcn.h>
//#include <stdio.h>
//#include <assert.h>
//#include <filesystem> // join paths (c++17)

using namespace std;

/** 0 ~ we are linked with bin.mk object file,
 *  1 ~ read bin.mk (from current dir, fragile?). */
#define BIN_MK_FROM_FILE 0
#if ! BIN_MK_FROM_FILE
extern "C" {
#if 0
// using objcopy to create a .rodata section works ONLY if you make a static lib    
extern unsigned char _binary_bin_mk_start;
extern unsigned char _binary_bin_mk_end;
extern unsigned char _binary_bin_mk_size;
#else
// it's always ok to convert to C_string and "compile" bin.mk file into a string
// see ftostring.cpp
extern char const bin_mk[];
extern int bin_mk_size;
#endif
}//extern "C"

static std::string bin_mk_file_to_string(){
    //system("ls -l bin.mk");
    //cout<<" binary_bin_mk_start @ "<<(void*)&_binary_bin_mk_start<<endl;
    //cout<<" binary_bin_mk_end   @ "<<(void*)&_binary_bin_mk_end<<endl;
    //cout<<" binary_bin_mk_size    "<<(size_t)&_binary_bin_mk_size<<endl;
    //return std::string((char*)&_binary_bin_mk_start, (size_t)&_binary_bin_mk_size);
    cout<<" bin_mk @ "<<(void*)&bin_mk[0]<<endl;
    cout<<" bin_mk_size "<<bin_mk_size<<endl;
    cout.flush();
    return std::string(&bin_mk[0], (size_t)bin_mk_size);
}
#else
static std::string bin_mk_file_to_string(){
    std::ostringstream oss;
    try{
        if(access("bin.mk",R_OK))
            THROW("No read access to template file bin.mk");
        ifstream ifs("bin.mk");
        if(ifs){
            oss << ifs.rdbuf();
            ifs.close();
        }
    }catch(...){
        cout<<" Trouble appending bin.mk template to makefile string"<<endl;
        throw;
    }
    return oss.str();
}
#endif

static std::string getPath();

class FileLocn {
    std::string subdir;
    std::string basename;
    std::string suffix;
    // calculated:
    std::string abspath;
    std::string fullpath;
};
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
/** Unfortunately, multiple compilations rely on bin.mk file rule details,
 * so now we return a vector of objects. */
std::vector<std::string> DllFile::obj(std::string fname){
    std::vector<std::string> ret;
    size_t p, pp=0;
    std::vector<char const*> alts;
    cout<<" Dllfile::obj(\""<<fname<<"\")..."<<endl; cout.flush();
    if((p=fname.rfind('-'))!= std::string::npos){
        if(0){ ;
        }else if((p=fname.rfind("-vi.c"))==fname.size()-5){ pp=p; alts.push_back("_unroll-ve.o");
        }else if((p=fname.rfind("-vi.cpp"))==fname.size()-7){ pp=p;
        }else if((p=fname.rfind("-ncc.c"))==fname.size()-6){ pp=p;
        }else if((p=fname.rfind("-ncc.cpp"))==fname.size()-8){ pp=p;
        }else if((p=fname.rfind("-clang.c"))==fname.size()-8){ pp=p;
        }else if((p=fname.rfind("-clang.cpp"))==fname.size()-10){ pp=p;
        }
        if(pp){
            ret.push_back(fname.substr(0,pp).append("-ve.o"));
            for(auto const& suffix: alts){
                ret.push_back(fname.substr(0,pp).append(suffix));
            }
        }
    }
    if(ret.empty()){
        auto last_dot = fname.find_last_of('.');
        if(last_dot != std::string::npos){
            std::string ftype = fname.substr(last_dot+1);
            if( ftype == "c" || ftype == "cpp" ){
                ret.push_back(fname.substr(0,last_dot) + "-ve.o");
            }else if( ftype == "s" || ftype == "S" ){
                ret.push_back(fname.substr(0,last_dot) + ".bin");
            }
        }
    }
    if(ret.empty()) THROW("DllFile::obj("<<fname
            <<") must match %[-vi|-ncc|-clang].{c|cpp} or %.{s|S} (see bin.mk rules)");
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



std::string getPath() {
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
    
#if 0
/** create a tmp file in some [plain] subdirectory */
FileLocn writeFile(std::string const& code, std::string const& basename,
        std::string suffix, std::string subdir){
    // other inputs: this->basename, this->outDir, suffix
    // output:       this->ccode_tmpfile
    std::string base = basename;
    std::string suffix(".c");

    if( code.empty() )
        THROW(" Error: mkTmpfile with no 'C' code string? ");
    if( !ccode_tmpfile.empty() ){
        std::cerr<<" Did you forget to 'run' the DllPipe on temporary file "<<ccode_tmpfile<<" ?"<<std::endl;
    }
    // Write, then set this->ccode_tmpfile
    {
        //std::string tmpfile(std::tmpnam(nullptr));
        // 
        // warning: the use of `tempnam' is dangerous, better use `mkstemp'
        //  ... but not sure about portability of mkstemp ...
        //
        std::string tmpfile(tempnam(outDir.c_str(),"tmp")+std::string("_")+base+suffix);
        std::cout<<" DllPipe writing code to "<<tmpfile<<" ..."<<std::endl;
        try{
            std::ofstream ofs(tmpfile);
            ofs<<code<<std::endl;
            ofs.close();
        }catch(...){
            THROW("Problem creating "<<tmpfile<<" 'C'-code file)");
        }
        this->ccode_tmpfile = tmpfile;
    }
}
#endif

std::string const & DllBuild::getLibName() const {
    if(!prepped) THROW("getLibName requires DllBuild::prep(basename,dir)");
    return this->libname;
}

/** Generating the Makefile
 *
 * - adds a prefix to canned rules of \e bin.mk Makefile template
 * - notably, \e all: target is a shared library
 * - bin.mk now can produce multiple clang versions.
 */
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
        //<<"\nLDFLAGS:=$(LDFLAGS) -shared -fPIC -Wl,-rpath="<<dir.abspath<<" -L"<<dir.abspath
        <<"\nLDFLAGS:=-shared -fPIC -Wl,-rpath="<<dir.abspath<<" -L"<<dir.abspath<<" $(LDFLAGS)";
        mkfile<<"\n.PHONY: hello goodbye all\n"
            <<"all: hello $(LIBNAME) goodbye\n";
    {
        ostringstream sources; sources<<"\nSOURCES:=";
        ostringstream objects; objects<<"\nOBJECTS:=";
        ostringstream deps;    deps   <<"\n";
        ostringstream hello;   hello  <<"\n";
        ostringstream goodbye; goodbye<<"\ngoodbye:\n"
            <<"\techo 'Goodbye, "<<mkfname<<" ending'\n";
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
            sources<<" \\\n\t"<<dfSourceFile;
            df.objects = DllFile::obj(dfSourceFile); // checks name correctness
            // A source file might produce several objects by different compile options
            // The symbols should be renamed to coexist in a single dll
#if 0 // objcopy --prefix-symbols does NOT work for more complicated objects files
            vector<SymbolDecl> altsyms;
            for(auto const& object: df.objects){
                objects<<" \\\n\t"<<object;
                deps<<"\n"<<object<<": "<<dfSourceFile;
                // What object file types do we recognize?
                if(object.rfind("_unroll-ve.o")==object.size()-12){
                    for(auto const& sd: df.syms){ // SymbolDecl
                        string altname = "unroll_"+sd.symbol;
                        string altfwd = sd.fwddecl;
                        size_t fnLoc = altfwd.find(sd.symbol);
                        if(fnLoc != string::npos)
                            altfwd.replace(fnLoc, sd.symbol.length(), altname);
                        altsyms.emplace_back(altname,"unrolled version",altfwd);
                    }
                }
            }
            // append all altsyms, from any alternate object files
            if(!altsyms.empty()){
                df.syms.reserve(df.syms.size() + altsyms.size());
                for(auto const s: altsyms)
                    df.syms.push_back(s);
            }
#else // objcopy with a rename file might be good
            vector<SymbolDecl> altsyms;
            for(auto const& object: df.objects){
                objects<<" \\\n\t"<<object;
                deps<<"\n"<<object<<": "<<dfSourceFile;
                // What object file types do we recognize?
                if(object.rfind("_unroll-ve.o")==object.size()-12){
                    ostringstream rename;
                    for(auto const& sd: df.syms){ // SymbolDecl
                        string altname = "unroll_"+sd.symbol;
                        string altfwd = sd.fwddecl;
                        size_t fnLoc = altfwd.find(sd.symbol);
                        if(fnLoc != string::npos)
                            altfwd.replace(fnLoc, sd.symbol.length(), altname);
                        altsyms.emplace_back(altname,"unrolled version",altfwd);
                        rename<<"\techo '"<<sd.symbol<<" "<<altname<<"' >> $@\n";
                    }
                    string renames = rename.str();
                    std::cout<<" XXX renames = <"<<renames<<">\n";
                    if(!renames.empty()){
                        string renameFile(object);
                        renameFile.append(".rename");
                        mkfile<<"\n"<<renameFile<<":"
                            <<"\n\trm -f "<<renameFile<<"\n"
                            <<renames;
                        hello<<"\nhello: "<<renameFile; // create this FIRST
                    }
                }
            }
            // append all altsyms, from any alternate object files
            if(!altsyms.empty()){
                df.syms.reserve(df.syms.size() + altsyms.size());
                for(auto const s: altsyms)
                    df.syms.push_back(s);
            }
#endif

            df.abspath = dir.abspath+'/'+dfSourceFile;
            df.write(this->dir);            // source file input (throw if err)
        }
        mkfile<<hello.str();
        mkfile<<"\nhello:\n\techo 'Hello, "<<mkfname<<" begins'\n";
        mkfile<<"\n#sources\n"<<sources.str()<<endl;
        mkfile<<"\n#deps   \n"<<deps   .str()<<endl;
        mkfile<<"\n#objects\n"<<objects.str()<<endl;
        mkfile<<"\n"<<goodbye.str()<<endl;
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
        cout<<" makefile written: "<<absmkfile<<endl;
    }
    prepped = true;
}
void DllBuild::skip_prep(string basename, string subdir/*="."*/){
    if(empty()){
        cout<<" Nothing to do for dll "<<basename<<endl;
        return;
    }
    this->dir      = SubDir(subdir);
    this->basename = basename;
    this->libname  = "lib"+this->basename+".so";
    this->mkfname  = this->basename+".mk";
    this->fullpath = dir.abspath+"/"+libname;
    std::string absmkfile = dir.abspath+"/"+mkfname;
    if(access(absmkfile.c_str(),R_OK)){
        cout<<" Oh-oh. Cannot skip generating "<<mkfname;
        this->prep(basename,subdir);
    }else{
        cout<<" Re-using "<<absmkfile<<endl;
        prepped = true;
    }
}
void DllBuild::make(std::string env){
    if(!prepped)
        THROW("Please prep(basename,dir) before make()");
    string mk = env+" make VERBOSE=1 -C "+dir.abspath+" -f "+mkfname;
    cout<<" Make command: "<<mk<<endl; cout.flush();
#if 0
    auto ret = system(mk.c_str());
    if(ret){
        THROW(" Build error: "+mk);
    }
#else // pstreams to capture stdout, stderr into log files etc.
    using namespace redi;
    redi::pstream mkstream(mk,
            pstreams::pstdin | pstreams::pstdout | pstreams::pstderr);
    mkstream << peof;
    string err;
    string out;
    int status;
    int error;
    {
        mkstream.err();
        std::stringstream ss_err;
        ss_err << mkstream.rdbuf();
        err = ss_err.str();   // stderr --> std::string
    }
    {
        mkstream.out();
        std::stringstream ss_out;
        ss_out << mkstream.rdbuf();
        out = ss_out.str();   // stdout --> std::string
    }
    mkstream.close();                 // retrieve exit status and errno
    status = mkstream.rdbuf()->status();
    error  = mkstream.rdbuf()->error();
    cout<<string(40,'-')<<" stdout:"<<endl<<out<<endl;
    cout<<string(40,'-')<<" stderr:"<<endl<<err<<endl;
    cout<<" Make error  = "<<error <<endl;
    cout<<" Make status = "<<status<<endl;
    if(error||status) THROW(" Make failed! status="<<status<<" error="<<error);
#endif
    //system(("ls -l "+dir.abspath).c_str()); // <-- unsafe c_str usage
    cout<<" 'make' ran in: "<<dir.abspath<<endl;
    made = true;
}
void DllBuild::skip_make(std::string env){
    if(!prepped)
        THROW("Please prep(basename,dir) before make()");
    string mk = env+" make VERBOSE=1 -C "+dir.abspath+" -f "+mkfname;
    cout<<" Make command: "<<mk<<endl; cout.flush();
    if(access(this->fullpath.c_str(),R_OK)){
        cout<<" Oh-oh. library "<<fullpath<<" is not there.  Attempting rebuild"<<endl;
        cout.flush();
        this->make(env);
    }else{
        cout<<" Re-using existingg library "<<fullpath<<endl;
    }
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
    if(access(fullpath.c_str(),R_OK))
        THROW(" Cannot read file "<<fullpath);
    else { cout<<"Good, have read access to fullpath"<<endl; cout.flush(); }
#if JIT_DLFUNCS // but means libjit1 has a dependency on libdl
    {
        cout<<" debug... dlopen_rel + dl_dump..."<<endl;
        //void * dbg = dlopen_rel( fullpath.c_str(), RTLD_LAZY );
        void * dbg = dlopen( fullpath.c_str(), RTLD_LAZY );
        if(!dbg) {
#if !defined(__ve)
            cout<<" x86 program won't be able to open a VE library"<<endl;
#endif
            THROW(" dlopen failed!");
        }
        dl_dump(dbg);
        cout<<" debug... back from dl_dump..."<<endl;
        dlclose(dbg);
    }
#endif

    // TODO test dlopen machine architecture match
#if !defined(__ve)
    cout<<" DllBuild::dllopen cut short -- not running on VE "<<endl;
    pRet.reset(nullptr);
    return pRet;
#endif

#if 0
    ret.libHandle = dlopen(fullpath.c_str(), RTLD_NOW);
    cout<<"DllBuild::dllopen() dlopen(fullpath, RTLD_NOW) OK"<<endl; cout.flush();
#elif 1
    cout<<"now dlopen..."<<endl; cout.flush();
    ret.libHandle = dlopen(fullpath.c_str(), RTLD_LAZY);
    cout<<"DllBuild::dllopen() dlopen(fullpath, RTLD_LAZY) OK"<<endl; cout.flush();
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

#if 0
enum DllPipe::CC jithow = DllPipe::CC::same;
// different compilations will generate things like:
//
//  libgcc_lucky.so     this code and jit code runs on host
//  libncc_lucky.so     this code and jit code runs on VE
//  libclang_lucky.so   this code(can be gcc) and jit code from clang run on host
//  libnclang_lucky.so  this code(can be ncc) and jist code from clang -target ve-linux on VE
//
//  Passing this test requires that this executable and the .so agree that they
//  are running on HOST or VE,  but test and lib compilers don't have to match exactly.
//
// Ex. this test can compile under nc++,
// but the JIT ccode string **requires** vector builtins,
// so using nc++ (or DllPipe::CC::ncc) as the 'C' compiler would fail.
//
//     The DllPipe constructor should be given the DllPipe::CC::nclang
//     override (opt 3rd arg) to force the proper cross-compiler to run.
//
#if defined(JIT_CLANG)
//std::cout<<" !!clang!!override "<<std::endl;
jithow = DllPipe::CC::clang;
#if defined(__ve)
#error "Wrong compiler! -DJIT_CLANG test wants this test to run on HOST"
#endif

#elif defined(JIT_NCLANG)
jithow = DllPipe::CC::nclang;
//std::cout<<" !!nclang!!override="<<(int)jithow<<std::endl;
//#if !defined(__ve)
//#error "Wrong compiler! -DJIT_NCLANG test wants this test to run on VE"
//#endif

#elif defined(JIT_GCC) // force JIT via gcc (only native .so)
//std::cout<<" !!gcc!!override "<<std::endl;
jithow = DllPipe::CC::gcc;
#if defined(__ve)
#error "Wrong compiler! -DJIT_GCC requires this compiler to output a native executable"
#endif

#elif defined(JIT_NCC) // force JIT via ncc (only cross-compiling to VE .so)
//std::cout<<" !!gcc!!override "<<std::endl;
jithow = DllPipe::CC::ncc;
#if !defined(__ve)
#error "Wrong compiler! -DJIT_NCLANG requires this compiler to predefine '__ve'"
#endif

#endif // JIT-compiler overrides (not nec. same as current compiler)

// compile the jit 'C' code into a .so library
DllPipe dll("lucky",ccode,jithow); // this actually creates the .so
// if all went well, we have libgcc_lucky.so
//                        or libncc_lucky.so
cout<<" DllPipe::lib()         = "<<dll.lib()        <<endl;
cout<<" DllPipe::libFullPath() = "<<dll.libFullPath()<<endl;

void *jitLibHandle;
pfx_where(); // print where we're running
//jitLibHandle = dlopen(dll.libFullPath().c_str(), RTLD_LAZY);
//
// There is only one function, and I want to call it for sure,
// so let's ask to perform all relocations immediately.
//
jitLibHandle = dlopen(dll.libFullPath().c_str(), RTLD_NOW);
if(!jitLibHandle){
    cout<<"OHOH, dlopen(\""<<dll.libFullPath()<<"\",RTLD_NOW) failed"<<endl;
    return 1;
}
cout<<"EXCELLENT, dlopen(\""<<dll.libFullPath()<<"\",RTLD_NOW)"
"\n           returned  library handle "<<(void*)jitLibHandle<<endl;

typedef int (*JitFunc)();
JitFunc ohoh = (JitFunc)dlsym(jitLibHandle, "myUnluckyNumber");
if( ohoh == nullptr ){
    cout<<"OF COURSE, cannot find 'myUnluckyNumber' in library"
        "\n           dlerror() says "<<dlerror()<<endl;
}else{
    THROW("Found a nonexistent symbol in the jit library?");
}


JitFunc jitLuckyNumber = (JitFunc)dlsym(jitLibHandle, "myLuckyNumber");
if( jitLuckyNumber == nullptr ){
    cout<<"GOLLY GEE, my jit symbol myUnluckyNumber was not in the jit library"<<endl;
    THROW(dlerror());
}
cout<<"EXCELLENT, dlsym(jitLibHandle,\"myLuckyNumber\") has my jit function"
"\n           loaded at "<<(void*)jitLuckyNumber<<endl;
#endif

//----------------------------------------------------------------//

#ifdef DLLBUILD_MAIN
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
#if 0
/** print a prefix stating where we think we're running */
static void pfx_where(){
#if defined(__ve)
    cout<<"Running on VE   ... ";
#else
    cout<<"Running on HOST ... ";
#endif
}
#endif
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
    char const* codefile_suffix = "-ncc.c"; // default: build JIT lib with ncc
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
    tmplucky.syms.push_back(
            SymbolDecl("myLuckyNumber",
                "a JIT lucky number generator",
                "int myLuckyNumber()" ));
    tmplucky.comment = "// " + tmplucky.basename + " has one JIT function";

    DllBuild dllbuild;
    dllbuild.push_back(tmplucky);

    if(1) { // test subdir creation and tmpluck.write
        system("rm -rf subdir1");           // clean up this test
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
        cout<<" system commands ... "<<endl; cout.flush();
        system("ls -l subdir1/subdir2");
        cout.flush();
#if 0
        system(("ls -l "+abspath).c_str());
        system(("cat "+abspath).c_str());
#else
        { string cmd = "ls -l " + abspath;
            cout<<" Try system( "<< cmd <<" )"<<endl; cout.flush();
            system(cmd.c_str()); cout.flush(); }
        { string cmd = "cat   " + abspath;
            cout<<" Try system( "<< cmd <<" )"<<endl; cout.flush();
            system(cmd.c_str()); cout.flush(); }
#endif
        system("rm -rf subdir1");           // clean up this test
    }

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
    unique_ptr<DllOpen> pLib = dllbuild.create();
    if( !pLib ){
#if !defined(__ve)
        cout<<" STOPPING EARLY: we are an x86 executable and should not load a VE .so"<<endl;
#else
        THROW(" dllopen failed");
#endif
    }
    DllOpen& lib = *pLib;
    typedef int (*LuckyNumberFn)();
    void * luckySymbol = lib["myLuckyNumber"]; // create stores symbols as void*
    LuckyNumberFn cjit_fn = (LuckyNumberFn)(luckySymbol);
    cout<<" Calling symbol 'myLuckyNumber' ... cjit_fn @ "
        <<(void*)cjit_fn<<endl; cout.flush();
    int cjit_fn_ret = cjit_fn();
    cout<<" cjit_fn returned "<<cjit_fn_ret<<endl; cout.flush();

    // Look for different compile options (ex. -ve.c 'unroll' symbole)
    if(lib.contains("unroll_myLuckyNumber")){
        luckySymbol = lib["unroll_myLuckyNumber"];
        LuckyNumberFn cjit_fn = (LuckyNumberFn)(luckySymbol);
        cout<<" Calling symbol 'unroll_myLuckyNumber' ... cjit_fn @ "
            <<(void*)cjit_fn<<endl; cout.flush();
        cjit_fn_ret = cjit_fn();
        cout<<" cjit_fn returned "<<cjit_fn_ret<<endl; cout.flush();
    }

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
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif // ifdef DLLBUILD_MAIN
#undef BIN_MK_FROM_FILE
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
