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
* $Id: ~|^` @(#)    civil_time_normalize.c copyright 2009 - 2016 Bruce Lilly.   \ civil_time_normalize.c $
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
/* $Id: ~|^` @(#)   This is civil_time_normalize.c version 2.7 dated 2016-06-29T03:15:44Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "civil_time" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libcivil_time/src/s.civil_time_normalize.c */

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
* The date-time normalization function calling synopsis is:
void civil_time_normalize(struct civil_time_struct *pcts);
* Arguments:
*     pcts      pointer to the structure which holds the local date-time value
*                  to sub-second resolution
*                  tm_wday, tm_isdst components are ignored
*                  other date-time component values must be normalized
*                     (e.g. from localtime or via utc_mktime)
*                  fraction of a second time; range 0.0 to < 1.0
*                  offset of local time zone in seconds (and fraction if
*                     needed) East of the UTC Prime Meridian
*                  component values are modified by this function
*   Calls:
*              civil_time_zone_change
*                  change to/from UTC for proper handling of leap seconds
*              normalize_date_time
* Return value:    none
*
* The date-time zone change calling synopsis is:
void civil_time_zone_change(struct civil_time_struct *pcts, double new_offset);
* Arguments:
*     pcts      pointer to the structure which holds the local date-time value
*                  to sub-second resolution
*                  fraction of a second time; range 0.0 to < 1.0
*                  offset of local time zone in seconds (and fraction if
*                     needed) East of the UTC Prime Meridian
*                  component values are modified by this function
*     new_offset specifies the local zone offset that the data is to be
*                   translated to
*   Calls:
*               civil_time_normalize
* Return value:  none
* Translates a date-time expressed in some local zone (offset from UTC) to
*   an equivalent in a different zone (offset from UTC).
*   In particular, specifying a new_offset of 0.0 translates to UTC.
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
#define ID_STRING_PREFIX "$Id: civil_time_normalize.c ~|^` @(#)"
#define SOURCE_MODULE "civil_time_normalize.c"
#define MODULE_VERSION "2.7"
#define MODULE_DATE "2016-06-29T03:15:44Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2009 - 2016"

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

/* local header files needed */
#include "civil_time.h"      /* header file for public definitions and declarations */ /* includes time.h */
#include "utc_mktime.h"         /* is_leap mdays normalize_date_time */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#ifndef NULL
# include <stdlib.h>            /* NULL strtod */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char civil_time_normalize_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* copyright notices are required to be visible! */
static void initialize_civil_time_normalize(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    civil_time_normalize_initialized
        = register_build_strings(NULL, &source_file, s);
}

/* adjust_month: called from adjust_mday */
/* return year adjustment */
/* ARGSUSED2 */
static inline int adjust_month(struct tm *ptm, int adjustment,
   void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int new_value, next_adjustment = 0;

    if ((unsigned char)0U == civil_time_normalize_initialized)
        initialize_civil_time_normalize();
    new_value = ptm->tm_mon + adjustment;
    if (11 < new_value) {
        next_adjustment = new_value / 12;
        new_value %= 12;
    } else if (0 > new_value) {
        adjustment = 12 - new_value;  /* positive value to avoid implementation discrepancies with modulus */	
        next_adjustment = 0 - adjustment / 12;
        adjustment %= 12;
        new_value = 12 - adjustment;
    }
    ptm->tm_mon = new_value;
    if (0 != next_adjustment)
        ptm->tm_year += next_adjustment;
    return next_adjustment;
}

/* adjust_mday: called from adjust_hour */
static inline void adjust_mday(struct tm *ptm, int adjustment,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int ld, leap, x;

    ptm->tm_mday += adjustment;
    leap = is_leap(ptm->tm_year + 1900, f, log_arg);
    ld = mdays(ptm->tm_mon, leap, f, log_arg);
    while (ld < ptm->tm_mday) {
        ptm->tm_mday -= ld;
        x = adjust_month(ptm, 1, f, log_arg);
        if (0 != x)
            leap = is_leap(ptm->tm_year + 1900, f, log_arg);
        ld = mdays(ptm->tm_mon, leap, f, log_arg);
    }
    while (1 > ptm->tm_mday) {
        x = adjust_month(ptm, -1, f, log_arg);
        if (0 != x)
            leap = is_leap(ptm->tm_year + 1900, f, log_arg);
        ld = mdays(ptm->tm_mon, leap, f, log_arg);
        ptm->tm_mday += ld;
    }
}

/* adjust_hour: called from adjust_minute and civil_time_zone_change */
static inline void adjust_hour(struct tm *ptm, int adjustment,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int new_value, next_adjustment = 0;

    new_value = ptm->tm_hour + adjustment;
    if (23 < new_value) {
        next_adjustment = new_value / 24;
        new_value %= 24;
    } else if (0 > new_value) {
        adjustment = 24 - new_value;  /* positive value to avoid implementation discrepancies with modulus */	
        next_adjustment = 0 - adjustment / 24;
        adjustment %= 24;
        new_value = 24 - adjustment;
    }
    ptm->tm_hour = new_value;
    if (0 != next_adjustment)
        adjust_mday(ptm, next_adjustment, f, log_arg);
}

/* adjust_minute: called from civil_time_zone_change */
static inline void adjust_minute(struct tm *ptm, int adjustment,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int new_value, next_adjustment = 0;

    new_value = ptm->tm_min + adjustment;
    if (59 < new_value) {
        next_adjustment = new_value / 60;
        new_value %= 60;
    } else if (0 > new_value) {
        adjustment = 60 - new_value;  /* positive value to avoid implementation discrepancies with modulus */	
        next_adjustment = 0 - adjustment / 60;
        adjustment %= 60;
        new_value = 60 - adjustment;
    }
    ptm->tm_min = new_value;
    if (0 != next_adjustment)
        adjust_hour(ptm, next_adjustment, f, log_arg);
}

/* publicly exposed functions */

/* normalize structure components.
   First normalize the fraction, then the struct tm.
   Offset is not affected by normalization.
*/
/* calls: civil_time_zone_change, normalize_date_time */
/* called by: civil_time_zone_change, sn_date_time */
void civil_time_normalize(struct civil_time_struct *pcts,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int adjustment = 0;
    double offset;
#ifndef PP__FUNCTION__
    static const char __func__[] = "civil_time_normalize";
#endif

    if (NULL == pcts) {
        if (NULL != f) {
            if ((unsigned char)0U == civil_time_normalize_initialized)
                initialize_civil_time_normalize();
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL source pointer",
                __func__, source_file, __LINE__
             );
        }
        return;
    }
    while (pcts->fraction >= 1.0) {
        adjustment++;
        pcts->fraction -= 1.0;
    }
    while (pcts->fraction < 0.0) {
        adjustment--;
        pcts->fraction += 1.0;
    }
    if (0 != adjustment)
        pcts->tm.tm_sec += adjustment;
    /* save zone offset */
    offset = pcts->offset;
    /* translate to UTC for struct tm normalization */
    if (0.0 != offset)
        civil_time_zone_change(pcts, 0.0, f, log_arg);
    normalize_date_time(&(pcts->tm), f, log_arg);
    /* restore zone */
    if (0.0 != offset)
        civil_time_zone_change(pcts, offset, f, log_arg);
}

