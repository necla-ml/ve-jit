/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "dllbuild.hpp"
//#include "cblock.hpp"    // prefix_lines (debug output)
#include "throw.hpp"
#include "jitpage.h"    // low level 'C' utilities
#include <fstream>
#include <cstring>
#include <assert.h>
#include <unistd.h>     // getcwd, sysconf, pathconf, access
#include <sys/stat.h>   // stat (possibly faster than 'access') and I check size

#define PSTREAMS 0
#if PSTREAMS
#include "pstreams-1.0.1/pstream.h"
#endif

/** need a better way to [easily] disable unrolled compiles */
#define UNROLLS 1

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

// When inlined from cblock.hpp I got linker errors (std::string::size
// multiple defn of Cblock::append or so !!!!!)
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
std::string DllFile::short_descr() const {
    std::ostringstream oss;
    oss <<basename<<suffix
        <<" "<<code.size()<<" code bytes, "
        <<syms.size()<<" symbols";
    return oss.str();
}
/** Unfortunately, multiple compilations rely on bin.mk file rule details,
 * so now we return a vector of objects. */
std::vector<std::string> DllFile::obj(std::string fname){
    std::vector<std::string> ret;
    size_t p, pp=0;
    std::vector<char const*> alts;
    cout<<" Dllfile::obj(\""<<fname<<"\")..."<<endl; cout.flush();
    if((p=fname.rfind('-'))!= std::string::npos){
        string objsuffix("-ve.o");
        if(0){ ;
        }else if((p=fname.rfind("-x86.c"))==fname.size()-6){
            pp=p; objsuffix="-x86.o";
        }else if((p=fname.rfind("-x86.cpp"))==fname.size()-8){
            pp=p; objsuffix="-x86.o";
        }else if((p=fname.rfind("-vi.c"))==fname.size()-5){
            pp=p;
            // need a better way to disable unrolled compiles XXX
#if UNROLLS
            alts.push_back("_unroll-ve.o");
#endif
        }else if((p=fname.rfind("-vi.cpp"))==fname.size()-7){ pp=p;
        }else if((p=fname.rfind("-ncc.c"))==fname.size()-6){ pp=p;
        }else if((p=fname.rfind("-ncc.cpp"))==fname.size()-8){ pp=p;
        }else if((p=fname.rfind("-clang.c"))==fname.size()-8){ pp=p;
        }else if((p=fname.rfind("-clang.cpp"))==fname.size()-10){ pp=p;
        }
        if(pp){ // all make fname[0:pp)+"-ve.o", and perhaps some alts
            ret.push_back(fname.substr(0,pp).append(objsuffix));
            for(auto const& suffix: alts){
                ret.push_back(fname.substr(0,pp).append(suffix));
            }
        }
    }
    if(ret.empty()){
        // if we still haven't recognized the source file assume Aurora
        // [maybe get rid of this dubious assumption?]
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
            <<") must match %[-vi|-ncc|-clang|-x86].{c|cpp} or %.{s|S} (see bin.mk rules)");
    return ret;
}
/** \b new: if file exists and "same", don't rewrite it */
std::string DllFile::write(SubDir const& subdir){
    int const v = 1;
    string myfile;
    {
        std::ostringstream oss;
        oss <<"//Dllfile: basename = "<<basename
            <<"\n//Dllfile: suffix   = "<<suffix
            <<"\n//Dllfile: abspath  = "<<abspath;
        if(v>1){cout<<" generating comment: "<<comment<<endl; cout.flush();}
        oss <<"\n"<<comment;
        if(v>1){cout<<" copying code: "<<code<<endl; cout.flush();}
        oss <<"\n"<<code
            <<endl;
        myfile = oss.str();
    }

    this->abspath = subdir.abspath + "/" + this->basename + this->suffix;

    // check file existence, and whether we can skip the rewrite
    bool writeit = true;
    {
        struct stat st;
        if(stat(abspath.c_str(), &st)){
            cout<<" my size "<<myfile.size();
            if((size_t)st.st_size == myfile.size()){ // file size matches => "same" (hack)
                writeit = false;
                if(v>1)
                    cout<<" matches existing file, so NOT overwriting";
            }else{
                if(v>1) cout<<" differs from existing file size "<<st.st_size;
            }
        }else{
            if(v>1) cout<<" and target file does not exist";
        }
    }

    if(writeit){
        try{
            std::ofstream ofs(abspath);
            if(v>1){cout<<" ofstream ofs("<<abspath<<") CREATED"<<endl; cout.flush();}
            ofs<<myfile;
            if(v>1){cout<<" and an extra ofs.flush() !!!"<<endl; cout.flush();}
            ofs.flush();
            ofs.close();
        }catch(...){
            cout<<" Trouble writing file "<<abspath<<endl;
            throw;
        }
        if(v>0){cout<<" Wrote file "<<abspath<<endl;}
    }
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

/** debug: show some state */
void DllBuild::dump(std::ostream& os){
    if(this->empty()){
        os<<"\nDllBuild has no symbols set up yet";
    }else{
        os<<"\nDllBuild expects symbols...\n";
        for(auto const& df: *this){
            os<<"   File: "<<df.basename<<df.suffix<<" :"<<endl;
            for(auto const& sym: df.syms){
                os<<"      "<<sym.symbol<<endl;
                if(!sym.comment.empty()){prefix_lines(os, sym.comment, "        // ");os<<endl;}
                if(!sym.fwddecl.empty()){prefix_lines(os, sym.fwddecl, "        ");os<<endl;}
            }
        }
    }
    if(prepped){
        os<<"DllBuild::prep (or skip_prep) has been called."<<endl;
        os<<"          subdir   "<<dir.subdir<<endl;
        os<<"          basename "<<basename<<endl;
        os<<"          libname  "<<libname<<endl;
        os<<"          mkfname  "<<mkfname<<endl;
        os<<"          fullpath "<<fullpath<<endl;
        os<<"          target library "<<getLibName()<<endl; // might throw
    }
    if(made){
        os<<"DllBuild::make() has been called (makefiles should exist)."<<endl;
    }
}

/** Generating the Makefile
 *
 * - adds a prefix to canned rules of \e bin.mk Makefile template
 * - notably, \e all: target is a shared library
 * - bin.mk now can produce multiple clang versions.
 * - also creates basename.OBJECTS so Makefile can circumvent command line limits
 *
 * \c skip added so existing Makefile or source files would not get
 * rewritten.
 */
void DllBuild::prep(string basename, string subdir/*="."*/){
    if(empty()){
        cout<<" Nothing to do for dll "<<basename<<endl;
        return;
    }
    this->dir      = SubDir(subdir);
    this->basename = basename;
    this->libname  = "lib"+this->basename+".so";
    string archive = "lib"+this->basename+".a";  // NEW
    this->mkfname  = this->basename+".mk";
    this->fullpath = dir.abspath+"/"+libname;

    ostringstream mkfile;
    // at_file is so mkfile can use @FILE way to avoid command line length limits
    string at_filename(this->basename+".OBJECTS");
    ostringstream at_file;
    
    at_file<<"\n";
    mkfile<<"# Auto-generated Makefile for "<<libname;
    mkfile<<"\nLIBNAME:="<<libname
        <<"\nARCHIVE:="<<archive
        //<<"\nLDFLAGS:=$(LDFLAGS) -shared -fPIC -Wl,-rpath="<<dir.abspath<<" -L"<<dir.abspath
        <<"\nLDFLAGS:=-shared -fPIC -Wl,-rpath="<<dir.abspath<<" -L"<<dir.abspath<<" $(LDFLAGS)";
    mkfile<<"\n.PHONY: hello goodbye all\n"
        <<"all: hello $(ARCHIVE) $(LIBNAME) goodbye\n";
    // Let's be even more careful about duplicates (multiply-defined-symbols if identical code)
    for(size_t i=0U; i<size(); ++i){
        auto& df_i = (*this)[i];
        if( df_i.basename.empty() ){
            cout<<" DllFile "<<i<<" had no basename (removing)"<<endl;
            continue;
        }
        for(size_t j=0U; j<i; ++j){
            auto& df_j = (*this)[j];
            if( df_i.basename == df_j.basename ){
                if( df_i.suffix == df_j.suffix){
                    if( df_i.code == df_j.code && df_i.syms.size() == df_j.syms.size() ){
                        cout<<" Duplicate DllFile "<<i<<" matches "<<j<<", IGNORED "<<i
                            <<"\n    prev: "<<df_j.short_descr()
                            <<"\n    skip: "<<df_i.short_descr()
                            <<endl;
                    }else{
                        cout<<" Duplicate DllFile "<<i<<" vs "<<j<<"! code/syms do not match "
                                <<"\n    prev: "<<df_j.short_descr()
                                <<"\n    skip: "<<df_i.short_descr();
                        // This might means you have incorrectly generated the function name.
                        // So print both versions to help debug.

                        cout<<"\ndf_i.syms -----------------------------------------------------------\n";
                        for(auto const& sym: df_i.syms){
                            cout<<"      "<<sym.symbol<<endl;
                            if(!sym.comment.empty()){ cout<<"        // "<<sym.comment<<endl; }
                        }
                        cout<<"\ndf_j.syms -----------------------------------------------------------\n";
                        for(auto const& sym: df_j.syms){
                            cout<<"      "<<sym.symbol<<endl;
                            if(!sym.comment.empty()){ cout<<"        // "<<sym.comment<<endl; }
                        }
                        cout<<"\ndf_i.code -----------------------------------------------------------\n"
                            <<df_i.code
                            <<endl;
                        cout<<"\ndf_j.code -----------------------------------------------------------\n"
                            <<df_j.code
                            <<endl;
                        THROW(" Duplicate DllFile "<<i<<" vs "<<j<<"! code/syms do not match "
                                <<"\n    prev: "<<df_j.short_descr()
                                <<"\n    skip: "<<df_i.short_descr());
                    }
                }else{
                    cout<<" Duplicate DllFile "<<i<<" w/ different suffix from "<<j<<", IGNORED "<<i
                        <<"\n    prev: "<<df_j.short_descr()
                        <<"\n    skip: "<<df_i.short_descr()
                        <<endl;
                }
#define DLLBUILD_ERASE_SUSPICIOUS 1
#if DLLBUILD_ERASE_SUSPICIOUS
                df_i.basename.clear(); // mark for full erasure from build
                break;
#else // keep it around, but disable its syms and library inclusion (for debug?)
                // UNTESTED CODE
                std::ostringstream oss;
                oss<<df_i.suffix<<".dup_"<<i<<'_'<<j;
                df_i.suffix = oss.str(); // remove by change suffix and clearing syms
                cout<<"    changed DllFile "<<i<<" suffix to "<<df_i.suffix<<endl;

                df_i.comment = "REMOVED "+df_i.comment;
                df_i.syms.clear();
                cout<<"    and clearing its symbols"<<endl;
                // keep the tag? or punt it to the old one?  (not sure)

                if(0){ // with changed suffix and empty syms, we can still write a file
                    // in case it helps debug...
                    oss.str("");
                    oss<<"#if 0 /* Test file "<<i<<" removed, similar to prev "<<j
                        <<"\n    prev: "<<(*this)[ j ].short_descr()
                        <<"\n    skip: "<<(*this)[ i ].short_descr()
                        <<df_i.code
                        <<"\n#endif /* test removed! */";
                    df_i.code = newcode.str();
                }
#endif
                break; // important!
            }
        }
    }
    if(DLLBUILD_ERASE_SUSPICIOUS) { // fully remove the DllFile?
        for(auto it=begin(); it!=end(); ){
            if( it->basename.empty() ){
                it = erase(it);
            }else{
                cout<<" Keeping: "<<it->basename<<it->suffix<<endl;
                ++it;
            }
        }
    }
    {
        ostringstream sources; sources<<"\nSOURCES:=";
        ostringstream objects; objects<<"\nOBJECTS_FILE:="<<at_filename<<"\nOBJECTS:=";
        ostringstream deps;    deps   <<"\n";
        ostringstream hello;   hello  <<"\n";
        ostringstream goodbye; goodbye<<"\ngoodbye:\n"
            <<"\t@echo 'Goodbye, "<<mkfname<<" ending'\n";
#define DLLBUILD_SIMPLE_RENAMES 0
#if DLLBUILD_SIMPLE_RENAMES
        //
        // To think about...
        //
        // By creating rename files for every possible target, we will always generate
        // an alternate unroll_FUNC call for every FUNC in FUNC-vi.c.
        // Downside:
        //   1. you only want some unroll_FUNCs
        //   2. FUNC names are not obtained from the sourcefile stem
        //
        // current method allows client-specified symbols to be renamed, but can
        // still run into problems with multiply defined symbols for the *other* FUNCs
        //
        // XXX Eventually, will want also to objcopy -N <name>  to --strip-symbol
        // the alternate-compile symbols that we want to remove
        //
        hello<<
            "\n$(patsubst %-vi.c,%_unroll-ve.o.rename,$(SOURCES)): %_unroll-ve.o.rename:: %-vi.c"
            "\n\t@# assume stem exactly matches the function name!"
            "\n\techo '$* unroll_$*' > $@"
            "\nhello: | $(patsubst %-vi.c,%_unroll-ve.o.rename,$(SOURCES))"
            "\n\techo 'Hello, cjitConv.mk begins'"
            ;
#endif
        // get "already-known" symbols
        void* mainSyms = dlopen(nullptr,RTLD_LAZY);
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

            string dfSourceFile = df.basename+df.suffix; // no "." because suffix could be "-vi.c"
            df.objects = DllFile::obj(dfSourceFile); // checks name correctness
            // A source file might produce several objects by different compile options
            // The symbols should be renamed to coexist in a single dll
            //
            // objcopy with a rename file seems best way to rename
            // compile-option-differentiated symbols.
            //
            vector<std::string> dfRenames(df.objects.size());
            // Find alternate symbols and their rename script recipes
            {
                vector<SymbolDecl> altsyms;
                for(size_t i=0U; i<df.objects.size(); ++i){
                    string object = df.objects[i];
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
                        dfRenames[i] = rename.str();
                    }
                }
                // append all altsyms, from any alternate object files
                if(!altsyms.empty()){
                    df.syms.reserve(df.syms.size() + altsyms.size());
                    for(auto const s: altsyms)
                        df.syms.push_back(s);
                }
            }

            df.abspath = dir.abspath+'/'+dfSourceFile;
            // Are all symbols already available to us?
            //   For example, if executable is linked again libmegajit.so,
            //   we can avoid re-compiling if all object file symbols already known.
            bool df_know_all_symbols = true;
            {
                cout<<"Symbol check for "<<dfSourceFile<<endl;
                for(auto const& sym: df.syms){
                    // do I need to use handle to specific lib?
                    //string megalib=getPath()+"/libmegajit.so";
                    //void* mainSyms = dlopen(megalib.c_str(), RTLD_LAZY);
                    void* addr = dlsym(mainSyms, sym.symbol.c_str());
                    char const* dlerr=dlerror();
                    if(dlerr){
                        cout<<"    main symbol "<<sym.symbol<<" is unknown"<<endl;
                        df_know_all_symbols  =  false;
                        break;
                    }else{
                        cout<<"    main symbol "<<sym.symbol<<" is knonwn @ "<<addr;
                    }
                }
            }

            if( !df_know_all_symbols ){
                sources<<" \\\n\t"<<dfSourceFile;
                // add objects, at_file, deps and renames to build recipe
                for(size_t i=0U; i<df.objects.size(); ++i){
                    string object = df.objects[i];
                    objects<<" \\\n\t"<<object; // add to makefile OBJECTS:=
                    at_file<<object<<"\n";      // also add to @FILE to circumvent command line limits
                    deps<<"\n"<<object<<": "<<dfSourceFile;
                    auto const& renames = dfRenames[i];
                    if(renames.size()){
                        std::cout<<" XXX renames[obj "<<i<<" "<<object<<"] = <"<<renames<<">"
                            <<" --> mkfile!\n";
                        string renameFile(object);
                        renameFile.append(".rename");
                        mkfile<<"\n"<<renameFile<<":"
                            <<"\n\t@rm -f "<<renameFile<<"\n"
                            <<renames;
                        hello<<"\nhello: |"<<renameFile; // create this FIRST (only if not present)
                    }else{
                        std::cout<<" XXX renames[obj "<<i<<" "<<object<<"] = EMPTY <"<<renames<<">\n";
                    }
                }
                std::cout<<"Writing source file[s]\n";
                df.write(this->dir);            // source file input (throw if err)
            }
        }

        // Final assembly of the makefile from sources, deps, objects, ...
        mkfile<<"\n#sources\n"<<sources.str()<<endl;
        mkfile<<"\n#deps   \n"<<deps   .str()<<endl;
        mkfile<<"\n#objects\n"<<objects.str()<<endl;
        mkfile<<hello.str();
        mkfile<<"\nhello:\n\techo 'Hello, "<<mkfname<<" begins'\n";
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
                ofs.flush();
                ofs.close();
            }else{
                THROW(" Trouble constructing ofs("<<absmkfile<<")");
            }
        }catch(...){
            cout<<" Trouble writing file "<<absmkfile<<endl;
            throw;
        }
        cout<<" makefile written: "<<absmkfile<<endl;
    }

    // NOTE alternate is to write the @FILE as a series of 'echo' commands, right
    // into the makefile itself.
    { // write at_file.str() to <dir.abspath>/<at_filename>
        std::string absatfile;
        try{
            absatfile = dir.abspath+"/"+at_filename;
            ofstream ofs(absatfile);
            if(ofs){
                //ofs << mkfile.rdbuf();
                ofs << at_file.str();
                ofs.flush();
                ofs.close();
            }else{
                THROW(" Trouble constructing ofs("<<absatfile<<")");
            }
        }catch(...){
            cout<<" Trouble writing file "<<absatfile<<endl;
            throw;
        }
        cout<<" @FILE list of object files written: "<<absatfile<<endl;
    }
    // sometimes the 'make' does nothing ??? check that files are really there.
    {
        auto ls = "ls -l "+dir.abspath+" *.mk lib*";
        if(system(ls.c_str())!=0) cout<<"Issues with command `"<<ls<<"`"<<endl;
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
    string archive = "lib"+this->basename+".a";  // NEW
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
static int try_make( std::string mk, std::string mklog, int const v/*verbose*/ ){
    string mk_cmd = mk;
#if 1
    auto ret = system(mk_cmd.c_str());
    if(ret){
        //THROW(" Build error: "+mk);
        cout<<" Build error: "<<mk_cmd<<endl;
        cout<<" Build ret  : "<<ret<<endl;
    }
    return ret;
#else // pstreams to capture stdout, stderr into log files etc.
    if(!mklog.empty())
        mk_cmd.append(" >& ").append(mklog);
    cout<<" Build command: "<<mk_cmd<<endl;

    using namespace redi;
    redi::pstream mkstream(mk_cmd,
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
    if(error || status){
        cout<<" Please check build log in "<<mklog<<endl;
    }
    if(0){ //Is following correct?
        // even in quiet mode mode: write 'make' output into a file:
        std::ofstream ofs(mklog, ios_base::app);
        ofs<<string(40,'-')<<" stdout:"<<endl<<out<<endl;
        ofs<<string(40,'-')<<" stderr:"<<endl<<err<<endl;
        ofs.flush();
        ofs.close();
    }
    if(v>0 || error || status){
        cout<<string(40,'-')<<" stdout:"<<endl<<out<<endl;
    }
    if(v>1 || error || status){
        cout<<string(40,'-')<<" stderr:"<<endl<<err<<endl;
        cout<<" Make error  = "<<error <<endl;
        cout<<" Make status = "<<status<<endl;
    }
    if(error||status){
        //THROW(" Make failed! status="<<status<<" error="<<error);
        // no we will retry, and try to keep going...
        cout<<"\n\n WARNING: please check build logs for more info"
            <<"\n            We will try to continue anyways.\n"<<endl;
    }
    return status | error;
#endif
}
void DllBuild::make(std::string env){
    int const v = 0;
    if(!prepped)
        THROW("Please prep(basename,dir) before make()");
    std::string mklog = dir.abspath+"/"+mkfname+".log";
    string mk = env+" make VERBOSE=1 -C "+dir.abspath+" -f "+mkfname;
    if(v>0){cout<<" Make command: "<<mk<<endl; cout.flush();}
    //system(("ls -l "+dir.abspath).c_str()); // <-- unsafe c_str usage
    int bad = try_make(mk,mklog,v);
    if(bad){
        mklog.append("2");
        cout<<"Trying make once again..."<<endl;
        bad = try_make(mk, mklog, 2);
        if(bad){
            cout<<"BUILD ERROR! see "<<mklog<<endl;
            THROW("Build error");
        }
    }
    if(v>0){cout<<" 'make' ran in: "<<dir.abspath<<endl;}
    made = true;
}
void DllBuild::skip_make(std::string env){
    if(!prepped)
        THROW("Please prep(basename,dir), or at least skip_prep(), before make()");
    string mk = env+" make VERBOSE=1 -C "+dir.abspath+" -f "+mkfname;
    cout<<" Make command: "<<mk<<endl; cout.flush();
    if(access(this->fullpath.c_str(),R_OK)){
        cout<<" Oh-oh. library "<<fullpath<<" is not there.  Attempting rebuild"<<endl;
        cout.flush();
        this->make(env);
    }else{
        cout<<" Re-using existing library "<<fullpath<<endl;
    }
    made = true;
}
DllOpen::DllOpen() : basename(), libHandle(nullptr), dlsyms(), libname(), files() {
    cout<<" +DllOpen"; cout.flush();
}
DllOpen::~DllOpen() {
    int const v = 0;
    if(v){cout<<" -DllOpen"; cout.flush();}
    if(libHandle) dlclose(libHandle);
    libHandle = nullptr;
    if(v){cout<<" back from dlclose\n"; cout.flush();}
    //files.clear();
}
std::unique_ptr<DllOpen> DllBuild::dllopen(){
    int const v = 1;
    //using cprog::prefix_lines;
    if(v)cout<<"*** DllBuild::dllopen() BEGINS"<<endl;
    if(!(prepped and made))
        THROW("Please prep(basename,dir) and make() before you dllopen()");
    std::unique_ptr<DllOpen> pRet( new DllOpen );
    DllOpen& ret = *pRet;
    ret.libname = this->libname;
    if(v>1){
        cout<<"DllBuild::dllopen() calling dlopen... libname="<<libname<<endl;
        cout<<"DllBuild::dllopen() calling dlopen... fullpath="<<fullpath<<endl;
        cout.flush();
    }
    if(access(fullpath.c_str(),R_OK))
        THROW(" Cannot read file "<<fullpath);
    else
        if(v>1){cout<<"Good, have read access to fullpath"<<endl; cout.flush();}

#if JIT_DLFUNCS // but means libjit1 has a dependency on libdl
    if(v>1){
        //
        // Check right away that dlopen *can* succeed [debug]
        //
        if(v>1){cout<<" debug... dlopen_rel + dl_dump..."<<endl;}
        //void * dbg = dlopen_rel( fullpath.c_str(), RTLD_LAZY );
        void * dbg = dlopen( fullpath.c_str(), RTLD_LAZY );
        if(!dbg) {
#if !defined(__ve)
            cout<<" x86 program won't be able to open a VE library"<<endl;
#endif
            THROW(" dlopen failed!");
        }
        if(v>2){
            dl_dump(dbg);
            cout<<" debug... back from dl_dump..."<<endl;
            dlclose(dbg);
        }
    }
#endif

    // TODO test dlopen machine architecture match
#if !defined(__ve)
    //cout<<" DllBuild::dllopen cut short -- not running on VE "<<endl;
    //pRet.reset(nullptr);
    //return pRet;
    cout<<" DllBuild::dllopen [NEW] trying to handle x86 library"<<endl;
#endif

    if(v>1){cout<<"now dlopen..."<<endl; cout.flush();}
    ret.libHandle = dlopen(fullpath.c_str(), RTLD_LAZY);
    if(v>0){cout<<"DllBuild::dllopen() dlopen(fullpath, RTLD_LAZY) OK"<<endl; cout.flush();}
    if(!ret.libHandle){ std::ostringstream oss; oss<<"failed dlopen("<<fullpath<<") dlerror "<<dlerror(); cout<<oss.str()<<endl; cout.flush(); THROW(oss.str()); }
    //
    // Now go through all our expected symbols and actually
    // obtain their address, before we return
    //
    int nerr=0; // symbol load error count
    if(v>0){cout<<"Library: "<<this->libname<<endl; cout.flush();}
    ret.dlsyms_num.resize(this->size()); // create 'i'-->vector<symbolName> entries
    for(size_t i=0U; i<this->size(); ++i) // for each source file
    {
        auto const& df = (*this)[i];
        auto const filepath = df.getFilePath();
        if(v>1){cout<<"   File: "<<df.basename<<df.suffix
            <<"\n       : "<<filepath<<endl;
            cout.flush();
        }
        ret.files.push_back(filepath);
        ret.tag.push_back(df.tag);
        for(auto const& sym: df.syms){
            dlerror(); // clear previous error [if any]
            void* addr = dlsym(ret.libHandle, sym.symbol.c_str());
            if(v>0){
                cout<<"   Symbol: "<<sym.symbol<<" @ "<<addr<<"\n";
                if(!sym.comment.empty())
                    prefix_lines(cout,sym.comment,"        // ")<<"\n";
                if(!sym.fwddecl.empty())
                    prefix_lines(cout,sym.fwddecl,"        ")<<"\n";
                cout.flush();
            }
            char const* dlerr=dlerror();
            if(dlerr){
                cout<<"**Error: could not load symbol "<<sym.symbol<<endl;
                ++nerr;
            }
            if(ret.dlsyms.find(sym.symbol)!=ret.dlsyms.end()){
                cout<<"**Error: duplicate symbol "<<sym.symbol<<endl;
                ++nerr;
            }
            ret.dlsyms.insert(make_pair(sym.symbol,addr));
            ret.dlsyms_num[i].push_back(sym.symbol);
        }
    }
    if(v){cout<<"*** DllBuild::dllopen() DONE : nerr="<<nerr<<endl; cout.flush();}
    if(nerr) THROW(nerr<<" symbol load errors from "<<libname);
    return pRet;
}

/** debug: create, but on error dump and throw.
 * \post return value evaluates as \c true.  */
std::unique_ptr<DllOpen> DllBuild::safe_create(
        std::string basename,
        std::string dir /*="."*/,
        std::string env /*="" */)
{
    // ORIGINALLY we just threw whenever...
    //if(!prepped){prep(basename,dir); prepped=true;}
    //if(!made){make(); made=true;}
    //return dllopen();
    // NOW we catch (with nicer debug) and and rethrow
    unique_ptr<DllOpen> plib;
    int err=0;
    try{ // dllbuild will throw on errors. provide some debug info if so
        // call individual steps, if necessary
        if(!prepped){
            this->prep(basename,dir);
            // Note: doesn't allow skip_prep
            prepped = true;
        }
        if(!made){
            this->make();
            made=true;
        }
        // before actual 'create', print what we expect to find
        //cout<<"Library: "<<this->getLibName()<<endl;
        plib = dllopen();
        assert(plib);
    }
    catch(exception& e){ cout<<"DllBuild exception: "<<e.what()<<endl; ++err; }
    catch(std::string& e){ cout<<"DllBuild exception: "<<e<<endl; ++err; }
    catch(char const* e){ cout<<"DllBuild exception: "<<e<<endl; ++err; }
    catch(...){ cout<<"DllBuild threw(?)"<<endl; ++err; }
    if(err){
        cout<<"Tried DllBuild::safe_create(basename,dir,env)\n"
            <<" with basename = basename\n"
            <<"      dir      = "<<this->dir.subdir<<"\n"
            <<"      env      = "<<env /* not used ? !!! */
            <<endl;
        dump(cout);
        THROW("error: DllBuild failed");
    }
    return plib;
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

#include "stringutil.hpp" // multiReplace

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

    unique_ptr<DllOpen> pLib;
    {
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
        pLib = dllbuild.create();
    }
    if( !pLib ){
#if !defined(__ve)
        cout<<" STOPPING EARLY: we are an x86 executable and should not load a VE .so"<<endl;
        //cout<<" Warning: DllBuild for x86 is a new feature..."<<endl;
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
