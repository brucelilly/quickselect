/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: snn_double - simple, fast interpolation of numbers to a string
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    snn_double.c copyright 2011 - 2017 Bruce Lilly.   \ snn_double.c $
* This software is provided 'as-is', without any express or implied warranty.
* In no event will the authors be held liable for any damages arising from the
* use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it freely,
* subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not claim
*    that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
****************************** (end of license) ******************************/
/* $Id: ~|^` @(#)   This is snn_double.c version 2.14 2017-02-05T17:40:36Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libsnn/src/s.snn_double.c */

/********************** Long description and rationale: ***********************
* snprintf() is a versatile function that can write formatted information into
*  strings with intricately controllable options.  Unfortunately, many of the
*  options are not portable, and even the return value is known to vary (int vs.
*  char *).  The flexibility comes with a price; parsing the format string
*  and variable argument list isn't cheap.  Moreover, it is necessary to know
*  how large a buffer to provide by means other than snprintf itself (in some
*  cases), or risk truncated output. Generating signed integer output in bases
*  other than 10 which is compatible with strtol() (i.e. -0x1234) can be
*  challenging.  Likewise, while snprintf() can render floating point numbers in
*  scientific notation, it is challenging to say the least to get it to produce
*  engineering notation (i.e. where the exponent is a multiple of 3,
*  corresponding to prefixes pico, nano, micro, milli, kilo, mega, giga, etc.
*  The programmer must specify a fixed precision (or use the fixed default) or
*  determine precision separately and provide it to snprintf() via an argument;
*  there is no provision to produce floating point output with only as many
*  digits as necessary (i.e. avoid trailing zeroes).
*
* The number formatting functions provided here have fewer, simpler formatting
*  options, but provide functionality missing from (or difficult to coerce
*  from) snprintf().
*
* All number formatting functions provided here return an integer which denotes
*  the number of characters written to the specified string -- if that string is
*  large enough -- or the necessary size (if the result pointer is NULL or if
*  the size is too small).  Each function takes as arguments a pointer to the
*  result buffer and the size of that buffer in characters.
* Therefore, these functions can themselves be used to determine the size needed
*  to hold the result, e.g.:
*         char *buf = NULL;
*         int n, sz = 0;
*         n = snul(buf, sz, "0x", "UL", 0x123abcUL, 16, (int)'0', 0);
*         buf = malloc(sz=n);
*         if (NULL != buf) {
*             n = snul(buf, sz, "0x", "UL", 0x123abcUL, 16, (int)'0', 0);
*             ...
*             free(buf);
*         }
*
* Each number formatting function also accepts optional fixed strings to be
*  prefixed and/or suffixed to the numeric output, as in the example above.  Not
*  as flexible as snprintf(), but also not as expensive, and more compatible
*  with string-to-number conversion functions (strtol, strtoul, strtod).
*
* Floating point type number formatting functions allow specification of
*  specific precision or significant figures, or (by specifying a negative
*  value) permit the function to determine the optimum precision avoiding
*  trailing zeroes.  Run-time determination of an upper bound on the available
*  precision limits meaningless trailing digits in the output (compare snf
*  to snprintf for 9.87e+35, for example).  Functions are provided for fixed
*  point (like snprintf's "%f") and scientific notation (similar to snprintf's
*  "%E") output; the latter permits specification of allowable increments for
*  the exponent, e.g. to use exponents which are multiples of three as for
*  engineering units.
*
*  int snf(char *buf, int sz, const char *prefix, const char *suffix, double d,
*    int pad, int min_dig, int precision);
*
*  int sng(char *buf, int sz, const char *prefix, const char *suffix, double d,
*    int max_sig_fig, int exp_mod);
*
* Additional utility functions are provided:
*
* snsf returns the number of significant figures in a specified floating point
*  number, limited to the run-time capability:
*
*  int snsf(double d);
*
* N.B. "snf" and "snsf" are distinct functions with similar names.
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: snn_double.c ~|^` @(#)"
#define SOURCE_MODULE "snn_double.c"
#define MODULE_VERSION "2.14"
#define MODULE_DATE "2017-02-05T17:40:36Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2017"

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 600
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files */
#include "snn.h"      /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum [isprint] isspace */
#ifndef EINVAL
# include <errno.h>             /* errno EINVAL */
#endif
#include <limits.h>             /* *_MAX LONG_BIT */
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
/* digits through base 36; used by snn_sf_double, snn_uint, snn_double */
static const char snn_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static char snn_double_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
/* run-time-initialized value slightly < 1.0, maximum significant figures */
static double snn_almost_one = 1.0;
static double snn_epsilon = 7.62939453125e-6; /* 2^-17 */
static int snn_msf = 15; /* maximum significant figures */

