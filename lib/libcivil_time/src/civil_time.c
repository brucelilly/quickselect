/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for civil_time functions
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    civil_time.c copyright 2009-2017 Bruce Lilly.   \ civil_time.c $
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
/* $Id: ~|^` @(#)   This is civil_time.c version 2.10 dated 2017-02-11T04:39:36Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "civil_time" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libcivil_time/src/s.civil_time.c */

/********************** Long description and rationale: ***********************
*  It is sometimes desirable to read or write date-time in a standard format
*  and to represent date-time in a manner which:
*   a. is not subject to machine word size limits in the near future
*   b. preserves local time offset from UTC information, where that information
*      is useful
*   c. has sufficient resolution for the foreseeable future.
*   d. properly handles all relevant aspects of date-time, including leap
*      seconds
*
* The functions provided by this file are intended to provide means to convert
* between a flexible machine representation of date-time and standardized
* text streams for interchange of date-time information (logs, data transfer
* which is independent of machine word size, byte-ordering, endianness, etc.).
*
* The need for the funtionality provided by this file arises from limitations
* of generally-available means of representing date-time and time intervals.
*
* POSIX time_t has poorer than one second resolution (it cannot handle leap
* seconds) and many 32-bit systems will soon run into a barrier with time_t.
* Resolution has been supplemented first to millisecond resolution between
* seconds by coupling time_t with a count of milliseconds in struct timeb,
* then to microsecond resolution between seconds by coupling time_t with a
* count of microseconds in struct timeval, then similarly supplemented to
* nanoseconds in struct timespec.  None of those structures address either the
* 32-bit time_t barrier issue or the leap second issue.  Each of those
* structures holds a fixed-resolution fractional second component;
* applications can only extend resolution via a rewrite and recompile, and
* that only after an extended resolution structure is defined and has had
* time to be incorporated into a reasonable number of target platforms.
*
* struct tm has integer representation for date components providing one
* second resolution (including leap seconds). As the year, month, day, etc.
* are represented as integers (with varying offsets), the structure will
* not itself pose any problems in the foreseeable future; it can handle years
* well beyond the limits of 4-digit-year text formats.  struct tm is used for
* part of the date-time information processed by the functions in this file.
* Standardized struct tm does not itself provide any indication of whether the
* date-time that it represents is local time or UTC time.  The functions in
* this file always use struct tm as local time and provide a separate mechanism
* for indicating local time offset from UTC.
*
* The ANSI/ISO C and POSIX ctime package has a volatile, non-thread-safe
* variable to hold part of the information about offset of local time of the
* execution system relative to UTC, but it is incomplete, and is not useful
* for representation of other date-time value offsets (i.e. those associated
* with objects other than the execution system itself).  The incompleteness
* consists of the inability to represent some historical local time offsets
* from UTC, some of which include fractions of a second.
*
* The functions provided by this file use a combination of three variables
* to represent date-time and offset from UTC with a resolution approaching
* femtosecond resolution on most systems.  The types used are:
*  1. struct tm holds local date and time with one second resolution and can
*     represent leap seconds
*  2. a double-precision floating point variable holds fractions of a second
*  3. another double-precision floating point variable holds the number of
*     seconds (including fractions of a second) offset between local time
*     and UTC (no standardized text format can represent offsets to better
*     than one minute resolution, although historically some zones had offsets
*     specified to fractions of a second resolution; no current time zone
*     requires better than five minute resolution).  The available resolution
*     and precision may be useful in accurately representing historical events
*     in machine-representable form.
*
* There are a number of standardized date-time text formats, and many others
* which are not formally standardized; it would be impractical to try to support
* all of them.  The following criteria were considered in selecting supported
* formats:
*  1. The format should be specified by a finished published standard or
*     proposed standard, not a work in progress; the published standard should
*     not have contradictory versions.
*  2. The standard document(s) describing the format should be freely available
*     to support review of and contribution to the implementation without
*     hindrance or unreasonable expense.
*  3. The standard and its standards development organization (SDO) should
*     neither preclude implementation nor utilization of the standard for the
*     purpose of reviewing implementation correctness.
*  4. Implementations should not be subject to royalties or other encumbrances
*     or threats of the same from the SDO or others.
*  5. The standard should not be unclear or ambiguous or have poorly-defined
*     semantics.
*  6. The format should be self-contained and unambiguous; it should not require
*     external context or out-of-band information or "mutual agreement".
*  7. The format should not include obscure or frivolous features, or features
*     which are onerous or impractical to implement.
*  8. The formats should have reasonable characteristics; e.g. date-times
*     expressed in a common format with a common time zone and with the same
*     resolution should sort in chronological order using the text representation
*     with the system sort utility.
*
* In consideration of these criteria, the following text formats have been
* identified for bidirectional conversion (parsing text into a set of the three
* data variables mentioned above, and production of text from such a data set):
*  1. CCSDS 301.0-B-4 section 3.5 type A and B formats
*     http://public.ccsds.org/publications/archive/301x0b4e1.pdf
*  2. EDSC EX000013.1
*     http://www.exchangenetwork.net/standards/Rep_Date_Time_01_06_2006_Final.pdf
*  3. RFC 3339
*     https://www.ietf.org/rfc/rfc3339.txt
*
* It should be noted (as is here repeated) that none of these text formats can
* accurately represent historical local date-time where the local offset is not
* an integral number of minutes from UTC.  RFC 3339 in particular asserts that
* in such cases "the closest representable UTC offset" might be used, but that
* fails to account for the historical local zone offsets in Belgium and Liberia,
* which at times had offsets equidistant between two minutes (viz. including 30
* seconds of offset), i.e. there is no "closest representable UTC offset".
*
* The three formats referenced above themselves reference another set of
* specifications (which fail seven of the eight criteria noted above).  In turn,
* several additional specifications reference the above, e.g. RFC 5424 (syslog
* protocol) references RFC 3339 with added restrictions.
*
* Several text formats support missing or optional date-time components; the
* parse function has three options to fill in missing components:
*  1. Missing components can be set to minimum legal values (e.g. January,
*     first day of month, 0 hours/minutes/seconds/fraction)
*  2. Missing components can be set to maximum legal values (e.g. December,
*     last day of month, hour 23, minute 59, etc.)
*  3. Missing components can be copied from a reference data set.
*
* Generation of text is controllable by a few options:
*  1. Date-time can be converted to UTC for text generation, or can be
*     expressed as local time with offset from UTC.
*  2. The date portion can be expressed as year-month-day or as year-yday.
*  3. Fractional seconds can be suppressed or presented with desired fixed
*     resolution, or with enough resolution to represent the fractional part
*     given (e.g. 1/4 second as .25, not .250000000) with an upper bound
*     placed on the number of digits of resolution.
* There are some differences in what the supported text format standards permit:
* due care should be exercised in selecting options for text generation for
* specific applications; e.g. note the additional restrictions imposed by
* RFC 5424 on the use of RFC 3339 format.  Note also that some text standards
* specify use of UTC only.
*
* It is sometimes necessary to compare two date-times (which may represent local
* times in different time zones).  It is also sometimes useful to copy a
* date-time representation or to translate a local date-time expressed in one
* time zone to the corresponding (local) date-time in another zone.  And it is
* frequently useful to compute the time difference between two date-times,
* taking leap seconds and time zone differences into account.  This file also
* includes functions to perform those actions.
*
* Some actions can be conveniently performed by arithmetic on struct tm
* components or other variables used to represent civil_time which denormalize
* the structure (e.g. by specifying negative minutes or minutes greater than
* 60). A function is provided to normalize the civil_time_struct structure's
* components.
*
* The parse function calling synopsis is:
int parse_civil_time_text(const char *s, struct civil_time_struct *pcts_result,
    char **ppe, int which, struct civil_time_struct *pcts_ref);
* Arguments:
*   s           points to the string to be parsed
*   pcts_result points to the data set which will hold the result
*   ppe         points to the character in s beyond the end of parsing
*   which       determines how missing elements are filled:
*               <0 set element values to minimum legal values
*               >0 set element values to maximum legal values
*               ==0 set element values from the corresponding reference
*                   data set elements
*   pcts_ref    points to the reference data set
* Return value:
*   0 on success
*   -1 on failure with errno set:
*      EINVAL if some result pointer is NULL
*
* The data structure comparison function calling synopsis is:
int civil_time_cmp(const struct civil_time_struct *pcts1, const struct civil_time_struct *pcts2);
* Arguments:
*     pcts1,pcts2  pointers to the structures which hold the local date-time
*                  value to sub-second resolution
*                  tm_wday, tm_isdst components are ignored
*                  fraction of a second time; range 0.0 to < 1.0
*                  offset of local time zone in seconds (and fraction if
*                     needed) East of the UTC Prime Meridian
*                  component values are not altered by this function
* Compares the two date-time structures (which may be in different zones).
*  The two compare as equal if the equivalent UTC date-times are the same
*  to within the smallest representable fraction of a second.
*   Calls:
*              civil_time_cpy
*                  makes local copies for comparison
*              civil_time_zone_change
*                  change to UTC for proper comparison
* Return value:
*   zero for equality, negative if pcts1 points to an earlier date-time than
*   pcts2, positive of pcts1 points to a later date-time than pcts2.
*
* The time difference function calling synopsis is:
double civil_time_difference(const struct civil_time_struct *pcts1, const struct civil_time_struct *pcts2);
* Arguments:
*     pcts1,pcts2  pointers to the structures which hold the local date-time
*                  value to sub-second resolution
*                  tm_wday, tm_isdst components are ignored
*                  fraction of a second time; range 0.0 to < 1.0
*                  offset of local time zone in seconds (and fraction if
*                     needed) East of the UTC Prime Meridian
*                  component values are not altered by this function
* Computes the time difference in seconds between the to date-times, including
*  leap seconds.  The result is positive if pcts1 points to a later date-time
*  than pcts2.
*   Calls:
*              civil_time_cpy
*                  makes local copies for comparison
*              civil_time_zone_change
*                  change to UTC for proper comparison
*              leap_seconds_between (leap_second.c)
*              years_since_epoch (years_since_epoch.c)
* Return value:
*   0.0 with errno set to EINVAL for invalid arguments.
*   The number of seconds time difference between pcts1 and pcts2, including
*     leap seconds and fractions.
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: civil_time.c ~|^` @(#)"
#define SOURCE_MODULE "civil_time.c"
#define MODULE_VERSION "2.10"
#define MODULE_DATE "2017-02-11T04:39:36Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2009-2017"

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 600
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* library header files needed */
#include "civil_time.h"      /* header file for public definitions and declarations */ /* includes time.h */
#include "leap_second.h"        /* leap_second leap_seconds_between */
#include "snn.h"                /* sn1en [snf snul] */
#include "strntol.h"            /* nstrtol */ /* includes sys/types.h */
#include "utc_mktime.h"         /* inverse_yday is_leap mdays [wday] yday ydays leap_years_since_epoch */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum isdigit isspace */
#include <errno.h>              /* errno EINVAL */
#include <stdlib.h>             /* NULL strtod */
#include <string.h>             /* strrchr strspn */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static const char civil_time_digits[] = "0123456789";
/* run-time-initialized value slightly < 1.0 */
static double civil_time_almost_one = 1.0;
static double civil_time_epsilon = 7.62939453125e-6; /* 2^-17 */
static char civil_time_almost_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* initialize civil_time_almost_one, etc. at run-time */
/* copyright notices are required to be visible! */
/* called by: civil_time_struct_check, civil_time_cmp, parse_civil_time_text */
static void initialize_civil_time_almost_one(void (*f)(int, void *,
    const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "initialize_civil_time_almost_one";
#endif

    if ((unsigned char)0U == civil_time_almost_initialized) {
        int n;
        volatile double d, e;   /* 'volatile' avoids bugs due to excessive optimization */
        const char *s;

        s = strrchr(filenamebuf, '/');
        if (NULL == s)
            s = filenamebuf;
        civil_time_almost_initialized
            = register_build_strings(NULL, &source_file, s);
        /* reduce civil_time_epsilon until civil_time_almost_one = 1.0 - civil_time_epsilon is indistinguishable from 1.0:
           resulting civil_time_epsilon will be too small
        */
        for (civil_time_almost_one=1.0-civil_time_epsilon; 1.0 > civil_time_almost_one; civil_time_epsilon *= 0.5) {
            civil_time_almost_one = 1.0 - civil_time_epsilon;
            if (1.0e-18 > civil_time_epsilon) break; /* gcc 6.2.0 issue */
        }
        if (NULL != f) {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: civil_time_almost_one=%.36g, civil_time_epsilon=%.36g (too small)",
                __func__, source_file, __LINE__,
                civil_time_almost_one, civil_time_epsilon
                );
        }
        /* revise based on tests at a few powers of ten */
        for (n=-308; n<=308; n+=88) { /* 616 = 2^3 * 7 * 11 */
            d = sn1en(n, f, log_arg);
            e = d * civil_time_almost_one;
            while (0.0 == d - e) {
                civil_time_epsilon *= 2.0;
                civil_time_almost_one = 1.0 - civil_time_epsilon;
                e = d * civil_time_almost_one;
            }
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: civil_time_almost_one=%.36g, civil_time_epsilon=%.36g (OK at %.19g)",
                    __func__, source_file, __LINE__,
                    civil_time_almost_one, civil_time_epsilon, d
                    );
            }
        }
        /* more extensive testing might result in a further doubling, so: */
        civil_time_epsilon *= 2.0; /* safety margin */
        civil_time_almost_one = 1.0 - civil_time_epsilon;
    }
}

