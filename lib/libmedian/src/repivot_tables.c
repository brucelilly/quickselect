/*INDENT OFF*/

/* Description: C source code for quickselect tables */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    repivot_tables.c copyright 2017-2019 Bruce Lilly.   \ repivot_tables.c $
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
/* $Id: ~|^` @(#)   This is repivot_tables.c version 1.12 dated 2019-04-22T21:37:40Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.repivot_tables.c */

/********************** Long description and rationale: ***********************
 Partitioning can result in an unfavorable split of the initial sub-array;
 when this happens, an emergency "break-glass" pivot selection function is used
 to ensure a better split, avoiding quadratic behavior even when handling a
 worst-case input array, such as can be generated by an adversary function
 (M. D. McIlroy "A Killer Adversary for Quicksort").
 The tables defined by this file determine how lopsided a prtition may be before
 the large region is repartitioned.
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
#define ID_STRING_PREFIX "$Id: repivot_tables.c ~|^` @(#)"
#define SOURCE_MODULE "repivot_tables.c"
#define MODULE_VERSION "1.12"
#define MODULE_DATE "2019-04-22T21:37:40Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2019"

/* local header files needed */
#include "quickselect_config.h" /* SORTING_TABLE_ENTRIES */
#include "tables.h"             /* struct repivot_table_struct */

/* system header files */
#include <stddef.h>             /* size_t */

/* data structures */
/*
   Repivoting with median-of medians always results in a ratio of the large
      region size to the remaining elements less than 3:1 (worst case is at size
      8 (2 sets of 3, with 2 elements left over; largest possible large region
      size is 6 for a ratio of 3:1), but sub-arrays of size 8 are never
      repivoted; the practical worst case is then at size 14 (4 sets of 3, with
      2 elements left over; worst case large region size 10 for a ratio of
      5:2)).  Median of samples with > 2/3 elements as samples provides at
      least as good a rank guarantee as median-of-medians at lower cost.
   With 729 or more samples, the ratio of the large region size to the remaining
      elements is rarely as high as 3:1 with random or nearly-random input, so
      factor2 and factor1 for those repivot table entries can safely be set to
      3UL without any noticeable effect on the performance for random and/or
      frequently encountered structured input sequences.
   With adverse input, break-glass processing can be less costly than continued
      lopsided divide-and-conquer at or above a certain ratio of large region
      size to other (small region plus elements comparing equal to the pivot)
      elements, where that ratio varies depending on the cost for median
      selection.  That ratio sets an upper bound on the repivoting factor
      values; always repivot if doing so is less costly that repeated divide-
      and-conquer, at least for repeated occurrences (factor2).
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

/* for remedian of samples pivot selection (limited rank guarantee) */
struct repivot_table_struct ros_sorting_repivot_table[] = {
    {   1UL,      10U,  4U },
    {   9UL,      10U,  9U },
    {  27UL,      12U,  8U },
    {  81UL,      12U,  5U },
    { (SIZE_MAX),  3U,  2U } /* sentinel */
};
struct repivot_table_struct ros_selection_repivot_table[] = {
    {   1UL,       5U,  4U },
    {   3UL,       7U,  3U },
    {   9UL,       6U,  3U },
    {  27UL,       3U,  3U },
    { (SIZE_MAX),  3U,  2U } /* sentinel */
};

/* for median of samples pivot selection */
struct repivot_table_struct mos_sorting_repivot_table[] = {
    {   1UL,      14U,  9U },
    {   5UL,      12U,  9U },
    {  15UL,      12U, 10U },
    {  17UL,      12U,  9U },
    {  21UL,      12U,  8U },
    {  27UL,      12U,  5U },
    {  31UL,      12U,  4U },
    { 101UL,      10U,  3U },
    { (SIZE_MAX),  2U,  2U } /* sentinel */
};
struct repivot_table_struct mos_selection_repivot_table[] = {
    {   1UL,      10U, 10U }, /* not actually used */
    {   5UL,       5U,  3U },
    { (SIZE_MAX),  2U,  2U } /* sentinel */
};
