#ifndef ASMFMT_FWD_HPP
#define ASMFMT_FWD_HPP
#include "jitpage.h"
#include <iosfwd>
#include <string>
//#include <stack>
#include <vector>       // now can match #defines with regex
#include <regex>
#include <list>

// identify deprecated features...
#ifndef ASMFMT_USE_DEPRECATED
#define ASMFMT_USE_DEPRECATED 0
#endif

#if ASMFMTREMOVE < 2
/* terminal .S --> .bin or throw */
std::string fname_bin( std::string const& fname_S );

/** asm2bin : .S --> .bin . \post .bin file exists and has >0 bytes.
 * Makefile bin.mk should exist.
 * \pre You want a VE binary blob (so we use ncc/nas/nobjcopy/nobjdump)
 * \ret size of binary blob file created.
 * \throw on error. */
size_t asm2bin( std::string const& fname_S, int const verbose=1 );

/** read a binary blob file into a new code page */
struct ExecutablePage {
    ExecutablePage( std::string const& fname_bin );
    ~ExecutablePage();
    void *addr() const {return page.mem;}

    JitPage const page;
    static const int verbosity=0; 
};
#endif //CODEREMOVE < 2

/* assemble a .S file to a .bin file and load it into an ExecutablePage */
//ExecutablePage asm2page( std::string const& fname_S );


#if ASMFMTREMOVE < 1
/** Simple assembly line formatting. -std=c++11.
 * An \c ostringstream wrapper for nice-looking assembler output.
 *
 * Keeping adding labels, instructions and comments to build up
 * an output string.  When destroyed [or with \c write] the assembler
 * code string is output to \c cout [or a file, depending on constructor].
 *
 * Being a wrapper around std::ostringstream, you can also get
 * a copy of the internal string with \c str().  This way you can
 * pretty-format canned coded chunks and assemble them with greater
 * control.
 *
 * \c scope is meant to support a stack of #define #undef things,
 * (or maybe they can be implemented as asm macros?)
 *
 * Should flush() and write() return the formatter to empty state,
 * via a->clear(), a->set("") ?  Write now it is dead, unusable, inactive!
 *
 * \ref loops/fuse2.cpp for a demo of how to <em>wire in sub-kernel asm code to
 * preselected parent locations</em>.  This technique replaces the arbitrariy-deep
 * nesting and node search mechanisms of cblock.hpp.  Register alloc is done
 * <em>by hand</em>, just like in regular assembly programming.
 *
 * - The trick is to create \e separate AsmFmtCols for each section
 *   that a sub-kernel might want to add code to,
 * - and then at the end \e stitch together the AsmFmtCols outputs in
 *   correct order for the final program.
 *
 * - \ref cblock.hpp (to output C code) uses a tree-of-nodes with path-name
 *   lookups, a slightly different approach (allows nicely indented code output)
 *
 * \todo remove output file support (and 'cout' support too) since keeping
 * everything as string is really the best default policy.  Get rid of
 * 'write' and 'flush' and simplify the counterproductive 'written' state logic.
 */
class AsmFmtCols {
  public: // utility
      AsmFmtCols();                           ///< destructor writes to cout
#if ASMFMT_USE_DEPRECATED
      /** destructor write to file instead of cout.
       * Use an empty string for no file/cout output when you want
       * to grab the output only as std::string */
      AsmFmtCols(std::string const& fname);
#endif //ASMFMT_USE_DEPRECATED
      ~AsmFmtCols();  ///< write to cout if !this->written, or constructor fname if there is one
      /** default is to allow any same-named variables to alias.
       * this is a "root" property, so is we have a parent, we
       * propagate this setting to everyone.
       * \todo this is a root property that is \e inherited by
       * children (but we do not have a proper tree set up yet).
       */
      bool allow_alias() const { return this->_allow_alias; }
      bool allow_alias(bool const b) { return this->_allow_alias = b; }

      /** return copy of internal ostringstream.
       * \note This returns a temporary so that directly calling \c c_str()
       * on the return value results in a \b dangling pointer. */
      std::string str() const;

