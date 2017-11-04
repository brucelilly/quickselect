/*INDENT OFF*/

/* Description: C source code for quicksort_loop */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    quicksort_loop_src.h copyright 2017 Bruce Lilly.   \ quicksort_loop_src.h $
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
/* $Id: ~|^` @(#)   This is quicksort_loop_src.h version 1.4 dated 2017-09-23T04:59:51Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quicksort_loop_src.h */

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
#define ID_STRING_PREFIX "$Id: quickselect.c ~|^` @(#)"
#define SOURCE_MODULE "quicksort_loop_src.h"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-09-23T04:59:51Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"

/* Although the implementation is different, several concepts are adapted from:
   qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

/* local header files needed */
#include "quickselect_config.h"
#include "quickselect.h"        /* quickselect QSORT_FUNCTION_NAME */
#include "tables.h"     /* sampling_table_struct SAMPLING_TABLE_SIZE */
#if ! QUICKSELECT_BUILD_FOR_SPEED
#include "initialize_src.h"
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* (maybe errno_t [N1570 K3.2]) */
#include <limits.h>             /* *_MAX */
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t (maybe RSIZE_MAX [N1570 K3.4]) */
# if __STDC_VERSION__ >= 201001L
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
#  include <stdio.h>            /* (maybe errno_t rsize_t [N1570 K3.5]) */
#  include <stdlib.h>           /* (maybe errno_t rsize_t constraint_handler_t
                                   [N1570 K3.6]) */
# endif /* N1570 */
#endif /* C99 or later */

/* Simplified version of quickselect_loop for sorting. */

#if  __STDC_WANT_LIB_EXT1__
/* Preliminary support for 9899:201x draft N1570 qsort_s w/ "context".
   "context" is similar to glibc qsort_r; N1570 also has provision for
   runtime-constraint violation detection and handling. "Preliminary"
   because of the apparent lack of a convincing use-case for "context" and
   due to the poorly defined semantics and interface for constraint
   violation handling.

   Consequences of "context" are wide-ranging: any operation that requires
   comparisons (pivot selection, partitioning, dedicated sorting and/or
   selection, and support functions for any of those) has to be rewritten.
   The only operations unaffected are sampling, basic swaps, blockmoves using
   swaps, rank comparisons, and partition size evaluation; sampling and
   repivoting tables may also be reused without change.
   Executable code size is roughly doubled, not including external
   constraint-handler function code.
   Use case of "context" is questionable; one could simply use an appropriate
   comparison function.  Published supposed examples (e.g.
   https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/qsort-s
   tend to be rather silly (in the specific example above:
      1. functions are defined for specific locales, but not specifically
         implementing locale-specific collation -- one could at least as easily
         define locale-specific string comparison functions using the standard
         strcoll [+] function and use these with qsort to perform the
         locale-specific sorts with no need for "context", and avoiding the
         other implementation issues detailed below.
      2. Aside from inconsistent "context" argument order...
      3. ... the implementation comparison function copies (!) (leading portions
         of) supplied strings to fixed size (!!)[*] local buffers, then makes a
         second set of passes (!!!) over the copies to perform a conversion
         (which is discarded (!!!!) when the comparison function returns), which
         is then subjected to a third (!!!!!) pass to perform the comparison.
         Note that qsort implementations typically call the comparison function
         N log N times for an array of N elements; the above implementation
         makes 2 N log N copies and 6 N log N passes over data representing
         the strings to be sorted.  In particular, at each stage of partitioning
         qsort implementations compare a pivot element to many other elements;
         in the above implementation the string corresponding to that pivot
         element is copied, the copy modified and examined, only to be discarded
         before repeating those steps anew with the same pivot element's string
         for the next comparison.
      4. Although the copies (see #3 above) ignore the actual length of the
         original strings, strlen is subsequently called for each string (copy),
         resulting in a fourth (!!!!!!) pass over the data. Had strlen been
         called for the initial strings, the copies could have been made
         appropriate sizes (assuming that one still naively wished to make, then
         discard such copies).
      * a possibly-valid use of "context" *might* be to provide the maximum
        string length, avoiding magic numbers and failure to fully compare
        strings which have long identical leading substrings... *if* one
        insisted on the naive implementation involving copies (which are
        subsequently discarded) and multiple passes over the data.  It would
        be much more efficient to make one-time initial transformations (e.g.
        w/ strxfrm [+]) of the original string data supplemented by an index
        indicating the original order (or a pointer to the original string
        data), sorting the transformed and supplemented data using standard
        strcmp on the transformed strings.  That would involve N combined
        transformations and copies (instead of N log N copies and N log N
        transformations) plus N log N standard strcmp string comparisons (rather
        than 6 N log N passes over data), followed by access of the orginal
        string data via the indices or pointers for output.
      + Note that strcoll and strxfrm have been standardized since C89 (a.k.a.
        C90).
   ). In summary, there is no convincing use-case for "context" to justify
   doubling the size of library code.
*/
#endif /* __STDC_WANT_LIB_EXT1__ */

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* dedicated_sort declaration */
#include "dedicated_sort_decl.h"
;
/* klimits declaration */
#include "klimits_decl.h"
;
/* partition declaration */
#include "partition_decl.h"
;
/* quickselect_loop declaration */
#include "quickselect_loop_decl.h"
;
/* sample_index declaration */
#include "sample_index_decl.h"
;
/* select_pivot declaration */
#include "select_pivot_decl.h"
;
/* should_repivot declaration */
#include "should_repivot_decl.h"
;

