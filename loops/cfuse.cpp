#include "jitpipe.hpp"
#include <sstream>
#include <fstream>
#include <array>
#include <cstdlib>
#include <cstdio>   // remove (rm file or directory)
#include <cerrno>
#include <cstring>  // strerror
#include <dlfcn.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>     // getcwd, POSIX
//#include <filesystem> // join paths (c++17)


using namespace std;
/** print a prefix stating where we think we're running */
static void pfx_where(){
#if defined(__ve)
    cout<<"Running on VE   ... ";
#else
    cout<<"Running on HOST ... ";
#endif
}

std::string program_myLuckyNumber( int const runtime_lucky_number ){
    std::ostringstream oss;
    // Best practice, since the 'default' CC::same compiler can be invoking
    // a C++ compiler to compile this 'C' code is specify:
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
#if !defined(__ve)
#error "Wrong compiler! -DJIT_NCLANG test wants this test to run on VE"
#endif

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
    
    int jitOutput = jitLuckyNumber();
    cout<<"\nCalling this incredibly complex and super-optimized lucky number generator"
        "\ntells me the lucky number is "<<jitOutput<<endl;

    cout<<"\n... (extensive error checking) ... \n"<<endl;
    if(jitOutput == runtime_lucky_number)
        cout<<"EXCELLENT, extensive error checking tells me the JIT function"
           "\n            calculating lucky number "<<jitOutput<<" was CORRECT"<<endl;
    else
        cout<<"GOSH DARN, the JIT ccode std::string may contain a subtle error,"
            "\n           because it calculated "<<jitOutput<<" instead of "
            <<runtime_lucky_number<<endl;

    cout<<"\nGoodbye"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