/* civil_time_bit_name: called from civil_time_bits_check and parse_civil_time_text */
static inline const char *civil_time_bit_name(char *buf, int sz, unsigned int bit,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    switch (bit) {
        case CIVIL_TIME_YEAR:
        return "CIVIL_TIME_YEAR";
        case CIVIL_TIME_MONTH:
        return "CIVIL_TIME_MONTH";
        case CIVIL_TIME_MDAY:
        return "CIVIL_TIME_MDAY";
        case CIVIL_TIME_YDAY:
        return "CIVIL_TIME_YDAY";
        case CIVIL_TIME_HOUR:
        return "CIVIL_TIME_HOUR";
        case CIVIL_TIME_MIN:
        return "CIVIL_TIME_MIN";
        case CIVIL_TIME_SEC:
        return "CIVIL_TIME_SEC";
        case CIVIL_TIME_FRAC:
        return "CIVIL_TIME_FRAC";
        case CIVIL_TIME_OFFS:
        return "CIVIL_TIME_OFFS";
# ifdef CIVIL_TIME_WDAY
        case CIVIL_TIME_WDAY:
        return "CIVIL_TIME_WDAY";
# endif
        default:
            (void)snul(buf, sz, "unknown bit value 0x", NULL, bit, 16, (int)'0', 3, f, log_arg);
        return (const char *)buf;
    }
}

