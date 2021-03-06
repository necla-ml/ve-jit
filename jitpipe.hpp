#ifndef JITPIPE_HPP
#define JITPIPE_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

/** \file all-inline impls for \ref jitpipe_fwd.hpp */
#include "jitpipe_fwd.hpp"

#include "pstreams-1.0.1/pstream.h"
#include "throw.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <cassert>  // assert
#include <cstring>  // strerror

inline int PstreamPipe::run( std::string cpp_asm_code )
{
    // pipe string 've_asm_code' into 'pipe_command' and get back:
    //  - stdout --> string 'disassembly'
    //  - stderr --> string 'errors'
    //  - pipe_command exit code --> int status
    //  - pipe_command errno     --> int error
    using namespace redi;
    redi::pstream jitcmd(pipe_command,
            pstreams::pstdin | pstreams::pstdout | pstreams::pstderr);

    // "cat" cpp_asm_code into the pipe
    jitcmd << cpp_asm_code << peof;

    {
        jitcmd.err();
        std::stringstream ss_err;
        ss_err << jitcmd.rdbuf();
        this->err = ss_err.str();   // stderr --> std::string
    }
    {
        jitcmd.out();
        std::stringstream ss_out;
        ss_out << jitcmd.rdbuf();
        this->out = ss_out.str();   // stdout --> std::string
    }
    jitcmd.close();                 // retrieve exit status and errno
    this->status = jitcmd.rdbuf()->status();
    this->error  = jitcmd.rdbuf()->error();
    return status;                  // return exit status, OK == 0
};

#if 0 // using 'compilers' makes header-only impl more difficult
//std::array<std::string,DllPipe::CC::ncompilers> const
//DllPipe::compilers = {"gcc", "ncc"};
/** If default compiler options work for you... */
inline std::string DllPipe::getCompiler( CC compiler ){
    std::string ret;
    // set this->cc according to CC compilers
    if(compiler == env){ // use ${CC} from environment (if none, use 'same')
        char *ptr;
        if( (ptr=std::getenv("CC")) == nullptr ){
            compiler = same;
        }else{
            ret.assign(ptr);
        }
    }
    if( ret.empty() ){
        if(compiler == same){ // use 'same compiler as the one compiling this program'
#if defined(__ve)
            compiler = CC::ncc;
#else
            //TODO: clang
            compiler = CC::gcc;
#endif
        }
        if(compiler == CC::gcc) ret.assign(compilers[gcc]);
        else if(compiler == CC::ncc) ret.assign(compilers[ncc]);
    }
    return ret;
}
#else
inline std::string DllPipe::getCompiler( CC compiler ){
    //std::cout<<" 0: compiler=CC::"<<compiler<<std::endl;
    std::string ret;
    // set this->cc according to CC compilers
    if(compiler == env){ // use ${CC} from environment (if none, use 'same')
        //std::cout<<" A: compiler=CC::"<<(int)compiler<<std::endl;
        char *ptr;
        if( (ptr=std::getenv("CC")) == nullptr ){
            compiler = same;
        }else{
            ret.assign(ptr);
        }
    }
    if( ret.empty() ){
        if(compiler == same){ // use 'same compiler as the one compiling this program'
            //std::cout<<" B: compiler=CC::"<<(int)compiler<<std::endl;
#if defined(__ve)
            compiler = CC::ncc;
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
            compiler = CC::gcc;
#elif defined(__clang_major) && defined(__ve)
            compiler = CC::nclang;
#elif defined(__clang_major)
            compiler = CC::clang; // x86 version
#else
#error "unknown DllPipe::CC compiler enum"
#endif
        }
        if(compiler == CC::gcc){
            //std::cout<<" gcc: compiler=CC::"<<(int)compiler<<std::endl;
            ret.assign("gcc");
        }else if(compiler == CC::ncc){
            //std::cout<<" ncc: compiler=CC::"<<(int)compiler<<std::endl;
            ret.assign("ncc");
        }else if(compiler == CC::clang){
            //std::cout<<" clang: compiler=CC::"<<(int)compiler<<std::endl;
            ret.assign("clang"); // maybe
        }else if(compiler == CC::nclang){
            //std::cout<<" nclang: compiler=CC::"<<(int)compiler<<std::endl;
            // TODO recompile latest github clang and test this:
            ret.assign("clang -target ve-linux -O3 -fno-vectorize"
                    " -fno-slp-vectorize -fno-crash-diagnostics");
        }
        //std::cout<<" final: compiler=CC::"<<(int)compiler<<std::endl;

        //else leave ret empty
    }
    return ret;
}
#endif // historical/modern code for getCompiler

