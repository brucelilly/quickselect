/*INDENT OFF*/

/* Description: C source code for selection-related development */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    globals.c copyright 2016-2017 Bruce Lilly.   \ globals.c $
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
/* $Id: ~|^` @(#)   This is globals.c version 1.3 dated 2017-11-03T19:45:36Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.globals.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: globals.c ~|^` @(#)"
#define SOURCE_MODULE "globals.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2017-11-03T19:45:36Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

int use_shell=0;
int use_networks=0;
int method_partition=QUICKSELECT_PARTITION_FAST;
int method_pivot=QUICKSELECT_PIVOT_REMEDIAN_SAMPLES;

unsigned int network_mask=0x07F8U;
unsigned int debug=0U;

size_t quickselect_small_array_cutoff = 0UL;

long *input_data=NULL;
/* for instrumented comparisons, swaps, rotations, etc.: */
size_t nlt, neq, ngt, nsw;
size_t nrotations[MAXROTATION];
size_t nmerges, npartitions, nrecursions, nrepivot;

/* insertion sort parameters */
unsigned int introsort_final_insertion_sort = 1U;
size_t introsort_small_array_cutoff = 0UL; /* sentinel value */
size_t introsort_recursion_factor = 2UL;

unsigned int instrumented = 0U;
int instrumented_bmqsort = 1;

/* for selection */
size_t selection_nk = 1UL;
int strict_selection=0;
/* for repivoting */
size_t repivot_factor = 999UL;
int reset_selection_factor2_count=1;
/* random shuffle @nmemb=80, repivot_factor=39 (lopsided_partition_limit=1, no_aux_repivot=1), ~0.014% repivots */
size_t repivot_cutoff = 999UL;
int lopsided_partition_limit = 9;
int no_aux_repivot = 0;
unsigned int save_partial = 0U;
size_t sort_threshold = 10UL;

size_t dp_cutoff = 15UL; /* 15 seems to be optimal */

size_t histogram[8][37];
size_t histoswap[8];
unsigned int do_histogram=0U;

size_t y_cutoff = 27UL; /* 11 seems to be optimal */

/* in-place mergesort */
#if COMPARE1
size_t merge_cutoff = 7UL; /* "optimization" for already-sorted input (average performance decline) */
#else
size_t merge_cutoff = 1UL; /* 1UL for pure mergesort (for small_arrays table in paper), 3UL to use optimized decision tree */
#endif

int *global_iarray = NULL;
long *global_larray = NULL;
long *global_refarray = NULL;
struct data_struct **global_parray = NULL;
double *global_darray = NULL;
struct data_struct *global_data_array = NULL;
float *global_uarray = NULL;
float *global_sarray = NULL;
float *global_warray = NULL;

struct stats_table_struct stats_table[(SAMPLING_TABLE_SIZE)]; /* big enough to match biggest sampling_table_size */
struct breakpoint_table_struct breakpoint_table[(SAMPLING_TABLE_SIZE)]; /* big enough to match biggest sampling_table_size */

# if ASSERT_CODE + DEBUG_CODE
FILE *logfile = NULL;
#endif /* ASSERT_CODE */

/* for more adverse adversary */
size_t pivot_minrank = 0UL;
size_t nfrozen = 0UL;
