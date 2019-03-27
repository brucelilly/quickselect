/*INDENT OFF*/

/* Description: C source code for klimits */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    klimits_src.h copyright 2017-2019 Bruce Lilly.   \ klimits_src.h $
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
/* $Id: ~|^` @(#)   This is klimits_src.h version 1.9 dated 2019-03-26T20:41:26Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.klimits_src.h */

/********************** Long description and rationale: ***********************
 An implementation of multiple quickselect similar to that described by Mahmoud
 & Lent in "Average-case analysis of multiple Quickselect: An algorithm for
 finding order statistics".  The array of items to be sorted by quickselect,
 the array element size, and the comparison function are declared as for qsort,
 with two additional parameters to specify an optional array of desired order
 statistic ranks.
 N.B. the values for the order-statistic array elements are zero-based ranks
 (i.e. 0UL for the minimum, N-1UL for the maximum of an array of N elements,
 (N-1UL)/2UL and N/2UL for the lower- and upper-medians).
 The implementation can also be used to implement qsort; a wrapper function
 (which could alternatively be implemented as a macro) is provided for that
 purpose.
 In order to be able to compute the rank of array elements for selection, the
 initial array base is maintained, and a pair of indices brackets the sub-array
 being processed.  The indices correspond to the ranks of elements.  A similar
 scheme is used by Musser's introsort, described in "Introspective Sorting and
 Selection Algorithms".  The same method is used for the array of desired order
 statistic ranks, similar to that described by Lent & Mahmoud.
******************************************************************************/

/* Nothing to configure below this line. */

/* Minimum _XOPEN_SOURCE version for C99 (else illumos compilation fails) */
#undef MAX_XOPEN_SOURCE_VERSION
#undef MIN_XOPEN_SOURCE_VERSION
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) && defined(MIN_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && defined(MAX_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components
   are version control fields.
   ID_STRING_PREFIX is suitable for the what(1) and ident(1) utilities.
   MODULE_DATE uses modern SCCS extensions.
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: klimits_src.h ~|^` @(#)"
#define SOURCE_MODULE "klimits_src.h"
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2019-03-26T20:41:26Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2019"

#if !defined(QUICKSELECT_BUILD_FOR_SPEED) || !defined(QUICKSELECT_INLINE)
# include "quickselect_config.h"
#endif

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* local header files needed */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */
#endif

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <stddef.h>             /* size_t NULL */
#include <string.h>             /* strrchr */

/* macros */
/* space-saving abbreviations */
#undef V
#define V void
#undef A
#define A(me) assert(me)

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* static data */
static char klimits_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file;

/* initialize at run-time */
static QUICKSELECT_INLINE
void initialize_klimits(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s) s = filenamebuf; else s++;
    klimits_initialized = register_build_strings(NULL, &source_file, s);
}
#else
extern char klimits_initialized;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* klimits: find range of order statistic ranks corresponding to sub-array
            limits.
*/
QUICKSELECT_KLIMITS
{
    size_t h, l, lk, rk;

    A(NULL!=pfk);A(NULL!=pbk);A(NULL!=pk);
#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==klimits_initialized) initialize_klimits();
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#if LIBMEDIAN_TEST_CODE
    if (DEBUGGING(REPARTITION_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: firstk=%lu, pk[%lu]=%lu, beyondk=%lu, "
            "pk[%lu]=%lu, first=%lu, beyond=%lu */\n",
            __func__,source_file,__LINE__,(unsigned long)firstk,
            (unsigned long)firstk,(unsigned long)pk[firstk],
            (unsigned long)beyondk,(unsigned long)beyondk-1UL,
            (unsigned long)pk[beyondk-1UL],(unsigned long)first,
            (unsigned long)beyond);
    }
#endif
    /* binary search through pk to find limits for each region */
    for (l=firstk,h=beyondk,lk=BS_MID_L(l,h); l<h; lk=BS_MID_L(l,h)) {
        A(lk>=firstk);A(lk<beyondk);
        if (pk[lk]<first) l=lk+1UL; else h=lk;
    }
    A(lk>=firstk);A(lk<=beyondk);
    for (l=lk,h=beyondk,rk=BS_MID_L(l,h); l<h; rk=BS_MID_L(l,h)) {
        A(rk>=firstk);A(rk<beyondk);
        if (pk[rk]<beyond) l=rk+1UL; else h=rk;
    }
    A(lk>=firstk);A(lk<=beyondk);A(rk>=firstk);A(rk<=beyondk);
    A((lk==beyondk)||(pk[lk]>=first));A((rk==beyondk)||(pk[rk]>=beyond));
    *pfk=lk, *pbk=rk;
#if LIBMEDIAN_TEST_CODE
    if (DEBUGGING(REPARTITION_DEBUG)) {
        if (rk>lk) (V)fprintf(stderr,
            "/* %s: %s line %d: firstk=%lu, lk=%lu, pk[%lu]=%lu, rk=%lu, "
            "pk[%lu]=%lu, beyondk=%lu, first=%lu, beyond=%lu */\n",
            __func__,source_file,__LINE__,(unsigned long)firstk,
            (unsigned long)lk,(unsigned long)lk,(unsigned long)(pk[lk]),
            (unsigned long)rk,(unsigned long)rk-1UL,(unsigned long)(pk[rk-1UL]),
            (unsigned long)beyondk,(unsigned long)first,(unsigned long)beyond);
        else (V)fprintf(stderr,
            "/* %s: %s line %d: firstk=%lu, pk[%lu]=%lu, lk=%lu, rk=%lu, "
            "beyondk=%lu, first=%lu, beyond=%lu */\n",
            __func__,source_file,__LINE__,(unsigned long)firstk,
            (unsigned long)firstk,(unsigned long)pk[firstk],(unsigned long)lk,
            (unsigned long)rk,(unsigned long)beyondk,(unsigned long)first,
            (unsigned long)beyond);
    }
#endif
}
