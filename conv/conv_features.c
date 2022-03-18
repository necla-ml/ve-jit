// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#ifndef CCONVPARAM_C
// gcc and g++ did not quite agree on extern, inline and how to deal with
// multiple definitions.  One workaround was just to compile the 'C'
// sources as a monolithic file.  Then g++ -shared did not complain.
//
// So this code is included from cConvParam.c, even though it governs
// a logical distinct set of derived convolution capabilities.
#error "linkage issues mixxing C,C++ -- do not use this file directly"
#endif

#include "conv_features.h"
#include <stdio.h>
#include <string.h>

#if 0
#define mb batchNum
#define g group
#define ic inChannel
#define ih inHeight
#define iw inWidth
#define oc outChannel
#define oh outHeight
#define ow outWidth
#define kh kernHeight
#define kw kernWidth
#define sh strideHeight
#define sw strideWidth
#define ph padHeight
#define pw padWidth
#define dh dilationHeight
#define dw dilationWidth
#endif

#ifdef __cplusplus
extern "C" {
#endif

char const* const allFeature_names[] = {
    /* 0*/ "mb", "g", "ic", "ih", "iw", "oc", "oh", "ow",
    "kh", "kw", "sh", "sw", "ph1", "pw1", "dh1", "dw1",
    /*16*/ "1/mb", "1/g", "1/ic", "1/ih", "1/iw", "1/oc", "1/oh", "1/ow",
    "1/kh", "1/kw", "1/sh", "1/sw", "1/ph1", "1/pw1", "1/dh1", "1/dw1",
    /*32*/ "mb==1", "1<mb<=8", "mb<8", "mbic", "mbic<208",
    /*37*/ "mbocosz", "mbocosz<5152", "mbocosz<240384", "mbocosz<284736",
    /*41*/ "ic%1024==0", "icog<=1024", "ocog<=256", "iw<=256", // <-- new, required for vednnConvolutionOk.c (/ vednnConvolutionForward.c)
    /*45*/ "ohow<=16", "ohow<=4096", "vecC", // <-- new, for vecC test
    "iw2X", "ow2X", "ic==g", // for iw2XU256_ow2X impl, and '_c1' (ic==group)
    "ohOK", "owOK", // oh == (ih-kh)/sh + 1
    // note: error in vednnConvolution_ok was using ow<=256 for Fd1s1p0k3iw2XU256ow2Xal, so expect false negatives !
    /*53*/ "ow<=64", "ow<=128", "shsw", "s1", /*new*/"s2", /*gemm.1sw1*/"sw1",
    /*59*/ "ksq", "k1", "k2", "k3", "k4", "k5", "ksz", "ksz<3",
    /*67*/ "psq", "p0", "psqS",
    /*new:*/ "pS", /*"pwS",*/ "ohw==ihw", //"ow==iw",
    // /*72,new*/ "gemmk1p0", "gemmk1p0b", "noim2col", // remove if collapsing gemm:foo --> gemm
    /*72*/ "d1",
    // /*73*/ "thr", "thr==1", "thr>1",
    // /*73*/ "thr", "thr==1", "thr>1", "Itr_X", "Jit_X",
    /*73*/ "thr", "thr==1", "thr>1", "Gop", "Gop||", "Itr_X", "Jit_X",
    /*80*/
};
int64_t const allFeature_sz = sizeof(allFeature_names) / sizeof(char const*);
// In C, static_assert is for C11, compile with -std=c11
//static_assert( allFeature_sz == 80, "incorrect count of allFeature_names!" );
static_assert( sizeof(allFeature_names) / sizeof(char const*) == 80, "incorrect count of allFeature_names!" );

char const* const subFeature_values_names[] = {
    /* 0*/ "mb", "g", "ic", "ih", "iw", "oc", "oh", "ow",
    "kh", "kw", "sh", "sw", "ph1", "pw1", "dh1", "dw1",
    /*16*/ "1/mb", "1/g", "1/ic", "1/ih", "1/iw", "1/oc", "1/oh", "1/ow",
    "1/kh", "1/kw", "1/sh", "1/sw", "1/ph1", "1/pw1", "1/dh1", "1/dw1",
    /*32*/
};
int64_t const subFeature_values_sz = sizeof(subFeature_values_names) / sizeof(char const*);
//static_assert( subFeature_values_sz == 32, "incorrect count of convolution subFeature_values_names!" );
static_assert( sizeof(subFeature_values_names) / sizeof(char const*) == 32, "incorrect count of convolution subFeature_values_names!" );

void allFeature_dump()
{
    printf("\nallFeature_names[]:\n");
    for(int i=0; i<allFeature_sz; ++i){
        printf("   allFeatures[%2d] = %s\n", i, allFeature_names[i]);
    }
    fflush(stdout);
}

int allFeature_indices(
        /* inputs*/ char const* *feat, int const nfeat,
        /*outputs*/ int *idx, int* nidx )
{
    int *idx0 = idx;
    for(int f=0; f<nfeat; ++f){
        char const* const ff = feat[f];
        if (ff != NULL){
            // not a huge number, just do linear search
            for(int i=0; i<allFeature_sz; ++i){
                char const* const ai = allFeature_names[i];
                assert( ai != NULL ); // it's our C-string
                // so we don't need strncmp
                if (strcmp(ff, ai) == 0) // exact match only
                    *idx++ = i;
            }
        }
    }
    *nidx = idx - idx0;
    return *nidx == nfeat;
}

float ops_factor(int const mb, int const threads)
{
    // doItr "foo_X" and doJit "foo_X" decisions are based on
    // this estimate of "work per thread".
    float ops_mb_factor;
    {
        //int const mb = pConv->pParamIn->batch; // For mb-only ||ism
        int const mx = threads; //vednn_get_num_threads();
        int const nu = (mx<mb? mx: mb); //std::min(mx,mb);
        int const no = (mb+nu-1) / nu;  // round up mb/nu ~ # of full/fractional omp rounds
        // original, wrong? ops_mb_factor = 1.0 - (double)no / nu;
        ops_mb_factor = 1.0 - (double)no / mb; // tending to (1-1/mx) for large minibatch
    }
    return ops_mb_factor;
}

int allFeature_extract(
        /* inputs*/ ConvParamRaw const *r, int const *idx, int const nidx,
        /* aux in*/ int const threads, unsigned long long ops,
        /*outputs*/ float *vals )
{
    int err = 0;
    // debug: fill with magic value
    for(int i=0; i<nidx; ++i){
        vals[i] = -13.f;
    }

    int v=0;
    for(int fi=0; err==0 && fi<nidx; ++fi) {
        int const f = idx[fi];
        if (f<=36) switch(f){
            // Raw conv params (missing = dirn, layout)
            // These are converted STRICTLY POSITIVE floats
        case(0): vals[v++] = (float)r->batchNum; break;
        case(1): vals[v++] = (float)r->group; break;
        case(2): vals[v++] = (float)r->inChannel; break;
        case(3): vals[v++] = (float)r->inHeight; break;
        case(4): vals[v++] = (float)r->inWidth; break;
        case(5): vals[v++] = (float)r->outChannel; break;
        case(6): vals[v++] = (float)r->outHeight; break;
        case(7): vals[v++] = (float)r->outWidth; break;
        case(8): vals[v++] = (float)r->kernHeight; break;
        case(9): vals[v++] = (float)r->kernWidth; break;
        case(10): vals[v++] = (float)r->strideHeight; break;
        case(11): vals[v++] = (float)r->strideWidth; break;
        case(12): vals[v++] = (float)(r->padHeight + 1); break;
        case(13): vals[v++] = (float)(r->padWidth + 1); break;
        case(14): vals[v++] = (float)(r->dilationHeight-r->dil_start+1); break;
        case(15): vals[v++] = (float)(r->dilationWidth -r->dil_start+1); break;
                  // inverses
        case(16): vals[v++] = 1.0f / (float)r->batchNum; break;
        case(17): vals[v++] = 1.0f / (float)r->group; break;
        case(18): vals[v++] = 1.0f / (float)r->inChannel; break;
        case(19): vals[v++] = 1.0f / (float)r->inHeight; break;
        case(20): vals[v++] = 1.0f / (float)r->inWidth; break;
        case(21): vals[v++] = 1.0f / (float)r->outChannel; break;
        case(22): vals[v++] = 1.0f / (float)r->outHeight; break;
        case(23): vals[v++] = 1.0f / (float)r->outWidth; break;
        case(24): vals[v++] = 1.0f / (float)r->kernHeight; break;
        case(25): vals[v++] = 1.0f / (float)r->kernWidth; break;
        case(26): vals[v++] = 1.0f / (float)r->strideHeight; break;
        case(27): vals[v++] = 1.0f / (float)r->strideWidth; break;
        case(28): vals[v++] = 1.0f / (float)(r->padHeight+1); break;
        case(29): vals[v++] = 1.0f / (float)(r->padWidth+1); break;
        case(30): vals[v++] = 1.0f / (float)(r->dilationHeight-r->dil_start+1); break;
        case(31): vals[v++] = 1.0f / (float)(r->dilationWidth -r->dil_start+1); break;
                  // mb
        case(32): vals[v++] = (float)(r->batchNum == 1);
                  break;
        case(33): vals[v++] = (float)(r->batchNum > 1 && r->batchNum <= 8);
                  break;
        case(34): vals[v++] = (float)(r->batchNum < 8);
                  break;
                  // mbic
        case(35): vals[v++] = (float)(r->batchNum * r->inChannel);
                  break;
        case(36): vals[v++] = (float)(r->batchNum * r->inChannel < 208);
                  break;
        }
        else if(f<=40){ // mbocosz
            int mbocosz = r->batchNum * r->outChannel * r->outHeight * r->outWidth;
            switch(f) {
            case(37): vals[v++] = (float)mbocosz;
                      break;
            case(38): vals[v++] = (float)(mbocosz < 5152);
                      break;
            case(39): vals[v++] = (float)(mbocosz < 240384); // !
                      break;
            case(40): vals[v++] = (float)(mbocosz < 284736); // !
                      break;
            }
        }
        else if(f<=75) switch(f) {
            // new, other things for vednnConvolutionForward.c (or vednnConvolutionOk.c)
            //assert(i==41);
        case(41): vals[v++] = (float)(r->inChannel % 1024 == 0); // (not req'd?) used for Fd1s1pSk3_c1024x_T
                  break;
        case(42): vals[v++] = (float)(r-> inChannel / r->group <= 1024);
                  break;
        case(43): vals[v++] = (float)(r->outChannel / r->group <= 256);
                  break;
        case(44): vals[v++] = (float)(r->inWidth <= 256);
                  break;
        case(45): vals[v++] = (float)(r->outHeight * r->outWidth <= 16); // part of vecC test
                  break;
        case(46): vals[v++] = (float)(r->outHeight * r->outWidth <= 4096); // from _ok.c
                  break;
        case(47): vals[v++] = (float)(r->outHeight * r->outWidth <= 16
                          || (r->outHeight * r->outWidth < 64
                              && r->outHeight * r->outWidth <= r->inChannel)); // full vecC condition
                  break;
        case(48): vals[v++] = (float)((r-> inWidth & 0x1) == 0); //iw2X
                  break;
        case(49): vals[v++] = (float)((r->outWidth & 0x1) == 0); //ow2X
                  break;
        case(50): vals[v++] = (float)(r-> inChannel == r->group); //ic1
                  break;
        case(51): vals[v++] = (float)(r->outHeight == (r->inHeight - r->kernHeight) / r->strideHeight + 1);
                  break;
        case(52): vals[v++] = (float)(r->outWidth == (r->inWidth - r->kernWidth) / r->strideWidth + 1);
                  break;
                  // ow, stride
        case(53): vals[v++] = (float)(r->outWidth <= 64); // 48? 53?
                  break;
        case(54): vals[v++] = (float)(r->outWidth <= 128); // 256?
                  break;
                  //int const sh = r->strideHeight, sw = r->strideWidth;
                  //int const ssz = sh*sw;
        case(55): vals[v++] = (float)(r->strideHeight * r->strideWidth);
                  break;
        case(56): vals[v++] = (float)(r->strideHeight * r->strideWidth == 1); // sh==sw==1
                  break;
        case(57): vals[v++] = (float)(r->strideHeight==2 && r->strideWidth==2); // new
                  break;
        case(58): vals[v++] = (float)(r->strideWidth==1); // new, for gemm.1sw1
                  break;
                  // kernel bools
                  //int const kh = r->kernHeight, kw = r->kernWidth;
                  //bool ksq = (kh == kw);
        case(59): vals[v++] = (float)(r->kernHeight == r->kernWidth); // ksq
                  break;
        case(60): vals[v++] = (float)(r->kernHeight == r->kernWidth && r->kernHeight==1);
                  break;
        case(61): vals[v++] = (float)(r->kernHeight == r->kernWidth && r->kernHeight==2);
                  break;
        case(62): vals[v++] = (float)(r->kernHeight == r->kernWidth && r->kernHeight==3);
                  break;
        case(63): vals[v++] = (float)(r->kernHeight == r->kernWidth && r->kernHeight==4);
                  break;
        case(64): vals[v++] = (float)(r->kernHeight == r->kernWidth && r->kernHeight==5);
                  break;
                  //int ksz = kh * kw;
        case(65): vals[v++] = (float)(r->kernHeight * r->kernWidth);
                  break;
        case(66): vals[v++] = (float)(r->kernHeight * r->kernWidth < 3); // from tree!
                  break;
                  // pad bools
        case(67): vals[v++] = (float)(r->padHeight == r->padWidth);         // psq
                  break;
        case(68): vals[v++] = (float)(r->padHeight==0 && r->padWidth==0);   // p0
                  break;
        case(69): vals[v++] = (float)(r->padHeight == r->padWidth           // psq
                          && r->kernHeight == r->kernWidth            // ksq
                          && r->kernHeight == r->padHeight*2+1 );     // psqS 
                  break;
                  //case(): vals[v++] = (float)(kh == ph*2 + 1); //phS  (new)
                  //case(): vals[v++] = (float)(kw == pw*2 + 1); //pwS  (new
        case(70): vals[v++] = (float)(r->padHeight == (r->kernHeight-1)/2 // phS
                          && r->padWidth == (r->kernWidth-1)/2); // phS & pwS for _padsame
                  break;
                  //case(): vals[v++] = (float)(pw == (kw-1)/2 ); // pwS
        case(71): vals[v++] = (float)(r->outHeight == r->inHeight && r->outWidth == r->inWidth);
                  break;
                  //case(): vals[v++] = (float)(r->outWidth  == r->inWidth );
        case(72): vals[v++] = r->dilationHeight == r->dil_start
                    && r->dilationWidth == r->dil_start; // no dilation
                  break;
        case(73): vals[v++] = threads;
                  break;
        case(74): vals[v++] = (float)(threads==1);
                  break;
        case(75): vals[v++] = (float)(threads>1);
                  break;
        }
        // Note: d1s1pS has strangeness for even kernel sizes (asymmetric padding
        //       is required to not keep same output size).
        //       I notice dil1_str1_padsame_ker2 is never run (it may have incorrect code!)

        // Some attributes corresponded to now-removed gemm "species" selection
        //
        // Since gemm impls are internally decided, should NOT try to predict
        // which gemm impls actually got used. XXX
        //
        // kernHeight+kernWidth==2 && padHeight+padWidth==0 && strideHeight+strideWidth>2 // k1p0_str
        //dest[i++] = (float)(kh+kw==2 && ph+pw==0 && sh+sw>2); // k1p0_str
        // channels < 2*nThreads && channels%nThreads && channels*output_h > 1 
        //dest[i++] = (float)((c.inChannel / c.group) * c.outHeight > 1);
        // (kernWidth == 1 && kernHeight == 1 && strideWidth == 1 && strideHeight == 1 && padWidth == 0 && padHeight == 0) // no_im2col
        //dest[i++] = (float)(c.kernWidth==1 && c.kernHeight==1
        //        && c.strideWidth==1 && c.strideHeight==1
        //        && c.padWidth==0 && c.padHeight==0);

#if 0
        else if(f==72){ // dil bools
            // "no dilation"
            //int dh=r->dilationWidth - r->dil_start + 1;
            //int dw=r->dilationHeight - r->dil_start + 1;
            //int no_dil = (dh==1 && dw==1);
            //vals[v++] = no_dil;
            vals[v++] = r->dilationHeight == r->dil_start
                    && r->dilationWidth == r->dil_start;
        }
        else if(f<=76) switch(f) {
            case(73): vals[v++] = threads;
            case(74): vals[v++] = (float)(threads==1);
            case(75): vals[v++] = (float)(threads>1);
            //case(76): vals[v++] = ops * 1.e-9;
        }
        else if (f==76) {
            if (ops == 0) ops = count_ops(r);
            vals[v++] = ops * 1.e-9;
        }
#endif
        else if(f<=79) {
            if (ops == 0)
                ops = count_ops(r);
            if(f==76){
                vals[v++] = ops * 1.e-9;
            }else{
                float const ops_mbpara = ops * ops_factor(r->batchNum, threads);
                switch(f) {
                case(77): vals[v++] = ops_mbpara * 1.e-9;
                          break;
                case(78): vals[v++] = ops_mbpara < 8.e7;  // doItr "foo_X" unthreaded
                          break;
                case(79): vals[v++] = ops_mbpara < 1.e10; // doJit "foo_X"
                          break;
                }
            }
        }
    }
    return err;
}

static int allFeatures_idx_32bools[32] = {
   32, 33, 34,              //["mb==1", "1<mb<=8", "mb<8", #"mbic<208",
                            // #"mbocosz<5152", "mbocosz<240384", "mbocosz<284736",
   41, 42, 43, 44,          // "ic%1024==0", "icog<=1024", "ocog<=256", "iw<=256",
   45, 46, 47,              // "ohow<=16", "ohow<=4096", "vecC",
   48, 49, 50,              // "iw2X", "ow2X", "ic==g",
   51, 52,                  // "ohOK", "owOK",
   53, 54, 56, 57, 58,      // "ow<=64", "ow<=128", "s1", "s2", "sw1",
   59, 60, 61, 62, 63, 64, 66, // "ksq", "k1", "k2", "k3", "k4", "k5", "ksz<3",
   67, 68, 69,              // "psq", "p0", "psqS",
                            // #"gemmk1p0", "gemmk1p0b", "noim2col",
                            // #"thr==1", "thr>1",
   78, 79                   // "Itr_X", "Jit_X"
};                          //]

/** prefilt0.py used these feature names:
 *
 * @pre vals[allFeature_sz]
 * @pre bools[32]
```
feat_bool1 = ["mb==1", "1<mb<=8", "mb<8", #"mbic<208",
              #"mbocosz<5152", "mbocosz<240384", "mbocosz<284736",
              "ic%1024==0", "icog<=1024", "ocog<=256", "iw<=256",
              "ohow<=16", "ohow<=4096", "vecC",
              "iw2X", "ow2X", "ic==g",
              "ohOK", "owOK",
              "ow<=64", "ow<=128", "s1", "s2", "sw1",
              "ksq", "k1", "k2", "k3", "k4", "k5", "ksz<3",
              "psq", "p0", "psqS",
              #"gemmk1p0", "gemmk1p0b", "noim2col",
              #"thr==1", "thr>1",
              "Itr_X", "Jit_X"
              ]
```
 */
void allFeatures_extract_32bools(
        float const* const vals, // vals[allFeature_sz]
        float * const bools      // --> bools[32] (0/1 values, as it happens)
        )
{
    for(int i=0; i<32; ++i){
        bools[i] = vals[allFeatures_idx_32bools[i]];
    }
}

int allFeatures(
        /* inputs*/ ConvParamRaw const *r,
        /* aux in*/ int const threads, unsigned long long ops,
        /*outputs*/ float *vals )
{
    assert(v==0);
    // NOTE: dh,dw and ph,pw begin at 1 so inverses can be taken
    int mb = r->batchNum;
    int g  = r->group;
    int ic = r->inChannel;
    int ih = r->inHeight;
    int iw = r->inWidth;
    int oc = r->outChannel;
    int oh = r->outHeight;
    int ow = r->outWidth;
    int kh = r->kernHeight;
    int kw = r->kernWidth;
    int sh = r->strideHeight;
    int sw = r->strideWidth;
    int ph = r->padHeight;
    int pw = r->padWidth;
    int dh = (r->dilationHeight-r->dil_start+1);
    int dw = (r->dilationWidth -r->dil_start+1);


    int v=0;
    vals[v++] = (float)mb;
    vals[v++] = (float)g;
    vals[v++] = (float)ic;
    vals[v++] = (float)ih;
    vals[v++] = (float)iw;
    vals[v++] = (float)oc;
    vals[v++] = (float)oh;
    vals[v++] = (float)ow;
    vals[v++] = (float)kh;
    vals[v++] = (float)kw;
    vals[v++] = (float)sh;
    vals[v++] = (float)sw;
    vals[v++] = (float)(ph+1); // so inv exists
    vals[v++] = (float)(pw+1); // so inv exists
    vals[v++] = (float)dh; // 1-based
    vals[v++] = (float)dw; // 1-based
    assert(v==16);
    // inverses
    for (/*v=16*/; v<32; ++v) {
        vals[v] = 1.0f / vals[v-16];
    }
    assert(v==32);
    // mb
    vals[v++] = (float)(mb == 1);
    vals[v++] = (float)(mb > 1 && mb <= 8);
    vals[v++] = (float)(mb < 8);
    // mbic
    vals[v++] = (float)(mb * ic);
    vals[v++] = (float)(mb * ic < 208);
    assert(v==37);
    int const mbocosz = mb * oc * oh * ow;
    vals[v++] = (float)mbocosz;
    vals[v++] = (float)(mbocosz < 5152);
    vals[v++] = (float)(mbocosz < 240384); // !
    vals[v++] = (float)(mbocosz < 284736); // !
    // new, other things for vednnConvolutionForward.c (or vednnConvolutionOk.c)
    assert(v==41);
    vals[v++] = (float)(ic % 1024 == 0); // (not req'd?) used for Fd1s1pSk3_c1024x_T
    vals[v++] = (float)(ic / g <= 1024);
    vals[v++] = (float)(oc / g <= 256);
    vals[v++] = (float)(iw <= 256);
    vals[v++] = (float)(oh * ow <= 16); // part of vecC test
    vals[v++] = (float)(oh * ow <= 4096); // from _ok.c
    assert(v==47);
    vals[v++] = (float)(oh * ow <= 16
            || (oh * ow < 64
                && oh * ow <= ic)); // full vecC condition
    vals[v++] = (float)((iw & 0x1) == 0); //iw2X
    vals[v++] = (float)((ow & 0x1) == 0); //ow2X
    vals[v++] = (float)(ic == g); //ic1
    vals[v++] = (float)(oh == (ih - kh) / sh + 1);
    vals[v++] = (float)(ow == (iw - kw) / sw + 1);
    assert(v==53);
    // ow, stride
    vals[v++] = (float)(ow <= 64); // 48? 53?
    vals[v++] = (float)(ow <= 128); // 256?
    int const ssz = sh*sw;
    vals[v++] = (float)(ssz);
    vals[v++] = (float)(ssz == 1); // sh==sw==1
    vals[v++] = (float)(sh==2 && sw==2); // new
    vals[v++] = (float)(sw==1); // new, for gemm.1sw1
    assert(v==59);
    // kernel bools
    //int const kh = kh, kw = kw;
    int ksq = (kh == kw);
    vals[v++] = (float)(ksq); // ksq
    vals[v++] = (float)(ksq && kh==1);
    vals[v++] = (float)(ksq && kh==2);
    vals[v++] = (float)(ksq && kh==3);
    vals[v++] = (float)(ksq && kh==4);
    vals[v++] = (float)(ksq && kh==5);
    int ksz = kh * kw;
    vals[v++] = (float)(ksz);
    vals[v++] = (float)(ksz < 3); // from tree!
    // pad bools
    vals[v++] = (float)(ph == pw);         // psq
    vals[v++] = (float)(ph==0 && pw==0);   // p0
    assert(v==69);
    vals[v++] = (float)(ph == pw           // psq
            && ksq                         // ksq
            && kh == ph*2+1 );             // psqS 
    //vals[v++] = (float)(kh == ph*2 + 1); //phS  (new)
    //vals[v++] = (float)(kw == pw*2 + 1); //pwS  (new
    vals[v++] = (float)(
            ph == (kh-1)/2 // phS
            && pw == (kw-1)/2); // phS & pwS for _padsame
    //vals[v++] = (float)(pw == (kw-1)/2 ); // pwS
    vals[v++] = (float)(oh == ih && ow == iw);
    //vals[v++] = (float)(ow  == iw );
    //vals[v++] = (float)(r->dilationHeight == r->dil_start
    //            && r->dilationWidth == r->dil_start); // no dilation
    vals[v++] = (float)(dh==1 && dw==1); // no dilation
    assert(v==73);
    vals[v++] = threads;
    vals[v++] = (float)(threads==1);
    vals[v++] = (float)(threads>1);
    assert(v==76);
    if (ops == 0) ops = count_ops(r);
    vals[v++] = ops * 1.e-9;
    float const ops_mbpara = ops * ops_factor(mb, threads);
    vals[v++] = ops_mbpara * 1.e-9;
    vals[v++] = ops_mbpara < 8.e7;  // doItr "foo_X" unthreaded
    vals[v++] = ops_mbpara < 1.e10; // doJit "foo_X"
    assert( v==80 );
    return 0;
}



#ifdef __cplusplus
}//extern "C"
#endif
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
