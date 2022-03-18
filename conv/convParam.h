#ifndef CONVPARAM_H
#define CONVPARAM_H
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

/** mark unset ConvParam values, as in convParamRaw_unset. */
#define CONVPARAM_UNSET -1

//#if defined(CONVPARAM_CLIB) 
#if defined(CCONVPARAM_C)
#define CONVPARAM_EXTERN
#define CONVPARAM_INLINE
#else
#define CONVPARAM_EXTERN extern
#define CONVPARAM_INLINE extern inline __attribute__((always_inline))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* valid `dil_start` values have two conventions */
enum DilationStart {
    DIL_ONEDNN=0,
    DIL_VEDNN=1,
};

/// return value for convParamCheck()
enum ConvParamCheck {
    CONVCHECK_ERROR=0,  ///< invalid convolution
    CONVCHECK_SPARSE=1, ///< output memory layout with holes (sparse)
    CONVCHECK_DENSE=2,  ///< most stringent, dense layout (req'd for libvednn)
};

/// libvednn supports abx [=nchw] and axb [=nhwc]
enum Layout {
    LAYOUT_UNDEF = CONVPARAM_UNSET,
    // LAYOUT_AUTO match whatever src is, at runtime?

    LAYOUT_ABX = 0, // i.e. batch, channel, spatial
    LAYOUT_AXB = 1,
            // for 2D:
    LAYOUT_NCHW = 0,
    LAYOUT_NHWC = 1,
};

/// OneDNN supports many convolution directions.
/// libVednn supports \e forward and \e backward_weights (?)
/// how do forward_data & forward_both match with these?
/// or is the 'B' for bias?
enum PropKind {  // see dnnl::prop_kind
    PROPKIND_UNDEF = CONVPARAM_UNSET,
    PROPKIND_FORWARD = 0,
    PROPKIND_FORWARD_TRAINING = 0, // alias
    // to cover different impl code in libvednn
    PROPKIND_FORWARD_BIAS = 0, // "FB" forward bias impls
    PROPKIND_FORWARD_DATA=1,   // "FD" forward data impls
    PROPKIND_FORWARD_NOBIAS=1, // alias

    PROPKIND_FORWARD_INFERENCE = 1,
    //PROPKIND_FORWARDBIAS_INFERENCE = ,
    PROPKIND_BACKWARD=4, // wrt all parameters 'both' "BB"
    PROPKIND_BACKWARD_DATA=5,           // vednn has this "BD"
    PROPKIND_BACKWARD_WEIGHTS=6,        // vednn has this "BW"
    PROPKIND_BACKWARD_BIAS=7,           // (irrelevant for convolution?)
};

/** convolution parameters for "C". */
struct ConvParamRaw_s {
    // OneDNN dilations start at 0; libvednn, at 1
    int dil_start; // equiv "is_vednn", dilation start point
    int dirn;      // enum PropKind (we focus on FORWARD)
    int layout;    // enum Layout ly;
    // the usual parameters
    int batchNum;
    int group;
    int inChannel;
    int inHeight;
    int inWidth;
    int outChannel;
    int outHeight;
    int outWidth;
    int kernHeight;
    int kernWidth;
    int strideHeight;
    int strideWidth;
    int padHeight;
    int padWidth;
    int dilationHeight; /* "no dilation" is 1 (OneDNN value + 1) */
    int dilationWidth;
};

typedef struct ConvParamRaw_s ConvParamRaw;

extern struct ConvParamRaw_s const convParamRaw_unset;

/** For legibility we'll apply a bunch of shortenings to
 * the `layer` strings (implementation names).  This follows mechanisms
 * in `dtree.R`, `short_layerNames` roughly.  For speed we'll hardwire
 * many of the translations to avoid regexps. */
void layer_short(char const* layer, char *buf, int bufsz);


//
// The C api is somewhat klunky to use.
//

/// Return 2 if consistent/dense, 1 if ok-but-inconsistent, 0 if crazy.
/// libvednn assumes dense memory layouts,
/// whereas OneDNN permits stride > dimensional
enum ConvParamCheck convParamCheck( ConvParamRaw const* const r);