#define SNN_EXTREME_PRECISION 512

/* initialize snn_almost_one, etc. at run-time */
static void initialize_snn_double(void (*f)(int, void *, const char *, ...),
    void *log_arg)
{
    int n;
    volatile double d, e;       /* 'volatile' avoids bugs due to excessive optimization */
    const char *s;
#ifndef PP__FUNCTION__
    static const char __func__[] = "initialize_snn_double";
#endif

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    snn_double_initialized = register_build_strings(NULL, &source_file, s);
    /* Reduce snn_epsilon until snn_almost_one = (1.0 - snn_epsilon) is
       indistinguishable from 1.0: resulting snn_epsilon will be too small.
       Bail out if snn_epsilon falls below 1e-18 (gcc 6.2.0).
    */
    for (snn_almost_one=1.0-snn_epsilon; 1.0 > snn_almost_one; snn_epsilon *= 0.5) {
        snn_almost_one = 1.0 - snn_epsilon;
        if (1.0e-18>snn_epsilon) break;
    }
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: snn_almost_one=%.36g, snn_epsilon=%.36g (too small)",
            __func__, source_file, __LINE__,
            snn_almost_one, snn_epsilon
            );
    }
    /* revise based on tests at a few powers of ten */
    for (n=SNN_MIN_POW10+20; n<SNN_MAX_POW10-20; n+=31) {
        d = sn1en(n, f, log_arg);
        e = d * snn_almost_one;
        while (0.0 == d - e) {
            snn_epsilon *= 2.0;
            snn_almost_one = 1.0 - snn_epsilon;
            e = d * snn_almost_one;
        }
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: snn_almost_one=%.36g, snn_epsilon=%.36g (OK at %.19g)",
                __func__, source_file, __LINE__,
                snn_almost_one, snn_epsilon, d
                );
        }
    }
    snn_msf = snmagnitude(snn_almost_one / snn_epsilon, f, log_arg) - 1;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: snn_almost_one=%.36g, snn_epsilon=%.36g, snn_msf=%d",
            __func__, source_file, __LINE__,
            snn_almost_one, snn_epsilon, snn_msf
            );
    }
}

/* internal function returns d rounded to max_sig_figs or precision
   determine significant digits needed for decimal output, place in array pointed to by pdigits
   return significant figures count via psf
   return mag (possibly increased by rounding) via pmag
   return needed precision (possibly reduced by rounding) via pprec
*/
/* calls: initialize_snn */
/* called by: snsf, snf, sng */
static inline double snn_sf_double(double d,
    unsigned char *pdigits, unsigned int ndigits,
    int mag, int max_sig_figs, int max_precision,
    int *psf, int *pmag, int *pprec,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    unsigned char r;
    int h, i, inc=0, j, k, n, offs=0, p, sf;
    double x, z;
    volatile double g; /* volatile, else gcc 6.2.0 optimization causes errors */
#ifndef PP__FUNCTION__
    static const char __func__[] = "snn_sf_double";
#endif

    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, mag=%d, max_sig_figs=%d, max_precision=%d",
            __func__, source_file, __LINE__,
            d, mag, max_sig_figs, max_precision
            );
    }
    if ((0 == mag) || ((0 > mag) && (0 <= max_precision) && (-1-mag > max_precision))) { /* d is zero or must be output as zero */
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: mag=%d, max_precision=%d",
                __func__, source_file, __LINE__,
                mag, max_precision
                );
        }
        if (NULL != psf)
            *psf = 1;
        if (NULL != pmag)
            *pmag = 0;
        if (NULL != pprec)
            *pprec = 0;
        if (NULL != pdigits) {
            for (i=0; i<ndigits; i++)
                pdigits[i] = 0U;
        }
        return 0.0;
    }
    /* limit significant figures to what run-time can provide */
    sf = max_sig_figs;
    if (sf > snn_msf)
        sf = snn_msf;
    p = max_precision;
    if (0 > p)
        p = 0 - p;
    /* virtual translation of number to range */
    if (0 < mag)
        h = mag - 1;
    else /* 0 > mag */
        h = mag;
    if (p + h + 1 > sf)
        p = sf - h - 1;
    if (sf > h + p + 1)
        sf = h + p + 1;
    /* how far to shift to put all figures to be output in integer portion? */
    offs = sf - h - 1;
