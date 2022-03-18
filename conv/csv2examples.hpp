#ifndef CSV2EXAMPLES_HPP
#define CSV2EXAMPLES_HPP
#include "csv2examples.h"
#include <vector>
#include <string>
#include <set>
#include <map>
//#include <iosfwd>

/** \file .csv file read C++ API.
 * 1. read and do various preprocessing steps.
 * 2. provide way to generate training examples.
 *    - training inputs are defined by convolution conditions,
 *      and a set of implementation descriptions.
 *    - training targets are timing measurements for each impl.
 * 3. translation of training examples into float matrix/vector form.
 *    - inputs are chans * dim, where chans correspond to all
 *      possible impls, and dim is some fixed feature set describing
 *      the convolution, threads, ..., as well as some features
 *      borrowed from the decision tree analyses.
 *    - training targets can be summarized as a vector of float.
 *      (one way is provided, but need to play with other versions).
 *      This vector is used to implement some sort of loss function.
 */

#ifndef __cplusplus
#error "This must be compiled with C++"
#endif

extern void test(); // declare a function that can access object internals

namespace expt
{//}

// AllRuns may be cast to:
class AllRunsxx {
    friend void ::test(); // for test programs
public:
    /** delegating constructor from csv-filename */
    explicit AllRunsxx(char const *fname);
    /** delegating constructor from string data (for testing) */
    explicit AllRunsxx(std::string s);

    /** Basic constructor.
     */
    AllRunsxx(std::istream&& in);

    ~AllRunsxx();

private:
    // pre-processing hierarchy
    /** remove std and 'R' ref measurements and (eventually) keep only
     * a given dirn and layout.
     * Set factorParam, factorLayertype, factorLayer.
     */
    void filter(bool const verbose=false);

    /** tricky sanity & precision check of ms and gflops fields. */
    void massage_ms_gflops();
    void massage_merge_dups(bool const verbose=false);

    /** Run filter, and a number of other data massaging steps.
     * \sa allruns_massage().
     * Runs `filter` if nec.
     */
    void massage();

public:
    /** Run `massage`,`filter` and do groupwise experiment conversions.
     * Sets up expts[] numeric data, one per group of measurements.
     */
    void group(bool const verbose=false);

    // low-level accessors
    // maybe easier: after group, simply provide getters
    /** After `group()` we can get individual Expt (or throw). */
    Expt const* expt(size_t const i) const { return &expts.at(i); }
    /** After `group()` we can get number of expts. */
    int expt_sz() const { return (int)expts.size(); }

    char const* layer(size_t const i) const { return factorLayer.at(i); }
    size_t layer_n() const { return (int)factorLayer.size(); }

    char layertype(size_t const i) const { return factorLayertype.at(i); }
    size_t layertype_n() const { return factorLayertype.size(); }

    static void dump(OneRunRaw const& r);
    void dump(Expt const& e) const;

    /** Print expts[i] as table.  If expts empty or 'raw',
     * dump vrr[i] instead. */
    void dump(size_t i, bool const raw=false) const;
    /** Print expts[beg..end-1] as table.
     * (or vrr[beg..end-1] if expts-empty or 'raw') */
    void dump(size_t beg, size_t end, bool raw=false) const;
    /** dump "everything".
     * expts, if available and not 'raw', otherwise vrr raw data. */
    void dump(bool const raw=false) const;

    int getStatus() const { return status; }
    std::vector<OneRunRaw> const& getVrr() const { return vrr; }

    /** return expt[i] as C array data.
     * 'ef' is from exptFloat_alloc(), and can be re-used before exptFloat_free().
     *
     * Note that layer factors (and thus ExptFloat "channels" size) are determined
     * by "whatever layer impl names are found" in the constructor input stream.
     *
     * \c ywhat must be supplied 1st time, but \c ef's internal type will be re-used
     * if subsequent calls have empty \c ywhat.
     */
    void exptFloat(size_t const i, ExptFloat *ef,
            //std::string xwhat="default", // NO: xwhat is from ef->xtype, set during alloc
            std::string ywhat="norm_gf") const;
    /** low-level alloc,free helper */
    ExptFloat* exptFloat_alloc(std::string xwhat="default") const;
    /** low-level alloc,free helper */
    static void exptFloat_free(ExptFloat *ef);

    /** Alloc and fill data fields. Client should \c exptFloat_free it.*/
    //ExptFloat* exptFloat(size_t const i, std::string ywhat="norm_gf") const;
    // or maybe return ref to temporary
    ExptFloat* exptFloat(size_t const i,
            std::string xwhat="default",
            std::string ywhat="norm_gf") const;

    /** Channel names ~ width \e w features in ExptFloat::expt[w].
     * @return a C-array of ExptFloat::w C-strings.
     */
    static char const* const* featureNames( std::string xwhat );
    static size_t feature_n( std::string xwhat );

    /** string/char --> factor integer conversions. throw if unfound. */
    int intParam(char const* s) const;
    int intLayertype(char s) const;
    int intLayer(char const* s) const;
    std::vector<char const*> const& getFactorParam() const {
        return factorParam;}
    std::vector<char       > const& getFactorLayertype() const {
        return factorLayertype;}
    std::vector<char const*> const& getFactorLayer() const {
        return factorLayer;}
    
private:
    /** Populate a labels vector 'y' for expt[i].
     * y size is factorLayer.size(),
     * with absent layers targetting y[absent]=0.0
     */
    void get_y(int const i, float *max_gf, float *dest, std::string what="norm_gf") const;

    /** Fill a float vector of convolution features.
     * dim is from `floats(ConvParamRaw*,float*)`
     *
     * Supplying too many features can make training different predictions
     * more fragile.
     *
     * @param xtype an EXPT_X_foo constant as per csv2examples.h
     *
     * - \em default \b EXPT_X_DEFAULT: for training stage 0 boolean filter mask
     *   - I still should reduce the number of features to those still allowing
     *     a "full" (no false negative predictions) masking filter.
     * - \em bool \b EXPT_X_BOOL: same as default(?)
     * - \em values \b EXPT_X_VALUES: features with int/float range of values
     *
     * throw/assert if xtype out of range?
     */
    //void get_x(int const i, float *x, std::string what="default") const;
    void get_x(int const i, float *x, int xtype) const;

public:
    // data
    int verbose;
private:
    int status; ///< nz upon constructor error
    bool filtered; ///< set to 1 after filter call
    bool massaged; ///< set to 1 after massage call
    bool grouped; ///< set to 1 after expts call
    std::vector<OneRunRaw> vrr; ///< results vector (ok after constructor)
    /// After `massage`, have expt group start indices {0,...,vrr.size()}.
    /// size is n_groups+1 (final element is vrr.size()
    std::vector<int> group_lo; // ok after group()
    // grow-only string storage (itrs,ptrs valid thru AllRunsxx lifetime)
    //   valid after constructor
    std::set<std::string> setParams;
    std::set<std::string> setNames;           // ignored, often set at "wip"
    std::set<char>        setLayertypes;
    std::set<std::string> setLayers;
    // valid after filter() : pruned, ordered factor levels.
    std::vector<char const*> factorParam;     // from the set of Params
    std::vector<char       > factorLayertype; // I|J|R  |S for std?
    std::vector<char const*> factorLayer;
    // valid after massage_ms_gflops
    float fprec_ms; // fixed point precision of ms (text) outputs (estimate)
    float fprec_gf; // -- " -- for gflops
    bool ms_gf_nonzero; // bool, do this step just once.

    // valid after group()
    std::vector<Expt> expts;
};

}//expt::
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#endif // CSV2EXAMPLES_HPP
