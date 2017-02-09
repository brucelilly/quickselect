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
* $Id: ~|^` @(#)    normalize_date_time.c copyright 2009 - 2017 Bruce Lilly.   \ normalize_date_time.c $
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
/* $Id: ~|^` @(#)   This is normalize_date_time.c version 2.7 dated 2017-01-16T00:16:16Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "normalize_date_time" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libutc_mktime/src/s.normalize_date_time.c */

/********************** Long description and rationale: ***********************
* POSIX mktime() produces a time_t from a struct tm containing local time,
* normalizing the struct tm elements in the process.
*
*    void normalize_date_time(struct tm *);
*      only normalize elements of the struct tm representing UTC time
*      N.B. attempting to normalize local time struct tm with this function will
*         not correctly handle possible positive leap seconds.
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
#define ID_STRING_PREFIX "$Id: normalize_date_time.c ~|^` @(#)"
#define SOURCE_MODULE "normalize_date_time.c"
#define MODULE_VERSION "2.7"
#define MODULE_DATE "2017-01-16T00:16:16Z"
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

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files needed */
#include "utc_mktime.h"         /* includes time.h (time_t, struct tm) */
#include "leap_second.h"        /* leap_second */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#ifndef EINVAL
# include <errno.h>             /* errno EINVAL */
#endif
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char normalize_date_time_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_normalize_date_time(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    normalize_date_time_initialized =
        register_build_strings(NULL, &source_file, s);
}

/* public function definitions */

/* normalize struct tm year, month, mday, hour, minutes, seconds representing UTC [N.B.!] date-time in place
   does not update yday or wday
   does not use or modify other elements (e.g. tm_isdst)
   recognizes possible leap seconds (i.e. 23:59:60Z at the end of June or December)
*/
/* calls mdays, is_leap, leap_second */
void normalize_date_time(struct tm *ptm,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "normalize_date_time";
#endif

    if (ptm) {
        int leap, oyr, x, y, yr, z;
        struct tm tm;

        /* normalized values put in local tm structure, then copied back */
        /* oyr, x, y, yr, z local copy values for indirection-free speed */
        x = ptm->tm_sec;        /* local copy for speed */
        y = ptm->tm_min;        /* local copy for speed */
        /* normalize seconds */
        if (0 > x) {
            z = 60 - x;         /* positive value to avoid implementation discrepancies with modulus */	
            y -= z / 60;        /* adjust local copy of minutes */
            z %= 60;
            x = 60 - z;         /* modified seconds value */
        }
        /* normalization of negative value (e.g. -60) might have left x == 60 */
        if (59 < x) { y += x / 60; x %= 60; }
        tm.tm_sec = x;
        x = ptm->tm_hour;       /* local copy for speed */
        /* normalize minutes */
        if (0 > y) {
            z = 60 - y;         /* positive value to avoid implementation discrepancies with modulus */	
            x -= z / 60;
            z %= 60;
            y = 60 - z;
        }
        /* normalization of negative value might have left y == 60 */
        if (59 < y) { x += y / 60; y %= 60; }
        tm.tm_min = y;
        y = ptm->tm_mday;       /* local copy for speed */
        /* normalize hours */
        if (0 > x) {
            z = 24 - x;         /* positive value to avoid implementation discrepancies with modulus */	
            y -= z / 24;
            z %= 24;
            x = 24 - z;
        }
        /* normalization of negative value might have left x == 24 */
        if (23 < x) { y += x / 24; x %= 24; }
        tm.tm_hour = x;
        /* normalize year,month,day */
        oyr = yr = ptm->tm_year + 1900;
        /* is it a leap year? */
        leap = is_leap(yr, f, log_arg);
        x = ptm->tm_mon;        /* local copy for speed */
        while ((11 < x) || (0 > x) || (1 > y)
            || (y > mdays(x, leap, f, log_arg))) {
            /* first normalize month, adjust year */
            for (; 0 > x; x += 12) { yr--; }
            for (; 11 < x; x -= 12) { yr++; }
            if (yr != oyr) {
                leap = is_leap(yr, f, log_arg);
                oyr = yr;
            }
            /* month is normalized; normalize day-of-month (may change month) */
            if (1 > y) {
                y += mdays((x+11)%12, leap, f, log_arg);
                x--;
            } else if (y > mdays(x, leap, f, log_arg)) {
                y -= mdays(x, leap, f, log_arg);
                x++;
            }
            /* month may have become non-normalized during day adjustment; loop to check again */
        }
        tm.tm_year = yr - 1900; /* tm_year has an offset of 1900 years */
        tm.tm_mon = x;
        tm.tm_mday = y;
        /* special-case recognition of possible original positive leap-second specification */
        if ((60 == ptm->tm_sec) && (0 == tm.tm_sec) && (1 == tm.tm_mday)
        && (0 == tm.tm_hour) && (0 == tm.tm_min)
        && ((6 == tm.tm_mon) || (0 == tm.tm_mon))
        ) { /* 23:59:60Z at end of June or December became 00:00:00 July 1 or January 1 */
            /* was a possible positive leap second specification */
            /* tentative acceptance */
            tm.tm_sec = 60;
            tm.tm_min = 59;
            tm.tm_hour = 23;
            if (0 == tm.tm_mon) {
                tm.tm_mday = 31;
                tm.tm_mon = 11;
                tm.tm_year--;
            } else {
                tm.tm_mday = 30;
                tm.tm_mon--;
            }
            /* test for valid leap second */
            if (0 >= leap_second(&tm, f, log_arg)) { /* rejected */
                tm.tm_sec = 0;
                tm.tm_min = 0;
                tm.tm_hour = 0;
                tm.tm_mday = 1;
                if (11 == tm.tm_mon) {
                    tm.tm_mon = 0;
                    tm.tm_year++;
                } else {
                    tm.tm_mon++;
                }
            }
        }
        /* copy normalized values back to source structure */
        ptm->tm_year = tm.tm_year;
        ptm->tm_mon = tm.tm_mon;
        ptm->tm_mday = tm.tm_mday;
        ptm->tm_hour = tm.tm_hour;
        ptm->tm_min = tm.tm_min;
        ptm->tm_sec = tm.tm_sec;
        return;
    }
    if (NULL != f) {
        if ((unsigned char)0U == normalize_date_time_initialized)
            initialize_normalize_date_time(); /* for source_file */
        f(LOG_ERR, log_arg,
            "%s: %s line %d: NULL source pointer",
            __func__, source_file, __LINE__
         );
    }
    errno = EINVAL;
}
