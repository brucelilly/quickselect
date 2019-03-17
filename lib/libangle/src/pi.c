/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code to provide double precision floating point constant pi
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    pi.c copyright 2009-2017 Bruce Lilly.   \ pi.c $
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
/* $Id: ~|^` @(#)   This is pi.c version 3.1422 2017-09-01T02:03:58Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "pi" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libangle/src/s.pi.c */

/********************** Long description and rationale: ***********************
*
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
#define ID_STRING_PREFIX "$Id: pi.c ~|^` @(#)"
#define SOURCE_MODULE "pi.c"
#define MODULE_VERSION "3.1422"
#define MODULE_DATE "2017-09-01T02:03:58Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2009-2017"

/* to include a main entry point for testing, compile with -DTESTING=1 */
#ifndef TESTING
# define TESTING 0
#endif

/* feature test macros defined before any header files are included */
/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) && defined(MIN_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && defined(MAX_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files */
#include "pi.h"      /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#if TESTING
# include <stdio.h>             /* printf */
#endif
#ifndef NULL
# include <stddef.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <math.h>               /* atan */

/* exported (global) data */
volatile double pi = 3.141592653589793238462643;      /* reasonable approximation if not initialized */

/* static data and function definitions */
static char pi_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

void initialize_pi(void)
{
    if ((unsigned char)0U == pi_initialized) {
        const char *s;

        pi = 4.0 * atan(1.0);
        s = strrchr(filenamebuf, '/');
        if (NULL == s)
            s = filenamebuf;
        pi_initialized
            = register_build_strings(NULL, &source_file, s);
    }
}

#if TESTING
int main(int argc, char **argv)
{
    double before, after;

    before = pi;
    initialize_pi();
    after = pi;
    return
        printf(
            "%s: %s line %d:\npi before initialization: %.21f\nafter: %.21f\n",
            __func__, source_file, __LINE__, before, after);
}
#endif