/* debug: check that bits are clear/set */
/* civil_time_bits_check: called from parse_civil_time_text */
static inline void civil_time_bits_check(const char *s, unsigned int desired,
    unsigned int actual, unsigned int min, unsigned int max,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char buf[32];   /* RATS: ignore (always used with sizeof()) */
    unsigned int bit;
# ifndef PP__FUNCTION__
    static const char __func__[] = "civil_time_bits_check";
# endif

    for (bit=min; bit<=max; bit <<= 1) {
        if (0U == desired) {
            if (bit == (bit & actual)) {
                if (NULL != f) {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: %s: %s set",
                        __func__, source_file, __LINE__,
                        s, civil_time_bit_name(buf, sizeof(buf), bit, f, log_arg)
                        );
                }
            }
        } else {
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s: %s not set",
                    __func__, source_file, __LINE__,
                    s, civil_time_bit_name(buf, sizeof(buf), bit, f, log_arg)
                    );
            }
        }
    }
}

/* copy specified struct tm values from source to destination */
/* civil_time_fillin: called from parse_civil_time_text */
/* ARGSUSED3 */
static inline void civil_time_fillin(struct tm *destination, const struct tm *source,
    unsigned int bits, void (*f)(int, void *, const char *, ...), void *log_arg)
{
    unsigned int bit;
#ifndef PP__FUNCTION__
    static const char __func__[] = "civil_time_fillin";
#endif

    if ((NULL != source) && (NULL != destination)) {
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1) {
            if (bit == (bit & bits)) {
                switch (bit) {
                    case CIVIL_TIME_YEAR:
                        destination->tm_year = source->tm_year;
                    break;
                    case CIVIL_TIME_MONTH:
                        destination->tm_mon = source->tm_mon;
                    break;
                    case CIVIL_TIME_MDAY:
                        destination->tm_mday = source->tm_mday;
                    break;
                    case CIVIL_TIME_YDAY:
                        destination->tm_yday = source->tm_yday;
                    break;
                    case CIVIL_TIME_HOUR:
                        destination->tm_hour = source->tm_hour;
                    break;
                    case CIVIL_TIME_MIN:
                        destination->tm_min = source->tm_min;
                    break;
                    case CIVIL_TIME_SEC:
                        destination->tm_sec = source->tm_sec;
                    break;
#if CIVIL_TIME_DO_WDAY
                    case CIVIL_TIME_WDAY:
                        destination->tm_wday = source->tm_wday;
                    break;
#endif
                }
            }
        }
    }
}