#if 0
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, h(mag)=%d, sf=%d, p=%d, offs=%d",
            __func__, source_file, __LINE__,
            d, h, sf, p, offs
            );
    }
#endif
    /* adjust by an offset to put all desired significant figures in integer portion */
    if (SNN_MAX_POW10 < offs) {
        n = offs - 205;
        g = d * sn1en(205, f, log_arg);
#if 0
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: g(pre-round)=%.24g, n=%d",
            __func__, source_file, __LINE__,
            g, n
            );
    }
#endif
        g = g * sn1en(n, f, log_arg);
    } else {
        g = d * sn1en(offs, f, log_arg);
    }
#if 0
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: g(pre-round)=%.24g",
            __func__, source_file, __LINE__,
            g
            );
    }
#endif
    z = g = snround(g, f, log_arg);
#if 0
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: g(rounded)=%.24g",
            __func__, source_file, __LINE__,
            g
            );
    }
#endif
    /* g >= 0.0 */
    if (0.0 > g)
        g = 0.0 - g;
    j = sf;
    for (i=0; (0.0!=g)&&(i<=sf)&&(i<ndigits);) {
        j--;
        x = sn1en(j, f, log_arg);
        for (r=0U; g >= x; r++) { g -= x; }
        if (0 == i) { /* first digit check */
            if (9U < r) {
                /* rounding increased magnitude; try again */
                if ((0.0 > d) && (0.0 < z))
                    z = 0.0 - z;
                if (0 != offs)
                    z = z * sn1en(0-offs, f, log_arg);
                h = mag++;
                if (0 == mag)
                    mag++;
                if (NULL != f) {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: rounding %.*g to %d significant figures rounded to %.*g changed magnitude from %d to %d",
                        __func__, source_file, __LINE__,
                        sf+3, d, sf, sf, z, h, mag
                        );
                }
                g = snn_sf_double(z, pdigits, ndigits, mag, max_sig_figs, max_precision, psf, pmag, pprec, f, log_arg);
                if ((0.0 > z) && (0.0 < g))
                    g = 0.0 - g;
                return g;
            }
        }
        pdigits[i++] = r;
    }
    while (i<ndigits)
        pdigits[i++] = 0U;
    if (NULL != f) {
        char buf[40];           /* RATS: ignore (large enough for 32 digits!) */

        for (n=0; n<i; n++)
            buf[n] = snn_digits[pdigits[n]];
        buf[n] = '\0';
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: digits: %s",
            __func__, source_file, __LINE__,
            buf
            );
    }
    if (0 < inc) /* rounding increased magnitude */
        mag++;
    if (0 == mag)
        mag++; /* went from -1 to 0; should be 1 */
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: inc=%d, mag=%d",
            __func__, source_file, __LINE__,
            inc, mag
            );
    }
    if (NULL != pmag)
        *pmag = mag;
    if (0 < mag)
        h = mag - 1;
    else /* 0 > mag */
        h = mag;
    for (j=0; j<ndigits; j++)
        if (0U != pdigits[j])
            break;
    for (sf=0,k=j-1; j<ndigits; j++) {
        if (0U != pdigits[j]) {
            sf += j-k; /* include sandwiched zeroes */
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: j=%d, k=%d, pdigits[j]=%u, sf=%d",
                    __func__, source_file, __LINE__,
                    j, k, pdigits[j], sf
                    );
            }
            k=j;      /* mark non-zero position */
        }
    }
    if (sf < 1)
        sf = 1;
    if (NULL != psf) {
        *psf = sf;
    }
    if (p + h + 1 > sf)
        p = sf - h - 1;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: max_precision=%d, p=%d",
            __func__, source_file, __LINE__,
            max_precision, p
            );
    }
    if (NULL != pprec)
        *pprec = p;
    if ((0.0 > d) && (0.0 < z))
        z = 0.0 - z;
    if (0 != offs)
        z = z * sn1en(0-offs, f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s returns %g",
            __func__, source_file, __LINE__,
            __func__, z
            );
    }
    return z;
}

