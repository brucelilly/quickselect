/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for simple_config query program
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    simple_config.c copyright 2016 Bruce Lilly.   \ simple_config.c $
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
/* $Id: ~|^` @(#)   This is simple_config.c version 1.5 dated 2016-12-03T15:29:05Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "simple_config" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/src/s.simple_config.c */

/********************** Long description and rationale: ***********************
The simple_config program provides a means to query a configuration file for
existence and/or value of a specified key, which is supplied as a series of
command-line arguments.
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
#define ID_STRING_PREFIX "$Id: simple_config.c ~|^` @(#)"
#define SOURCE_MODULE "simple_config.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2016-12-03T15:29:05Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016"

/* configuration (which might affect feature test macros) */

/* feature test macros must appear before any header file inclusion */
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

/* prevent errors from nonstandard BSD types used in standard header files */
#if ! _BSD_SOURCE
# undef _BSD_SOURCE
# define _BSD_SOURCE 1
#endif
#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE
#endif

/*INDENT ON*/
/* *INDENT-ON* */

#include <sys/types.h>          /* *_t */

/* this is a main program source file */
#define INCLUDE_EXTERN_DECLARATION 1

/* local header files needed */
#include "get_host_name.h"      /* get_host_name */
#include "get_ips.h"            /* get_ips */
#include "logger.h"             /* logger */
#include "paths_decl.h"         /* path_basename */
#include "re_config.h"          /* re_config find_config_entry */
#include "snn.h"                /* snul */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#define	optind	EFFIN_GCC_NONSENSE      /* work around gcc silliness `;-\ */
#include <sys/param.h>          /* MAXHOSTNAMELEN */
#include <errno.h>              /* errno */
#include <limits.h>             /* *_MAX */
#include <regex.h>              /* REG_* */
#include <stddef.h>             /* NULL */
#include <stdio.h>              /* FILE printf fprintf */
#include <string.h>             /* memset strcmp strchr */
#include <syslog.h>             /* LOG_* */
#include <unistd.h>             /* getpid */

/* local macros and structure definitions */
#undef	optind
#define	OPTSTRING	"df:hv"
#define	USAGE		"[options] names\noptions:\n\
	[-d]\tturn on or increase debugging\n\
	[-f path]\tuse configuration file at path specified\n\
	[-h]\tprint this help message\n\
	[-v]\tprint version and exit\n\
\n\
"

/* static data and function definitions */
static char simple_config_version_string[] = "$Id: ~|^` @(#) simple_config version 1.5\\ $";
static char simple_config_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *simple_config_version = NULL;
static const char *source_file = NULL;

#if 0
    /* regexp pattern for simple configuration file */
static const char *pat = "^[ 	]*([^ 	#]*)[ 	]*([^ 	#]*[^#]*[^ 	#]+)*[ 	]*[#]*.*$";
#else
    /* regexp pattern for simple configuration file plus partial libconfuse configuration files */
static const char *pat = "^[ 	{]*([^ 	#={]*)[ 	=\"{]*([^ 	#{}]*[^#{}]*[^ 	#\"{}]+)*[ 	\"{}]*[#]*.*$";
#endif
static int cflags = REG_EXTENDED;
static int eflags = 0;

static void initialize_simple_config(void)
{
    unsigned char *p;

    simple_config_version = (const char *)simple_config_version_string;
    simple_config_initialized
        = register_build_strings(simple_config_version, &source_file, filenamebuf);
    simple_config_version += 15; /* skip over rcs/sccs magic */
    /* elide trailing cruft used by rcs 'ident' at run-time */
    p = (unsigned char *)strchr((char *)simple_config_version_string, '\\');
    if (NULL != p)
        *p = '\0';
}

