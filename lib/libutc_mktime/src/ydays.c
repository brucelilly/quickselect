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
* $Id: ~|^` @(#)    ydays.c copyright 2009 - 2017 Bruce Lilly.   \ ydays.c $
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
/* $Id: ~|^` @(#)   This is ydays.c version 2.6 dated 2017-01-16T00:24:08Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "ydays" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libutc_mktime/src/s.ydays.c */

/********************** Long description and rationale: ***********************
* POSIX mktime() produces a time_t from a struct tm containing local time,
* normalizing the struct tm elements in the process.
*
*    int ydays(int month, int leap);
*      return the number of days in the year through the end of the preceding month
*        (struct tm.tm_mon zero-based numbering!)
*      leap is 0 for non-leap years, 1 for leap years
*      special (and trivial) case for month==12, return number of days in the year
*      return -1 with errno set to EINVAL unless 0 <= month <= 12
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
#define ID_STRING_PREFIX "$Id: ydays.c ~|^` @(#)"
#define SOURCE_MODULE "ydays.c"
#define MODULE_VERSION "2.6"
#define MODULE_DATE "2017-01-16T00:24:08Z"
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

/* local header files */
#include "utc_mktime.h"         /* includes time.h (time_t, struct tm) */
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
static char ydays_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* lookup table for efficient (cached) yday computation */
static int ydays_array[12] = {
/* days through end of previous month, non-leap years */
/* used by ydays */
/* Initialized to all zero values, lazily evaluated at run-time:
     these could simply be set initially, but incorrect values might result from
     careless editing, bitrot, gremlins, chercher la femme, je ne sais quoi, etc.
   Computing the values when required and caching the computed values is more
     efficient than computing a value each time it is needed, yet avoids
     discrepancies arising from the aforementioned mechanisms; it is a
     compromise between repeated recomputation of correct values and possibly
     corruptible unfamiliar hard-coded constants.
   Ydays values for leap years are the same as for non-leap years for January
     and February, and are one larger for later months -- the leap year values
     need not be (and are not) cached.
*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void initialize_ydays(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    ydays_initialized = register_build_strings(NULL, &source_file, s);
}

/* public function definitions */

/* return number of days in year at end of previous month (0-based tm_mon numbering), for leap/non-leap year */
/* special (and trivial) case for month==12, return number of days in the year */
/* return -1 for invalid argument */
/* use cached information if available */
/* calls self and mdays as required for non-cached information (updating cache) */
int ydays(int month, int leap,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int r;
#ifndef PP__FUNCTION__
    static const char __func__[] = "ydays";
#endif

#if 0 /* force (or not) bounds on leap (error return if not forced) */ 
    leap = (0 == leap)? 0: 1;
#endif
    if ((0 > month) || (12 < month) || (0 > leap) || (1 < leap)) {
        if (NULL != f) {
            if ((unsigned char)0U == ydays_initialized)
                initialize_ydays();
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid month %d or leap %d",
                __func__, source_file, __LINE__
                , month, leap
             );
        }
        errno = EINVAL;
        return -1;
    }
    switch (month) {
        case 0:                 /* January (trivial) */
        return month;
        case 12:                /* full year (trivial) */
            r = 365;            /* non-leap years; adjusted below if necessary */
        break;
        default:                /* check cache */
            r = ydays_array[month];  /* cached value */
            if (0 == r) {       /* cache needs to be updated */
                r = ydays(month - 1, 0, f, log_arg) + mdays(month - 1, 0, f, log_arg);  /* recurse */
                ydays_array[month] = r;      /* update cache */
            }
        break;
    }
    /* leap year adjustment */
    if ((0 != leap) && (1 < month))     /* March or later, leap years only */
        r++;                    /* add a day for February 29 */
    return r;
}
