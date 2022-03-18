#ifndef CSV2EXAMPLES_H
#define CSV2EXAMPLES_H

#include "convParam.h"
//#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** What do we parse from each line of the .csv file? */
struct OneRunRaw_s {
    char const* params;
    char const* name; // irrelevant
    char layertype;
    char const* layer;
    int best; // 0/1
    int reps;
    int threads;
    float ms;
    float gflops;
    float err;
    float ops;
};
typedef struct OneRunRaw_s OneRunRaw;

/** wrapper around opaque C++ object */
typedef struct {void* opaque;} AllRuns;

/** Set `ar` to point to a newly allocated AllRuns
 * @return error status, and set ar->opaque non-NULL if status==0.
 */
int readConvCsv(char const* fname, AllRuns *ar);

/** Free *ar and set it to NULL. */
void allruns_free(AllRuns **ar);

/** How many items (originally, lines) are in `ar`? */
size_t allruns_raw_sz(AllRuns const* ar);
/** Access `i`th raw data item of `ar` (or throw) */
OneRunRaw const* allruns_raw(AllRuns const* ar, int const ith);

#if 0
/** Std filters.  Keep only dirn and layout (for now nchw and fwd_bias
 * are assumed, will change).  Remove 'R' ref measurements and any
 * "std" layers.   (Possibly remove and roll into massage?) */
void allruns_filter(AllRuns *ar
        //, enum DilationStart dirn, enum Layout layout
        );
#endif

#if 0 // maybe no need to expose
/** Massage raw data.
 * i) Run allruns_filter.
 * i) adjust ms, Gflops and ops to lose least quantization info
 * ii) sort string param (assume canonical fmt, 1--1 with convolutions),
 *     threads ascending,
 *     ms ascending (gflops descending)
 * iii) merge 'reps' of duplicate lines in above.
 * iv) record (internal) indices of param+threads groups, and number of groups
 *     (this becomes the number of 'Expts').
 */
void allruns_massage(AllRuns *ar);
#endif


#if 0 // not exposed
/** Given an array of OneRunRaw_s, there are various
 * numerical conversions to perform.  Several of these
 * convert various string fields to/from integers, as *factors*.
 *
 * `params` can be treated both as factors and as the main component
 * of 'readConvParamRaw' conversion into numeric convolution parameters,
 * 'ConvParamRaw_s'.  Strictly speaking params strings aren't one-to-one,
 * but we know they are output in some "canonical" format in the .csv files.
 * So no two different param strings map to the same set of convolution
 * parameters.
 *
 * These are void so it can be done in C++
 */
typedef void const* FactorParam;
typedef void const* FactorLayertype;
typedef void const* FactorLayer;
#endif

/** Single Experiments are a set of runs, each with identical
 * param (/ConvParamRaw_s),  data layout, threads.  This set of
 * runs is a single input.  We will want a loss function that
 * records (ex.) top-1 speed loss, or some more general ranking
 * loss of predicted some set of top-N layers. */
struct Expt_s {
    // internal numbering for ConvParamRaw (back to original string)
    // int params_factor;
    // experimental conditions
    ConvParamRaw c;
    int threads;
    //enum Layout layout;       // constant, for now
    //enum PropKind propkind;   // constant, for now
    // common side info
    double ops;
    // set of measurements, a vector
    size_t msz;
    // generally, an absent layer means it can't (or shouldn't)
    // run the convolution c
    struct Measurement {
        int layertype; // ex I or J or R (probably filter R and possibly J?)
        int layer;     // ex unroll_cjitConvFwd6 or cnvFwd-owU128_T
        int reps;
        float ms;
        float gflops;
        float err;     // if err, is the timing still meaningful? (Yes, for now)
    } *m;
};
typedef struct Expt_s Expt;

/** Create experiment data.  Runs a bunch of preprocessing steps on `ar`.
 * After this, you may also access via `allruns_expt` and `allruns_expt_sz`.
 */
void mkExpts( AllRuns *ar
        //, TBD filters: enum PropKind dirn, enum Layout layout
        );

/** How many expts (after mkExpts). */
size_t allruns_expt_sz(AllRuns const* ar);
/** Access `i`th Expt item of `ar` (or throw) */
Expt const* allruns_expt(AllRuns const* ar, int const ith);
/** print an Expt*. Expt is numeric, so some string conversion need `ar`. */
void allruns_print(AllRuns const* ar, Expt const*);
/** print a series of expts */
void printExpts(AllRuns const* ar, int beg, int end);



#if 0 // remove ?
struct Expts_s {
    /** common factor conversions (opaque void*). */
    FactorParam fParam;
    FactorLayertype fLayertype;
    FactorLayer fLayer;
    /** vector of test data + observations */
    Expt const* expts;
    int n_expts;
};
typedef struct Expts_s Expts;

/** Convert vector of csv data into a numerical version of the experiments.
 * DilationStart is assumed to be 0 (OneDNN convention).
 *
 * `dirn` and `layout` are used to filter the data set.
 * For now, layout==LAYOUT_NCHW always and dirn==PROPKIND_FORWARD_BIAS
 * are hard-wired into these expts, but as usage of `jitconv` evolves
 * this might change.
 *
 * OneRunRaw data are passed through some standard filters
 *   (i.e. dirn, layout, remove 'R' reference measurements, remove
 *    libvednn "std" measurements)
 *
 * OneRunRaw ms, Gflops, ops are rationalized in case one is presented in
 * "too-quantized" a form.
 *
 * OneRunRaw are ordered by string param, ascending threads, descending Gflops.
 * Then reps of duplicates are merged.  (merge_dups)
 *
 * Grouping data by param, threads (dirn, layout), we now reorganize the
 * dataset as Expts.
 *
 * @return Expts* which must be freed by user.  Valid until
 * allruns_free(&ar);
 */