/* return number of significant digits in d */
/* calls: initialize_snn, snmagnitude, snn_sf_double */
/* called by: no other snn functions */
/* no inline: Sun cc barfs */
int snsf(double d,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int r=1;
#ifndef PP__FUNCTION__
    static const char __func__[] = "snsf";
#endif

    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g)",
            __func__, source_file, __LINE__,
            __func__, d
            );
    }
    if (0.0 > d) d = 0.0 - d;
    /* nothing to do if d == 0.0 */
    if (0.0 < d) {
        unsigned char digits[32];       /* RATS: ignore (snn_sf_digits uses sizeof(digits)) */
        if ((unsigned char)0U == snn_double_initialized)
            initialize_snn_double(f, log_arg);
        (void)snn_sf_double(d, digits, sizeof(digits), snmagnitude(d, f, log_arg), snn_msf, SNN_EXTREME_PRECISION, &r, NULL, NULL, f, log_arg);
    }
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g) returns %d",
            __func__, source_file, __LINE__,
            __func__, d, r
            );
    }
    return r;
}

/* internal function that interpolates floating-point numbers
   buf (size sz) holds the result; return value is as for snf and sng
   prefix and suffix are also as for snf and sng
   pdigits (array of size ndigits) hold significant digits of number d
   d itself is used only for sign
   mag indicates magnitude (a la snmagnitude) of number
   offs indicates the position of the radix point relative to the digits
   pad, min_dig, and precision are as for snf
*/
/* calls: initialize_snn, isspace, snn_sf_double */
/* called by: snf, sng */
static inline int snn_double(char *buf, int sz,
    const char *prefix, const char *suffix,
    unsigned char *pdigits, unsigned int ndigits,
    double d, int mag, int offs, int pad, int min_dig, int precision,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char *p = buf;
    int r=0, k, n=0, spp, zi=0, zf=0;
    unsigned int i=0U;
#ifndef PP__FUNCTION__
    static const char __func__[] = "snn_double";
#endif

    /* type (whitespace or not) of padding */
    spp = isspace(pad);
    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24G, spp=%d, mag=%d, offs=%d, min_dig=%d, precision=%d",
            __func__, source_file, __LINE__,
            d, spp, mag, offs, min_dig, precision
            );
    }
    if (0 >= min_dig)
        min_dig = 1;
    /* how many of the significant digits are required for the integer part of number?  */
    if (0 < offs) {
        n = offs;
    } else if ((0 == offs) && (0 <= mag)) {
        n = 1;
    } else {
        n = 0;
    }
    /* how many zeroes (not padding) must precede the first significant digit in the integer part? */
    if (0 > offs) {
        zi = 1;
    }
    /* how many zeroes must precede the first significant digit in the fraction? */
    if ((0 < precision) && (0 >= offs) && (0-offs <= precision))
        zf = 0 - offs - 1;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24G, n=%d, zi=%d, zf=%d, spp=%d, mag=%d, offs=%d, min_dig=%d, precision=%d",
            __func__, source_file, __LINE__,
            d, n, zi, zf, spp, mag, offs, min_dig, precision
            );
    }
    /* whitespace padding goes first for strtod compatibility */
    if (0 != spp) {
         for (k=n; k < min_dig; r++,k++) { /* padding */
             if ((NULL != buf) && (r < sz))
                 *p++ = (char)pad;
         }
    }
    /* sign before prefix for strtod compatibility (in case prefix is 0x) */
    if (0.0 > d) {
        if ((NULL != buf) && (r < sz))
            *p++ = '-';
        r++;
    }
    /* prefix */
    if (NULL != prefix) {
        for (; '\0' != *prefix; r++, prefix++)
            if ((NULL != buf) && (r < sz))
                *p++ = *prefix;
    }
    /* non-whitespace padding (presumably a digit, probably '0') if required */
    if (0 == spp) {
         for (k=n+zi; k < min_dig; r++,k++) { /* padding */
             if ((NULL != buf) && (r < sz))
                 *p++ = (char)pad;
         }
    }
    /* interpolate number's integer part digits */
    /* leading non-significant zeroes */
    for (k=0; k<zi; k++) {
        if ((NULL != buf) && (r < sz))
            *p++ = snn_digits[0];
        r++;
    }
    /* significant figures before radix point */
    while (0 < n) {
        n--;
        if (i >= ndigits) k=0; else k = pdigits[i++];
        if ((NULL != buf) && (r < sz))
            *p++ = snn_digits[k];
        r++;
    }
    /* radix point (if required) and fraction part digits */
    if (zf < precision) {
        if ((NULL != buf) && (r < sz))
            *p++ = '.';
        r++;
        /* leading non-significant zeroes */
        for (k=0; k<zf; k++) {
            if ((NULL != buf) && (r < sz))
                *p++ = snn_digits[0];
            r++;
        }
        precision -= zf;
        /* interpolate number's fraction part digits */
        while (0 < precision) {
            precision--;
            if (i >= ndigits) k=0; else k = pdigits[i++];
            if ((NULL != buf) && (r < sz))
                *p++ = snn_digits[k];
            r++;
        }
    }
    /* suffix */
    if (NULL != suffix) {
        for (; '\0' != *suffix; r++, suffix++)
            if ((NULL != buf) && (r < sz))
                *p++ = *suffix;
    }
    if ((NULL != buf) && (r < sz)) {
        *p = '\0';
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: %s wrote %d byte%s to \"%s\"",
                __func__, source_file, __LINE__,
                __func__, r, r==1?"":"s", buf
                );
        }
    } else {
        r++;
    }
    return r;
}

