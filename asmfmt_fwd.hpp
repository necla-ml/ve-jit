#ifndef ASMFMT_FWD_HPP
#define ASMFMT_FWD_HPP
#include "jitpage.h"
#include <iosfwd>
#include <string>
#include <stack>

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
/** simple assembly line formatting. -std=c++11.
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
 * */
class AsmFmtCols {
  public:
      AsmFmtCols();                           ///< destructor writes to cout
      /** destructor write to file instead of cout.
       * Use an empty string for no file/cout output when you want
       * to grab the output only as std::string */
      AsmFmtCols(std::string const& fname);
      ~AsmFmtCols();  ///< write to cout if !this->written, or constructor fname if there is one
      /** Can force a pre-destructor write (destructor won't write);
       * Any subsequent non-const function calls will throw an error.
       * To kill this formatter \em skipping cout, use \c flush(). */
      void write();
      /** return copy of internal ostringstream.
       * \note This returns a temporary so that directly calling \c c_str()
       * on the return value results in a \b dangling pointer. */
      std::string str() const;
      /** Silent pre-destructor \c write(), possibly with file output,
       * that returns the text as a std::string temporary. */
      std::string flush();
      /** Flush any code, and also output the \c stack_undefs. \return a temporary string */
      std::string flush_all() { pop_scopes(); return flush(); }
      //std::string flush_undefs(); // old name
      /** output #define text for these string pairs, and push a corresponding
       * string of #undef onto a scope-stack.
       * \return number of #defines in this scope (which could be zero)
       * \c pop_scope() emits the last bunch of #undef text.
       *
       * It is an error to destruct or early-destruct [via \c write() or \c flush()]
       * without having popped all scopes.  Or maybe we will auto-pop any remaining?
       */
      template<typename PAIRCONTAINER>
          std::size_t scope( PAIRCONTAINER const& pairs, std::string block_name="" );
      /** emit last set of #undefs. \return number of remaining stack-scopes
       * never errs (no-op if scope-stack is empty) */
      std::stack<std::string>::size_type pop_scope();
      /** pop all scopes (emit all active undefs) */
      void pop_scopes();

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

      AsmFmtCols& def(std::string const& symbol, std::string const& subst, std::string const& name=""); ///< #define symbol subst
      AsmFmtCols& undef(std::string const& symbol, std::string const& name="");    ///< might \em uncover a previous definition
      AsmFmtCols& raw(std::string const& anything);    ///< use this for stuff like cpp macros
      AsmFmtCols& lcom(std::string const& comment);    ///< left <// comment>
      AsmFmtCols& com(std::string const& comment);     ///< mid <// comment>
      AsmFmtCols& rcom(std::string const& comment);    ///< right <# comment>
      AsmFmtCols& lab(std::string const& label);       ///< <label:>
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
      friend void throw_if_written( AsmFmtCols const* asmfmt, std::string const& cannot );
      std::ostringstream *a;          ///< [cpp +] assembler code
      static char const* const ws;    ///< =" \t\r" within-statement whitespace (newline separates statement)
      static char const* const indent; ///< = "    ";
      static int const inwidth;       ///< indent width, 4
      static int const opwidth;       ///< = 12-1;
      //static std::streampos const argwidth;      ///< = 24-1;
      static int const argwidth;
      bool written;                   ///< track if user forced an early \c write();
      std::ofstream *of;              ///< optional file output (instread of cout)
      /** push with \c scope, pop with \c pop_scopes or pop_scope. */
      std::stack<std::string> stack_undefs;
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

#endif //CODEREMOVE < 1

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // ASMFMT_FWD_HPP
