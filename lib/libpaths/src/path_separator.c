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
* $Id: ~|^` @(#)    path_separator.c copyright 2010-2017 Bruce Lilly.   \ path_separator.c $
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
/* $Id: ~|^` @(#)   This is path_separator.c version 1.7 dated 2017-09-01T02:22:30Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/src/s.path_separator.c */

/********************** Long description and rationale: ***********************
const char *path_separator(const char *path)
   One function to find the separator between dirname and basename,
   used by path_dirname and by path_basename to ensure that a decomposed path
   can be reconstructed (unless emulating broken-by-design Linux/GNU).
   Returns a pointer to the separator character in path (if there is
   one), else returns NULL.
   There is no separator if:
     path is NULL
     path is the empty string ""
     path consists entirely of slash characters '/' (one or more)
     path contains zero slash characters
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
#define ID_STRING_PREFIX "$Id: path_separator.c ~|^` @(#)"
#define SOURCE_MODULE "path_separator.c"
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
#include "paths_decl.h"         /* path_separator */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <stdlib.h>             /* NULL */
#include <string.h>             /* strrchr */

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

/* One function to find the separator between dirname and basename,
   used by path_dirname and by path_basename to ensure that a
   decomposed path can be reconstructed (unless emulating Linux/GNU).
   Returns a pointer to the separator character in path (if there is
   one), else returns NULL.
   There is no separator if:
     path is NULL
     path is the empty string ""
     path consists entirely of slash characters '/' (one or more)
     path contains zero slash characters
*/
const char *path_separator(const char *path
    )
{
    const char *maybe, *sep;

    if (NULL == path) {
        if (0U == paths_initialized)
            initialize_paths();
        return path;
    }
    maybe = sep = NULL;
    do {
        /* look for the next slash starting at the character after the last
           separator (if any) ...
        */
        if (NULL != maybe) { path = maybe+1; }
        /* ... skipping any additional consecutive slashes */
        while ('/' == *path) { path++; }
        switch (*path) {
            case '/':
                /* can't happen here */
            break;
            case '\0':
            return sep; /* if any -- end of path */
            default:
                sep = maybe; /* last seen separator (or NULL if none seen) */
            break;
        }
        maybe = (const char *)strchr(path, '/'); /* next potential separator */
    } while (NULL != maybe); /* quit if no more candidates */
    return sep;
}
