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
/* $Id: ~|^` @(#)   This is sampling_tables.c version 1.6 dated 2018-08-13T20:58:02Z. \ $ */
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
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2018-08-13T20:58:02Z"
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
   Sorting (as distinct from order statistic selection, and indicated by 000)
   uses a separate sampling table.  When selecting a large number of
   order statistics, it may be more efficient to simply sort the sub-array,
   where "large" depends on the distribution of order statistics as well as
   their number and the size of the sub-array.  Order statistics ranks which are
   widely distributed result in an advantage to sorting at a lower proportion of
   desired ranks to the sub-array size than for other distributions.
*/
/* tables for remedian pivot selection */
/* Remedian of samples always estimates the sample median. */
struct sampling_table_struct sorting_sampling_table[] = { /* 000 */
   {                    17UL,           1UL }, /* ros sorting */
   {                    88UL,           3UL }, /* ros sorting */
   {                   483UL,           9UL }, /* ros sorting */
   {                  2883UL,          27UL }, /* ros sorting */
   {                 18923UL,          81UL }, /* ros sorting */
#if (SIZE_MAX) > 65535 /* (16 bits limit) */
   {                162215UL,         243UL }, /* ros sorting */
   {               1459939UL,         729UL }, /* ros sorting */
   {              13139450UL,        2187UL }, /* ros sorting */
   {             118255054UL,        6561UL }, /* ros sorting */
   {            1064295489UL,       19683UL }, /* ros sorting */
# if (SIZE_MAX) > 4294967295 /* (32 bits limit) */
   {            9578659400UL,       59049UL }, /* ros sorting */
   {           86207934600UL,      177147UL }, /* ros sorting */
   {          775871411402UL,      531441UL }, /* ros sorting */
   {         6982842702621UL,     1594323UL }, /* ros sorting */
   {        62845584323585UL,     4782969UL }, /* ros sorting */
   {       565610258912263UL,    14348907UL }, /* ros sorting */
   {      5090492330210370UL,    43046721UL }, /* ros sorting */
   {     45814430971893300UL,   129140163UL }, /* ros sorting */
   {    412329878747040000UL,   387420489UL }, /* ros sorting */
   {   3710968908723360000UL,  1162261467UL }, /* ros sorting */
/*    18446744073709551616 = 2^64 */
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

/* When one or more desired order statistic ranks are near the median rank,
   using additional samples (compared to sorting) can reduce the overall
   number of comparisons.  If the desired rank if matched, the problem size is
   directly reduced.  If the pivot is adjacent to the desired rank, the region
   containing the desired rank can be processed via one of the special-case
   selection functions.  If the desired rank is near the pivot, the region not
   containing the rank near the pivot (if it has any desired ranks) can have a
   pivot chosen so as to greatly reduce the problem size (i.e. by more than a
   factor of 2).
*/
struct sampling_table_struct middle_sampling_table[] = { /* 010 m */
   {                    10UL,           1UL }, /* ros middle */
   {                    26UL,           3UL }, /* ros middle */
   {                   110UL,           9UL }, /* ros middle */
   {                   498UL,          27UL }, /* ros middle */
   {                  1958UL,          81UL }, /* ros middle */
   {                 11000UL,         243UL }, /* ros middle */
#if (SIZE_MAX) > 65535 /* (16 bits limit) */
   {                 72730UL,         729UL }, /* ros middle */
   {                747154UL,        2187UL }, /* ros middle */
   {               3985811UL,        6561UL }, /* ros middle */
   {              20710866UL,       19683UL }, /* ros middle */
   {             107616806UL,       59049UL }, /* ros middle */
   {             559193312UL,      177147UL }, /* ros middle */
   {            2905653671UL,      531441UL }, /* ros middle */
# if (SIZE_MAX) > 4294967295 /* (32 bits limit) */
   {           15098219347UL,     1594323UL }, /* ros middle */
   {           78452649026UL,     4782969UL }, /* ros middle */
   {          407651922289UL,    14348907UL }, /* ros middle */
   {         2118221523611UL,    43046721UL }, /* ros middle */
   {        11006601901726UL,   129140163UL }, /* ros middle */
   {        57191981137406UL,   387420489UL }, /* ros middle */
   {       297178251346509UL,  1162261467UL }, /* ros middle */
/*    18446744073709551616 = 2^64 */
# else
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

/* Remedian of samples ends sampling table is used when there are no desired
   order statistic ranks near the median, but there are ranks near both array
   ends.  It is desirable to split the array near the median to minimize
   recursion depth, but a precisely even split is unimportant because the next
   iteration will eliminate the parts of the array which contain no desired
   ranks (currently near the middle of the sub-array being processed, at one
   end of each of the two regions resulting from the partition using the pivot
   element selected during this iteration).  Therefore fewer samples can be used
   than when sorting or selecting for a rank near the median.
*/
struct sampling_table_struct ends_sampling_table[] = { /* 101 s q2 */
   {                    12UL,           1UL }, /* ros ends */
   {                    63UL,           3UL }, /* ros ends */
   {                   563UL,           9UL }, /* ros ends */
   {                  5100UL,          27UL }, /* ros ends */
   {                 45900UL,          81UL }, /* ros ends */
#if (SIZE_MAX) > 65535 /* (16 bits limit) */
   {                413100UL,         243UL }, /* ros ends */
   {               3717900UL,         729UL }, /* ros ends */
   {              33461100UL,        2187UL }, /* ros ends */
   {             301149900UL,        2187UL }, /* ros ends */
   {            2710349100UL,        6561UL }, /* ros ends */
# if (SIZE_MAX) > 4294967295 /* (32 bits limit) */
   {           24393141900UL,       19683UL },
   {          219538277100UL,       59049UL },
   {         1975844493900UL,      177147UL },
   {        17782600445100UL,      531441UL },
   {       160043404006000UL,     1594323UL },
   {      1440390636050000UL,     4782969UL },
   {     12963515724500000UL,    14348907UL },
   {    116671641520000000UL,    43046721UL },
   {   1050044773680000000UL,   129140163UL },
   {   9450402963140000000UL,   387420489UL },
/*    18446744073709551616 = 2^64 */
# else /* 32 bits */
   {   (SIZE_MAX),                  19683UL },
   {   (SIZE_MAX),                  59049UL },
   {   (SIZE_MAX),                 177147UL },
   {   (SIZE_MAX),                 531441UL },
   {   (SIZE_MAX),                1594323UL },
   {   (SIZE_MAX),                4782969UL },
   {   (SIZE_MAX),               14348907UL },
   {   (SIZE_MAX),               43046721UL },
   {   (SIZE_MAX),              129140163UL },
   {   (SIZE_MAX),              387420489UL },
# endif
   {   (SIZE_MAX),             1162261467UL },
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

/* median-of-samples (mos) uses differently arranged tables using the common
   structure.  The number of samples is an odd number.  The table is the
   same size as other sampling tables; larger arrays have sample size computed.
*/
struct sampling_table_struct mos_sorting_sampling_table[] = {
   {                    15UL,           1UL }, /* mos sorting */
   {                    15UL,           3UL }, /* mos sorting */
   {                    72UL,           5UL }, /* mos sorting */
   {                   109UL,           7UL }, /* mos sorting */
   {                   187UL,           9UL }, /* mos sorting */
   {                   296UL,          11UL }, /* mos sorting */
   {                   438UL,          13UL }, /* mos sorting */
   {                   587UL,          15UL }, /* mos sorting */
   {                   767UL,          17UL }, /* mos sorting */
   {                   973UL,          19UL }, /* mos sorting */
   {                  1179UL,          21UL }, /* mos sorting */
   {                  1443UL,          23UL }, /* mos sorting */
   {                  1718UL,          25UL }, /* mos sorting */
   {                  2027UL,          27UL }, /* mos sorting */
   {                  2352UL,          29UL }, /* mos sorting */
   {                  2700UL,          31UL }, /* mos sorting */
   {                  3072UL,          33UL }, /* mos sorting */
   {                  3468UL,          35UL }, /* mos sorting */
   {                  3888UL,          37UL }, /* mos sorting */
   {                  4332UL,          39UL }, /* mos sorting */
   {                  4800UL,          41UL }, /* mos sorting */
};

struct sampling_table_struct mos_middle_sampling_table[] = {
   {                    10UL,           1UL }, /* mos middle */
   {                    24UL,           3UL }, /* mos middle */
   {                    48UL,           5UL }, /* mos middle */
   {                    74UL,           7UL }, /* mos middle */
   {                   100UL,           9UL }, /* mos middle */
   {                   115UL,          11UL }, /* mos middle */
   {                   212UL,          13UL }, /* mos middle */
   {                   279UL,          15UL }, /* mos middle */
   {                   338UL,          17UL }, /* mos middle */
   {                   422UL,          19UL }, /* mos middle */
   {                   499UL,          21UL }, /* mos middle */
   {                   562UL,          23UL }, /* mos middle */
   {                   627UL,          25UL }, /* mos middle */
   {                   715UL,          27UL }, /* mos middle */
   {                   801UL,          29UL }, /* mos middle */
   {                   892UL,          31UL }, /* mos middle */
   {                   988UL,          33UL }, /* mos middle */
   {                  1090UL,          35UL }, /* mos middle */
   {                  1187UL,          37UL }, /* mos middle */
   {                  1265UL,          39UL }, /* mos middle */
   {                  1399UL,          41UL }, /* mos middle */
};

/* When processing a sub-array with desired ranks which are near one end of the
   sub-array (or where there are no ranks near one end), the pivot may be
   selected from the samples at a rank other than the median of the samples so
   as to split the array near one of the desired ranks or to eliminate a large
   part of the sub-array which contains no desired ranks.  The pivot selection
   method is still called "median of samples" even if a sample other than the
   median is selected.
*/
struct sampling_table_struct mos_ends_sampling_table[] = {
   {                     8UL,           1UL }, /* mos ends */
   {                     9UL,           3UL }, /* mos ends */
   {                    23UL,           5UL }, /* mos ends */
   {                    44UL,           7UL }, /* mos ends */
   {                    92UL,           9UL }, /* mos ends */
   {                   107UL,          11UL }, /* mos ends */
   {                   126UL,          13UL }, /* mos ends */
   {                   227UL,          15UL }, /* mos ends */
   {                   252UL,          17UL }, /* mos ends */
   {                   363UL,          19UL }, /* mos ends */
   {                   406UL,          21UL }, /* mos ends */
   {                   439UL,          23UL }, /* mos ends */
   {                   503UL,          25UL }, /* mos ends */
   {                   538UL,          27UL }, /* mos ends */
   {                   669UL,          29UL }, /* mos ends */
   {                   835UL,          31UL }, /* mos ends */
   {                   892UL,          33UL }, /* mos ends */
   {                   932UL,          35UL }, /* mos ends */
   {                   990UL,          37UL }, /* mos ends */
   {                  1597UL,          39UL }, /* mos ends */
   {                  1667UL,          41UL }, /* mos ends */
};
