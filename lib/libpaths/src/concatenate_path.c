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
* $Id: ~|^` @(#)    concatenate_path.c copyright 2010-2018 Bruce Lilly.   \ concatenate_path.c $
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
/* $Id: ~|^` @(#)   This is concatenate_path.c version 1.8 dated 2018-12-16T19:20:59Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/src/s.concatenate_path.c */

/********************** Long description and rationale: ***********************
* int concatenate_path(const char *dir, const char *base, const char *suffix,
    char *buf, int sz)
* build full path from dir, base, and suffix
*   result is dir/base.suffix if all three are non-NULL
*         (leading slashes in base and leading dots in suffix are ignored)
*      base.suffix if dir is NULL (leading dots in suffix are ignored)
*      dir/base if suffix is NULL (leading slashes in base are ignored)
*      dir.suffix if base is NULL (leading dots in suffix are ignored)
*      copy non-NULL component if the other two are NULL
*      empty string (i.e. only '\0' character) if all three are NULL
*   put result in buf (if not NULL) of size sz (if large enough)
*   return length of result if buf is not NULL and is large enough,
*     or size required for result if buf is NULL or is too small
*   neither dir nor base are checked for existence or normalized
*   no expansion of relative path components is performed
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
#define ID_STRING_PREFIX "$Id: concatenate_path.c ~|^` @(#)"
#define SOURCE_MODULE "concatenate_path.c"
#define MODULE_VERSION "1.8"
#define MODULE_DATE "2018-12-16T19:20:59Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2010-2018"

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
#include "paths_decl.h"         /* compilation macros, declarations */
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

/* build full path from dir, base, and suffix
   result is dir/base.suffix if all three are non-NULL
         (leading slashes in base and leading dots in suffix are ignored)
      base.suffix if dir is NULL (leading dots in suffix are ignored)
      dir/base if suffix is NULL (leading slashes in base are ignored)
      dir.suffix if base is NULL (leading dots in suffix are ignored)
      copy non-NULL component if the other two are NULL
      empty string (i.e. only '\0' character) if all three are NULL
   put result in buf (if not NULL) of size sz (if large enough)
   return length of result if buf is not NULL and is large enough,
     or size required for result if buf is NULL or is too small
   neither dir nor base are checked for existence or normalized
   no expansion of relative path components is performed
*/
int concatenate_path(const char *dir, const char *base, const char *suffix,
    char *buf, int sz)
{
    int ret = 0;

    if (0U == paths_initialized)
        initialize_paths();
    /* copy dir to result */
    if (NULL != dir) {
        for (; '\0'!=*dir; dir++) {
            if ((++ret < sz) && (NULL != buf))
                *buf++ = *dir;
        }
        --dir; /* last character of dir */
    }
    /* append base */
    if (NULL != base) {
        /* skip over leading separators in base */
        if (NULL != base) {
            while ('/' == *base)
                base++;
        }
        /* append a separator if needed */
        if ((NULL != dir) && ('\0' != *base)) {
            if ('/' != *dir) {
                if ((++ret < sz) && (NULL != buf))
                    *buf++ = '/';
            }
        }
        for (; '\0'!=*base; base++) {
            if ((++ret < sz) && (NULL != buf))
                *buf++ = *base;
        }
        --base; /* last character of base */
    }
    /* append suffix */
    if (NULL != suffix) {
        /* skip over leading dots in suffix */
        if (NULL != suffix) {
            while ('.' == *suffix)
                suffix++;
        }
        /* append a separator and/or dot if needed */
        if ('\0' != *suffix) {
            if (NULL == base) {
                if ((NULL != dir) && ('/' != *dir)) {
                    if ((++ret < sz) && (NULL != buf))
                        *buf++ = '/';
                }
                if ((++ret < sz) && (NULL != buf))
                    *buf++ = '.';
            } else {
                if ('.' == *base) { /* base ending in dot is assumed to be a relative directory */
                    if ((++ret < sz) && (NULL != buf))
                        *buf++ = '/';
                }
                if ((++ret < sz) && (NULL != buf))
                    *buf++ = '.';
            }
        }
        for (; '\0' != *suffix; suffix++) {
            if ((++ret < sz) && (NULL != buf))
                *buf++ = *suffix;
        }
    }
    /* terminate the string with a NUL, or account for it in returned size */
    if ((NULL != buf) && (ret < sz))
       *buf = '\0';
    else
        ret++;
    return ret;
}
