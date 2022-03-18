#ifndef CONVPARAMXX_H
#define CONVPARAMXX_H
#ifndef __cplusplus
#error "This is really a C++ header (rename for Rcpp)"
#endif

#include "convParam.h" // lower-level C interface
#include <string>

//#define PARAM_BASE 0
#define PARAM_BASE 1

namespace conv { //}

class ConvParam;

}//conv::

typedef conv::ConvParam ConvParam_t;

namespace conv { //}

class ConvParam 
#if PARAM_BASE
: public ConvParamRaw_s
#endif
{
public:
#if PARAM_BASE
    typedef struct ConvParamRaw_s Base;
#endif
    /// default construct to 'all unset'.
    ConvParam() :
#if PARAM_BASE
        Base{ CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET,
            CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET,
            CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET,
            CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET,
            CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET, CONVPARAM_UNSET },
#else
        dil_start( CONVPARAM_UNSET ),
        dirn( CONVPARAM_UNSET ),
        layout( CONVPARAM_UNSET ),

        batchNum( CONVPARAM_UNSET ),
        group( CONVPARAM_UNSET ),
        inChannel( CONVPARAM_UNSET ),
        inHeight( CONVPARAM_UNSET ),
        inWidth( CONVPARAM_UNSET ),

        outChannel( CONVPARAM_UNSET ),
        outHeight( CONVPARAM_UNSET ),
        outWidth( CONVPARAM_UNSET ),

        kernHeight( CONVPARAM_UNSET ),
        kernWidth( CONVPARAM_UNSET ),
        strideHeight( CONVPARAM_UNSET ),
        strideWidth( CONVPARAM_UNSET ),
        padHeight( CONVPARAM_UNSET ),
        padWidth( CONVPARAM_UNSET ),
        dilationHeight( CONVPARAM_UNSET ),
        dilationWidth( CONVPARAM_UNSET ),
#endif
        name(),
        _ops(~0ULL)
    {}

    /// construct from OneDNN string (or vednn-convention if `is_vednn`)
    /// I'm not sure how to add optional constructor args to Rcpp,
    /// so there are two variants...
    ConvParam(std::string paramString);
    /// apply overrides (dumbly), after read is done. Set the overrides
    /// you want into a default-constructed \c ovr.
    ConvParam(std::string paramString, ConvParam const& ovr);

    /// dumb "gather" constructor (used internally)
    ConvParam( int is_vednn, int dir, int ly,
            int mb, int g,
            int ic, int ih, int iw,
            int oc, int oh, int ow,
            int kh, int kw,
            int sh, int sw,
            int ph, int pw,
            int dh, int dw )
            :
#if PARAM_BASE
            Base{/*dil_start*/(is_vednn==1? 1: 0),
            /*dirn*/(dir), /*layout*/(ly), /*batchNum*/(mb), /*group*/(g),
            /*inChannel*/(ic), /*inHeight*/(ih), /*inWidth*/(iw),
            /*outChannel*/(oc), /*outHeight*/(oh), /*outWidth*/(ow),
            /*kernHeight*/(kh), /*kernWidth*/(kw),
            /*strideHeight*/(sh), /*strideWidth*/(sw),
            /*padHeight*/(ph), /*padWidth*/(pw),
            /*dilationHeight*/(dh), /*dilationWidth*/(dw) },
#else
            dil_start(is_vednn==1? 1: 0),
            dirn(dir), layout(ly), batchNum(mb), group(g),
            inChannel(ic), inHeight(ih), inWidth(iw),
            outChannel(oc), outHeight(oh), outWidth(ow),
            kernHeight(kh), kernWidth(kw),
            strideHeight(sh), strideWidth(sw),
            padHeight(ph), padWidth(pw),
            dilationHeight(dh), dilationWidth(dw),
#endif
            _ops(~0ULL)
    {
        // check dirn and layout enums ?
    }

#if PARAM_BASE==0
    // Removed: "ve" at beginning of paramString is the way to
    //          specify that dilations start at `dil_start==1`.
    //ConvParam(std::string paramString, bool const is_vednn);
    // fb, fd, bd, bw, bb : fwd/bk both/data/weights
    //
    // The following happen to exactly match convParam.h ConvParamRaw
    //
    // NOTE: while R is OK with 'int', python/pytorch would rather use int64_t
    int        dil_start; // 0 for OneDNN, 1 for vednn
    int        dirn; // enum PropKind
    int        layout; // enum Layout

    int        batchNum;
    int        group;
    int        inChannel;
    int        inHeight;
    int        inWidth;
    int        outChannel;
    int        outHeight;
    int        outWidth;
    int        kernHeight;
    int        kernWidth;
    int        strideHeight;
    int        strideWidth;
    int        padHeight;
    int        padWidth;
    int        dilationHeight; // "undilated" begins at this.dil_start
    int        dilationWidth;
#else
    // ConvParamRaw_s is now a base class
#endif
    // todo: 3D {in|out|kern|stride|pad|dilation}Depth fields
    std::string name;  ///< std::string is nicer for Rcpp than char[250] or such
    enum ConvParamCheck check() const; ///< return 0 for err; 1 for ok-sparse; 2 for ok-dense (libvednn)

    /// @group string conversions
    /// string conversions respect dil_start; NO validity checks
    //@{
    std::string str() const;            ///< full string
    std::string str_short() const;      ///< short string
    // read(std::string); // you can use constructor instead
    //@}

    /// @group utilities
    //@{
    /// ops count (for fwd) is slow, so calc is on-demand and cached.
    unsigned long long ops(){
        if (this->_ops == ~0ULL)
            this->_ops = this->_count_ops();
        return this->_ops;
    }

    //@}

    ///@group Coercions
    //@{
    /// mkDense coerces settings to ensure dense output layout.
    /// @param ovr will slap on overrides and try to obey them while
    /// producing a consistent (dense) convolution. Versions only
    /// to help Rcpp.
    void mkDense();
    /// ... with overrides parameter.  Supplying \c ovr in the constructor
    /// instead has slightly different behavior wrt missing values.
    void mkDenseOvr( ConvParam const& ovr );
    void toOnednn(); ///< adjust dil_start to 0
    void toVednn(); ///< adjust dil_start to 1
    //@}

    /// @group "C" struct conversions
    /// dumb copy of all 'int` fields to/from "C" struct ConvParamRaw
    //@{
    void copy_to( ConvParamRaw * const r ) const;
    void copy_from( ConvParamRaw const * const r );
    ConvParamRaw raw() const; ///< copy_to local variable and return it

    //@}
private:
    void mk_dense( ConvParam const *ovr = nullptr );
    /** fwd ops brute-force count, as in bench-dnn */
    unsigned long long _count_ops() {
        ConvParamRaw const r = this->raw();
        return count_ops(&r);
    }
    unsigned long long _ops;
}; // ConvParam

inline void ConvParam::mkDense() { mk_dense(nullptr); }
inline void ConvParam::mkDenseOvr( ConvParam const& ovr) { mk_dense(&ovr); }

/// Makes sense if initially dil_start==0 or 1
inline void ConvParam::toOnednn(){
    int d = 0 - dil_start;
    dilationHeight += d;
    dilationWidth += d;
    dil_start += d; // this is now 0, the OneDNN convention
}
/// Makes sense if initially dil_start==0 or 1
inline void ConvParam::toVednn(){
    int d = 1 - dil_start;
    dilationHeight += d;
    dilationWidth += d;
    dil_start += d; // this is now 1, the vednn convention
}


} // convparam::
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#endif // CONVPARAMXX_H
