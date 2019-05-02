#ifndef R64_H_
#define R64_H_
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file r64.h
 * \brief poor-man's replacement for drand48 (fast, \b low-quality random numbers)
 */
#include <stdint.h>
//namespace sim
//{
    /** This is an old, simple, low-quality pseudorandom sequence.
     * If not too concerned about quality, this is a fast, inlinable
     * way to replace function calls to drand and friends.
     *
     * - Some constants with decent bit-shuffling properties are:
     *   - #define LCGMUL 1181783497276652981ULL
     *   - #define LCGINV 13515856136758413469ULL
     *   - #define LCGADD 76543217654321ULL
     *   - #define LCGMUL2 7664345821815920749ULL
     *   - #define LCGMUL3 2685821657736338717ULL
     * - MUL  MUL2 MUL3  are for m=2^64, c=0, and lack 8/16/32-D correlation
     *   - mlcg2exp64 _multiple and _inverse (LCGADD is any odd number, for now).
     *     Somewhere I coded forward/backward by N steps and direct access
     *     routines to M'th rand (iterate over first 64 powers of the constants)
     * - Note that LCGMUL and LCGINV will have equiv. statistical properties,
     *   if you need multiple, rands (ex. multi-dimensional hashing).
     * - MUL4 MUL5 MUL6  are full period for any odd c, decorrelated 8/16/32-tuples
     *   - #define LCGMUL4 2862933555777941757ULL
     *   - #define LCGMUL5 3202034522624059733ULL
     *   - #define LCGMUL6 3935559000370003845ULL
     *
     * - Above constants are from<br>
     *   https://www.iro.umontreal.ca/~lecuyer/myftp/papers/latrules.ps
     *   "Tables of Linear Congruential Generators of Different Sizes and
     *    Good Lattice Structure".
     *   - If using your own constant M, have (M&7)==5, \b and test them
     *     with a RNG test suite (ex. crush in TESTU01)
     *
     * - you can also go \c N steps forward/backward or directly access the
     *   \c N'th random number, but these are slower operations that iterate
     *   over bits of \c N and use lookup tables of powers of multipliers (or
     *   their multiplicative inverses), so these routines are not included.
     *   - If you really need 'direct access' rands, better to use something
     *     of form \f$rand(n) = mixer(K*n\f$),<br>
     *     see https://marc-b-reynolds.github.io/shf/2017/09/27/LPRNS.html
     *
     * - This project has no need for cryptographic hashing, so I often [mis]use
     *   this PRNG as a quick'n'dirty hash, ex. \ref vechash.hpp
     *
     * - For a fast \b VE RNG <B>that passes all BigCrush tests</B>, see
     *   <a href="http://www.pcg-random.org/pdf/hmc-cs-2014-0905.pdf">paper</a>
     *   and code at https://github.com/imneme/pcg-cpp
     *   - Note that VE has a double-wide unsigned right shift [SRD], so a
     *     really good and fast choice with 128 bit state (and 64 bit output)
     *     is PCG-XSL-RR-128/64, whose update is
     *     \f$output = rotate64(uint64_t(state ^ (state >> 64)), state >> 122);\f$
     *     - unfortunately, even clang with __uint128_t does not seem to produce
     *       the SRD opcode
     *   - a 32-bit generator with 64 bits state that is fast is PCG XSH RS 64/32
     */
#if 0 // there's a better way now... when I get around to it.
    struct PCG64 {
    };
#endif
    struct R64
    {
        R64(uint64_t const seed=123U) : r(seed) {}
        R64& operator++()
        {
            this->r = this->r * 2862933555777941757UL //LCGMUL4
                + 13U;
            return *this;
        }
        uint64_t operator()()
        {
            return (this->r = this->r * 2862933555777941757UL //LCGMUL4
                    + 13U);
        }
        uint64_t value() const { return r; }
        uint32_t u32() const { return r ^ (r>>32U); }
        //uint64_t u64() const { return r ^ (r>>32U); }
        uint64_t u64() const { return r; }
        void seed( uint64_t const seed )
        {
            r = seed;
        }
        /** returns 0.0 to 1.0(exclusive).  IEEE double is guaranteed to
         * have at least 53 signicand bits, so we'll base our double value
         * on ~50 bits....
         *
         * (Note that this is much more fine-grained than boost's uniform_01,
         *  but it still would be fastest to JUST twiddle the FP bits directly
         *  in a union{uint64_t;double;}  (but I don't have such twiddles handy)
         *
         * assembler code shows it using xmm registers (uggh!)
         *
         * Note: 50, magic constant, whereas
         *       gcc -dM -E - <<<''
         *       shows a predefined value
         *       #define __DBL_MANT_DIG__ 53
         */
        double drand()
        {
            return double( ((*this)()>>14)
                    * (1.0/(uint64_t(1U)<<50)) // compiler can precalculate this max val
                    );
        }
        private:
        //r50inv = double(1.0)/(uint64_t(1ULL<<50)));
        uint64_t r;
    };

//}//sim::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // R64_H_
