/*INDENT OFF*/

/* Description: C source code for quickselect */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    repivot_src.h copyright 2017-2018 Bruce Lilly.   \ repivot_src.h $
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
/* $Id: ~|^` @(#)   This is repivot_src.h version 1.10 dated 2018-04-15T03:18:38Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.repivot_src.h */

/********************** Long description and rationale: ***********************
 Partitioning can result in an unfavorable split of the initial sub-array;
 when this happens, an emergency "break-glass" pivot selection function is used
 to ensure a better split, avoiding quadratic behavior even when handling a
 worst-case input array, such as can be generated by an adversary function
 (M. D. McIlroy "A Killer Adversary for Quicksort").
 The function in this file examines the partition result and determines whether
 or not repartitioning of the large region using a break-glass pivot selection
 method is warranted.
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
#if defined(_XOPEN_SOURCE) \
&& defined(MIN_XOPEN_SOURCE_VERSION) \
&& ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) \
&& defined(MAX_XOPEN_SOURCE_VERSION) \
&& ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
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
#define ID_STRING_PREFIX "$Id: repivot_src.h ~|^` @(#)"
#define SOURCE_MODULE "repivot_src.h"
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2018-04-15T03:18:38Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"

/* local header files needed */
#include "quickselect_config.h" /* SELECTION_MIN_REPIVOT */
#include "tables.h"             /* struct repivot_table_struct */
#if ! QUICKSELECT_BUILD_FOR_SPEED
#include "initialize_src.h"
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <stddef.h>             /* size_t NULL */

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
#include <limits.h>             /* PATH_MAX */
#include "paths_decl.h"         /* path_basename */
#endif

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* declaration */
#include "should_repivot_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
/* not static; referenced by inline functions */
char repivot_src_file[PATH_MAX];
char repivot_src_file_initialized=0;
#endif

/* Determine if repivoting is warranted. Return an integer indicating the
   appropriate pivot selection method: 
      0U or QUICKSELECT_RESTRICT_RANK
*/
QUICKSELECT_VISIBILITY QUICKSELECT_INLINE
#include "should_repivot_decl.h"
{
    size_t q, ratio;
    const struct repivot_table_struct *prt;

#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file(__FILE__);
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if ((char)0==repivot_src_file_initialized) {
        (V)path_basename(__FILE__,repivot_src_file,sizeof(repivot_src_file));
        repivot_src_file_initialized++;
    }
#endif
    A((SAMPLING_TABLE_SIZE)>table_index);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)
    ||DEBUGGING(SHOULD_REPIVOT_DEBUG))
        (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, "
            "table_index=%u, pk=%p */\n",__func__,repivot_src_file,
            __LINE__,nmemb,n,pst[table_index].samples,table_index,
            (const void *)pk);
#endif
    if (NULL==pk) { /* sorting */
        if (SELECTION_MIN_REPIVOT>n) return 0U;
        prt=sorting_repivot_table;
    } else { /* order statistic selection */
        /* no repivot unless >= 3 medians-of-3 */
        if (SELECTION_MIN_REPIVOT>n) return 0U;
        prt=selection_repivot_table;
    }
    A(n<nmemb);
    q=nmemb-n;
    A(0UL<q);
    if (n<=q) { /* ratio 0; return w/o incurring division cost */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(SHOULD_REPIVOT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu"
                ", table_index=%u, pk=%p, ppeq=%p, ratio<1, no repivot */\n",
                __func__,repivot_src_file,__LINE__,nmemb,n,
                pst[table_index].samples,table_index,(const void *)pk,
                (void *)ppeq);
#endif
        return 0U;
    }
    ratio=n/q;
#if defined(DEBUGGING)
    if ((NULL==ppeq)&&(DEBUGGING(RATIO_GRAPH_DEBUG)))
        /* for graphing worst-case partition ratios */
        if (ratio>stats_table[table_index].max_ratio)
            stats_table[table_index].max_ratio = ratio;
#endif
    if (table_index>repivot_table_size) {
        /* ratio from median-of-medians < 3 */
        if (3UL<=ratio) {
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
            if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)
            ||DEBUGGING(SHOULD_REPIVOT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, n=%lu, samples"
                    "=%lu, table_index=%u, pk=%p, ppeq=%p, ratio=%lu>=3, "
                    "repivot */\n",__func__,repivot_src_file,__LINE__,nmemb,n,
                    pst[table_index].samples,table_index,(const void *)pk,
                    (void *)ppeq,ratio);
#endif
            return QUICKSELECT_RESTRICT_RANK;
        }
    } else {
        A(0U<table_index);
        table_index--; /* repivot table has no entry for single sample */
        if ((ratio>=prt[table_index].factor1)
        || ((ratio>=prt[table_index].factor2)&&(++*pn2>=2))) {
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
            if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)
            ||DEBUGGING(SHOULD_REPIVOT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, n=%lu, "
                    "samples=%lu, table_index=%u, pk=%p, ppeq=%p, ratio=%lu"
                    ", factor1=%d, factor2=%d, count=%d, repivot */\n",
                    __func__,repivot_src_file,__LINE__,nmemb,n,
                    pst[table_index+1UL].samples,table_index+1UL,
                    (const void *)pk,(void *)ppeq,ratio,
                    prt[table_index].factor1,
                    prt[table_index].factor2,*pn2);
#endif
            return QUICKSELECT_RESTRICT_RANK;
        }
    }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (3UL<=ratio)
        if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(SHOULD_REPIVOT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, n=%lu, samples"
                "=%lu, table_index=%u, pk=%p, ppeq=%p, ratio=%lu, "
                "factor1=%d, factor2=%d, count=%d, no repivot */\n",
                __func__,repivot_src_file,__LINE__,nmemb,n,
                pst[table_index+1UL].samples,table_index+1UL,(const void *)pk,
                (void *)ppeq,ratio,prt[table_index].factor1,
                prt[table_index].factor2,*pn2);
#endif
    return 0U;
}
