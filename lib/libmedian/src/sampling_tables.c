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
* $Id: ~|^` @(#)    sampling_tables.c copyright 2017-2018 Bruce Lilly.   \ sampling_tables.c $
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
/* $Id: ~|^` @(#)   This is sampling_tables.c version 1.4 dated 2018-04-18T00:43:43Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.sampling_tables.c */

/********************** Long description and rationale: ***********************
 Pivot selection uses Rousseeuw & Basset's remedian, described in "The Remedian:
 A Robust Averaging Method for Large Data Sets", on a sample of a size which is
 a power of 3 taken from the array elements, with sample size approximately the
 square root of the array size as described in "Optimal Sampling Strategies for
 Quicksort" by McGeoch and Tygar, and in "Optimal Sampling Strategies in
 Quicksort and Quickselect" by Martinez and Roura.
 The tables defined in this file are used to determine the number of samples to
 be used for pivot selection for a given sub-array size with a certain
 distribution of desired order statistic ranks.
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
#define ID_STRING_PREFIX "$Id: sampling_tables.c ~|^` @(#)"
#define SOURCE_MODULE "sampling_tables.c"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2018-04-18T00:43:43Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"

/* local header files needed */
#include "tables.h"             /* sampling_table_struct SAMPLING_TABLE_SIZE */

/* system header files */
#if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )
# include <stdint.h>            /* (header not standardized pre-C99) SIZE_MAX */
#endif /* C99 or later */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# include <limits.h>           /* ULONG_MAX */
# define SIZE_MAX ULONG_MAX
#endif /* SIZE_MAX */

/* data structures */
struct sampling_table_struct sorting_sampling_table[] = { /* 000 q -> SORTING 5U */
   {                     2UL,           1UL }, /* sorting */
   {                    79UL,           3UL }, /* sorting */
   {                   551UL,           9UL }, /* sorting */
   {                  3567UL,          27UL }, /* sorting */
   {                 23958UL,          81UL }, /* sorting */
#if (SIZE_MAX) > 65535
   {                179087UL,         243UL }, /* sorting */
   {               1611788UL,         729UL }, /* sorting */
   {              14506098UL,        2187UL }, /* sorting */
   {             130554892UL,        6561UL }, /* sorting */
   {            1174994040UL,       19683UL }, /* sorting */
# if (SIZE_MAX) > 4294967295 /* 64 bits */
   {           10574946371UL,       59049UL }, /* sorting */
   {           95174517350UL,      177147UL }, /* sorting */
   {          856570656158UL,      531441UL }, /* sorting */
   {         7709135905434UL,     1594323UL }, /* sorting */
   {        69382223148914UL,     4782969UL }, /* sorting */
   {       624440008340237UL,    14348907UL }, /* sorting */
   {      5619960075062138UL,    43046721UL }, /* sorting */
   {     50579640675559248UL,   129140163UL }, /* sorting */
   {    455216766080033152UL,   387420489UL }, /* sorting */
   {   4096950894720299008UL,  1162261467UL }, /* sorting */
# else /* 32 bits */
   {   (SIZE_MAX),                  59049UL },
   {   (SIZE_MAX),                 177147UL },
   {   (SIZE_MAX),                 531441UL },
   {   (SIZE_MAX),                1594323UL },
   {   (SIZE_MAX),                4782969UL },
   {   (SIZE_MAX),               14348907UL },
   {   (SIZE_MAX),               43046721UL },
   {   (SIZE_MAX),              129140163UL },
   {   (SIZE_MAX),              387420489UL },
   {   (SIZE_MAX),             1162261467UL },
# endif
   {            (SIZE_MAX),    3486784401UL }, /* 32 and 64 bits */
#else /* SIZE_MAX == 65535 (16 bits) */
   {   (SIZE_MAX),                    243UL },
   {   (SIZE_MAX),                    729UL },
   {   (SIZE_MAX),                   2187UL },
   {   (SIZE_MAX),                   6561UL },
   {   (SIZE_MAX),                  19683UL },
   {   (SIZE_MAX),                  59049UL },
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 177147 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 531441 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 1594323 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 4782969 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 14348907 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 43046721 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 129140163 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 387420489 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 1162261467 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 3486784401 */
#endif
};
struct sampling_table_struct ends_sampling_table[] = { /* 001 e -> ENDS 1U */
   {                     2UL,           1UL }, /* ends */
   {                   108UL,           3UL }, /* ends */
   {   (SIZE_MAX),                      9UL }, /* ends */
   {   (SIZE_MAX),                     27UL }, /* ends */
   {   (SIZE_MAX),                     81UL }, /* ends */
   {   (SIZE_MAX),                    243UL }, /* ends */
   {   (SIZE_MAX),                    729UL }, /* ends */
   {   (SIZE_MAX),                   2187UL }, /* ends */
   {   (SIZE_MAX),                   6561UL }, /* ends */
   {   (SIZE_MAX),                  19683UL },
   {   (SIZE_MAX),                  59049UL },
#if (SIZE_MAX) > 65535
   {   (SIZE_MAX),                 177147UL },
   {   (SIZE_MAX),                 531441UL },
   {   (SIZE_MAX),                1594323UL },
   {   (SIZE_MAX),                4782969UL },
   {   (SIZE_MAX),               14348907UL },
   {   (SIZE_MAX),               43046721UL },
   {   (SIZE_MAX),              129140163UL },
   {   (SIZE_MAX),              387420489UL },
   {   (SIZE_MAX),             1162261467UL },
   {   (SIZE_MAX),             3486784401UL },
#else /* SIZE_MAX == 65535 (16 bits) */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 177147 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 531441 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 1594323 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 4782969 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 14348907 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 43046721 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 129140163 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 387420489 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 1162261467 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 3486784401 */
#endif
};
struct sampling_table_struct middle_sampling_table[] = { /* 010 m -> MIDDLE 2U */
   {                     2UL,           1UL }, /* middle */
   {                    24UL,           3UL }, /* middle */
   {                   114UL,           9UL }, /* middle */
   {                   772UL,          27UL }, /* middle */
   {                  3867UL,          81UL }, /* middle */
   {                 19329UL,         243UL }, /* middle */
#if (SIZE_MAX) > 65535
   {                102563UL,         729UL }, /* middle */
   {                328153UL,        2187UL }, /* middle */
   {               1658941UL,        6561UL }, /* middle */
   {               8386583UL,       19683UL }, /* middle */
   {              42397374UL,       59049UL }, /* middle */
   {             214334865UL,      177147UL }, /* middle */
   {            1083544306UL,      531441UL }, /* middle */
# if (SIZE_MAX) > 4294967295
   {            5477728782UL,     1594323UL }, /* middle */
   {           27692003385UL,     4782969UL }, /* middle */
   {          139993614503UL,    14348907UL }, /* middle */
   {          707720991818UL,    43046721UL }, /* middle */
   {         3577798916285UL,   129140163UL }, /* middle */
   {        18087134949157UL,   387420489UL }, /* middle */
   {        91437349701199UL,  1162261467UL }, /* middle */
# else
   {   (SIZE_MAX),                1594323UL },
   {   (SIZE_MAX),                4782969UL },
   {   (SIZE_MAX),               14348907UL },
   {   (SIZE_MAX),               43046721UL },
   {   (SIZE_MAX),              129140163UL },
   {   (SIZE_MAX),              387420489UL },
   {   (SIZE_MAX),             1162261467UL },
# endif
   {   (SIZE_MAX),             3486784401UL },
#else /* SIZE_MAX == 65535 (16 bits) */
   {   (SIZE_MAX),                    729UL },
   {   (SIZE_MAX),                   2187UL },
   {   (SIZE_MAX),                   6561UL },
   {   (SIZE_MAX),                  19683UL },
   {   (SIZE_MAX),                  59049UL },
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 177147 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 531441 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 1594323 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 4782969 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 14348907 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 43046721 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 129140163 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 387420489 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 1162261467 */
   {   (SIZE_MAX),               (SIZE_MAX) }, /* 3486784401 */
#endif
};

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
   where a 1 indicates presence of desired order statistic ranks in a band.  The
   8 raw distribution values are then translated to one of 2 values for sampling
   for selection, based on whether or not there are desired ranks near the
   middle of the sub-array, as noted above (the middle band is tested first, and
   one other band is checked if necessary to discriminate between distributions
   which have ranks in 2 or 3 bands).  Sorting (as distinct from order statistic
   selection) uses a separate sampling table.  When selecting a large number of
   order statistics, it may be more efficient to simply sort the sub-array,
   where "large" depends on the distribution of order statistics as well as
   their number and the size of the sub-array.  Order statistics ranks which are
   widely distributed result in an advantage to sorting at a lower proportion of
   desired ranks to the sub-array size than for other distributions.  A table
   with two columns, for widely-distributed and for others, is used to indicate
   when sorting is preferred to continued selection; rows of the table
   correspond to sub-array size and the table entry values represent the largest
   number of desired order statistic ranks for which continued selection is
   appropriate.  The raw order statistic distribution is translated into a
   suitable column index value for that table.
*/
/* Lookup table; raw distribution to sampling_distribution_enum value. */
/* The number of array elements is determined by the number of possible raw
   distribution vales, i.e. 8.
*/
unsigned char sampling_distribution_remap[8] = {
    ENDS,      /* 00 000 -> ENDS */
    ENDS,      /* 01 001 -> ENDS */
    MIDDLE,    /* 02 010 -> MIDDLE */
    MIDDLE,    /* 03 011 -> MIDDLE */
    ENDS,      /* 04 100 -> ENDS */
    ENDS,      /* 05 101 -> ENDS */
    MIDDLE,    /* 06 110 -> MIDDLE */
    MIDDLE     /* 07 111 -> MIDDLE */
};
/* sampling table pointers indexed by sampling_distribution_enum value */
struct sampling_table_struct *sampling_tables[SAMPLING_TABLES] = {
    sorting_sampling_table,     /* SORTING 0U */
    ends_sampling_table,        /* ENDS 1U */
    middle_sampling_table,      /* MIDDLE 2U */
};
