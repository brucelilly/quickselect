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
/* $Id: ~|^` @(#)   This is sampling_tables.c version 1.5 dated 2018-05-18T01:35:57Z. \ $ */
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
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2018-05-18T01:35:57Z"
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
   {                     8UL,           1UL }, /* sorting */
   {                    60UL,           3UL }, /* sorting */
   {                   461UL,           9UL }, /* sorting */
   {                  3099UL,          27UL }, /* sorting */
   {                 22293UL,          81UL }, /* sorting */
#if (SIZE_MAX) > 65535 /* (16 bits limit) */
   {                138090UL,         243UL }, /* sorting */
   {                804783UL,         729UL }, /* sorting */
   {               6028983UL,        2187UL }, /* sorting */
   {              40424362UL,        6561UL }, /* sorting */
   {             271045547UL,       19683UL }, /* sorting */
   {            1817361729UL,       59049UL }, /* sorting */
# if (SIZE_MAX) > 4294967295 /* (32 bits limit) */
   {           12185419368UL,      177147UL }, /* sorting */
   {           81703297029UL,      531441UL }, /* sorting */
   {          547821009993UL,     1594323UL }, /* sorting */
   {         3673142576893UL,     4782969UL }, /* sorting */
   {        24628439114385UL,    14348907UL }, /* sorting */
   {       165133805866052UL,    43046721UL }, /* sorting */
   {      1107222983688002UL,   129140163UL }, /* sorting */
   {      7423935572594868UL,   387420489UL }, /* sorting */
   {     49777524670288100UL,  1162261467UL }, /* sorting */
# else /* 32 bits */
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
#else /* SIZE_MAX == 65535 (16 bits limit) */
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
   {                    12UL,           1UL }, /* ends */
   {                   117UL,           3UL }, /* ends */
   {                  6884UL,           9UL }, /* ends */
#if (SIZE_MAX) > 65535 /* (16 bits limit) */
   {                400198UL,          27UL }, /* ends */
   {              18849664UL,          81UL }, /* ends */
# if (SIZE_MAX) > 4294967295 /* (32 bits limit) */
   {            1102422680UL,         243UL }, /* ends */
   {           60384694013UL,         729UL }, /* ends */
   {         3307543773989UL,        2187UL }, /* ends */
   {       181169185265224UL,        6561UL }, /* ends */
   {      9923458594195450UL,       19683UL }, /* ends */
   {    543552869250629000UL,       59049UL }, /* ends */
# else /* 32 bits */
   {   (SIZE_MAX),                    243UL },
   {   (SIZE_MAX),                    729UL },
   {   (SIZE_MAX),                   2187UL },
   {   (SIZE_MAX),                   6561UL },
   {   (SIZE_MAX),                  19683UL },
   {   (SIZE_MAX),                  59049UL },
# endif
   {   (SIZE_MAX),                 177147UL },
   {   (SIZE_MAX),                 531441UL },
   {   (SIZE_MAX),                1594323UL },
   {   (SIZE_MAX),                4782969UL },
   {   (SIZE_MAX),               14348907UL },
   {   (SIZE_MAX),               43046721UL },
   {   (SIZE_MAX),              129140163UL },
   {   (SIZE_MAX),              387420489UL },
   {   (SIZE_MAX),             1162261467UL },
   {   (SIZE_MAX),             3486784401UL }, /* 32 and 64 bits */
#else /* SIZE_MAX == 65535 (16 bits limit) */
   {   (SIZE_MAX),                     27UL },
   {   (SIZE_MAX),                     81UL },
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
struct sampling_table_struct middle_sampling_table[] = { /* 010 m -> MIDDLE 2U */
   {                     8UL,           1UL }, /* middle */
   {                    24UL,           3UL }, /* middle */
   {                   430UL,           9UL }, /* middle */
   {                  3418UL,          27UL }, /* middle */
   {                 22486UL,          81UL }, /* middle */
#if (SIZE_MAX) > 65535 /* (16 bits limit) */
   {                157775UL,         243UL }, /* middle */
   {                884868UL,         729UL }, /* middle */
   {               9445653UL,        2187UL }, /* middle */
   {              74269197UL,        6561UL }, /* middle */
   {             583963175UL,       19683UL }, /* middle */
# if (SIZE_MAX) > 4294967295 /* (32 bits limit) */
   {            4591580387UL,       59049UL }, /* middle */
   {           36102636877UL,      177147UL }, /* middle */
   {          283867487789UL,      531441UL }, /* middle */
   {         2231990724059UL,     1594323UL }, /* middle */
   {        17549676544793UL,     4782969UL }, /* middle */
   {       137989438534381UL,    14348907UL }, /* middle */
   {      1084982113398765UL,    43046721UL }, /* middle */
   {      8530987580632442UL,   129140163UL }, /* middle */
   {     67077372246188300UL,   387420489UL }, /* middle */
   {    527415357826619000UL,  1162261467UL }, /* middle */
# else
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
   {   (SIZE_MAX),             3486784401UL }, /* 32 and 64 bits */
#else /* SIZE_MAX == 65535 (16 bits limit) */
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
   distribution values, i.e. 8.  Varying the sampling table during selection
   results in a decrease in the number of comparisons and swaps vs. what would
   be used if the sorting sampling table were always used.
*/
unsigned char sampling_distribution_remap[8] = {
    ENDS,      /* 00 000 -> ENDS */
    ENDS,      /* 01 001 -> ENDS */
    MIDDLE,    /* 02 010 -> MIDDLE */
    MIDDLE,    /* 03 011 -> MIDDLE */
    ENDS,      /* 04 100 -> ENDS */
    SORTING,   /* 05 101 -> SORTING */ /* for balanced recursion */
    MIDDLE,    /* 06 110 -> MIDDLE */
    MIDDLE     /* 07 111 -> MIDDLE */
};
/* sampling table pointers indexed by sampling_distribution_enum value */
struct sampling_table_struct *sampling_tables[SAMPLING_TABLES] = {
    sorting_sampling_table,     /* SORTING 0U */
    ends_sampling_table,        /* ENDS 1U */
    middle_sampling_table,      /* MIDDLE 2U */
};