/* snf : like snprintf %f for double
   no formatting options (i.e. simpler)
   other than the ability to specify the numeric base
   (like strtoul), provision for a fixed prefix (e.g. "$"),
   and ability to pad the number of digits before the decimal point
   and the ability to specify the precision after the decimal point
   return number of characters written or (if buf is NULL or sz is too small)
   the required buffer size
   Typical uses:
      testing strtod
      fixed-point output, e.g. -$123.45
   Arguments:
      buf       where to put the result (can be NULL to get size needed)
      sz        how big is buf?
      prefix    optional string before number
      suffix    optional string after number
      d         the number to be interpolated
      pad       character to use to pad number to minimum length
                   typically ' ' or '0'
      min_dig   minimum length of result (# of digit characters left of
                   decimal point for number, including padding, but
                   excluding (non-digit) sign character for negative values)
      precision number of digits to the right of the decimal point
         if negative, use only as many digits as needed, maximum |precision|
   Return value:
      number of characters written or (if buf is NULL or sz is too small)
         the required buffer size (including space for terminating '\0'
         if buf is NULL or too small; not counted if buf is large enough).
*/
/* calls: initialize_snn, isprint, snmagnitude, snn_sf_double, sn_double */
/* called by: no other snn functions */
int snf(char *buf, int sz, const char *prefix, const char *suffix, double d, int pad,
    int min_dig, int precision,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    unsigned char digits[32];   /* RATS: ignore (snn_sf_digits uses sizeof(digits)) */
    int p, r, mag, offs, sf;
#ifndef PP__FUNCTION__
    static const char __func__[] = "snf";
#endif

    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(0x%lx, %d, %s%s%s, %s%s%s, %.24g, %s%c%s, %d, %d)",
            __func__, source_file, __LINE__,
            __func__, (unsigned long)(buf), sz,
            NULL==prefix?"":"\"", NULL==prefix?"NULL":prefix, NULL==prefix?"":"\"",
            NULL==suffix?"":"\"", NULL==suffix?"NULL":suffix, NULL==suffix?"":"\"",
            d,
            isprint(pad)?"'":"unprintable", isprint(pad)?(char)pad:' ', isprint(pad)?"'":"character",
            min_dig, precision
            );
    }
    /* argument sanity */
    if (0 > min_dig) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid min_dig %d",
                __func__, source_file, __LINE__,
                min_dig
                );
        }
        errno = EINVAL;
        return -1;
    }
    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    /* magnitude and significant figures of number, rounded */
    d = snn_sf_double(d, digits, sizeof(digits), snmagnitude(d, f, log_arg), snn_msf, precision, &sf, &mag, &p, f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, mag=%d, sf=%d, p=%d",
            __func__, source_file, __LINE__,
            d, mag, sf, p
            );
    }
    if ((0 >= mag) && (p < sf))
        offs = mag+1;
    else
        offs = mag;
    if (0 <= precision)
        p = precision;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, mag=%d, offs=%d, sf=%d, pad='%c', min_dig=%d, precision=%d",
            __func__, source_file, __LINE__,
            d, mag, offs, sf, pad, min_dig, p
            );
    }
    r = snn_double(buf, sz, prefix, suffix, digits, sizeof(digits), d, mag, offs, pad, min_dig, p, f, log_arg);
    return r;
}

