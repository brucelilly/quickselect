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
* $Id: ~|^` @(#)    klimits_src.h copyright 2017 Bruce Lilly.   \ klimits_src.h $
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
/* $Id: ~|^` @(#)   This is klimits_src.h version 1.3 dated 2017-12-22T04:14:04Z. \ $ */
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
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
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
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2017-12-22T04:14:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"

/* local header files needed */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

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
/* declaration */
#include "klimits_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* static data */
static char klimits_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* initialize at run-time */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void initialize_klimits(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s) s = filenamebuf; else s++;
    klimits_initialized = register_build_strings(NULL, &source_file, s);
}

/* klimits: find range of order statistic ranks corresponding to sub-array
            limits.
*/
/* called from quickselect_loop and sampling_table */
#if QUICKSELECT_BUILD_FOR_SPEED
static QUICKSELECT_INLINE
#endif
#include "klimits_decl.h"
{
    size_t s, l, lk, rk;

    A(NULL!=pfk);A(NULL!=pbk);A(NULL!=pk);
#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_klimits();
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
    /* binary search through pk to find limits for each region */
    for (s=firstk,l=beyondk,lk=s+((l-s)>>1); s<l; lk=s+((l-s)>>1)) {
        A(lk>=firstk);A(lk<=beyondk);
        if (pk[lk]<first) s=lk+1UL; else l=lk;
    }
    A(lk>=firstk);A(lk<=beyondk);
    for (s=lk,l=beyondk,rk=s+((l-s)>>1); s<l; rk=s+((l-s)>>1)) {
        A(rk>=firstk);A(rk<=beyondk);
        if (pk[rk]<beyond) s=rk+1UL; else l=rk;
    }
    A(lk>=firstk);A(lk<=beyondk);A(rk>=firstk);A(rk<=beyondk);
    A((lk==beyondk)||(pk[lk]>=first));A((rk==beyondk)||(pk[rk]>=beyond));
    *pfk=lk, *pbk=rk;
}