/* declaration */
#include "quicksort_loop_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* clutter removal */
#undef PREFIX
#undef SUFFIX1
#undef SUFFIX2
#undef SUFFIX3
#if __STDC_WANT_LIB_EXT1__
# define PREFIX A(0==ret);ret=
# define SUFFIX1 A(NULL!=pivot);if(NULL==pivot)return ret=errno;
# define SUFFIX2 A(0==ret);if(0!=ret)return ret;
# define SUFFIX3 A(0==ret);return ret;
#else
# define PREFIX /**/
# define SUFFIX1 /**/
# define SUFFIX2 /**/
# define SUFFIX3 /**/
#endif /* __STDC_WANT_LIB_EXT1__ */

/* definition */
#include "quicksort_loop_decl.h"
{
#if __STDC_WANT_LIB_EXT1__
    QSORT_RETURN_TYPE ret=0;
#endif /* __STDC_WANT_LIB_EXT1__ */
    size_t nmemb=beyond-first;
    auto int c=0; /* repivot factor2 count */
    unsigned int table_index=2U; /* favors small nmemb */

#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file(__FILE__);
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
    while (cutoff<nmemb) {
        size_t p, q, r, s, t;
        char *pc, *pd, *pe, *pf, *pivot;

        /* Choose a pivot element from samples of the array. */
        A(first<beyond);
        table_index=sample_index(sorting_sampling_table,table_index,nmemb);
        A((SAMPLING_TABLE_SIZE)>table_index);
        A(0U<table_index);
        A(nmemb>sorting_sampling_table[table_index-1U].max_nmemb);
        pivot=SELECT_PIVOT_FUNCTION_NAME(base,first,beyond,size,COMPAR_ARGS,
            swapf,alignsize,table_index,NULL,pivot_selection_method,
            &pc,&pd,&pe,&pf); SUFFIX1
        A(base+first*size<=pivot);A(pivot<base+beyond*size);
        A(pc<=pd);A(pd<pe);A(pe<=pf);A(pd<=pivot);A(pivot<pe);

        /* Partition the array around the pivot element. */
        PARTITION_FUNCTION_NAME(base,first,beyond,pc,pd,pivot,pe,pf,size,
            COMPAR_ARGS,swapf,alignsize,&p,&q);
        /* sizes of < and > regions (elements) */
        s=p-first, t=beyond-q;
        A(q-p+s+t==nmemb);
        if (s<t) { /* > region is larger */
            /* recurse on small region, if size > 1 */
            if (1UL<s) {
                PREFIX QUICKSORT_LOOP(base,first,p,size,COMPAR_ARGS,swapf,
                    alignsize,cutoff,QUICKSELECT_PIVOT_REMEDIAN_SAMPLES,partition_method,network_map);
                SUFFIX2
            }
            /* large region [q,beyond) size t */
            first=q, r=t;
        } else { /* < region is larger, or regions are same size */
            /* recurse on small region, if size > 1 */
            if (1UL<t) {
                PREFIX QUICKSORT_LOOP(base,q,beyond,size,COMPAR_ARGS,swapf,
                    alignsize,cutoff,QUICKSELECT_PIVOT_REMEDIAN_SAMPLES,partition_method,network_map);
                SUFFIX2
            }
            /* large region [first,p) size s */
            beyond=p, r=s;
        }

        /* iterate on large region (size r), if size > cutoff */
        if (cutoff>=r) { nmemb=r; break; }

        /* should large region be repivoted? */
        if ((QUICKSELECT_PIVOT_REMEDIAN_SAMPLES)!=pivot_selection_method) c=0; /* reset factor2 count */
        pivot_selection_method=should_repivot(nmemb,r,cutoff,table_index,NULL,
            &c);
        nmemb=r;
    }
    if (2UL<=nmemb) {
        PREFIX
        DEDICATED_SORT(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize);
    }
    SUFFIX3
}
