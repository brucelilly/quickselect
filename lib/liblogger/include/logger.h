/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring functions defined in log.c
*/
#ifndef	LOGGER_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)  logger.h copyright 2010 - 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is logger.h version 2.6 2015-10-09T01:00:21Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "logger" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/liblogger/include/s.logger.h */

/* version-controlled header file version information */
#define LOGGER_H_VERSION "logger.h 2.6 2015-10-09T01:00:21Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* system header files needed for declarations */
#include <sys/types.h>          /* pid_t */
#include <netinet/in.h>         /* struct sockaddr_in */
#include <stdio.h>              /* FILE */
#include <syslog.h>             /* LOG_* */

/* macros used by code */
/* sometimes LOG_PERROR is not defined */
#ifndef LOG_PERROR
# define LOG_PERROR 0x20
#endif
/* formatting options */
#define LOGGER_FORMAT_PLAIN             1U
#define LOGGER_FORMAT_RFC3164           3164U
#define LOGGER_FORMAT_RFC5424_LOCAL     54240U
#define LOGGER_FORMAT_RFC5424_UTC       5424U
/* sequenceId limits */
#define LOGGER_MIN_SEQUENCEID 1U               /* per RFC 5424 sect. 7.3.1 */
#define LOGGER_MAX_SEQUENCEID 2147483547U      /* per RFC 5424 sect. 7.3.1 */
/* logging location (bitmap) */
#define LOGGER_LOCATION_NOWHERE      0x00U
#define LOGGER_LOCATION_STREAM       0x01U
#define LOGGER_LOCATION_SYSLOG_LOCAL 0x02U
#define LOGGER_LOCATION_SYSLOG_HOST  0x04U
#define LOGGER_LOCATION_SYSLOG_LIKE  0x08U
/* start of message passed to external syslog-like function */
#define LOGGER_MESSAGE_ALL            0x0U
#define LOGGER_MESSAGE_FROM_TIMESTAMP 0x1U
#define LOGGER_MESSAGE_MESSAGE_ONLY   0x2U

/* structures used by code */
struct logger_struct {
    /* logger control data held in this structure,
       not modified by logger
       initialized by caller before first call to logger;
       (may be (but typically is not) modified by caller between calls)
    */
    int options;
    int logfac;
    int logmask;
    unsigned int location;      /* LOGGER_LOCATION_* defined above */
    unsigned int format;        /* LOGGER_FORMAT_* defined above */
    unsigned int ext_msg_start; /* LOGGER_MESSAGE_* defined above */
    /* pointers to caller-held data used by logger,
       not modified by logger
       caller can set to a NULL pointer to ignore
       otherwise, caller must ensure data is up-to-date before each logger call
    */
    unsigned int *psysUpTime;   /* hundredths of a second RFC 5424 sect. 7.3.2 */ /* RFC 2579 SYNTAX INTEGER (0..2147483647) */
    unsigned int *psyncAccuracy;/* RFC 5424 timeQuality */ /* microseconds (unsigned) */
    unsigned int *ptzKnown;     /* RFC 5424 timeQuality */
    unsigned int *pisSynced;    /* RFC 5424 timeQuality */
    /* pointers to caller-held data used read-only by logger,
       initialized by caller before first call to logger;
       not modified by logger
       (may be (but typically is not) modified by caller between calls)
    */
    FILE *stream;               /* for stream logging */
    const char *fqdn;           /* this host fully-qualified domain name */
    const char *ident;          /* RFC 3164 TAG, RFC 5424 APP-NAME */
    const char *msgid;          /* RFC 5424 MSGID */
    const char *procid;         /* RFC 5424 PROCID */
    const char *enterpriseId;   /* RFC 5424 origin enterpriseId (may contain '.'-separated numbers) */
    const char *software;       /* RFC 5424 origin software */
    const char *swVersion;      /* RFC 5424 origin swVersion */
    const char *language;       /* RFC 5424 sect. 7.3.3 */
    void (*func)(int, const char *, ...); /* syslog-like external function */
    int *pmax_msg_len;          /* maximum message length for syslog[-like function] */
    int *pbufsz;                /* size of array pointed to by buf (q.v.) */
    /* pointers to caller-held data which is used (content modified) by logger
       (pointer may be (but typically is not) modified by caller between calls)
    */
    char *buf;                  /* message buffer; see *pbufsz (buffer size) */
    char *pip;                  /* sender IP list (e.g. for RFC 5424 origin ip=) *//* logger modifies comma separators */
    /* pointers to caller-held data used by logger
       may be modified by logger
       (caller should not change after initialization)
       caller initialization:
          sockfd to -1
          sequenceId to 0U or 1U except in unusual circumstances
             0U causes sequenceId to be omitted
          sysloghost to NULL or to character array holding host name for remote syslog
    */
    int *psockfd;               /* syslog receiver */
    unsigned int *psequenceId;  /* RFC 5424 meta sequenceId */
    char *sysloghost;           /* syslog receiver host */
    /* logger internal data which should not be modified except by logger itself
       caller need not initialize (but harmless to do so)
       caller may read, but must not modify
    */
    struct sockaddr_in serv_addr;       /* syslog receiver */
    unsigned int tsoffset;      /* start of timestamp relative to buf */
    unsigned int msgoffset;     /* start of non-timestamped message relative to buf */
    size_t msglen;              /* total length of message in buf */
};

/* logger.c function declarations */
#if defined(__cplusplus)
# define LOGGER_EXTERN extern "C"
#else
# define LOGGER_EXTERN extern
#endif

/* *INDENT-OFF* */
/*INDENT OFF*/

LOGGER_EXTERN void logger(int, void *, const char *, ...);

#define	LOGGER_H_INCLUDED
#endif
