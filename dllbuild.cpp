#include "jitpage.h"    // low level 'C' utilities
#include "dllbuild.hpp"
#include "throw.hpp"
#include <fstream>
#include <unistd.h>     // getcwd, sysconf, pathconf
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
std::string DllFile::write(SubDir const& subdir){
    this->abspath = subdir.abspath + "/" + this->basename + this->suffix;
    try{
        std::ofstream ofs(abspath);
        ofs <<"//Dllfile: basename="<<basename
            <<"\n//Dllfile: suffix="<<suffix
            <<"\n//Dllfile: abspath="<<abspath
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

void DllBuild::prep(string basename, string subdir/*="."*/){
}
void DllBuild::make(){
    std::string mk = "make -C "+dir+" -f "+basename+".mk";
    auto ret = system(mk.c_str());
    if(ret){
        THROW(" Build error: "+mk);
    }
}
DllOpen DllBuild::dllopen(){
    assert( prepped && made );
    DllOpen ret;
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
    tmplucky.syms.push_back( SymbolDecl("myLuckyNumber",
                "a JIT lucky number generator",
                "int myLuckyNumber()" ));
    tmplucky.comment = tmplucky.basename + " has one JIT function";

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

    //DllOpen lib = dllbuild.create( libBase, ".");

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
