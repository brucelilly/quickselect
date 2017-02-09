/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for civil_time_output functions
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    civil_time_output.c copyright 2009 - 2016 Bruce Lilly.   \ civil_time_output.c $
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
/* $Id: ~|^` @(#)   This is civil_time_output.c version 2.7 dated 2016-06-29T03:15:44Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "civil_time" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libcivil_time/src/s.civil_time_output.c */

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
* The text generation function calling synopsis is:
int sn_civil_time(char *buf, int sz, const struct civil_time_struct *pcts,
    int precision, int do_utc, int do_yday);
* Arguments:
*     buf       where to put the result (can be NULL to get size needed)
*     sz        how big is buf?
*     pcts      pointer to the structure which holds the local date-time value
*                  to sub-second resolution
*                  tm_wday, tm_isdst components are ignored
*                  other date-time component values must be normalized
*                     (e.g. from localtime or via utc_mktime)
*                  fraction of a second time; range 0.0 to < 1.0
*                  offset of local time zone in seconds (and fraction if
*                     needed) East of the UTC Prime Meridian
*                  component values are not altered by this function
*     precision resolution (decimal places) of fraction to use
*                  if negative, use only as many digits as required, but at
*                     most |precision| digits
*     do_utc    if non-zero, output time as UTC (else local time plus offset)
*     do_yday   if non-zero, use year-yday for date (else year-month-mday)
* Return value:
*      number of characters written to buf (not counting terminating '\0') if
*         buf was not NULL and sz was large enough; size required of buf
*         (including space for terminating '\0') if buf is NULL or sz is too
*         small
*      negative on error, with errno set
*   Example:
*      for pcts pointing to a structure with the following components:
*         tm_year 112 (i.e. calendar year 112 + 1900 = 2012)
*         tm_mon  1 (i.e. February)
*         tm_mday 29
*         tm_hour 3
*         tm_min  4
*         tm_sec  5
*         fraction 0.0123456789
*         offset 0.0
*      and precision = 6, do_utc = 1, do_yday = 0
*      buf (if large enough) will contain "2012-02-29T03:04:05.012346Z" and
*      the return value will be 27
*   Calls:
*      sn_date_time (see below), which calls snl and snf to do the drudge work
* Caveats: exercise caution when generating text; not all supported formats
*    permit all options.  While the functions in this package can read any of
*    the standard formats that it generates (and more), some other packages
*    might not be so flexible.
*    Setting do_utc to non-zero and do_yday to zero, with modest precision
*    should provide maximum compatibility.
*
* A utility function, sn_date_time provides additional text generation
*    flexibility; separator text between date elements, between date and time,
*    between time elements, between offset elements, and text for UTC zone
*    indication can be specified: the resulting text is not guaranteed to be
*    parseable by this or any other package if arbitrary text is used for any
*    of these items. sn_civil_time calls sn_date_time with appropriate text
*    separator and terminator strings.
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
#define ID_STRING_PREFIX "$Id: civil_time_output.c ~|^` @(#)"
#define SOURCE_MODULE "civil_time_output.c"
#define MODULE_VERSION "2.7"
#define MODULE_DATE "2016-06-29T03:15:44Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2009 - 2016"

/*INDENT ON*/
/* *INDENT-ON* */

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 600
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

/* local header files needed */
#include "civil_time.h"         /* header file for public definitions and declarations */ /* includes time.h */
#include "snn.h"                /* snl snf [sng] snmultiple */
#include "utc_mktime.h"         /* yday */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#ifndef EINVAL
# include <errno.h>             /* errno EINVAL */
#endif
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char civil_time_output_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* copyright notices are required to be visible! */
/* called by: civil_time_struct_check, civil_time_cmp, parse_civil_time_text */
static void initialize_civil_time_output(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    civil_time_output_initialized
        = register_build_strings(NULL, &source_file, s);
}

/* publicly exposed functions */