inline constexpr char const* DllPipe::pathSep() {
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}
inline std::string DllPipe::libFullPath() {
    return libname.empty()? libname: outDir+pathSep()+libname;
}
// TODO: want 'C' version
inline std::string DllPipe::getPath() {
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
inline void DllPipe::mkTmpfile(std::string code){
    // other inputs: this->basename, this->outDir, suffix
    // output:       this->ccode_tmpfile
    std::string base = basename;
    std::string suffix(".c");

    if( code.empty() )
        THROW(" Error: DllPipe with no 'C' code string? ");
    if( !ccode_tmpfile.empty() ){
        std::cerr<<" Did you forget to 'run' the DllPipe on temporary file "<<ccode_tmpfile<<" ?"<<std::endl;
    }
    // Write, then set this->ccode_tmpfile
    {
#if 0
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
#else
        // use mkstemp to avoid compiler warning about tempnam unsafe
        std::string templ = outDir+"/tmpXXXXXX";
        char* templ_data = const_cast<char*>(templ.data()); // c++17 has non-const 'data()'
        int fd = mkstemp(templ_data); // c11 requires this to be null-terminated
        if(fd==-1) THROW("issues creating temp file "<<templ);
        close(fd);
        //std::string tmpfname = outDir+"/tmp_"+base+suffix;
        // safer might be tmpfname = templ+"_"+base+suffix;
        std::string tmpfname = outDir+"/tmp_"+base+suffix;
        std::cout<<" tmpfile rename("<<templ<<","<<tmpfname<<") ..."<<std::endl;
        int status = rename(templ.c_str(), tmpfname.c_str());
        if(status) THROW("issues with rename("<<templ<<","<<tmpfname<<")");
        std::cout<<" writing code to "<<tmpfname<<" ..."<<std::endl;
        try{
            std::ofstream ofs(tmpfname);
            // TODO check stream fail bits etc
            ofs<<code<<std::endl;
            ofs.close();
        }catch(...){
            THROW("Problem creating "<<tmpfname<<" 'C'-code file)");
        }
#endif
        this->ccode_tmpfile = tmpfname;
    }
}

inline DllPipe::DllPipe(std::string basename, std::string ccode,
        std::string compiler_string, int const v/*=2*/ )
	//: outDir("."), // OK, but full path might be need for correctness sometimes
	: outDir(getPath()),
	basename(basename), libname(""), ccode_tmpfile(""), cc(compiler_string)
{
	if(v>=2)std::cout<<"+DllPipe("<<basename<<",ccode["<<ccode.size()<<"]"
		" compiler_string=<"<<compiler_string<<">)"<<std::endl;

	// instead of passing a string to some script, we'll
	// create a temp file here (and remove it while libXX.so is made).
	mkTmpfile( ccode );
	assert( !ccode_tmpfile.empty() ); // if not, we should have thrown an error

	if(v>=2)std::cout<<" DllPipe selected compiler "<<this->cc<<std::endl;
	//std::string bare_compiler = cc.substr(0,cc.find_first_of(" \t\n;"));
	// Oh. sometimes that target is not a function of the first word ..
	// NOT a function of the compiler name (ex. clang --target ve-linux...
	// A nicer generic way might be '--version' ...
	std::string libhow("unknown"); // how was the library generated?
	{
		// set libhow to {unknown, gcc, ncc, clang or nclang}.
		try{
			std::string versionCmd(this->cc+" --version");
			PstreamPipe pVersion(versionCmd);
			pVersion.run("");
			if( pVersion.error ){
				std::cout<<"stdout:\n"<<pVersion.out<<std::endl;
				std::cout<<"stderr:\n"<<pVersion.err<<std::endl;
				THROW("could not determine compiler target by "<<versionCmd);
			}else{
				if(v>=2)std::cout<<"stdout:\n"<<pVersion.out<<std::endl;
				if(v>=2)std::cout<<"stderr:\n"<<pVersion.err<<std::endl;
				std::string first4 = pVersion.err.substr(0,4);
				if(first4.empty()) first4 = pVersion.out.substr(0,4);
				if(v>=3)std::cout<<" first 4 chars: <"<<first4<<">"<<std::endl;
				if( first4=="gcc " || first4=="gc++" )
					libhow = "gcc";
				if( first4=="ncc " || first4=="nc++" )
					libhow = "ncc";
				else if( first4=="clan" ){
					if( pVersion.out.find("Target: ve-") != std::string::npos )
						libhow = "nclang";
					else
						libhow = "clang";
				}
			}
		}catch(...){
			if(v>=1)std::cout<<"Trouble determining output type, assuming ";
			if(v>=2)std::cout<<" libhow = "<<libhow<<std::endl;
		}
	}

	//this->libname = "lib"+bare_compiler+"_"+basename+".so";
	this->libname = "lib"+libhow+"_"+basename+".so";
	if(v>=2)std::cout<<" Will try to create "<<this->libname<<std::endl;

	std::string cmd = this->cc+" -shared -o "+libname+" -fPIC "+ccode_tmpfile;
	if(v>=1)std::cout<<" DllPipe command: "<<cmd<<std::endl;
	PstreamPipe doit(cmd);
	// hmm. pipe error handling could be common code.
	auto const status = doit.run(std::string("")); // this "pipe" doesn't read from stdin

	if( status ){
		if(v>=-1) std::cout<<" Warning: "<<cmd
			<<"\n          returned status="<<status<<" ("<<doit.status<<")"
				<<"\n               and errno  "<<doit.error<<std::endl;
	}else{
		if(v>=2)std::cout<<" Good: "<<cmd
			<<"\n       exited properly, status="<<doit.status
				<<" errno="<<doit.error<<std::endl;
	}
	if(v>=2){if( doit.out.size() ) std::cout<<">>> stdout:\n"<<doit.out<<std::endl;}
	if(v>=2){if( doit.err.size() ) std::cout<<">>> stderr:\n"<<doit.err<<std::endl;}
	auto ok = system(("ls -l "+libname+" "+ccode_tmpfile).c_str());
	if( doit.status == 0 && ok == 0 ){
		if(v>=2)std::cout<<" (removing the tmp file)"<<std::endl;
		//system(("rm -f "+ccode_tmpfile).c_str());
		if( remove(ccode_tmpfile.c_str()) != 0 ){ // posix 'rm' function call.
			THROW(strerror(errno));
		}
	}else{
            // create compilation error log file, appending to the '.c' file
            try{
                std::ofstream ofs(ccode_tmpfile, std::fstream::out | std::fstream::app);
                ofs<<"#if 0 // COMPILATION ERROR LOG\n"
                    <<"\nDllPipe tried to run a PstreamPipe with command:\n"
                    <<"\n\t"<<cmd<<"\n\n"
                    <<" The 'run' call returned status="<<doit.status
                    <<"-------- STDOUT --------\n"
                    <<doit.out<<"\n"
                    <<"-------- STDERR --------\n"
                    <<doit.err<<"\n"
                    <<"#endif"<<std::endl;
                ofs.close();
            }catch(...){
                std::cout<<"Problem writing compilation information to "<<ccode_tmpfile<<std::endl;
                throw;
            }
            if(v>=-1)std::cout<<" compilation info appended to "<<ccode_tmpfile<<std::endl;
            assert( ok == doit.status );
        }
	// check 'libname' exists and throw if not
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // JITPIPE_HPP
