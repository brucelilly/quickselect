/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring function defined in utc_mktime.c
*/
#ifndef	UTC_MKTIME_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    utc_mktime.h copyright 2009 - 2016 Bruce Lilly.   \ utc_mktime.h $
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
/* $Id: ~|^` @(#)   This is utc_mktime.h version 2.10 dated 2016-07-03T00:56:09Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "utc_mktime" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libutc_mktime/include/s.utc_mktime.h */

/* version-controlled header file version information */
#define UTC_MKTIME_H_VERSION "utc_mktime.h 2.10 2016-07-03T00:56:09Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <errno.h>              /* errno EINVAL */
#include <stddef.h>             /* NULL */
#include <time.h>               /* struct tm */

/* functions defined in utc_mktime.c */
#if defined(__cplusplus)
# define UTC_MKTIME_EXTERN extern "C"
#else
# define UTC_MKTIME_EXTERN extern
#endif

/* *INDENT-OFF* */
/*INDENT OFF*/

/* inverse_yday.c */
UTC_MKTIME_EXTERN void inverse_yday(struct tm *, void (*)(int, void *, const char *, ...), void *);

/* local_utc_offset.c */
UTC_MKTIME_EXTERN int local_utc_offset(int, void (*)(int, void *, const char *, ...), void *);

/* normalize_date_time.c */
UTC_MKTIME_EXTERN void normalize_date_time(struct tm *, void (*)(int, void *, const char *, ...), void *);

/* utc_mktime.c */
UTC_MKTIME_EXTERN time_t utc_mktime(struct tm *, void (*)(int, void *, const char *, ...), void *);

/* ydays.c */
UTC_MKTIME_EXTERN int ydays(int, int, void (*)(int, void *, const char *, ...), void *);

/* inline functions */

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
int is_leap(int year,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    int ret=0, y;

    if (0 > year) {
        y = 0 - year; /* avoid modulus implementation issues */
    } else {
        y = year;
    }
    if (y == ((y>>2)<<2)) {
        if ((0 != (y % 100))
        || (0 == (y % 400))
        ) {
            ret = 1;
        }
    }
    return ret;
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
int leap_years_since_epoch(int year, int epoch,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    int l = 0;

    /* number of leap years is the difference in
       the number of leap years (through previous years)
       in proleptic Gregorian calendar
    */
    if (year != epoch) {
        int t, u, x, z;

        t = (year-1) >> 2;
        u = t / 25; /* == (year-1) / 100 */
        x = (epoch-1) >> 2;
        z = x / 25; /* == (epoch-1) / 100 */
        l = t - u + (u >> 2) - x + z - (z >> 2);
    }
    return l;
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
int mdays(int month, int leap,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    static const int mdays_array[12][2] = {
        { 31, 31 },             /* January */
        { 28, 29 },             /* February */
        { 31, 31 },             /* March */
        { 30, 30 },             /* April */
        { 31, 31 },             /* May */
        { 30, 30 },             /* June */
        { 31, 31 },             /* July */
        { 31, 31 },             /* August */
        { 30, 30 },             /* September */
        { 31, 31 },             /* October */
        { 30, 30 },             /* November */
        { 31, 31 },             /* December */
    };


#if 1 /* force (or not) bounds on leap (error return if not forced) */ 
    leap = (0 == leap)? 0: 1;
#endif
    if ((0 > month) || (11 < month) || (0 > leap) || (1 < leap)) {
        errno = EINVAL;
        return -1;
    }
    return mdays_array[month][leap];
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
int wday(const struct tm *ptm,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    int ret = -1;

    if (NULL == ptm) {
        errno = EINVAL;
    } else {
        int d, m;

        d = ptm->tm_yday; /* cache ptm->tm_yday to avoid multiple pointer dereference */
        m = ptm->tm_mon; /* cache ptm->tm_mon to avoid multiple pointer dereference */
        if ((0 > d) || (365 < d)) {
            errno = EINVAL;
        } else if ((0 > m) || (11 < m)) {
            errno = EINVAL;
        } else {
            int y;

            y = ptm->tm_year + 1900;
            /* y = (year-1) [modulo 400]; always positive */
            --y;
            if (0 > y) {        /* make positive */
                m = (0 - y) / 400;
                y += (m + 1) * 400;
            }
            ret =
                (int)((d + 1 + (((y % 100) * 5) >> 2) +
                    ((y / 100) % 4) * 5) % 7);
            /* Magic numbers and formula explained:
               year-1 is the number of full years elapsed since the beginning of
                  a quatercentenary (tm_year has an offset of -1900).
               (year-1)%100 is the number of years in the current century.
                  This is multiplied by 365.25 to give the number of days, but
                  since the end result is taken modulo 7, modular arithmetic
                  allows reducing this to multiplication by 1.25 or 5/4.
                  Note that ((year-1)%400)%100 is the same as (year-1)%100.
               ((year-1)/100)%4 is the number of centuries in the current
                  quatercentenary Gregorian calendar cycle.
                  Multiplying by 5 (or 5 + N*7) gives an offset which
                  compensates for the fact that century years are not leap years
                  unless also quatercentenary years.
                  Note that (((year-1)%400)/100)%4 is the same thing.
               yday (d) is added for the elapsed days in the current year.
               A further offset of 1 is required, since a quatercentenary year
                  in the Gregorian calendar begins on a Saturday, and yday has
                  a one day offset (Jan 1 is yday of 0).
               The numbers 4, 5, and 100 are chosen so that a good optimizing
                  compiler can take advantage of the repetition.
               7 is the number of days in a week, of course.
            */
        }
    }
    return ret;
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
int yday(const struct tm *ptm,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{
    if (NULL != ptm) {
        int d, leap, m, y;

        y = ptm->tm_year + 1900;
        m = ptm->tm_mon;
        d = ptm->tm_mday;
        leap = is_leap(y, NULL, NULL);
        if ((0 <= m) && (11 >= m)) {    /* bounds check */
            if ((1 <= d) && (d <= mdays(m, leap, NULL, NULL))) {    /* bounds check */
                return ydays(m, leap, NULL, NULL) +  d - 1; /* days in earlier months plus days this month, range-adjusted */
            }
        }
    }
    errno = EINVAL;
    return -1;
}

#define	UTC_MKTIME_H_INCLUDED
#endif
