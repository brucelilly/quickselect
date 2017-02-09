/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for UTC struct tm to time_t conversion (mktime w/o zone offset) and related utility functions
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    utc_mktime.c copyright 2009 - 2017 Bruce Lilly.   \ utc_mktime.c $
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
/* $Id: ~|^` @(#)   This is utc_mktime.c version 2.7 dated 2017-01-16T00:18:05Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "utc_mktime" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libutc_mktime/src/s.utc_mktime.c */

/********************** Long description and rationale: ***********************
* POSIX mktime() produces a time_t from a struct tm containing local time,
* normalizing the struct tm elements in the process.
*
* It is sometimes desirable to generate a time_t from UTC time rather than
* local time, e.g. to avoid "daylight savings" and/or local time offset issues.
*
* utc_mktime() does this efficiently. It is frequently faster than system
* mktime. It is called analogously to system mktime():
*    time_t utc_mktime(struct tm *);
*
* Like POSIX mktime, utc_mktime normalizes the values in the struct tm pointed
*   to by the caller; but unlike POSIX mktime, utc_mktime does nothing with the
*   tm_isdst element and sets it to zero (because "dst" doesn't apply to UTC).
*   utc_mktime sets the tm_yday and tm_wday elements appropriately. utc_mktime
*   is oblivious to the underlying implementation limits on time_t; it will
*   allow the return value to wrap or overflow.
*
* Neither POSIX mktime nor utc_mktime take leap seconds into account in
*   generating the time_t result.  Because utc_mktime uses a UTC-representing
*   struct tm, its normalization of the struct tm will allow possible positive
*   leap second specifications (i.e. 23:59:60Z at the end of June or December).
*
* support functions publicly exposed include:
*    unsigned long leap_years_since_epoch(unsigned long, unsigned long);
*      return number of leap years since start of epoch year
*        i.e. start of epoch year through end of year-1 inclusive
*        year and epoch year are conventional years, NOT offset as in struct tm tm_year
*        1582 <= epoch < year <= ULONG_MAX - 399UL
*        e.g. leap_years_since_epoch(2011UL, 1970UL)
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
#define ID_STRING_PREFIX "$Id: utc_mktime.c ~|^` @(#)"
#define SOURCE_MODULE "utc_mktime.c"
#define MODULE_VERSION "2.7"
#define MODULE_DATE "2017-01-16T00:18:05Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2009 - 2017"

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

/* for hstrerror in netdb.h */
#undef _BSD_SOURCE
#define _BSD_SOURCE 1
#undef _SVID_SOURCE
#define _SVID_SOURCE 1
#undef _NETBSD_SOURCE
#define _NETBSD_SOURCE 1

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files */
#include "utc_mktime.h"         /* includes time.h (time_t, struct tm) */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#ifndef EINVAL
# include <errno.h>             /* errno EINVAL */
#endif
#ifndef NULL
# include <stddef.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char utc_mktime_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_utc_mktime(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    utc_mktime_initialized = register_build_strings(NULL, &source_file, s);
}

/* public function definitions */

/* UTC mktime
   normalize struct tm (representing UTC time)
   fill in correct values for yday and wday
   return time_t (seconds since Epoch)
*/
/* calls normalize_date_time, yday, wday, leap_years_since_epoch */
time_t utc_mktime(struct tm *ptm,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    time_t t;
#ifndef PP__FUNCTION__
    static const char __func__[] = "utc_mktime";
#endif

    if (NULL == ptm) {
        if (NULL != f) {
            if ((unsigned char)0U == utc_mktime_initialized)
                initialize_utc_mktime(); /* for source_file */
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL source pointer",
                __func__, source_file, __LINE__
                );
        }
        t = (time_t)(-1);
        errno = EINVAL;
    } else {
        int years, leaps;

        normalize_date_time(ptm, f, log_arg);   /* normalize year, month, mday, hour, minute, second */
        ptm->tm_yday = yday(ptm, f, log_arg);   /* requires normalized year, month, mday */
        ptm->tm_wday = wday(ptm, f, log_arg);   /* requires normalized year, correct yday */
        ptm->tm_isdst = 0;          /* no DST in UTC */
        /* seconds so far this year */
        t = (time_t)(ptm->tm_sec) /* seconds this minute */
            + (time_t)60 * (time_t)(ptm->tm_min) /* minutes this hour */
            + (time_t)3600 * (time_t)(ptm->tm_hour) /* hours today */
            + (time_t)86400 * (time_t)(ptm->tm_yday); /* days since the year began */
        /* seconds in previous years since the Epoch */
        years = ptm->tm_year + (1900 - 1970);
        leaps = leap_years_since_epoch(ptm->tm_year + 1900, 1970, f, log_arg);
        t += (time_t)years * (time_t)31536000 /* seconds in a non-leap year */
          + (time_t)leaps * (time_t)86400; /* seconds in a (leap) day */
        /* N.B. POSIX time_t ignores leap seconds! */
    }
    return t;
}
