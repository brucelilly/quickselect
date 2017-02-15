/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: leap_second.c - determine if a UTC struct tm corresponds to a leap second, count leap seconds between dates
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    leap_second.c copyright 2015-2017 Bruce Lilly.   \ leap_second.c $
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
/* $Id: ~|^` @(#)   This is leap_second.c version 2.12 2017-02-14T22:12:37Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "leap_second" */
/*****************************************************************************/
/* maintenance note: master file  /src/relaymail/lib/libleap_second/src/s.leap_second.c */

/********************** Long description and rationale: ***********************
* Leap seconds are introduced as needed to prevent noon from occurring in the
* middle of the night, while maintaining the SI second as a constant (in
* opposition to the method of varying the duration of a second as was done prior
* to the introduction of leap seconds).
* This file provides functions for handling leap seconds:
* int leap_second(const struct tm *ptm, void (*f)(int, void *, const char *, ...), void *log_arg)
* indicates whether or not a UTC [N.B.] date-time pointed to by ptm is a leap
* second.
* int leap_seconds_between(const struct tm *ptm1, const struct tm *ptm2, void (*f)(int, void *, const char *, ...), void *log_arg)
* returns a count of the number of seconds between two UTC [N.B.] date-times.
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
#define ID_STRING_PREFIX "$Id: leap_second.c ~|^` @(#)"
#define SOURCE_MODULE "leap_second.c"
#define MODULE_VERSION "2.12"
#define MODULE_DATE "2017-02-14T22:12:37Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2015-2017"

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

/* library header files needed */
#include "leap_second.h"        /* public definitions and declarations */
#include "snn.h"                /* snul */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

/* system header files needed for code not included with declaration header */
#include <ctype.h>              /* isalnum */
#ifndef EINVAL
# include <errno.h>             /* errno EINVAL */
#endif
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* local macros and structure definitions */
struct leap_second_struct {
    int year;        /* conventional year number */
    int month;       /* calendar month number: June=6, December=12 */
    int sign;        /* +1 for positive leap second, -1 for negative */
};

/* static data and function definitions */
static char leap_second_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
/* add new leap seconds to the start of the array */
/* year and month numbers used are conventional numbers (e.g. 6 means June) */
/* positive leap second (sign 1) inserted UTC 23:59:60 last day of specified month in specified year */
/* negative leap second (sign -1) removed UTC 23:59:59 last day of specified month in specified year */
static const struct leap_second_struct leap_second_struct_array[] = {
    { 2016, 12,  1 },
    { 2015,  6,  1 },
    { 2012,  6,  1 },
    { 2008, 12,  1 },
    { 2005, 12,  1 },
    { 1998, 12,  1 },
    { 1997,  6,  1 },
    { 1995, 12,  1 },
    { 1994,  6,  1 },
    { 1993,  6,  1 },
    { 1992,  6,  1 },
    { 1990, 12,  1 },
    { 1989, 12,  1 },
    { 1987, 12,  1 },
    { 1985,  6,  1 },
    { 1983,  6,  1 },
    { 1982,  6,  1 },
    { 1981,  6,  1 },
    { 1979, 12,  1 },
    { 1978, 12,  1 },
    { 1977, 12,  1 },
    { 1976, 12,  1 },
    { 1975, 12,  1 },
    { 1974, 12,  1 },
    { 1973, 12,  1 },
    { 1972, 12,  1 },
    { 1972,  6,  1 },
    { 0,     0,  0 } /* sentinel */
};

/* local array of (non-leap year) mdays; avoid library dependency cycle with utc_mktime */
static const int mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*INDENT ON*/
/* *INDENT-ON* */

static void initialize_leap_second(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    leap_second_initialized = register_build_strings(NULL, &source_file, s);
}