// These work with dil_start==0 or 1
char const* to_cstr( ConvParamRaw const* const r,
        char * const buf, size_t const n);
char const* to_cstr_short( ConvParamRaw const* const p,
        char * const buf, size_t const n);
ConvParamRaw* cvt_vednn( ConvParamRaw* const r );
ConvParamRaw* cvt_onednn( ConvParamRaw* const r );

// caller supplies height/width versions of
// i,k,s,p,d = dims for input, kernel, stride, pad, dilation
int compute_out(int i, int k, int s, int p, int d, int dil_start);
// caller supplies height/width versions of
// o,i,k,s,d = dims for input, output, kernel, stride, dilation
int compute_pad(int o, int i, int k, int s, int d, int dil_start);

int compute_oh( ConvParamRaw const* r );
int compute_ow( ConvParamRaw const* r );
int compute_ph( ConvParamRaw const* r );
int compute_pw( ConvParamRaw const* r );
unsigned long long count_ops(ConvParamRaw const* const r);

/** Generic read from string, with optional override. \sa readConvParamRaw. */
int readConvParamOvr( char const* const _cstr,
        ConvParamRaw const* ovr,
        ConvParamRaw* const r, char** const name);

/** read a OneDNN/vednn `_cstr` into `r` (and maybe `name`).
 * *name should be NULL, and if name ends up non-NULL you must free it.
 *
 * Three steps:
 *
 * - preparse the string
 * - fill in missing values
 * - does not ensure consistency:
 *   - use \c mkConsistent or \c mkConsistentOverrides
 *     for transforming \c r into a usable libvednn convolution test.
 *
 * It is recommended to keep STRINGS in OneDNN format to avoid confusion.
 * So `is_vednn==0` is recommended.  This may disappear since it is possible
 * to mark such strings with a "ve" prefix.
 *
 * After \c readConvParamRaw you can always \c cvt_vednn for libvednn tests.
 * @return error if nonzero.
 * \post if no error, ConvParamRaw with dil_start==0 (OneDNN convention)
 *
 * canonical form:
 * [ve]dYgXmbXicXihXiwXocXohXowXkhXkwXshXswXphXpwXdhXdwXnS
 *
 * where: Y = {fd, fb, bd, bw, bb}, X is number, S - string
 * note: symbol `_` is ignored, and `#` always terminates
 *
 * - optional prefix "ve" means dilations begin at 1 (libvednn convention)
 * - dY "direction" TBD
 *
 * implicit rules, when tag-integer values not specified
 *  - default values:
 *      Y = fd~0 (forward data) (todo)
 *      mb = 2, g = 1, ic = oc = 1,
 *      kh = kw = 3, sh = sw = 1, dh = dw = 0,
 *      S="" (was "wip")
 *  - if H is undefined => H = W
 *  - if W is undefined => W = H
 *  - if padding is undefined => compute trivial padding (else pad=0)
 *  - if `output` is undefined => compute output
 *  - todo: OneDNN also has id, od & other depths for 3D convolutions
 *
 *  \deprecated parameter \c is_vednn ; use "ve" prefix on \c _cstr instead.
 */
int readConvParamRaw( char const* const _cstr,
        ConvParamRaw* const r, char** const name);

/** after preparse of \c _cstr, apply anything set in overrides \c ovr. */
/** Move set values from ovr into r and make a consistent convolution.
 * Dumbly copy unset (!= -1) values in ovr into r.  
 * @return number of overrides
 */
int apply_overrides( ConvParamRaw *r, ConvParamRaw const *ovr);

/** Make p into a consistent libvednn test (if it isn't already).
 * libvednn tests assume dense layout, or "exact" output sizing.
 * @return number of changes
 */
int mkConsistent( ConvParamRaw* p );

/** apply_overrides followed by a mkConsistent that tries hard
 * not to change the overridden entries.
 * @return number of changes
 */
int mkConsistentOverrides( ConvParamRaw *r, ConvParamRaw const *ovr);