      /** Forcibly empty the output buffer */
      void clear();
#if ASMFMT_USE_DEPRECATED
      /** Silent pre-destructor \c write(), possibly with file output,
       * that returns the text as a std::string temporary. */
      std::string flush();
#else
      /** Same as str(), with an implied clear() */
      std::string flush(){ std::string ret=str(); flush(); return ret; }
#endif
      /** Flush any code, \b and also output the \c stack_undefs.
       * \return a temporary string */
      std::string flush_all() { pop_scopes(); return flush(); }

#if ASMFMT_USE_DEPRECATED
      /** Can force a pre-destructor write (destructor won't write);
       * Any subsequent non-const function calls will throw an error.
       * To kill this formatter \em skipping cout, use \c flush(). */
      void write();
#endif //ASMFMT_USE_DEPRECATED

      //
      /** output #define text for these string pairs, and push a corresponding
       * string of #undef onto a scope-stack.
       * \return number of #defines in this scope (which could be zero)
       * \c pop_scope() emits the last bunch of #undef text.
       *
       * It is an error to destruct or early-destruct [via \c write() or \c flush()]
       * without having popped all scopes.  Or maybe we will auto-pop any remaining?
       *
       * Note: you may \c def [and \e deprecated: \c undef] symbols one-by-one.
       *       Such definitions behave as though operating on the global
       *       scope; i.e. they add to stack_defs[0], stack_undefs[0].
       *
       * NEW: you can pattern-match the symbol substitutions. The intent is to
       *      allow searching for "all used registers matching...
       *      Ex. std::vector<std::string> vs = asm.match("^%v[:digit:]+.*"
       */
      template<typename PAIRCONTAINER>
          std::size_t scope( PAIRCONTAINER const& pairs, std::string const& block_name="" );
      /** emit last set of #undefs. \return number of remaining stack-scopes
       * never errs (no-op if scope-stack is empty) */
      std::vector<std::string>::size_type pop_scope();
      /** pop all scopes (emit all active undefs) */
      void pop_scopes();

      /** Search active macro \e substitutions that begin \c with.
       * Matches stop at the first [ \n\t#/;] character, so if a match with
       * "%s1\/\*foo\*\/" would return just plain "%s1".
       * We assume that '\#define' is only used to introduce macro substitutions.
       * \post returned strings all begin with \c with.
       */
      std::vector<std::string> def_words_starting(std::string with) const;
      /** like \c def_words_starting, but return cpp macros {name,word}s
       * instead of just words (words begin \c with). */
      std::vector<std::pair<std::string,std::string>> def_macs_starting(std::string with) const;
      /** all defined macros (this and parent[s]) */
      std::vector<std::pair<std::string,std::string>> def_macs() const;

      /** Set scoping parent, returning old pointer [default=NULL].
       * You can arrange AsmFmtCols as a nesting of scopes, so we can
       * consult \c parent to know about other in-scope \c def macros.
       * \return old AsmFmtCols \c parent (usually nullptr).
       */
      AsmFmtCols *setParent( AsmFmtCols* p );

      /// \group simple formatting
      ///{
      typedef struct {
          std::string label;
          std::string op;
          std::string args;
          std::string comment;
          std::string remain; ///< allow multi-statement, ';' as separator
      } AsmLine;
      static AsmLine parts(std::string const& instruction);        ///< split into op and args (only?)

