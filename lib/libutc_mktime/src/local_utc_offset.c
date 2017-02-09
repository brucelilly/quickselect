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
* $Id: ~|^` @(#)    local_utc_offset.c copyright 2009 - 2017 Bruce Lilly.   \ local_utc_offset.c $
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
/* $Id: ~|^` @(#)   This is local_utc_offset.c version 2.6 dated 2017-01-16T00:14:21Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "local_utc_offset" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libutc_mktime/src/s.local_utc_offset.c */

/********************** Long description and rationale: ***********************
*    int local_utc_offset(int scale);
*      return the present offset between local time (for the calling execution
*        environment) and UTC
*      return value is rounded and scaled according to the supplied scale factor
*        scale=1 gives local time offset in seconds East of the prime meridian
*        scale=-60 gives minutes West of the prime meridian
*        etc.
*        scale=0 is treated as scale=1 (seconds East)
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
#define ID_STRING_PREFIX "$Id: local_utc_offset.c ~|^` @(#)"
#define SOURCE_MODULE "local_utc_offset.c"
#define MODULE_VERSION "2.6"
#define MODULE_DATE "2017-01-16T00:14:21Z"
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
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char local_utc_offset_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_local_utc_offset(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    local_utc_offset_initialized = register_build_strings(NULL, &source_file, s);
}

/* public function definitions */

/* compute present local time zone offset from UTC
   scale result; scale=1 gives seconds East of UTC prime meridian
      scale = -1 gives seconds West of UTC prime meridian
      scale = 60 gives minutes East of prime meridian (rounded to nearest minute)
      etc.
      scale=0 is treated as scale=1
*/
/* calls utc_mktime */
int local_utc_offset(int oscale,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret, scale, sign;
    time_t diff, posix_local, posix_utc;
    struct tm tm;
#ifndef PP__FUNCTION__
    static const char __func__[] = "local_utc_offset";
#endif

    (void)time(&posix_local);
    tzset();
    (void)localtime_r(&posix_local, &tm);
    /* the following method of determining offset from UTC works;
          using timezone as set by localtime does not work
    */
    /* pretend specified time is UTC (no DST); what is POSIX time_t? */
    tm.tm_isdst = 0;
    posix_utc = utc_mktime(&tm, f, log_arg); 
    /* local time offset (at specified time) from UTC in seconds is given
          by the difference between the two POSIX time_t values
    */
    if (posix_local >= posix_utc) {
        diff = posix_local - posix_utc;
        sign = 1;
        ret = (int)diff;        /* absolute value of offset in seconds */
    } else {
        diff = posix_utc - posix_local;
        sign = -1;
        ret = 0 - (int)diff;    /* absolute value of offset in seconds */
    }
    scale = oscale;
    if (0 > scale) {
        scale = 0 - scale;
        sign = 0 - sign;
    } else if (0 == scale) {
        scale = 1;              /* avoid division by zero */
    }
    ret = (ret + scale / 2) / scale;    /* round and scale */
    ret *= sign;                /* adjust sign */
    if (NULL != f) {
        if ((unsigned char)0U == local_utc_offset_initialized)
            initialize_local_utc_offset(); /* for source_file */
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%d) returns %d",
            __func__, source_file, __LINE__
            , __func__, oscale, ret
         );
    }
    return ret;
}
