/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for generating build information strings
*/
#ifndef ZZ_BUILD_STR_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    zz_build_str.h copyright 2015 - 2016 Bruce Lilly.   \ zz_build_str.h $
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
/* $Id: ~|^` @(#)   This is zz_build_str.h version 1.15 2017-11-21T05:23:23Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "zz_build_str" */
/*****************************************************************************/
/* maintenance note: master file  /src/relaymail/include/s.zz_build_str.h */

/* version-controlled header file version information */
#define ZZ_BUILD_STR_H_VERSION "zz_build_str.h 1.15 2017-11-21T05:23:23Z"

/* Some preprocessors and/or compilers generate strings for __func__ or __FUNCTION__ */
/* ANSI/ISO C99 defines __func__ as a pre-defined identifier which
   evaluates to the unadorned name of the enclosing function.
*/
/* When implemented independent of __func__, __FUNCTION__ is typically a string
   constant.  However, it might merely be a synonym for __func__ as defined by
   ANSI/ISO C99.
*/
/* NetBSD cdefs.h defines macro __func__ as __PRETTY_FUNCTION__ leading to
   "undefined behavior", viz. clang in pre-C99 modes gets __PRETTY_FUNCTION__
   instead of its internal __func__ during preprocessing.  __PRETTY_FUNCTION__
   as defined by clang is incompatible with the gcc definition, resulting in
   undesirable excess adornment of the function name.
   System header files shouldn't redefine __func__:
     1. It is known to lead to undefined behaivior:
        [ISO 9899:1999 7.1.3 (2), also J.2, specifically
        "The identifier _ _func_ _ is explicitly declared (6.4.2.2)."]
     2. It precludes addressing the compiler issue *via* the compiler, as e.g.
        `gcc -std=c90 -D__func__=__FUNCTION__`
     netbsd bug standards/51044, clang/llvm bug #27189
   This header file does not define or undefine __func__, as that would
   lead to the same sort of problems as those cause by NetBSD cdefs.h.
   Compiler workarounds:
     1. for gcc modes -std=c90, -std=gnu90, -std=iso9899:199409
        add -D__func__=__FUNCTION__
        Note: NetBSD cdefs.h overrides this on NetBSD, but gcc manages OK
     2. for clang modes -std=c89, -std=gnu89 -std=c94
        add -D__PRETTY_FUNCTION__=__FUNCTION__
        Note: this is strictly only required on NetBSD to undo the damage
          done by NetBSD cdefs.h
*/
/* PP_FUNCTION__ is defined if __func__ is usable; it is undefined
   if there is no __func__ available.
*/
#undef PP__FUNCTION__
/* __STDPP__ */
#ifndef PP__FUNCTION__
# if defined(__STDPP__)
#  define PP__FUNCTION__
# endif
#endif
/* clang */
#ifndef PP__FUNCTION__
# if defined (__clang__)
#  define PP__FUNCTION__
# endif
#endif
/* __GNUC__ >= 2 */
#ifndef PP__FUNCTION__
# if ( defined(__GNUC__) && (__GNUC__ >= 2))
#  define PP__FUNCTION__
# endif
#endif
/* __INTEL_COMPILER */
#ifndef PP__FUNCTION__
# if defined(__INTEL_COMPILER)
#  define PP__FUNCTION__
# endif
#endif
/* __SUNPRO_C */
#ifndef PP__FUNCTION__
# if defined(__SUNPRO_C)
#  define PP__FUNCTION__
# endif
#endif
/* __STDC__, __STDC_VERSION__ >= 199001L */
#ifndef PP__FUNCTION__
# if ( defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199001L))
#  define PP__FUNCTION__
# endif
#endif
/* lint et. al. */
#ifndef PP__FUNCTION__
# if ( defined(lint) || defined(__lint__) || defined(__LINT__) || defined(__lint) || defined(__LCLINT__) || defined(S_SPLINT_S))
#  define PP__FUNCTION__
# endif
#endif

