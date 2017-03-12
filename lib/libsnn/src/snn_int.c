/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: snn_int - integer interpolation funtions of snn
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    snn_int.c copyright 2011-2017 Bruce Lilly.   \ snn_int.c $
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
/* $Id: ~|^` @(#)   This is snn_int.c version 2.11 2017-03-08T20:38:47Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libsnn/src/s.snn_int.c */

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
* Integral type number formatting functions allow the programmer to specify the
*  number base from 2 through 36, compatible with strtol() and strtoul(); a
*  minimum length can be specified (for right-alignment of numbers) with a
*  user-specified pad character.  Signed integer output places the sign
*  character between whitespace padding (but not other padding, e.g. '0') and
*  the optional prefix, also for compatibility with strtol(). The integral type
*  functions are snul() for unsigned long integers and snl for signed long
*  integers:
*
*  int snul(char *buf, int sz, const char *prefix, const char *suffix,
*    unsigned long int ul, int base, int pad, int min_dig);
*
*  int snl(char *buf, int sz, const char *prefix, const char *suffix, long int l,
*    int base, int pad, int min_dig);
*
* Additional utility functions are provided:
*
* snlround is like lround(), but without dependencies on the math library and
*  its compilation option requirements:
*
*  long int snlround(double d);
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
#define ID_STRING_PREFIX "$Id: snn_int.c ~|^` @(#)"
#define SOURCE_MODULE "snn_int.c"
#define MODULE_VERSION "2.11"
#define MODULE_DATE "2017-03-08T20:38:47Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011-2017"

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
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
#include "snn.h"                /* header file for public definitions and declarations */
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
static char snn_int_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
/* powers of ten */

#define SNN_BASE_OFFSET SNN_MIN_BASE
#define SNN_MIN_POWER 0
#define SNN_MAX_POWER (LONG_BIT)
/* lazy evaluation to deal with ULONG_MAX on various machines */
static unsigned long int snn_base_pow[SNN_MAX_BASE+1-SNN_BASE_OFFSET][SNN_MAX_POWER+1-SNN_MIN_POWER];
static unsigned int snn_max_pow[SNN_MAX_BASE+1-SNN_BASE_OFFSET];

/* initialize snn_almost_one, etc. at run-time */
static void initialize_snn_int(void)
{
    unsigned long int base, pow, ul;
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    snn_int_initialized = register_build_strings(NULL, &source_file, s);
    for (base=SNN_MIN_BASE; SNN_MAX_BASE >= base; base++) {
        snn_base_pow[base-SNN_BASE_OFFSET][0] = 1UL;
        ul = snn_base_pow[base-SNN_BASE_OFFSET][1] = base;
        for (pow=2UL; SNN_MAX_POWER >= pow; pow++) {
            if ((ULONG_MAX) / base <= ul) {
                snn_base_pow[base-SNN_BASE_OFFSET][pow] = ULONG_MAX;
            } else {
                ul = snn_base_pow[base-SNN_BASE_OFFSET][pow] = ul * base;
                snn_max_pow[base-SNN_BASE_OFFSET] = pow;
            }
        }
    }
}

/* return of base raised to power pow */
/* N.B. initialize_snn must be called before first use */
/* called by: int_d, snn_uint */
static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
unsigned long int snn_base_power(unsigned int base, unsigned int pow,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    return snn_base_pow[base-SNN_BASE_OFFSET][pow];
}

/* maximum power to which a base can be raised with result <= ULONG_MAX */
/* N.B. initialize_snn must be called before first use */
/* called by: snn_uint */
static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
unsigned int snn_max_power(unsigned int base,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    return snn_max_pow[base-SNN_BASE_OFFSET];
}

/* d is >= 0.0
   return largest integer <= d
*/
/* calls: snmagnitude, snn_base_power */
/* called by: snlround */
static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
long int int_d(double d,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    double e;
    long int m=0L;
#ifndef PP__FUNCTION__
    static const char __func__[] = "int_d";
#endif

    if (0.0 > d) e = 0.0 - d;
    else e = d;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: d=%.24g, e=%.24g",
            __func__, source_file, __LINE__,
            d, e
            );
    }
    if (1.0 <= e) {
        int mag;
        double g;
        unsigned long int ul=0UL, ul2;

        mag = snmagnitude(e, f, log_arg);
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: e=%.24g, mag=%d",
                __func__, source_file, __LINE__,
                e, mag
                );
        }
        while (0 < mag) {
            --mag;
            g = sn1en(mag, f, log_arg);
            ul2 = snn_base_power(10U, (unsigned)mag, f, log_arg);
            while (e >= g) {
                ul += ul2;
                e -= g;
            }
        }
        m = ul;
    }
    return m;
}

