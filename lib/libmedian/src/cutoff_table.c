/*INDENT OFF*/
#error "obsolete file"

/* Description: C source code for quickselect */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    cutoff_table.c copyright 2017-2018 Bruce Lilly.   \ cutoff_table.c $
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
/* $Id: ~|^` @(#)   This is cutoff_table.c version 1.5 dated 2018-02-03T17:33:01Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.cutoff_table.c */

/********************** Long description and rationale: ***********************
 Dedicated sorting is used for small sub-arrays to avoid quicksort overhead. The
 largest sub-array size which uses dedicated sorting is given by a table (one of
 several based on options such as partial order stability).
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
#define ID_STRING_PREFIX "$Id: cutoff_table.c ~|^` @(#)"
#define SOURCE_MODULE "cutoff_table.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2018-02-03T17:33:01Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"

/* local header files needed */
#include "quickselect_config.h" /* QUICKSELECT_STABLE */
#include "cutoff_table.h"       /* cutoff_table_struct */

/* system header files */
#include <limits.h>             /* *_MAX */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# define SIZE_MAX ULONG_MAX
#endif /* SIZE_MAX */
#include <stddef.h>             /* size_t */

/* default table (non-stable sort/selection, cutoffs for best run-time) */
struct cutoff_table_struct cutoff_table[] =
{ /* ratio cutoff */
#if 0
 {    1UL,    17UL },
 {    2UL,    16UL },
 {    3UL,    14UL },
 {    4UL,    12UL },
 {    6UL,    11UL },
 {    7UL,    10UL },
 {    8UL,     9UL },
 {   12UL,     8UL },
 {   14UL,     6UL },
 {   16UL,     5UL },
 {   22UL,     4UL },
 { (SIZE_MAX), 4UL }
#else
 { (SIZE_MAX), 9UL }
#endif
};

/* table for optimized comparisons (in-place merge sort) */
struct cutoff_table_struct cutoff_table_c[] =
{ /* ratio cutoff */
#if 0
 {    1UL,    24UL },
 {    2UL,    13UL },
 {    4UL,    12UL },
 {    6UL,    11UL },
 {    8UL,    10UL },
 {   16UL,     9UL },
 {   22UL,     8UL },
 {   24UL,     7UL },
 {   47UL,     6UL },
 {   62UL,     5UL },
 { 1000UL,     4UL },
 { (SIZE_MAX), 4UL }
#else
 { (SIZE_MAX), 9UL }
#endif
};

#if QUICKSELECT_STABLE
/* table for stable sort */
struct cutoff_table_struct cutoff_table_s[] =
{ /* ratio cutoff */
#if 0
 {    1UL,   433UL },
 {    2UL,    87UL },
 {    3UL,    58UL },
 {    4UL,    41UL },
 {    5UL,    34UL },
 {    6UL,    32UL },
 {    7UL,    25UL },
 {   11UL,    22UL },
 {   13UL,    20UL },
 {   14UL,    18UL },
 {   15UL,    17UL },
 {   20UL,    12UL },
 {   28UL,     9UL },
 {   38UL,     7UL },
 {   41UL,     5UL },
 {   54UL,     4UL },
 { (SIZE_MAX), 4UL }
#else
 { (SIZE_MAX), 9UL }
#endif
};

/* table for stable sort with optimized comparisons (in-place merge sort) */
struct cutoff_table_struct cutoff_table_sc[] =
{ /* ratio cutoff */
#if 0
 {       1UL,  2904UL },
 {       2UL,   823UL },
 {       3UL,   660UL },
 {       4UL,   444UL },
 {       5UL,   394UL },
 {       6UL,   280UL },
 {       8UL,   253UL },
 {       9UL,   230UL },
 {      11UL,   210UL },
 {      12UL,   180UL },
 {      14UL,   165UL },
 {      15UL,   143UL },
 {      18UL,   120UL },
 {      22UL,   100UL },
 {      28UL,    90UL },
 {      32UL,    80UL },
 {      41UL,    73UL },
 {      47UL,    63UL },
 {      62UL,    54UL },
 {     106UL,    38UL },
 {     167UL,    35UL },
 {    1550UL,    32UL },
 { 1600000UL,    28UL },
 /* This should decrease, but very slowly... */
 { (SIZE_MAX), 4UL }
#else
 { (SIZE_MAX), 9UL }
#endif
};
#endif /* QUICKSELECT_STABLE */
