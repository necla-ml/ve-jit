
#include <iostream>
#include <sstream>
#include "stdint.h"
#include <iomanip>
#include <cassert>
using namespace std;

#define MVL 256
int64_t ve_vlen_suggest(int64_t const nitems){
    int const v=0; // verbose
    int64_t ret=MVL;
    //bool x0_check_vl = true;
    if( nitems <= MVL ){
        ret = nitems; // trivial: only once through the loop
    }else{
        int64_t const nFull = nitems/MVL;
        int64_t const nLoops = (nitems+MVL-1)/MVL;
        int64_t const rem   = nitems%MVL;
        if(v>0)cout<<" loop0.."<<nitems<<" nFull="<<nFull<<" rem="<<rem<<" nLoops="<<nLoops
    <<" nitems/nLoops="<<nitems/nLoops
      <<" nitems%nLoops="<<nitems%nLoops
      <<" (nitems+nLoops-1)/nLoops="<<(nitems+nLoops-1)/nLoops
      <<" vleq*nLoops-nitems="<<(nitems+nLoops-1)/nLoops*nLoops-nitems;
      ;
  // NO: assert( (nitems%nLoops == 0) == (nFull == nLoops) );
        if(rem+32 >= MVL){ // rem also large, latency already roughly equal.
      if(v>0)cout<<" [short]";
            ret = MVL;
        }else if( nitems%nLoops == 0 ){
            // this is "good enough" for vector latency, but more importantly
            // avoids some special handling for last-time-through-loop.
            ret = nitems/nLoops;
            //if(v>0)cout<<"loop 0.."<<nitems<<" vlen perfect division as "<<ret<<"*"<<nitems/ret<<endl;
      if(v>0) cout<<" [perfect]";
            //x0_check_vl = false;
        }else{ // redistribute...
            // we need some remainder, set up a "nice" main vector length
            // DO NOT use rem as loop entry value, only as last-time-through value.
            // [ vector inductions REQUIRE last-loop vlen <= main vlen ]
            int64_t vleq = (nitems+nLoops-1) / nLoops;
      //if(v>0)cout<<" vleq*nLoops-nitems="<<vleq*nLoops-nitems;
      assert( vleq * nLoops >= nitems );
            //
            // Note: above ensures that remainder (vector length for last pass)
            // is always less than vleq.
            // For example, N=257, MVL=256 should not use 128+129.
            // Instead, we propose 160+97
            //
            // Why? you will usually set up vector loop induction for the initial
            // vector length, which may then be inalid if vector length ever
            // increases (but stays OK if you shorten vector length).
            //
            // I.e. you can choose between
            // A. ignoring correct unused values, and
            // B. using incorrect [uncalculated?] extra values
            // during the last loop pass.  (A.) is much better.
            //
            //if(v>0)cout<<" vleq="<<vleq<<" = "<<nitems<<"/"<<nLoops<<" = nitems/nLoops"<<endl;
            assert( vleq <= MVL );
      ret = vleq;
      if(v>0)cout<<" [vleq]";
#if 1 // round-up-to-multiple-of-32 [optional]
            if( nitems%vleq == 0 ){
            }else{
                // guess latency increments for VE as vector length passes multiples of 32
                // so round "main" vector up to a multiple of 32 (take whatever remains as remainder)
                vleq = (vleq+31)/32*32;
                if(v>0)cout<<"vleq rounded up to "<<vleq<<endl;

                ret = vleq;
                if(v>0)cout<<"loop 0.."<<nitems<<" vlen redistributed from "<<MVL<<"*"<<nFull<<"+"<<rem
                    <<" to "<<vleq<<"*"<<nitems/vleq<<"+"<<nitems%vleq<<endl;
                // Paranoia: if we somehow increased loop count, this logic has a bug
                assert( (nitems+vleq-1)/vleq == nLoops );
            }
#endif
        }
  if(v>0)cout<<endl;
    }
    return ret;
}

// streamlined calculation, in prep for jitting this calc
uint64_t vej_vlen_suggest(uint64_t const nitems){
#define VEJ_VLEN_SUGGEST_VERBOSE 0
#if !VEJ_VLEN_SUGGEST_VERBOSE
  uint64_t ret = nitems;
  if((int64_t)nitems > MVL){
    if( nitems % MVL + 32U >= MVL ){
      ret = MVL;
    }else{
      uint64_t const nLoops = (nitems+MVL-1U)/MVL;
      ret = (nitems+nLoops-1U) / nLoops;
      if(ret * nLoops != nitems ){
        ret = (ret+31U)/32U*32U;
      }
    }
  }
  return ret;
#else
  uint64_t ret = nitems;
  if((int64_t)nitems > MVL){
    if( nitems%MVL + 32U >= MVL ){
      ret = MVL;
      //cout<<"A"<<ret;
    }else{
      int64_t const nLoops = (nitems+MVL-1)/MVL;
      //int64_t const nFull = nitems/MVL;
      //int64_t const rem   = nitems%MVL;
      ret = (nitems+nLoops-1) / nLoops;
      if(ret * nLoops != nitems ){
        ret = (ret+31U)/32U*32U;
        //cout<<"B"<<ret;
      }else{
        //cout<<"C"<<ret;
      }
    }
  }
  return ret;
#endif
#undef VEJ_VLEN_SUGGEST_VERBOSE
}
std::string vej_vlen_suggest(std::string var, uint64_t const nitems){
  ostringstream oss;
  uint64_t const vl = vej_vlen_suggest(nitems);
  oss<<var<<" = "<<vl<<"UL; // "
    <<"vej_vlen_suggest("<<nitems<<" items) as "<<nitems/vl<<" fulls";
  if(nitems%vl){
           oss<<" + "<<nitems%vl<<" rem";
  }
  return oss.str();
}
std::string vej_vlen_suggest(std::string var, std::string nitems){
  ostringstream oss;
  oss<<"{ // "<<var<<" = vej_vlen_suggest("<<nitems<<")\n";
  oss<<"  "<<var<<" = nitems;\n";
  oss<<"  if((int64_t)nitems > MVL){\n";
  oss<<"    if( nitems % MVL + 32U >= MVL ){\n";
  oss<<"      "<<var<<" = MVL;\n";
  oss<<"    }else{\n";
  oss<<"      uint64_t const nLoops = (nitems+MVL-1U)/MVL;\n";
  oss<<"      "<<var<<" = (nitems+nLoops-1U) / nLoops;\n"; // the only expensive division
  oss<<"      if("<<var<<" * nLoops != nitems ){\n";
  oss<<"        "<<var<<" = ("<<var<<"+31U)/32U*32U;\n";
  oss<<"    }\n";
  oss<<"  }\n";
  oss<<"}\n";
  return oss.str();
}
int main(int argc,char**argv){
  ostringstream oss;
  oss.clear();
  oss.str("Hello ? ");
  if(oss.good()){
    oss<<"Hello!"<<endl;
  }
  cout<<oss.str();
  cout<<"ve_vlen_suggest:"<<endl;
  for(int64_t i=-1; i<4000; ++i){
    int64_t const vl = ve_vlen_suggest(i);
    cout<<" "<<setw(3)<<right<<i<<":"<<setw(3)<<left<<vl;
    if(i%16==0) cout<<endl;
    int64_t const vl2 = vej_vlen_suggest(i);
    if( vl != vl2 ){
      cout<<endl<<" vl="<<vl<<"   but vej_vlen_suggest("<<i<<")="<<vl2<<endl;
    }
    assert( vl == vl2 );
  }
}
// vim: ts=2 sw=2 et
