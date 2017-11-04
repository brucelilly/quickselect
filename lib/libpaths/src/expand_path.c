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
* $Id: ~|^` @(#)    expand_path.c copyright 2010-2017 Bruce Lilly.   \ expand_path.c $
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
/* $Id: ~|^` @(#)   This is expand_path.c version 1.7 dated 2017-09-01T02:22:30Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/src/s.expand_path.c */

/********************** Long description and rationale: ***********************
* int expand_path(const char *path, char *buf, int sz)
* expand possibly relative path to full path in supplied buffer
*   put result in buf (if not NULL) of size sz (if large enough)
*   return length of result if buf is not NULL and is large enough,
*     or size required for result if buf is NULL or is too small
*   return -1 on error
*   path is not checked for existence or modified
* NULL path and empty string for path expand to current directory
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
#define ID_STRING_PREFIX "$Id: expand_path.c ~|^` @(#)"
#define SOURCE_MODULE "expand_path.c"
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
#include "paths_decl.h"         /* expand_path normalize_path relative_path */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <errno.h>              /* errno */
#include <limits.h>             /* *_MAX */
#include <stdlib.h>             /* NULL */
#include <string.h>             /* strncmp strrchr strstr */
#include <unistd.h>             /* getcwd */

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

/* expand possibly relative path to full path in supplied buffer
   put result in buf (if not NULL) of size sz (if large enough)
   return length of result if buf is not NULL and is large enough,
     or size required for result if buf is NULL or is too small
   return -1 on error
   path is not checked for existence or modified
*/
int expand_path(const char *path, char *buf, int sz)
{
    char *p, curdir[PATH_MAX], temp[PATH_MAX];
    const char *q;
    int r, ret = 0;

    if (0U == paths_initialized)
        initialize_paths();
    if (NULL != buf)
        buf[0] = '\0';
    if (NULL != path) {
        if (0 == strncmp(path, "..", 2)) {
            curdir[0] = '\0';
            p = getcwd(temp, sizeof(temp));    /* if getcwd fails, p is NULL */
            if (NULL == p)
                return -1;
            r = normalize_path(temp, curdir, sizeof(curdir));
            if ((int)sizeof(curdir) < r)
                return -1;
            p = strrchr(curdir, '/');
            switch (path[2]) {
                case '\0':         /* parent directory, if any */
                    if (NULL == p) {
                        /* curdir must have been "/" before normalization */
                        /* treat ../ from root as root */
                        q = "/";
                    } else {
                        *p = '\0'; /* trim to directory above curdir */
                        q = curdir;
                        if ('\0' == *q)
                            q = "/";
                    }
                    for (p = buf; '\0' != *q; q++)
                        if ((++ret < sz) && (NULL != buf))
                            *p++ = *q;
                    if (NULL != buf)
                        *p = '\0';
                break;
                case '/':          /* directory under parent directory, if any */
                    /* have path = "../something" (or maybe just "../") */
                    for (q = path; 0 == strncmp(q, "../", 3); ) {
                        q += 3;
                        if (NULL == p)
                            p = strncpy(curdir, "/", (size_t)2);
                        else {
                            *p = '\0';
                             p = strrchr(curdir, '/');
                        }
                        while (0 == strncmp(q, "./", 2))
                            q += 2;
                    }
                    if (curdir[0] == '\0')
                        (void)strncpy(curdir, "/", (size_t)2);
                return relative_path(curdir, q, buf, sz);
                default:           /* directory or file name beginning with "..", relative to the current directory */
                return relative_path(curdir, path, buf, sz);
            }
        } else {
            size_t ssz;

            switch (path[0]) {
                case '.': /* doesn't start with ".." (strncmp tested) */
                    /* might be sole ".", or start with "./", or be a hidden directory or file */
                    switch (path[1]) {
                        case '/':  /* "./" can be ignored */
                            if (path[2] != '\0')
                                return expand_path(path + 2, buf, sz);
                            /* else special case; path is sole "./"; return current directory */
                        /*FALLTHROUGH*/
                        case '\0': /* current directory */
                            curdir[0] = '\0';
                            p = getcwd(temp, sizeof(temp));    /* if getcwd fails, p is NULL */
                            if (NULL == p)
                                return -1;
                            r = normalize_path(temp, curdir, sizeof(curdir));
                            if ((int)sizeof(curdir) < r)
                                return -1;
                            q = curdir;
                            if ('\0' == *q)
                                q = "/";
                            for (p = buf; '\0' != *q; q++)
                                if ((++ret < sz) && (NULL != buf))
                                    *p++ = *q;
                            if (NULL != buf)
                                *p = '\0';
                        break;
                        default:   /* [hidden] directory or file name beginning with ".", relative to the current directory */
                            curdir[0] = '\0';
                            p = getcwd(temp, sizeof(temp));    /* if getcwd fails, p is NULL */
                            if (NULL == p)
                                return -1;
                            r = normalize_path(temp, curdir, sizeof(curdir));
                            if ((int)sizeof(curdir) < r)
                                return -1;
                            p = curdir;
                        return relative_path(p, path, buf, sz);
                    }
                break;
                case '/': /* a full path which might have embedded relative redirection */
                    for (q = path, q = strstr(q, "/.."); NULL != q; q = strstr(q+3, "/..")) {
                        const char *s;

                        switch (q[3]) {
                            case '\0':  /* at end; ignore last component before "/.." */
                            /*FALLTHROUGH*/
                            case '/': /* redirection; eliminate component before "/../" */
                                if (q > path) {
                                    for (s=q-1; s > path; s--)
                                        if ('/' == *s)
                                            break;
                                    ssz = (size_t)s - (size_t)path;
                                } else {
                                    s = path;
                                    ssz = 0;
                                }
                                if (sizeof(curdir) < ssz) {
                                    errno = ENAMETOOLONG;
                                    return -1;
                                }
                                curdir[0] = '\0';
                                if (s != path) {
                                    (void)strncpy(curdir, path, ssz);
                                    curdir[ssz] = '\0';
                                }
                                if ('\0' == curdir[0])
                                    (void)strncpy(curdir, "/", (size_t)2);
                            return relative_path(curdir, q + 3, buf, sz);
                            default: /* ".." is part of some directory or file name; try for another "/.." later in the path */
                            continue;
                        }
                    }
                    for (q = path, q = strstr(q, "/."); q != NULL; q = strstr(q+2, "/.")) {
                        switch (q[2]) {
                            case '\0':  /* at end; ignore trailing "/." */
                            /*FALLTHROUGH*/
                            case '/': /* redirection; eliminate redundant "/." */
                                ssz = (size_t)q - (size_t)path;
                                if (sizeof(curdir) < ssz) {
                                    errno = ENAMETOOLONG;
                                    return -1;
                                }
                                curdir[0] = '\0';
                                if (q != path) {
                                    (void)strncpy(curdir, path, ssz);
                                    curdir[ssz] = '\0';
                                }
                                if ('\0' == curdir[0])
                                    (void)strncpy(curdir, "/", (size_t)2);
                            return relative_path(curdir, q + 2, buf, sz);
                            default: /* "." is part of some hidden directory or file name; try for another "/." later in the path */
                            continue;
                        }
                    }
                    /* no redirection; just copy */
                    for (q = path, p = buf; '\0' != *q; q++)
                        if ((++ret < sz) && (NULL != buf))
                            *p++ = *q;
                    if (NULL != buf)
                        *p = '\0';
                break;
                default:           /* some directory or file name relative to current directory */
                                   /* empty path string (path[0] == '\0') is also handled here */
                    curdir[0] = '\0';
                    p = getcwd(temp, sizeof(temp));    /* if getcwd fails, p is NULL */
                    if (NULL == p)
                        return -1;
                    r = normalize_path(temp, curdir, sizeof(curdir));
                    if ((int)sizeof(curdir) < r)
                        return -1;
                    p = curdir;
                return relative_path(p, path, buf, sz);
            }
        }
    } else { /* NULL path is expanded to current directory */
        curdir[0] = '\0';
        p = getcwd(temp, sizeof(temp));    /* if getcwd fails, p is NULL */
        if (NULL == p)
            return -1;
        r = normalize_path(temp, curdir, sizeof(curdir));
        if ((int)sizeof(curdir) < r)
            return -1;
        p = curdir;
        return relative_path(p, path, buf, sz);
    }
    if ((NULL == buf) || (ret >= sz))
        ret++;
    return ret;
}
