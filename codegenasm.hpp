#ifndef CODEGENASM_HPP
#define CODEGENASM_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "stringutil.hpp"
#include "velogic.hpp" // fully-tested "load scalar reg with constant"
#include "asmfmt_fwd.hpp"
#include <array>
/** this class returns raw string for assembler ops */
struct CodeGenAsm
{
  public:
      CodeGenAsm() : used(), latest(0) {
          used.fill(0);
      }
      /** scoped tmp reg */
      struct TmpReg
      {
          TmpReg( CodeGenAsm* owner );    ///< allocate and mark used an entry
          ~TmpReg();
          //operator std::string() const { return str; }
          CodeGenAsm * const owner;
          int const tmp;
          //char const* const creg;
          std::string const str;
      };
      struct TmpReg tmp() { return TmpReg(this); }

      /** return asm string to load \c value into scalar register \c reg.
       * \todo There is tested code that covers all reasonable optimizations,
       *       and it should be used to load constants into scalar registers!!!
       */
      std::string load( std::string const reg, uint64_t value );
  private:
      /** return and mark used an entry in tmpRegs */
      int alloc() {
          int f = (latest+1)%10;
          for(int i=0; i<10; ++i){
              if( !used[f] ){
                  used[f] = 1;
                  return latest = f;
              }
          }
          THROW(" too many tmp regs in use");
      }
      void free(int const u){
          used[u] = 0;
      }
      char const* tmpRegs[10] = {
          "%s40","%s41","%s42","%s43","%s44",
          "%s41","%s42","%s43","%s44","%s45"
      };
      std::array<int,10> used;
      int latest;         ///< latest allocated reg
};

    inline CodeGenAsm::TmpReg::TmpReg( CodeGenAsm* owner )
: owner(owner), tmp(owner->alloc()), str(std::string(owner->tmpRegs[tmp]))
{
}
inline CodeGenAsm::TmpReg::~TmpReg() {
    owner->free(tmp);
}
inline std::string CodeGenAsm::load( std::string const reg, uint64_t value ){
#if 1
    return multiReplace("OUT",reg,
            choose(opLoadregStrings(value)));
#else
    std::ostringstream oss;
    if( (int64_t)value >= -64 && (int64_t)value <= 63 ){
        auto t=tmp();
        oss<<"or "<<reg<<","<<(int64_t)value<<","<<jitimm(0);
    }else if( isimm(value) ){
        auto t=tmp();
        oss<<"or "<<reg<<","<<0<<","<<jitimm(value);
    }else if( value < (uint64_t{1}<<32) ){
        oss<<"lea "<<reg<<","<<jitdec(value);
    }else if( (int64_t)value < (int64_t)(int32_t)value ){
        // sext(32-bit) can yield 64-bit value exactly
        oss<<"lea "<<reg<<","<<jitdec(value);
        //else search for xor I,M
        //else search for eqv I,M
        //else search for nnd I,M
        //else search for add I,M
        //else search for sub I,M
        //else search for mpy I,M
        // ?? 2 instruction LEA ??
    }else{
        // needs testing !!! -- often have an explicit shl !!!
        uint32_t hi = (value>>32);
        uint32_t lo = (value & ((uint64_t{1}<<32)-1));
        if((int32_t)lo >= 0){ // sext(lo) has zeros in hi word
            auto t=tmp();
            oss<<"lea.sl "<<t.str<<",0x"<<std::hex<<hi<<"#(TmpReg);";
            oss<<"lea "<<reg<<",0x"<<std::setfill('0')<<std::setw(8)<<lo<<"(,"<<t.str<<")";
        }else{
            auto t=tmp();
            oss<<"lea.sl "<<t.str<<",0x"<<std::hex<<~hi<<";"; // sext of lo will toggle hi bits to OK
            oss<<"lea "<<reg<<","<<std::setfill('0')<<std::setw(8)<<lo<<"(,"<<t.str<<")";
        }
    }
    return oss.str();
#endif
            }
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break 
#endif //CODEGENASM_HPP