#define CIVIL_TIME_CHECK_LEAP_YEAR -2
#define CIVIL_TIME_CHECK_LEAP_SECOND -3
/* return < 0 if UTC struct tm year/month/day/hour/minute/second are inconsistent
   return 0 if all's well
*/
/* civil_time_struct_check: called from parse_civil_time_text */
static inline int civil_time_struct_check(struct civil_time_struct *pcts,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int leap, maxdays, year;
    struct civil_time_struct cts;
#ifndef PP__FUNCTION__
    static const char __func__[] = "civil_time_struct_check";
#endif

    if ((unsigned char)0U == civil_time_almost_initialized)
        initialize_civil_time_almost_one(f, log_arg);
    /* local copy */
    civil_time_cpy(pcts, &cts, f, log_arg);
    year = cts.tm.tm_year + 1900;
    if ((CIVIL_TIME_YEAR_MIN > year) || (CIVIL_TIME_YEAR_MAX < year)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid year %lu",
                __func__, source_file, __LINE__,
                year
                );
        }
        return -1; /* bogus year */
    }
    /* year is in range */
    if ((0 > cts.tm.tm_mon) || (11 < cts.tm.tm_mon)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid month %d",
                __func__, source_file, __LINE__,
                cts.tm.tm_mon
                );
        }
        return -1; /* illegal month */
    }
    /* month is in range */
    if (1 > cts.tm.tm_mday) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid mday %d",
                __func__, source_file, __LINE__,
                cts.tm.tm_mday
                );
        }
        return -1; /* lowest valid day is 1 */
    }
    leap = is_leap(year, f, log_arg);
    maxdays = mdays(cts.tm.tm_mon, leap, f, log_arg);
    if (cts.tm.tm_mday > maxdays) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: mday %d > %d (mon %d, leap %d, year %lu)",
                __func__, source_file, __LINE__,
                cts.tm.tm_mday, maxdays, cts.tm.tm_mon, leap, year
                );
        }
        return CIVIL_TIME_CHECK_LEAP_YEAR; /* month isn't that long */
    }
    /* mday is in range */
    if ((0 > cts.tm.tm_hour) || (23 < cts.tm.tm_hour)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid hour %d",
                __func__, source_file, __LINE__,
                cts.tm.tm_hour
                );
        }
        return -1; /* invalid hour */
    }
    /* hour is in range */
    if ((0 > cts.tm.tm_min) || (59 < cts.tm.tm_min)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid minute %d",
                __func__, source_file, __LINE__,
                cts.tm.tm_min
                );
        }
        return -1; /* illegal minute */
    }
    /* min is in range */
    if ((0 > cts.tm.tm_sec) || (60 < cts.tm.tm_sec)) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: invalid second %d",
                __func__, source_file, __LINE__,
                cts.tm.tm_sec
                );
        }
        return -1; /* second out of bounds */
    }
    if (60 == cts.tm.tm_sec) {
        /* need to translate to UTC to determine if positive leap second is possible */
        civil_time_zone_change(&cts, 0.0, f, log_arg);
        /* leap second occurs only at the end of 23:59 UTC */
        if ((23 != cts.tm.tm_hour) || (59 != cts.tm.tm_min)) {
            if (NULL != f) {
                char buf[64];       /* RATS: ignore (always used with sizeof()) */
                char buf2[64];      /* RATS: ignore (always used with sizeof()) */

                (void)sn_civil_time(buf, sizeof(buf), pcts, -15, 0, 0, f, log_arg);
                (void)sn_civil_time(buf2, sizeof(buf2), &cts, -15, 0, 0, f, log_arg);
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s -> %s: %02d:%02d != 23:59",
                    __func__, source_file, __LINE__,
                    buf, buf2, cts.tm.tm_hour, cts.tm.tm_min
                    );
            }
            return CIVIL_TIME_CHECK_LEAP_SECOND; /* wrong time-of-day */
        }
        /* leap second only occurs at the end of June or December */
        if ((5 != cts.tm.tm_mon) && (11 != cts.tm.tm_mon)) {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: month %d",
                    __func__, source_file, __LINE__,
                    cts.tm.tm_mon
                    );
            }
            return CIVIL_TIME_CHECK_LEAP_SECOND; /* wrong month */
        }
        year = cts.tm.tm_year + 1900;
        leap = is_leap(year, f, log_arg);
        maxdays = mdays(cts.tm.tm_mon, leap, f, log_arg);
        if (cts.tm.tm_mday != maxdays) {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: mday %d != %d",
                    __func__, source_file, __LINE__,
                    cts.tm.tm_mday, maxdays
                    );
            }
            return CIVIL_TIME_CHECK_LEAP_SECOND; /* not last day of month */
        }
        if (0 >= leap_second(&cts.tm, f, log_arg))
            return CIVIL_TIME_CHECK_LEAP_SECOND; /* not a leap second */
    }
    /* sec is in range */
    return 0;
}

/* publicly exposed functions */

/* Compare two date-time structures (which may be in different zones).
   The two compare as equal if the equivalent UTC date-times are the same
   to within the smallest representable fraction of a second.  Return value is
   zero for equality, negative if pcts1 points to an earlier date-time than
   pcts2, positive if pcts1 points to a later date-time than pcts2.
*/
/* calls: civil_time_cpy, civil_time_zone_change, initialize_civil_time_almost_one */
/* called by: civil_time_difference */
int civil_time_cmp(const struct civil_time_struct *pcts1, const struct civil_time_struct *pcts2)
{
    if (NULL == pcts1) {
        if (NULL == pcts2)
            return 0;
        return -1;
    } else if (NULL == pcts2) {
        return 1;
    } else {
        struct civil_time_struct cts1, cts2; /* local copies, converted to UTC and normalized for comparison */

        if ((unsigned char)0U == civil_time_almost_initialized)
            initialize_civil_time_almost_one(NULL, NULL); /* no logger */
        civil_time_cpy(pcts1, &cts1, NULL, NULL);   /* no logger */
        civil_time_zone_change(&cts1, 0.0, NULL, NULL);   /* no logger */
        civil_time_cpy(pcts2, &cts2, NULL, NULL);   /* no logger */
        civil_time_zone_change(&cts2, 0.0, NULL, NULL);   /* no logger */
        if (cts1.tm.tm_year < cts2.tm.tm_year)
            return -1;
        if (cts1.tm.tm_year > cts2.tm.tm_year)
            return 1;
        if (cts1.tm.tm_mon < cts2.tm.tm_mon)
            return -1;
        if (cts1.tm.tm_mon > cts2.tm.tm_mon)
            return 1;
        if (cts1.tm.tm_mday < cts2.tm.tm_mday)
            return -1;
        if (cts1.tm.tm_mday > cts2.tm.tm_mday)
            return 1;
        if (cts1.tm.tm_hour < cts2.tm.tm_hour)
            return -1;
        if (cts1.tm.tm_hour > cts2.tm.tm_hour)
            return 1;
        if (cts1.tm.tm_min < cts2.tm.tm_min)
            return -1;
        if (cts1.tm.tm_min > cts2.tm.tm_min)
            return 1;
        if (cts1.tm.tm_sec < cts2.tm.tm_sec)
            return -1;
        if (cts1.tm.tm_sec > cts2.tm.tm_sec)
            return 1;
        if ((unsigned char)0U == civil_time_almost_initialized)
            initialize_civil_time_almost_one(NULL, NULL); /* no logger */
        if (cts1.fraction + civil_time_epsilon <= cts2.fraction)
            return -1;
        if (cts1.fraction >= cts2.fraction + civil_time_epsilon)
            return 1;
    }
    return 0;
}