//
// simple inlines
//
CONVPARAM_INLINE int is_vednn( ConvParamRaw const* r ){
    return r->dil_start==1;
} 
/** change to dil_start==0 (OneDNN) and return the modified r. */
CONVPARAM_INLINE ConvParamRaw* cvt_onednn( ConvParamRaw* const r ){
    int d = 0 - r->dil_start;
    r->dilationHeight += d;
    r->dilationWidth += d;
    r->dil_start += d; // this is now 0, the OneDNN convention
    return r;
}

/** change to dil_start==1 and return the modified r */
CONVPARAM_INLINE ConvParamRaw* cvt_vednn( ConvParamRaw* const r ){
    int d = 1 - r->dil_start;
    r->dilationHeight += d;
    r->dilationWidth += d;
    r->dil_start += d; // this is now 1, the vednn convention
    return r;
}

CONVPARAM_INLINE int readConvParamRaw( char const* const _cstr,
        ConvParamRaw* const r, char** const name)
{
    return readConvParamOvr( _cstr, NULL, r, name );
}

#if 0 // ORIGINALS (with comments re OneDNN vs veddnn)
inline int
compute_out( int i, int k, int s, int p, int d ){
    // convolution min output size:
    //    mkl-dnn dilation convention ~ d==0 for "no dilation"
    //return (i - ((k - 1) * (d + 1) + 1) + 2 * p) / s + 1;
    //    libvednn convention
    //
    // Better: if out size should be zero detect -ve
    // (round-to-negative-infinity signed division would be better here)
    //
    int numer = (i - ((k - 1) * (d + 0) + 1) + 2 * p);
    return numer < 0
        ? 0
        : numer / s + 1;
    // mkl-dnn deconv: return (i - 1) * s + (k - 1) * (d + 1) + 2 * p + 1;
}
inline int
compute_pad( int o, int i, int k, int s, int d ){
    // note: swap o and i for deconv -- libvednn convention only!
    int numer = (o-1)*s - i + (k-1)*(d) + 1; // for vednn (dil_start 1)
    return numer < 0
        ? 0
        : numer / 2;
}
#endif
CONVPARAM_INLINE int
compute_out(int i, int k, int s, int p, int d, int dil_start) {
    if (s<=0) return 0;
    // OneDNN formula with d replaced by d-dil_start
    int numer = (i-1) + 2*p - (k-1)*(d+1-dil_start);
    // deconv:  return (i-1)*s + (k-1)*(d+1-dil_start) -2*p +1
    return numer < 0? 0: numer / s + 1;
    // badly formed --> return output dim 0
}

CONVPARAM_INLINE int
compute_pad(int o, int i, int k, int s, int d, int dil_start) {
    // deconv: swap i and o
    int numer = (o-1)*s - i + (k-1)*(d+1-dil_start) + 1;
    //      0 for OneDnn; 1 for vednn    ---------
    return numer < 0? -1: numer / 2;
    // badly formed --> return padding -1
}

CONVPARAM_INLINE int
compute_oh( ConvParamRaw const* const r ){
    return compute_out( r->inHeight, r->kernHeight, r->strideHeight,
            r->padHeight, r->dilationHeight, r->dil_start );
}

CONVPARAM_INLINE int compute_ow( ConvParamRaw const* const r ){
    return compute_out( r->inWidth, r->kernHeight, r->strideHeight,
            r->padWidth, r->dilationHeight, r->dil_start );
}

CONVPARAM_INLINE int compute_ph( ConvParamRaw const* const r ){
    return compute_pad( r->outHeight, r->inHeight, r->kernHeight,
            r->strideHeight, r->dilationHeight, r->dil_start );
}

CONVPARAM_INLINE int compute_pw( ConvParamRaw const* const r ){
    return compute_pad( r->inWidth, r->outWidth, r->kernWidth,
            r->strideWidth, r->dilationWidth, r->dil_start );
}

#ifdef __cplusplus
}//extern "C"
#endif
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#endif // CONVPARAM_H
