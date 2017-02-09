/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for leap_second.c
*/
#ifndef LEAP_SECOND_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    leap_second.h copyright 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is leap_second.h version 2.4 2015-10-05T16:10:14Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "leap_second" */
/*****************************************************************************/
/* maintenance note: master file  /src/relaymail/lib/libleap_second/include/s.leap_second.h */

/* version-controlled header file version information */
#define LEAP_SECOND_H_VERSION "leap_second.h 2.4 2015-10-05T16:10:14Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* system header files needed for declarations */
#include <time.h>               /* struct tm */

/* leap_second.c function declarations */
#if defined(__cplusplus)
# define LEAP_SECOND_EXTERN extern "C"
#else
# define LEAP_SECOND_EXTERN extern
#endif

/* *INDENT-OFF* */
/*INDENT OFF*/

LEAP_SECOND_EXTERN int leap_second(const struct tm *, void (*)(int, void *, const char *, ...), void *);
LEAP_SECOND_EXTERN int leap_seconds_between(const struct tm *, const struct tm *, void (*)(int, void *, const char *, ...), void *);

/* some preprocessors and/or compilers generate strings for __func__ or __FUNCTION__ */
#undef PP__FUNCTION__
#if defined(__STDPP__) || ( defined(__GNUC__) && (__GNUC__ >= 2)) || defined(lint) || defined(__lint__) || defined(__LINT__) || defined(__lint) || defined(__LCLINT__) || defined(S_SPLINT_S) || defined(__INTEL_COMPILER) || defined(__SUNPRO_C) || ( defined (__STDC__) && (__STDC_VERSION >= 199901L))
# define PP__FUNCTION__
# if ( __STDC_VERSION__ < 199901L )
#  ifndef __func__
#   define __func__ __FUNCTION__
#  endif
# endif
#else
# undef  PP__FUNCTION__
#endif

/* boilerplate fields used for object file generation: */
/* stringification macros needed for __SUNPRO_C */
#undef xstr
#define xstr(s) str(s)
#undef str
#define str(s) #s
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
#   define COMPILER_USED "SunPro C version " xstr(__SUNPRO_C)
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
# define WHERE_COMPILED " on host " HOST_FQDN
#else
# define WHERE_COMPILED ""
#endif

/*INDENT ON*/
/* *INDENT-ON* */

#define LEAP_SECOND_H_INCLUDED
#endif
