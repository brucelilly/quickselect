/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for configurable logging
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    logger.c copyright 2010 - 2017 Bruce Lilly.   \ logger.c $
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
/* $Id: ~|^` @(#)   This is logger.c version 2.16 dated 2017-01-16T00:05:46Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "logger" */
/*****************************************************************************/
/* maintenance note: master file /src/radioclk/radioclk-1.0/lib/liblogger/src/s.logger.c */

/********************** Long description and rationale: ***********************
* Send syslog to UDP syslog port on a particular host, system syslog, a stream,
* a character array, and/or a syslog-like external function
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
#define ID_STRING_PREFIX "$Id: logger.c ~|^` @(#)"
#define SOURCE_MODULE "logger.c"
#define MODULE_VERSION "2.16"
#define MODULE_DATE "2017-01-16T00:05:46Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2010 - 2017"

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

/* for hstrerror in netdb.h */
#undef _BSD_SOURCE
#define _BSD_SOURCE 1
#undef _SVID_SOURCE
#define _SVID_SOURCE 1
#undef _NETBSD_SOURCE
#define _NETBSD_SOURCE 1

/*INDENT ON*/
/* *INDENT-ON* */

#define	LOG_TABLES

/* local header files needed */
#include "civil_time.h"         /* sn_civil_time */
#if TESTING
#include "get_host_name.h"      /* get_host_name */
#include "get_ips.h"            /* get_ips */
#endif
#include "logger.h"             /* header file for public definitions and declarations */ /* includes sys/types.h stdio.h syslog.h netinet/in.h */
#include "snn.h"                /* snl snul */
#include "utc_mktime.h"         /* local_utc_offset */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <sys/socket.h>         /* getaddrinfo sendto SOCK_... */
#if TESTING
# include <sys/param.h>         /* MAXHOSTNAMELEN */
#endif
#include <ctype.h>              /* isalnum isspace */
#ifndef EINVAL
# include <errno.h>             /* errno */
#endif
#include <netdb.h>              /* getaddrinfo struct hostent hstrerror (Linux: _BSD_SOURCE || _SVID_SOURCE || _GNU_SOURCE, NetBSD: _NETBSD_SOURCE, Solaris: __EXTENSIONS__) */
#include <stdarg.h>             /* va_... */
#ifndef NULL
# include <stddef.h>            /* NULL */
#endif
#include <string.h>             /* strerror strlen strncpy strrchr strspn */ /* memset (TESTING) */
#include <unistd.h>             /* getpid */ /* (TESTING) gethostname sleep */
#include <time.h>               /* localtime_r */

/* static data and function definitions */
static const char *month_abbr[] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
    "Nov", "Dec"
};
static char logger_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* copyright notices are required to be visible! */
static void initialize_logger(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    logger_initialized = register_build_strings(NULL, &source_file, s);
}

