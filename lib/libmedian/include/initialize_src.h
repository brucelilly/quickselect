/*INDENT OFF*/
#ifndef	INITIALIZE_SRC_H_INCLUDED
#define	INITIALIZE_SRC_H_INCLUDED
/* Description: common C source code for qsort and qsort_s */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    initialize_src.h copyright 2017-2018 Bruce Lilly.   \ initialize_src.h $
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
/* $Id: ~|^` @(#)   This is initialize_src.h version 1.12 dated 2018-07-18T16:21:41Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.initialize_src.h */

/********************** Long description and rationale: ***********************
 Maintenance, specifically responding to bug reports, is facilitated if reports
 include sufficiently detailed information to enable replication of the reported
 behavior.  This file ensures that strings documenting compilation options are
 recorded in the object files in such a way as to permit their extraction using
 what(1) or ident(1) or strings(1) for inclusion with bug reports.  It also
 initializes a character string variable (source_file) which is a friendlier
 version of __FILE__ for use in diagnostics.
******************************************************************************/

/* Nothing to configure below this line. */

/* use ID_STRING_PREFIX, SOURCE_MODULE, MODULE_VERSION, etc. from caller */

/* local header files needed */
#include "quickselect_config.h"
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

/* system header files */
#include <stddef.h>             /* NULL */
#include <string.h>             /* strrchr */

/* macros */
/* stringification (don't change!) */
#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

/* static data (for version tracking for debugging/error reports) */
static char file_initialized=(char)0;
static const char *filenamebuf;
static const char *source_file= NULL;
static const char *file_build_options= ID_STRING_PREFIX SOURCE_MODULE " "
    MODULE_VERSION " built with configuration options: "
    "ASSERT_CODE=" xbuildstr(ASSERT_CODE)
#ifdef BUILD_NOTES
    ", BUILD_NOTES=" BUILD_NOTES
#endif
#if defined(DEBUG_CODE) && DEBUG_CODE
    ", DEBUG_CODE=" xbuildstr(DEBUG_CODE)
#endif
#ifdef FAVOR_SORTED
    ", FAVOR_SORTED=" xbuildstr(FAVOR_SORTED)
#endif
#if defined(GENERATOR_TEST) && GENERATOR_TEST
    ", GENERATOR_TEST=" xbuildstr(GENERATOR_TEST)
#endif
#if defined(QUICKSELECT_MAX_NETWORK) && QUICKSELECT_MAX_NETWORK
    ", QUICKSELECT_MAX_NETWORK=" xbuildstr(QUICKSELECT_MAX_NETWORK)
#endif
    ", QUICKSELECT_DEFAULT_CACHE_SIZE=" xbuildstr(QUICKSELECT_DEFAULT_CACHE_SIZE)
    ", QUICKSELECT_INDIRECT=" xbuildstr(QUICKSELECT_INDIRECT)
#if QUICKSELECT_STABLE
    ", QUICKSELECT_LINEAR_STABLE=" xbuildstr(QUICKSELECT_LINEAR_STABLE)
#endif /* QUICKSELECT_STABLE */
    ", QUICKSELECT_N_LINEAR=" xbuildstr(QUICKSELECT_N_LINEAR)
    ", QUICKSELECT_OPTIMIZE_COMPARISONS=" xbuildstr(QUICKSELECT_OPTIMIZE_COMPARISONS)
    ", QUICKSELECT_STABLE=" xbuildstr(QUICKSELECT_STABLE)
#if defined(SILENCE_WHINEY_COMPILERS) && SILENCE_WHINEY_COMPILERS
    ", SILENCE_WHINEY_COMPILERS=" xbuildstr(SILENCE_WHINEY_COMPILERS)
#endif
    ", built with " QUICKSELECT_CONFIG_H_VERSION
    "\\ $"
    ;

/* static functions */
/* initialization of static strings at run-time */
static QUICKSELECT_INLINE
void initialize_file(const char *file)
{
    const char *s;

    /* Initialize strings */
    filenamebuf=file;
    s=strrchr(filenamebuf,'/');
    if (NULL==s) s=filenamebuf; else s++;
    file_initialized=register_build_strings(file_build_options,&source_file,s);
}

#endif /* INITIALIZE_SRC_H_INCLUDED */