/* Compute the difference in seconds between two date-time structures (which may be in different zones).
   Take leap seconds into account.
   The difference is positive if the time pointed to by pcts1 is later than that pointed to by pcts2.
*/
/* calls: civil_time_cmp, civil_time_cpy, civil_time_zone_change, leap_seconds_between, years_since_epoch */
/* not called by other civil_time functions */
double civil_time_difference(const struct civil_time_struct *pcts1,
    const struct civil_time_struct *pcts2 ,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    double diff = 0.0;

    if ((NULL == pcts1)
    || (NULL == pcts2)
    ) {
        if (NULL != f) {
            char buf[24];       /* RATS: ignore (always used with sizeof()) */
            char buf2[24];      /* RATS: ignore (always used with sizeof()) */

            (void)snul(buf, sizeof(buf), "0x", NULL, (unsigned long)pcts1, 16, (int)'0', 1, f, log_arg);
            (void)snul(buf2, sizeof(buf2), "0x", NULL, (unsigned long)pcts2, 16, (int)'0', 1, f, log_arg);
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL pointer in %s(%s, %s)",
                __func__, source_file, __LINE__,
                __func__, buf, buf2
                );
        }
        errno = EINVAL;
    } else {
        int n;
        struct civil_time_struct cts1, cts2; /* local copies, converted to UTC and normalized for comparison */

        if ((unsigned char)0U == civil_time_almost_initialized)
            initialize_civil_time_almost_one(f, log_arg);
        civil_time_cpy(pcts1, &cts1, f, log_arg);
        civil_time_zone_change(&cts1, 0.0, f, log_arg);
        civil_time_cpy(pcts2, &cts2, f, log_arg);
        civil_time_zone_change(&cts2, 0.0, f, log_arg);
        if (NULL != f) f(LOG_DEBUG, log_arg, "%s: %04d-%02d-%02dT%02d:%02d:%02dZ to %04d-%02d-%02dT%02d:%02d:%02dZ", __func__, cts1.tm.tm_year + 1900, cts1.tm.tm_mon + 1, cts1.tm.tm_mday, cts1.tm.tm_hour, cts1.tm.tm_min, cts1.tm.tm_sec, cts2.tm.tm_year + 1900, cts2.tm.tm_mon + 1, cts2.tm.tm_mday, cts2.tm.tm_hour, cts2.tm.tm_min, cts2.tm.tm_sec);
        /* leap seconds */
        errno = 0;
        n = leap_seconds_between(&(cts1.tm), &(cts2.tm), f, log_arg);
        if ((-1 != n) || (errno == 0)) {
            int c, leap, yadj=0;
            int maxy, miny, years, leaps;

            /* fraction */
            diff += cts1.fraction - cts2.fraction;
            /* leap seconds adjustment */
            if ((cts1.tm.tm_min == cts2.tm.tm_min)
            && ((60 == cts1.tm.tm_sec) || (60 == cts2.tm.tm_sec))
            && (0 < n)
            )
                n--; /* one also counted in seconds difference below */
            /* leap_seconds_between always counts from earlier to later time regardless of argument order */
            c = civil_time_cmp(&cts1, &cts2);
            if (0 > c)
                n = 0 - n; /* adjust for argument order given to civil_time_difference */
            diff += 1.0 * (double)n; /* leap seconds adjustment */
            if (NULL != f) f(LOG_DEBUG, log_arg, "%s: leap seconds adjustment %d", __func__, n);
            /* seconds */
            n = cts1.tm.tm_sec - cts2.tm.tm_sec;
            diff += 1.0 * (double)n;
            if (NULL != f) f(LOG_DEBUG, log_arg, "%s: seconds adjustment %d", __func__, n);
            /* minutes */
            n = cts1.tm.tm_min - cts2.tm.tm_min;
            diff += 60.0 * (double)n;
            if (NULL != f) f(LOG_DEBUG, log_arg, "%s: seconds adjustment for %d minute%s %d0", __func__, n, n==1?"":"s", n*6);
            /* hours */
            n = cts1.tm.tm_hour - cts2.tm.tm_hour;
            diff += 3600.0 * (double)n;
            if (NULL != f) f(LOG_DEBUG, log_arg, "%s: seconds adjustment for %d hour%s %d00", __func__, n, n==1?"":"s", n*36);
            /* days */
            /*  if cts1 is later than cts2
                    seconds (in days) from cts2 to the end of that year (unless cts1 is in the same year) need to be counted
                    seconds (in days) from the start of the year of cts1 (unless cts2 is in the same year) to cts1 need to be counted
                if cts2 is later than cts1
                    seconds (in days) from cts1 to the end of that year (unless cts2 is in the same year) need to be counted
                    seconds (in days) from the start of the year of cts2 (unless cts1 is in the same year) to cts2 need to be counted
            */
            cts1.tm.tm_yday = yday(&(cts1.tm), f, log_arg);
            cts2.tm.tm_yday = yday(&(cts2.tm), f, log_arg);
            /* beware leap years! */
            /* year numbers (also used later) */
            miny = cts1.tm.tm_year + 1900;
            maxy = cts2.tm.tm_year + 1900;
            if (miny == maxy) {
                n = cts1.tm.tm_yday - cts2.tm.tm_yday;
            } else if (miny < maxy /* i.e. cts2 is later than cts1 */ ) {
                leap = is_leap(miny, f, log_arg);
                n = ydays(12, leap, f, log_arg) - cts1.tm.tm_yday; /* days to end of cts1 year */
                n += cts2.tm.tm_yday;
                n = 0 - n; /* sign adjustment */
                yadj = 1; /* one year is accounted for by ydays addition */
            } else /* miny > maxy i.e. cts1 is later than cts2 */ {
                leap = is_leap(maxy, f, log_arg);
                n = ydays(12, leap, f, log_arg) - cts2.tm.tm_yday; /* days to end of cts2 year */
                n += cts1.tm.tm_yday;
                yadj = -1; /* one year is accounted for by ydays addition */
            }
            diff += 86400.0 * (double)n;
            if (NULL != f) f(LOG_DEBUG, log_arg, "%s: seconds adjustment for %d yday%s %d00", __func__, n, n==1?"":"s", n*864);
            /* years */
            n = miny - maxy + yadj;
            diff += 31536000.0 * (double)n;
            if (NULL != f) f(LOG_DEBUG, log_arg, "%s: seconds adjustment for %d year%s %d000", __func__, n, n==1?"":"s", n*31536);
            if (miny > maxy) {
                years = miny;
                miny = maxy;
                maxy = years;
            }
            leaps = leap_years_since_epoch(maxy, miny, f, log_arg);
            /* if the minimum year is a leap year, but the date is after leap day, the return from leap_years_since_epoch must be decreased */
            /* one of the leap years' leap day's seconds might have already been counted via ydays (if minimum year is a leap year and date is leap day or earlier) */
            if ((0 < leaps) && (0 != is_leap(miny, f, log_arg))) {
                leaps--;
                if (NULL != f) f(LOG_DEBUG, log_arg, "%s: leap year adjustment changed to %d day%s because %d is a leap year", __func__, leaps, leaps==1?"":"s", miny);
            }
            if (0 > c)
                diff -= 86400.0 * (double)leaps;
            else
                diff += 86400.0 * (double)leaps;
        }
    }
    if (NULL != f) f(LOG_DEBUG, log_arg, "%s returns %.0f", __func__, diff);
    return diff;
}