/* sng : like snprintf %E but fewer options (simpler and faster), but more flexible
   exp_mod allows setting increment for exponent
      e.g. exp_mod = 3 will use ... -6, -3, 0, 3, 6, ...
   Typical uses:
      testing strtod, sscanf, etc.
      output in scientific or engineering notation, e.g. 3.45E9 Hz
   Arguments:
      buf         where to put the result (can be NULL to get size needed)
      sz          how big is buf?
      prefix      optional string before number
      suffix      optional string after number
      d           the number to be interpolated
      max_sig_fig maximum significant figures (digits only) to preserve
      exp_mod     modulus for exponent; e.g. exp_mod=3 gives exponents which
                     are a multiple of 3, i.e. ..., -12, -9, -6, -3, 0, 3, ...
                     This is an extension to what printf can do, and is
                     typically used in Engineering to correspond to SI unit
                     prefixes pico, nano, micro, milli, kilo, mega, giga, etc.
   Return value:
      number of characters written or (if buf is NULL or sz is too small)
         the required buffer size (including space for terminating '\0'
         if buf is NULL or too small; not counted if buf is large enough).
*/
/* calls: initialize_snn, snmagnitude, snn_sf_double, sn_double, snl */
/* called by: no other snn functions */
int sng(char *buf, int sz, const char *prefix, const char *suffix, double d,
    int max_sig_fig, int exp_mod,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char *p = buf;
    unsigned char digits[32];   /* RATS: ignore (snn_sf_digits uses sizeof(digits)) */
    int exponent, r = 0, sz2 = sz, mag, n, offs, q, rsf, sf;
#ifndef PP__FUNCTION__
    static const char __func__[] = "sng";
#endif

    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(0x%lx, %d, %s%s%s, %s%s%s, %.24g, %d, %d)",
            __func__, source_file, __LINE__,
            __func__, (unsigned long)(buf), sz,
            NULL==prefix?"":"\"", NULL==prefix?"NULL":prefix, NULL==prefix?"":"\"",
            NULL==suffix?"":"\"", NULL==suffix?"NULL":suffix, NULL==suffix?"":"\"",
            d, max_sig_fig, exp_mod
            );
    }
    /* rsf (requested significant figures) is absolute value of max_sig_fig */
    if (0 > max_sig_fig) rsf = 0 - max_sig_fig; else rsf = max_sig_fig;
    /* argument sanity */
    if ((1 > rsf) /* need at least one figure */
    || (1 > exp_mod) /* illegal exponent modulus */
    ) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid max_sig_fig %d or exp_mod %d",
                __func__, source_file, __LINE__,
                max_sig_fig, exp_mod
                );
        }
        errno = EINVAL;
        return -1;
    }
    if (rsf > snn_msf) /* requested more significant figures than run-time can provide */
        rsf = snn_msf;
    if (exp_mod > rsf) {
        /* no point in having exponent modulus greater than significant figures */
        exp_mod = 3 * ((rsf + 2) / 3);
        if (1 > exp_mod)
            exp_mod = 1;
        if (rsf < exp_mod)
            rsf = exp_mod;
    }
    if ((unsigned char)0U == snn_double_initialized)
        initialize_snn_double(f, log_arg);
    /* round, get digits, magnitude, significant figures */
    d = snn_sf_double(d, digits, sizeof(digits), snmagnitude(d, f, log_arg), rsf, SNN_EXTREME_PRECISION, &sf, &mag, NULL, f, log_arg);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, mag=%d, sf=%d, max_sig_fig=%d, exp_mod=%d",
            __func__, source_file, __LINE__,
            d, mag, sf, max_sig_fig, exp_mod
            );
    }
    if (sf > rsf) {
        /* limit significant figures to maximum specified */
        sf = rsf;
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: d=%.24g, mag=%d, sf=%d, max_sig_fig=%d, exp_mod=%d",
                __func__, source_file, __LINE__,
                d, mag, sf, max_sig_fig, exp_mod
                );
        }
    }
    /* exponent (for 1 mantissa digit left of decimal point */
    offs = 1;
    if (mag > 0) {
        exponent = mag - 1;
    } else {
        exponent = mag;
    }
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: exponent=%d",
            __func__, source_file, __LINE__,
            exponent
            );
    }
    n = 0; /* change to exponent (decrease) and offset (increase) */
    /* massage exponent, mantissa offset per exp_mod */
    if (0 < exponent) {
        if (0 != (q = (exponent % exp_mod))) {
            n = q;
        }
    } else if (0 > exponent) {
        if (0 != (q = ((0 - exponent) % exp_mod))) {
            n = exp_mod - q;
        }
    }
    if (0 != n) {
        exponent -= n;
        offs += n;
    }
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, sf=%d, mag=%d, n=%d, exponent=%d, offs=%d",
            __func__, source_file, __LINE__,
            d, sf, mag, n, exponent, offs
            );
    }
    /* how many significant fraction decimal places in mantissa? */
    if ((0 > max_sig_fig) && (sf < rsf)) /* exact number of figures requested */
        sf = rsf;
    q = sf - 1 - n;
    if (0 > q)
        q = 0;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, sf=%d, mag=%d, q=%d, n=%d, exponent=%d",
            __func__, source_file, __LINE__,
            d, sf, mag, q, n, exponent
            );
    }
    /* interpolate mantissa */
    n = snn_double(p, sz2, prefix, exponent==0? NULL : "E", digits, sizeof(digits), d, mag, offs, (int)'0', sf-q, q, f, log_arg);
    if (0 > n)
        return n;
    if ((NULL == buf) || (n > sz2))
        n--; /* undo count for terminating NUL */
    r += n;
    if ((NULL != buf) && (r < sz))
        p += n, sz2 -= n;
    /* interpolate exponent if necessary */
    if (0 != exponent) {
        n = snl(p, sz2, NULL, NULL, (long)exponent, 10, (int)'0', 1, f, log_arg);
        if (0 > n)
            return n;
        if ((NULL == buf) || (n > sz2))
            n--; /* undo count for terminating NUL */
        r += n;
        if ((NULL != buf) && (r < sz))
            p += n;
    }
    /* suffix */
    if (NULL != suffix) {
        for (; '\0'!=*suffix; r++, suffix++)
            if ((NULL != buf) && (r < sz))
                *p++ = *suffix;
    }
    if ((NULL != buf) && (r < sz))
        *p = '\0';
    else
        r++;
    return r;
}