/* like lround() but:
   a) doesn't require magic incantations like #define _XOPEN_SOURCE 600
   b) doesn't depend on <math.h>
   c) doesn't require -lm
*/
/* calls: initialize_snn, int_d */
/* called by: no other snn functions */
/* no inline: Sun cc barfs */
long int snlround(double d,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "snlround";
#endif

    if ((char)0 == snn_int_initialized)
        initialize_snn_int();
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g)",
            __func__, source_file, __LINE__,
            __func__, d
            );
    }
    if (-0.5 >= d) return 0L - int_d(0.5 - d, f, log_arg);
    return int_d(d+0.5, f, log_arg);
}

/* internal function that interpolates integers
      sign < 0 puts '-' between whitespace padding (if any) and prefix
      sign >= 0 does not
*/
/* calls: snn_max_power, snn_base_power, isspace */
/* called by: snul,  snl */
static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int snn_uint(char *buf, int sz,
    const char *prefix, const char *suffix,
    unsigned long int ul, int base, int pad, int sign, int min_dig,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char *p = buf;
    int r = 0, k, n, spp;
    unsigned int l, m, u;
    unsigned long int d, x;
#ifndef PP__FUNCTION__
    static const char __func__[] = "snn_uint";
#endif

    x = 0UL;
    /* how many digits are required for number (ignoring sign and padding)?
       at least one digit is always required (i.e. for 0)
       1 digit is required if ul < snn_base_power(base,1U)
       if more than one digit is required, n digits are needed if snn_base_power(base,n-1) <= ul < snn_base_power(base,n)
    */
    if (ul >= base) {
        for (l=2U,u=snn_max_power((unsigned)base, f, log_arg),m=(l+u)/2U; l<=u; m=(l+u)/2U) {
            x = snn_base_power((unsigned)base, m, f, log_arg);
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: ul=%lu, x=%lu, l=%u, m=%u, u=%u",
                    __func__, source_file, __LINE__,
                    ul, x, l, m, u
                    );
            }
            if (x > ul)
                u = m - 1U;
            else
                l = m + 1U;
        }
        n = (int)l;
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: ul=%lu, x=%lu, l=%u, m=%u, u=%u",
                __func__, source_file, __LINE__,
                ul, x, l, m, u
                );
        }
    } else
        n=1;
    /* type (whitespace or not) of padding */
    spp = isspace(pad);
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: ul=%lu, n=%d, spp=%d, sign=%d",
            __func__, source_file, __LINE__,
            ul, n, spp, sign
            );
    }
    /* whitespace padding always goes first for strtoul, strtol compatibility */
    if (0 != spp) {
         for (k=n; k < min_dig; r++,k++) { /* padding */
             if ((NULL != buf) && (r < sz))
                 *p++ = (char)pad;
         }
    }
    /* sign (if required) before prefix for strtoul, strtol compatibility */
    if (0 > sign) {
        if ((NULL != buf) && (r < sz))
            *p++ = '-';
        r++;
    }
    /* prefix between sign (if required) and digits */
    if (NULL != prefix) {
        for (; '\0' != *prefix; r++, prefix++)
            if ((NULL != buf) && (r < sz))
                *p++ = *prefix;
    }
    /* non-whitespace padding (presumably a digit, probably '0') if required */
    if (0 == spp) {
         for (k=n; k < min_dig; r++,k++) { /* padding */
             if ((NULL != buf) && (r < sz))
                 *p++ = (char)pad;
         }
    }
    /* interpolate number's digits */
    while (1 < n) {
        n--;
        x = snn_base_power((unsigned)base, (unsigned)n, f, log_arg);
        for (d=0; ul>=x; ul-=x)
            d++;
        if ((NULL != buf) && (r < sz))
            *p++ = snn_digits[d];
        r++;
    }
    /* last (units) digit is trivial */
    if ((NULL != buf) && (r < sz))
        *p++ = snn_digits[ul];
    r++;
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