/* read text date-time string and fill in result data set.
   use minimum legal, maximum legal, or reference data set
   values for missing elements.
*/
/* calls: nstrtol, isdigit, strspn, strtod, initialize_civil_time_almost_one,
      civil_time_cpy, civil_time_fillin, inverse_yday, civil_time_struct_check,
      is_leap, mday, yday, wday, civil_time_bits_check
*/
/* not called by other civil_time functions */
int parse_civil_time_text(const char *s, struct civil_time_struct *pcts_result,
    char **ppe, int which, const struct civil_time_struct *pcts_ref,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char sign, *p;
    char buf[32];               /* RATS: ignore (always used with sizeof()) */
    const char *q = s;
    int leap, osec;
    unsigned int bit, bits, deferred = 0x0U, set = 0x0U;
    int year;
    size_t sz;
    long l;
    double fraction = 0.0, offset;
    struct civil_time_struct cts, rcts;
#ifndef PP__FUNCTION__
    static const char __func__[] = "parse_civil_time_text";
#endif

    if (NULL == pcts_result) {
        if (NULL != f) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL result pointer",
                __func__, source_file, __LINE__
                );
        }
        errno = EINVAL;
        return -1;
    }
    /* skip leading whitespace */
    if (NULL != s)
        while (0 != isspace((int)*s))
            s++;
    /* year */
    if (NULL == s)
        sz = 0;
    else
        sz = strspn(s, civil_time_digits);
    if ((2 == sz) && (':' == s[2]))
        sz = 0; /* special case for time only */
    if ((unsigned char)0U == civil_time_almost_initialized)
        initialize_civil_time_almost_one(f, log_arg);
    switch (sz) {
        case 4: /* 4-digit year */
#if defined(lint)
            p = (char *)s + 4;
#endif
            cts.tm.tm_year = (int)nstrtol(s, sz, &p, 10, f, log_arg) - 1900;
            if ('-' == *p)
                p++;
            set |= CIVIL_TIME_YEAR;
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s tm_year %d, remainder %s",
                    __func__, source_file, __LINE__,
                    s, cts.tm.tm_year, p
                    );
            }
            s = p;
        break;
        case 0: /* no year */
            /* defer processing until later */
            deferred |= CIVIL_TIME_YEAR;
            while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s) && ('-' != *s))
                s++;
            if ((NULL != s) && ('-' == *s))
                s++;
            while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s) && ('-' != *s))
                s++;
        break;
    }
    /* remainder of date: month and mday or yday */
    if (NULL == s)
        sz = 0;
    else
        sz = strspn(s, civil_time_digits);
    if (2 == sz) {
        switch (s[2]) {
            case ':': /* special case for time only */
            /*FALLTHROUGH*/
            case 'T': /* special cases for mday only */
            /*FALLTHROUGH*/
            case 't': /* special cases for mday only */
            /*FALLTHROUGH*/
            case ' ': /* special cases for mday only */
                sz = 0; /* no month */
            break;
            default:
            break;
        }
    }
    switch (sz) {
        case 3: /* 3-digit CCSDS yday */
            cts.tm.tm_yday = (int)nstrtol(s, sz, &p, 10, f, log_arg) - 1;
            set |= CIVIL_TIME_YDAY;
            deferred |= CIVIL_TIME_MONTH;
            deferred |= CIVIL_TIME_MDAY;
            /* skip over date-time separator if present (presumably 'T') */
            if ('\0' != *p)
                p++;
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s tm_yday %d, remainder %s",
                    __func__, source_file, __LINE__,
                    s, cts.tm.tm_yday, p
                    );
            }
            s = p;
        break;
        case 2: /* 2-digit month */
            cts.tm.tm_mon = (int)nstrtol(s, sz, &p, 10, f, log_arg) - 1;
            set |= CIVIL_TIME_MONTH;
            /* skip over any non-digit separators between MM and DD */
            for (; ('\0' != *p) && (0 == isdigit((int)*p)); )
                p++;
                if (NULL != f) {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: %s tm_mon %d, remainder %s",
                        __func__, source_file, __LINE__,
                        s, cts.tm.tm_mon, p
                        );
                }
            if ('\0' == *p) { /* no mday */
                deferred |= CIVIL_TIME_MDAY;
                s = p;
            } else {
                s = p;
                cts.tm.tm_mday = (int)nstrtol(s, 2, &p, 10, f, log_arg);
                set |= CIVIL_TIME_MDAY;
                /* skip over non-digit separator after mday (if any) */
                /* CCSDS 301.0-B-4 time cannot be truncated on the left */
                for (; ('\0' != *p) && (0 == isdigit((int)*p)); )
                    p++;
                if (NULL != f) {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: %s tm_mday %d, remainder %s",
                        __func__, source_file, __LINE__,
                        s, cts.tm.tm_mday, p
                        );
                }
                s = p;
            }
            deferred |= CIVIL_TIME_YDAY;
        break;
        case 0: /* no month or yday */
            if (NULL != s) {
                if ('-' == *s) { /* might still have mday */
                    s++;
                    if ((0 != isdigit((int)*s)) && (':' != s[2])) { /* have mday */
                        cts.tm.tm_mday = (int)nstrtol(s, 2, &p, 10, f, log_arg);
                        set |= CIVIL_TIME_MDAY;
                        /* skip over non-digit separator after mday (if any) */
                        /* CCSDS 301.0-B-4 time cannot be truncated on the left */
                        for (; ('\0' != *p) && (0 == isdigit((int)*p)); )
                            p++;
                        if (NULL != f) {
                            f(LOG_DEBUG, log_arg,
                                "%s: %s line %d: %s tm_mday %d, remainder %s",
                                __func__, source_file, __LINE__,
                                s, cts.tm.tm_mday, p
                                );
                        }
                        s = p;
                    } else { /* no mday; skip over time separator (if any) */
                        deferred |= CIVIL_TIME_MDAY;
                        if (('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s))
                            s++;
                    }
                } else {
                    deferred |= CIVIL_TIME_MDAY;
                }
            }
            deferred |= CIVIL_TIME_MONTH;
            deferred |= CIVIL_TIME_YDAY;
            while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s))
                s++;
        break;
    }
    /* hour */
    if (NULL == s)
        sz = 0;
    else
        sz = strspn(s, civil_time_digits);
    switch (sz) {
        case 0: /* no hour */
            deferred |= CIVIL_TIME_HOUR;
        break;
        default: 
            cts.tm.tm_hour = (int)nstrtol(s, 2, &p, 10, f, log_arg);
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s tm_hour %d, remainder %s",
                    __func__, source_file, __LINE__,
                    s, cts.tm.tm_hour, p
                    );
            }
            s = p;
            set |= CIVIL_TIME_HOUR;
        break;
    }
    while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s))
        s++;
    if ((NULL != s) && (':' == *s))
        s++;
    while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s))
        s++;
    /* minute */
    if (NULL == s)
        sz = 0;
    else
        sz = strspn(s, civil_time_digits);
    switch (sz) {
        case 0: /* no minute */
            deferred |= CIVIL_TIME_MIN;
        break;
        default: 
            cts.tm.tm_min = (int)nstrtol(s, 2, &p, 10, f, log_arg);
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s tm_min %d, remainder %s",
                    __func__, source_file, __LINE__,
                    s, cts.tm.tm_min, p
                    );
            }
            s = p;
            set |= CIVIL_TIME_MIN;
        break;
    }
    while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s))
        s++;
    if ((NULL != s) && (':' == *s))
        s++;
    while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)) && (':' != *s))
        s++;
    /* second */
    if (NULL == s)
        sz = 0;
    else
        sz = strspn(s, civil_time_digits);
    switch (sz) {
        case 0: /* no second */
            deferred |= CIVIL_TIME_SEC;
        break;
        default: 
            cts.tm.tm_sec = (int)nstrtol(s, 2, &p, 10, f, log_arg);
            if (NULL != f) {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: %s tm_sec %d, remainder %s",
                    __func__, source_file, __LINE__,
                    s, cts.tm.tm_sec, p
                    );
            }
            s = p;
            set |= CIVIL_TIME_SEC;
        break;
    }
    /* fraction */
    if ((NULL != s) && ('.' == *s)) {
        cts.fraction = fraction = strtod(s, &p);
        if (NULL != f) {
            (void)snf(buf, sizeof(buf), NULL, NULL, fraction, (int)'0', 1, -20, f, log_arg);
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: %s fraction %s, remainder %s",
                __func__, source_file, __LINE__,
                s, buf, p
                );
        }
        s = p;
    } else { /* no fraction */
        cts.fraction = 0.0;
        if (0 < which) /* (which > 0) */{ 
            fraction = civil_time_almost_one;
        } else if (0 > which) /* (which < 0) */ {
            fraction = 0.0;
        } else /* (0 == which) */ {
            if (NULL != pcts_ref)
                fraction = pcts_ref->fraction;
        }
    }
    set |= CIVIL_TIME_FRAC;
    /* offset */
    if (NULL == s) { /* no offset specified */
        cts.offset = 0.0;
        if ((0 == which) && (NULL != pcts_ref)) {
            offset = pcts_ref->offset;
            set |= CIVIL_TIME_OFFS;
        } else {
            /* unreasonable to attempt to track political time zone offsets here */
            /* set to UTC */
            offset = 0.0;
            set |= CIVIL_TIME_OFFS;
        }
    } else {
        switch (*s) {
            case '\0': /* CCSDS 301.0-B-4 (UTC) */
                cts.offset = 0.0;
                if ((0 == which) && (NULL != pcts_ref)) {
                    offset = pcts_ref->offset;
                    set |= CIVIL_TIME_OFFS;
                } else {
                    /* unreasonable to attempt to track political time zone offsets here */
                    /* set to UTC */
                    offset = 0.0;
                    set |= CIVIL_TIME_OFFS;
                }
            break;
            case 'z': /* special case RFC 3339 */
            /*FALLTHROUGH*/
            case 'Z': /* UTC */
                cts.offset = offset = 0.0;
                set |= CIVIL_TIME_OFFS;
            break;
            case '+':
            /*FALLTHROUGH*/
            case '-': /* numeric offset [-+]hh:mm */
                sign = *s;
                s++;
                l = nstrtol(s, (size_t)2, &p, 10, f, log_arg) * 60L; /* absolute value */
                if ('\0' != *p) {
                    s = p;
                    if ((NULL != s) && (':' == *s))
                        s++;
                    while ((NULL != s) && ('\0' != *s) && (0 == isdigit((int)*s)))
                        s++;
                    l += nstrtol(s, (size_t)2, &p, 10, f, log_arg);
                }
                if ('-' == sign) { /* sign is not what one might expect */
                    offset = 0.0 - 60.0 * l;
                } else {
                    offset = 60.0 * l;
                }
                cts.offset = offset;
                set |= CIVIL_TIME_OFFS;
                if (NULL != f) {
                    (void)snf(buf, sizeof(buf), NULL, NULL, offset, (int)'0', 1, -20, f, log_arg);
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: %s offset %s, remainder %s",
                        __func__, source_file, __LINE__,
                        s, buf, p
                        );
                }
            break;
            default: /* trailing junk; no offset */
                cts.offset = 0.0;
                if ((0 == which) && (NULL != pcts_ref)) {
                    offset = pcts_ref->offset;
                    set |= CIVIL_TIME_OFFS;
                } else {
                    /* unreasonable to attempt to track political time zone offsets here */
                    /* set to UTC */
                    offset = 0.0;
                    set |= CIVIL_TIME_OFFS;
                }
            break;
        }
    }
    /* set end pointer */
    if ((NULL != s) && (NULL != ppe))
        *ppe = p;
    /* some tm values were filled in from string-specified text */
    /* any values not explicitly set are by default deferred */
    for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
        if (bit != (bit & set))
            deferred |= bit;
    bits = CIVIL_TIME_YEAR | CIVIL_TIME_MONTH | CIVIL_TIME_MDAY
        | CIVIL_TIME_HOUR | CIVIL_TIME_MIN | CIVIL_TIME_SEC;
    if (0 == which) { 
        /* first: get reference values in same offset zone as tentative result */
        /* but cannot account for political "daylight saving" zone changes */
        civil_time_cpy(NULL!=pcts_ref?pcts_ref:pcts_result, &rcts, f, log_arg);
        /* translate to destination offset and normalize */
        civil_time_zone_change(&rcts, offset, f, log_arg);
        /* now rcts has reference values adjusted to same zone as specified by string */
        /* next: fill in missing values */
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
            if (bit == (bit & set))
                bits &= ~bit;
        civil_time_fillin(&(cts.tm), &(rcts.tm), bits, f, log_arg);
        /* if there was a text-specified yday, fill in month and mday */
        if (CIVIL_TIME_YDAY == ((CIVIL_TIME_MONTH|CIVIL_TIME_MDAY|CIVIL_TIME_YDAY) & set)) {
            inverse_yday(&(cts.tm), f, log_arg);   /* requires normalized year, yday */
            set |= CIVIL_TIME_MONTH;
            set |= CIVIL_TIME_MDAY;
        }
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
            if (bit == (bit & bits))
                set |= bit;
        deferred &= ~(set);
        /* next: is result legal? */
        cts.offset = offset;
        osec = civil_time_struct_check(&cts, f, log_arg);
        if (0 > osec) {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: invalid civil_time structure",
                    __func__, source_file, __LINE__
                    );
            }
            errno = EINVAL;
            return -1;
        }
    } else if (0 < which) { /* maximum legal values */
        /* values apply in the result zone */
        rcts.tm.tm_year = CIVIL_TIME_YEAR_MAX - 1900;
        rcts.tm.tm_mon = 11;
        rcts.tm.tm_mday = 31; /* tentative; might need to adjust if month is specified */
        rcts.tm.tm_hour = 23;
        rcts.tm.tm_min = 59;
        rcts.tm.tm_sec = 60; /* tentative; positive leap second */
        /* next: fill in missing values */
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
            if (bit == (bit & set))
                bits &= ~bit;
        civil_time_fillin(&(cts.tm), &(rcts.tm), bits, f, log_arg);
        /* if there was a text-specified yday, fill in month and mday */
        if (CIVIL_TIME_YDAY == ((CIVIL_TIME_MONTH|CIVIL_TIME_MDAY|CIVIL_TIME_YDAY) & set)) {
            inverse_yday(&(cts.tm), f, log_arg);   /* requires normalized year, yday */
            set |= CIVIL_TIME_MONTH;
            set |= CIVIL_TIME_MDAY;
        }
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
            if (bit == (bit & bits))
                set |= bit;
        deferred &= ~(set);
        cts.offset = offset;
        osec = civil_time_struct_check(&cts, f, log_arg);
        if (CIVIL_TIME_CHECK_LEAP_YEAR == osec) {
            /* maybe month was too long */
            if (CIVIL_TIME_MDAY == (bits & CIVIL_TIME_MDAY)) {
                year = cts.tm.tm_year + 1900;
                leap = is_leap(year, f, log_arg);
                cts.tm.tm_mday = mdays(cts.tm.tm_mon, leap, f, log_arg);
                osec = civil_time_struct_check(&cts, f, log_arg);
            }
        }
        if (CIVIL_TIME_CHECK_LEAP_YEAR == osec) {
            if (CIVIL_TIME_YEAR == (bits & CIVIL_TIME_YEAR)) {
                /* maybe specified February 29 and need a leap year */
                if ((1 == cts.tm.tm_mon) && (29 == cts.tm.tm_mday)) {
                    year = cts.tm.tm_year + 1900;
                    leap = is_leap(year, f, log_arg);
                    if (0 == leap) {
                        do {
                            cts.tm.tm_year--;
                            year = cts.tm.tm_year + 1900;
                            leap = is_leap(year, f, log_arg);
                        } while (0 == leap);
                        osec = civil_time_struct_check(&cts, f, log_arg);
                    }
                }
            }
        }
        if (CIVIL_TIME_CHECK_LEAP_SECOND == osec) {
            if (CIVIL_TIME_SEC == (bits & CIVIL_TIME_SEC)) {
                if (60 == cts.tm.tm_sec) {
                    cts.tm.tm_sec = 59;
                    osec = civil_time_struct_check(&cts, f, log_arg);
                }
            }
        }
        if (0 != osec) {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: invalid civil_time structure",
                    __func__, source_file, __LINE__
                    );
            }
            errno = EINVAL;
            return -1;
        }
    } else /* 0 > which */ { /* minimum legal values */
        /* values apply in the result zone */
        rcts.tm.tm_year = CIVIL_TIME_YEAR_MIN - 1900;
        rcts.tm.tm_mon = 0;
        rcts.tm.tm_mday = 1;
        rcts.tm.tm_hour = 0;
        rcts.tm.tm_min = 0;
        rcts.tm.tm_sec = 0;
        rcts.offset = 0.0;
        /* next: fill in missing values */
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
            if (bit == (bit & set))
                bits &= ~bit;
        civil_time_fillin(&(cts.tm), &(rcts.tm), bits, f, log_arg);
        /* if there was a text-specified yday, fill in month and mday */
        if (CIVIL_TIME_YDAY == ((CIVIL_TIME_MONTH|CIVIL_TIME_MDAY|CIVIL_TIME_YDAY) & set)) {
            inverse_yday(&(cts.tm), f, log_arg);   /* requires normalized year, yday */
            set |= CIVIL_TIME_MONTH;
            set |= CIVIL_TIME_MDAY;
        }
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1)
            if (bit == (bit & bits))
                set |= bit;
        deferred &= ~(set);
        cts.offset = offset;
        osec = civil_time_struct_check(&cts, f, log_arg);
        if (CIVIL_TIME_CHECK_LEAP_YEAR == osec) {
            if (CIVIL_TIME_YEAR == (bits & CIVIL_TIME_YEAR)) {
                year = cts.tm.tm_year + 1900;
                leap = is_leap(year, f, log_arg);
                if (0 == leap) {
                    do {
                        cts.tm.tm_year++;
                        year = cts.tm.tm_year + 1900;
                        leap = is_leap(year, f, log_arg);
                    } while (0 == leap);
                    osec = civil_time_struct_check(&cts, f, log_arg);
                }
            }
        }
        if (0 != osec) {
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: invalid civil_time structure",
                    __func__, source_file, __LINE__
                    );
            }
            errno = EINVAL;
            return -1;
        }
    }
    /* year, month, mday, hour, minute, second values assigned & normalized */
    /* handle deferred settings */
    if (0x0U != deferred) {
        for (bit=0x01U; CIVIL_TIME_MAX >= bit ; bit <<= 1) {
            if (bit == (bit & deferred)) {
                switch (bit) {
                    case CIVIL_TIME_YDAY:
                        cts.tm.tm_yday = yday(&(cts.tm), f, log_arg);   /* requires normalized year, month, mday */
                    break;
#if CIVIL_TIME_DO_WDAY
                    case CIVIL_TIME_WDAY:
                        cts.tm.tm_wday = wday(&(cts.tm), f, log_arg);   /* requires normalized year, correct yday */
                    break;
#endif
                    default:
                        if (NULL != f) {
                            f(LOG_WARNING, log_arg,
                                "%s: %s line %d: shouldn't see %s here",
                                __func__, source_file, __LINE__,
                                civil_time_bit_name(buf, sizeof(buf), bit, f, log_arg)
                                );
                        }
                    continue;
                }
                deferred &= ~bit;
                set |= bit;
            }
        }
    }
    civil_time_fillin(&(pcts_result->tm), &(cts.tm), set, f, log_arg);
    pcts_result->fraction = fraction;
    pcts_result->offset = offset;
    civil_time_bits_check(q, 0UL, deferred, 0x01UL, CIVIL_TIME_MAX, f, log_arg);
    civil_time_bits_check(q, 1UL, set, 0x01UL, CIVIL_TIME_MAX, f, log_arg);
    return 0;
}