/* Find indices into leap_second_struct_array for leap second
   before and after UTC time represented by struct tm pointed to by ptm.
   If ptm represents a leap second, *pduring is set to the index of the
   corresponding entry in the leap_second_struct_array (else -1).
   If there is no leap second after the represented time, *pafter is set to -1.
   If there was no leap second before the represented time, *pbefore is set to
   the number of non-sentinel entries in the leap_second_struct_array.
   N.B. indices are such that earlier date-times have larger indices.
   Return value is zero except in the event of invalid arguments.
*/
static inline int leap_straddle(const struct tm *ptm, int *pbefore,
    int *pduring, int *pafter, void (*f)(int, void *, const char *, ...),
    void *log_arg)
{
    int i, before, during, after, year, mon, mday, tm_mon, tm_year;
#ifndef PP__FUNCTION__
    static const char __func__[] = "leap_straddle";
#endif

    if ((NULL == ptm)
    || (NULL == pbefore)
    || (NULL == pduring)
    || (NULL == pafter)
    ) {
        if ((unsigned char)0U == leap_second_initialized)
            initialize_leap_second();
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL pointer",
                __func__, source_file, __LINE__
                );
        }
        errno = EINVAL;
        return -1;
    }
    tm_year = ptm->tm_year + 1900;      /* use conventional year number */
    tm_mon = ptm->tm_mon;       /* 0-based */
    mday = mdays[tm_mon];       /* don't care about leap year unless/until leap
                                   seconds are introduced in February */
    tm_mon++;                   /* now conventional month number */
    after = before = during = -1;
    if (NULL != f)
        f(LOG_DEBUG, log_arg,
            "%s testing date %04d-%02d-%02dT%02d:%02d:%02dZ",
            __func__, tm_year, tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
            ptm->tm_sec);
    for (i=0; 0 != leap_second_struct_array[i].sign; i++) {
        year = leap_second_struct_array[i].year;
        if (tm_year > year) {
            before = i;
            if (NULL != f)
                f(LOG_DEBUG, log_arg,
                    "%s %d: %04d > %04d: before = %d", __func__, i, tm_year,
                    year, before);
            break;
        }
        else
        if (tm_year == year) {
            mon = leap_second_struct_array[i].month;
            if (tm_mon > mon) {
                before = i;
                if (NULL != f)
                    f(LOG_DEBUG, log_arg,
                        "%s %d: %04d == %04d && %02d > %02d: before = %d",
                        __func__, i, tm_year, year, tm_mon, mon, before);
                break;
            }
            else
            if (tm_mon == mon) {
                if ((ptm->tm_mday == mday)
                && (23 == ptm->tm_hour)
                && (59 == ptm->tm_min)
                && (59 <= ptm->tm_sec)
                ) {
                    int sign = leap_second_struct_array[i].sign;
                    if (((0 < sign) && (60 == ptm->tm_sec))
                    || ((0 > sign) && (59 == ptm->tm_sec))
                    ) {
                        during = i;  /* found an entry */
                        if (NULL != f)
                            f(LOG_DEBUG, log_arg,
                                "%s %d: %04d == %04d && %02d == %02d: during = %d",
                                __func__, i, tm_year, year, tm_mon, mon, during);
                    }
                }
                else {
                    after = i;
                    if (NULL != f)
                        f(LOG_DEBUG, log_arg,
                            "%s %d: %04d == %04d && %02d == %02d: after = %d",
                            __func__, i, tm_year, year, tm_mon, mon, after);
                }
            }
            else
            if (tm_mon < mon) {
                after = i;
                if (NULL != f)
                    f(LOG_DEBUG, log_arg,
                        "%s %d: %04d == %04d && %02d < %02d: after = %d",
                        __func__, i, tm_year, year, tm_mon, mon, after);
            }
        }
        else
        if (tm_year < year) {
            after = i;
            if (NULL != f)
                f(LOG_DEBUG, log_arg,
                    "%s %d: %04d < %04d: after = %d", __func__, i, tm_year,
                    year, after);
        }
    }
    if (-1 == before)
        before = i;
    *pafter = after;
    *pbefore = before;
    *pduring = during;
    return 0;
}

/* public function definitions */

