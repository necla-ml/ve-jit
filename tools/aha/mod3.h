// mod 3 using VE packed-vector ops ONLY
// VE packed vector operations:
// Fixed:
//     pvsll pvsrl pvsla     
//     pvaddu pvadds pvsubu pvsubs
//     pvcmpu pvcmps pvmaxs pvmins
//     pvand pvor pvxor pveqvi
//     pvldz pvpcnt pvbrv
// Float:
//     pvfadd pvfsub pvfmul pvfdiv
//     pvfcmp pvfmax pvfmin
//     pvfmad pvfmsb pvfnmad pvfnmsb
//     pvrcp pvcvt[float<-->int32]
// Other:
//     pvbrd pvseqlo
//

#include <stdint.h>
/* This is the function for which it is desired to find more efficient
code. It must have either one or two arguments, both int, and must
return a 32-bit int quantity. */

#define NARGS 1                 // Number of args in userfun, 1 or 2.
const int debug = 0;            // 0 or 1; debugging printouts if 1.
const int counters = 1;         // 0 or 1; count number of evaluations.
#if NARGS == 1
uint_least32_t userfun(uint_least32_t x){
    return x%3;
}
#else
uint_least32_t userfun(uint_least32_t x, uint_least32_t /*xprev*/) {
    x=x+1;
    if(x>=3) x=0;
    return x;
}
#endif

/* A note about the registers:

They are divided into four groups.  The first group, starting with
register 0, holds ordinary immediate values.  The second group, starting
with register NIM, holds the shift immediate values.  The next 1 or 2
regs are the arguments to the user-defined function.  The last group
holds the results of computations done by the trial programs.

0        Start of ordinary immediate values (those given by IMMEDS)
NIM      Start of shift immediate values    (those given by SHIMMEDS)
RX       First (or only) user function argument
RY       Second user function argument
RI0      Result of instruction 0 goes here
RI0 + i  Result of instruction i goes here
where:
NIM   = number of ordinary immediate values
NSHIM = number of shift immediate values
*/

#define MAXNEG 0x80000000
#define MAXPOS 0x7FFFFFFF
#define NBSM 63                 // Shift mask.  Use 63 for mod 64
                                // shifts, or 31 for mod 32.

int trialx[] = {239, 39, 88,
    0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25,26,27,28,29,30,31,32,33,
    40,41,42,43, 63,64,65, 70,71,72, 80,81,82, 90,91,92, 100,101,102, 127,128,129, 200,201,202, 252,253,254,255
};
#if NARGS == 1
int trialy[] = {0, 1, 2, 3, 4, 7, 8, 9, 31,32,33, 63,64,65, 127,128,129}; //, 1023,1024,1025};
#endif
// First three values of IMMEDS must be 0, -1, and 1.
#define IMMEDS 0, -1, 1, (int)MAXNEG, -2, 2, 3, -3, 4, \
    1<<0, 1<<2, 1<<4, 1<<6, 1<<8, 1<<10, 1<<12, 1<<14, 1<<16, \
((1<<0)+(1<<2)+(1<<4)+(1<<6)+(1<<8)+(1<<10)+(1<<12)+(1<<14)+(1<<16))
#define SHIMMEDS 1, 2, 3, 4, 7,8, 9, 10, 11, 12, 13, 14, 15, 16, 17

int dummy1[] = {IMMEDS};        // These get optimized out of existence.
int dummy2[] = {SHIMMEDS};

#define NIM (int)(sizeof(dummy1)/sizeof(dummy1[0]))
#define NSHIM (int)(sizeof(dummy2)/sizeof(dummy2[0]))
#define RX (NIM + NSHIM)        // First (or only) user function argument
#define RY (RX + 1)             // Second user function argument
#define RI0 (RX + NARGS)        // Result of instruction 0 goes here

int unacceptable;               // Code below sets this to 1 for an
                                // unacceptable operation, such as
                                // divide by 0.  It is initially 0.

/* Collection of simulator routines for the instructions in the isa. */
int neg(int x, int, int) {return -x;}
int _not(int x, int, int) {return ~x;}
int pop(int xx, int, int) {
   unsigned x = xx;
   x = x - ((x >> 1) & 0x55555555);
   x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
   x = (x + (x >> 4)) & 0x0F0F0F0F;
   x = x + (x << 8);
   x = x + (x << 16);
   return x >> 24;
}
int brv(int xx, int, int) { /* bit-reverse, not fancy */
    unsigned int v = xx;     // input bits to be reversed
    unsigned int r = v; // r will be reversed bits of v; first get LSB of v
    int s = sizeof(v) * 8 - 1; // extra shift needed at end

    for (v >>= 1; v; v >>= 1)
    {   
        r <<= 1;
        r |= v & 1;
        --s;
    }
    r <<= s; // shift when v's highest bits are zero
    return r;
}