int main(int argc, const char **argv)
{
    char cf[PATH_MAX + 1];      /* RATS: ignore (it's big enough) */
    char prog[PATH_MAX + 1];    /* RATS: ignore (it's big enough) */
    char host[256];             /* RATS: ignore (size is checked) */
    char iplist[1024];          /* RATS: ignore (size is checked) */
    char logbuf[65536];         /* RATS: ignore (it's big enough) */
    char loghost[MAXHOSTNAMELEN]="localhost";  /* RATS: ignore (big enough) */
    char procid[32];            /* RATS: ignore (used with sizeof()) */
    int bufsz, c, debug=0, errcount=0, maxlen=1024, optind, sockfd=-1;
    unsigned int seqid=1U, tzknown=1U;
    const char *p;
    pid_t pid;
    void *log_arg;
    struct config_entry *config_root = NULL, *temp;
    struct logger_struct ls;

    /* initialization */
    cf[0] = '\0';
    host[0] = '\0';
    iplist[0] = '\0';
    procid[0] = '\0';
    if ((char)0 == simple_config_initialized)
        initialize_simple_config();
    (void)setvbuf(stdout, NULL, _IOLBF, 0);
    pid = getpid();
    /* process the command line arguments */
    (void)path_basename(argv[0], prog, sizeof(prog));
    /* initial logging configuration */
    memset(&ls, 0, sizeof(struct logger_struct));
    ls.options = LOG_PID /* | LOG_PERROR | LOG_CONS */ ;
    ls.logfac = LOG_DAEMON;
    ls.logmask = LOG_UPTO(LOG_INFO);
    ls.location = LOGGER_LOCATION_STREAM | LOGGER_LOCATION_SYSLOG_HOST ;
    ls.format = LOGGER_FORMAT_RFC5424_UTC;
    ls.ext_msg_start = LOGGER_MESSAGE_ALL;
    ls.psysUpTime = NULL;
    ls.psyncAccuracy = NULL;
    ls.ptzKnown = &tzknown;
    ls.pisSynced = NULL;
    ls.stream = stderr;
    ls.sysloghost = loghost;
    ls.fqdn = host;             /* filled in below (uses logger for error reporting) */
    ls.ident = prog;
    ls.msgid = "simple_config";
    ls.procid = procid;         /* filled in below (uses logger for error reporting) */
    ls.enterpriseId = NULL;
    ls.software = "simple_config main";
    ls.swVersion = "1.5";       /* maintained by version control system */
    ls.language = NULL;
    ls.pip = iplist;            /* filled in below (uses logger for error reporting) */
    ls.func = NULL;
    ls.pmax_msg_len = &maxlen;
    bufsz = sizeof(logbuf);
    ls.pbufsz = &bufsz;
    ls.buf = logbuf;
    ls.psockfd = &sockfd;
    ls.psequenceId = &seqid;
    log_arg = &ls;
    p = get_host_name(host, sizeof(host), logger, log_arg);
    ls.fqdn = host;
    /* host address(es) */
    (void)get_ips(iplist, sizeof(iplist), host, AF_UNSPEC, AI_ADDRCONFIG, logger, log_arg);
    (void)snul(procid, sizeof(procid), NULL, NULL, (unsigned long)pid, 10, '0', 1, logger, log_arg); /* logging of snn OK from logger caller after initialization */
    /*
       process command-line arguments
       return early for -h or -v
       track position number of first non-option argument (1st name)
       return 1 if option error
    */
    /* emulate getopt (too many implementation differences to rely on it) */
    for (optind = 1; (optind < argc) && (argv[optind][0] == '-'); optind++) {
        if (!strcmp(argv[optind], "--")) {
            optind++;
            break;
        }
        for (p = argv[optind] + 1; (c = *p) != '\0'; p++) {
            if ((c == ':') || !strchr(OPTSTRING, c))
                c = '?';
            switch (c) {
                case 'd':
                    debug++;
                break;
                case 'f':
                    if (*(++p) == '\0')
                        p = argv[++optind];
                    (void)strncpy(cf, p, PATH_MAX);
                    for (; *p != '\0'; p++) ;   /* pass over arg to satisfy loop conditions */
                    p--;
                break;
                case 'h':
                    (void)fprintf(stdout, USAGE);
                return 0;
                case 'v':
                    (void)fprintf(stdout, "%s\n", simple_config_version);
                return 0;
                break;
                default:
                    errcount++;
                break;
            }
        }
    }
    if (errcount) {
        (void)fprintf(stderr, "%s: usage: %s ", prog, prog);
        (void)fputs(USAGE, stderr);
        return 1;
    }
    if (0 < debug)
        (void)fprintf(stderr, "%s: end of options: optind = %d, argc = %d\n", prog, optind, argc);
    /*
       try to read configuration file
          specified via -f command-line option argument
          stdin if not specified or if -f option argument is '-'
       return 2 if no file
       return 3 if empty file (no configuration keys)
    */
    if ('\0' == cf[0])
        strcpy(cf, "/dev/stdin"); /* XXX maybe modify lib/libconfig/src/re_config.c to accept "-" for path */
    if (0 < debug)
        (void)fprintf(stderr, "%s: configuration file \"%s\"\n", prog, cf);
    /*
       load configuration into memory
       return 4 if there's an error
    */
    config_root = re_config(cf, config_root, pat, cflags, eflags, logger, log_arg);
    if (NULL == config_root)
        return 4;
    /*
       search for key matching supplied names
       return 5 if no key specified
       if the key is found
          print the key value (if any) to stdout
       return 6 if the key is not found
    */
    temp = find_config_entry(config_root, argv+optind, argc-optind, configcmp, logger, log_arg);
    if (NULL == temp) {
        c = free_config(config_root, logger, log_arg);
        return 6;
    } else {
        if (NULL != temp->value)
            (void)printf("%s\n", temp->value);
    }
    c = free_config(config_root, logger, log_arg);
    return 0;
}
