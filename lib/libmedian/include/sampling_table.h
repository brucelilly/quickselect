/*INDENT OFF*/
#ifndef	SAMPLING_TABLE_H_INCLUDED

/* Description: type declaration for sampling tables */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    sampling_table.h copyright 2017 Bruce Lilly.   \ sampling_table.h $
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
/* $Id: ~|^` @(#)   This is sampling_table.h version 1.2 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.sampling_table.h */

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
 Tables (structure defined in this file) are used to obtain the optimum number
 of samples for pivot selection for a given sub-array size.
******************************************************************************/

/* version-controlled header file version information */
#define SAMPLING_TABLE_H_VERSION "sampling_table.h 1.2 2017-12-22T04:14:04Z"

/* system header files */
#include <stddef.h>             /* size_t */

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
/* N.B.: The sampling table's samples member for sampling_table[x] is the x'th
          power of 3. Several parts of the code depend on this. Don't add
          entries for sample sizes which are not powers of 3!
*/

/* For simplicity, all sampling tables are the same size */
#define SAMPLING_TABLE_SIZE  21 /* 3^0 - 3^20 */
/* 3^20 < 2^32, so samples values are safe for 32-bit machines */

#define	SAMPLING_TABLE_H_INCLUDED
#endif