int nlz(int xx, int, int) {
   unsigned x = xx;
   int n;

   if (x == 0) return(32);
   n = 0;
   if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
   if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
   if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
   if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
   if (x <= 0x7FFFFFFF) {n = n + 1;}
   return n;
}

int rev(int xi, int, int) {
   unsigned x = xi;
   x = (x & 0x55555555) <<  1 | ((x >>  1) & 0x55555555);
   x = (x & 0x33333333) <<  2 | ((x >>  2) & 0x33333333);
   x = (x & 0x0F0F0F0F) <<  4 | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) |
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

int add (int x, int y, int) {return x + y;}
int sub (int x, int y, int) {return x - y;}
int mul (int x, int y, int) {return x * y;}
int max (int x, int y, int) {return x>y? x: y;}
int min (int x, int y, int) {return x<y? x: y;}
/* For division overflow we return arbitrary values, hoping they fail
to be part of a solution.  (User must check solutions, in general.) */
int div (int x, int y, int) {
   if (y == 0 || (y == -1 && x == (int)0x80000000))
      {unacceptable = 1; return 0;}
   else return x/y;}
int divu(int x, int y, int) {
   if (y == 0) {unacceptable = 1; return 0;}
   else return (int)((unsigned)x/(unsigned)y);}
int _and(int x, int y, int) {return x & y;}
int _or (int x, int y, int) {return x | y;}
int _xor(int x, int y, int) {return x ^ y;}
int _nand(int x, int y, int) {return (~x) ^ y;}
int _eqv(int x, int y, int) {return ~(x^y);}
int _mrg(int x, int y, int z) {return (x&(~z))|(y&z);}//bitwise merge
// note: often chips use lsb's of 's' only
int rotl(int x, int y, int) {int s = y & NBSM;
   return x << s | (unsigned)x >> (32 - s);}
int shl (int x, int y, int) {int s = y & NBSM;
   if (s >= 32) return 0; else return x << s;}
int shr(int x, int y, int) {int s = y & NBSM;
   if (s >= 32) return 0; else return (unsigned)x >> s;}
int shrs(int x, int y, int) {int s = y & NBSM;
   if (s >= 32) return x >> 31; else return x >> s;}
// aurora does not have these, instead have cmov   
int cmpeq(int x, int y, int) {return x == y;}
int cmplt(int x, int y, int) {return x < y;}
int cmpltu(int x, int y, int) {return (unsigned)(x) < (unsigned)(y);}
// aurora has many cmov... x = (cond(y)? x: z), but I don't know how to represent
// register-modifying instructions in aha!
int cmoveq(int x, int y, int z) {return y == 0 ? x : z;}
int cmovgt(int x, int y, int z) {return y > 0 ? x : z;}
int cmovlt(int x, int y, int z) {return y < 0 ? x : z;}

int seleq(int x, int y, int z) {return x == 0 ? y : z;}
int sellt(int x, int y, int z) {return x < 0 ? y : z;}
int selle(int x, int y, int z) {return x <= 0 ? y : z;}
// Aurora has cmov: if( cond(y,0) ) x; else z;

                                // The machine's instruction set:
