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
* $Id: ~|^` @(#)    sampling_table_src.h copyright 2017 Bruce Lilly.   \ sampling_table_src.h $
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
/* $Id: ~|^` @(#)   This is sampling_table_src.h version 1.4 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.sampling_table_src.h */

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
 Pivot selection uses Rousseeuw & Basset's remedian, described in "The Remedian:
 A Robust Averaging Method for Large Data Sets", on a sample of a size which is
 a power of 3 taken from the array elements, with sample size approximately the
 square root of the array size as described in "Optimal Sampling Strategies for
 Quicksort" by McGeoch and Tygar, and in "Optimal Sampling Strategies in
 Quicksort and Quickselect" by Martinez and Roura.
 Elements sampled from the array for use by the pivot selection methods are
 chosen to provide good overall performance, avoiding choices that would fare
 poorly for commonly-seen input sequences (e.g. organ-pipe, already-sorted).
 Partitioning uses a method described by Bentley & McIlroy, with a modification
 to reduce disorder induced in arrays.
 Comparisons are made between array elements; there is no special-case code nor
 initialization macro.
 Partitioning can still result in an unfavorable split of the initial sub-array;
 when this happens, an emergency "break-glass" pivot selection function is used
 to ensure a better split, avoiding quadratic behavior even when handling a
 worst-case input array, such as can be generated by an adversary function
 (M. D. McIlroy "A Killer Adversary for Quicksort").
 Break-glass pivot selection uses median-of-medians using sets of 3 elements
 (ignoring leftovers) and using a call to quickselect to find the median of
 medians, rather than recursion.  Aside from those details, it is similar to the
 method described by Blum, Floyd, Pratt, Rivest, & Tarjan in "Time Bounds for
 Selection".
 Swapping of elements is handled by efficient inline swap functions.  No attempt
 is made to optimize for specific array element types, but swapping can be
 performed in units of basic language types.  The swap functions avoid useless
 work (e.g. when given two pointers to the same element).  No specialized macros
 are required.
******************************************************************************/

/* Nothing to configure below this line. */

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
#define ID_STRING_PREFIX "$Id: sampling_table_src.h ~|^` @(#)"
#define SOURCE_MODULE "sampling_table_src.h"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-12-22T04:14:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"

/* local header files needed */
#include "quickselect_config.h" /* quickselect QSORT_FUNCTION_NAME */
#include "tables.h"     /* sampling_table_struct SAMPLING_TABLE_SIZE */
#if ! QUICKSELECT_BUILD_FOR_SPEED
#include "initialize_src.h"
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <limits.h>             /* *_MAX */
#include <stddef.h>             /* size_t NULL */

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* klimits declaration */
#include "klimits_decl.h"
;

/* declaration */
#include "sample_index_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* sample_index: return index into sampling table psts (size stsz entries) for
   nmemb
*/
/* called from sampling_table and quicksort */
#if QUICKSELECT_BUILD_FOR_SPEED
static QUICKSELECT_INLINE
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#include "sample_index_decl.h"
{
    if (3UL>nmemb) idx=0U;
    else {
        A((psts==sorting_sampling_table) ||(psts==ends_sampling_table)
        ||(psts==middle_sampling_table));
        if (idx>=(SAMPLING_TABLE_SIZE)-1U) idx=(SAMPLING_TABLE_SIZE)-2U;
        while (nmemb>psts[idx].max_nmemb) idx++;
        A((SAMPLING_TABLE_SIZE)>idx);
        while (nmemb<=psts[idx-1U].max_nmemb) idx--;
    }
    A((SAMPLING_TABLE_SIZE)>idx);
    return idx;
}

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* declaration */
#include "sampling_table_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* sampling_table: select a suitable sampling table */
/* called from select_pivot, quickselect_loop and quickselect public wrapper
   function
*/
#if QUICKSELECT_BUILD_FOR_SPEED
static QUICKSELECT_INLINE
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#include "sampling_table_decl.h"
{
    unsigned char distribution, raw, sort=1U;
    struct sampling_table_struct *psts=sorting_sampling_table;

    A(NULL!=pindex);
#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file(__FILE__);
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
    if (NULL!=pk) { /* selection, not sorting */
        size_t bk, fk, m1, m2, nk, nr, qk, x;
        A(beyondk>firstk);
        nk=beyondk-firstk;
        /* Characterize distribution of desired order statistic ranks */
        raw=0UL; /* raw 000 */
        /* median(s): lower(m1) and upper (m2) */
        m1=first+((nmemb-1UL)>>1); m2=first+(nmemb>>1);
        x=(nmemb>>3)+1UL; /* nmemb/8 + 1 */
        klimits(m1-x,m2+x,pk,firstk,beyondk,&fk,&bk);
        nr = ((bk>fk)?bk-fk:0UL); /* ranks in [3/8-,5/8+) */
        if (0UL!=nr) raw|=2U;
        qk=nk>>2; /* 1/4 */
        if ((1UL<nk)&&(qk<=nr+1UL)&&(nr<=qk+1UL)) {
            /* nr OK for distributed ranks; check for ends,separated */
            klimits(first,m2+x,pk,firstk,beyondk,&fk,&bk);
            qk = ((bk>fk)?bk-fk:0UL); /* ranks in [0,3/8-) */
            if (0UL!=qk) raw|=4U;
            nr += qk; /* now ranks in [0,5/8+) */
            qk=(nk>>1)+(nk>>3); /* 5/8 */
            if ((qk<=nr+1UL)&&(nr<=qk+1UL))
                raw|=1U;
        }
        /* convert raw distribution to enumerated value */
        distribution=sampling_distribution_remap[raw];
        if (NULL==ppeq) {
            /* Worst-case sort of 3 elements costs 3 comparisons and at most
               2 swaps.  Worst-case selection of minimum or maximum of 3
               elements (with MINMAX) costs 2 comparisons and at most 1 swap.
               Sorting of 3 elements (i.e. selection of both minimum and
               maximum) with MINMAX costs at most 3 comparisons and at most
               2 swaps -- the same as dedicated sorting.
               Therefore, sorting is only advantageous over selection for at
               least 4 elements (depending on the distribution of the
               desired ranks).
            */
            if (SELECTION_BREAKPOINT_OFFSET<=nmemb) { /* possibly select */
                if ((SELECTION_BREAKPOINT_TABLE_MAX_NMEMB)>=nmemb) {
                    /* table breakpoints determine sorting vs. selection */
                    if (selection_breakpoint
                        [nmemb-SELECTION_BREAKPOINT_OFFSET]
                        [selection_distribution_remap[raw]]
                    >=nk)
                        sort=0U;
                } else { /* large sub-arrays */
                    /* 1/8, 15/16 sorting vs. selection rules of thumb */
                    if (7U==raw) { /* distributed ranks */
                        if (nk<=(nmemb>>3)) sort=0U; /* <= nmemb/8 ranks */
                    } else { /* middle, extended, separated, ends */
                        if (nk<=nmemb-(nmemb>>4)) sort=0U; /* <=15/16 nmemb */
                    }
                }
            } /* sort/select equivalent @2 elements (extents irrelevant) */
        } else {
            /* no sort if == region extents are required */
            sort=0U;
            /* Sorting is not possible because the pivot for sorting is not
               necessarily the desired order statistic rank element.
            */
        }
    } /* else sorting */
    if (NULL!=psort) *psort=sort;
    /* Select sampling table based on whether sorting or selecting, and if the
       latter, on the distribution of the desired order statistic ranks.
       If sort!=1U (i.e. it has been reset to 0U), distribution has been set.
    */
    A((0U!=sort)||((SAMPLING_TABLES)>distribution));
    if (0U==sort) psts=sampling_tables[distribution];
    *pindex=sample_index(psts, *pindex, nmemb);
    return psts;
}