/* Translate a date-time expressed in some local zone (offset from UTC) to
   an equivalent in a different zone (offset from UTC).
   In particular, specifying a new_offset of 0.0 translates to UTC.
*/
/* calls: civil_time_normalize, adjust_minute, adjust_hour */
/* called by: civil_time_normalize, civil_time_cmp, civil_time_difference, parse_civil_time_text, sn_date_time */
void civil_time_zone_change(struct civil_time_struct *pcts, double new_offset,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int adjustment = 0, new_value;
#ifndef PP__FUNCTION__
    static const char __func__[] = "civil_time_zone_change";
#endif

    if (NULL == pcts) {
        if (NULL != f) {
            if ((unsigned char)0U == civil_time_normalize_initialized)
                initialize_civil_time_normalize();
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL source pointer",
                __func__, source_file, __LINE__
             );
        }
        return;
    }
    /* first translate original to UTC (prior to normalization) */
    while (pcts->offset >= 3600.0) {
        pcts->offset -= 3600.0;
        adjustment--;
    }
    while (pcts->offset <= -3600.0) {
        pcts->offset += 3600.0;
        adjustment++;
    }
    if (0 != adjustment) {
        adjust_hour(&(pcts->tm), adjustment, f, log_arg);
        adjustment = 0;
    }
    while (pcts->offset >= 60.0) {
        pcts->offset -= 60.0;
        adjustment--;
    }
    while (pcts->offset <= -60.0) {
        pcts->offset += 60.0;
        adjustment++;
    }
    if (0 != adjustment) {
        adjust_minute(&(pcts->tm), adjustment, f, log_arg);
        adjustment = 0;
    }
    while (pcts->offset >= 1.0) {
        pcts->offset -= 1.0;
        adjustment--;
    }
    while (pcts->offset < 0.0) {
        pcts->offset += 1.0;
        adjustment++;
    }
    pcts->fraction -= pcts->offset;
    pcts->offset = 0.0;
    /* now have UTC (zero offset) */
    civil_time_normalize(pcts, f, log_arg);
    /* apply new offset */
    if (0.0 != new_offset) {
        pcts->offset = new_offset;
        /* apply in pieces, largest to smallest */
        while (new_offset >= 3600.0) {
            new_offset -= 3600.0;
            adjustment++;
        }
        while (new_offset <= -3600.0) {
            new_offset += 3600.0;
            adjustment--;
        }
        if (0 != adjustment) {
            adjust_hour(&(pcts->tm), adjustment, f, log_arg);
            adjustment = 0;
        }
        while (new_offset >= 60.0) {
            new_offset -= 60.0;
            adjustment++;
        }
        while (new_offset <= -60.0) {
            new_offset += 60.0;
            adjustment--;
        }
        if (0 != adjustment) {
            adjust_minute(&(pcts->tm), adjustment, f, log_arg);
#if 0 /* adjustment is no longer needed */
            adjustment = 0;
#endif
        }
        /* prepare to adjust seconds (including fraction) */
        new_value = pcts->tm.tm_sec;
        while (new_offset >= 1.0) {
            new_offset -= 1.0;
            new_value--;
        }
        while (new_offset < 0.0) {
            new_offset += 1.0;
            new_value++;
        }
        /* adjust fraction and revise seconds adjustment if necessary */
        pcts->fraction += new_offset;
        while (pcts->fraction >= 1.0) {
            new_value++;
            pcts->fraction -= 1.0;
        }
        while (pcts->fraction < 0.0) {
            new_value--;
            pcts->fraction += 1.0;
        }
        /* seconds and adjustment to minutes */
        pcts->tm.tm_sec = new_value;
    }
}