// Note: Commutative ops are commutative in operands 0 and 1.
struct Isa {
   int  (*proc)(int, int, int); // Procedure for simulating the op.
   int  numopnds;               // Number of operands, 1 to 3.
   int  commutative;            // 1 if opnds 0 and 1 commutative.
   int  opndstart[3];           // Starting reg no. for each operand.
   int  outputreg;              // if 0,1,2, copy output into this operand reg
   char const*mnemonic;         // Name of op, for printing.
   char const*fun_name;         // Function name, for printing.
   char const*op_name;          // Operator name, for printing.
};
struct Isa isa[] = {
   {neg,    1, 0, {RX,  0,  0}, -1, "neg",   "-(",   ""     },  // Negate.
   {_not,   1, 0, {RX,  0,  0}, -1, "not",   "~(",   ""     },  // One's-complement.
   {pop,    1, 0, {RX,  0,  0}, -1, "pop",   "pop(", ""     },  // Population count.
   {nlz,    1, 0, {RX,  0,  0}, -1, "nlz",   "nlz(", ""     },  // Num leading 0's.
   {rev,    1, 0, {RX,  0,  0}, -1, "rev",   "rev(", ""     },  // Bit reversal.
//   {brv,    1, 0, {RX,  0,  0}, -1, "brv",   "brv(", ""     },  // Bit reverse. oops, dup
   {add,    2, 1, {RX,  2,  0}, -1, "add",   "(",    " + "  },  // Add.
   {sub,    2, 0, { 2,  2,  0}, -1, "sub",   "(",    " - "  },  // Subtract.
   {max,    2, 0, { 2,  2,  0}, -1, "max",   "(",    " - "  },  // Max.
   {min,    2, 0, { 2,  2,  0}, -1, "min",   "(",    " - "  },  // Min..
// {mul,    2, 1, {RX,  3,  0}, -1, "mul",   "(",    "*"    },  // Multiply.
// {div,    2, 0, { 1,  3,  0}, -1, "div",   "(",    "/"    },  // Divide signed.
// {divu,   2, 0, { 1,  1,  0}, -1, "divu",  "(",    " /u " },  // Divide unsigned.
   {_and,   2, 1, {RX,  2,  0}, -1, "and",   "(",    " & "  },  // AND.
   {_or,    2, 1, {RX,  2,  0}, -1, "or",    "(",    " | "  },  // OR.
   {_xor,   2, 1, {RX,  2,  0}, -1, "xor",   "(",    " ^ "  },  // XOR.
// {_nand,  2, 1, {RX,  2,  0}, -1, "nand",  "(",    " ^ "  },  // NAND.
   {_eqv,   2, 1, {RX,  2,  0}, -1, "eqv",   "(",    " ^ "  },  // EQV. == ~_xor
   {shr,    2, 0, { 1,NIM,  0}, -1, "shr",   "(",    " >>u "},  // Shift right.
   {shl,    2, 0, { 1,NIM,  0}, -1, "shl",   "(",    " << " },  // Shift left.
   {shrs,   2, 0, { 3,NIM,  0}, -1, "shrs",  "(",    " >>s "},  // Shift right signed.
//   {rotl,   2, 0, { 1,NIM,  0}, -1, "rotl",  "(",    " <<r "},  // Rotate shift left.
//   {_mrg,   3, 0, {RX,  0,  0},  0, "mrg",   "mrg(",   ", " },  // Merge. x = {x&(~z)}|{y&z}
//   {cmoveq, 3, 0, {RX, RX,  0},  0, "cmoveq","cmoveq(", ", "},  // CMOV x=(signed_cnd(y)?x:z)
//   {cmovgt, 3, 0, {RX, RX,  0},  0, "cmovgt","cmovgt(", ", "},  // CMOV x=((y>0?z:x)
//   {cmovlt, 3, 0, {RX, RX,  0},  0, "cmovlt","cmovlt(", ", "},  // CMOV x=((y<0?z:x)
// {cmpeq,  2, 1, {RX,  0,  0}, -1, "cmpeq", "(",    " == " },  // Compare equal.
// {cmplt,  2, 0, { 0,  0,  0}, -1, "cmplt", "(",    " < "  },  // Compare less than.
// {cmpltu, 2, 0, { 1,  1,  0}, -1, "cmpltu","(",    " <u " },  // Compare less than unsigned.
//   {seleq,  3, 0, {RX,  0,  0}, -1, "seleq", "seleq(", ", " },  // Select if = 0.
//   {sellt,  3, 0, {RX,  0,  0}, -1, "sellt", "sellt(", ", " },  // Select if < 0.
//   {selle,  3, 0, {RX,  0,  0}, -1, "selle", "selle(", ", " },  // Select if <= 0.
};

/* ------------------- End of user-setup Portion -------------------- */

#define MAXNUMI 5               // Max num of insns that can be tried.

#define NTRIALX (int)(sizeof(trialx)/sizeof(trialx[0]))
#define NTRIALY (int)(sizeof(trialy)/sizeof(trialy[0]))

#if NARGS == 1
   int correct_result[NTRIALX];
#elif NARGS == -1
   int correct_result[NTRIALX];
#else
   int correct_result[NTRIALX][NTRIALY];
#endif

int corr_result;                // Correct result for current trial.

#define NUM_INSNS_IN_ISA (int)(sizeof(isa)/sizeof(isa[0]))

struct Pgm {                        // The current program.
   int op;                      // Index into isa.
   int opnd[3];                 // Operands of op.  Register numbers
                                // except if negative, it's the negative
                                // of a shift amount.
};
struct Pgm pgm[MAXNUMI];

int numi;                       // Current size of the trial programs,
                                // must be from 1 to MAXNUMI.

/* GPR array:  First NIM slots hold ordinary immediate values (IMMEDS),
next NSHIM slots hold shift immediate values (SHIMMEDS), next NARGS
slots hold the arguments x and, optionally, y, and the last numi slots
hold the result of instructions 0 through numi - 1. */

int r[NIM + NSHIM + NARGS + MAXNUMI] = {IMMEDS, SHIMMEDS};
unsigned counter[MAXNUMI];        // Count num times insn at level i is
                                  // evaluated.