/* boilerplate fields used for object file identification string generation: */
/* stringification macros needed for __SUNPRO_C */
#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s
/* compiler used: */
#undef COMPILER_USED
#if defined (__PCC__)
# if defined(__VERSION__)
#  define COMPILER_USED "pcc version " __VERSION__
# else
#  define COMPILER_USED "pcc version " __PCC__ "." __PCC_MINOR__ "." __PCC_MINORMINOR__
# endif
#else
# if defined (__clang__)
#  if defined(__VERSION__)
#   define COMPILER_USED "clang version " __VERSION__
#  else
#   define COMPILER_USED "clang version " __clang_version__
#  endif
# else
#  if defined (__INTEL_COMPILER)
#   if defined(__VERSION__)
#   define COMPILER_USED "Intel compiler version " __VERSION__
#   else
#   define COMPILER_USED "Intel compiler version " __INTEL_COMPILER
#   endif
#  else
#   if defined(__SUNPRO_C)
#    if defined(__VERSION__)
#   define COMPILER_USED "SunPro C version " __VERSION__
#    else
#   define COMPILER_USED "SunPro C version " xbuildstr(__SUNPRO_C)
#    endif
#   else
#    if defined(__GNUC__)
#   define COMPILER_USED "gcc version " __VERSION__
#    else
#   define COMPILER_USED "unknown compiler"
#    endif
#   endif
#  endif
# endif
#endif
/* date-time and other boilerplate */
#ifndef ZONE_OFFSET
# define ZONE_OFFSET "(unknown zone)"
#endif
#undef DATE_TIME_COMPILED
#define DATE_TIME_COMPILED __TIME__ ZONE_OFFSET " " __DATE__
#undef ID_STRING_SUFFIX
#define ID_STRING_SUFFIX "\\ $"
#undef WHERE_COMPILED
#ifdef HOST_FQDN
# ifdef OS
#  ifdef OSVERSION
#   ifdef DISTRIBUTION
#    define OSDETAILS " running " OS " version " OSVERSION " " DISTRIBUTION
#   else
#    define OSDETAILS " running " OS " version " OSVERSION
#   endif
#  else
#   define OSDETAILS " running " OS
#  endif
# else
#  define OSDETAILS ""
# endif
# define WHERE_COMPILED " on host " HOST_FQDN OSDETAILS
#else
# define WHERE_COMPILED ""
#endif
/* feature test and similar compilation conditions */
#undef COMPILATION_FEATURES
#ifdef __STDC__
# ifdef __STDC_VERSION__
#  ifdef _XOPEN_SOURCE
#   ifdef POSIX_C_SOURCE
#    define COMPILATION_FEATURES " with __STDC__, __STDC_VERSION__=" xbuildstr(__STDC_VERSION__) ", _XOPEN_SOURCE=" xbuildstr(_XOPEN_SOURCE) ", POSIX_C_SOURCE=" xbuildstr(POSIX_C_SOURCE)
#   else /* POSIX_C_SOURCE */
#    define COMPILATION_FEATURES " with __STDC__, __STDC_VERSION__=" xbuildstr(__STDC_VERSION__) ", _XOPEN_SOURCE=" xbuildstr(_XOPEN_SOURCE)
#   endif /* POSIX_C_SOURCE */
#  else /* _XOPEN_SOURCE */
#   ifdef POSIX_C_SOURCE
#    define COMPILATION_FEATURES " with __STDC__, __STDC_VERSION__=" xbuildstr(__STDC_VERSION__) ", POSIX_C_SOURCE=" xbuildstr(POSIX_C_SOURCE)
#   else /* POSIX_C_SOURCE */
#    define COMPILATION_FEATURES " with __STDC__, __STDC_VERSION__=" xbuildstr(__STDC_VERSION__)
#   endif /* POSIX_C_SOURCE */
#  endif /* _XOPEN_SOURCE */
# else /* __STDC_VERSION__ */
#  ifdef _XOPEN_SOURCE
#   ifdef POSIX_C_SOURCE
#    define COMPILATION_FEATURES " with __STDC__, _XOPEN_SOURCE=" xbuildstr(_XOPEN_SOURCE) ", POSIX_C_SOURCE=" xbuildstr(POSIX_C_SOURCE)
#   else /* POSIX_C_SOURCE */
#    define COMPILATION_FEATURES " with __STDC__, _XOPEN_SOURCE=" xbuildstr(_XOPEN_SOURCE)
#   endif /* POSIX_C_SOURCE */
#  else /* _XOPEN_SOURCE */
#   ifdef POSIX_C_SOURCE
#    define COMPILATION_FEATURES " with __STDC__, POSIX_C_SOURCE=" xbuildstr(POSIX_C_SOURCE)
#   endif /* POSIX_C_SOURCE */
#  endif /* _XOPEN_SOURCE */
# endif /* __STDC_VERSION__ */
#else /* __STDC__ */
# ifdef _XOPEN_SOURCE
#  ifdef POSIX_C_SOURCE
#   define COMPILATION_FEATURES " with _XOPEN_SOURCE=" xbuildstr(_XOPEN_SOURCE) ", POSIX_C_SOURCE=" xbuildstr(POSIX_C_SOURCE)
#  else /* POSIX_C_SOURCE */
#   define COMPILATION_FEATURES " with _XOPEN_SOURCE=" xbuildstr(_XOPEN_SOURCE)
#  endif /* POSIX_C_SOURCE */
# else /* _XOPEN_SOURCE */
#  ifdef POSIX_C_SOURCE
#   define COMPILATION_FEATURES " with POSIX_C_SOURCE=" xbuildstr(POSIX_C_SOURCE)
#  endif /* POSIX_C_SOURCE */
# endif /* _XOPEN_SOURCE */
#endif /* __STDC__ */
#ifndef COMPILATION_FEATURES
# define COMPILATION_FEATURES ""
#endif
#ifndef ID_STRING_PREFIX
# define ID_STRING_PREFIX "$Id: ~|^` @(#)"
#endif

