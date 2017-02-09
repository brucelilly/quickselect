/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: snn_round - simple, fast interpolation of numbers to a string
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    snn_round.c copyright 2011 - 2016 Bruce Lilly.   \ snn_round.c $
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
/* $Id: ~|^` @(#)   This is snn_round.c version 2.5 2016-04-03T13:13:15Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libsnn/src/s.snn_round.c */

/********************** Long description and rationale: ***********************
* snround is like round(), but without dependencies on the math library and
*  its compilation option requirements:
*
*  double snround(double d);
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
#define ID_STRING_PREFIX "$Id: snn_round.c ~|^` @(#)"
#define SOURCE_MODULE "snn_round.c"
#define MODULE_VERSION "2.5"
#define MODULE_DATE "2016-04-03T13:13:15Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2016"

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
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
#include <ctype.h>              /* isalnum */
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char snn_round_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* initialize snn_almost_one, etc. at run-time */
static void initialize_snn_round(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    snn_round_initialized = register_build_strings(NULL, &source_file, s);
}

/* d is >= 0.0
   return largest integer (as double) <= d
*/
/* calls: snmagnitude */
/* called by: snround */
static double d_int_d(double d,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    double e, i = 0.0;
# ifndef PP__FUNCTION__
    static const char __func__[] = "d_int_d";
# endif

    if ((unsigned char)0U == snn_round_initialized)
        initialize_snn_round();
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
            while (e >= g) {
                i += g;
                e -= g;
            }
        }
    }
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(d=%.24g) returns %.24g",
            __func__, source_file, __LINE__,
            __func__, d, i
            );
    }
    return i;
}

/* like round() but:
   a) doesn't require magic incantations like #define _XOPEN_SOURCE 600
   b) doesn't depend on <math.h>
   c) doesn't require -lm
*/
/* calls: snn_initialize, d_int_d */
/* called by: snmultiple */
double snround(double d,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
# ifndef PP__FUNCTION__
    static const char __func__[] = "snround";
# endif
    if ((unsigned char)0U == snn_round_initialized)
        initialize_snn_round();
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g)\n",
            __func__, source_file, __LINE__,
            __func__, d
            );
    }
    if (-0.5 >= d) return 0.0 - d_int_d(0.5 - d, f, log_arg);
    return d_int_d(d+0.5, f, log_arg);
}
