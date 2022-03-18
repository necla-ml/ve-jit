// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
#define CCONVPARAM_C /*make some thing NOT extern or inline (force code gen)*/
#include "convParam.h"

#include "conv_features.c"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdio.h> // stdout, fflush

#define VE_PREFIX 1 // allow 've' prefix to signal is_vednn

#ifdef __cplusplus
#define TRUE true
#define FALSE false
#else
#define TRUE 1
#define FALSE 0
#endif

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

#ifdef __cplusplus
extern "C" {
#endif

// designated initializers are pure-C (or C++ with -std=c++2a or such)
struct ConvParamRaw_s const convParamRaw_unset = {
    .dil_start = CONVPARAM_UNSET,
    .dirn = CONVPARAM_UNSET, /*fb?*/
    .layout = CONVPARAM_UNSET,
    .batchNum = CONVPARAM_UNSET,
    .group = CONVPARAM_UNSET,
    .inChannel = CONVPARAM_UNSET,
    .inHeight = CONVPARAM_UNSET,
    .inWidth = CONVPARAM_UNSET,
    .outChannel = CONVPARAM_UNSET,
    .outHeight = CONVPARAM_UNSET,
    .outWidth = CONVPARAM_UNSET,
    .kernHeight = CONVPARAM_UNSET,
    .kernWidth = CONVPARAM_UNSET,
    .strideHeight = CONVPARAM_UNSET,
    .strideWidth = CONVPARAM_UNSET,
    .padHeight = CONVPARAM_UNSET,
    .padWidth = CONVPARAM_UNSET,
    .dilationHeight = CONVPARAM_UNSET,
    .dilationWidth = CONVPARAM_UNSET
};

static inline char const* ve_prefix( ConvParamRaw const* const p ){
#if VE_PREFIX
    return (p->dil_start==1
            && (p->dh!=p->dil_start || p->dw!=p->dil_start))
            ? "ve": "";
#else
    return "";
#endif
}

static void convparam_str_error_exit(){
    perror("ConvParam -->string error, exiting now");
    exit(1);
}

/** round i>0 upward to a multiple of a>0 */
static inline int
upMul( int const i, int const a ) {
    return ((i+a-1)/a)*a;
}

// XXX keep code in sync with convParmxx.cpp
enum ConvParamCheck
convParamCheck( ConvParamRaw const* const r)
{
    // default constructor fails first test, rest don't execute
    // Missing: check dirn, and (later) 3D fields
    int check_range = (r->dil_start==0 || r->dil_start==1)
            && r->dirn >= 0     // weak
            && r->layout >= 0   // weak
            && r->batchNum>0 && r->group>0
            && r->inChannel>0 && r->inHeight>0 && r->inWidth>0
            && r->outChannel>0 && r->outHeight>0 && r->outWidth>0
            && r->kernHeight>0 && r->kernWidth>0
            && r->strideHeight>0 && r->strideWidth>0
            && r->padHeight>=0 && r->padWidth>=0
            && r->dilationHeight>=r->dil_start
            && r->dilationWidth>=r->dil_start
            ;
    if (!check_range)
        return CONVCHECK_ERROR;

    // group must be in [1,min(ic,oc)] and divide ic and oc exactly.
    if (!(         r->group <= r->inChannel  && (r->inChannel % r->group) == 0
                && r->group <= r->outChannel && (r->outChannel % r->group) == 0
         ))
        return CONVCHECK_ERROR;

    // "consistent" implies a packed memory layout with no "holes", so
    // a.k.a. "dense" layout with strides ~ multiples of tensor dims.

    // calculate perfect, minimal outHeight, outWidth
    int poh = compute_out(r->ih,r->kh,r->sh,r->ph,r->dh,r->dil_start);
    int pow = compute_out(r->iw,r->kw,r->sw,r->pw,r->dw,r->dil_start);

    if (r->oh == poh && r->ow == pow)
        return CONVCHECK_DENSE; // libvednn requires dense memory layout

    if (r->oh < poh || r->ow < pow) // not enough space for output --> ERROR
        return CONVCHECK_ERROR;

    // at least one of oh, ow is sparse (still OK for libvednn)
    return CONVCHECK_SPARSE;
}

int apply_overrides( ConvParamRaw *r, ConvParamRaw const *ovr)
{
    int n=0U;
    printf(" applying overrides... ");
#define OVRCHK(VAR) do \
    { \
        if(ovr->VAR != CONVPARAM_UNSET){ \
            ++n; \
            printf(" " #VAR "-->%d",ovr->VAR); \
            r->VAR = ovr->VAR; \
        }}while(0)
    OVRCHK(dil_start      );
    OVRCHK(dirn           );
    OVRCHK(layout         );
    OVRCHK(batchNum       );
    OVRCHK(group          );
    OVRCHK(inChannel      );
    OVRCHK(inHeight       );
    OVRCHK(inWidth        );
    OVRCHK(outChannel     );
    OVRCHK(outHeight      );
    OVRCHK(outWidth       );
    OVRCHK(kernHeight     );
    OVRCHK(kernWidth      );
    OVRCHK(strideHeight   );
    OVRCHK(strideWidth    );
    OVRCHK(padHeight      );
    OVRCHK(padWidth       );
    OVRCHK(dilationHeight );
    OVRCHK(dilationWidth  );
#undef OVRCHK
    if(n) printf("\n");
    return n;
}

/** As-is conv param data --> long string. */
char const* to_cstr( ConvParamRaw const* const p,
        char * const buf, size_t const n) {
    // NEW: if dil_start is for vednn and non-standard dilation,
    //      then prefix the output string with "ve"
    int fields = snprintf(buf, n,
            "%smb%dg%d_ic%dih%diw%d_oc%doh%dow%d_kh%dph%dsh%ddh%d_kw%dpw%dsw%ddw%d",
            ve_prefix(p),
            p->mb, p->g,
            p->ic, p->ih, p->iw,
            p->oc, p->oh, p->ow,
            p->kh, p->ph, p->sh, p->dh,
            p->kw, p->pw, p->sw, p->dh
            );
    if (fields < 0) { /* error? */
        convparam_str_error_exit();
    }
    return buf;
}

/** NOTE: [ejk] Although OneDNN uses default mb==2, I find it nicer
 * for sorting experiments to **always** include the 'mb' at the beginning.
 */
char const* to_cstr_short( ConvParamRaw const* const p,
        char * const buf, size_t const n)
{
    char *buffer = buf;
    int rem_len = n;
#define ADD(...) do \
    { \
        int l = snprintf(buffer, rem_len, __VA_ARGS__); \
        if(l<0) convparam_str_error_exit(); \
        buffer += l; rem_len -= l; \
    }while(0)
    ADD("%s", ve_prefix(p)); // "ve" (if it makes a difference?)

    int const half_form = ( 1
            && p->ih == p->iw && p->kh == p->kw && p->oh == p->ow
            && p->sh == p->sw && p->ph == p->pw && p->dh == p->dw
            //&& p->inDepth==1
            );
    // TODO: allow half-form for some but not all?
    // ex. ihXiwY_kh3 with no kw assumes kw=3, etc.
    // (to match what readParamString can handle)
    if(half_form){ // as per mkl
        // *** NOTE *** mkl-dnn default mb is 2 !
        int f=0; // field count
        //if(p->mb!=2)                            // mb field ALWAYS present
        { ADD("mb%d",p->mb); ++f; }
        if(p->g !=1){ ADD("g%d",p->g);   ++f; }
        if(f) ADD("_");
        ADD( "ic%dih%doc%doh%d_kh%d",
                p->ic, p->ih, p->oc, p->oh, p->kh);
        if(p->sh!=1 || p->ph!=0 || p->dh!=1 ){
            if(p->sh!=1) ADD("sh%d",p->sh);
            if(p->ph!=0) ADD("ph%d",p->ph);
            if(p->dh!=p->dil_start) ADD("dh%d",p->dh);
        }
    }else if(1){ // more aggressive default-or-equal eliminations
        // --> shorter jit symbol names, nicer screen output
        // *** NOTE *** mkl-dnn default mb is 2 !
        int f=0; // field count
        //if(p->mb!=2)                            // mb field ALWAYS present
        { ADD("mb%d",p->mb); ++f; }
        if(p->g!=1){ ADD("g%d",p->g);    ++f; }
        if(f) ADD("_");
        ADD("ic%d",p->ic);
        ADD("ih%d",p->ih);
        if(p->iw!=p->ih) ADD("iw%d",p->iw);

        ADD("_oc%d",p->oc);
        ADD("oh%d",p->oh);
        if(p->ow!=p->oh) ADD("ow%d",p->ow);

        ADD("_kh%d",p->kh );
        if(p->kw!=p->kh) ADD("kw%d",p->kw);

        // elide default stride pad and dil
        if(p->sh !=1 || p->sw != 1){
            if(p->sw == p->sh) ADD("sh%d",p->sh);
            else ADD("sh%dsw%d",p->sh,p->sw);
        }
        if(p->ph !=0 || p->pw != 0){
            if(p->pw == p->ph) ADD("ph%d",p->ph);
            else ADD("ph%dpw%d",p->ph,p->pw);
        }
        if(p->dh!=p->dil_start || p->dw!=p->dil_start){ // 1 is libvednn "no dilation"
            if(p->dw == p->dh) ADD("dh%d",p->dh);
            else ADD("dh%ddw%d",p->dh,p->dw);
        }
    }else{ // very long string, every field explicit
        to_cstr(p,buf,n);
    }
#undef ADD
    return buf; // note: return unparsed trailer
}

/** return error count. vals are parsed values, have are booleans
 * for parsed values.  Ex.
 * ```
 * ConvParamRaw vals, have;
 * memset(&vals, 0, sizeof(ConvParamRaw));
 * memset(&have, 0, sizeof(ConvParamRaw));
 * char const *name = NULL;
 * preparseConvParamRaw( str, &vals, &have, &name );
 * // ... perhaps construct a std::string from name
 * if (name) free(name)
 * ```
 * Note: raw parse does not care about 'is_vednn' string convention,
 *       EXCEPT if the string explicitly begins with "ve".
 */
static int preparseConvParamRaw( char const* str,
        ConvParamRaw *vals,
        ConvParamRaw *have,
        char ** const name)
{
    // since vals default is CONVPARAM_UNSET (-1),
    // 'have' can be gotten rid of, because valid tag vals >=0
    int const v=0; // verbose
    char * end = NULL; // where we last finished parsing
    if(v>0){printf("preparse string %s\n",str); fflush(stdout);}

    // instead of regex, just do a single fwd pass
    int err = 0;
    char const* s = (char*)str; // parse position

    // At WHERE check for tag STR of length LEN 
#define MATCH(PTR, STR, LEN) (strncmp(PTR,STR,LEN)==0)

#define VALS_HAVE( ITEM, value ) \
    do { \
        vals->ITEM = (value); \
        have->ITEM = TRUE; \
    }while(0)

    //
    // some instructions are only allowed in a prefix region
    //
    for( /*s*/; s!=NULL && *s!='\0'; )
    {
        // any prefix tag match executes 'continue'
#define MATCH_SET(STR, LEN, ITEM, VAL) \
        if(MATCH(s,STR,LEN)) { \
            VALS_HAVE(ITEM, VAL); \
            s+=LEN; \
            continue; \
        }
#if VE_PREFIX
        MATCH_SET("ve",2, dil_start,DIL_VEDNN/*1*/);
#endif
        // dirn:  offer possibilities covering
        //   vednn note: FB~fwd bias, BB~bkwd both
        //   - fwd has no inference optimizations
        //   - bkwd has only BD and BW ?
        //   OneDNN: TBD
        MATCH_SET("FD",2, dirn,PROPKIND_FORWARD_NOBIAS);
        MATCH_SET("FB",2, dirn,PROPKIND_FORWARD_BIAS);
        MATCH_SET("BD",2, dirn,PROPKIND_BACKWARD_DATA);
        MATCH_SET("BW",2, dirn,PROPKIND_BACKWARD_WEIGHTS);

        if(MATCH(s,"ly",2)){ // layout (vednn only has 2)
            if(MATCH(s+2,"abx",3)){
                VALS_HAVE(layout, LAYOUT_ABX);
                s+=5; continue;
            }
            if(MATCH(s+2,"axb",3)){
                VALS_HAVE(layout, LAYOUT_ABX);
                s+=5; continue;
            }
        }
        break; // matched NO tags
    }
    // Done prefix tag region!
    // ALWAYS set up a dil_start convention
    if (!have->dil_start) VALS_HAVE( dil_start, DIL_ONEDNN/*0*/ );
        
    for( /*s*/; s!=NULL && *s!='\0'; )
    {
        char const* s0 = s; // remember start of current tag parse
        if(v>1){printf(" enter: s@%p\n",(void*)s); fflush(stdout);}
        if(v>1){printf(" enter: s %s\n",s); fflush(stdout);}

        // Special: "#"  means ignore rest of line (comment)
        if(*s == '#'){
            break;
        }
        if(*s == '_'){
            ++s;
            continue;
        }

        // normal checks:  tag followed by integer
#define CONV_TEST_PARAM_CHK(STR,LEN,batchNum) \
        do { \
            if(strncmp(s,STR,LEN)==0){ \
                /*printf("  s @  %p\n",(void*)s);*/ \
                if(v>1){printf(" matched %s in %s",STR,s);} \
                int val = (int)strtol(&s[LEN],&end,10); \
                if(end==&s[LEN]){ /* ohoh. no value! */ \
                    ++err; break; \
                } \
                if(v>1){printf(" = %d\n",val); fflush(stdout);} \
                if(v==1){printf(" %s=%d",STR,val);} \
                VALS_HAVE(batchNum, val); \
                s=end; \
                while(*s=='_') ++s; \
                /*printf(" remain %s\n",s); fflush(stdout);*/ \
                continue; \
            }else{ \
                /*printf(" no %s in %s\n",STR,s); fflush(stdout);*/ \
            } \
        } while(0)

        // 1st successful match does "continue" to keep going
        CONV_TEST_PARAM_CHK("mb",2,batchNum);
        CONV_TEST_PARAM_CHK("g",1,group);
        CONV_TEST_PARAM_CHK("ic",2,inChannel);
        CONV_TEST_PARAM_CHK("ih",2,inHeight);
        CONV_TEST_PARAM_CHK("iw",2,inWidth);
        CONV_TEST_PARAM_CHK("oc",2,outChannel);
        CONV_TEST_PARAM_CHK("oh",2,outHeight);
        CONV_TEST_PARAM_CHK("ow",2,outWidth);
        CONV_TEST_PARAM_CHK("kh",2,kernHeight);
        CONV_TEST_PARAM_CHK("kw",2,kernWidth);
        CONV_TEST_PARAM_CHK("sh",2,strideHeight);
        CONV_TEST_PARAM_CHK("sw",2,strideWidth);
        CONV_TEST_PARAM_CHK("ph",2,padHeight);
        CONV_TEST_PARAM_CHK("pw",2,padWidth);
        CONV_TEST_PARAM_CHK("dh",2,dilationHeight);
        CONV_TEST_PARAM_CHK("dw",2,dilationWidth);
        // nothing matched!
#undef CONV_TEST_PARAM_CHK
#undef VALS_HAVE

        // Special tag: "n"  name is a terminal option,
        // default = empty string, # terminates
        if( s[0] == 'n' ){
            if(v>0){printf(" matched n in %s, name terminates options",s);}
            char const* sbeg = s+1;
            char const* send = sbeg;
            while(*send != '\0' && *send != '#')
                ++send; // find terminator
            int const slen = send - sbeg;
            // zero-length name is same as "no name"
            if (slen) {
                char* newname = (char*)malloc(slen+1);
                strncpy(newname,sbeg,slen);
                newname[slen] = '\0';
                if(v>0){printf(" name is <%s>\n",newname);}
                *name = newname;
            }
            break; // "n" is terminal
        }

        // if still where we started, we did not recognize any option (error)
        if(s==s0){
            printf(" unrecognized characters at: %s\n",s);
            ++err;
            break;
        }

    }
    // All tags were recognized.
    return err;
}

/** return error count.  vals and have are from preparseConvParamRaw.
 * vals is filled in as much as possible.
 * have is filled in to all-1 upon success (mostly).
 * @param is_vednn \deprecated is 0/1 and denotes the *string* interpretation.
 */
static int fillinConvParamRaw(
        ConvParamRaw* vals,
        ConvParamRaw* have)
{
    int const v=0; // verbose?
    if(v) {
        char b[80];
        printf(" fillin BEG vals=%s\n",to_cstr(vals,b,80));
        printf("            have=%s\n",to_cstr(have,b,80));
        printf("            dil_start=%d is_vednn=deprecated\n",
                vals->dil_start);
    }
    int err = 0; // return value

#define VALS_HAVE( ITEM, value ) \
    do { \
        vals->ITEM = (value); \
        have->ITEM = TRUE; \
    }while(0)

#define DEFAULT_VALUE(ITEM, value) \
    do { \
        if( !have->ITEM ) VALS_HAVE( ITEM, (value) ); \
    }while(0)

#define DEFAULT_VALUES(ITEM1, ITEM2, value) \
    do { \
        if( !have->ITEM1 && !have->ITEM2 ) { \
            VALS_HAVE( ITEM1, (value) ); \
            VALS_HAVE( ITEM2, (value) ); \
        }}while(0)

    DEFAULT_VALUE( dil_start, DIL_ONEDNN/*0*/ ); // just in case.

    DEFAULT_VALUE(dirn, PROPKIND_FORWARD_BIAS );
    DEFAULT_VALUE(layout, LAYOUT_ABX ); // nchw default
    DEFAULT_VALUE(batchNum, 2); // follow OneDNN for this default
    DEFAULT_VALUE(group, 1);
    DEFAULT_VALUE(inChannel, 1);
    DEFAULT_VALUE(outChannel, 1);

    // if both missing, both get default
    DEFAULT_VALUES(kh, kw, 3);
    DEFAULT_VALUES(sh, sw, 1);
    DEFAULT_VALUES(ph, pw, 0);
    DEFAULT_VALUES(dh, dw, vals->dil_start);
    if(v) printf("fillin dil_start %d : dh,dw-->%d,%d\n",vals->dil_start,vals->dh,vals->dw);

    // leave default name as empty
    //if( !have.name.size() ){
    //    vals.name = std::string();
    //    have.name = std::string("1");
    //}

    // many single missing fields default to "square" Height = Width
#define MISSING_SAME(FIELD1,FIELD2) \
    do { \
        if( have->FIELD1 && !have->FIELD2 ){ \
            VALS_HAVE(FIELD2, vals->FIELD1); \
            if(v) printf(#FIELD2 " <- " #FIELD1 " (%d)\n", vals->FIELD2); \
        }else if( have->FIELD2 && !have->FIELD1 ){ \
            VALS_HAVE(FIELD1, vals->FIELD2); \
            if(v) printf(#FIELD1 " <- " #FIELD2 " (%d)\n", vals->FIELD1); \
        } \
    }while(0)
    MISSING_SAME(inHeight,inWidth);
    MISSING_SAME(outHeight,outWidth);
    MISSING_SAME(kernHeight,kernWidth);
    MISSING_SAME(strideHeight,strideWidth);
    MISSING_SAME(padHeight,padWidth);
    MISSING_SAME(dilationHeight,dilationWidth);
    assert( have->kernHeight && have->kernWidth );
    assert( have->strideHeight && have->strideWidth );
    assert( have->dilationHeight && have->dilationWidth );
    if(v) printf("fillin2 dil_start %d : dh,dw-->%d,%d\n",vals->dil_start,vals->dh,vals->dw);
    // ih unknown?
    // NOTE: logical inconsistency:  Here we are doing a little more
    // than "filling in" preset defaults, but at least move toward
    // "something plausible".
    if( !have->ih ){
        if (!have->ph)
            VALS_HAVE(ph,0);
        //if (!have->oh) naah, just err if missing both ih & oh
        // don't have handy formula yet, so just search for approx OK
        int iok = 0;
        for(int i=1; i<1024; ++i) {
            int osz = compute_out(i, vals->kh, vals->sh, vals->ph,
                    vals->dh, vals->dil_start);
            if( osz >= vals->oh ){ iok=i; break; }
        }
        if(iok > 0) VALS_HAVE( inHeight, iok );
        else { // approximate
            VALS_HAVE(inHeight, vals->oh * vals->sh);
            have->oh = FALSE; // force oh re-adjust
        }
    }
    if( !have->iw ){
        if (!have->pw)
            VALS_HAVE(pw,0);
        int iok = 0;
        for(int i=1; i<8192; ++i) {
            int osz = compute_out(i, vals->kw, vals->sw, vals->pw,
                    vals->dw, vals->dil_start);
            if( osz >= vals->ow ){ iok=i; break; }
        }
        if(iok > 0) VALS_HAVE( inWidth, iok );
        else { // approximate
            VALS_HAVE(inWidth, vals->ow * vals->sw);
            have->ow = FALSE; // force ow re-adjust
        }
    }
    assert( have->ih && have->iw );

    // infer padding from in/out + kernel size, if possible, else pad=0
    // padHeight?  If pad < 0, assume outH/W was incorrect and zero the pad
    if( !have->padHeight ){
        if( have->ih && have->oh /*&& have->kh && have->sh && have->dh*/ ){
            vals->ph = compute_ph( vals );
            if(vals->ph < 0){
                vals->ph = 0;
                have->oh = FALSE; // assume outHeight mistake
            }
        }else{
            vals->ph = 0;
        }
        have->ph = TRUE;
    }
    // padWidth?
    if( !have->pw ){
        if( have->iw && have->ow /*&& have->kw && have->sw && have->dw*/ ){
            vals->pw = compute_pw( vals );
            if(vals->pw < 0){
                vals->pw = 0;
                have->ow = FALSE; // assume outWidth mistake
            }
        }else{
            vals->pw = 0;
        }
        have->pw = TRUE;
    }
    assert( have->padHeight && have->padWidth );
    if(v) printf("fillin3 dil_start %d : dh,dw-->%d,%d\n",vals->dil_start,vals->dh,vals->dw);

#if 0
    // Can we calculate a missing out size?
    // A too-big out size is OK; too-small outsize is error.
    // -----> leave such checks for mkConsistent <------
    if( have->ih && have->kh && have->sh && have->ph && have->dh ){
        exact_oh = compute_oh(vals);
        if( have->oh && vals->oh < exact_oh ){
            // specified output height too small
            have->ih = FALSE; // unrecoverable error?
        }
        vals->oh = exact_oh;
        have->oh = TRUE;
    }
    if( have->iw && have->kw && have->sw && have->pw && have->dw ){
        exact_ow = compute_ow(vals);
        if( have->ow && vals->ow < exact_ow ){
            // specified output height too small
            have->iw = FALSE; // unrecoverable error?
        }
        vals->ow = exact_ow;
        have->ow = TRUE;
    }
#else // dumber version
    if( !have->oh ){
        if (have->ih && have->kh && have->sh && have->ph && have->dh ){
            vals->oh = compute_oh( vals );
            if(v){printf("outHeight <- compute %d\n", vals->oh);}
        }
        if( vals->oh <= 0 ){ // probably something is really wrong
            vals->oh = 1;    // but we are't constrained to be perfect
            if(v){printf("outHeight <- 1\n");}
        }
        have->oh = TRUE;
    }
    if( !have->ow ){
        if (have->iw && have->kw && have->sw && have->pw && have->dw ){
            vals->ow = compute_ow( vals );
            if(v){printf("outWidth <- compute %d\n", vals->oh);}
        }
        if( vals->ow <= 1 ){
            if(v){printf("outWidth <- 1\n");}
            vals->ow = 1;
        }
        have->ow = TRUE;
    }
#endif
    // We do not try to infer input size.
    // Any undefined fields at this point mean there is some error.
    if( have->dil_start == FALSE
            || have->dirn == FALSE
            || have->layout == FALSE
            || have->batchNum == FALSE
            || have->group == FALSE
            || have->inChannel == FALSE
            || have->inHeight == FALSE
            || have->inWidth == FALSE
            || have->outChannel == FALSE
            || have->outHeight == FALSE
            || have->outWidth == FALSE
            || have->kernHeight == FALSE
            || have->kernWidth == FALSE
            || have->strideHeight == FALSE
            || have->strideWidth == FALSE
            || have->padHeight == FALSE
            || have->padWidth == FALSE
            || have->dilationHeight == FALSE
            || have->dilationWidth == FALSE ){
        ++err;
#undef DEFAULTS_VALUES
#undef DEFAULTS_VALUE
#undef MISSING_SAME
#undef VALS_HAVE
    }
    // 'vals' may not describe a realizable convolution
    //
    // User-supplied values are NOT modified, and compute_xx values
    // are NOT rigorously checked.
    // (e.g. dense memory layout, group and channel consistency, etc.)
    //
    // Only after 'mkConsistent' should libvednn tests have
    // well-defined behavior.
    //
    if(v) {
        char b[80];
        printf(" fillin END vals=%s\n",to_cstr(vals,b,80));
        printf("            have=%s\n",to_cstr(have,b,80));
    }
    return err;
}

/** return # of modifications needed to make the test look reasonable.
 * Reasonable, for libvednn tests, actually means 'dense layouts',
 * so output size is "exact".
 * p must be OneDNN convention.
 */
int mkConsistent( ConvParamRaw* p )
{
    // fix bad things so at least some test gets run.
    // (If I change padding or dilation or ... I am too lazy
    //  to calculate the new output size, for example).
    //
    // I try not to weed out valid test settings...
    //
    // apply changes, warn...
    int changed = 0;
#define FIXIT( ITEM, EXPR ) do \
    { \
        int const val = (EXPR); \
        if( p->ITEM != val ){ \
            printf(" " #ITEM " %d->%d ",(int)p->ITEM, val); \
            p->ITEM = val; \
            ++changed; \
        } \
    }while(0)
    // dil_start SHOULD already be known, but ...
    if(p->dil_start!=0 || p->dil_start!=1) FIXIT(dil_start, 0);
    int dil_start = p->dil_start;

    // We can change g,ic,oc at will to satisfy ic%g==0 and oc%g==0
    int g = p->group;
    // g cannot be larger then p->inChannel
    if (g > p->inChannel) g = p->inChannel;
    // g must divide evenly into p->inChannel
    while( p->inChannel % g != 0 ) --g;
    FIXIT( group, g );

    int ic = upMul( p->inChannel, g );
    //printf(" p:ic,oc=%d,%d ", p->inChannel, p->outChannel);
    FIXIT( inChannel, ic );
    // One pass over input channels uses ic/g kernels to make g output channels.
    // So round oc up to a multiple of g
    int oc = upMul( p->outChannel, g );
    FIXIT( outChannel, oc ); // any number is fine
    //printf(" -->ic,oc=%d,%d ", ic,oc);
    //printf(" p:ic,oc=%d,%d\n", p->inChannel, p->outChannel);

    // "no dilation" is p->dil_start
    int dh = p->dilationHeight;
    int dw = p->dilationWidth;
    if(dh<p->dil_start) dh=dil_start;
    if(dw<p->dil_start) dw=dil_start;
    FIXIT( dilationHeight, dh );
    FIXIT( dilationWidth,  dw );

    if(p->padHeight < 0) FIXIT( padHeight, 0 );
    if(p->padWidth  < 0) FIXIT( padWidth,  0 );
    if(p->kernHeight < 1) FIXIT( kernHeight, 3 );
    if(p->kernWidth  < 1) FIXIT( kernWidth,  3 );
    if(p->strideHeight < 1) FIXIT( strideHeight, 1 );
    if(p->strideWidth  < 1) FIXIT( strideWidth,  1 );

    //int min_inHeight = p->kernHeight + 2*p->padHeight;
    int min_inHeight = 1;
    if(p->inHeight < min_inHeight) FIXIT(inHeight, min_inHeight);
    //int min_inWidth = p->kernWidth + 2*p->padWidth;
    int min_inWidth = 1;
    if(p->inWidth < min_inWidth) FIXIT(inWidth, min_inWidth);

    // bump input size until output dimension positive
    ConvParamRaw q = *p;
    int oh = compute_oh(&q);
    for(; oh <= 0; ++q.ih) oh = compute_oh(&q); // bump ih until oh>0
    FIXIT( inHeight, q.ih);
    FIXIT( outHeight, oh);
    int ow = compute_ow(&q);
    for(; ow <= 0; ++q.iw) ow = compute_ow(&q); // bump iw until ow>0
    FIXIT( inWidth, q.iw);
    FIXIT( outWidth, ow);
#undef FIXIT

    if (changed){
        printf("mkConsistent: %d changes\n", changed);
        fflush(stdout);
    }
    return changed;
}

int mkConsistentOverrides( ConvParamRaw *p, ConvParamRaw const *ovr)
{
    // fix bad things so at least some test gets run.
    // (If I change padding or dilation or ... I am too lazy
    //  to calculate the new output size, for example).
    //
    // I try not to weed out valid test settings...
    // But never change non-CONVPARAM_UNSET settings in 'ovr'
    //
    // apply changes, warn...

    //if (p->dil_start==CONVPARAM_UNSET) p->dil_start = 0; // OneDNN convention
    assert( p->dil_start == 0 || p->dil_start == 1 );
    int dil_start = p->dil_start;

    int changed = apply_overrides(p, ovr);
    printf(" consistency ... ");
#define MODIFIABLE(PARAM) (ovr->PARAM != CONVPARAM_UNSET)
#define FIXIT( ITEM, EXPR ) do \
    { \
        int const val = (EXPR); \
        if( p->ITEM != val ){ \
            printf(" " #ITEM " %d->%d",(int)p->ITEM, val); \
            p->ITEM = val; \
            ++changed; \
        } \
    }while(0)

    int g = p->group;
    if( g < 1 ) FIXIT( group, g = 1 );
    int ic = p->inChannel;
    if( ic < 1 ) FIXIT( inChannel, ic = 1 );
    int oc = p->outChannel;
    if( oc < 1 ) FIXIT( outChannel, oc = 1);
    if( p->strideHeight < 1 ) FIXIT(strideHeight, 1);
    if( p->strideWidth  < 1 ) FIXIT(strideWidth,  1);
    if( p->dilationHeight < dil_start ) FIXIT(dilationHeight, dil_start);
    if( p->dilationWidth  < dil_start ) FIXIT(dilationWidth,  dil_start);

    // ic AND oc must be divisible by g
    //printf(" p:g,ic,oc=%d,%d,%d ", (int)(p->group), (int)(p->inChannel), (int)(p->outChannel));
    if(MODIFIABLE(group) && (ic%g || oc%g)){
        // set g to gcd of ic,oc   See also vejit/include/intutil.hpp, but this is 'C' code...
        int gcd=0;
        {
            int a=ic, b=oc;
            for(;;) {
                if(a==0){ gcd=b; break; }
                b %= a;
                if(b==0){ gcd=a; break; }
            }
        }
        //printf(" gcd%d", gcd);
        g = gcd;
    }else{ // keep g, and potentially modify ic AND oc (simplest way)
        //printf(" b");
        ic = upMul(ic,g);
        oc = upMul(oc,g);
    }
    //printf(" -->g,ic,oc=%d,%d,%d ", g,ic,oc);
    FIXIT( group, g );
    FIXIT( inChannel, ic );
    FIXIT( outChannel, oc );
    //printf(" p:g,ic,oc=%d,%d,%d ", (int)(p->group), (int)(p->inChannel), (int)(p->outChannel));

    // "no dilation" is p->dil_start (0 for OneDNN; 1 for vednn)
    int dh = p->dilationHeight;
    int dw = p->dilationWidth;
    int kh = p->kernHeight;
    int kw = p->kernWidth;
    if(dh<dil_start || kh==1/*dh irrelevant*/) dh=dil_start;
    if(dw<dil_start || kw==1/*dw irrelvant */) dw=dil_start;
    FIXIT( dilationHeight, dh );
    FIXIT( dilationWidth,  dw );

    if(kh < 1) FIXIT( kernHeight, kh=3 );
    if(kw < 1) FIXIT( kernWidth,  kw=3 );

    // TODO extra padding not handled well by vednn tests (SHOULD be allowed):
    // TODO check even kernels
    if(p->padHeight < 0)
        FIXIT( padHeight, 0 );
    else if( p->padHeight ==0 )
        ; // always acceptable
    else if( p->kernHeight != 2*p->padHeight + 1 )
        FIXIT( padHeight, (p->kernHeight - 1)/2 );

    if(p->padWidth  < 0)
        FIXIT( padWidth,  0 );
    else if( p->padWidth ==0 )
        ; // always acceptable
    else if( p->kernWidth  != 2*p->padWidth  + 1 )
        FIXIT( padWidth , (p->kernWidth  - 1)/2 );

    printf(" (kh=%d~%d)",kh,p->kernHeight);

    //if(p->inHeight <= 0) FIXIT( inHeight, 1 );
    //if(p->inWidth  <= 0) FIXIT( inWidth,  1 );
    int min_inHeight = p->kernHeight + 2*p->padHeight;
    if(p->inHeight < min_inHeight) FIXIT(inHeight, min_inHeight);
    int min_inWidth = p->kernWidth + 2*p->padWidth;
    if(p->inWidth < min_inWidth) FIXIT(inWidth, min_inWidth);

    // Req: libvednn p->oh==oh, but OneDNN p->oh>=oh
    // Here we enforce strict equality, for libvednn tests.
    //
    // Testing often has you fiddling with selections of conv values.
    // Some may be crazy, others nonstandard...
    //
    // Still want to end up with a usable libvednn test case.
    // But adhering to ovr override values "as much as possible".
    //
    // NOTE readParamString does a more careful job (bugfix ow Aug 2019)
    //      (almost foolproof trick of just setting kh=kw=1)
    // Here we change the logic to see if ih, then iw, then pw are
    // modifiable and can improve things.
    // Then we resort to more brute-force ih/oh consistency,
    // without regard to "MODIFIABLE".

    int oh = compute_oh(p);
    if(MODIFIABLE( outHeight )){
        printf(" oh:compute_out(%d,%d,%d,%d,%d)->%d", p->ih , p->kh,
                p->sh, p->ph , p->dh, oh);
        FIXIT( outHeight, oh );
    }
    int ow = compute_ow(p);
    if(MODIFIABLE(outWidth)){
        printf(" ow:compute_out(%d,%d,%d,%d,%d)->%d", p->iw , p->kw,
                p->sw , p->pw , p->dw, ow);
        FIXIT( outWidth,  ow );
    }
    // (oh, ow may still be <=0, even if modifiable)
    // Some easy dim fixes: bump input size until output dimension positive
    oh = compute_oh(p);
    if(p->oh <= 0 && MODIFIABLE( inHeight )){ // bump ih until about right?
        ConvParamRaw q = *p;
        oh = compute_oh(&q);
        for(; oh <= 0; ++q.ih) oh = compute_oh(&q);
        FIXIT( inHeight, q.ih);
        FIXIT( outHeight, oh);
    }
    ow = compute_ow(p);
    if(p->ow <= 0 && MODIFIABLE( inWidth )){ // bump iw until about right?
        ConvParamRaw q = *p;
        ow = compute_ow(&q);
        for(; ow <= 0; ++q.iw) ow = compute_ow(&q);
        FIXIT( inWidth, q.iw);
        FIXIT( outWidth, ow);
    }
    oh = compute_oh(p);
    if(oh <= p->oh && MODIFIABLE( padHeight )){ // bump pad until about right?
        ConvParamRaw q = *p;
        for(; oh <= p->oh; ++q.ph) oh = compute_oh(&q);
        FIXIT( padHeight, q.ph);
    }
    ow = compute_ow(p);
    if(ow <= p->ow && MODIFIABLE( padWidth )){ // bump pad until about right?
        ConvParamRaw q = *p;
        for(; ow <= p->ow; ++q.pw) ow = compute_ow(&q);
        FIXIT( padWidth, q.pw);
    }
    // last resort: ignore MODIFIABLE hints
    oh = compute_oh(p);
    if (oh > 0 && oh != p->oh){ // modify oh even if !MODIFIABLE
        FIXIT( outHeight, oh);
    }else if (oh <= 0){         // modify ih and oh even if !MODIFIABLE
        ConvParamRaw q = *p;
        for(; oh <= 0; ++q.ih) oh = compute_oh(&q); // bump ih until oh>0
        FIXIT( inHeight, q.ih);
        FIXIT( outHeight, oh);
    }
    ow = compute_ow(p);
    if (ow > 0 && ow != p->ow){ // modify ow even if !MODIFIABLE
        FIXIT( outWidth, ow);
    }else if (ow <= 0){         // modify iw and ow even if !MODIFIABLE
        ConvParamRaw q = *p;
        for(; ow <= 0; ++q.iw) ow = compute_ow(&q); // bump iw until ow>0
        FIXIT( inWidth, q.iw);
        FIXIT( outWidth, ow);
    }
#undef FIXIT
#undef MODIFIABLE

    if (changed){
        printf("mkConsistentOverrides: %d changes\n", changed);
        fflush(stdout);
    }
    return changed;
}

// always return OneDNN ConvParamRaw
int readConvParamOvr( char const* const _cstr,
        ConvParamRaw const* const ovr,
        ConvParamRaw* const r, char** const name)
{
    int v=0;
    assert( *name == NULL );
    //*name = NULL; // if not, potential mem leak
    *r = convParamRaw_unset;
    ConvParamRaw vals;
    memset(&vals, 0, sizeof(ConvParamRaw));
    int err=0;
    if(v){printf("read...begin\n"); fflush(stdout);}
    {
        ConvParamRaw have;
        memset(&have, 0, sizeof(ConvParamRaw));
        if(v){printf("read...preparse\n"); fflush(stdout);}
        err = preparseConvParamRaw( _cstr, &vals, &have, name );
        if (err)
            return err; // return w/ default r, invalid

        if (ovr) {
#define OVR(ITEM) \
            do { \
                if (ovr->ITEM != CONVPARAM_UNSET) { \
                    vals.ITEM = ovr->ITEM; \
                    have.ITEM = TRUE; \
                }}while(0)
            OVR(dil_start);
            OVR(dirn);
            OVR(layout);
            OVR(batchNum);
            OVR(group);
            OVR(inChannel);
            OVR(inHeight);
            OVR(inWidth);
            OVR(outChannel);
            OVR(outHeight);
            OVR(outWidth);
            OVR(kernHeight);
            OVR(kernWidth);
            OVR(strideHeight);
            OVR(strideWidth);
            OVR(padHeight);
            OVR(padWidth);
            OVR(dilationHeight);
            OVR(dilationWidth);
#undef OVR
        }

        // Supply something for all missing values
        if(v){printf("read...fillin\n"); fflush(stdout);}
        err = fillinConvParamRaw( &vals, &have );
        // DO NOT apply many consistency checks,
        //        modifying vals to "something valid"
        // int nChanges = mkConsistent(vals);
    }
    if (err)
        return err; // return w/ default r, invalid

    if(v){printf("read...success\n"); fflush(stdout);}
    *r = vals; // successful parse
    //cvt_onednn(r); // just in case
    return 0;
}

#if 0
// Does this have issues?
//
// IT IS SLOW !! !! !!
//
/// fwd conv, dump loop count, as in Onednn (benchdnn)
unsigned long long count_ops(ConvParamRaw const* const r){
    unsigned long long sp_ops = 0;
    //int const dd = r->dilationDepth - r->dil_start + 1;
    int const dh = r->dilationHeight - r->dil_start + 1;
    int const dw = r->dilationWidth - r->dil_start + 1;
    //for (int od = 0; od < this->od; ++od) {
    for (int oh = 0; oh < r->outHeight; ++oh) {
        for (int ow = 0; ow < r->outWidth; ++ow) {
            //for (int kd = 0; kd < this->kd; ++kd) {
            //    const int id = od * this->sd - this->pd + kd * dd;
            //    if (id < 0 || id >= this->id) continue;
            for (int kh = 0; kh < r->kernHeight; ++kh) {
                //const int ih = oh * this->sh - this->ph + kh * dh;
                const int ih = oh * r->strideHeight - r->padHeight + kh * dh;
                if (ih < 0 || ih >= r->inHeight) continue;
                for (int kw = 0; kw < r->kernWidth; ++kw) {
                    //const int iw = ow * this->sw - this->pw + kw * dw;
                    const int iw = ow * r->strideWidth - r->padWidth + kw * dw;
                    if (iw < 0 || iw >= r->inWidth) continue;
                    ++sp_ops;
                }
            }
            //}
        }
    }
    //}

    //ops = 2 * this->mb * this->oc * this->ic / this->g * sp_ops;
    return 2 * r->batchNum * r->outChannel * (r->inChannel / r->group) * sp_ops;
}
#else
//
// Here is a faster 'C' version, via code transforms in test_loop_count.cpp test program
// Identical for all 9k test convolutions.
// But FAST
//
// We C-ify relevant bits of idiv.hpp and hoist.hpp
/** division by +ve integer, rounding toward -ve infinity.
 *
 * \sa idiv.hpp
 * \pre d > 0 (unchecked)
 * \return n/d rounded downward toward \f$-\infty\f$
 * \note This fast case often applies to strided "forward"
 *       index calculations.  Any-valued strides should use
 *       \c div_floorx. Negative strides can use \c div_floorn.
 */
static inline int div_floor( int const n, int const d )
{
    return (n/d) - (n%d<0? 1: 0);
}

/// see hoist.hpp
static inline void hoist_ApiB(
        int* ilo, int* ihi,                     // effective for(i) loop range
        const int ibeg, const int iend,         // orig for(i=ibeg;i<end;i+=...)
        const int a,    const int b,            // linear fn o=a+ib
        const int obeg, const int oend)         // linear fn range [obeg,oend)
{
    // div_floor approach for int args, not the unsigned generalization
    assert( b > 0 );
    *ilo = div_floor( obeg -a+b-1, b );
    *ihi = div_floor( oend -a+b-1, b );
    if( *ilo < ibeg ) *ilo = ibeg;
    //else if( ilo > iend ) ilo = iend; // intentionally NOT enforced
    if( *ihi > iend ) *ihi = iend;
    //else if( ihi < ibeg ) ihi = ibeg; // intentionally NOT enforced

    //
    // original assumes "for" loop still exists.
    // We want to also guarantee that ihi - ilo is >= 0
    // So we ADD a quick check:
    //
    if (*ilo > *ihi) *ilo = *ihi;
    //
    // This now guarantees *ihi >= *ilo
    //
}

/// specialize to `for(i=0;i<iend;++i) {o=a+ib; if(o<0 || o>=oend) continue; BODY;}`.
/// Here ibeg=obeg=0. @pre b>0.
/// @return just a correct loop-BODY execution count.
static inline int hoist0_ApiB_count(
        const int iend,                 // orig for(i=0;i<end;i+=...)
        const int a, const int b,       // linear fn o=a+ib, b>0
        const int oend)                 // linear fn range [0,oend)
{
    int ilo = div_floor( /*obeg=0*/ -a+b-1, b );
    int ihi = div_floor(   oend     -a+b-1, b );
    ilo = (ilo<0    ? 0  : ilo );
    ihi = (ihi<iend ? ihi: iend);
    return (ilo < ihi? ihi-ilo: 0);
}

unsigned long long count_ops(ConvParamRaw const* const r)
{
    unsigned long long sp_ops = 0;
    // Note: ConvParamRaw does not support 'depth'
    //int const dd = r->dilationDepth - r->dil_start + 1;
    int const dh = r->dilationHeight - r->dil_start + 1;
    int const dw = r->dilationWidth - r->dil_start + 1;
    //for (int od = 0; od < this->od; ++od) {
#define OW_BLOCKING 512
    for (int ow0 = 0; ow0 < r->outWidth; ow0 += OW_BLOCKING) {
        int kwloops[OW_BLOCKING];              // <-- fixed-size memo area
        int ow9 = ow0 + OW_BLOCKING;
        ow9 = (ow9 < r->outWidth? ow9: r->outWidth);
        // block loop memoization, kwloops[] on stack
        for (int ow = ow0; ow < ow9; ++ow) {
            kwloops[ow-ow0] = hoist0_ApiB_count( r->kernWidth,
                    ow * r->strideWidth - r->padWidth, dw, r->inWidth);
        }
        // now the oh- & ow-loops
        for (int oh = 0; oh < r->outHeight; ++oh) {
            int khloops = hoist0_ApiB_count( r->kernHeight,
                    oh * r->strideHeight - r->padHeight, dh, r->inHeight);
            for (int ow = 0; ow < ow9-ow0; ++ow) {
                // accumulate body count of kh- and kw-loops
                sp_ops += khloops * kwloops[ow];
            }
        }
    }
    //}
    return 2 * r->batchNum * r->outChannel * (r->inChannel / r->group) * sp_ops;
}
#endif // count_ops orig or fast?

#ifdef __cplusplus
}//extern "C"
#endif
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