/* copyright/build/identification information string */
static const char *copyright_id = ID_STRING_PREFIX
#ifdef SOURCE_MODULE
 SOURCE_MODULE " "
#endif
#if defined(COPYRIGHT_DATE) || defined(COPYRIGHT_HOLDER)
 "copyright"
# ifdef COPYRIGHT_DATE
 " " COPYRIGHT_DATE
# endif
# ifdef COPYRIGHT_HOLDER
 " " COPYRIGHT_HOLDER
# endif
 ". "
#endif
#ifdef MODULE_VERSION
 "Version " MODULE_VERSION " "
# ifdef MODULE_DATE
 "dated " MODULE_DATE " "
# endif
#endif
"compiled " DATE_TIME_COMPILED WHERE_COMPILED " by " COMPILER_USED COMPILATION_FEATURES ID_STRING_SUFFIX ;

/* remove stringification macros before executable code and other file inclusion
   (to avoid clashes with header file macros, function and variable names)
*/
#undef buildstr
#undef xbuildstr

/* local header file versions used for build */
/* check all known local header files */
static const char *build_id = ID_STRING_PREFIX
#ifdef SOURCE_MODULE
 SOURCE_MODULE " "
#endif
 "compiled with header versions "
#undef NOT_FIRST
#ifdef ADDRESS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ADDRESS_H_VERSION
#endif
#ifdef ANGLE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ANGLE_H_VERSION
#endif
#ifdef ASTRO_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ASTRO_H_VERSION
#endif
#ifdef AUTOBAUD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 AUTOBAUD_H_VERSION
#endif
#ifdef BOUNDARY_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 BOUNDARY_H_VERSION
#endif
#ifdef CHECK_DISP_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 CHECK_DISP_H_VERSION
#endif
#ifdef CIVIL_TIME_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 CIVIL_TIME_H_VERSION
#endif
#ifdef COMPARE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 COMPARE_H_VERSION
#endif
#ifdef CONFIG_THREAD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 CONFIG_THREAD_H_VERSION
#endif
#ifdef CONFIGURE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 CONFIGURE_H_VERSION
#endif
#ifdef DATE_TIME_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 DATE_TIME_H_VERSION
#endif
#ifdef DAYS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 DAYS_H_VERSION
#endif
#ifdef DOMAIN_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 DOMAIN_H_VERSION
#endif
#ifdef DSN_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 DSN_H_VERSION
#endif
#ifdef ENCODING_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ENCODING_H_VERSION
#endif
#ifdef ERRORS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ERRORS_H_VERSION
#endif
#ifdef EXCHANGE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 EXCHANGE_H_VERSION
#endif
#ifdef FIELD_REMEDY_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 FIELD_REMEDY_H_VERSION
#endif
#ifdef GET_HOST_NAME_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 GET_HOST_NAME_H_VERSION
#endif
#ifdef GET_IPS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 GET_IPS_H_VERSION
#endif
#ifdef GRAPHING_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 GRAPHING_H_VERSION
#endif
#ifdef HEADER_BODY_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 HEADER_BODY_H_VERSION
#endif
#ifdef INDENT_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 INDENT_H_VERSION
#endif
#ifdef INDIRECT_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 INDIRECT_H_VERSION
#endif
#ifdef IP_DECL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 IP_DECL_H_VERSION
#endif
#ifdef JD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 JD_H_VERSION
#endif
#ifdef LEAP_SECOND_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEAP_SECOND_H_VERSION
#endif
#ifdef LEMD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEMD_H_VERSION
#endif
#ifdef LEMD_CONFIG_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEMD_CONFIG_H_VERSION
#endif
#ifdef LEMD_DATA_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEMD_DATA_H_VERSION
#endif
#ifdef LEMD_GRAPH_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEMD_GRAPH_H_VERSION
#endif
#ifdef LEMD_POLL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEMD_POLL_H_VERSION
#endif
#ifdef LEMD_RECV_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LEMD_RECV_H_VERSION
#endif
#ifdef LIST_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LIST_H_VERSION
#endif
#ifdef LOGGER_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 LOGGER_H_VERSION
#endif
#ifdef MDN_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 MDN_H_VERSION
#endif
#ifdef MID_RANK_PIVOT_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 MID_RANK_PIVOT_H_VERSION
#endif
#ifdef MJD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 MJD_H_VERSION
#endif
#ifdef MONTHS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 MONTHS_H_VERSION
#endif
#ifdef MSG_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 MSG_H_VERSION
#endif
#ifdef NEWSGROUP_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 NEWSGROUP_H_VERSION
#endif
#ifdef NMEA_THREAD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 NMEA_THREAD_H_VERSION
#endif
#ifdef PARAMETERS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PARAMETERS_H_VERSION
#endif
#ifdef PARSE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PARSE_H_VERSION
#endif
#ifdef PATH_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PATH_H_VERSION
#endif
#ifdef PATHS_DECL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PATHS_DECL_H_VERSION
#endif
#ifdef PHRASE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PHRASE_H_VERSION
#endif
#ifdef PI_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PI_H_VERSION
#endif
#ifdef POOL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 POOL_H_VERSION
#endif
#ifdef PORTS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PORTS_H_VERSION
#endif
#ifdef PPS_THREAD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 PPS_THREAD_H_VERSION
#endif
#ifdef QUEUE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 QUEUE_H_VERSION
#endif
#ifdef QUICKSELECT_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 QUICKSELECT_H_VERSION
#endif
#ifdef QVALUE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 QVALUE_H_VERSION
#endif
#ifdef RANDOM64N_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RANDOM64N_H_VERSION
#endif
#ifdef RE_CONFIG_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RE_CONFIG_H_VERSION
#endif
#ifdef RECEIVED_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RECEIVED_H_VERSION
#endif
#ifdef REPEATED_MEDIAN_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 REPEATED_MEDIAN_H_VERSION
#endif
#ifdef RESOLV_DECL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RESOLV_DECL_H_VERSION
#endif
#ifdef RFC2369_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RFC2369_H_VERSION
#endif
#ifdef RFC2822_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RFC2822_H_VERSION
#endif
#ifdef RFC3066_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RFC3066_H_VERSION
#endif
#ifdef RFC821_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 RFC821_H_VERSION
#endif
#ifdef SERIAL_PINS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 SERIAL_PINS_H_VERSION
#endif
#ifdef SHUFFLE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 SHUFFLE_H_VERSION
#endif
#ifdef SIGNALS_DECL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 SIGNALS_DECL_H_VERSION
#endif
#ifdef SIMPLE_CONFIG_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 SIMPLE_CONFIG_H_VERSION
#endif
#ifdef SNN_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 SNN_H_VERSION
#endif
#ifdef SORTED_ARRAY_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 SORTED_ARRAY_H_VERSION
#endif
#ifdef STATUS_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 STATUS_H_VERSION
#endif
#ifdef STRING_DECL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 STRING_DECL_H_VERSION
#endif
#ifdef STRNTOL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 STRNTOL_H_VERSION
#endif
#ifdef THERMOSTATD_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 THERMOSTATD_H_VERSION
#endif
#ifdef THERMOSTATD_CONFIG_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 THERMOSTATD_CONFIG_H_VERSION
#endif
#ifdef THERMOSTATD_DATA_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 THERMOSTATD_DATA_H_VERSION
#endif
#ifdef THERMOSTATD_GRAPH_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 THERMOSTATD_GRAPH_H_VERSION
#endif
#ifdef THERMOSTATD_POLL_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 THERMOSTATD_POLL_H_VERSION
#endif
#ifdef THERMOSTATD_RECV_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 THERMOSTATD_RECV_H_VERSION
#endif
#ifdef TIMESPEC_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 TIMESPEC_H_VERSION
#endif
#ifdef TOKEN_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 TOKEN_H_VERSION
#endif
#ifdef TOKEN_REMEDY_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 TOKEN_REMEDY_H_VERSION
#endif
#ifdef TRIM_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 TRIM_H_VERSION
#endif
#ifdef TRIPARTITION_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 TRIPARTITION_H_VERSION
#endif
#ifdef URI_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 URI_H_VERSION
#endif
#ifdef USAGE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 USAGE_H_VERSION
#endif
#ifdef UTC_MKTIME_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 UTC_MKTIME_H_VERSION
#endif
#ifdef X400ACT_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 X400ACT_H_VERSION
#endif
#ifdef ZONE_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ZONE_H_VERSION
#endif
#ifdef ZZ_BUILD_STR_H_VERSION
# ifdef NOT_FIRST
 ", "
