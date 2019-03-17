/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: get_ips - put ip address list for a host into a buffer
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    get_ips.c copyright 2015-2018 Bruce Lilly.   \ get_ips.c $
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
/* $Id: ~|^` @(#)   This is get_ips.c version 1.5 dated 2018-12-25T16:23:20Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "get_ips" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libid/src/s.get_ips.c */

/********************** Long description and rationale: ***********************
* get_ips: fill a buffer with a list of IP addresses for a given hostname.
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
#define ID_STRING_PREFIX "$Id: get_ips.c ~|^` @(#)"
#define SOURCE_MODULE "get_ips.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2018-12-25T16:23:20Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2015-2018"

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

#if 0
/* for hstrerror in netdb.h */
#undef _BSD_SOURCE
#define _BSD_SOURCE 1
#undef _SVID_SOURCE
#define _SVID_SOURCE 1
#undef _NETBSD_SOURCE
#define _NETBSD_SOURCE 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* other local header files needed */
#include "get_ips.h"            /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <arpa/inet.h>          /* inet_ntop ntohl */
#define __POSIX_VISIBLE 200112 /* for FreeBSD */
#include <netinet/in.h>         /* struct sockaddr_in struct sockaddr_in6 */
#include <errno.h>              /* errno */
#include <netdb.h>              /* getaddrinfo */
#ifndef NULL
# include <stddef.h>            /* NULL */
#endif
#include <string.h>             /* memcpy memset strlen strncpy strrchr strstr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char get_ips_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_get_ips(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    get_ips_initialized = register_build_strings(NULL, &source_file, s);
}

/* encapsulation of IP address determination */
int get_ips(char *buf, int sz, char *hostname, int family, int flags,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char *p;
    int c;
    unsigned long ul;
    size_t len;
    struct addrinfo hints, *result, *rp;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "get_ips";
#endif

    if (NULL != f) {
        if (0 == get_ips_initialized)
            initialize_get_ips(); /* for source_file */
    }
    if ((NULL == buf) || (16 > sz) || (NULL == hostname) || ('\0' == *hostname)) {
        if (NULL != f) {
            ul = (unsigned long)buf;
            f(LOG_ERR, log_arg,
                "%s: %s line %d: %s(0x%lx, %d, %s, ...) has invalid argument",
                __func__, source_file, __LINE__,
                __func__, ul, sz, NULL==hostname? "NULL": hostname
            );
        }
        errno = EINVAL;
        return -1;
    }
    /* host address(es) */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_flags = flags;
    result = NULL;
    c = getaddrinfo(hostname, NULL, &hints, &result);
    switch (c) {
        case 0:
            p = buf;
            for (rp = result; NULL != rp; rp = rp->ai_next) {
                char ipbuf[64]; /* RATS: ignore (large enough even for verbose ipv6) */
                char *seen;
                struct sockaddr_in  ipv4;
                struct sockaddr_in6 ipv6;

                switch (rp->ai_addr->sa_family) {
                    case AF_INET:
                        /* LINTED */
                        /* avoid possible alignment issues */
                        memset(&ipv4, 0, sizeof(struct sockaddr_in));
                        memcpy(&ipv4, rp->ai_addr, sizeof(struct sockaddr_in));
                        ul = ntohl(ipv4.sin_addr.s_addr);
                        if (127UL == ((ul & 0xff000000UL)>>24))
                            continue; /* ignore loopback */
                        inet_ntop(rp->ai_addr->sa_family,
                            &(ipv4.sin_addr), ipbuf, sizeof(ipbuf));
                        seen = strstr(buf, ipbuf);
                        if (NULL == seen) {
                            len = strlen(ipbuf); /* RATS: ignore (inet_ntop terminates string) */
                            if (len < (unsigned int)sz + 1U) {
                                if ('\0' != buf[0]) {
                                    *p = ',';
                                    sz--;
                                    p++;
                                }
                                strncpy(p, ipbuf, (size_t)sz); /* Flawfinder: ignore */
                                sz -= len;
                                p += len;
                            }
                        }
                    break;
                    case AF_INET6:
                        /* LINTED */
                        /* avoid possible alignment issues */
                        memset(&ipv6, 0, sizeof(struct sockaddr_in6));
                        memcpy(&ipv6, rp->ai_addr, sizeof(struct sockaddr_in6));
                        inet_ntop(rp->ai_addr->sa_family,
                            &(ipv6.sin6_addr), ipbuf, sizeof(ipbuf));
                        seen = strstr(buf, ipbuf);
                        if (NULL == seen) {
                            len = strlen(ipbuf); /* RATS: ignore (inet_ntop terminates string) */
                            if (len < (unsigned int)sz + 1U) {
                                if ('\0' != buf[0]) {
                                    *p = ',';
                                    sz--;
                                    p++;
                                }
                                strncpy(p, ipbuf, (size_t)sz); /* Flawfinder: ignore */
                                sz -= len;
                                p += len;
                            }
                        }
                    break;
                    default:
                        if (NULL != f) {
                            f(LOG_WARNING, log_arg,
                                "%s: %s line %d: unknown address family 0x%lx",
                                __func__, source_file, __LINE__,
                                (unsigned long)(rp->ai_addr->sa_family)
                            );
                        }
                    break;
                }
            }
        break;
        case EAI_BADFLAGS:        /* Invalid value for `ai_flags' field.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_BADFLAGS: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_NONAME:        /* NAME or SERVICE is unknown.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_NONAME: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_AGAIN:                /* Temporary failure in name resolution.  */
            errno = EAGAIN;
        break;
        case EAI_FAIL:                /* Non-recoverable failure in name res.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_FAIL: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_FAMILY:        /* `ai_family' not supported.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_FAMILY: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_SOCKTYPE:        /* `ai_socktype' not supported.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_SOCKTYPE: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_SERVICE:        /* SERVICE not supported for `ai_socktype'.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_SERVICE: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_MEMORY:        /* Memory allocation failure.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_MEMORY: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                     __func__, hostname, hints.ai_family,
                     hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
        case EAI_SYSTEM:        /* System error returned in `errno'.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_SYSTEM: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x: errno %d", 
                    __func__, hostname, hints.ai_family,
                    hints.ai_socktype, hints.ai_protocol, hints.ai_flags, errno
                );
            }
        break;
        case EAI_OVERFLOW:        /* Argument buffer overflow.  */
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "EAI_OVERFLOW: %s was given node \"%s\" fam %d socktype %d protocol %d flags 0x%x", 
                    __func__, hostname, hints.ai_family,
                    hints.ai_socktype, hints.ai_protocol, hints.ai_flags
                );
            }
        break;
    }
    freeaddrinfo(result);           /* No longer needed */
    return c;
}