/* snul : like snprintf for unsigned long, but
   no formatting options (i.e. faster)
   other than the ability to specify the numeric base
   (like strtoul), provision for a fixed prefix (e.g. "0x"),
   and ability to pad the number of digits
   Typical uses:
      testing strtoul
      writing C constants, e.g. 0x123abcUL
   Arguments:
      buf    where to put the result (can be NULL to get size needed)
      sz      how big is buf?
      prefix  optional string before number
      suffix  optional string after number
      ul      the number to be interpolated
      base    number base for output; 2 <= base <= 36
      pad     character to use to pad number to minimum length
                 typically ' ' or '0'
      min_dig minimum length of result (# of characters for number)
   Return value:
      number of characters written or (if buf is NULL or sz is too small)
         the required buffer size (including space for terminating '\0'
         if buf is NULL or too small; not counted if buf is large enough).
      E.g.:
         char *buf = NULL;
         int n, sz = 0;
         n = snul(buf, sz, "0x", "UL", 0x123abcUL, 16, "0', 0);
         buf = malloc(sz=n);
         n = snul(buf, sz, "0x", "UL", 0x123abcUL, 16, '0', 0);
*/
/* calls: initialize_snn, isprint, snn_uint */
/* called by: no other snn functions*/
int snul(char *buf, int sz, const char *prefix, const char *suffix,
    unsigned long int ul, int base, int pad, int min_dig,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "snul";
#endif

    if ((char)0 == snn_int_initialized)
        initialize_snn_int();
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(0x%lx, %d, %s%s%s, %s%s%s, %lu, %d, %s%c%s, %d)",
            __func__, source_file, __LINE__,
            __func__, (unsigned long)(buf), sz,
            NULL==prefix?"":"\"", NULL==prefix?"NULL":prefix, NULL==prefix?"":"\"",
            NULL==suffix?"":"\"", NULL==suffix?"NULL":suffix, NULL==suffix?"":"\"",
            ul, base,
            isprint(pad)?"'":"unprintable", isprint(pad)?(char)pad:' ', isprint(pad)?"'":"character",
            min_dig
            );
    }
    if ((SNN_MIN_BASE > base) || (SNN_MAX_BASE < base)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid base %d",
                __func__, source_file, __LINE__,
                base
                );
        }
        errno = EINVAL;
        return -1;
    }
    return snn_uint(buf, sz, prefix, suffix, ul, base, pad, 0, min_dig, f, log_arg);
}

/* snl : like snprintf for long integers
   like snul for unsigned long (q.v.), which this function calls
   negative sign always precedes prefix (e.g. -0x123abc)
   whitespace padding always precedes sign
   Typical uses:
      testing strtol
      writing C constants, e.g. -1234
   Arguments:
      buf     where to put the result (can be NULL to get size needed)
      sz      how big is buf?
      prefix  optional string before number
      suffix  optional string after number
      l       the number to be interpolated
      base    number base for output; 2 <= base <= 36
      pad     character to use to pad number to minimum length
                 typically ' ' or '0'
      min_dig minimum length of result (# of characters for number,
                 including sign and padding)
   Return value:
      number of characters written or (if buf is NULL or sz is too small)
         the required buffer size (including space for terminating '\0'
         if buf is NULL or too small; not counted if buf is large enough).
      E.g.:
         char *buf = NULL;
         int n, sz = 0;
         n = snl(buf, sz, "0", "L", 01234567L, 8, "0', 0);
         buf = malloc(sz=n);
         n = snl(buf, sz, "0", "L", 01234567L, 8, '0', 0);
*/
/* calls: initialize_snn, isprint, snn_uint */
/* called by: sng */
int snl(char *buf, int sz, const char *prefix, const char *suffix, long int l,
    int base, int pad, int min_dig,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int sign=0;
    unsigned long int ul;
#ifndef PP__FUNCTION__
    static const char __func__[] = "snl";
#endif

    if ((char)0 == snn_int_initialized)
        initialize_snn_int();
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(0x%lx, %d, %s%s%s, %s%s%s, %ld, %d, %s%c%s, %d)",
            __func__, source_file, __LINE__,
            __func__, (unsigned long)(buf), sz,
            NULL==prefix?"":"\"", NULL==prefix?"NULL":prefix, NULL==prefix?"":"\"",
            NULL==suffix?"":"\"", NULL==suffix?"NULL":suffix, NULL==suffix?"":"\"",
            l, base,
            isprint(pad)?"'":"unprintable", isprint(pad)?(char)pad:' ', isprint(pad)?"'":"character",
            min_dig
            );
    }
    if ((SNN_MIN_BASE > base) || (SNN_MAX_BASE < base)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid base %d",
                __func__, source_file, __LINE__,
                base
                );
        }
        errno = EINVAL;
        return -1;
    }
    /* handle negative values */
    if (0L > l) {
        long int l2;

        sign = -1;
        l2 = 0L - l;
        ul = (unsigned long)l2;
    } else {
        ul = (unsigned long)l;
    }
    return snn_uint(buf, sz, prefix, suffix, ul, base, pad, sign, min_dig, f, log_arg);
}
