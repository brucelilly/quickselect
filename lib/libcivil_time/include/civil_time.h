/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for civil time text parsing and generation
*/
#ifndef	CIVIL_TIME_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)  civil_time.h copyright 2010 - 2016 Bruce Lilly.   \ civil_time.h $
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
/* You may send bug reports to bruce.lilly@gmail.com with subject "civil_time" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libcivil_time/include/s.civil_time.h */

/* version-controlled header file version information */
#define CIVIL_TIME_H_VERSION "civil_time.h 2.9 2016-07-03T00:57:48Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <stddef.h>             /* NULL */
#include <time.h>               /* struct tm */

/* macros used by code */
/* limits */
#define CIVIL_TIME_YEAR_MIN -9999
#define CIVIL_TIME_YEAR_MAX 9999

#ifndef CIVIL_TIME_DO_WDAY
# define CIVIL_TIME_DO_WDAY 1   /* update (or not) tm_wday field in struct tm */
#endif

/* bits for keeping track of elements */
#define CIVIL_TIME_YEAR  0x0001U
#define CIVIL_TIME_MONTH 0x0002U
#define CIVIL_TIME_MDAY  0x0004U
/* YDAY bit must be more significant than YEAR, MONTH, and MDAY */
#define CIVIL_TIME_YDAY  0x0008U
#define CIVIL_TIME_HOUR  0x0010U
#define CIVIL_TIME_MIN   0x0020U
#define CIVIL_TIME_SEC   0x0040U
#define CIVIL_TIME_FRAC  0x0080U
#define CIVIL_TIME_OFFS  0x0100U
#if CIVIL_TIME_DO_WDAY
/* WDAY bit must be more significant than YDAY */
# define CIVIL_TIME_WDAY 0x0200U
# define CIVIL_TIME_MAX   CIVIL_TIME_WDAY
#else
# define CIVIL_TIME_MAX   CIVIL_TIME_OFFS
#endif

/* structures used by code */
struct civil_time_struct {
    struct tm tm;     /* local date-time year, ..., seconds */
    double fraction;  /* 0.0..0.999... second */
    double offset;    /* seconds East of UTC */
};

/* function prototypes */
#if defined(__cplusplus)
# define CIVIL_TIME_EXTERN extern "C"
#else
# define CIVIL_TIME_EXTERN extern
#endif

/* *INDENT-OFF* */
/*INDENT OFF*/

/* civil_time.c function declarations */
CIVIL_TIME_EXTERN int parse_civil_time_text(const char *, struct civil_time_struct *, char **, int, const struct civil_time_struct *, void (*)(int, void *, const char *, ...), void *);
CIVIL_TIME_EXTERN int civil_time_cmp(const struct civil_time_struct *, const struct civil_time_struct *);
CIVIL_TIME_EXTERN double civil_time_difference(const struct civil_time_struct *, const struct civil_time_struct *, void (*)(int, void *, const char *, ...), void *);

/* civil_time_normalize.c */
CIVIL_TIME_EXTERN void civil_time_normalize(struct civil_time_struct *, void (*)(int, void *, const char *, ...), void *);
CIVIL_TIME_EXTERN void civil_time_zone_change(struct civil_time_struct *, double, void (*)(int, void *, const char *, ...), void *);

/* civil_time_output.c */
CIVIL_TIME_EXTERN int sn_date_time(char *, int, const struct civil_time_struct *, int, const char *, const char *, const char *, const char *, const char *, int, int, int, void (*)(int, void *, const char *, ...), void *);

/* inline functions */

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
void civil_time_cpy(const struct civil_time_struct *pcts1, struct civil_time_struct *pcts2,
    void (*unused)(int, void *, const char *, ...), void *also_unused)
{

    if ((NULL == pcts1) || (NULL == pcts2)) { return; }
    pcts2->tm = pcts1->tm;
    pcts2->fraction = pcts1->fraction;
    pcts2->offset = pcts1->offset;
}

/* macros (because more than a decade and a half later, c99 still doesn't always work...) */
#define sn_civil_time(buf,sz,pcts,precision,do_utc,do_yday,f,log_arg) sn_date_time(buf,sz,pcts,4,"-","T",":","Z",":",precision,do_utc,do_yday,f,log_arg)

#define CIVIL_TIME_H_INCLUDED
#endif
