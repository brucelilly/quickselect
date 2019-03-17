/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: get_host_name - put fully-qualified host name in a buffer, accounting for system dependencies
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    get_host_name.c copyright 2015-2019 Bruce Lilly.   \ get_host_name.c $
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
/* $Id: ~|^` @(#)   This is get_host_name.c version 1.5 dated 2019-03-15T20:40:37Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "get_host_name" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libid/src/s.get_host_name.c */

/********************** Long description and rationale: ***********************
* gethostname() should work on all Unix-like systems, but the Linux
* implementation returns an unqualified name.
* get_host_name works around that bug, providing a uniform interface that works
*   properly on all Unix-like systems as well as Linux.
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
#define ID_STRING_PREFIX "$Id: get_host_name.c ~|^` @(#)"
#define SOURCE_MODULE "get_host_name.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2019-03-15T20:40:37Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2015-2019"

/* feature test macros defined before any header files are included */
/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 400 /* <=500 for illumos, <500 for FreeBSD */
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

#define GET_HOST_NAME_C_XOPEN_SOURCE 400
#if ! defined(_XOPEN_SOURCE) || (GET_HOST_NAME_C_XOPEN_SOURCE > _XOPEN_SOURCE)
# ifdef _XOPEN_SOURCE
#  undef _XOPEN_SOURCE
# endif
# define _XOPEN_SOURCE GET_HOST_NAME_C_XOPEN_SOURCE
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* for hstrerror in netdb.h */
#undef _BSD_SOURCE
#define _BSD_SOURCE 1
#undef _SVID_SOURCE
#define _SVID_SOURCE 1
#undef _NETBSD_SOURCE
#define _NETBSD_SOURCE 1

/*INDENT ON*/
/* *INDENT-ON* */

/* other local header files needed */
#include "get_host_name.h"      /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <sys/utsname.h>        /* uname */
#include <errno.h>              /* errno */
#include <netdb.h>              /* struct hostent hstrerror (Linux: _BSD_SOURCE || _SVID_SOURCE || _GNU_SOURCE, NetBSD: _NETBSD_SOURCE, Solaris: __EXTENSIONS__) */
#ifndef NULL
# include <stddef.h>            /* NULL */
#endif
#include <string.h>             /* strerror strncpy strrchr */
#include <syslog.h>             /* LOG_* */
#include <unistd.h>             /* access close gethostname getpid getopt optarg opterr optind optopt sleep stat unlink */

/* static data and function definitions */
static char get_host_name_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_get_host_name(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    get_host_name_initialized = register_build_strings(NULL, &source_file, s);
}

/* encapsulation of hostname determination */
char *get_host_name(char *buf, int sz,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "get_host_name";
#endif

    if (NULL != buf) {
        char *p, *q;
        int c, i;

        if (NULL != f) {
            if ((char)0 == get_host_name_initialized)
                initialize_get_host_name(); /* for source_file */
        }
        /* (should be) fully-qualified host name */
        if ('\0' == *buf) {
            i = gethostname(buf, (size_t)sz);
        } else
            i = 0;
        if ((0 != i) || ('\0' == *buf)) { /* try uname if gethostname failed */
            struct utsname local_utsname;

            if (0 != i) {
                i = errno;
                if (NULL != f) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: gethostname failed: %m",
                        __func__, source_file, __LINE__
                    );
                }
                errno = i;
            }
            i = uname(&local_utsname);
            if (0 > i) {
                i = errno;
                if (NULL != f) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: uname() failed: %m",
                        __func__, source_file, __LINE__
                    );
                }
                errno = i;
            } else {
                i = sz - 1;
                c = sizeof(local_utsname.nodename);
                if (c < i)
                    i = c;
                (void)strncpy(buf, local_utsname.nodename, (size_t)i); /* Flawfinder: ignore */
                if (NULL != f) {
                    f(LOG_INFO, log_arg,
                        "%s: %s line %d: host uname nodename %s",
                        __func__, source_file, __LINE__, buf
                    );
                }
            }
        }
        if ('\0' == *buf) {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: no host name found",
                    __func__, source_file, __LINE__
                );
            }
        } else {
            if (0 < sz)
                buf[sz-1] = '\0';
            p = strchr(buf, '.');
            /* the following several dozen lines exist to deal with Linux braindamage */
            if (NULL == p) { /* Linux gethostname does NOT return a FQDN `;-/ */
                struct hostent *servhost;

                if (NULL != f) {
                    f(LOG_WARNING, log_arg,
                        "%s: %s line %d: unqualified host name %s",
                        __func__, source_file, __LINE__, buf
                    );
                }
                c = 0;
                do {
                    errno = 0;
                    h_errno = 0;
                    servhost = gethostbyname(buf);  /* RATS: ignore (name is used for logging only) */
                    c++;
                    if (NULL == servhost) {
                        if (0 >= h_errno) {
                            if (NULL != f) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: (%d): gethostbyname(%s): %s (%d)",
                                    __func__, source_file, __LINE__, c,
                                    buf, strerror(errno), errno
                                );
                            }
                        } else {
                            if (NULL != f) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: (%d): gethostbyname(%s): %s (%d)",
                                    __func__, source_file, __LINE__, c,
                                    buf, hstrerror(h_errno), h_errno
                                );
                            }
                        }
                        if ((EAGAIN == errno) || (TRY_AGAIN == h_errno)) {
                            if (6 > c)
                                sleep(1U << (c - 1));
                        }
                    }
                } while ((NULL == servhost) && (6 > c));
                if (NULL != servhost) {
                    if (NULL != servhost->h_name) {
                        if ('\0' != servhost->h_name[0]) {
                            q = strchr(servhost->h_name, '.');
                            if (NULL != q) {
                                c = sz - 1;
                                (void)strncpy(buf, servhost->h_name, (size_t)c); /* Flawfinder: ignore */
                            }
                        }
                    }
                }
            }
            q = strchr(buf, '.');
            if (NULL == q) {
                if (NULL != f) {
                    f(LOG_WARNING, log_arg,
                        "%s: %s line %d: could not qualify unqualified name %s",
                        __func__, source_file, __LINE__, buf
                    );
                }
            } else if (NULL == p) {
                if (NULL != f) {
                    f(LOG_INFO, log_arg,
                        "%s: %s line %d: qualified host name %s",
                        __func__, source_file, __LINE__, buf
                    );
                }
            }
        }
    }
    return buf;
}