/* return integer corresponding to UTC time pointed to by ptm:
   0 if it is not a leap second (or on error)
   >0 (1) if it is a positive leap second
   <0 (-1) if it is a negative leap second
*/
int leap_second(const struct tm *ptm,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int i, during, junk, mday;
#ifndef PP__FUNCTION__
    static const char __func__[] = "leap_second";
#endif

    if (NULL == ptm) {
        if ((unsigned char)0U == leap_second_initialized)
            initialize_leap_second();
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL source pointer",
                __func__, source_file, __LINE__
                );
        }
        errno = EINVAL;
        return 0;
    }
    if ((23 != ptm->tm_hour)
    || (59 != ptm->tm_min)
    )
        return 0;               /* leap second only at end of 23:59 */
    if (59 > ptm->tm_sec)
        return 0;               /* leap second only 23:59:59 or 23:59:60 */
    mday = mdays[ptm->tm_mon];  /* don't care about leap year unless/until leap
                                   seconds are introduced in February */
    if (mday != ptm->tm_mday)
        return 0;               /* can't be a leap second if not the last day of
                                   the month */
    i = leap_straddle(ptm, &junk, &during, &junk, f, log_arg);
    if ((0 == i)
    && (-1 != during)
    ) {
        return leap_second_struct_array[during].sign;
    }
    return 0;
}

/* Return number of leap seconds between two UTC date-times.
   Return value is the sum of the number of leap seconds between the
   two specified date-times (inclusive).
   Return value is -1 with errno set to EINVAL for invalid arguments.
   If there is ever a negative leap second, -1 will be a valid return
   value, so errno should be set to 0 before calling this function, and
   checked if the return value is -1.
*/
int leap_seconds_between(const struct tm *ptm1, const struct tm *ptm2,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int i, j, r, before, during, after, before2, during2, after2;
#ifndef PP__FUNCTION__
    static const char __func__[] = "leap_seconds_between";
#endif

    if ((NULL == ptm1) || (NULL == ptm2)) {
        if ((unsigned char)0U == leap_second_initialized)
            initialize_leap_second();
        if (NULL != f) {
            char buf[24];       /* RATS: ignore (always used with sizeof()) */
            char buf2[24];      /* RATS: ignore (always used with sizeof()) */

            (void)snul(buf, sizeof(buf), "0x", NULL, (unsigned long)ptm1, 16,
                (int)'0', 1, f, log_arg);
            (void)snul(buf2, sizeof(buf2), "0x", NULL, (unsigned long)ptm2, 16,
                (int)'0', 1, f, log_arg);
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL pointer in %s(%s, %s)",
                __func__, source_file, __LINE__,
                __func__, buf, buf2
                );
        }
        errno = EINVAL;
        return 0;
    }
    r = leap_straddle(ptm1, &before, &during, &after, f, log_arg);
    if (0 != r)
        return r;
    r = leap_straddle(ptm2, &before2, &during2, &after2, f, log_arg);
    if (0 != r)
        return r;
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %04d-%02d-%02dT%02d:%02d:%02dZ %d before, %d during, %d after",
            __func__, ptm1->tm_year + 1900, ptm1->tm_mon + 1, ptm1->tm_mday,
            ptm1->tm_hour, ptm1->tm_min, ptm1->tm_sec, before, during, after);
        f(LOG_DEBUG, log_arg,
            "%s: %04d-%02d-%02dT%02d:%02d:%02dZ %d before, %d during, %d after",
            __func__, ptm2->tm_year + 1900, ptm2->tm_mon + 1, ptm2->tm_mday,
            ptm2->tm_hour, ptm2->tm_min, ptm2->tm_sec, before2, during2,
            after2);
    }
    if ((-1 != during)
    && (-1 != during2)
    && (ptm1->tm_year == ptm2->tm_year)
    && (ptm1->tm_mon == ptm2->tm_mon)
    )
        return 0; /* same leap second date-time */
    j = before;
    if (j < before2)
        j = before2;
    i = after;
    if (i > after2)
        i = after2;
    ++i;
    if (NULL != f) f(LOG_DEBUG, log_arg, "%s: i = %d, j = %d", __func__, i, j);
    for (r=0; i<j; i++)
        r += leap_second_struct_array[i].sign;
    if (NULL != f) f(LOG_DEBUG, log_arg, "%s returns %d", __func__, r);
    return r;
}