Expts *toExpts( AllRuns *ar
        //, TBD filters: enum PropKind dirn, enum Layout layout
        );

/** print expts[beg..end-1]. beg and end are capped to lie within 0..n_expts. */
void printExpts(Expts *expts, int beg, int end);
#endif

#if 0
typedef enum ExptFloatX_e {
    EXPT_X_DEFAULT = 0,
    EXPT_X_BOOL    = 0,
    EXPT_X_VALUES  = 1,
} ExptFloatX;
typdef enum ExptFloatY_e
    EXPT_Y_UNSET   = 100,
    EXPT_Y_NORM_GF = 100,
    EXPT_Y_GF      = 101,
} ExptFloatY;
#else
// add EXPT_SET to EXPT_X_foo or EXPT_Y_foo when valid values are set up
#define EXPT_SET 1000
// X type must be known at 'alloc' stage
#define EXPT_X_UNKNOWN 0
#define EXPT_X_DEFAULT 1
#define EXPT_X_BOOL 1
#define EXPT_X_VALUES 2
// Y type is set when y values are set
#define EXPT_Y_UNKNOWN 0
#define EXPT_Y_NORM_GF 101
#define EXPT_Y_GF      102
#endif

/** For ML, we reorganize each Expt as an input vector and an output vector.
 * expt[w] are conv features, y[c] are speed info.  Absent/impossible impls
 * are given y[c]=0.0f target speed. */
struct ExptFloat_s {
    int xtype; // EXPT_X_foo set during alloc, add EXPT_SET after 'get_x'
    int const c;  // channels: how many layer impls?
    int const w;  // width:    what float dimensionality per convolution?
    float max_gf; // potly scale factor (ex. Gflops of the fastest impl)
    int ytype; // (EXPT_Y_foo + EXPT_SET) determined during 'get_y'
    // input data vector expt[w]
    float *expt; ///< expt[w] input features
    // y[c] output gFlops (or gFlops normalized by fastest)
    float *y; ///< y[c] output targets
};
typedef struct ExptFloat_s ExptFloat;

/** convert a single Expt into data[channel,dim] and gflops[channel] output.
 * Here each channel corresponds to a layer (implementation).
 * If layer info is absent, then its y is (ex.) -2.0 Gflops.
 * The 'dim' data are various ConvParamRaw values, -1/ConvParamRaw values,
 * and various common boolean conditions, all expressed as *float*.
 * There should be few (say 30 of these),
 * filtering out any "constant columns" (like dil_start, dirn, layout).
 *
 * 'dim' features are internal details, and have names given by
 * `char const* featureName(int i)`.  Ex. ksq, p0, pS, ksq3, icOg, ...
 * (try to reuse R package usage).
 *
 * There are many ways to formulate the problem.
 * A simple one is as multiple independent regressions:
 *   Input: data[channel]          Output: Gflops
 *
 * As MLP with L layers, each channel uses a common MLP:
 *     data[-1,dim] * M_1[dim,n] * ... * M_L[x,1] --> scalar y.
 * yields ch sets of regression matrices {M_-1..L}.
 * Hidden layer sizes are {n,...x} terminate with hidden layer size -1.
 *
 * Total weights are then a 1D tensor, produces ch scalars y.
 * (Can this be done succinctly in pyTorch?)
 *
 * Loss is calculated on the regression outputs (and probably doesn't even
 * care about ch that are not the fastest "top-N" layers).
 *
 * Because Expt layer info is sparse (only a few impls can run under all
 * conditions), the "test data" expands in size!  Absent impls have target
 * 'y' of -2.0.
 *
 * If *dest is non-NULL and its 'c,w' agree (etc.), we'll reuse that memory
 * allocation.  Otherwise we'll allocate and reallocate memory as needed.
 *
 * @param dest Begin with *dest==nullptr, it will be allocated.
 * @return with *dest pointing to an ExptFloat
 */
void toExptFloat(AllRuns *ar, size_t const i, ExptFloat **dest, char const* xwhat);
/** Numeric data from `toExptFloat` can re-use `dest`, but eventually
 * you should free the memory. */
void exptFloat_free(ExptFloat **dest);


// helper functions

// Convert factor values 'i' back to strings (or throw)
char const* strParam    (AllRuns const* ar, int i);
char        strLayertype(AllRuns const* ar, int i);
char const* strLayer    (AllRuns const* ar, int i);
// Convert strings to factor values (or throw)
int intParam    (AllRuns const* ar, char const* param);
int intLayertype(AllRuns const* ar, char const c);
int intLayer    (AllRuns const* ar, char const* layer);
// get factor sizes
size_t intParam_sz    (AllRuns const* ar);
size_t intLayertype_sz(AllRuns const* ar);
size_t intLayer_sz    (AllRuns const* ar);

#ifdef __cplusplus
}//extern "C"
#endif
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#endif // CSV2EXAMPLES_H
