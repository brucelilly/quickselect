/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: snn - simple, fast interpolation of numbers to a string
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    snn.c copyright 2011 - 2017 Bruce Lilly.   \ snn.c $
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
/* $Id: ~|^` @(#)   This is snn.c version 2.7 dated 2017-01-16T00:32:49Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libsnn/src/s.snn.c */

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
*
* snmagnitude returns an indication of the magnitude of a specified floating
*  point number by indicating the position of the first significant digit in
*  the decimal representation of the number.  If the left-most most significant
*  digit is the first position to the left of the radix point (e.g. 5.1234),
*  the return value is 1.  A negative return value indicates that the left-most
*  most significant digit is to the right of the radix point, e.g. -2 is
*  returned for 0.0123.  Zero is returned if the argument is zero.
*
*  int snmagnitude(double d);
*
* snround is like round(), but without dependencies on the math library and
*  its compilation option requirements:
*
*  double snround(double d);
*
* snlround is like lround(), but without dependencies on the math library and
*  its compilation option requirements:
*
*  long int snlround(double d);
*
* snmultiple returns the nearest multiple of a specified increment to a
*  specified floating-point number.  If the increment is 1.0, this is equivalent
*  to snround.  However, it can also be used to provide the nearest multiple
*  of 10.0, of 0.1, of 25.4, etc.:
*
*  double snmultiple(double d, double incr)
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
#define ID_STRING_PREFIX "$Id: snn.c ~|^` @(#)"
#define SOURCE_MODULE "snn.c"
#define MODULE_VERSION "2.7"
#define MODULE_DATE "2017-01-16T00:32:49Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2017"

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
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
#include <ctype.h>              /* isalnum */
#include <limits.h>             /* *_MAX LONG_BIT */
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char snn_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* initialize snn_almost_one, etc. at run-time */
static void initialize_snn(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    snn_initialized = register_build_strings(NULL, &source_file, s);
}

/*
   Return an integer indicating the position of the first digit in the floating-point decimal representation of d.
   If d == 0.0, return 0.
   If 1.0 <= |d| < 10.0, return 1 (1 digit left of radix point).
   If 10.0 <= |d| < 100.0, return 2 (2 digits left of radix point), etc.
   If 0.1 <= |d| < 1.0, return -1 (first non-zero digit is one position right of radix point).
   If 0.01 <= |d| < 0.1, return -2 (2nd position right of radix point), etc.
*/
/* calls: initialize_snn */
/* called by: int_d, d_int_d, snsf, snf, sng */
int snmagnitude(double d,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret = 0;
#ifndef PP__FUNCTION__
    static const char __func__[] = "snmagnitude";
#endif

    if ((unsigned char)0U == snn_initialized)
        initialize_snn();
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g)",
            __func__, source_file, __LINE__,
            __func__, d
            );
    }
    if (0.0 != d) {
        int l, m, u;

        if (0.0 > d) d = 0.0 - d;
        l = SNN_MIN_POW10;
        u = SNN_MAX_POW10;
        /* binary search for magnitude */
        for (m=(l+u)/2; l<=u; m=(l+u)/2) {
            if ((SNN_MAX_POW10 < m) || (SNN_MIN_POW10 > m))
                break;
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: l=%d, m=%d, u=%d",
                    __func__, source_file, __LINE__,
                    l, m, u
                    );
            }
            if (d < sn1en(m, f, log_arg))
                u = m - 1;
            else
                l = m + 1;
        }
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: l=%d, m=%d, u=%d",
                __func__, source_file, __LINE__,
                l, m, u
                );
        }
        if (1.0 <= d) m++; else m--;
        ret = m;
    }
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g) returns %d",
            __func__, source_file, __LINE__,
            __func__, d, ret
            );
    }
    return ret;
}