/* write extended complete date-time format with fractional
      seconds to a given buffer with specified size, with bounds checking
   date-time is given in struct tm pointed to by ptm
   time zone offset from UTC in minutes East of UTC given by offset argument
   fractional seconds given by fraction, which should be non-negative and < 1.0
      precision specifies the number of digits of fractional seconds
   Arguments:
      buf       where to put the result (can be NULL to get size needed)
      sz        how big is buf?
      pcts      pointer to the struct civil_time_struct which holds the
                   local date-time value
                   tm_wday, tm_isdst components are ignored
                   other date-time component values must be normalized
                      (e.g. from localtime or via utc_mktime)
                   component values are not altered by this function
      year_digits  the minimum number of digits to include for the year
      precision resolution (decimal places) of fraction to use
                   if negative, use only as many digits as required but
                   no more than the absolute value of the specified precision
                   e.g. -6 will produce at most 6 decimal places
      do_utc    if non-zero, output time as UTC (else local time plus offset)
      do_yday   use year-yday for date (else year-month-mday)
   Return value:
      number of characters written to buf (not counting terminating '\0') if
         buf was not NULL and sz was large enough; size required of buf
         (including space for terminating '\0') if buf is NULL or sz is too
         small
      negative on error, with errno set
   Example:
      for pcts pointing to a structure with the following components:
         tm_year 112 (i.e. calendar year 112 + 1900 = 2012)
         tm_mon  1 (i.e. February)
         tm_mday 29
         tm_hour 3
         tm_min  4
         tm_sec  5
         fraction = 0.0123456789
         and offset = 0.0
      and precision = 6, do_utc = 1, do_yday = 0
      and standard separators and indicators "-", "T", ":", "Z", ":"
      buf (if large enough) will contain "2012-02-29T03:04:05.012346Z" and
      the return value will be 27
   Calls:
      snl and snf to do the drudge work
*/
/* calls: civil_time_cpy, civil_time_zone_change, civil_time_normalize,
      snmultiple, snl, snf
*/
/* called by: sn_civil_time */
int sn_date_time(char *buf, int sz, const struct civil_time_struct *pcts,
    int year_digits, const char *date_sep, const char *date_time_sep,
    const char *time_sep, const char *utc_zone, const char *offset_sep,
    int precision, int do_utc, int do_yday,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int r = -1;
#ifndef PP__FUNCTION__
    static const char __func__[] = "sn_date_time";
#endif

    if (NULL != pcts) {
        char c = '?', *p = buf;
        int n, sz2 = sz;
        long x;
        double offset = pcts->offset;
        struct civil_time_struct cts;

        /* function argument sanity checks */
        if ((0.0 > pcts->fraction) || (1.000 <= pcts->fraction)) {
            if (NULL != f) {
                char buf2[32];      /* RATS: ignore (always used with sizeof()) */

                (void)sng(buf2, sizeof(buf2), NULL, NULL, pcts->fraction, 16, 3, f, log_arg);
                if ((unsigned char)0U == civil_time_output_initialized)
                    initialize_civil_time_output(); /* for source_file */
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: illegal fraction %s",
                    __func__, source_file, __LINE__
                    , buf2
                 );
            }
            errno = EINVAL;
            return r;
        }
        /* copy pcts input (may modify copy for normalization) */
        civil_time_cpy(pcts, &(cts), f, log_arg);
        /* apply offset to local time to get UTC for normalization */
        civil_time_zone_change(&cts, 0.0, f, log_arg);
        civil_time_normalize(&cts, f, log_arg); /* no special precautions needed for UTC */
        /* if not producing UTC output, reapply local time offset from UTC */
        if ((0 == do_utc) && (0.0 != offset)) {
            /* offset for text output needs to be a multiple of 60.0 seconds */
            offset = snmultiple(offset, 60.0, f, log_arg);
            civil_time_zone_change(&cts, offset, f, log_arg);
        }
        /* handle edge case: precision == 0, fraction >= 0.5 */
        if ((0 == precision) && (0.5 <= cts.fraction)) {
            cts.fraction = 0.0;
            cts.tm.tm_sec += 1;
            civil_time_normalize(&cts, f, log_arg); /* no special precautions needed for UTC */
        }
        /* compute yday if required */
        if (0 != do_yday) {
            cts.tm.tm_yday = yday(&(cts.tm), f, log_arg);   /* requires normalized year, month, mday */
        }
        /* year: 4 zero-padded digits */
        x = cts.tm.tm_year + 1900L;
        n = snl(p, sz2, NULL, NULL, x, 10, (int)'0', year_digits, f, log_arg);
        if (0 > n)
            return n;
        if ((NULL == buf) || (n > sz2))
            n--; /* undo count for terminating NUL */
        r = n;
        if ((NULL != buf) && (r < sz))
            p += n, sz2 -= n;
        if (0 != do_yday) {
            /* separator and yday */
            x = cts.tm.tm_yday + 1L;
            n = snl(p, sz2, date_sep, NULL, x, 10, (int)'0', 3, f, log_arg);
            if (0 > n)
                return n;
            if ((NULL == buf) || (n > sz2))
                n--; /* undo count for terminating NUL */
            r += n;
            if ((NULL != buf) && (r < sz))
                p += n, sz2 -= n;
        } else {
            /* separator and month */
            x = cts.tm.tm_mon + 1L;
            n = snl(p, sz2, date_sep, NULL, x, 10, (int)'0', 2, f, log_arg);
            if (0 > n)
                return n;
            if ((NULL == buf) || (n > sz2))
                n--; /* undo count for terminating NUL */
            r += n;
            if ((NULL != buf) && (r < sz))
                p += n, sz2 -= n;
            /* separator and day */
            x = cts.tm.tm_mday;
            n = snl(p, sz2, date_sep, NULL, x, 10, (int)'0', 2, f, log_arg);
            if (0 > n)
                return n;
            if ((NULL == buf) || (n > sz2))
                n--; /* undo count for terminating NUL */
            r += n;
            if ((NULL != buf) && (r < sz))
                p += n, sz2 -= n;
        }
        /* separator and hour */
        x = cts.tm.tm_hour;
        n = snl(p, sz2, date_time_sep, NULL, x, 10, (int)'0', 2, f, log_arg);
        if (0 > n)
            return n;
        if ((NULL == buf) || (n > sz2))
            n--; /* undo count for terminating NUL */
        r += n;
        if ((NULL != buf) && (r < sz))
            p += n, sz2 -= n;
        /* separator and minutes */
        x = cts.tm.tm_min;
        n = snl(p, sz2, time_sep, NULL, x, 10, (int)'0', 2, f, log_arg);
        if (0 > n)
            return n;
        if ((NULL == buf) || (n > sz2))
            n--; /* undo count for terminating NUL */
        r += n;
        if ((NULL != buf) && (r < sz))
            p += n, sz2 -= n;
        /* seconds */
        n = snl(p, sz2, time_sep, NULL, (long)(cts.tm.tm_sec), 10, (int)'0', 2, f, log_arg);
        if (0 > n)
            return n;
        if ((NULL == buf) || (n > sz2))
            n--; /* undo count for terminating NUL */
        /* save units digit; overwrite with zero for fraction, restore units digit */
        n--; /* don't count seconds units digit twice */
        r += n;
        if ((NULL != buf) && (r < sz)) {
            p += n, sz2 -= n;
            c = *p; /* seconds units digit */
        }
        /* decimal point (for POSIX, C execution environment compatibility) */
        /* and fraction digits */
        /* also UTC zone indicator as suffix if not producing offset */
        n = snf(p, sz2, NULL, 0!=do_utc? utc_zone: NULL, cts.fraction, (int)'0', 1, precision, f, log_arg);
        r += n;
        if ((NULL != buf) && (r-n < sz)) {
            *p = c; /* restore saved seconds units digit */
            if (r < sz)
                p += n, sz2 -= n;
        }
        /* offset */
        if (0 == do_utc) {
            int abs_off, sign;
            long hh, mm;

            if (0.0 <= cts.offset) {
                abs_off = ((int)(0.01 + cts.offset)) / 60;
                sign = 1;
            } else {
                abs_off = ((int)(0.01 - cts.offset)) / 60;
                sign = -1;
            }
            hh = abs_off / 60;
            mm = abs_off - hh * 60;
            if (0 > sign) {
                n = snl(p, sz2, "-", offset_sep, hh, 10, (int)'0', 2, f, log_arg);
            } else {
                n = snl(p, sz2, "+", offset_sep, hh, 10, (int)'0', 2, f, log_arg);
            }
            if (0 > n)
                return n;
            if ((NULL == buf) || (n > sz2))
                n--; /* undo count for terminating NUL */
            r += n;
            if ((NULL != buf) && (r < sz))
                p += n, sz2 -= n;
            n = snl(p, sz2, NULL, NULL, mm, 10, (int)'0', 2, f, log_arg);
            if (0 > n)
                return n;
            r += n;
        }
        /* snf or snl handled NUL terminator */
    } else {
        if (NULL != f) {
            if ((unsigned char)0U == civil_time_output_initialized)
                initialize_civil_time_output(); /* for source_file */
            f(LOG_ERR, log_arg,
                "%s: %s line %d: NULL source pointer",
                __func__, source_file, __LINE__
             );
        }
        errno = EINVAL;
    }
    return r;
}
