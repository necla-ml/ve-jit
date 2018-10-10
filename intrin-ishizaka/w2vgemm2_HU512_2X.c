
#include "veintrin.h"

#define VLEN 256

void w2v_gemm2_HU512_2X( unsigned long n, unsigned long h, unsigned long w, float *A,  float *B, float *C ) 
{
  // assert( h<=512 && h%2==0 ) ;

  const unsigned long h2 = h>>1 ;
  
  unsigned int *pIA = (unsigned int *) A ;

  volatile unsigned long packed0 = 0x0000000100000001UL ;
  const unsigned long int packed = packed0 ;

#if 0
  _ve_lvl(h2) ;
  for( unsigned long i=0; i<w; i++ ) {
    __vr vc = _ve_vbrd_vs_f64(0.0) ;

    float *pB = B ;
    for(unsigned long j=0; j<n; j++) {
      const unsigned long sa = pIA[w*j+i] * packed ;
      const __vr  vb = _ve_vld(pB, 8) ;
      
      vc = _ve_pvfmad_vvsv(vc, sa, vb) ;

      pB += h ;
    }
    
    _ve_vst(C, vc, 8) ;
    C += h ;
  }
#else
  const unsigned long w0 = w ;

  _ve_lvl(h2) ;
  if( (w&0x01) == 1 ) { 
    __vr vc = _ve_vbrd_vs_f64(0.0) ;

    float *pB = B ;
    for(unsigned long j=0; j<n; j++) {
      const unsigned long sa = pIA[w0*j] * packed ;
      const __vr  vb = _ve_vld(pB, 8) ;
      
      vc = _ve_pvfmad_vvsv(vc, sa, vb) ;

      pB += h ;
    }
    pIA += 1 ;    

    _ve_vst(C, vc, 8) ;
    C += h ;
  }
  w = w>>1 ; 
  if( (w&0x01) == 1 ) { 
    __vr vc0 = _ve_vbrd_vs_f64(0.0) ;
    __vr vc1 = _ve_vbrd_vs_f64(0.0) ;

    float *pB = B ;
    for(unsigned long j=0; j<n; j++) {
      const unsigned long sa0 = pIA[w0*j]   * packed ;
      const unsigned long sa1 = pIA[w0*j+1] * packed ;
      const __vr  vb = _ve_vld(pB, 8) ;
      
      vc0 = _ve_pvfmad_vvsv(vc0, sa0, vb) ;
      vc1 = _ve_pvfmad_vvsv(vc1, sa1, vb) ;

      pB += h ;
    }
    pIA += 2 ;    

    _ve_vst(C, vc0, 8) ;
    C += h ;
    _ve_vst(C, vc1, 8) ;
    C += h ;
  }
  w = w>>1 ; 
  for(unsigned long i=0; i<w; i++) {
    __vr vc0 = _ve_vbrd_vs_f64(0.0) ;
    __vr vc1 = _ve_vbrd_vs_f64(0.0) ;
    __vr vc2 = _ve_vbrd_vs_f64(0.0) ;
    __vr vc3 = _ve_vbrd_vs_f64(0.0) ;

    float *pB = B ;
    for(unsigned long j=0; j<n; j++) {
      const unsigned long sa0 = pIA[w0*j]   * packed ;
      const unsigned long sa1 = pIA[w0*j+1] * packed ;
      const unsigned long sa2 = pIA[w0*j+2] * packed ;
      const unsigned long sa3 = pIA[w0*j+3] * packed ;
      const __vr  vb = _ve_vld(pB, 8) ;
      
      vc0 = _ve_pvfmad_vvsv(vc0, sa0, vb) ;
      vc1 = _ve_pvfmad_vvsv(vc1, sa1, vb) ;
      vc2 = _ve_pvfmad_vvsv(vc2, sa2, vb) ;
      vc3 = _ve_pvfmad_vvsv(vc3, sa3, vb) ;

      pB += h ;
    }
    pIA += 4 ;    

    _ve_vst(C, vc0, 8) ;
    C += h ;
    _ve_vst(C, vc1, 8) ;
    C += h ;
    _ve_vst(C, vc2, 8) ;
    C += h ;
    _ve_vst(C, vc3, 8) ;
    C += h ;
  }
#endif
}