# else
#  define NOT_FIRST
# endif
 ZZ_BUILD_STR_H_VERSION
#endif
ID_STRING_SUFFIX ;
#undef NOT_FIRST

static char build_strings_registered = (char)0;

#ifndef NULL
# include <stddef.h>            /* NULL */
#endif

/* The primary function of this function is to ensure that
   overzealous compiler optimization does not discard
   copyright/version and build strings from generated object files;
   the strings are needed for identification for bug reports, etc.
   A secondary function is to skip over initial relative path components
   in the caller's view of the pre-defined string constant __FILE__.
   Args:
     p  additional caller-supplied string to protect
     q  pointer to char * to hold pointer to start of sanitized __FILE__ content
     r  pointer to caller-space buffer holding compilation __FILE__
*/
static char register_build_strings(const char *p, const char **q, const char *r)
{
    const char *s;

    if (NULL != p)
        for (s=p; '\0'!=*s; s++)
            build_strings_registered |= *s;
    for (s=copyright_id; '\0'!=*s; s++)
        build_strings_registered |= *s;
    for (s=build_id; '\0'!=*s; s++)
        build_strings_registered |= *s;
    if ((NULL != q) && (NULL != r)) {
        for (s=r; '\0'!=*s; s++) {
            switch (*s) {
                case '.': /*FALLTHROUGH*/
                case '/': /*FALLTHROUGH*/
                case '\0': /* shouldn't happen; in place of another FALLTHROUGH */
                break;
                default:
                    *q = s;
                return build_strings_registered;
            }
        }
        *q = s;
    }
    return build_strings_registered;
}

#define ZZ_BUILD_STR_H_INCLUDED
#endif
