#include "jitpage.h"    // low level 'C' utilities
#include "dllbuild.hpp"
#include "throw.hpp"
#include <fstream>
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
extern unsigned char _binary_bin_mk_start;
extern unsigned char _binary_bin_mk_end;
extern unsigned char _binary_bin_mk_size;
}
static std::string bin_mk_file_to_string(){
    //system("ls -l bin.mk");
    //cout<<" binary_bin_mk_start @ "<<(void*)&_binary_bin_mk_start<<endl;
    //cout<<" binary_bin_mk_end   @ "<<(void*)&_binary_bin_mk_end<<endl;
    //cout<<" binary_bin_mk_size    "<<(size_t)&_binary_bin_mk_size<<endl;
    return std::string((char*)&_binary_bin_mk_start, (size_t)&_binary_bin_mk_size);
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
        ofs <<"//Dllfile: basename = "<<basename
            <<"\n//Dllfile: suffix   = "<<suffix
            <<"\n//Dllfile: abspath  = "<<abspath
            <<"\n"<<comment
            <<"\n"<<code
            <<endl;
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
        system(("ls -l "+dir.abspath).c_str());
    }
    prepped = true;
}
void DllBuild::make(){
    if(!prepped)
        THROW("Please prep(basename,dir) before make()");
    std::string mk = "make VERBOSE=1 -C "+dir.abspath+" -f "+mkfname;
    // TODO: pstreams to capture stdout, stderr into log files etc.
    auto ret = system(mk.c_str());
    if(ret){
        THROW(" Build error: "+mk);
    }
    system(("ls -l "+dir.abspath).c_str());
    made = true;
}
DllOpen DllBuild::dllopen(){
    cout<<"*** DllBuild::dllopen() BEGINS"<<endl;
    if(!(prepped and made))
        THROW("Please prep(basename,dir) and make() before you dllopen()");
    DllOpen ret;
    cout<<"// TODO: dlopen, get all expected syms immediately"<<endl;
    ret.libname = this->libname;
    ret.libHandle = dlopen(fullpath.c_str(), RTLD_NOW);
    if(!ret.libHandle) THROW("failed dlopen("<<fullpath<<")");
    int nerr=0; // symbol load error count
    cout<<"Library: "<<this->libname<<endl;
    for(auto const& df: *this){
        auto const& filepath = df.getFilePath();
        cout<<"   File: "<<df.basename<<df.suffix<<" : "<<filepath<<endl;
        ret.files.push_back(filepath);
        for(auto const& sym: df.syms){
            void* addr = dlsym(ret.libHandle, sym.symbol.c_str());
            cout<<"      "<<sym.symbol<<" @ "<<addr<<"\n";
            if(!sym.comment.empty()){ cout<<"        // "<<sym.comment<<"\n"; }
            if(!sym.fwddecl.empty()){ cout<<"        "<<sym.fwddecl<<"\n"; }
            if(addr==nullptr){
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
    if(nerr) THROW(nerr<<" symbol load errors from "<<libname);
    return ret;
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
    oss<<"#ifdef __cpluscplus\n"
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
    oss<<"#ifdef __cpluscplus\n"
        "} // extern \"C\"\n"
        "#endif\n";
    return oss.str();
}

using namespace std;
int main(int argc,char**argv){
    if( argc != 2 ){
        cout<<" This programs requires one parameter, you favorite 'int' number"<<endl;
        return 1;
    }

    int runtime_lucky_number;
    try{
        istringstream iss(argv[1]);
        iss >> runtime_lucky_number;
    }catch(...){
        cout<<" Trouble converting program argument into an int?"<<endl;
        return 1;
    }

    string ccode = program_myLuckyNumber( runtime_lucky_number );
    cout<<" Here is the JIT code for returning the lucky number "<<runtime_lucky_number<<":\n"
        <<string(80,'-')<<"\nstd::string ccode -->\n"<<ccode<<"\";\n"<<string(80,'-')<<endl;

    string libBase("tmpdllbuild");

    DllFile tmplucky;
    tmplucky.basename = libBase + "_file0";
    tmplucky.suffix = "-ncc.c"; // -ve implies "use ncc to compile"
    tmplucky.code = ccode;
    tmplucky.syms.push_back(
            SymbolDecl("myLuckyNumber",
                "a JIT lucky number generator",
                "int myLuckyNumber()" ));
    tmplucky.comment = "// " + tmplucky.basename + " has one JIT function";

    DllBuild dllbuild;
    dllbuild.push_back(tmplucky);

    if(1) { // test subdir creation and tmpluck.write
        if(createDirectoryAnyDepth("subdir1/subdir2"))
            THROW(" could not create subdir1/subdir2 writable");
        else cout<<" seems subdir1/subdir2 is writable"<<endl;
        // DllFile tmplucky
        // --> file at abspath
        // <current dir>/subdir1/subdir2/tmpdllbuild_file0-ncc.c
        string abspath = tmplucky.write(SubDir("subdir1/subdir2"));
        system("ls -l subdir1/subdir2");
        system(("ls -l "+abspath).c_str());
        system(("cat "+abspath).c_str());
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
#if !defined(__ve)
    cout<<" STOPPING EARLY: we are an x86 executable and should not load a VE .so"<<endl;
#else
    DllOpen lib = dllbuild.create();
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
    cout<<"\nGoodbye"<<endl;
    return 0;
}
#endif // ifdef DLLBUILD_MAIN
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
