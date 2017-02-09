/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for inverse_yday
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    inverse_yday.c copyright 2009 - 2017 Bruce Lilly.   \ inverse_yday.c $
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
/* $Id: ~|^` @(#)   This is inverse_yday.c version 2.6 dated 2017-01-16T00:08:33Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "inverse_yday" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libutc_mktime/src/s.inverse_yday.c */

/********************** Long description and rationale: ***********************
* POSIX mktime() produces a time_t from a struct tm containing local time,
* normalizing the struct tm elements in the process.
*
*    void inverse_yday(struct tm *);
*      given normalized year and yday, fill in correct values for month and mday
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
#define ID_STRING_PREFIX "$Id: inverse_yday.c ~|^` @(#)"
#define SOURCE_MODULE "inverse_yday.c"
#define MODULE_VERSION "2.6"
#define MODULE_DATE "2017-01-16T00:08:33Z"
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
static char inverse_yday_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_inverse_yday(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    inverse_yday_initialized = register_build_strings(NULL, &source_file, s);
}

/* public function definitions */

/* inverse of yday:
   given normalized year and yday, fill in correct values for month and mday
*/
/* calls mdays, ydays, is_leap */
void inverse_yday(struct tm *ptm,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "inverse_yday";
#endif

    if (NULL != f) {
        if ((unsigned char)0U == inverse_yday_initialized)
            initialize_inverse_yday(); /* for source_file */
    }
    if (NULL != ptm) {
        int d;

        d = ptm->tm_yday;   /* local copy for speed */
        if ((0 <= d) && (365 >= d)) {
            int l, leap, m, x, u, z;

            leap = is_leap(ptm->tm_year + 1900, f, log_arg);
            /* binary search for month with leap day compensation */
            for (l=0,u=11,m=5; l<=u; m=(l+u)/2) {
                x = ydays(m, leap, f, log_arg); /* x is tm_yday at first of tm_mon m (Calendar month m+1) */
                /* check upper bound */
                if (d < x) {    /* true m must be below test m */
                    u = m - 1;
                    continue;
                }
                z = x + mdays(m, leap, f, log_arg) - 1; /* z is tm_yday at last day of tm_mon m */
                /* check lower bound */
                if (d > z) { /* true m must be above test m */
                    l = m + 1;
                    continue;
                }
                /* d lies between x and z; m is tm_mon */
                ptm->tm_mon = m;
                /* compute and store mday and return */
                ptm->tm_mday = d + 1 - x;
                return;
            }
        } else {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: invalid yday %d",
                    __func__, source_file, __LINE__
                    , d
                 );
            }
        }
    } else {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL source pointer",
                __func__, source_file, __LINE__
             );
        }
    }
    errno = EINVAL;
}