      /** \#define symbol subst (into \b global scope) */
      AsmFmtCols& def(std::string const& symbol, std::string const& subst, std::string const name="");
      /** undef something explicitly. XXX not robust:
       * 1) cannot \em uncover a previous definition?
       * 2) might spit out duplicate undef lines?
       * Is this fn necesary?  Can \c push_scope, \c pop_scope do everything you need?
       * POSSIBLY useful for non-register macros.
       */
      AsmFmtCols& undef(std::string const& symbol, std::string const& name="");
      //@}
      AsmFmtCols& raw(std::string const& anything);    ///< use this for stuff like cpp macros or output from other AsmFmtCols 'kernels'
      AsmFmtCols& lcom(std::string const& comment);    ///< left <// comment>
      AsmFmtCols& com(std::string const& comment);     ///< mid <// comment>
      AsmFmtCols& rcom(std::string const& comment);    ///< right <# comment>
      AsmFmtCols& lab(std::string const& label, std::string const& comment="");       ///< <label:>
      AsmFmtCols& ins();                               ///< blank line
      AsmFmtCols& ins(std::string const& instruction); ///< <    ><op       >[<args>]
      /** <    ><op          >[<args>]<      # <asmcomment> */
      AsmFmtCols& ins(std::string const& instruction, std::string const& comment);
      ///}
      /// \group comment strings can also be a list of comment lines
      ///{
      template< typename... LComments >
          AsmFmtCols& lcom(std::string const& comment, LComments... lcomments){
              lcom(comment);
              lcom(lcomments...);
              return *this;
          }
      template< typename... Comments >
          AsmFmtCols& com(std::string const& comment, Comments... comments){
              com(comment);
              com(comments...);
              return *this;
          }
      template< typename... RComments >
          AsmFmtCols& rcom(std::string const& comment, RComments... rcomments){
              rcom(comment);
              rcom(rcomments...);
              return *this;
          }
      /** ```
       * <    ><op          >[<args>]<      # <asmcomment>
       * <                   // <comment2>i
       *                     ...
       * ``` */
      template< typename... RComments >
          AsmFmtCols& ins(std::string const& instruction, std::string const& comment,
                  std::string const& comment2, RComments... rcomments){
              ins(instruction,comment);
              rcom(comment2, rcomments...);
              return *this;
          }
      ///}
  protected:
      std::string fmt_def(std::string const& symbol, std::string const& subst, std::string const& name="");
      std::string fmt_undef(std::string const& symbol, std::string const& name);
  private:
      std::ostringstream *a;          ///< [cpp +] assembler code
      static char const* const ws;    ///< =" \t\r" within-statement whitespace (newline separates statement)
      static char const* const indent; ///< = "    ";
      static int const inwidth;       ///< indent width, 4
      static int const opwidth;       ///< = 12-1;
      //static std::streampos const argwidth;      ///< = 24-1;
      static int const argwidth;
#if ASMFMT_USE_DEPRECATED
      friend void throw_if_written( AsmFmtCols const* asmfmt, std::string const& cannot );
      bool written;                   ///< track if user forced an early \c write();
      std::ofstream *of;              ///< optional file output (instread of cout)
#endif //ASMFMT_USE_DEPRECATED
      /** push with \c scope, pop with \c pop_scopes or pop_scope.
       * Each element is a multiline-string (one or more '\#undef') */
      std::vector<std::string> stack_undefs;
      /** Each scope has a vector of macro --> substitution strings */
      struct StringPairs
          : public std::vector<std::pair<std::string,std::string>>
      {
          /// Convenient \c push_back trimmed versions of \c name and \c subst.
          void push_trimmed(std::string name, std::string subst);
      };
      /** Now that we maintain the original \c stack_defs mapping, we can
       * hide the undef string creation details. */
      std::string defs2undefs( StringPairs const& macs, std::string block_name );
      /** push with \c scope, pop with \c pop_scopes or pop_scope.
       * Each element represents one or macro definitions, delimited
       * by '\#define'.  This was added so the that the set of substitutions
       * could be \c match'ed to search for 'all defined regs'.
       *
       * - Invariant after all calls:
       *   - \c stack_defs.size()==stack_undefs.size()
       * - Every '\#define' in stack_defs[1,2,...] has a matching '\#undef',
       *   but not for stack_defs[0], the "global" scope, where you can
       *   manually add/remove defintions with \c def and \c undef.
       *
       * Hmmm.  Rather than parsing lines, it is more convenient to store
       * the raw strings (as a reverse mapping of \e subst --> \e macro name.
       */
      std::vector<StringPairs> stack_defs;
      AsmFmtCols *parent;             ///< optional scope parent to support \c def_words_starting
      bool _allow_alias; ///< probably should end up as a tree Root property?
};

/** add VE-specific optimizations */
struct AsmFmtVe : public AsmFmtCols
{
    static unsigned const MVL=256;
    AsmFmtVe& set_vector_length(uint64_t const vl);
};
#if 0
/** extend AsmFmtCols with scoped symbolic register names */
class AsmFmt : protected AsmFmtCols {
    AsmSym();
    AsmSym(std::string const&  fname);

};
#endif
/** remove asm comments (after '#'),
 * accepting ';'- or newline-separated multiline \c asmcode.
 * The returned string may be uglified, so you might need to
 * pass it through an \c AsmFmtCols::ins again.
 * Could be a static member fn? */
std::string uncomment_asm( std::string asmcode );

/// \group VE assembler helpers
//@{
/** \c out = replicated MSB (sign bit) of \c in.
 * \return code setting \c out to 0 or -1. */