static void vlog_string(int pri,
    struct logger_struct *pls, struct civil_time_struct *pcts,
    const char *fmt, va_list ap)
{
    /* format string is built here, in the fmtbuf array, then passed to vsnprintf to build message */
    char fmtbuf[65536];         /* RATS: ignore (it's big enough) */
    char *p, *q;
    const char *s, *t, *u, *v;
    char ch;
    int i, m, n, r, saved_errno = errno, sz;
    unsigned int ui, ui2, ui3, *pui, *pui2, *pui3;
    size_t len, len2, len3, len4;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "vlog_string";
#endif

    if (0 == logger_initialized)
        initialize_logger();
    if ((NULL == pls->buf) || (NULL == pls->pbufsz) || (32 > *(pls->pbufsz))) {
#if TESTING
        if (NULL == pls->buf)
            (void)fprintf(stderr,
                "%s: %s line %d: NULL buffer pointer\n",
                __func__, source_file, __LINE__
            );
        if (NULL == pls->pbufsz) {
            (void)fprintf(stderr,
                "%s: %s line %d: NULL buffer size pointer\n",
                __func__, source_file, __LINE__
            );
        } else {
            (void)fprintf(stderr,
                "%s: %s line %d: buffer size %d\n",
                __func__, source_file, __LINE__,
                *(pls->pbufsz)
            );
        }
#endif
        pls->msgoffset = 0U;
        pls->msglen = 0U;
        return; /* can't write anything useful */
    }
    /* build format string in fmtbuf, substituting error message for %m */
    /* start with date, time, program name (if not NULL), and PID (if requested) */
    pri &= LOG_PRIMASK; /* ignore facility */
    p = fmtbuf;
    sz = (int)sizeof(fmtbuf);
    *p = '\0';
    r = 0;
    /* Build the message. */
    /* syslog priority */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    switch (pls->format) {
        case LOGGER_FORMAT_RFC3164:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            r = snl(p, sz, "<", ">", pri | pls->logfac, 10, '0', 1, NULL, NULL); /* don't log snl activity (avoid infinite loop) */
            if ((0 <= r) && (r < sz)) {
                p += r;
                sz -= r;
            }
        break;
        case LOGGER_FORMAT_PLAIN:
        break;
        default:
        break;
    }
    /* RFC 5424 version and SP */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    switch (pls->format) {
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            *p++ = '1';
            sz--;
            *p++ = ' ';
            sz--;
            r += 2;
        break;
        case LOGGER_FORMAT_RFC3164:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    pls->tsoffset = (unsigned)r;
    /* timestamp and SP */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    switch (pls->format) {
        case LOGGER_FORMAT_RFC3164:
            /* cannot simply pad between month and day with snl (whitespace padding is put before prefix) */
            n = snprintf(p, (size_t)sz, "%s ", month_abbr[pcts->tm.tm_mon]); /* Flawfinder: ignore (compliant snprintf assumed) */
            if ((0 <= n) && (n < sz)) {
                p += n;
                sz -= n;
                r += n;
            }
            n = snl(p, sz, NULL, " ", pcts->tm.tm_mday, 10, ' ', 2, NULL, NULL); /* don't log snl activity (avoid infinite loop) */
            if ((0 <= n) && (n < sz)) {
                p += n;
                sz -= n;
                r += n;
            }
            n = snl(p, sz, NULL, ":", pcts->tm.tm_hour, 10, '0', 2, NULL, NULL); /* don't log snl activity (avoid infinite loop) */
            if ((0 <= n) && (n < sz)) {
                p += n;
                sz -= n;
                r += n;
            }
            n = snl(p, sz, NULL, ":", pcts->tm.tm_min, 10, '0', 2, NULL, NULL); /* don't log snl activity (avoid infinite loop) */
            if ((0 <= n) && (n < sz)) {
                p += n;
                sz -= n;
                r += n;
            }
            n = snl(p, sz, NULL, " ", pcts->tm.tm_sec, 10, '0', 2, NULL, NULL); /* don't log snl activity (avoid infinite loop) */
            if ((0 <= r) && (r < sz)) {
                p += n;
                sz -= n;
                r += n;
            }
        break;
        case LOGGER_FORMAT_RFC5424_LOCAL:
            n = sn_civil_time(p, sz, pcts, -6, 0, 0, NULL, NULL); /* at most 6 digits in fraction for RFC 5424 */ /* don't log sn_civil_time activity (avoid infinite loop) */
            if ((0 <= n) && (n < sz)) {
                p += n;
                sz -= n;
                *p++ = ' ';
                sz--;
                r += n + 1;
            }
        break;
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            n = sn_civil_time(p, sz, pcts, -6, 1, 0, NULL, NULL); /* at most 6 digits in fraction for RFC 5424 */ /* don't log sn_civil_time activity (avoid infinite loop) */
            if ((0 <= n) && (n < sz)) {
                p += n;
                sz -= n;
                *p++ = ' ';
                sz--;
                r += n + 1;
            }
        break;
    }
    /* hostname and SP */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
/* protect against illegal characters (esp. '%') in caller-supplied strings (e.g. fqdn) */
    s = pls->fqdn;
    switch (pls->format) {
        case LOGGER_FORMAT_RFC3164:
            if ((NULL != s) && ('\0' != *s) && (NULL == strchr(s, '%'))) {
                q = strchr(s, '.');
                if (NULL != q) {
                    n = q - s;
                    m = snprintf(p, (size_t)sz, "%*.*s ", n, n, s); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                } else {
                    m = snprintf(p, (size_t)sz, "%s ", s); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
            } else {
                m = snprintf(p, (size_t)sz, "unknown "); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
        break;
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            if ((NULL != s) && ('\0' != *s) && (NULL == strchr(s, '%'))) {
                m = snprintf(p, (size_t)sz, "%s ", s); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            } else {
                m = snprintf(p, (size_t)sz, "- "); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
        break;
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    /* TAG or APP-NAME SP PROCID SP */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    s = pls->ident;
    t = pls->procid;
    switch (pls->format) {
/* protect against illegal characters (esp. '%') in caller-supplied strings (e.g. fqdn) */
        case LOGGER_FORMAT_RFC3164:
            if ((NULL != s) && ('\0' != *s) && (NULL == strchr(s, '%'))) {
                /* ident is OK */
                if ((NULL != t) && ('\0' != *t) && (NULL == strchr(t, '%'))) {
                    /* procid is OK */
                    m = snprintf(p, (size_t)sz, "%s[%s]: ", s, t); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                } else {
                    m = snprintf(p, (size_t)sz, "%s: ", s); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
            } else {
                if ((NULL != t) && ('\0' != *t) && (NULL == strchr(t, '%'))) {
                    /* procid is OK */
                    m = snprintf(p, (size_t)sz, "unknown[%s]: ", t); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                } else {
                    m = snprintf(p, (size_t)sz, "unknown: "); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
            }
        break;
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            if ((NULL != s) && ('\0' != *s) && (NULL == strchr(s, '%'))) {
                /* ident is OK */
                if ((NULL != t) && ('\0' != *t) && (NULL == strchr(t, '%'))) {
                    /* procid is OK */
                    m = snprintf(p, (size_t)sz, "%s %s ", s, t); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                } else {
                    m = snprintf(p, (size_t)sz, "%s - ", s); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
            } else {
                if ((NULL != t) && ('\0' != *t) && (NULL == strchr(t, '%'))) {
                    /* procid is OK */
                    m = snprintf(p, (size_t)sz, "- %s ", t); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                } else {
                    m = snprintf(p, (size_t)sz, "- - "); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
            }
        break;
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    /* RFC 3164 format (and protected version) have inadequate timestamp information (no zone information, no fraction): add detailed timestamp to start of CONTENT */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    switch (pls->format) {
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
        break;
        case LOGGER_FORMAT_RFC3164:
            m = sn_civil_time(p, sz, pcts, -6, 1, 0, NULL, NULL); /* at most 6 digits in fraction */ /* don't log sn_civil_time activity (avoid infinite loop) */
            if ((0 <= m) && (m < sz)) {
                p += m;
                sz -= m;
                *p++ = ' ';
                sz--;
                r += m + 1;
            }
        break;
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    /* RFC 5424 MSGID and SP */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
/* protect against illegal characters (esp. '%') in caller-supplied strings (e.g. fqdn) */
    s = pls->msgid;
    switch (pls->format) {
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            if ((NULL != s) && ('\0' != *s) && (NULL == strchr(s, '%'))) {
                m = snprintf(p, (size_t)sz, "%s ", pls->msgid); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            } else {
                m = snprintf(p, (size_t)sz, "- "); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
        break;
        case LOGGER_FORMAT_RFC3164:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    /* RFC 5424 STRUCTURED-DATA (no SP in case the MSG starts with user-defined structured-data) */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    switch (pls->format) {
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            n = 0; /* counter for all structured data elements */
            /* timeQuality */
            i = 0; /* counter for timeQuality elements */
                /* txKnown */
            pui = pls->ptzKnown;
            ui = 2U; /* invalid value sentinel */
            if (NULL != pui)
                ui = *pui;
            if (1U >= ui) {
                i++;
            } else {
                pui = NULL;
                ui = 2U;
            }
                /* isSynced */
            pui2 = pls->pisSynced;
            ui2 = 2U; /* invalid value sentinel */
            if (NULL != pui2)
                ui2 = *pui2;
            if (1U >= ui2) {
                i++;
            } else {
                pui2 = NULL;
                ui2 = 2U;
            }
                /* syncAccuracy */
            pui3 = pls->psyncAccuracy;
            ui3 = 3600000000U; /* 1 hr.; mostly to tell gcc to STFU */
            if (NULL != pui3) {
                ui3 = *pui3;
                i++;
            } else {
                pui3 = NULL;
            }
            /* timeQuality */
            if (0 != i) {
                n++;
                m = snprintf(p, (size_t)sz, "[timeQuality"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
                /* txKnown */
                if ((NULL != pui) && (1U >= ui)) {
                    m = snul(p, sz, " tzKnown=\"", "\"", (unsigned long)ui, 2, '0', 1, NULL, NULL); /* don't log snul activity (avoid infinite loop) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                /* isSynced */
                if ((NULL != pui2) && (1U >= ui2)) {
                    m = snul(p, sz, " isSynced=\"", "\"", (unsigned long)ui2, 2, '0', 1, NULL, NULL); /* don't log snul activity (avoid infinite loop) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                /* syncAccuracy */
                if ((1U == ui2) && (NULL != pui3)) { /* syncAccuracy is valid only if isSynced if true */
                    m = snul(p, sz, " syncAccuracy=\"", "\"", (unsigned long)ui3, 10, '0', 1, NULL, NULL); /* don't log snul activity (avoid infinite loop) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                m = snprintf(p, (size_t)sz, "]"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
            /* origin */
            i = 0; /* counter for origin elements */
            len = 0U;
                /* ip */
            s = pls->pip;
            if ((NULL != s) && ('\0' != *s)) {
                if ((NULL == strchr(s, '%')) && (NULL == strchr(s, '"'))) {
                    i++;
                    len = strlen(s); /* Flawfinder: ignore */
                } else {
                    s = NULL;
                    len = 0U;
                }
            }
                /* enterpriseId */
            len2 = 0U;
            v = pls->enterpriseId;
            if ((NULL != v) && ('\0' != *v)) {
                len2 = strlen(v); /* Flawfinder: ignore */
                len4 = strspn(v, "0123456789.");
                if (len2 == len4) {
                    i++;
                } else {
                    v = NULL;
                    len2 = 0U;
                }
            }
                /* software */
            len3 = 0U;
            t = pls->software;
            if ((NULL != t) && ('\0' != *t)) {
                if ((NULL == strchr(t, '%')) && (NULL == strchr(t, '"'))) {
                    len3 = strlen(t); /* Flawfinder: ignore */
                    if (48U >= len3) { /* RFC 5424 sect. 7.2.3 */
                        i++;
                    } else {
                        t = NULL;
                        len3 = 0U;
                    }
                }
            }
                /* swVersion */
            len4 = 0U;
            u = pls->swVersion;
            if ((NULL != u) && ('\0' != *u)) {
                if ((NULL == strchr(u, '%')) && (NULL == strchr(u, '"'))) {
                    len4 = strlen(u); /* Flawfinder: ignore */
                    if (32U >= len4) { /* RFC 5424 sect. 7.2.4 */
                        i++;
                    } else {
                        u = NULL;
                        len4 = 0U;
                    }
                }
            }
            /* origin */
            if (0 != i) {
                n++;
                m = snprintf(p, (size_t)sz, "[origin"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
                /* ip */
                if ((NULL != s) && (0U < len)) {
                    do {
                        q = strchr(s, ',');
                        if (NULL != q)
                            *q = '\0';
                        m = (unsigned long)snprintf(p, sz, " ip=\"%s\"", s); /* Flawfinder: ignore (compliant snprintf assumed) */
                        if ((0 <= m) && (m < sz)) {
                            p += m;
                            sz -= m;
                            r += m;
                        }
                        s = q;
                        if (NULL != s)
                            ++s;
                    } while ((NULL != s) && ('\0' != *s));
                }
                /* enterpriseId */
                if ((NULL != v) && (0U < len2)) {
                    m = (unsigned long)snprintf(p, sz, " enterpriseId=\"%s\"", v); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                /* software */
                if ((NULL != t) && (0U < len3)) {
                    m = (unsigned long)snprintf(p, sz, " software=\"%s\"", t); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                /* swVersion */
                if ((NULL != u) && (0U < len4)) {
                    m = (unsigned long)snprintf(p, sz, " swVersion=\"%s\"", u); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                m = (unsigned long)snprintf(p, (size_t)sz, "]"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
            /* meta */
            i = 0; /* counter for meta elements */
                /* sequenceId */
            pui = pls->psequenceId;
            ui = 0U; /* invalid sentinel value */
            if (NULL != pui)
                ui = *pui;
            if (LOGGER_MIN_SEQUENCEID <= ui) {
                if (LOGGER_MAX_SEQUENCEID >= ui) {
                    i++;
                } else {
                    pui = NULL;
                    ui = 0U;
                }
            }
                /* sysUpTime */
            pui2 = pls->psysUpTime;
            if (NULL != pui2) {
                ui2 = *pui2;
                i++;
            }
                /* language */
            s = pls->language;
            if ((NULL != s) && ('\0' != *s)) {
                /* not checked against full RFC 4646 ABNF (!) */
                len = strlen(s); /* Flawfinder: ignore */
                len4 = strspn(s, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-");
                if (len == len4) {
                    i++;
                } else {
                    s = NULL;
                    len = 0U;
                }
            }
            /* meta */
            if (0 != i) {
                n++;
                m = (unsigned long)snprintf(p, (size_t)sz, "[meta"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
                /* sequenceId */
                pui = pls->psequenceId;
                ui = 0U;
                if (NULL != pui)
                    ui = *pui;
                if (LOGGER_MIN_SEQUENCEID <= ui) {
                    if (LOGGER_MAX_SEQUENCEID >= ui) {
                        m = snul(p, sz, " sequenceId=\"", "\"", (unsigned long)ui, 10, '0', 1, NULL, NULL); /* don't log snul activity (avoid infinite loop) */
                        if ((0 <= m) && (m < sz)) {
                            p += m;
                            sz -= m;
                            r += m;
                        }
                    }
                }
                /* sysUpTime */
                if (NULL != pui2) {
                    m = snul(p, sz, " sysUpTime=\"", "\"", (unsigned long)ui, 10, '0', 1, NULL, NULL); /* don't log snul activity (avoid infinite loop) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                /* language */
                if ((NULL != s) && (0U < len)) {
                    m = (unsigned long)snprintf(p, sz, " language=\"%s\"", s); /* Flawfinder: ignore (compliant snprintf assumed) */
                    if ((0 <= m) && (m < sz)) {
                        p += m;
                        sz -= m;
                        r += m;
                    }
                }
                m = (unsigned long)snprintf(p, (size_t)sz, "]"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
            if (0 == n) {
                m = (unsigned long)snprintf(p, (size_t)sz, "-"); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= m) && (m < sz)) {
                    p += m;
                    sz -= m;
                    r += m;
                }
            }
        break;
        case LOGGER_FORMAT_RFC3164:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    /* CONTENT or MSG */
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    /* RFC 5424 SP before MSG unless it looks like MSG has user-defined structured-data */
    switch (pls->format) {
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
            if ((NULL != fmt) && ('\0' != *fmt) && ('[' != *fmt)) {
                *p++ = ' ';
                sz--;
                r++;
            }
        break;
        case LOGGER_FORMAT_RFC3164:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        default:
        break;
    }
    pls->msgoffset = (unsigned)r;
    /* Substitute error message for %m. */
    if ((NULL != fmt) && ('\0' != *fmt)) {
        for (; ('\0' != (ch = *fmt)) && (0 <= sz); ++fmt) {
            if (('%' == ch) && ('m' == fmt[1])) {
                ++fmt;
                r = snprintf(p, (size_t)sz, "%s", strerror(saved_errno)); /* Flawfinder: ignore (compliant snprintf assumed) */
                if ((0 <= r) && (r < (int)sz)) {
                    p += r;
                    sz -= r;
                }
                *p = '\0';
            } else {
                *p++ = ch;
                sz--;
            }
        }
    }
    if (0 >= sz)
        p = fmtbuf + sizeof(fmtbuf) - 1;
    *p = '\0';
    pls->msglen = (size_t)vsnprintf(pls->buf, (size_t)(*(pls->pbufsz)), fmtbuf, ap); /* Flawfinder: ignore (fmtbuf is built here and contains % from supplied format) */
}

static int syslog_send(struct logger_struct *pls)
{
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "syslog_send";
#endif

    if (0 == logger_initialized)
        initialize_logger();
    if ((NULL != pls) && (0U != pls->msglen) && ('\0' != pls->buf[0])) {
        /* check message length */
        if (NULL != pls->pmax_msg_len) {
            int maxlen = *(pls->pmax_msg_len);

            if (1 < maxlen) {
                char c = '\0', *p = pls->buf;
                unsigned int len = pls->msglen;
                int ret;

                /* check message length */
                if (len > 0U + (unsigned)maxlen) {
                    c = p[maxlen];
                    p[maxlen] = '\0';
                }
                ret = sendto(*(pls->psockfd), pls->buf, pls->msglen, 0,
                    (struct sockaddr *)(&(pls->serv_addr)), sizeof(struct sockaddr));
                if (len > 0U + (unsigned)maxlen) {
                    p[maxlen] = c;
                }
                return ret;
#if TESTING
            } else {
                (void)fprintf(stderr,
                    "%s: %s line %d: maxlen %d\n",
                    __func__, source_file, __LINE__,
                    maxlen
                );
#endif
            }
#if TESTING
        } else {
            (void)fprintf(stderr,
                "%s: %s line %d: NULL pmax_msg_len\n",
                __func__, source_file, __LINE__
            );
#endif
        }
    }
    if (0U == pls->msglen) {
#if TESTING
        (void)fprintf(stderr,
            "%s: %s line %d: zero msglen\n",
            __func__, source_file, __LINE__
        );
#endif
        return 0;
    }
    return -1;
}

static void vlogger(int pri, struct logger_struct *pls, register const char *fmt, va_list ap)
{
/* RFC 3164: summary: PRI TIMESTAMP SP HOSTNAME SP TAG CONTENT */
/* RFC 5424 summary: '<'pri'>' VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP STRUCTURED-DATA [SP MSG] */
    struct timespec ts;
    struct civil_time_struct cts;
    unsigned int did=0U, ui, *pui;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "vlogger";
#endif

    if (0 == logger_initialized)
        initialize_logger();
    if (NULL == pls) {
#if TESTING
        (void)fprintf(stderr,
            "%s: %s line %d: NULL pointer\n",
            __func__, source_file, __LINE__
        );
#endif
        return; /* cannot do anything useful */
    }
    /* get time for timestamp */
#if defined(lint)
    ts.tv_sec = (time_t)0;
    ts.tv_nsec = 0L;
#endif
    (void)clock_gettime(CLOCK_REALTIME, &ts);
    (void)localtime_r(&(ts.tv_sec), &(cts.tm));
    cts.fraction = (double)(ts.tv_nsec) / 1.0e9;
    cts.offset = 1.0 * local_utc_offset(-1, NULL, NULL); /* don't log utc_mktime activity (avoid infinite loop) */
    /* Set default facility if none specified. */
    if (0 == (pls->logfac & LOG_FACMASK)) /* shouldn't happen */
        pls->logfac |= LOG_LOCAL7;
    /* check format request */
    switch (pls->format) {
        case LOGGER_FORMAT_PLAIN:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC3164:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_LOCAL:
        /*FALLTHROUGH*/
        case LOGGER_FORMAT_RFC5424_UTC:
        break;
        default:
            pls->format =  LOGGER_FORMAT_RFC5424_UTC;
        break;
    }
    /* Build the message. */
    vlog_string(pri, pls, &cts, fmt, ap);
    /* write to requested locations */
    if (0U != ((LOGGER_LOCATION_SYSLOG_LOCAL | LOGGER_LOCATION_SYSLOG_HOST) & pls->location)) { 
            if (0 != (pls->options & (LOG_PERROR | LOG_CONS))) {
                (void)fprintf(stderr, "%s", pls->buf + pls->tsoffset);
                if ('\n' != pls->buf[pls->msglen-1])
                    (void)fprintf(stderr, "\n");
                fflush(stderr);
                did++;
            } else if ((0U != (LOGGER_LOCATION_STREAM & pls->location)) && (stderr == pls->stream)) {
                (void)fprintf(pls->stream, "%s", pls->buf + pls->tsoffset);
                if ('\n' != pls->buf[pls->msglen-1])
                    (void)fprintf(pls->stream, "\n");
                fflush(pls->stream);
                did++;
            }
            if ((0U != (LOGGER_LOCATION_STREAM & pls->location)) && (stderr != pls->stream)) {
                (void)fprintf(pls->stream, "%s", pls->buf + pls->tsoffset);
                if ('\n' != pls->buf[pls->msglen-1])
                    (void)fprintf(pls->stream, "\n");
                fflush(pls->stream);
                did++;
            }
            /* no trailing control characters for syslog */
            while (('\n' == pls->buf[pls->msglen-1]) || ('\r' == pls->buf[pls->msglen-1]))    /* len can not decrease to zero because of priority string */
                pls->buf[--(pls->msglen)] = '\0';  /* no trailing crud for syslog */
            if (0U != (LOGGER_LOCATION_SYSLOG_LOCAL & pls->location)) {
                /* check message length */
                if (NULL != pls->pmax_msg_len) {
                    int maxlen = *(pls->pmax_msg_len);

                    if (1 < maxlen) {
                        char c = '\0', *p = pls->buf;
                        unsigned int len = pls->msglen;

                        /* check message length */
                        if (len > 0U + (unsigned)maxlen) {
                            c = p[maxlen];
                            p[maxlen] = '\0';
                        }
                        if ((NULL != p) && (0U < len)) {
                            openlog(pls->ident, pls->options, pls->logfac & LOG_FACMASK);
                            (void)setlogmask(pls->logmask);
                            syslog(pri | pls->logfac, "%s", pls->buf); /* RATS: ignore (message length truncated above) */
                            closelog();
                            did++;
                        }
                        if (len > 0U + (unsigned)maxlen) {
                            p[maxlen] = c;
                        }
#if TESTING
                    } else {
                        (void)fprintf(stderr,
                            "%s: %s line %d: maxlen %d\n",
                            __func__, source_file, __LINE__,
                            maxlen
                        );
#endif
                    }
#if TESTING
                } else {
                    (void)fprintf(stderr,
                        "%s: %s line %d: NULL pmax_msg_len\n",
                        __func__, source_file, __LINE__
                    );
#endif
                }
            }
            if (0U != (LOGGER_LOCATION_SYSLOG_HOST & pls->location)) {
                /* send to sysloghost via UDP port 514 */
                if ((0 <= *(pls->psockfd)) && (NULL != pls->sysloghost) && ('\0' != *(pls->sysloghost))) {
                    int r = syslog_send(pls);

                    if (r != (int)(pls->msglen)) {
                        /* write to stderr if syslog_send fails and message hasn't already been written to stderr */
                        if ((0 == (pls->options & (LOG_PERROR | LOG_CONS)))
                        || ((0U == (LOGGER_LOCATION_STREAM & pls->location)) || (stderr != pls->stream))
                        ) {
                            (void)fprintf(stderr, "%s\n", pls->buf + pls->tsoffset);
                            fflush(stderr);
                        }
                    }
                    if (0 < r)
                        did++;
                }
            }
    } else if (0U != (LOGGER_LOCATION_STREAM & pls->location)) {
        (void)fprintf(pls->stream, "%s", pls->buf + pls->tsoffset);
        if ('\n' != pls->buf[pls->msglen-1])
            (void)fprintf(pls->stream, "\n");
        fflush(pls->stream);
        did++;
    }
    if (0U != (LOGGER_LOCATION_SYSLOG_LIKE & pls->location)) {
        if ((NULL != pls->func) && (NULL != pls->pmax_msg_len)) {
            int maxlen = *(pls->pmax_msg_len);

            if (1 < maxlen) {
                char c = '\0', *p = NULL;
                unsigned int len = 0U;

                switch (pls->ext_msg_start) {
                    case LOGGER_MESSAGE_ALL :
                        p = pls->buf;
                        len = pls->msglen;
                    break;
                    case LOGGER_MESSAGE_FROM_TIMESTAMP :
                        p = pls->buf + pls->tsoffset;
                        len = pls->msglen - pls->msgoffset;
                    break;
                    case LOGGER_MESSAGE_MESSAGE_ONLY :
                        p = pls->buf + pls->msgoffset;
                        len = pls->msglen - pls->msgoffset;
                    break;
                    default:
                    break;
                }
                /* check message length */
                if (len > 0U + (unsigned)maxlen) {
                    c = p[maxlen];
                    p[maxlen] = '\0';
                }
                if ((NULL != p) && (0U < len)) {
                    pls->func(pri, "%s", p);
                    did++;
                }
                if (len > 0U + (unsigned)maxlen) {
                    p[maxlen] = c;
                }
#if TESTING
            } else {
                (void)fprintf(stderr,
                    "%s: %s line %d: maxlen %d\n",
                    __func__, source_file, __LINE__,
                    maxlen
                );
#endif
            }
#if TESTING
        } else {
            (void)fprintf(stderr,
                "%s: %s line %d: NULL func or pmax_msg_len\n",
                __func__, source_file, __LINE__
            );
#endif
        }
    }
    if (0U < did) {
        pui = pls->psequenceId;
        if (NULL != pui) {
            ui = *pui;
            if (LOGGER_MIN_SEQUENCEID <= ui) {
                if (LOGGER_MAX_SEQUENCEID <= ui)
                    ui = LOGGER_MIN_SEQUENCEID;
                else
                    ui++;
                *pui = ui;
            }
        }
    }
}

void logger(int pri, void *vp, const char *fmt, ...)
{
    struct logger_struct *p = (struct logger_struct *)vp;
    va_list ap;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "logger";
#endif

    if (0 == logger_initialized)
        initialize_logger();
    if (NULL != p) {
        struct sockaddr_in *pserv_addr = &(p->serv_addr);
        struct sockaddr_in cli_addr;
        int saved_errno;

        saved_errno = errno;
        /* Check priority against setlogmask values. */
        if (0 == (LOG_MASK(pri & LOG_PRIMASK) & p->logmask)) {
#if TESTING
            (void)fprintf(stderr,
                "%s: %s line %d: nothing to log: pri 0x%x, logmask 0x%x\n",
                __func__, source_file, __LINE__,
                (unsigned int)(pri & LOG_PRIMASK), (unsigned int)(p->logmask)
            );
#endif
            return;
        }
        /* prepare to send to syslog host if so desired by caller */
        if (NULL != p->psockfd) {
            char *host = p->sysloghost;
            int fd = *(p->psockfd);

            if (
            (LOGGER_LOCATION_SYSLOG_HOST == (LOGGER_LOCATION_SYSLOG_HOST & p->location)) 
            && (NULL != host) && ('\0' != *host)
            && (0 > fd)
            ) {
                int c, optval, s;
                unsigned long ul;
                struct addrinfo hints, *result=NULL, *rp;

                memset(&hints, 0, sizeof(struct addrinfo));
                hints.ai_family = AF_INET;    /* IPv4 */
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_flags = AI_ADDRCONFIG;
                s = getaddrinfo(host, "514", &hints, &result);
                if (0 != s) {
                    /* avoid infinite loop! */
                    p->sysloghost = NULL;
                    logger(LOG_ERR, p,
                        "%s: %s line %d: getaddrinfo(%s, \"514\", ...): %m",
                        __func__, source_file, __LINE__, host);
                    p->sysloghost = host;
                } else {
                    for (rp = result; NULL != rp; rp = rp->ai_next) {
                        memcpy(pserv_addr, rp->ai_addr, sizeof(struct sockaddr_in)); /* RATS: ignore */
                        ul = ntohl(pserv_addr->sin_addr.s_addr);
                        if (127UL == ((ul & 0xff000000UL)>>24))
                            continue;
                        break;
                    }
                }
                if (NULL != result) freeaddrinfo(result);       /* No longer needed */
                if (-1 == (fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
                    /* avoid infinite loop! */
                    p->sysloghost = NULL;
                    logger(LOG_ERR, p,
                        "%s: %s line %d: socket failed: %m",
                        __func__, source_file, __LINE__);
                    p->sysloghost = host;
                } else {
                    optval = 1;
                    c = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
                    if (0 > c) {
                        /* avoid infinite loop! */
                        p->sysloghost = NULL;
                        logger(LOG_WARNING, p,
                            "%s: %s line %d: setsockopt SO_REUSEADDR failed: %m",
                            __func__, source_file, __LINE__);
                        p->sysloghost = host;
                    }
#ifdef SO_REUSEPORT
                    c = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
                    if (0 > c) {
                        /* avoid infinite loop! */
                        p->sysloghost = NULL;
                        logger(LOG_WARNING, p,
                            "%s: %s line %d: setsockopt SO_REUSEPORT failed: %m",
                            __func__, source_file, __LINE__);
                        p->sysloghost = host;
                    }
#endif
                    memset((char *)&cli_addr, 0, sizeof(cli_addr));
                    cli_addr.sin_family = (sa_family_t) (AF_INET);
                    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                    cli_addr.sin_port = htons(0);
                    if (bind(fd, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in))==-1) {
                        /* avoid infinite loop! */
                        p->sysloghost = NULL;
                        logger(LOG_ERR, p,
                            "%s: %s line %d: bind failed: %m",
                            __func__, source_file, __LINE__);
                        p->sysloghost = host;
                        close(fd);
                        fd = -1;
                    }
                }
                if (0 <= fd)
                    *(p->psockfd) = fd;
            }
        }
        switch (p->format) {
            case LOGGER_FORMAT_PLAIN:
            /*FALLTHROUGH*/
            case LOGGER_FORMAT_RFC3164:
            break;
            case LOGGER_FORMAT_RFC5424_LOCAL:
            /*FALLTHROUGH*/
            case LOGGER_FORMAT_RFC5424_UTC:
            break;
            default:
                p->format =  LOGGER_FORMAT_RFC5424_UTC;
            break;
        }
        errno = saved_errno;
#if TESTING
    } else {
        (void)fprintf(stderr,
            "%s: %s line %d: NULL pointer\n",
            __func__, source_file, __LINE__
        );
#endif
    }
    pri &= LOG_PRIMASK; /* ignore facility */
    va_start(ap, fmt);
    vlogger(pri, p, fmt, ap);
    va_end(ap);
}

/* main entry point for testing follows */
#if TESTING
static void syslog_like(int unused, const char *fmt, ...)
{
    char buf[65536];            /* RATS: ignore (OK for testing) */
    va_list ap;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "syslog_like";
#endif

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, ap); /* Flawfinder: ignore (buf is large enough; fmt is pre-built) */
    va_end(ap);
    (void)printf("%s: %s\n", __func__, buf);
}

int main(int unused, char *argv[])
{
    char host[MAXHOSTNAMELEN];  /* RATS: ignore (big enough) */
    char iplist[1024];          /* RATS: ignore (size is checked) */
    char logbuf[65536];         /* RATS: ignore (it's big enough) */
    char loghost[MAXHOSTNAMELEN] = "localhost";  /* RATS: ignore (big enough) */
    char procid[32];            /* RATS: ignore (used with sizeof()) */
    char *p, *prog = NULL;
    int bufsz;
    int maxlen = 1024;
    int sockfd = -1;
    unsigned int seqid = 1U;
    unsigned int tzknown = 1U;
    pid_t pid;
    void *log_arg;
    struct logger_struct ls;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "main";
#endif

    /* I/O initialization */
    (void)setvbuf(stdout, NULL, _IOLBF, 0);
    /* variable initialization */
    host[0] = '\0';
    iplist[0] = '\0';
    procid[0] = '\0';
    prog = argv[0];
    p = strrchr(prog, '/');
    if (NULL == p)
        p = strrchr(prog, '\\');
    if (NULL != p)
        prog = p + 1;
    if (0 == logger_initialized)
        initialize_logger();
    pid = getpid();
    (void)snul(procid, sizeof(procid), NULL, NULL, (unsigned long)pid, 10, '0', 1, NULL, NULL); /* don't log snul activity (avoid infinite loop: logger not yet set up) */
    /* initial logging configuration */
    memset(&ls, 0, sizeof(struct logger_struct));
    ls.options = LOG_PID /* | LOG_PERROR | LOG_CONS */ ;
    ls.logfac = LOG_DAEMON ;
    ls.logmask = LOG_UPTO(LOG_INFO) ;
    ls.location = LOGGER_LOCATION_STREAM | LOGGER_LOCATION_SYSLOG_HOST | LOGGER_LOCATION_SYSLOG_LIKE ;
    ls.format = LOGGER_FORMAT_RFC5424_UTC ;
    ls.ext_msg_start = LOGGER_MESSAGE_FROM_TIMESTAMP ;
    ls.psysUpTime = NULL;
    ls.psyncAccuracy = NULL;
    ls.ptzKnown = &tzknown;
    ls.pisSynced = NULL;
    ls.stream = stderr;
    ls.sysloghost = loghost;
    ls.fqdn = host;             /* filled in below (uses logger for error reporting) */
    ls.ident = prog;
    ls.msgid = "test";
    ls.procid = procid;         /* filled in below (uses logger for error reporting) */
    ls.enterpriseId = NULL;
    ls.software = "logtest";
    ls.swVersion = "2.16";
    ls.language = NULL;
    ls.pip = iplist;            /* filled in below (uses logger for error reporting) */
    ls.func = syslog_like;
    ls.pmax_msg_len = &maxlen;
    bufsz = sizeof(logbuf);
    ls.pbufsz = &bufsz;
    ls.buf = logbuf;
    ls.psockfd = &sockfd;
    ls.psequenceId = &seqid;
    log_arg = &ls;
    /* fully-qualified host name */
    p = get_host_name(host, sizeof(host), logger, log_arg);
    ls.fqdn = host;
    /* host address(es) */
    (void)get_ips(iplist, sizeof(iplist), host, AF_UNSPEC, AI_ADDRCONFIG, logger, log_arg);
    (void)snul(procid, sizeof(procid), NULL, NULL, (unsigned long)pid, 10, '0', 1, logger, log_arg); /* logging of snn OK from logger caller after initialization */
    /* process the command line arguments */
    (void)printf("%s: %s line %d: %s[%d] host %s, address(es) %s\n", __func__, source_file, __LINE__, prog, pid, host, iplist);
    ls.format = LOGGER_FORMAT_RFC3164;
    logger(LOG_INFO, log_arg,
        "%s: %s line %d: RFC 3164 format test",
        __func__, source_file, __LINE__);
    ls.format = LOGGER_FORMAT_RFC5424_LOCAL;
    logger(LOG_INFO, log_arg,
        "%s: %s line %d: RFC 5424 local time format test",
        __func__, source_file, __LINE__);
    ls.format = LOGGER_FORMAT_RFC5424_UTC;
    logger(LOG_INFO, log_arg,
        "%s: %s line %d: RFC 5424 UTC time format test",
        __func__, source_file, __LINE__);
    if ((0 < ls.msgoffset) && (ls.tsoffset <= ls.msgoffset) && (ls.msgoffset < ls.msglen))
        printf("log buffer contains \"%s\"\n", ls.buf + ls.msgoffset);
    ls.format = LOGGER_FORMAT_PLAIN;
    logger(LOG_INFO, log_arg,
        "%s: %s line %d: plain format test",
        __func__, source_file, __LINE__);
    return 0;
}
#endif
