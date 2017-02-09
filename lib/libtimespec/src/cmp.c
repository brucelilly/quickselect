/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: source code for normalizing, adding, subtracting and comparing timespecs
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    cmp.c copyright 2010 - 2016 Bruce Lilly.   \ cmp.c $
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
/* $Id: ~|^` @(#)   This is cmp.c version 3.1 2016-05-31T16:37:39Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "timespec" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libtimespec/src/s.cmp.c */

/********************** Long description and rationale: ***********************
******************************************************************************/

/* source module identification */
/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: timespec.c ~|^` @(#)"
#define SOURCE_MODULE "cmp.c"
#define MODULE_VERSION "3.1"
#define MODULE_DATE "2016-05-31T16:37:39Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2010 - 2016"

/* configuration (which might affect feature test macros) */
/* to include a main entry point for testing, compile with -DTESTING=1 */
#ifndef TESTING
# define TESTING 0
#endif

/* feature test macros must appear before any header file inclusion */
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
#include "timespec.h"           /* includes time.h (time_t, gmtime_r, struct timespec) */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <stdlib.h>             /* NULL */

/* static data and function definitions */
static char timespec_initialized = (char)0;

static void initialize_timespec(void)
{
    timespec_initialized
        = register_build_strings(NULL, NULL, NULL);
}


/* public function definitions */
/* compare two timespec values */
int timespec_cmp(const void *v1, const void *v2)
{
    const struct timespec *t1 = (const struct timespec *)v1, *t2 = (const struct timespec *)v2;
    struct timespec n1, n2;

    if ((unsigned char)0U == timespec_initialized)
        initialize_timespec();
    n1.tv_sec = t1->tv_sec;
    n1.tv_nsec = t1->tv_nsec;
    normalize_timespec_nonnegative(&n1);
    n2.tv_sec = t2->tv_sec;
    n2.tv_nsec = t2->tv_nsec;
    normalize_timespec_nonnegative(&n2);
    if (n1.tv_sec < n2.tv_sec)
        return -1;
    if (n1.tv_sec > n2.tv_sec)
        return 1;
    if (n1.tv_nsec < n2.tv_nsec)
        return -1;
    if (n1.tv_nsec > n2.tv_nsec)
        return 1;
    return 0;
}