std::string ve_signum64(std::string out, std::string in);
/** \c out = absolute value of int64_t \c in */
std::string ve_abs64(std::string out, std::string in);

/** This helper is unlike a kernel, because a base pointer remains valid
 * forever onward, perhaps even into data stored after the code area.
 * \p a assembler output container.
 * \p bp assembler register (Ex. %s33)
 * \p name of func (Ex. foo)*/
void ve_set_base_pointer( AsmFmtCols & a, std::string bp="%s34", std::string name="foo" );

/** \group VE load scalar register with constant */
//@{
struct OpLoadregStrings{
    std::string lea;
    std::string log;
    std::string shl;
    std::string ari;
    std::string lea2; ///< 2-op lea
    bool one_op() const;
    std::string choose() const;
};

/** return all types of found loadreg strings, according to instruction type */
OpLoadregStrings opLoadregStrings( uint64_t const parm );
/** use just one choice with some default instruction-type preference.
 * - register usage:  the string uses scalar registers
 *   - OUT      (#define as %s3 for \ref veli_loadreg.cpp tests)
 *   - T0	(tmp register, %s40 in \c veli_loadreg.cpp tests)
 *
 * so you could output a chunk to load 77 into a scalar register with:
 * ```
 * std::string load77;
 * {
 *     AsmFmtCols prog;
 *     prog.def("OUT",[name of your scalar output register]);
 *     prog.def("T0",[name of your scalar temporary register]);
 *     prog.ins(choose(opLoadregStrings(77))
 *     load77 = prog.flush_all();
 * }
 * ```
 * (oh.  T0 is no longer required)
 * Desired: comment as OUT = hexdec(77), stripping off the opLoadregStrings "debug" comments
 *
 * \p context  is for future use (ex. supply an instruction types of surrounding
 * instruction[s] so we can better overlap execution units)
 */
std::string choose(OpLoadregStrings const& ops, void* context=nullptr);

inline std::string OpLoadregStrings::choose() const {
    return ::choose(*this);
}
inline bool OpLoadregStrings::one_op() const {
    return !lea.empty()
        ||  !log.empty()
        ||  !shl.empty()
        ||  !ari.empty() ;
}

/** \b NOT optimized -- reduce dependence on other headers!
 * \c s is a VE register, \c v is the constant to load. */
std::string ve_load64_opt0(std::string s, uint64_t v);

/** load reg \c s with value \c v, optimized (but w/o context) */
std::string ve_load64(std::string s, uint64_t v);

/** set vector length (local \c tmp register if immN out of 0-127 range). */
std::string ve_set_vector_length(uint64_t immN, std::string tmp);

/** A common pattern is to pre-select any required registers into an \c AsmScope block,
 * and then shove all the assignments into AsmFmtCols::scope(block,name) */
typedef std::list<std::pair<std::string,std::string>> AsmScope;

enum RegSearch { SCALAR, SCALAR_TMP, VECTOR, VECTOR_TMP };
/** add a scalar register to \c block that does conflicts with neither \c block
 * nor \c asmfmt scopes (or parent scopes).
 *
 * - \c search_pattern SCALAR searches %s{{0-7},{34,63},{18,33},
 *   - VECTOR searches %v{{0-63}}
 *   - and *_TMP search in reverse order.
 */
void ve_propose_reg( std::string variable,
        AsmScope& block,        // add {variable,register} to block
        AsmFmtCols const& a,    // excluding registers known to block or a
        enum RegSearch const how );
/** A customized register search order. */
void ve_propose_reg( std::string variable,
        AsmScope& block,
        AsmFmtCols const& a,
        std::string prefix,
        std::vector<std::pair<int,int>> const search_order
        );

//@}
/* dispatch table based on 0 <= %s0 < nCases.
 * - Register Usage example:
 *   - CASE     %s0
 *   - CASE_ERR %s1
 * - %s0 out-of-range returns CASE_ERR = +/- 1; normally CASE_ERR is 0
 *
 * TODO return a vector of code entry points, where client can inject arbitrary
 *      assembler implementations.
 *
 * We need a ProgNode tree structure to make this easier!
 *
 * - i.e. something like a regs/progTree.hpp,
 * - or its path-based 'C' version in cblock.hpp)
 */

//@}

#endif //CODEREMOVE < 1

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // ASMFMT_FWD_HPP
