#ifndef CONV_FEATURES_H
#define CONV_FEATURES_H
/** \file
 * This defines a bunch of features, useful in creating a good guess
 * of which convolution implementations might be really fast.
 */
#include "convParam.h"
//#include <stdlib.h>
//#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @group ConvFeatureNames
/// Convolution Feature Names.
/** This is the full set of built-in features.
 * For timing datasets, sometimes we only use a subset of the full set.
 *
 * There are ExptFloat::w of these, hard-wired (ind't of `AllRuns`).
 * The ExptFloat::c convolution channel (layer) names can be gotten
 * from `strLayer(ar,i)`.
 */
//@{
extern int64_t const allFeature_sz;
extern char const* const allFeature_names[];
//char const* allFeature_name(int64_t const i);
CONVPARAM_INLINE char const* allFeature_name(int64_t const i) {
    return ((uint64_t)i < (uint64_t)allFeature_sz
            ? allFeature_names[i]
            : "");
}

/** dump allFeature_names to stdout */
void allFeature_dump();

// WIP: support "subset of features", using C arrays (C++ vectors) of feature names
extern int64_t const subFeature_values_sz;
extern char const* const subFeature_values_names[]; // 16 basic params and their inverses

/** convert user array of feature names into feature indices within allFeature_names[].
 * @pre feat[nfeat] inputs are valid C-strings
 * @pre idx[] outpus hold at least nfeat items.
 *
 * @param feat input C-string array
 * @param nfeat input C-string size
 * @param idx output index array, size at least nfeat
 * @param nidx output index size (illegal features ignored and return nonzero)
 * @return nonzero on error (*nidx != nfeat)
 */
int allFeature_indices(
        /* inputs*/ char const* *feat, int const nfeat,
        /*outputs*/ int *idx, int* nidx );


/** A work-per-thread estimate.
 * doItr "foo_X" and doJit "foo_X" decisions are based on
 * this estimate of "work per thread".
 */
float ops_factor(int const mb, int const threads);

/** calculate float vector corresponding to allFeature_names[idx[]].
 *
 * @pre idx[nidx] has values in range 0..allFeature_sz
 * @pre vals[nidx] float array can be written
 * @pre r corresponds to a valid convolution
 *
 * @param r convolution basic features
 * @param idx array of allFeature_names[] indices
 * @param nidx size idx[nidx]
 * @param threads auxiliary input needed for \b some features. Auxiliary inputs
 *                are \em outside ConvParamRaw (ex. threads, dirn, layout)
 * @param ops aux in: if zero (we will redo a slow ops count whenever needed)
 * @param vals vals[nidx] output vector
 * @return nonzero if vals[nidx] could not be fully populated for any reason
 *
 * Example:
 * ```
 * // names matching allFeature_names[]
 * char const * feat[3] = {"mb","ic","oc"};
 * int nidx = 0;
 * int idx[3];
 * int err = allFeature_indices(feat, 3, idx, &nidx));
 * if (!err)
 *     float vec[3];
 *     err = allFeature_extract( r, idx, nidx, 8, 0, vec );
 *     if (!err) {
 *         ; // do something with vec[]
 *     }
 * }
 * ```
 *
 * Most flexible, but might be slow.
 */
int allFeature_extract(
        /* inputs*/ ConvParamRaw const *r, int const *idx, int const nidx,
        /* aux in*/ int const threads, unsigned long long ops,
        /*outputs*/ float *vals );

/** Extract float feature vector into `vals[allFeatures_sz]`.
 *
 * @param r convolution basic features
 * @param threads auxiliary input needed for \b some features. Auxiliary inputs
 *                are \em outside ConvParamRaw (ex. threads, dirn, layout)
 * @param ops aux in: if zero (we will redo a slow ops count whenever needed)
 * @param vals vals[allFeature_sz] output vector
 * @return nonzero iff err
 *
 * Equivalent to allFeature_extract with `idx[]={0,1,2,...allFeatures_sz-1]}`.
 *
 * On VE, might be preferable to use this instead of `allFeatures_extract`
 * and generate subset of `vals` in client code.
 * Faster still if client can cache or supply `ops=count_ops(r)`.
 *
 * TODO: make a fast version of count_ops -- it is a significant slowdown!
 *       (it goes through all convolution loops in a braindead fashion)
 */
int allFeatures(
        /* inputs*/ ConvParamRaw const *r,
        /* aux in*/ int const threads, unsigned long long ops,
        /*outputs*/ float *vals );

/** prefilt0.py trains a prefilter network based on boolean features.
 * Convert an \c allFeatures float vector to a standard set of 32 bools.
 */
void allFeatures_extract_32bools(
        float const* const vals, // vals[allFeature_sz]
        float * const bools      // --> bools[32] (0/1 values, as it happens)
        );

//@}

#ifdef __cplusplus
}//extern "C"
#endif
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#endif //CONV_FEATURES_H
