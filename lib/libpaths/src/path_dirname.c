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
* $Id: ~|^` @(#)    path_dirname.c copyright 2010-2017 Bruce Lilly.   \ path_dirname.c $
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
/* $Id: ~|^` @(#)   This is path_dirname.c version 1.7 dated 2017-09-01T02:22:30Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/src/s.path_dirname.c */

/********************** Long description and rationale: ***********************
* int path_dirname(const char *path, char *buf, int sz)
*Thread-safe, non-modifying, compile-time-configurable alternative to dirname().
*  Caller supplies a buffer for the result, so it's thread-safe unless
*    the caller goes out of his way to make it otherwise (e.g. by passing
*    a pointer to memory shared with other threads).
*  Supplied path is never written to, so it won't segfault if you pass a
*    constant string, and if you pass a pointer to an argv array element
*    or similar, it won't be altered.
*  Caller specifies the size of the result buffer; writing won't happen
*    beyond that specified size.
*  Return value is the number of characters (excluding the terminating '\0')
*    copied to the buffer if the entire directory name fits, or if it does
*    not fit, the return value is the size of buffer that would be required
*    (including space for the terminating '\0').  So if the return value is
*    larger than the specified buffer size, the result has been truncated
*    and the caller needs a larger buffer (at least the number of characters
*    given by the return value).
*  To determine the required size if no reasonable guess (e.g. PATH_MAX) is
*    available, one may call this function with zero size specified and/or
*    with a NULL buffer; the return value indicates the required size.
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
#define ID_STRING_PREFIX "$Id: path_dirname.c ~|^` @(#)"
#define SOURCE_MODULE "path_dirname.c"
#define MODULE_VERSION "1.7"
#define MODULE_DATE "2017-09-01T02:22:30Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2010-2017"

/* configuration (which might affect feature test macros) */

/* dirname implementation choices: */
/* N.B. see also path_basename.c */
/* dirname of "//" (exactly) is "//" (1) or "/" (0) */
#define PATHS_DIR_DOUBLE_SLASH_ONLY  0
/* dirname of something beginning with "//" followed by something other than a third '/' begins with "//" (1) or "/" (0) */
#define PATHS_DIR_DOUBLE_SLASH_START 1
/* emulation:         dirname          basename
                   _ONLY _START _END_EMPTY _EMPTY_EMPTY Comments
     NetBSD:         0      1       0            0      confirmed
     Linux/POSIX:    1      1       0            0      confirmed (oddball dirname)
     Linux/GNU:      1      1       1            1      confirmed (oddball basename (not POSIX compliant) and dirname)
     OpenIndiana     0      1       0            0      confirmed
*/
/* POSIX and X/OPEN conformance notes:
   dirname:
     POSIX 1003.1, 2013 edition dirname() is not clear regarding return for "//" (only).
       Linux dirname behavior might or might not be conforming.
       Unclear whether both values for PATHS_DIR_DOUBLE_SLASH_ONLY result in conformance.
     POSIX 1003.1, 2013 edition permits "//" or "/" given "//foo".
       Clearly, either value for PATHS_DIR_DOUBLE_SLASH_START result in conformance.
   basename:
     POSIX 1003.1, 2013 edition says that trailing slashes should be elided.
     POSIX 1003.1, 2013 edition permits "//" or "/" for path "//".
       Linux/GNU implementation is clearly non-conforming ("" is not permitted).
       Conformance requires PATHS_BASE_SLASH_END_EMPTY set to 0.
     POSIX 1003.1, 2013 edition says that a path consisting only of slash(es) should result in "/", and gives path "///" as an example.
       Linux/GNU implementation is clearly non-conforming ("" is not permitted).
       Conformance requires PATHS_BASE_SLASH_END_EMPTY set to 0.
     POSIX 1003.1, 2013 edition says that for NULL or empty path, return should be ".".
       Linux/GNU implementation is clearly non-confornming.
       Conformance requires PATHS_BASE_SLASH_EMPTY_EMPTY set to 0.
 POSIX compliance: _ONLY _START _END_EMPTY _EMPTY_EMPTY
                     ?      X       0            0
                  unclear don't  required    required
                          care
*/

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
#include "paths_decl.h"         /* path_dirname path_separator */
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

/*
 Thread-safe, non-modifying, compile-time-configurable alternative to dirname().
   Caller supplies a buffer for the result, so it's thread-safe unless
     the caller goes out of his way to make it otherwise (e.g. by passing
     a pointer to memory shared with other threads).
   Supplied path is never written to, so it won't segfault if you pass a
     constant string, and if you pass a pointer to an argv array element
     or similar, it won't be altered.
   Caller specifies the size of the result buffer; writing won't happen
     beyond that specified size.
   Return value is the number of characters (excluding the terminating '\0')
     copied to the buffer if the entire directory name fits, or if it does
     not fit, the return value is the size of buffer that would be required
     (including space for the terminating '\0').  So if the return value is
     larger than the specified buffer size, the result has been truncated
     and the caller needs a larger buffer (at least the number of characters
     given by the return value).
   To determine the required size if no reasonable guess (e.g. PATH_MAX) is
     available, one may call this function with zero size specified and/or
     with a NULL buffer; the return value indicates the required size.
*/
/* calls path_separator */
int path_dirname(const char *path
    , char *buf
    , int sz
    )
{
    int ret = 0;
    char *p=buf;
    const char *sep;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "path_dirname";
#endif

    sep = path_separator(path);
    if (NULL == sep) {
        if ((NULL == path) || ('/' != *path)) {
            if ((++ret < sz) && (NULL != buf)) {
                *p++ = '.';
            }
        } else {
            if ((++ret < sz) && (NULL != buf)) {
                *p++ = '/';
            }
#if PATHS_DIR_DOUBLE_SLASH_ONLY
            if (('/' == path[1]) && ('/' != path[2])) {
                if ((++ret < sz) && (NULL != buf)) {
                    *p++ = '/';
                }
            }
#endif
        }
    } else {
        for(; path<sep;) {
            if ((++ret < sz) && (NULL != buf)) {
                *p++ = *path;
            }
            switch (*path) {
                case '/':
#if PATHS_DIR_DOUBLE_SLASH_START
                    if (('/' == path[1]) && ('/' != path[2])) {
                        if ((++ret < sz) && (NULL != buf)) {
                            *p++ = '/';
                        }
                    }
#endif
                    while ('/' == *path) ++path;
                break;
                default:
                    ++path;
                break;
            }
        }
    }
    if (NULL != buf) {
        *p = '\0';
    } else {
        ++ret;
        if (0U == paths_initialized)
            initialize_paths();
    }
    return ret;
}
