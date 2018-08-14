/*INDENT OFF*/
#ifndef	TABLES_H_INCLUDED

/* Description: type declarations for quickselect tables */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    tables.h 1.8 copyright 2017-2018 Bruce Lilly.   \ tables.h $
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
/* $Id: ~|^` @(#)   This is tables.h version 1.8 dated 2018-06-12T18:36:53Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.tables.h */

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

/* version-controlled header file version information */
#define TABLES_H_VERSION "tables.h 1.8 2018-06-12T18:36:53Z"

/* system header files */
#include <stddef.h>             /* size_t */

/* Nothing to configure below this line. */

/* data structures */

/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
   McGeoch & Tygar, in "Optimal Sampling Strategies for Quicksort" also
   suggest use of a table, and determine the optimal sample size as proportional
   to the square root of the array size.
*/
struct sampling_table_struct {
    size_t max_nmemb;    /* smallest sub-array for this number of samples */
    size_t samples;      /* the number of samples used for pivot selection */
};
/* N.B.: The remedian-based sampling tables' samples member for
          sampling_table[x] is the x'th power of 3. Several parts of the code
          depend on this. Don't add entries for sample sizes which are not
          powers of 3!
*/
extern struct sampling_table_struct sorting_sampling_table[];
extern struct sampling_table_struct ends_sampling_table[];
extern struct sampling_table_struct middle_sampling_table[];

extern struct sampling_table_struct mos_sorting_sampling_table[];
extern struct sampling_table_struct mos_middle_sampling_table[];
extern struct sampling_table_struct mos_ends_sampling_table[];

/* For simplicity, all sampling tables are the same size */
#define SAMPLING_TABLE_SIZE  21 /* 3^0 - 3^20 */
/* 3^20 < 2^32, so samples values are safe for 32-bit machines */

/* Sampling for selection depends on the distribution of order statistic ranks.
   Specifically on whether or not there are desired order statistic ranks near
   the middle of the sub-array.  If there are desired ranks near the middle,
   more samples for pivot selection leads to an improved rank (i.e. closer to
   the median) for the pivot, and a better chance of selecting one of the
   desired ranks as the pivot (which eliminates it from further processing).
   Otherwise, fewer samples may be used to reduce the cost of pivot selection,
   as partitioning need not be so precise.  The distribution of desired order
   statistic ranks starts with a determination of a "raw" distribution, based on
   the presence or absence of desired ranks in three bands of the sub-array,
   leading to 8 possible values: 000, 001, 010, 011, 100, 101, 110, and 111,
   where a 1 indicates presence of desired order statistic ranks in a band.
   Sorting (as distinct from order statistic
   selection) uses a separate sampling table.  When selecting a large number of
   order statistics, it may be more efficient to simply sort the sub-array,
   where "large" depends on the distribution of order statistics as well as
   their number and the size of the sub-array.  Order statistics ranks which are
   widely distributed result in an advantage to sorting at a lower proportion of
   desired ranks to the sub-array size than for other distributions.
*/

/*
   Repivoting with median-of medians always results in a ratio of the large
      region size to the remaining elements less than 3:1 (worst case is at size
      8 (2 sets of 3, with 2 elements left over; largest possible large region
      size is 6 for a ratio of 3:1), but sub-arrays of size 8 are never
      repivoted; the practical worst case is then at size 14 (4 sets of 3, with
      2 elements left over; worst case large region size 10 for a ratio of
      5:2)).
   With 729 or more samples, the ratio of the large region size to the remaining
      elements is rarely as high as 3:1, so factor2 and factor1 for those
      repivot table entries can safely be set to 3UL without any noticeable
      effect on the performance for random and/or frequently encountered
      structured input sequences.
   The factor2 member is set by finding the worst-case adversarial performance
      for sorting and for selection, and reducing the factor2 value at the
      appropriate max_nmemb entry to eliminate that worst-case performance,
      repeating the process until some desired absolute worst-case is achieved
      or the worst-case performance point cannot be eliminated by reducing
      factor2.
   When factor2 reduction is no longer able to eliminate a worst-case
      performance point, factor1 may be reduced to eliminate the worst-case
      performance point. This may be repeated (after first trying factor2!)
      until a desired absolute worst-case performance point is reached, no
      further improvement can be obtained using factor2 and factor1, or the
      elimination of a worst-case performance point entails too large of a
      performance penalty for random input sequences.
*/
/* repivot factors are relatively small unsigned integers */
struct repivot_table_struct {
    size_t min_samples;
    unsigned char factor1; /* lopsided ratio for immediate repivoting */
    unsigned char factor2; /* lopsided ratio for repivoting on 2nd occurrence */
};

extern struct repivot_table_struct ros_sorting_repivot_table[];
extern struct repivot_table_struct ros_selection_repivot_table[];
extern struct repivot_table_struct mos_sorting_repivot_table[];
extern struct repivot_table_struct mos_selection_repivot_table[];

#define	TABLES_H_INCLUDED
#endif /* TABLES_H_INCLUDED */
