/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: indent.c - determine indentation level from leading whitespace characters
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    indent.c copyright 2011 - 2016 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is indent.c version 1.8 2016-05-31T14:21:46Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "indent" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/src/s.indent.c */

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
#define ID_STRING_PREFIX "$Id: indent.c ~|^` @(#)"
#define SOURCE_MODULE "indent.c"
#define MODULE_VERSION "1.8"
#define MODULE_DATE "2016-05-31T14:21:46Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2016"

/* configuration (which might affect feature test macros) */
/* to include a main entry point for testing, compile with -DTESTING=1 */
#ifndef TESTING
# define TESTING 0
#endif

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

#include "indent.h"             /* public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum isspace */
#include <errno.h>              /* errno EINVAL */
#include <stdlib.h>             /* NULL */

/* static data and function definitions */
static unsigned char indent_initialized = (unsigned char)0U;
static char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_indent(void)
{
    indent_initialized
        = register_build_strings(NULL, &source_file, filenamebuf);
}

/* determine indentation level based on leading whitespace in string s
      comparing up to nc initial characters
   Python-like, UNIX-consistent rules: tab stops every 8 spaces
*/
int indent_level(const char *s, int nc)
{
    if ((NULL == s) || (0 > nc)) {
        errno = EINVAL;
        return -1;
    } else {
        int c, n = 0;

        if ((unsigned char)0U == indent_initialized)
            initialize_indent();
        for (c=0; (c < nc) && (0 != isspace((int)*s)); s++,c++) {
            switch (*s) {
                case ' ':
                    n++;
                break;
                case '\t': /* tab advances to next multiple of eight */
                    n = ((n+8)>>3)<<3;
                break;
                default:
                    n = 0;
                break;
            }
        }
        return n;
    }
}
