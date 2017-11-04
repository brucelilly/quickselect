/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for handling file and directory paths
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    is_dir.c copyright 2010-2017 Bruce Lilly.   \ is_dir.c $
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
/* $Id: ~|^` @(#)   This is is_dir.c version 1.7 dated 2017-09-01T02:22:30Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/src/s.is_dir.c */

/********************** Long description and rationale: ***********************
* int is_dir(const char *dir, const char *prog, const char *ver, int log_mask,
    void *log_arg, void (*f)(void *, const char *, ...))
* test if dir is a directory
*   return > 0 if it is
*   return 0 if it is not a directory
*   use prog, ver, log_mask, log_arg, f to log information
*     pointers may be NULL is logging is not required or desired
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
#define ID_STRING_PREFIX "$Id: is_dir.c ~|^` @(#)"
#define SOURCE_MODULE "is_dir.c"
#define MODULE_VERSION "1.7"
#define MODULE_DATE "2017-09-01T02:22:30Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2010-2017"

/* configuration (which might affect feature test macros) */

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

/* local header files needed */
#include "paths_decl.h"         /* declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <sys/stat.h>           /* stat */
#include <errno.h>              /* errno */
#include <limits.h>             /* *_MAX */
#include <stdlib.h>             /* NULL */
#include <string.h>             /* strerror strrchr */
#include <unistd.h>             /* access */

/* sanity check */
#ifndef	S_ISDIR                 /* required to be in sys/stat.h by POSIX.1 */
# if 1
#  error S_ISDIR is not defined
# else
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
# endif
#endif

/* static data and function definitions */
static char paths_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_paths(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    paths_initialized = register_build_strings(NULL, &source_file, s);
}

/* test if dir is a directory
   return > 0 if it is
   return 0 if it is not a directory
   use prog, ver, log_mask, log_arg, f to log information
*/
int is_dir(const char *dir)
{
    int nonexistent, ret = 0;
    struct stat statbuf;

    if (0U == paths_initialized)
        initialize_paths();
    nonexistent = access(dir, 0);
    if (0 != nonexistent) {
        return ret;
    }
    if (0 == stat(dir, &statbuf)) {
        ret = (S_ISDIR(statbuf.st_mode));
    }
    return ret;
}

