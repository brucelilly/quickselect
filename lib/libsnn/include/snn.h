/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for simple/fast number interpolation to a string
*/
#ifndef	SNN_H_INCLUDED
/******************************************************************************
* This software is covered by the zliblibpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    snn.h copyright 2011 - 2017 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is snn.h version 2.10 2017-02-10T19:00:44Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libsnn/include/s.snn.h */

/* version-controlled header file version information */
#define SNN_H_VERSION "snn.h 2.10 2017-02-10T19:00:44Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <stddef.h>             /* NULL */

/* macros */
#define SNN_DEBUG 1
/*   range of integer bases */
#define SNN_MIN_BASE 2
#define SNN_MAX_BASE 36
/*   range of powers of ten */
#define SNN_MIN_POW10 -323
#define SNN_MAX_POW10 308

#define SNN_EXTREME_PRECISION 512

#if defined(__cplusplus)
# define SNN_EXTERN extern "C"
#else
# define SNN_EXTERN extern
#endif

/* *INDENT-OFF* */
/*INDENT OFF*/

/* sn1en.c */
SNN_EXTERN double sn1en(int, void (*)(int, void *, const char *, ...), void *);

/* snn_double.c */
SNN_EXTERN int snf(char *, int, const char *, const char *, double, int, int, int, void (*)(int, void *, const char *, ...), void *);
SNN_EXTERN int sng(char *, int, const char *, const char *, double, int, int, void (*)(int, void *, const char *, ...), void *);
SNN_EXTERN int snsf(double, void (*)(int, void *, const char *, ...), void *);

/* snn_int.c */
SNN_EXTERN int snl(char *, int, const char *, const char *, long, int, int, int, void (*)(int, void *, const char *, ...), void *);
SNN_EXTERN int snul(char *, int, const char *, const char *, unsigned long, int, int, int, void (*)(int, void *, const char *, ...), void *);
SNN_EXTERN long int snlround(double, void (*)(int, void *, const char *, ...), void *);

/* snn.c function declarations */
SNN_EXTERN int snmagnitude(double, void (*)(int, void *, const char *, ...), void *);

/* inline functions */

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
double snround(double d,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    double c, e, i = 0.0;

    if (-0.5 >= d) c = 0.5 - d;
    else c = d + 0.5;
    if (0.0 > c) e = 0.0 - c;
    else e = c;
    if (1.0 <= e) {
        int mag;
        double g;

        mag = snmagnitude(e, NULL, NULL);
        while (0 < mag) {
            --mag;
            g = sn1en(mag, NULL, NULL);
            while (e >= g) {
                i += g;
                e -= g;
            }
        }
    }
    if (-0.5 >= d) return 0.0 - i;
    return i;
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
double snmultiple(double f, double incr,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    double d, i;

    if (0.0 > incr) incr = 0.0 - incr;
    if (0.0 == incr) return f;
    d = f / incr;
    i = snround(d,NULL,NULL);
    i *= incr;
    if (-0.5 >= d) return 0.0 - i;
    return i;
}

#define	SNN_H_INCLUDED
#endif
