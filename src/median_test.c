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
* $Id: ~|^` @(#)    %M% copyright 2016-2017 %Q%.   \ median_test.c $
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
/* $Id: ~|^` @(#)   This is %M% version %I% dated %O%. \ $ */
/* You may send bug reports to %Y% with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file %P% */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* configuration options */
/* 64-bit machine: struct size is 40 bytes for string size 13-20
                                  48 for string size 21-28
                                  56 for string size 29-36
*/
/* 32-bit machine: struct size is 28 bytes for string size 5-8
                                  28 for string size 9-12
                                  32 for string size 13-16
                                  36 for string size 17-20
                                  40 for string size 21-24
                                  44 for string size 25-28
                                  48 for string size 29-32
                                  52 for string size 33-36
*/
/* should be at least 8 to hold base-36 32-bit integers */
/* should be at least 15 to hold base-36 64-bit integers */
#define BIG_STRUCT_STRING_SIZE   17

#define ASSERT_CODE              0 /* N.B. ASSERT_CODE > 1 may affect comparison and swap counts */
#define DEBUG_CODE               0
#define GENERATOR_TEST           0
#define SILENCE_WHINEY_COMPILERS 0

#define REPARTITION_DEBUG_LEVEL  2  /* low to print debug output, high to hide */
#define AQCMP_DEBUG_LEVEL        3

#define TEST_TIMEOUT             5.0  /* seconds (default value) */

#define RESULTS_COLUMN           72

#define MAX_ARRAY_PRINT          290UL

#define QUICKSELECT_ARG_CHECKS   DEBUG_CODE

#define SWAP_FUNCTION            1 /* else swap by passed typeindex */

#define DISCREPANCY_THRESHOLD    1.05

/* for even-sized arrays, return lower- (else upper-) median */
#define LOWER_MEDIAN             0

#define BM_INSERTION_CUTOFF      7UL  /* 7UL in original B&M code */
#define INSERTION_CUTOFF         6UL  /* insertion sort arrays this size or smaller (like B&M) */

/* When repivoting is necessary using median-of-medians, the middle third of
   the array is partitioned as a result of finding the median of the medians.
   This partial partitioning may be preserved to avoid some recomparisons
   when repartitioning. Careful coding does not affect regular partitioning,
   but object file size may increase due to the additional code required to
   preserve and use the partial partitioning information to avoid some
   comparisons. (Or object file might be smaller when saving partial
   partitioning, depending on compiler options.)
   Saving partial partitioning recomparisons improves performance on adverse
   inputs by about 2% for sorting, about 10% for selection.  But it adds a
   little overhead for non-adverse inputs.
*/
/* N.B. macro SAVE_PARTIAL builds code; use is controlled by variable
   save_partial, which defaults to 0U (no savings) and can be set with command-
   line option -5.
*/
#ifndef SAVE_PARTIAL
# define SAVE_PARTIAL            1
#endif

#define SAMPLING_DENOMINATOR     3 /* 3 (uniform spacing) or 4 (organ-pipe optimum) */

#define USE_FMED5                0 /* using median-of-5 from 25 <= nmemb < 81 raises sorting and selection cost */

/* Sorting networks vs. insertion sort for small sub-array sizes 3-7:
   1 for sorting network, 0 for insertion sort.
   Sizes 2 and 8 are unconditionally better with sorting networks.
   Size 3 insertion sort uses fewer average comparisons and runs faster than network
   Size 4 network sort runs 5% faster, has only 1.6% more comparisons, fewer swaps
   Size 5 insertion sort uses average 15% fewer comparisons than network, but 0.86 N log(N) for reversed input
   Size 6 insertion sort uses 9.09% fewer comparisons than network, but 0.97 N log(N) for reversed input
   Size 7 network sort runs 9.5% faster than insertion sort but has 7.4% more comparisons; insertion sort 1.07 N log(N) for reversed input
*/
#define NETWORK3 0
#define NETWORK4 1
#define NETWORK5 0
#define NETWORK6 1
#define NETWORK7 1

/* nothing (much) to configure below this line */

#if ! DEBUG_CODE
# if ! ASSERT_CODE
#  define NDEBUG 1
# endif
#endif

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE 1
#endif

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: median_test.c ~|^` @(#)"
#define SOURCE_MODULE "%M%"
#define MODULE_VERSION "%I%"
#define MODULE_DATE "%O%"
#define COPYRIGHT_HOLDER "%Q%"
#define COPYRIGHT_DATE "2016-2017"

/* configuration for swap2, before including exchange.h */
#define EXCHANGE_DO_SHORT 1     /* 1 support operation size tests on short */
#define EXCHANGE_DO_INT   1     /* 1 bigger, but also faster */
#define EXCHANGE_DO_LONG  1     /* 1 bigger, but also faster */

/* local header files needed */
#include "compare.h"            /* charcmp ... */
#include "exchange.h"           /* exchange swap swap2 */
#include "get_host_name.h"      /* get_host_name */
#include "get_ips.h"            /* get_ips */
#include "logger.h"             /* logger */
#include "paths_decl.h"         /* path_basename */
#include "quickselect.h"        /* quickselect qsort_wrapper */
#include "random64n.h"          /* random64n */
#include "shuffle.h"            /* fisher_yates_shuffle */
#include "snn.h"                /* sng snl snlround snul */
#include "timespec.h"           /* timespec_subtract timespec_to_double */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <sys/param.h>          /* MAXHOSTNAMELEN */
#include <sys/resource.h>       /* RUSAGE_* struct rusage */
#include <sys/time.h>           /* struct timeval */
#include <sys/times.h>          /* times */
#include <sys/types.h>          /* *_t */
#include <assert.h>             /* assert */
#include <ctype.h>              /* isalpha isdigit ispunct isspace tolower */
#include <errno.h>
#include <limits.h>             /* *_MAX *_BIT */
#include <math.h>               /* log2 */
#include <regex.h>              /* regcomp regerror regexec regfree */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>             /* strtol strtoul */
#include <string.h>             /* memcpy strrchr strstr */
#include <time.h>               /* CLOCK_* clock_gettime */
#include <unistd.h>             /* getpid */

#ifndef SIZE_T_MAX /* not defined on Solaris */
# define SIZE_T_MAX ULONG_MAX
#endif

#undef optind

#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

#define OPTSTRING "aAb:BcC:d:D:eEfFgGhHiIjJ:k:KlLm:M:nNoO:pPq:Q:r:RsStT:uU:vVwW:xXy:Y:zZ:0:1:2:3:4:56:7:8:9!#:"
#define USAGE_STRING     "[-a] [-A] [-b [0]] [-B] [-c [c1[,c2[,c3[...]]]]] [-C sequences] [-d [n]] [-D [n]] [-e] [-E] [-f] [-F] [-g] [-G] [-h] [-H] [-i] [-I] [-j] [-J [gap[,gap[,...]]]] [-k col] [-l] [-L] [-m [t[,n[,f[,c[,l]]]]] [-M [m3,n]] [-n] [-N] [-o] [-O n] [-P] [-q [n[,f[,c[,l]]]]] [-Q timeout] [-r [i[,n[,f]]]] [-R] [-s] [-S] [-t] [-T sequences] [-u] [-U n] [-v] [-V] [-w] [-W [f,c[,l]]] [-x] [-X] [-y [n]] [-z] [-0 name] [-1 f,f,f...] -2 [f,f,f...] [-3 [c1[,c2[,c3[...]]]]] [-4 c] [-5] [-6 name] [-7 [f,f,f...]] [-8 [f,f,f...]] [-9] [-!] [-# n] -- [[start incr]] [size [count]]\n\
-a\ttest with McIlroy quicksort adversary\n\
-A\talphabetic (string) data type tests\n\
-b [0]\ttest Bentley&McIlroy qsort optionally w/o instrumentation\n\
-B\toutput data for drawing box plots\n\
-c\ttest simplified sort-only quickselect\n\
-C sequences\tinclude correctness tests for specified sequences\n\
-d [n]\tturn on debugging output\n\
-D [n]\ttest dual-pivot qsort code with optional small-array sort cutoff\n\
-e\ttest heapsort code\n\
-E\ttest selection sort\n\
-f\ttest sqrt sort\n\
-F\tfloating-point (double) tests\n\
-g\ttest glibc qsort code\n\
-G\tuse Shell sort instead of insertion sort for small arrays\n\
-h\tomit program headings (args, parameters, machine)\n\
-H\toutput host information\n\
-i\tuse instrumented comparison functions and report results\n\
-I\tinteger data type tests\n\
-j\ttest insertion sort\n\
-J [gap[,gap[,...]]]\ttest Shell sort with specified gaps\n\
-k col\t align timing numbers at column col\n\
-K\twrite integer sequence generated by adversary (if given with -a)\n\
-l\ttest library qsort\n\
-L\tlong integer data type tests\n\
-m [t[,n[,f[,c[,l]]]]]\ttest quickselect selection with optional order statistics distribution type, number of order statistics, repivot_factor, repivot_cutoff, lopsided_partition_limit\n\
-M [m3,n]\ttest modified Bentley&McIlroy qsort with optional cutoff values for median-of-3 and ninther\n\
-n\tdo nothing except as specified by option flags\n\
-N\ttest NetBSD qsort code\n\
-o\tprint execution costs of operations on basic types\n\
-O n\t set sampling nmemb breakpoints according to Beta value n\n\
-p\tswap pivot initially (normally deferred)\n\
-P\tpointer to structure data type tests\n\
-q [n[,f[,c]]]\ttest quickselect sort with optional small-array sort cutoff, repivot_factor, and repivot_cutoff and lopsided_partition_limit\n\
-Q timeout\ttime out tests after timeout seconds\n\
-r [i[,n[,f]]]\ttest introsort sort with optional final insertion sort flag, small-array sort cutoff and recursion depth factor\n\
-R\traw timing output (median time (and comparison counts if requested) only)\n\
-s\tprint sizes of basic types\n\
-S\tstructure data type tests\n\
-t\tthorough tests\n\
-T sequences\tinclude timing tests for specified sequences\n\
-u\tuse a copy of the pivot for comparisons\n\
-U n\t set sampling nmemb breakpoints according to factor n\n\
-v\ttest sorting networks for small arrays\n\
-V\tuse sorting networks instead of insertion sort for small arrays\n\
-w\ttest qsort_wrapper\n\
-W [f,c]\ttest lopsided partitions in quickselect with optional repivot_factor and repivot_cutoff and lopsided_partition_limit\n\
-x\ttest log sort\n\
-X\ttest smoothsort\n\
-y [n]\tYaroslavskiy's dual-pivot sort with optional insertion sort cutoff\n\
-Y[c|o|s|t][+|-]\tattempt to optimize quickselect sampling breakpoints for selection for comparisons, operations, swaps, or time\n\
-z\tset repeatable random number generator state\n\
-Z[c|o|s|t][+|-]\tattempt to optimize quickselect sampling breakpoints for sorting for comparisons, operations, swaps, or time\n\
-0 name\tselect sorting repivot table by name (disabled, aggressive, loose, relaxed, transparent, experimental)\n\
-1 f,f,f...\tset sorting repivot factor1 values\n\
-2 f,f,f...\tset sorting repivot factor2 values\n\
-3 c1[,c2[,...]]\tset sampling table breakpoint sizes\n\
-4 c\tset small-array sort cutoff to c\n\
-5\tsave partial partitioning comparisons\n\
-6 name\tselect selection repivot table by name (disabled, aggressive, loose, relaxed, transparent, experimental)\n\
-7 f,f,f...\tset selection repivot factor1 values\n\
-8 f,f,f...\tset selection repivot factor2 values\n\
-9\ttest plan9 qsort\n\
-!\tcollect and report statistics for small array insertion sort\n\
-# n\tsort if nk/nmemb>k\n\
start\tbegin testing with array size\n\
incr\tincrement array size (prefix with * for geometric sequence)\n\
size\tnumber of items in each test (maximum size if start is given) (default 10000)\n\
count\tnumber of times to run each test (default 10)"

/* repivot tuning */
#define DISABLED                               0
#define TRANSPARENT                            1 /* (almost) no repivots for random inputs */
#define LOOSE                                  2 /* adversary < 2 N log N */
#define RELAXED                                3 /* adversary < 1.5 N log N */
#define AGGRESSIVE                             4 /* penalty < 0.1% */
#define EXPERIMENTAL                           5 /* experimental */

/* space-saving */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* data types */
#define DATA_TYPE_LONG                         1
#define DATA_TYPE_INT                          2
#define DATA_TYPE_DOUBLE                       3
#define DATA_TYPE_STRUCT                       4
#define DATA_TYPE_STRING                       5
#define DATA_TYPE_POINTER                      6

/* test sequence macros (could be an enum) */
#define TEST_SEQUENCE_STDIN                    0U  /* 0x000001 */ /* must be first (some lopps start at 1 to avoid STDIN) */
#define TEST_SEQUENCE_SORTED                   1U  /* 0x000002 */
#define TEST_SEQUENCE_REVERSE                  2U  /* 0x000004 */
#define TEST_SEQUENCE_ORGAN_PIPE               3U  /* 0x000008 */
#define TEST_SEQUENCE_INVERTED_ORGAN_PIPE      4U  /* 0x000010 */
#define TEST_SEQUENCE_ROTATED                  5U  /* 0x000020 */ /* after organ-pipe for graphing */
#define TEST_SEQUENCE_SAWTOOTH                 6U  /* 0x000040 */ /* 3 distinct values, repeated sequence */
#define TEST_SEQUENCE_TERNARY                  7U  /* 0x000080 */ /* 3 distinct values, random */
#define TEST_SEQUENCE_BINARY                   8U  /* 0x000100 */ /* 2 distinct values */
#define TEST_SEQUENCE_CONSTANT                 9U  /* 0x000200 */ /* only 1 value */
#define TEST_SEQUENCE_MANY_EQUAL_LEFT          10U /* 0x000400 */
#define TEST_SEQUENCE_MANY_EQUAL_MIDDLE        11U /* 0x000800 */
#define TEST_SEQUENCE_MANY_EQUAL_RIGHT         12U /* 0x001000 */
#define TEST_SEQUENCE_MANY_EQUAL_SHUFFLED      13U /* 0x002000 */
#define TEST_SEQUENCE_RANDOM_DISTINCT          14U /* 0x004000 */
#define TEST_SEQUENCE_RANDOM_VALUES            15U /* 0x008000 */
#define TEST_SEQUENCE_RANDOM_VALUES_LIMITED    16U /* 0x010000 */
#define TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED 17U /* 0x020000 */
#define TEST_SEQUENCE_RANDOM_VALUES_NORMAL     18U /* 0x040000 */
#define TEST_SEQUENCE_HISTOGRAM                19U /* 0x080000 */
#define TEST_SEQUENCE_MEDIAN3KILLER            20U /* 0x100000 */
#define TEST_SEQUENCE_DUAL_PIVOT_KILLER        21U /* 0x200000 */
#define TEST_SEQUENCE_JUMBLE                   22U /* 0x400000 */ /* worst-case swaps for sorting networks */
#define TEST_SEQUENCE_PERMUTATIONS             23U  /* also via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_COMBINATIONS             24U  /* also via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_ADVERSARY                25U  /* also via TEST_TYPE_ADVERSARY */

#define TEST_SEQUENCE_COUNT                    26U

#define MAX_PERMUTATION_SIZE                   20  /* 20! is huge */
#define MAX_COMBINATION_SIZE                   ((LONG_BIT)-1) /* avoid sign bit */

#define EXPECTED_RANDOM_MIN                    (0)
#define EXPECTED_RANDOM_MAX                    (INT_MAX) /* XXX might not work for long */

#define MAXIMUM_SAWTOOTH_MODULUS               3  /* ensures maximum 3 distinct values */
#define CONSTANT_VALUE                         3  /* a more-or-less random value */

/* 32-bit signed integers in base-36 uses at least 8 chars (incl. '\0') */
/* 64-bit signed integers in base-36 uses at least 15 chars (incl. '\0') */
#if ULONG_MAX > 0xffffffffUL
# if BIG_STRUCT_STRING_SIZE < 15
#  undef BIG_STRUCT_STRING_SIZE
#  define BIG_STRUCT_STRING_SIZE               15
# endif
#else
# if BIG_STRUCT_STRING_SIZE < 8
#  undef BIG_STRUCT_STRING_SIZE
#  define BIG_STRUCT_STRING_SIZE               8
# endif
#endif

struct big_struct {
    double d;
    long l;
    int val;
    char string[BIG_STRUCT_STRING_SIZE];
};

/* aligned data block */
union aligned_union {
    void *p;
    long l;
    double d;
};

/* definitions:
   size is the size of the element type in bytes as returned by sizeof()
*/

static char median_test_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
static int debug=0;
static int pivot_swap=0;
static int pivot_copy=0;
static int use_shell=0;
static int use_networks=0;
static const char *build_options = "@(#)median_test.c: " "built with configuration options: "
                  "LOWER_MEDIAN=" xbuildstr(LOWER_MEDIAN)
                  ", ASSERT_CODE=" xbuildstr(ASSERT_CODE)
                  ", DEBUG_CODE=" xbuildstr(DEBUG_CODE)
                  ", BIG_STRUCT_STRING_SIZE=" xbuildstr(BIG_STRUCT_STRING_SIZE)
                  ", GENERATOR_TEST=" xbuildstr(GENERATOR_TEST)
#ifdef EXCHANGE_DO_LONG
                  ", EXCHANGE_DO_LONG=" xbuildstr(EXCHANGE_DO_LONG)
#endif
#ifdef EXCHANGE_DO_INT
                  ", EXCHANGE_DO_INT=" xbuildstr(EXCHANGE_DO_INT)
#endif
#ifdef EXCHANGE_DO_SHORT
                  ", EXCHANGE_DO_SHORT=" xbuildstr(EXCHANGE_DO_SHORT)
#endif
                  ", BM_INSERTION_CUTOFF=" xbuildstr(BM_INSERTION_CUTOFF)
                  ", TEST_TIMEOUT=" xbuildstr(TEST_TIMEOUT)
#ifdef SAVE_PARTIAL
    ", SAVE_PARTIAL=" xbuildstr(SAVE_PARTIAL)
#endif
#ifdef SWAP_FUNCTION
    ", SWAP_FUNCTION=" xbuildstr(SWAP_FUNCTION)
#endif
                  ;
static struct big_struct *input_data=NULL;
/* for instrumented comparisons and swaps: */
static size_t nlt, neq, ngt, nsw;

/* Element swapping code depends on the size and alignment of elements. */
/* Assume basic types sizeof(foo) etc. are powers of 2. */
#define NTYPES 6 /* double, pointer, long, int, short, char */

/* alignment; no fiddly bits */
#ifndef is_aligned /* otherwise use definition from exchange.h */
# define is_aligned(var,shift) (0U==(((unsigned long)(var))&bitmask[(shift)]))
#endif

/* insertion sort parameters */
static unsigned int introsort_final_insertion_sort = 1U;
static size_t introsort_small_array_cutoff = 0UL; /* sentinel value */
static size_t introsort_recursion_factor = 2UL;

static size_t quickselect_small_array_cutoff = 8UL;

static unsigned int instrumented = 0U;
static int instrumented_bmqsort = 1;

/* for selection */
static size_t selection_nk = 1UL;
/* for repivoting */
static size_t npartitions, nrepivot;
static size_t repivot_factor = 999UL;
/* random shuffle @nmemb=80, repivot_factor=39 (lopsided_partition_limit=1, no_aux_repivot=1), ~0.014% repivots */
static size_t repivot_factor2 = 999UL;
static size_t repivot_cutoff = 999UL;
static int lopsided_partition_limit = 9;
static int no_aux_repivot = 0;
#if SAVE_PARTIAL
static unsigned int save_partial = 0U;
#endif
static size_t sort_threshold = 10UL;

/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
*/
struct sampling_table_struct {
    size_t min_nmemb;    /* smallest subarray for this number of samples */
    size_t samples;      /* the number of samples used for pivot selection */
};
struct repivot_table_struct {
    size_t factor1;      /* lopsided ratio for immediate repivoting */
    size_t factor2;      /* lopsided ratio for repivoting on 2nd occurrence */
};
struct stats_table_struct {
    size_t max_ratio; /* for statistics */
    size_t repivots;  /* for statistics */
    size_t repivot_ratio;  /* for statistics */
};
struct breakpoint_table_struct {
    size_t min_min_bp;    /* for optimization */
    size_t max_min_bp;    /* for optimization */
    size_t min_max_bp;    /* for optimization */
    size_t max_max_bp;    /* for optimization */
    size_t min_min_nmemb; /* for optimization */
    size_t max_min_nmemb; /* for optimization */
    size_t min_max_nmemb; /* for optimization */
    size_t max_max_nmemb; /* for optimization */
    size_t optimum_bp;    /* for optimization */
};
/* XXX N.B. different sampling tables may have different sizes */
static struct sampling_table_struct middle_sampling_table[] = {
   {                     1UL,             1UL }, /* single sample */
   {                     3UL,             3UL }, /* median-of-3 */
# if USE_FMED5
   {                     5UL,             5UL }, /* median of 5 */
# endif
   {                    27UL,             9UL }, /* remedian of samples */
   {                   140UL,            27UL },
   {                   729UL,            81UL },
   {                  3788UL,           243UL },
   {                 19683UL,           729UL },
   {                102276UL,          2187UL },
   {                531441UL,          6561UL },
   {               2761448UL,         19683UL },
   {              14348907UL,         59049UL },
   {              55768465UL,        177147UL },
   {             387420489UL,        531441UL },
   {            2013095913UL,       1594323UL },
#if (SIZE_T_MAX) > 4294967295
   /* silly to include large sizes? check again in a decade or two */
   {           10460353203UL,       4782969UL },
   {           54353589638UL,      14348907UL },
   {          282429536481UL,      43046721UL },
   {         1467546920230UL,     129140163UL },
   {         7625597484987UL,     387420489UL },
   {        39623766846201UL,    1162261467UL },
   {       205891132094649UL,    3486784401UL },
   {      1069841704847422UL,   10460353203UL },
   {      5559060566555524UL,   31381059609UL },
   {     28885726030880400UL,   94143178827UL },
   {    150094635296999000UL,  282429536481UL },
   {    779914602833770000UL,  847288609443UL },
   {   4052555153018980000UL, 2541865828329UL },
   {   (SIZE_T_MAX),          7625597484987UL }
#else
   {   (SIZE_T_MAX),                4782969UL }
#endif
};
static struct sampling_table_struct separated_sampling_table[] = {
   {                     1UL,          1UL }, /* single sample */
   {                     3UL,          3UL }, /* median-of-3 */
# if USE_FMED5
   {                     5UL,          5UL }, /* median of 5 */
# endif
   {                   102UL,          9UL }, /* remedian of samples */
   {                  2756UL,         27UL },
   {                 74402UL,         81UL },
   {               2008847UL,        243UL },
   {              54238868UL,        729UL },
   {            1464449448UL,       2187UL },
#if (SIZE_T_MAX) > 4294967295
   /* silly to include large sizes? check again in a decade or two */
   {           39540135107UL,       6561UL },
   {         1067583647898UL,      19683UL },
   {        28824758493251UL,      59049UL },
   {       778268479317773UL,     177147UL },
   {     21013248941579900UL,     531441UL },
   {    567357721422657000UL,    1594323UL },
   {  15318658478411700000UL,    4782969UL },
   {   (SIZE_T_MAX),             4782969UL }
#else
   {   (SIZE_T_MAX),                6561UL }
#endif
};
static struct sampling_table_struct sorting_sampling_table[] = {
   {                     1UL,          1UL }, /* single sample */
   {                     3UL,          3UL }, /* median-of-3 */
# if USE_FMED5
   {                    25UL,          5UL }, /* median of 5 */
# endif
   {                    69UL,          9UL }, /* remedian of samples */
   {                   617UL,         27UL },
   {                  5554UL,         81UL },
   {                 49989UL,        243UL },
   {                449904UL,        729UL },
   {               4049135UL,       2187UL },
   {              36442217UL,       6561UL },
   {             327979954UL,      19683UL },
   {            2951819589UL,      59049UL },
#if (SIZE_T_MAX) > 4294967295
   /* silly to include large sizes? check again in a decade or two */
   {           26566376300UL,     177147UL }, /* silly to include large sizes?
                                              check again in a decade or two */
   {          239097386700UL,     531441UL },
   {         2151876480298UL,    1594323UL },
   {        19366888322686UL,    4782969UL },
   {       174301994904175UL,   14348907UL },
   {      1568717954137576UL,   43046721UL },
   {     14118461587238200UL,  129140163UL },
   {    127066154285144000UL,  387420489UL },
   {   1143595388566290000UL, 1162261467UL },
   {  10292358497096600000UL, 3486784401UL },
   {   (SIZE_T_MAX),          3486784401UL }
#else
   {   (SIZE_T_MAX),              177147UL }
#endif
};

static struct repivot_table_struct selection_repivot_table_aggressive[] = {
   {  7UL,  2UL }, /* median-of-3 */ /* aggressive */
# if USE_FMED5
   {  8UL,  2UL }, /* median of 5 */
# endif
   {  8UL,  2UL }, /* remedian of samples */
   { 13UL,  2UL }, /* aggressive */
   {  6UL,  2UL }, /* aggressive */
   {  3UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct selection_repivot_table_disabled[] = {
   { 34UL, 31UL }, /* disabled */
# if USE_FMED5
   { 40UL, 10UL }, /* median of 5 */
# endif
   { 90UL, 28UL }, /* disabled */
   { 86UL, 43UL }, /* disabled */
   {  6UL,  3UL }, /* disabled */
   {  3UL,  2UL }, /* disabled */
   {  2UL,  2UL }, /* disabled */
};

static struct repivot_table_struct selection_repivot_table_loose[] = {
   { 34UL, 16UL }, /* median-of-3 */ /* loose */
# if USE_FMED5
   { 43UL, 16UL }, /* median of 5 */
# endif
   { 90UL, 19UL }, /* remedian of samples */
   { 86UL, 41UL }, /* loose */
   {  6UL,  3UL }, /* loose */
   {  3UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct selection_repivot_table_relaxed[] = {
   { 19UL,  9UL }, /* median-of-3 */ /* relaxed */
# if USE_FMED5
   { 19UL,  9UL }, /* median of 5 */
# endif
   { 16UL, 10UL }, /* remedian of samples */
   { 60UL, 33UL }, /* relaxed */
   {  6UL,  3UL }, /* relaxed */
   {  2UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct selection_repivot_table_transparent[] = {
   { 34UL, 30UL }, /* median-of-3 */ /* transparent */
# if USE_FMED5
   { 40UL, 10UL }, /* median of 5 */
# endif
   { 90UL, 27UL }, /* remedian of samples */
   { 86UL, 40UL }, /* transparent */
   {  6UL,  3UL }, /* transparent */
   {  2UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct selection_repivot_table_experimental[] = {
   { 34UL, 16UL }, /* median-of-3 */ /* experimental */
# if USE_FMED5
   { 43UL, 16UL }, /* median of 5 */
# endif
   { 90UL, 16UL }, /* remedian of samples */
   { 86UL, 16UL }, /* experimental */
   {  6UL,  3UL }, /* experimental */
   {  3UL,  2UL }, /* experimental */
   {  2UL,  2UL }, /* experimental */
};

static struct repivot_table_struct sorting_repivot_table_aggressive[] = {
   { 18UL,  8UL }, /* median-of-3 */ /* aggressive */
# if USE_FMED5
   { 19UL, 10UL }, /* median of 5 */
# endif
   { 16UL, 10UL }, /* remedian of samples */
   { 60UL, 33UL }, /* aggressive */
   {  6UL,  3UL }, /* aggressive */
   {  2UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct sorting_repivot_table_disabled[] = {
   { 34UL, 31UL }, /* disabled */
# if USE_FMED5
   { 40UL, 10UL }, /* median of 5 */
# endif
   { 90UL, 28UL }, /* disabled */
   { 86UL, 43UL }, /* disabled */
   {  6UL,  3UL }, /* disabled */
   {  3UL,  2UL }, /* disabled */
   {  2UL,  2UL }, /* disabled */
};

static struct repivot_table_struct sorting_repivot_table_loose[] = {
   { 34UL, 16UL }, /* median-of-3 */ /* loose */
# if USE_FMED5
   { 43UL, 16UL }, /* median of 5 */
# endif
   { 90UL, 19UL }, /* remedian of samples */
   { 86UL, 41UL }, /* loose */
   {  6UL,  3UL }, /* loose */
   {  3UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct sorting_repivot_table_relaxed[] = {
   { 19UL,  9UL }, /* median-of-3 */ /* relaxed */
# if USE_FMED5
   { 19UL,  9UL }, /* median of 5 */
# endif
   { 16UL, 10UL }, /* remedian of samples */
   { 60UL, 33UL }, /* relaxed */
   {  6UL,  3UL }, /* relaxed */
   {  2UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct sorting_repivot_table_transparent[] = {
   { 34UL, 30UL }, /* median-of-3 */ /* transparent */
# if USE_FMED5
   { 40UL, 10UL }, /* median of 5 */
# endif
   { 90UL, 27UL }, /* remedian of samples */
   { 86UL, 40UL }, /* transparent */
   {  6UL,  3UL }, /* transparent */
   {  2UL,  2UL },
   {  2UL,  2UL },
};

static struct repivot_table_struct sorting_repivot_table_experimental[] = {
   { 34UL, 16UL }, /* median-of-3 */ /* experimental */
# if USE_FMED5
   { 40UL, 16UL }, /* median of 5 */
# endif
   { 90UL, 16UL }, /* remedian of samples */
   { 86UL, 16UL }, /* experimental */
   {  6UL,  3UL }, /* experimental */
   {  3UL,  2UL }, /* experimental */
   {  2UL,  2UL }, /* experimental */
};
static const char *sorting_repivot_table_name = "loose"; /* default */
static const char *selection_repivot_table_name = "aggressive"; /* default */
static struct repivot_table_struct *sorting_repivot_table = sorting_repivot_table_loose; /* default */
static struct repivot_table_struct *selection_repivot_table = selection_repivot_table_aggressive; /* default */
static size_t repivot_table_size = sizeof(sorting_repivot_table_experimental)/sizeof(sorting_repivot_table_experimental[0]);
static struct stats_table_struct stats_table[30]; /* big enough to match biggest sampling_table_size */
static struct breakpoint_table_struct breakpoint_table[30]; /* big enough to match biggest sampling_table_size */
#if 0
3 2-3  2
4 3-6  4
5 4-10 7
6 5-15 11
7 6-21 16
8 7-28 22
9 8-36 29
10 9-45 37
#endif
/* Selection breakpoints for nmemb 6 through 261:
   First index is nmemb-6UL
   Second index: 0 for distributed order statistics
                 1 for order statistics grouped in middle of subarray
                 2 for order statistics grouped away from middle of subarray
                 3 for order statistics grouped at one end of subarray
   Value is maximum number of desired order statistics with a certain
     distribution which can be selected via multiple selection; for more
     order statistics, sort the subarray.
*/
static unsigned char selection_breakpoint[256][4] = {
    /* d     m     s     l,r     nmemb      d     m     s     l,r    nmemb */
    {  1U,   2U,   1U,   3U }, /*   6 */ {  0U,   0U,   1U,   2U }, /*   7 */
    {  1U,   2U,   1U,   4U }, /*   8 */ {  1U,   2U,   1U,   4U }, /*   9 */
    {  2U,   4U,   4U,   6U }, /*  10 */ {  2U,   2U,   5U,   6U }, /*  11 */
    {  2U,   6U,   6U,   7U }, /*  12 */ {  3U,   5U,   7U,   8U }, /*  13 */
    {  3U,   8U,   8U,   9U }, /*  14 */ {  3U,   8U,   8U,  10U }, /*  15 */
    {  3U,  10U,   9U,  10U }, /*  16 */ {  3U,  10U,   9U,  11U }, /*  17 */
    {  3U,  12U,   9U,  12U }, /*  18 */ {  3U,  12U,  10U,  13U }, /*  19 */
    {  3U,  12U,  11U,  13U }, /*  20 */ {  3U,  12U,  12U,  14U }, /*  21 */
    {  4U,  14U,  13U,  15U }, /*  22 */ {  4U,  14U,  14U,  16U }, /*  23 */
    {  4U,  14U,  14U,  16U }, /*  24 */ {  4U,  14U,  14U,  17U }, /*  25 */
    {  4U,  16U,  15U,  18U }, /*  26 */ {  4U,  16U,  16U,  18U }, /*  27 */
    {  4U,  16U,  17U,  19U }, /*  28 */ {  4U,  16U,  18U,  20U }, /*  29 */
    {  5U,  18U,  18U,  21U }, /*  30 */ {  5U,  18U,  19U,  22U }, /*  31 */
    {  5U,  18U,  20U,  22U }, /*  32 */ {  5U,  18U,  20U,  23U }, /*  33 */
    {  5U,  20U,  20U,  23U }, /*  34 */ {  5U,  20U,  20U,  23U }, /*  35 */
    {  5U,  20U,  20U,  23U }, /*  36 */ {  5U,  20U,  20U,  23U }, /*  37 */
    {  5U,  22U,  20U,  23U }, /*  38 */ {  5U,  22U,  20U,  23U }, /*  39 */
    {  6U,  22U,  28U,  30U }, /*  40 */ {  6U,  22U,  28U,  31U }, /*  41 */
    {  6U,  24U,  28U,  32U }, /*  42 */ {  6U,  24U,  28U,  32U }, /*  43 */
    {  6U,  26U,  28U,  32U }, /*  44 */ {  6U,  26U,  28U,  32U }, /*  45 */
    {  6U,  28U,  28U,  32U }, /*  46 */ {  6U,  28U,  28U,  32U }, /*  47 */
    {  6U,  30U,  28U,  32U }, /*  48 */ {  6U,  30U,  28U,  32U }, /*  49 */
    {  7U,  32U,  33U,  37U }, /*  50 */ {  7U,  32U,  33U,  37U }, /*  51 */
    {  7U,  34U,  33U,  37U }, /*  52 */ {  7U,  34U,  33U,  37U }, /*  53 */
    {  7U,  36U,  33U,  37U }, /*  54 */ {  7U,  36U,  33U,  37U }, /*  55 */
    {  7U,  37U,  33U,  37U }, /*  56 */ {  7U,  32U,  33U,  37U }, /*  57 */
    {  7U,  32U,  33U,  37U }, /*  58 */ {  7U,  32U,  33U,  37U }, /*  59 */
    {  8U,  42U,  44U,  48U }, /*  60 */ {  8U,  42U,  44U,  48U }, /*  61 */
    {  8U,  44U,  44U,  48U }, /*  62 */ {  8U,  44U,  44U,  48U }, /*  63 */
    {  8U,  46U,  44U,  48U }, /*  64 */ {  8U,  46U,  44U,  48U }, /*  65 */
    {  8U,  48U,  44U,  48U }, /*  66 */ {  8U,  48U,  44U,  48U }, /*  67 */
    {  8U,  48U,  44U,  48U }, /*  68 */ {  8U,  43U,  44U,  48U }, /*  69 */
    { 10U,  52U,  54U,  60U }, /*  70 */ { 10U,  52U,  56U,  61U }, /*  71 */
    { 10U,  54U,  54U,  62U }, /*  72 */ { 10U,  54U,  56U,  63U }, /*  73 */
    { 10U,  56U,  56U,  64U }, /*  74 */ { 10U,  56U,  58U,  65U }, /*  75 */
    { 10U,  58U,  58U,  66U }, /*  76 */ { 10U,  58U,  60U,  67U }, /*  77 */
    { 10U,  60U,  60U,  68U }, /*  78 */ { 10U,  60U,  62U,  69U }, /*  79 */
    { 19U,  62U,  62U,  70U }, /*  80 */ { 19U,  62U,  62U,  71U }, /*  81 */
    { 20U,  64U,  62U,  72U }, /*  82 */ { 19U,  64U,  64U,  73U }, /*  83 */
    { 24U,  66U,  64U,  74U }, /*  84 */ { 20U,  66U,  66U,  75U }, /*  85 */
    { 20U,  68U,  66U,  76U }, /*  86 */ { 20U,  68U,  68U,  77U }, /*  87 */
    { 21U,  70U,  68U,  78U }, /*  88 */ { 21U,  70U,  70U,  79U }, /*  89 */
    { 21U,  72U,  68U,  80U }, /*  90 */ { 21U,  72U,  70U,  81U }, /*  91 */
    { 21U,  74U,  70U,  80U }, /*  92 */ { 22U,  74U,  72U,  82U }, /*  93 */
    { 27U,  76U,  72U,  84U }, /*  94 */ { 22U,  76U,  74U,  85U }, /*  95 */
    { 23U,  78U,  74U,  86U }, /*  96 */ { 23U,  78U,  75U,  86U }, /*  97 */
    { 24U,  80U,  76U,  88U }, /*  98 */ { 23U,  80U,  78U,  89U }, /*  99 */
    { 23U,  82U,  78U,  90U }, /* 100 */ { 28U,  82U,  80U,  91U }, /* 101 */
    { 29U,  84U,  80U,  92U }, /* 102 */ { 26U,  84U,  82U,  93U }, /* 103 */
    { 24U,  86U,  82U,  94U }, /* 104 */ { 24U,  86U,  84U,  95U }, /* 105 */
    { 25U,  88U,  84U,  96U }, /* 106 */ { 24U,  88U,  86U,  97U }, /* 107 */
    { 25U,  90U,  86U,  98U }, /* 108 */ { 25U,  90U,  88U,  99U }, /* 109 */
    { 26U,  92U,  88U, 100U }, /* 110 */ { 30U,  92U,  90U, 101U }, /* 111 */
    { 26U,  94U,  90U, 102U }, /* 112 */ { 29U,  94U,  92U, 103U }, /* 113 */
    { 25U,  96U,  92U, 104U }, /* 114 */ { 27U,  96U,  94U, 105U }, /* 115 */
    { 28U,  98U,  94U, 106U }, /* 116 */ { 28U,  98U,  96U, 107U }, /* 117 */
    { 30U, 100U,  96U, 108U }, /* 118 */ { 28U, 100U,  98U, 109U }, /* 119 */
    { 25U, 102U,  98U, 110U }, /* 120 */ { 29U, 102U, 100U, 111U }, /* 121 */
    { 29U, 104U, 100U, 112U }, /* 122 */ { 29U, 104U, 102U, 113U }, /* 123 */
    { 30U, 106U, 102U, 114U }, /* 124 */ { 30U, 106U, 104U, 115U }, /* 125 */
    { 30U, 108U, 104U, 116U }, /* 126 */ { 30U, 108U, 106U, 117U }, /* 127 */
    { 30U, 110U, 106U, 118U }, /* 128 */ { 31U, 110U, 108U, 119U }, /* 129 */
    { 31U, 112U, 108U, 120U }, /* 130 */ { 27U, 112U, 110U, 121U }, /* 131 */
    { 31U, 114U, 110U, 122U }, /* 132 */ { 25U, 114U, 112U, 123U }, /* 133 */
    { 25U, 116U, 112U, 124U }, /* 134 */ { 26U, 116U, 114U, 125U }, /* 135 */
    { 26U, 118U, 114U, 126U }, /* 136 */ { 26U, 118U, 116U, 127U }, /* 137 */
    { 33U, 120U, 116U, 128U }, /* 138 */ { 24U, 120U, 118U, 129U }, /* 139 */
    { 26U, 122U, 118U, 130U }, /* 140 */ { 27U, 122U, 120U, 131U }, /* 141 */
    { 27U, 124U, 120U, 132U }, /* 142 */ { 27U, 124U, 122U, 133U }, /* 143 */
    { 27U, 126U, 122U, 134U }, /* 144 */ { 27U, 126U, 124U, 135U }, /* 145 */
    { 25U, 128U, 124U, 136U }, /* 146 */ { 47U, 128U, 126U, 137U }, /* 147 */
    { 43U, 130U, 126U, 138U }, /* 148 */ { 43U, 130U, 128U, 139U }, /* 149 */
    { 24U, 132U, 128U, 140U }, /* 150 */ { 44U, 132U, 130U, 141U }, /* 151 */
    { 39U, 134U, 130U, 142U }, /* 152 */ { 37U, 134U, 132U, 143U }, /* 153 */
    { 37U, 136U, 132U, 144U }, /* 154 */ { 44U, 136U, 134U, 145U }, /* 155 */
    { 25U, 138U, 134U, 146U }, /* 156 */ { 38U, 138U, 136U, 147U }, /* 157 */
    { 25U, 140U, 136U, 148U }, /* 158 */ { 25U, 140U, 138U, 149U }, /* 159 */
    { 39U, 142U, 138U, 150U }, /* 160 */ { 39U, 142U, 140U, 151U }, /* 161 */
    { 25U, 144U, 140U, 152U }, /* 162 */ { 24U, 144U, 142U, 153U }, /* 163 */
    { 46U, 146U, 142U, 154U }, /* 164 */ { 25U, 146U, 144U, 155U }, /* 165 */
    { 42U, 148U, 144U, 156U }, /* 166 */ { 26U, 148U, 146U, 157U }, /* 167 */
    { 22U, 150U, 146U, 158U }, /* 168 */ { 27U, 150U, 148U, 159U }, /* 169 */
    { 55U, 152U, 148U, 160U }, /* 170 */ { 26U, 152U, 150U, 161U }, /* 171 */
    { 56U, 154U, 150U, 162U }, /* 172 */ { 25U, 154U, 152U, 163U }, /* 173 */
    { 25U, 156U, 152U, 164U }, /* 174 */ { 27U, 156U, 154U, 165U }, /* 175 */
    { 26U, 158U, 154U, 166U }, /* 176 */ { 43U, 158U, 156U, 167U }, /* 177 */
    { 24U, 160U, 156U, 168U }, /* 178 */ { 24U, 160U, 158U, 169U }, /* 179 */
    { 24U, 162U, 158U, 170U }, /* 180 */ { 50U, 162U, 160U, 171U }, /* 181 */
    { 27U, 164U, 160U, 172U }, /* 182 */ { 53U, 164U, 162U, 173U }, /* 183 */
    { 23U, 166U, 162U, 173U }, /* 184 */ { 27U, 166U, 164U, 175U }, /* 185 */
    { 29U, 168U, 164U, 176U }, /* 186 */ { 25U, 168U, 166U, 177U }, /* 187 */
    { 25U, 170U, 166U, 178U }, /* 188 */ { 25U, 170U, 168U, 179U }, /* 189 */
    { 24U, 172U, 168U, 180U }, /* 190 */ { 26U, 172U, 170U, 181U }, /* 191 */
    { 26U, 174U, 170U, 182U }, /* 192 */ { 26U, 174U, 172U, 183U }, /* 193 */
    { 24U, 176U, 172U, 184U }, /* 194 */ { 59U, 176U, 174U, 185U }, /* 195 */
    { 26U, 178U, 174U, 186U }, /* 196 */ { 26U, 178U, 176U, 187U }, /* 197 */
    { 26U, 180U, 176U, 188U }, /* 198 */ { 27U, 180U, 178U, 188U }, /* 199 */
    { 27U, 182U, 178U, 190U }, /* 200 */ { 26U, 182U, 180U, 191U }, /* 201 */
    { 26U, 184U, 180U, 192U }, /* 202 */ { 26U, 184U, 182U, 193U }, /* 203 */
    { 25U, 186U, 182U, 193U }, /* 204 */ { 27U, 186U, 184U, 195U }, /* 205 */
    { 24U, 188U, 184U, 194U }, /* 206 */ { 27U, 188U, 186U, 197U }, /* 207 */
    { 26U, 190U, 186U, 197U }, /* 208 */ { 26U, 190U, 188U, 198U }, /* 209 */
    { 30U, 192U, 188U, 200U }, /* 210 */ { 27U, 192U, 190U, 200U }, /* 211 */
    { 26U, 193U, 190U, 201U }, /* 212 */ { 26U, 194U, 192U, 203U }, /* 213 */
    { 25U, 196U, 192U, 203U }, /* 214 */ { 26U, 196U, 194U, 204U }, /* 215 */
    { 27U, 198U, 194U, 204U }, /* 216 */ { 28U, 198U, 196U, 207U }, /* 217 */
    { 27U, 200U, 196U, 207U }, /* 218 */ { 28U, 200U, 198U, 209U }, /* 219 */
    { 27U, 202U, 198U, 210U }, /* 220 */ { 28U, 202U, 200U, 210U }, /* 221 */
    { 28U, 204U, 200U, 211U }, /* 222 */ { 27U, 204U, 202U, 211U }, /* 223 */
    { 27U, 206U, 202U, 213U }, /* 224 */ { 36U, 206U, 204U, 215U }, /* 225 */
    { 28U, 208U, 204U, 214U }, /* 226 */ { 28U, 208U, 206U, 215U }, /* 227 */
    { 27U, 210U, 206U, 216U }, /* 228 */ { 28U, 210U, 208U, 218U }, /* 229 */
    { 37U, 212U, 208U, 219U }, /* 230 */ { 29U, 212U, 210U, 220U }, /* 231 */
    { 37U, 214U, 210U, 221U }, /* 232 */ { 37U, 214U, 212U, 223U }, /* 233 */
    { 32U, 216U, 212U, 224U }, /* 234 */ { 28U, 210U, 214U, 223U }, /* 235 */
    { 38U, 218U, 214U, 226U }, /* 236 */ { 30U, 218U, 216U, 225U }, /* 237 */
    { 38U, 220U, 216U, 228U }, /* 238 */ { 37U, 220U, 218U, 229U }, /* 239 */
    { 30U, 222U, 205U, 230U }, /* 240 */ { 39U, 222U, 205U, 229U }, /* 241 */
    { 29U, 224U, 205U, 230U }, /* 242 */ { 29U, 224U, 205U, 231U }, /* 243 */
    { 39U, 226U, 205U, 233U }, /* 244 */ { 39U, 226U, 205U, 234U }, /* 245 */
    { 39U, 226U, 205U, 236U }, /* 246 */ { 40U, 222U, 205U, 237U }, /* 247 */
    { 34U, 228U, 205U, 235U }, /* 248 */ { 64U, 230U, 205U, 238U }, /* 249 */
    { 39U, 232U, 205U, 237U }, /* 250 */ { 45U, 232U, 205U, 239U }, /* 251 */
    { 44U, 234U, 205U, 242U }, /* 252 */ { 30U, 234U, 205U, 242U }, /* 253 */
    { 40U, 236U, 205U, 243U }, /* 254 */ { 30U, 236U, 205U, 242U }, /* 255 */
    { 41U, 236U, 205U, 245U }, /* 256 */ { 31U, 238U, 205U, 245U }, /* 257 */
    { 31U, 240U, 205U, 246U }, /* 258 */ { 41U, 240U, 205U, 247U }, /* 259 */
    { 40U, 242U, 238U, 247U }, /* 260 */ { 36U, 242U, 240U, 248U }  /* 261 */
};
static size_t histogram[8][37];
static size_t histoswap[8];
#define HISTOGRAM_INDEX1_OFFSET 3
static unsigned int do_histogram=0U;

/* initialize at run-time */
static void initialize_median_test(V)
{
    const char *s;

    /* Initialize strings */
    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    else
        s++;
    median_test_initialized = register_build_strings(build_options, &source_file, s);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void repivot_factors(unsigned int sampling_table_index, size_t *pk, size_t *pf1, size_t *pf2)
{
    if (0UL==sampling_table_index) {
        *pf1 = *pf2 = 2UL;
        return;
    }
    sampling_table_index--; /* repivot table has no entry for single sample */
    if (sampling_table_index>=repivot_table_size)
        sampling_table_index = repivot_table_size - 1UL;
    if (NULL!=pk) { /* selection */
        *pf1 = selection_repivot_table[sampling_table_index].factor1;
        *pf2 = selection_repivot_table[sampling_table_index].factor2;
    } else { /* sorting */
        *pf1 = sorting_repivot_table[sampling_table_index].factor1;
        *pf2 = sorting_repivot_table[sampling_table_index].factor2;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void set_factor1(unsigned int sampling_table_index, size_t *pk, size_t factor1)
{
    if (0UL==sampling_table_index) return;
    sampling_table_index--; /* repivot table has no entry for single sample */
    if (sampling_table_index>=repivot_table_size)
        sampling_table_index = repivot_table_size - 1UL;
    if (NULL!=pk) { /* selection */
        selection_repivot_table[sampling_table_index].factor1 = factor1;
    } else { /* sorting */
        sorting_repivot_table[sampling_table_index].factor1 = factor1;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void set_factor2(unsigned int sampling_table_index, size_t *pk, size_t factor2)
{
    if (0UL==sampling_table_index) return;
    sampling_table_index--; /* repivot table has no entry for single sample */
    if (sampling_table_index>=repivot_table_size)
        sampling_table_index = repivot_table_size - 1UL;
    if (NULL!=pk) { /* selection */
        selection_repivot_table[sampling_table_index].factor2 = factor2;
    } else { /* sorting */
        sorting_repivot_table[sampling_table_index].factor2 = factor2;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int ilongcmp(const void *p1, const void *p2)
{
    int r = longcmp(p1,p2);
    if (0<r) ngt++;
    else if (0>r) nlt++;
    else neq++;
    return r;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int iintcmp(const void *p1, const void *p2)
{
#if 0
fprintf(stderr, "%s line %d: p1=%p(%d), p2=%p(%d)\n",__func__,__LINE__,p1,*((int *)p1),p2,*((int *)p2));
#endif
    int r = intcmp(p1,p2);
    if (0<r) ngt++;
    else if (0>r) nlt++;
    else neq++;
    return r;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int idoublecmp(const void *p1, const void *p2)
{
    int r = doublecmp(p1,p2);
    if (0<r) ngt++;
    else if (0>r) nlt++;
    else neq++;
    return r;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int big_struct_intcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        int c;
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;
        c = intcmp(&(pa->val),&(pb->val));
        return c;
    }
    return 0;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int ibig_struct_intcmp(const void *p1, const void *p2)
{
    int r=big_struct_intcmp(p1,p2);
    if (0<r) ngt++;
    else if (0>r) nlt++;
    else neq++;
    return r;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int big_struct_strcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        int c;
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;

        c = strcmp(pa->string, pb->string);
        return c;
    }
    return 0;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int ibig_struct_strcmp(const void *p1, const void *p2)
{
    int c = big_struct_strcmp(p1, p2);
    if (0 < c) ngt++;
    else if (0 > c) nlt++;
    else neq++;
    return c;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int indcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        int c;
        const struct big_struct **pa = (const struct big_struct **)p1,
            **pb = (const struct big_struct **)p2;

        /* compare as strings (intentionally slow) */
        c = big_struct_strcmp(*pa, *pb);
        return c;
    }
    return 0;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int iindcmp(const void *p1, const void *p2)
{
    int c = indcmp(p1, p2);
    if (0 < c) ngt++;
    else if (0 > c) nlt++;
    else neq++;
    return c;
}

/* Array element swaps: */
/* count is in chars */
/* versions to swap by char, short, int, double */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void charswap(char *pa, char *pb, size_t count)
{
    char t;
    A(pa!=pb);A(0UL<count);
    do {
        t=*pa, *pa=*pb, *pb=t;
        if (0UL==--count) break;
        pa++, pb++;
    } while (1);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void icharswap(char *pa, char *pb, size_t count)
{
    char t;

    A(pa!=pb);A(0UL<count);
    nsw+=count;
    do {
        t=*pa, *pa=*pb, *pb=t;
        if (0UL==--count) break;
        pa++, pb++;
    } while (1);
}

#if 0
        do {                                \
            t=*px, *px=*py, *py=t;          \
            if (0UL==--count) break;        \
            px++, py++;                     \
        } while (1);
    for ( ; 0UL<count; count--) t=*px, *px++=*py, *py++=t;
    while (0UL<count) t=*px, *px++=*py, *py++=t, count--;
    do { t=*px, *px++=*py, *py++=t, count--; } while (0UL<count);
#endif

#undef GENERIC_SWAP
#undef COUNTING_SWAP
# define GENERIC_SWAP(type)                 \
    type *px=(type *)pa, *py=(type *)pb, t; \
    A(pa!=pb);A(0UL<count);                 \
    count /= sizeof(type);                  \
    do { t=*px, *px++=*py, *py++=t, count--; } while (0UL<count);
# define COUNTING_SWAP(type)                \
    type *px=(type *)pa, *py=(type *)pb, t; \
    A(pa!=pb);A(0UL<count);                 \
    count /= sizeof(type);                  \
    nsw+=count;                             \
    do { t=*px, *px++=*py, *py++=t, count--; } while (0UL<count);

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shortswap(char *pa, char *pb, size_t count)
{
    GENERIC_SWAP(short)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void ishortswap(char *pa, char *pb, size_t count)
{
    COUNTING_SWAP(short)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void intswap(char *pa, char *pb, size_t count)
{
    GENERIC_SWAP(int)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void iintswap(char *pa, char *pb, size_t count)
{
    COUNTING_SWAP(int)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void doubleswap(char *pa, char *pb, size_t count)
{
    GENERIC_SWAP(double)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void idoubleswap(char *pa, char *pb, size_t count)
{
    COUNTING_SWAP(double)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void iswap2(char *pa, char *pb, size_t count)
{
    nsw+=count;
    swap2(pa,pb,count);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void swapper(char *pa, char *pb, size_t count, int typeindex)
{
    A(pa!=pb);A(0UL<count);
    switch (typeindex) {
        case 0 : /* double */
            {   register double *px=(double *)pa, *py=(double *)pb, t;
                for ( ; 0UL<count; count--) t=*px, *px++=*py, *py++=t;
            }
        return;
        case 3 : /* int */
            {   register int *px=(int *)pa, *py=(int *)pb, t;
                do { t=*px, *px++=*py, *py++=t, count--; } while (0UL<count);
            }
        return;
        case 4 : /* short */
            {   register short *px=(short *)pa, *py=(short *)pb, t;
                do { t=*px, *px++=*py, *py++=t, count--; } while (0UL<count);
            }
        return;
        case 5 : /* char */
            {   register char t;
                do { t=*pa, *pa++=*pb, *pb++=t, count--; } while (0UL<count);
            }
        return;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void iswap(char *pa, char *pb, size_t bytes, int typeindex)
{
    bytes /= typsz[typeindex]; /* number of swaps of basic type */
    swapper(pa,pb,bytes,typeindex);
    nsw+=bytes;
}

/* Determine and return an integer which is an index into an array of types
   suitable for swapping functions.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int type_index(char *base, size_t size)
{
    int i, t;  /* general integer variables */
    size_t s;  /* general size_t variable */

    /* Determine size of data chunks to copy for element swapping.  Size is
       determined by element size and alignment. typsz is an
       array of type sizes (double, pointer, long, int, short, char).
       Types char, short, int, double have sizes 1, 2, 4, 8 on most (all?)
       32-bit and 64-bit architectures.  Types long and pointer sizes vary
       by architecture.
    */
    s=typsz[t=0]; /* double */
    if ((size<s)||(!(is_aligned(size,i=log2s[s])))||(!(is_aligned(base,i)))) {
        s=typsz[t=3]; /* int */
        if ((size<s)||(!(is_aligned(size,i=log2s[s])))
        ||(!(is_aligned(base,i)))) {
            s=typsz[t=4]; /* short */
            if ((size<s)||(!(is_aligned(size,i=log2s[s])))
            ||(!(is_aligned(base,i))))
                s=typsz[t=5]; /* char */
        }
    }
    return t;
}

/* determine and return a pointer to an appropriate swap function */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void (*whichswap(char *base, size_t size))
    (char *, char *, size_t)
{
    switch (type_index(base,size)) {
        case 0 :
            if (0U!=instrumented) return idoubleswap;
        return doubleswap;
        case 3 :
            if (0U!=instrumented) return iintswap;
        return intswap;
        case 4 :
            if (0U!=instrumented) return ishortswap;
        return shortswap;
        default :
            if (0U!=instrumented) return icharswap;
        return charswap;
    }
}

/* Print elements of integer array with indices l through u inclusive. */
static void print_int_array(int *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%d", i, target[i]);
    }
    (V)fprintf(stderr, "\n");
}

static void print_long_array(long *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%ld", i, target[i]);
    }
    (V)fprintf(stderr, "\n");
}

static void print_big_array_keys(struct big_struct *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%d", i, target[i].val);
    }
    (V)fprintf(stderr, "\n");
}

static void print_indirect_keys(struct big_struct **target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%d", i, target[i]->val);
    }
    (V)fprintf(stderr, "\n");
}

static void print_double_array(double *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%.12G", i, target[i]);
    }
    (V)fprintf(stderr, "\n");
}

#if 1
static void print_size_t_array(size_t *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%lu", i, target[i]);
    }
    (V)fprintf(stderr, "\n");
}
#endif

#if 0
static void write_int_array(char *buf, size_t sz, int *target, size_t l, size_t u)
{
    size_t i;
    int n;

    for (i=l; i<=u; i++) {
        if (i != l) {
            n=snprintf(buf,sz, ", ");
            buf+=n;
            if (n+2>=sz) break;
            sz-=n;
        }
        n=snprintf(buf,sz, "%lu:%d", i, target[i]);
        buf+=n;
        if (n+2>=sz) break;
        sz-=n;
    }
    (V)snprintf(buf,sz, "\n");
}
#endif

#if ASSERT_CODE /* XXX */
static void fprint_int_array(FILE *f, int *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%d", i, target[i]);
    }
    (V)fprintf(f, "\n");
}

static void fprint_double_array(FILE *f, double *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%.9G", i, target[i]);
    }
    (V)fprintf(f, "\n");
}

#if 0
static void fprint_long_array(FILE *f, long *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%ld", i, target[i]);
    }
    (V)fprintf(f, "\n");
}
#endif

#if ASSERT_CODE
static FILE *logfile = NULL;

static FILE *start_log(const char *path) {
    return fopen(path,"w+");
}

static void close_log(FILE *f) {
    if (NULL!=f) {
        fflush(f);
        fclose(f);
    }
}
#endif
#endif

/* optimized for & less expensive than % (esp. for unsigned long) */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int is_even(size_t n)
{
    if (0UL == (n & 1UL)) return 1;
    return 0;
}

#if DEBUG_CODE + ASSERT_CODE
#if GENERATOR_TEST
static size_t test_array_distinctness(const void *pv, size_t l, size_t u, size_t size, int(*comparison_function)(const void *, const void *), void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, j;

    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+1UL;
    }
    for (i=l; i<u; i++) {
        for (j=i+1UL; j<=u; j++) {
            c = comparison_function((const char *)pv+i*size, (const char *)pv+j*size);
            if (0 == c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed distinctness comparison at indices %lu, %lu", __func__, source_file, __LINE__, i, j);
                return i;
            }
        }
    }
    return u;
}
#endif /* GENERATOR_TEST */
#endif /* DEBUG_CODE + ASSERT_CODE */

/* Array pv should be partitioned such that elements at indices < pl are
      not larger than element at pl, elements at indices > pu are not smaller
      than element at index pu, and elements from index pl through pu compare
      as equal.
   l <= pl <= pu <= u
   If tests hold for all indices between l and u inclusive, return pl.
   If arguments are not valid, return >= u+1UL.
   On the first failed test, return the index of the item failing the test.
*/
static size_t test_array_partition(const void *pv, size_t l, size_t pl, size_t pu, size_t u, size_t size, int(*comparison_function)(const void *, const void *), void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, n, lim;

    if (l > pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > pl (%lu)", __func__, source_file, __LINE__, l, pl);
        return u+1UL;
    }
    if (pu > u) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) > u (%lu)", __func__, source_file, __LINE__, pu, u);
        return u+2UL;
    }
    if (pu < pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) < pl (%lu)", __func__, source_file, __LINE__, pu, pl);
        return u+3UL;
    }
    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+4UL;
    }
    if (pl >= l+1UL)
        for (i=pl-1UL; 1; i--) {
            c = comparison_function((const char *)pv+i*size, (const char *)pv+pl*size);
            if (0 < c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, i, pl);
                return i;
            }
            if (i < l+1UL) break;
        }
    for (i=pl+1UL; i<=pu; i++) {
        c = comparison_function((const char *)pv+i*size, (const char *)pv+pl*size);
        if (0 != c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, pl, i);
            return i;
        }
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    for (i=pu+1UL; i<=lim; i++) {
        c = comparison_function((const char *)pv+i*size, (const char *)pv+pu*size);
        if (0 > c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, i, pu);
            return i;
        }
    }
    return pl;
}

/* verify median:
      no more than n/2 elements < median
      no more than n/2 elements > median
      every element is <, ==, or > median
*/
static size_t test_array_median(const void *pv, size_t l, size_t m, size_t u, size_t size, int(*comparison_function)(const void *, const void *), void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, eq, gt, lt, lim, n;

    if (l > m) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > m (%lu)", __func__, source_file, __LINE__, l, m);
        return u+1UL;
    }
    if (m > u) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: m (%lu) > u (%lu)", __func__, source_file, __LINE__, m, u);
        return u+2UL;
    }
    if (NULL == pv) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+3UL;
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    gt=lt=0UL,eq=1UL;
    if (m >= l+1UL)
        for (i=m-1UL; 1; i--) {
            c = comparison_function((const char *)pv+i*size, (const char *)pv+m*size);
            if (0 < c) gt++; else if (0 > c) lt++; else eq++;
            if (i < l+1UL) break;
        }
    for (i=m+1UL; i<=lim; i++) {
        c = comparison_function((const char *)pv+i*size, (const char *)pv+m*size);
        if (0 < c) gt++; else if (0 > c) lt++; else eq++;
    }
    if (eq + gt + lt != n) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: eq %lu + gt %lu + lt %lu != n %lu", __func__, source_file, __LINE__, eq, gt, lt, n);
        return u+4UL;
    }
    if (gt > (n >> 1)) { /* n >> 1 is a fast version of n / 2 */
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: gt %lu > n %lu / 2", __func__, source_file, __LINE__, gt, n);
        return u+5UL;
    }
    if (lt > (n >> 1)) { /* n >> 1 is a fast version of n / 2 */
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: lt %lu > n %lu / 2", __func__, source_file, __LINE__, lt, n);
        return u+6UL;
    }
    return m;
}

/* Test array for sorting correctness */
static size_t test_array_sort(const void *pv, size_t l, size_t u, size_t size, int(*comparison_function)(const void *, const void *), unsigned int distinct, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i;

    if (l > u) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > u (%lu)", __func__, source_file, __LINE__, l, u);
        return u+1UL;
    }
    if (NULL == pv) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+2UL;
    }
    for (i=l; i<u; i++) {
        c = comparison_function((const char *)pv+i*size, (const char *)pv+(i+1UL)*size);
        if (0 <= c) {
            if ((0U==distinct)&&(0==c)) continue; /* uniqueness not required */
            if (NULL != f)
                f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at index %lu", __func__, source_file, __LINE__, i);
            return i;
        }
    }
    return u;
}

static
size_t factorial(size_t n)
{
    switch (n) {
        case 0UL :
        return 1UL;
        case 1UL : /*FALLTHROUGH*/
        case 2UL :
        return n;
    }
    return n * factorial(n-1UL);
}

/* ***** code copied from M.D.McIlroy "A Killer Adversary for Quicksort" ******* */
/* Copyright 1998, M. Douglas McIlroy
   Permission is granted to use or copy with this notice attached.
 
   Aqsort is an antiquicksort.  It will drive any qsort implementation
   based on quicksort into quadratic behavior, provided the imlementation
   has these properties:

   1.  The implementation is single-threaded.

   2.  The pivot-choosing phase uses O(1) comparisons.

   3.  Partitioning is a contiguous phase of n-O(1) comparisons, all
   against the same pivot value.

   4.  No comparisons are made between items not found in the array.
   Comparisons may, however, involve copies of those items.

   Method

   Values being sorted are dichotomized into "solid" values that are
   known and fixed, and "gas" values that are unknown but distinct and
   larger than solid values.  Initially all values are gas.  Comparisons
   work as follows:

   Solid-solid.  Compare by value.  Solid-gas.  Solid compares low.
   Gas-gas.  Solidify one of the operands, with a value greater
      than any previously solidified value.  Compare afresh.

   During partitioning, the gas values that remain after pivot-choosing
   will compare high, provided the pivot is solid.  Then qsort will go
   quadratic.  To force the pivot to be solid, a heuristic test
   identifies pivot candidates to be solidified in gas-gas comparisons.
   A pivot candidate is the gas item that most recently survived
   a comparison.  This heuristic assures that the pivot gets
   solidified at or before the second gas-gas comparison during the
   partitioning phase, so that n-O(1) gas values remain.

   To allow for copying, we must be able to identify an operand even if
   it was copied from an item that has since been solidified.  Hence we
   keep the data in fixed locations and sort pointers to them.  Then
   qsort can move or copy the pointers at will without disturbing the
   underlying data.

   int aqsort(int n, int *a); returns the count of comparisons qsort used
   in sorting an array of n items and fills in array a with the
   permutation of 0..n-1 that achieved the count.
*/
/* variable names have namespace prefix added */
/* type of ncmp modified to size_t */
static size_t antiqsort_ncmp; /* number of comparisons */
static size_t antiqsort_nsolid; /* number of solid items */
static size_t pivot_candidate; /* pivot candidate */
static size_t antiqsort_gas; /* gas value */
static size_t antiqsort_n;
/* qsort uses one array, antiqsort another */
static long *antiqsort_base;
static char *qsort_base;
static size_t qsort_typsz;
static size_t qsort_type;

/* freeze implemented as a function */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
long freeze(long z)
{
    long l;

#if DEBUG_CODE
if (3<debug) (V)fprintf(stderr,"// %s: z=%ld\n",__func__,z);
#endif
    antiqsort_base[z] = l = antiqsort_nsolid++;
    return l;
}

/* comparison function called by qsort; values corresponding to pointers are
   used as indices into the antiqsort array
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int aqcmp(const void *px, const void *py) /* per C standard */
{
    long a, b, x, y;
    const struct big_struct *pbs;

    antiqsort_ncmp++;
    switch (qsort_type) {
        case DATA_TYPE_LONG :
            x = *((const long *)px);
            y = *((const long *)py);
        break;
        case DATA_TYPE_INT :
            x = (long)(*((const int *)px));
            y = (long)(*((const int *)py));
#if DEBUG_CODE
if (AQCMP_DEBUG_LEVEL<debug) (V)fprintf(stderr,"// %s: x=%ld, y=%ld\n",__func__,x,y);
#endif
        break;
        case DATA_TYPE_DOUBLE :
            x = (long)(*((const double *)px));
            y = (long)(*((const double *)py));
#if DEBUG_CODE
if (AQCMP_DEBUG_LEVEL<debug) (V)fprintf(stderr,"// %s: *px=%G, x=%ld, *py=%G, y=%ld\n",__func__,*((const double *)px),x,*((const double *)py),y);
#endif
        break;
        case DATA_TYPE_POINTER :
            pbs = *((const struct big_struct **)px);
            x = pbs->l;
            pbs = *((const struct big_struct **)py);
            y = pbs->l;
        break;
        case DATA_TYPE_STRUCT : /*FALLTHROUGH*/
        case DATA_TYPE_STRING :
            pbs = (const struct big_struct *)px;
            x = pbs->l;
            pbs = (const struct big_struct *)py;
            y = pbs->l;
        break;
    }
if ((0L>x)||(x>=antiqsort_n)) (V)fprintf(stderr,"// %s line %d: x (%ld) is out of range 0<=x<%ld\n",__func__,__LINE__,x,antiqsort_n);
if ((0L>y)||(y>=antiqsort_n)) (V)fprintf(stderr,"// %s line %d: y (%ld) is out of range 0<=y<%ld\n",__func__,__LINE__,y,antiqsort_n);
    A(0L<=x);A(x<antiqsort_n);A(0L<=y);A(y<antiqsort_n);
    a=antiqsort_base[x];
    b=antiqsort_base[y];
#if DEBUG_CODE
if (AQCMP_DEBUG_LEVEL<debug) (V)fprintf(stderr,"// %s line %d: a=%ld, b=%ld\n",__func__,__LINE__,a,b);
#endif
    /* brackets added BL */
    if ((a==antiqsort_gas) && (b==antiqsort_gas)) {
        if (x == pivot_candidate) {
            a=freeze(x);
            if (x==y) b=a;
        } else {
            b=freeze(y);
            if (x==y) a=b;
        }
    }
    if (a==antiqsort_gas) {
        pivot_candidate = x;
    } else if (b==antiqsort_gas) {
        pivot_candidate = y;
    }
#if 0 /* original code */
int
cmp(const void *px, const void *py)  /* per C standard */
{
	const int x = *(const int*)px;
	const int y = *(const int*)py;
	ncmp++;
	if(val[x]==gas && val[y]==gas)
		if(x == candidate)
			freeze(x);
		else
			freeze(y);
	if(val[x] == gas)
		candidate = x;
	else if(val[y] == gas)
		candidate = y;
	return val[x] - val[y];
}
#endif
#if 0
    if (x==y) return 0; /* spurious self-comparison */
#endif
    A((x!=y)||(a==b));A((x==y)||(a!=b));
    if (a > b) return 1; else if (a < b) return -1; else return 0;
}
/* ********** modified into a separate function for intitalization ************ */
/* added args: pv points to array of type type elements each having size sz used by qsort, alt to array used by antiqsort */
static void initialize_antiqsort(size_t n, char *pv, int type, const size_t sz, long *alt)
{
    size_t i;

    A(NULL!=pv);A(NULL!=alt);A(pv!=alt);
    qsort_base = pv;
    qsort_type = type;
    qsort_typsz = sz;
    antiqsort_n = n;
    antiqsort_base = alt;
    antiqsort_gas = n - 1UL;
    antiqsort_nsolid = 0UL;
    pivot_candidate = 0UL;
    antiqsort_ncmp = 0UL;
#if DEBUG_CODE
if (AQCMP_DEBUG_LEVEL<debug) (V)fprintf(stderr,"// %s: n=%lu, pv=%p, type=%d, sz=%lu, alt=%p\n",__func__,n,pv,type,sz,alt);
#endif
    for(i=0UL; i<n; i++) {
        /* initialize the array that qsort uses */
        switch (qsort_type) {
            case DATA_TYPE_LONG :
                {
                    long *plong;
                    plong = (long *)(qsort_base + qsort_typsz*i);
                    *plong = (long)i;
                }
            break;
            case DATA_TYPE_INT :
                {
                    int *pint;
                    pint = (int *)(qsort_base + qsort_typsz*i);
                    *pint = (int)i;
                }
            break;
            case DATA_TYPE_DOUBLE :
                {
                    double *pd;
                    pd = (double *)(qsort_base + qsort_typsz*i);
                    *pd = (double)i;
                }
            break;
            case DATA_TYPE_POINTER :
                {
                    struct big_struct *pbs;
                    pbs = *((struct big_struct **)(qsort_base + qsort_typsz*i));
                    pbs->l = (long)i;
                    pbs->val = (int)i;
                    (V)snul(pbs->string, BIG_STRUCT_STRING_SIZE, NULL, NULL, i, 36, '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
                    pbs->d = (double)i;
                }
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                {
                    struct big_struct *pbs;
                    pbs = (struct big_struct *)(qsort_base + qsort_typsz*i);
                    pbs->l = (long)i;
                    pbs->val = (int)i;
                    (V)snul(pbs->string, BIG_STRUCT_STRING_SIZE, NULL, NULL, i, 36, '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
                    pbs->d = (double)i;
                }
            break;
        }
        /* initialize the array that antiqsort uses */
        antiqsort_base[i] = (long)antiqsort_gas;
    }
}
/* ***************************************************************************** */

/* ******************** Bentley & McIlroy qsort ******************************** */
/* namespace changed */
/* qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

    /*assume sizeof(long) is a power of 2 */
#define SWAPINIT(a, es) swaptype =         \
    (a-(char*)0 | es) % sizeof(long) ? 2 : es > sizeof(long);
#define swapcode(TYPE, parmi, parmj, n) {  \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while ((n -= sizeof(TYPE)) > 0);     \
}
#define iswapcode(TYPE, parmi, parmj, n) {  \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    nsw += n/sizeof(TYPE);                 \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while ((n -= sizeof(TYPE)) > 0);     \
}
static void swapfunc(char *a, char *b, size_t n, int swaptype)
{   if (swaptype <= 1) swapcode(long, a, b, n)
    else swapcode(char, a, b, n)
}
static void iswapfunc(char *a, char *b, size_t n, int swaptype)
{   if (swaptype <= 1) iswapcode(long, a, b, n)
    else iswapcode(char, a, b, n)
}
#define bmswap(a, b)                       \
    if (swaptype == 0) {                   \
        long t = *(long*)(a);              \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
    } else                                 \
        swapfunc(a, b, es, swaptype)
#define ibmswap(a, b)                       \
    if (swaptype == 0) {                   \
        long t = *(long*)(a);              \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
        nsw++;                             \
    } else                                 \
        iswapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)
#define ivecswap(a, b, n) if (n > 0) iswapfunc(a, b, n, swaptype)

#define PVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; bmswap(pv, pm); }    \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }
#define IPVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; ibmswap(pv, pm); }    \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }

#define bmmin(x, y) ((x)<=(y) ? (x) : (y))

static char *med3(char *a, char *b, char *c, int (*compar)(const void *, const void *))
{
        return compar(a, b) < 0 ?
                  (compar(b, c) < 0 ? b : compar(a, c) < 0 ? c : a)
                : (compar(b, c) > 0 ? b : compar(a, c) > 0 ? c : a);
}

void bmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s;

        SWAPINIT(a, es);
        if (n < (BM_INSERTION_CUTOFF)) {         /* Insertion sort on smallest arrays */
                for (pm = a + es; pm < a + n*es; pm += es)
                        for (pl = pm; pl > a && compar(pl-es, pl) > 0; pl -= es)
                                bmswap(pl, pl-es);
                return;
        }
        pm = a + (n/2)*es;    /* Small arrays, middle element */
        if (n > 7) {
                pl = a;
		pn = a + (n-1)*es; /* moved outside of pivot selection guard */
                if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                        s = (n/8)*es;
                        pl = med3(pl, pl+s, pl+2*s, compar);
                        pm = med3(pm-s, pm, pm+s, compar);
                        pn = med3(pn-2*s, pn-s, pn, compar);
                }
                pm = med3(pl, pm, pn, compar); /* Mid-size, med of 3 */
        }
        PVINIT(pv, pm);       /* pv points to partition value */
        pa = pb = a; /* original code; redundant comparison pa vs. pa */
        pc = pd = a + (n-1)*es; /* original code; could have been set along with pn before pivot selection test */
        for (;;) {
                while (pb <= pc && (r = compar(pb, pv)) <= 0) {
                        if (r == 0) { bmswap(pa, pb); pa += es; }
                        pb += es;
                }
                while (pb <= pc && (r = compar(pc, pv)) >= 0) {
                        if (r == 0) { bmswap(pc, pd); pd -= es; }
                        pc -= es;
                }
                if (pb > pc) break;
                bmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        pn = a + n*es;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower == region */
        /* pa-a (a.k.a. pa-pl) is the number of chars in the lower == block */
        /* pb-pa is the number of chars in the < block */
        s = bmmin(pa-a,  pb-pa   ); vecswap(a,  pb-s, s);
        /* after the swap (above), the start of the == section is at pb-s */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper == region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper == block */
        /* pd-pc is the number of chars in the > block */
        s = bmmin(pd-pc, pn-pd-es); vecswap(pb, pn-s, s);

        if ((s = pb-pa) > es) bmqsort(a,    s/es, es, compar);
        if ((s = pd-pc) > es) bmqsort(pn-s, s/es, es, compar);
}

/* instrumented version */
void ibmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s;

        SWAPINIT(a, es);
        if (n < (BM_INSERTION_CUTOFF)) {         /* Insertion sort on smallest arrays */
                for (pm = a + es; pm < a + n*es; pm += es)
                        for (pl = pm; pl > a && compar(pl-es, pl) > 0; pl -= es)
                                ibmswap(pl, pl-es);
                return;
        }
        pm = a + (n/2)*es;    /* Small arrays, middle element */
        if (n > 7) {
                pl = a;
		pn = a + (n-1)*es; /* moved outside of pivot selection guard */
                if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                        s = (n/8)*es;
                        pl = med3(pl, pl+s, pl+2*s, compar);
                        pm = med3(pm-s, pm, pm+s, compar);
                        pn = med3(pn-2*s, pn-s, pn, compar);
                }
                pm = med3(pl, pm, pn, compar); /* Mid-size, med of 3 */
        }
        IPVINIT(pv, pm);       /* pv points to partition value */
        pa = pb = a; /* original code; redundant comparison pa vs. pa */
        pc = pd = a + (n-1)*es; /* original code; could have been set along with pn before pivot selection test */
        for (;;) {
                while (pb <= pc && (r = compar(pb, pv)) <= 0) {
                        if (r == 0) { ibmswap(pa, pb); pa += es; }
                        pb += es;
                }
                while (pb <= pc && (r = compar(pc, pv)) >= 0) {
                        if (r == 0) { ibmswap(pc, pd); pd -= es; }
                        pc -= es;
                }
                if (pb > pc) break;
                ibmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        pn = a + n*es;
        npartitions++;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower == region */
        /* pa-a (a.k.a. pa-pl) is the number of chars in the lower == block */
        /* pb-pa is the number of chars in the < block */
        s = bmmin(pa-a,  pb-pa   ); ivecswap(a,  pb-s, s);
        /* after the swap (above), the start of the == section is at pb-s */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper == region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper == block */
        /* pd-pc is the number of chars in the > block */
        s = bmmin(pd-pc, pn-pd-es); ivecswap(pb, pn-s, s);

        if ((s = pb-pa) > es) ibmqsort(a,    s/es, es, compar);
        if ((s = pd-pc) > es) ibmqsort(pn-s, s/es, es, compar);
}
/* ************************************************************************** */

/* NetBSD qsort code (derived from Bentley&McIlroy code) */
/* _DIAGASSERT -> assert */
void nbqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
        size_t d, r;
        int swaptype, cmp_result;

        assert(a != NULL || n == 0 || es == 0);
        assert(compar != NULL);

nbloop:        SWAPINIT((char *)a, es);
        if (n < 7) {
                for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
                        for (pl = pm; pl > (char *) a && compar(pl - es, pl) > 0;
                             pl -= es)
                                ibmswap(pl, pl - es);
                return;
        }
        pm = (char *) a + (n / 2) * es;
        if (n > 7) {
                pl = (char *)a;
                pn = (char *) a + (n - 1) * es;
                if (n > 40) {
                        d = (n / 8) * es;
                        pl = med3(pl, pl + d, pl + 2 * d, compar);
                        pm = med3(pm - d, pm, pm + d, compar);
                        pn = med3(pn - 2 * d, pn - d, pn, compar);
                }
                pm = med3(pl, pm, pn, compar);
        }
        ibmswap(a, pm);
        pa = pb = (char *) a + es;

        pc = pd = (char *) a + (n - 1) * es;
        for (;;) {
                while (pb <= pc && (cmp_result = compar(pb, a)) <= 0) {
                        if (cmp_result == 0) {
                                ibmswap(pa, pb);
                                pa += es;
                        }
                        pb += es;
                }
                while (pb <= pc && (cmp_result = compar(pc, a)) >= 0) {
                        if (cmp_result == 0) {
                                ibmswap(pc, pd);
                                pd -= es;
                        }
                        pc -= es;
                }
                if (pb > pc)
                        break;
                ibmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        npartitions++;

        pn = (char *) a + n * es;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower == region */
        /* pb-pa is the number of chars in the < block */
        r = bmmin(pa - (char *) a, pb - pa);
        ivecswap(a, pb - r, r);
        /* after the swap (above), the start of the == section is at pb-r */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper == region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper == block */
        /* pd-pc is the number of chars in the > block */
        r = bmmin((size_t)(pd - pc), pn - pd - es);
        ivecswap(pb, pn - r, r);

        if ((r = pb - pa) > es)
                nbqsort(a, r / es, es, compar);
        if ((r = pd - pc) > es) { 
                /* Iterate rather than recurse to save stack space */
                a = pn - r;
                n = r / es;
                goto nbloop;
        }
/*                nbqsort(pn - r, r / es, es, compar);*/
}
/* ************************************************************************** */

/* ********************** glibc qsort code ********************************** */
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Written by Douglas C. Schmidt (schmidt@ics.uci.edu).
 
The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
 
The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, see
<http://www.gnu.org/licenses/>. */

/* If you consider tuning this algorithm, you should consult first:
Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993. */


/* Byte-wise swap two items of size SIZE. */
#define SWAP(a, b, size) \
do { \
    size_t __size = (size); \
    char *__a = (a), *__b = (b); \
    nsw+=__size; \
    do { \
        char __tmp = *__a; \
        *__a++ = *__b; \
        *__b++ = __tmp; \
    } while (--__size > 0); \
} while (0)

/* Discontinue quicksort algorithm when partition gets below this size.
This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct {
  char *lo;
  char *hi;
} stack_node;

/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
log(MAX_THRESH)). Since total_elements has type size_t, we get as
upper bound for log (total_elements):
bits per byte (CHAR_BIT) * sizeof(size_t). */
#define STACK_SIZE (CHAR_BIT * sizeof(size_t))
#define PUSH(low, high) ((V) ((top->lo = (low)), (top->hi = (high)), ++top))
#define POP(low, high) ((V) (--top, (low = top->lo), (high = top->hi)))
#define STACK_NOT_EMPTY (stack < top)


/* Order size using quicksort. This implementation incorporates
four optimizations discussed in Sedgewick:
 
1. Non-recursive, using an explicit stack of pointer that store the
next array partition to sort. To save time, this maximum amount
of space required to store an array of SIZE_MAX is allocated on the
stack. Assuming a 32-bit (64 bit) integer for size_t, this needs
only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
Pretty cheap, actually.
 
2. Chose the pivot element using a median-of-three decision tree.
This reduces the probability of selecting a bad pivot value and
eliminates certain extraneous comparisons.
 
3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
insertion sort to order the MAX_THRESH items within each partition.
This is a big win, since insertion sort is faster for small, mostly
sorted array segments.
 
4. The larger of the two sub-partitions is always pushed onto the
stack first, with the algorithm then concentrating on the
smaller partition. This *guarantees* no more than log (total_elems)
stack size is needed (actually O(1) in this case)! */

void
glibc_quicksort (void *const pbase, size_t total_elems, size_t size,
            int(*compar)(const void *, const void *))
{
  char *base_ptr = (char *) pbase;

  const size_t max_thresh = MAX_THRESH * size;

  if (total_elems == 0)
/* Avoid lossage with unsigned arithmetic below. */
    return;

  if (total_elems > MAX_THRESH)
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;

      PUSH (NULL, NULL);

      while (STACK_NOT_EMPTY)
        {
          char *left_ptr;
          char *right_ptr;

/* Select median value from among LO, MID, and HI. Rearrange
LO and HI so the three values are sorted. This lowers the
probability of picking a pathological pivot value and
skips a comparison for both the LEFT_PTR and RIGHT_PTR in
the while loops. */

          char *mid = lo + size * ((hi - lo) / size >> 1);

          if ((compar) ((void *) mid, (void *) lo) < 0)
            SWAP (mid, lo, size);
          if ((compar) ((void *) hi, (void *) mid) < 0)
            SWAP (mid, hi, size);
          else
            goto jump_over;
          if ((compar) ((void *) mid, (void *) lo) < 0)
            SWAP (mid, lo, size);
        jump_over:;

          left_ptr = lo + size;
          right_ptr = hi - size;

/* Here's the famous ``collapse the walls'' section of quicksort.
Gotta like those tight inner loops! They are the main reason
that this algorithm runs much faster than others. */
          /* glibc partition
             initially:
             +---------------------------+
             | |     ?    |m|    ?     | |
             +---------------------------+
                l          m          r
             partial:
             +---------------------------+
             |   < | ?    |m|    ? | >   |
             +---------------------------+
                    l      m      r    
             final:
             +---------------------------+
             |   <      |  =  |      >   |
             +---------------------------+
                       r   m   l       
          */
          do
            {
              while ((compar) ((void *) left_ptr, (void *) mid) < 0)
                left_ptr += size;

              while ((compar) ((void *) mid, (void *) right_ptr) < 0)
                right_ptr -= size;

              if (left_ptr < right_ptr)
                {
                  SWAP (left_ptr, right_ptr, size);
                  if (mid == left_ptr)
                    mid = right_ptr;
                  else if (mid == right_ptr)
                    mid = left_ptr;
                  left_ptr += size;
                  right_ptr -= size;
                }
              else if (left_ptr == right_ptr)
                {
                  left_ptr += size;
                  right_ptr -= size;
                  break;
                }
            }
          while (left_ptr <= right_ptr);

/* Set up pointers for next iteration. First determine whether
left and right partitions are below the threshold size. If so,
ignore one or both. Otherwise, push the larger partition's
bounds on the stack and continue sorting the smaller one. */

          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
/* Ignore both small partitions. */
                POP (lo, hi);
              else
/* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
/* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
/* Push larger left partition indices. */
              PUSH (lo, right_ptr);
              lo = left_ptr;
            }
          else
            {
/* Push larger right partition indices. */
              PUSH (left_ptr, hi);
              hi = right_ptr;
            }
        }
    }

/* Once the BASE_PTR array is partially sorted by quicksort the rest
is completely sorted using insertion sort, since this is efficient
for partitions below MAX_THRESH size. BASE_PTR points to the beginning
of the array to sort, and END_PTR points at the very last element in
the array (*not* one beyond it!). */

#define glmin(x, y) ((x) < (y) ? (x) : (y))

  {
    char *const end_ptr = &base_ptr[size * (total_elems - 1)];
    char *tmp_ptr = base_ptr;
    char *thresh = glmin (end_ptr, base_ptr + max_thresh);
    char *run_ptr;

/* Find smallest element in first threshold and place it at the
array's beginning. This is the smallest array element,
and the operation speeds up insertion sort's inner loop. */

    for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
      if ((compar) ((void *) run_ptr, (void *) tmp_ptr) < 0)
        tmp_ptr = run_ptr;

    if (tmp_ptr != base_ptr)
      SWAP (tmp_ptr, base_ptr, size);

/* Insertion sort, running from left-hand-side up to right-hand-side. */

    run_ptr = base_ptr + size;
    while ((run_ptr += size) <= end_ptr)
      {
        tmp_ptr = run_ptr - size;
        while ((compar) ((void *) run_ptr, (void *) tmp_ptr) < 0)
          tmp_ptr -= size;

        tmp_ptr += size;
        if (tmp_ptr != run_ptr)
          {
            char *trav;

            trav = run_ptr + size;
            while (--trav >= run_ptr)
              {
                char c = *trav;
                char *hi, *lo;

                for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                  *hi = *lo;
                *hi = c;
              }
          }
      }
  }
}
/* ************************************************************************** */

 /* insertion sort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void isort_internal(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    char *pa, *pb, *pc, *pl, *pu;
    size_t nmemb=beyond-first;

#if 0
if (3<debug) {
fprintf(stderr, "%s line %d: base=%p, first=%lu, beyond=%lu\n",__func__,__LINE__,base,first,beyond);
print_int_array(base,first,beyond-1UL);
}
#endif
    if (2UL>nmemb) return;
    for (pa=pl=base+first*size,pu=base+(beyond-1UL)*size; pa<pu; pa+=size)
        for (pb=pa; (pb>=pl)&&(0<compar(pb,pc=pb+size)); pb-=size)
#if SWAP_FUNCTION
            swapf(pb,pc,size);
#else
            iswap(pb,pc,size,typeindex);
#endif
#if 0
if (3<debug) {
fprintf(stderr, "%s line %d:\n",__func__,__LINE__);
print_int_array(base,first,beyond-1UL);
}
#endif
#if ASSERT_CODE > 1
    for (pa=pl; pa<pu; pa+=size)
        A(0>=compar(pa,pa+size));
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void isort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
#if SWAP_FUNCTION
    isort_internal(base,0UL,nmemb,size,compar,whichswap(base,size));
#else
    isort_internal(base,0UL,nmemb,size,compar,type_index(base,size));
#endif
}

 /* selection sort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void selsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t nmemb=beyond-first;

    if (2UL<=nmemb) {
        char *pa, *pb, *pc, *pl, *pr;

        for (pa=pl=base+first*size,pr=base+(beyond-1UL)*size; pa<pr; pa+=size) {
            for (pb=pa,pc=pa+size; pc<=pr; pc+=size) {
                if (0<compar(pb,pc)) pb=pc;
            }
#if SWAP_FUNCTION
            if (pb!=pa) swapf(pa,pb,size);
#else
            if (pb!=pa) iswap(pa,pb,size,typeindex);
#endif
        }
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void selsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
#if SWAP_FUNCTION
    selsort_internal(base,0UL,nmemb,size,compar,whichswap(base,size));
#else
    selsort_internal(base,0UL,nmemb,size,compar,type_index(base,size));
#endif
}

#if 0
static size_t ciura[] = { 1, 4, 10, 23, 57, 132, 301, 701, (SIZE_T_MAX) };
static size_t gaps[] = { 1, 9, 67, 479, 3359, 23531, 164729, 1153109, 8071783, (SIZE_T_MAX) };
static size_t ktokuda[] = { 1, 4, 9, 20, 46, 103, 233, 525, 1182, 2660, 5985, 13467, 30301, 68178, 153401, 345152, 776591, 1747331, 3931496, 8845866, 19903198, (SIZE_T_MAX) };
static size_t kprimes[] = { 1, 2, 5, 11, 23, 47, 97, 197, 397, 797, 1597, 3203, 6421, 12853, 25717, 51437, 102877, 205759, 411527, 823117, 1646237, 3292489, 6584983, 13169977, (SIZE_T_MAX) };
/* khibbard: 63 non-prime 1023 non-prime 4095 non-prime 16383 non-prime 65535 non-prime 262143 non-prime 1048575 non-prime 4194303 non-prime 16777215 non-prime */
static size_t khibbard[] = { 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607, 16777215, (SIZE_T_MAX) };
/* kbentley: 4 non-prime 40 non-prime 3280 non-prime 29524 non-prime 265720 non-prime 2391484 non-prime */
static size_t kbentley[] = { 1, 4, 13, 40, 121, 364, 1093, 3280, 9841, 29524, 88573, 265720, 797161, 2391484, 7174453, (SIZE_T_MAX) };
static size_t kprimes2[] = { 1, 3, 7, 17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 21911, 43853, 87719, 175447, 350899, 701819, 1403641, 2807303, 5614657, 11229331, (SIZE_T_MAX) };
#else
static size_t gaps[] = { 1, 4, 10, 23 };
#endif
static size_t ngaps = sizeof(gaps)/sizeof(size_t);

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shellsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t))
#else
    int typeindex)
#endif
{
    size_t h, n, nmemb, o;
    char *pa, *pb, *pc, *pl, *pu;

    pl=base+first*size;
    pu=base+beyond*size;
    /* loop, decreasing gap */
    /* obtain largest applicable gap h */
    for (nmemb=beyond-first,n=0UL,h=gaps[n]; h<=nmemb; h=gaps[++n]) { ; }
    for (h=gaps[--n]; ; h=gaps[--n]) {
        for (o=h*size,pa=pc=pl+o; pa<pu; pa+=size) {
            for (pb=pa; (pb>=pc)&&(0<compar(pb-o,pb)); pb-=o) {
#if SWAP_FUNCTION
                swapf(pb-o,pb,size);
#else
                iswap(pb-o,pb,size,typeindex);
#endif
            }
        }
        if (1UL==h) return;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shellsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
#if SWAP_FUNCTION
    shellsort_internal(base,0UL,nmemb,size,compar,whichswap(base,size));
#else
    shellsort_internal(base,0UL,nmemb,size,compar,type_index(base,size));
#endif
}


#define MAX_NETWORK 10UL
#if SWAP_FUNCTION
#define COMPARE_EXCHANGE(ma,mb) if(0<compar(ma,mb))swapf(ma,mb,size)
#else
#define COMPARE_EXCHANGE(ma,mb) if(0<compar(ma,mb))iswap(ma,mb,size,typeindex)
#endif

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void networksort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t nmemb;

    if ((2UL<=(nmemb=beyond-first))&&((MAX_NETWORK)>=nmemb)) {
        char *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9;

        p0=base+first*size;
        p1=p0+size;
        switch (nmemb) {
            case 2UL : /* 1 comparison */
                COMPARE_EXCHANGE(p0,p1);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
#endif
            return;
            case 3UL : /* 3 comparisons */
                p2=p1+size;
                COMPARE_EXCHANGE(p0,p2); /* 1 */
                COMPARE_EXCHANGE(p0,p1); /* 2 */
                COMPARE_EXCHANGE(p1,p2); /* 3 */
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
#endif
            return;
            case 4UL : /* 5 comparisons in 3 parallel groups */
                p2=p1+size;
                p3=p2+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p2);
                    COMPARE_EXCHANGE(p1,p3);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p2,p3);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p1,p2);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
#endif
            return;
            case 5UL : /* 9 comparisons in 5 parallel groups */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p4);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p0,p2);
                    COMPARE_EXCHANGE(p1,p3);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p2,p4);
                /* parallel group 4 */
                    COMPARE_EXCHANGE(p2,p3);
                    COMPARE_EXCHANGE(p1,p4);
                /* parallel group 5 */
                    COMPARE_EXCHANGE(p1,p2);
                    COMPARE_EXCHANGE(p3,p4);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
                A(0>=compar(p3,p4));
#endif
            return;
            case 6UL : /* 12 comparisons in 6 parallel groups */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p4);
                    COMPARE_EXCHANGE(p1,p5);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p0,p2);
                    COMPARE_EXCHANGE(p1,p3);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p2,p4);
                    COMPARE_EXCHANGE(p3,p5);
                /* parallel group 4 */
                    COMPARE_EXCHANGE(p2,p3);
                    COMPARE_EXCHANGE(p4,p5);
                /* parallel group 5 */
                    COMPARE_EXCHANGE(p1,p4);
                /* parallel group 6 */
                    COMPARE_EXCHANGE(p1,p2);
                    COMPARE_EXCHANGE(p3,p4);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
                A(0>=compar(p3,p4));
                A(0>=compar(p4,p5));
#endif
            return;
            case 7UL : /* 16 comparisons in 6 parallel groups */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p4);
                    COMPARE_EXCHANGE(p1,p5);
                    COMPARE_EXCHANGE(p2,p6);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p0,p2);
                    COMPARE_EXCHANGE(p1,p3);
                    COMPARE_EXCHANGE(p4,p6);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p2,p4);
                    COMPARE_EXCHANGE(p3,p5);
                /* parallel group 4 */
                    COMPARE_EXCHANGE(p2,p3);
                    COMPARE_EXCHANGE(p4,p5);
                /* parallel group 5 */
                    COMPARE_EXCHANGE(p1,p4);
                    COMPARE_EXCHANGE(p3,p6);
                /* parallel group 6 */
                    COMPARE_EXCHANGE(p1,p2);
                    COMPARE_EXCHANGE(p3,p4);
                    COMPARE_EXCHANGE(p5,p6);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
                A(0>=compar(p3,p4));
                A(0>=compar(p4,p5));
                A(0>=compar(p5,p6));
#endif
            return;
            case 8UL : /* 19 comparisons in 6 parallel groups */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                p7=p6+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p4);
                    COMPARE_EXCHANGE(p1,p5);
                    COMPARE_EXCHANGE(p2,p6);
                    COMPARE_EXCHANGE(p3,p7);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p0,p2);
                    COMPARE_EXCHANGE(p1,p3);
                    COMPARE_EXCHANGE(p4,p6);
                    COMPARE_EXCHANGE(p5,p7);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p2,p4);
                    COMPARE_EXCHANGE(p3,p5);
                    COMPARE_EXCHANGE(p6,p7);
                /* parallel group 4 */
                    COMPARE_EXCHANGE(p2,p3);
                    COMPARE_EXCHANGE(p4,p5);
                /* parallel group 5 */
                    COMPARE_EXCHANGE(p1,p4);
                    COMPARE_EXCHANGE(p3,p6);
                /* parallel group 6 */
                    COMPARE_EXCHANGE(p1,p2);
                    COMPARE_EXCHANGE(p3,p4);
                    COMPARE_EXCHANGE(p5,p6);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
                A(0>=compar(p3,p4));
                A(0>=compar(p4,p5));
                A(0>=compar(p5,p6));
                A(0>=compar(p6,p7));
#endif
            return;
            case 9UL : /* 25 comparisons in 9 parallel groups */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                p7=p6+size;
                p8=p7+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p3,p4);
                    COMPARE_EXCHANGE(p6,p7);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p1,p2);
                    COMPARE_EXCHANGE(p4,p5);
                    COMPARE_EXCHANGE(p7,p8);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p3,p4);
                    COMPARE_EXCHANGE(p6,p7);
                    COMPARE_EXCHANGE(p2,p5);
                /* parallel group 4 */
                    COMPARE_EXCHANGE(p0,p3);
                    COMPARE_EXCHANGE(p1,p4);
                    COMPARE_EXCHANGE(p5,p8);
                /* parallel group 5 */
                    COMPARE_EXCHANGE(p3,p6);
                    COMPARE_EXCHANGE(p4,p7);
                    COMPARE_EXCHANGE(p2,p5);
                /* parallel group 6 */
                    COMPARE_EXCHANGE(p0,p3);
                    COMPARE_EXCHANGE(p1,p4);
                    COMPARE_EXCHANGE(p5,p7);
                    COMPARE_EXCHANGE(p2,p6);
                /* parallel group 7 */
                    COMPARE_EXCHANGE(p1,p3);
                    COMPARE_EXCHANGE(p4,p6);
                /* parallel group 8 */
                    COMPARE_EXCHANGE(p2,p4);
                    COMPARE_EXCHANGE(p5,p6);
                /* parallel group 9 */
                    COMPARE_EXCHANGE(p2,p3);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
                A(0>=compar(p3,p4));
                A(0>=compar(p4,p5));
                A(0>=compar(p5,p6));
                A(0>=compar(p6,p7));
                A(0>=compar(p7,p8));
#endif
            return;
            case 10UL : /* 29 comparisons in 9 parallel groups */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                p7=p6+size;
                p8=p7+size;
                p9=p8+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(p0,p5);
                    COMPARE_EXCHANGE(p1,p6);
                    COMPARE_EXCHANGE(p2,p7);
                    COMPARE_EXCHANGE(p3,p8);
                    COMPARE_EXCHANGE(p4,p9);
                /* parallel group 2 */
                    COMPARE_EXCHANGE(p0,p3);
                    COMPARE_EXCHANGE(p1,p4);
                    COMPARE_EXCHANGE(p5,p8);
                    COMPARE_EXCHANGE(p6,p9);
                /* parallel group 3 */
                    COMPARE_EXCHANGE(p0,p2);
                    COMPARE_EXCHANGE(p3,p6);
                    COMPARE_EXCHANGE(p7,p9);
                /* parallel group 4 */
                    COMPARE_EXCHANGE(p0,p1);
                    COMPARE_EXCHANGE(p2,p4);
                    COMPARE_EXCHANGE(p5,p7);
                    COMPARE_EXCHANGE(p8,p9);
                /* parallel group 5 */
                    COMPARE_EXCHANGE(p1,p2);
                    COMPARE_EXCHANGE(p3,p5);
                    COMPARE_EXCHANGE(p4,p6);
                    COMPARE_EXCHANGE(p7,p8);
                /* parallel group 6 */
                    COMPARE_EXCHANGE(p1,p3);
                    COMPARE_EXCHANGE(p2,p5);
                    COMPARE_EXCHANGE(p4,p7);
                    COMPARE_EXCHANGE(p6,p8);
                /* parallel group 7 */
                    COMPARE_EXCHANGE(p2,p3);
                    COMPARE_EXCHANGE(p6,p7);
                /* parallel group 8 */
                    COMPARE_EXCHANGE(p3,p4);
                    COMPARE_EXCHANGE(p5,p6);
                /* parallel group 9 */
                    COMPARE_EXCHANGE(p4,p5);
#if ASSERT_CODE > 1
                A(0>=compar(p0,p1));
                A(0>=compar(p1,p2));
                A(0>=compar(p2,p3));
                A(0>=compar(p3,p4));
                A(0>=compar(p4,p5));
                A(0>=compar(p5,p6));
                A(0>=compar(p6,p7));
                A(0>=compar(p7,p8));
                A(0>=compar(p8,p9));
#endif
            return;
        }
    }
    /* Fall back to insertion sort in case size is out-of-bounds. */
#if SWAP_FUNCTION
    isort_internal(base,first,beyond,size,compar,swapf);
#else
    isort_internal(base,first,beyond,size,compar,typeindex);
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void networksort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
#if SWAP_FUNCTION
    networksort_internal(base,0UL,nmemb,size,compar,whichswap(base,size));
#else
    networksort_internal(base,0UL,nmemb,size,compar,type_index(base,size));
#endif
}

/* support functions */

/* array element moves in blocks */
/* Given two adjacent blocks of data delimited by pointers
   pa = start of block A, pb = start of block B, and pc pointing just past the
   end of block B, swap the minimum number of chars to place all of block B
   before block A (order within blocks is not preserved), returning a pointer
   to the start of block A (which is now after block B).
 Z |  A |   B  |  C
    pa   pb     pc
    vvvvvvvvvvv
 Z |  B  |  A  |  C
    pa    p     pc
 returned ^
   In addition to reversing the relative order of blocks A and B, this function
   is useful to consolidate like regions separated by a different type of
   region, e.g. when it is desirable to consolidate Z and B and/or A and C in
   the above diagram.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *blockmove(char *pa, char *pb, char *pc,
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    A(pa<=pb);A(pb<=pc);
    if (pb<pc) {
        if (pa<pb) {
            size_t p=pc-pb, n=pb-pa;
            if (p<n) n=p;
#if SWAP_FUNCTION
            swapf(pa,pc-n,n); return pa+p;
#else
            iswap(pa,pc-n,n,typeindex); return pa+p;
#endif
        } return pc;
    } return pa;
}

/* Ternary median-of-3, biased toward middle element if possible, else first element. */
/* Average 8/3 comparisons for 3 elements (distinct values) = 0.889 comparisons/element */
static char *fmed3(char *pa, char *pb, char *pc, int(*compar)(const void *,const void *))
{
    int c=compar(pa,pb);
    if (0!=c) {
        int d=compar(pb,pc);
        if (0!=d) {
            if ((0<d)&&(0>c)) {
                if (0>compar(pa,pc)) return pc;
                else return pa;
            } else if ((0>d)&&(0<c)) {
                if (0<compar(pa,pc)) return pc;
                else return pa;
            }
        }
    }
    return pb;
}

static char *fmed5(char *pa, char *pb, char *pc, char *pd, char *pe,
    int(*compar)(const void *,const void *))
{
    char *t;

    /* No swapping of data, only pointers. */
#if 0
    /* The following 7 comparators implement median finding as a sorting network:
       a --o--o---------------- l1 smaller than 3 of a,b,d,e (cannot be median of 5)
           |  |     :
       b --o--+--o----o--o----- l2 a,b,c,d, or e
              |  |  : |  |
       c -----+--+----+--o--o-- m  a,b,c,d, or e
              |  |  : |     |
       d --o--o--+----o-----o-- u1 a,b,c,d, or e
           |     |  :
       e --o-----o------------- u2 larger than 3 of a,b,d,e (cannot be median of 5)
       Four compare-exchange pairs followed by median-of-3.
       Using ternary median of 2 instead of a sorting network reduces
          comparisons for the median-of-3 part from 3 comparisons / 3 elements
          to 8/3 comparisons / 3 elements (for distinct inputs).
       Average (distinct inputs) 6.667 comparisons per 5 elements =
          1.333 comparisons/element.
    */
    if (0<compar(pa,pb)) t=pa, pa=pb, pb=t;
    if (0<compar(pd,pe)) t=pd, pd=pe, pe=t;
    if (0<compar(pa,pd)) t=pa, pa=pd, pd=t;
    if (0<compar(pb,pe)) t=pb, pb=pe, pe=t;
    return fmed3(pb,pc,pd,compar);
#else
if (2<debug) (V)fprintf(stderr,"%s line %d: pa=%p, pb=%p, pc=%p, pd=%p, pe=%p\n",__func__,__LINE__,pa,pb,pc,pd,pe);
    /* 6 comparisons per set of 5 elements = 1.2 comparisons/element */
    if (0<compar(pa,pb)) t=pa, pa=pb, pb=t;
    if (0<compar(pc,pd)) t=pc, pc=pd, pd=t;
    if (0<compar(pa,pc)) { pc=pe; if (0<compar(pc,pd)) t=pc, pc=pd, pd=t;
    } else { pa=pe; if (0<compar(pa,pb)) t=pa, pa=pb, pb=t; }
    if (0<compar(pa,pc)) { if (0<compar(pa,pd)) return pa; else return pd;
    } else { if (0<compar(pb,pc)) return pb; else return pc; }
    A(0>1); /* shouldn't get here */
#endif
}

/* medians of sets of 3 elements */
/* Given base, size, and compar as for qsort, plus a row offset and sample
   offset, both in chars, find medians of sets of 3 elements where the three
   elements for each median are taken from three rows starting with base, and
   each subsequent set of three elements is offset from the previous set by
   sample_offset. As each median is determined, it is placed in the
   middle row, to which a pointer is returned. When moving the median to the
   middle, avoid creating a reverse-sorted set by using rotation to place the
   elements in sorted order.  This is not quite a full sort of the three
   elements, as (e.g.) the third element might be the minimum if the first two
   compare equal (in which case there is no data movement).
   N.B. base is not necessarily the first element in the array passed to qsort.
*/
/* called from repivot() */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *medians3(char *base, size_t size,
    int(*compar)(const void *, const void *), size_t row_offset,
    size_t sample_offset, size_t row_samples,
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t))
#else
    int typeindex)
#endif
{
    char *pa, *pb;
    size_t n, o;

#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr, "// %s line %d: row_offset=%lu, sample_offset=%lu, row_samples=%lu\n",__func__,__LINE__,row_offset,sample_offset,row_samples);
}
#endif
    for (n=o=0UL; n<row_samples; n++,o+=sample_offset) {
        pa=base+o;
        pb=pa+row_offset; /* middle element */
        pa=fmed3(pa,pb,pb+row_offset,compar);
#if SWAP_FUNCTION
        if (pa!=pb) swapf(pa,pb,size);
#else
        if (pa!=pb) iswap(pa,pb,size,typeindex);
#endif
    }
    return base+row_offset; /* middle row */
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
/* spacings are in chars */
char *remedian(char *middle, size_t row_spacing, size_t sample_spacing,
    unsigned int idx, int(*compar)(const void*,const void*))
{
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr, "// %s line %d: middle=%p, row_spacing=%lu, sample_spacing=%lu, idx=%u\n",__func__,__LINE__,middle,row_spacing,sample_spacing,idx);
}
#endif
    if (0UL < --idx) {
        char *pa, *pb, *pc;
        size_t s=sample_spacing/3UL;

        pa=remedian(middle-s,row_spacing,s,idx,compar);
        pb=remedian(middle,row_spacing,s,idx,compar);
        pc=remedian(middle+s,row_spacing,s,idx,compar);
        return fmed3(pa,pb,pc,compar);
    }
    return fmed3(middle-row_spacing,middle,middle+row_spacing,compar);
}

/* fast pivot selection using a single sample, median-of-3, or remedian of samples */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *select_pivot(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void*), unsigned int table_index,
    size_t *pk)
{
    size_t n, nmemb, r, s, t;
    char *pivot;

    nmemb=beyond-first;
    /* total samples (a power of 3) */
    n=sorting_sampling_table[table_index].samples;
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) fprintf(stderr, "// %s line %d: base=%p, first=%lu, beyond=%lu, nmemb=%lu, n=%lu\n",__func__,__LINE__,base,first,beyond,nmemb,n);
#endif
    pivot=base+size*(first+(nmemb>>1));      /* [upper-]middle element */
    switch (n) {
        case 1UL :
            s=r=0UL;
        break;
        case 3UL : /*FALLTHROUGH*/
        case 5UL :
            s=r=(nmemb/n)*size;
        break;
        default : /* > 3, power of 3 */
#if USE_FMED5
            t=n/3UL;  /* use sampling table if no entries for 5 */
#else
            t=sorting_sampling_table[table_index-1U].samples;    /* per row (n/3) */
#endif
            s=nmemb/n;                         /* uniform sample spacing */
            r=s*t;                             /* uniform row spacing */
            s*=size, r*=size;                  /* spacing in chars */
        break;
    }
#if DEBUG_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr, "// %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p, pu=%p\n// ",__func__,__LINE__,nmemb,pivot,(pivot-base)/size,*((int *)pivot),base+first*size,base+beyond*size);
    print_int_array(base,first,beyond-1UL);
}
#endif
    A(base+first*size<=pivot);A(pivot<base+beyond*size);
    switch (n) {
        case 1UL :
        break;
        case 5UL :
            /* median-of-5, no data movement */
            t=(s<<1); /* 2s */
            pivot = fmed5(pivot-t,pivot-s,pivot,pivot+s,pivot+t,compar);
        break;
        default : /* power of 3, remedian of samples */
            n/=3UL;
            pivot=remedian(pivot,r,r,table_index,compar);
        break;
    }
#if 0
if (2<debug) {
    fprintf(stderr, "%s line %d: pivot=%p[%lu], r=%lu, s=%lu\n",__func__,__LINE__,pivot,(pivot-base)/size,r,s);
    print_int_array(base,first,beyond-1UL);
}
#endif
#if DEBUG_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr, "// %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p, pu=%p\n// ",__func__,__LINE__,nmemb,pivot,(pivot-base)/size,*((int *)pivot),base+first*size,base+beyond*size);
    print_int_array(base,first,beyond-1UL);
}
#endif
    A(base+first*size<=pivot);A(pivot<base+beyond*size);
    return pivot;
}

/* forward declaration */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_loop(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *),
    const size_t *pk, size_t firstk, size_t beyondk,
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    struct sampling_table_struct *pst, unsigned int table_index,
    char *pv, size_t swaps, int nr
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
   );

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned int do_sort(unsigned int distribution, size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
    )
{
    if ((NULL!=pk)&&(beyondk>firstk)) {
        /* no sort if finding median of medians with SAVE_PARTIAL */
#if 1 /* 0 to select unless no order statistics (for development/testing) */
      /* 0 actually works fine and saves tests of ppeq/ppgt/nk vs. nmemb */
      /* 1 allows for sorting when not saving comparisons of partial partitioning (median-of-medians) */
# if SAVE_PARTIAL
        if ((NULL==ppeq)&&(NULL==ppgt)) {
# endif
            size_t nk=beyondk-firstk, nmemb=beyond-first;
            if (5UL>=nmemb) {
                if (1<debug) (V)fprintf(stderr, "// %s line %d: nmemb=%lu: sorting\n",__func__,__LINE__,nmemb);
                return 1U;
            } else if (261UL>=nmemb) {
                size_t idx=nmemb-6UL, idx2;
                switch (distribution) {
                    case 0U : /*FALLTHROUGH*/
                    case 7U : /*FALLTHROUGH*/
                        idx2=0UL;
                    break;
                    case 2U :
                        idx2=1UL;
                    break;
                    case 5U :
                        idx2=2UL;
                    break;
                    default :
                        idx2=3UL;
                    break;
                }
                if (selection_breakpoint[idx][idx2]<nk) {
                    if (1<debug) (V)fprintf(stderr, "// %s line %d: nk=%lu, nmemb=%lu, idx=%lu, distribution=%u, idx2=%lu, selection_breakpoint[%lu][%lu]=%u: sorting\n",__func__,__LINE__,nk,nmemb,idx,distribution,idx2,idx,idx2,selection_breakpoint[idx][idx2]);
                    return 1U;
                }
                  else if (1<debug) (V)fprintf(stderr, "// %s line %d: nk=%lu, nmemb=%lu, idx=%lu, distribution=%u, idx2=%lu, selection_breakpoint[%lu][%lu]=%u: not sorting\n",__func__,__LINE__,nk,nmemb,idx,distribution,idx2,idx,idx2,selection_breakpoint[idx][idx2]);
            } else {
                size_t t=nmemb/10UL;
                switch (distribution) {
                    case 0U : /*FALLTHROUGH*/
                    case 7U : /*FALLTHROUGH*/
                        if (nk>t) {
                            if (1<debug) (V)fprintf(stderr, "// %s line %d: nk=%lu, nmemb=%lu, t=%lu: sorting\n",__func__,__LINE__,nk,nmemb,t);
                            return 1U;
                        }
                          else if (1<debug) (V)fprintf(stderr, "// %s line %d: pk=%p, firstk=%lu beyondk=%lu, nk=%lu, nmemb=%lu, t=%lu: not sorting\n",__func__,__LINE__,pk,firstk,beyondk,nk,nmemb,t);
                    break;
                    default :
                        if (nk>nmemb-t) {
                            if (1<debug) (V)fprintf(stderr, "// %s line %d: nk=%lu, nmemb=%lu, t=%lu: sorting\n",__func__,__LINE__,nk,nmemb,t);
                            return 1U;
                        }
                          else if (1<debug) (V)fprintf(stderr, "// %s line %d: pk=%p, firstk=%lu beyondk=%lu, nk=%lu, nmemb=%lu, t=%lu: not sorting\n",__func__,__LINE__,pk,firstk,beyondk,nk,nmemb,t);
                    break;
                }
            }
# if SAVE_PARTIAL
        }
# endif
#endif
        return 0U;
    }
    return 1U;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *pfk, size_t *pbk)
{
    A(NULL!=pfk);A(NULL!=pbk);A(NULL!=pk);
    size_t lk, ll, lr, rk, rl, rr;

    /* binary search through pk to find limits for each region */
    ll=rl=firstk, lr=rr=beyondk;
    lk=rk=ll+((lr-ll)>>1);
    while (ll<lr) {
        if (pk[lk]>=first) { if (0UL<lk) lr=lk-1UL; else lr=lk; }
        else ll=lk;
        lk=ll+((lr+1UL-ll)>>1);
        if (lk==beyondk) break;
        A(lk>=firstk);A(rk<=beyondk);
    }
    if ((lk<beyondk)&&(pk[lk]<first)) lk++;
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) (V)fprintf(stderr, "// %s line %d: lk=%lu, rk=%lu, pk[%lu]=%lu, first=%lu, beyond=%lu\n",__func__,__LINE__,lk,rk,lk,pk[lk],first,beyond);
#endif
    A(lk>=firstk);A(rk<=beyondk);
    while (rl<rr) {
        if (pk[rk]<beyond) { if (rk<beyondk) rl=rk+1UL; else rl=rk; }
        else rr=rk;
        rk=rl+((rr-rl)>>1);
        if (rk==beyondk) break;
        A(lk>=firstk);A(rk<=beyondk);
    }
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) (V)fprintf(stderr, "// %s line %d: lk=%lu, rk=%lu, pk[%lu]=%lu, first=%lu, beyond=%lu\n",__func__,__LINE__,lk,rk,lk,pk[lk],first,beyond);
#endif
    *pfk=lk;
    *pbk=rk;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned int kdistribution(size_t first, size_t beyond, const size_t *pk,
    size_t firstk, size_t beyondk)
{
    if (NULL!=pk) {
        size_t nmemb=beyond-first;

        if (5UL<nmemb) {
            size_t nk=beyondk-firstk;
            size_t f=nmemb>>2 /* /4UL */ ;
            size_t t=nmemb/9UL;
            size_t m[2];
            size_t bk, fk, x, y;
            unsigned int r=0UL;

            m[0]=first+((nmemb-1UL)>>1);
            m[1]=first+(nmemb>>1);
            x=y=10UL;
            if (f<x) x=f;
            if (t<y) y=t;
            if (pk[beyondk-1UL]<m[0]) r|=4U;
            else {
                klimits(first,first+x,pk,firstk,beyondk,&fk,&bk);
                if ((bk>fk)||(pk[beyondk-1UL]<m[0])) r|=4U;
            }
            if ((pk[firstk]>=m[0]-y)&&(pk[beyondk-1UL]<=m[1]+y)) r|=2U;
            else {
                klimits(m[0]-y,m[1]+1UL+y,pk,firstk,beyondk,&fk,&bk);
                if (bk>fk) r|=2U;
            }
            if (pk[firstk]>m[1]) r|=1U;
            else {
                klimits(beyond-x,beyond,pk,firstk,beyondk,&fk,&bk);
                if ((bk>fk)||(pk[firstk]>m[1])) r|=1U;
            }
#if DEBUG_CODE
if (0<debug) (V)fprintf(stderr, "// %s line %d: first=%lu, beyond=%lu, nmemb=%lu, nk=%lu, pk[%lu]=%lu, m[0]=%lu, m[1]=%lu, f=%lu, t=%lu, x=%lu, y=%lu, fk=%lu, bk=%lu, pk[%lu]=%lu, r=%u\n",__func__,__LINE__,first,beyond,nmemb,nk,firstk,pk[firstk],m[0],m[1],f,t,x,y,fk,bk,beyondk-1UL,pk[beyondk-1UL],r);
#endif
            return r;
        }
        return 7U;
    }
    return 7U;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
struct sampling_table_struct *sampling_table(size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk,
#if SAVE_PARTIAL
    char **ppeq, char **ppgt,
#endif
    unsigned int *psort, unsigned int *pd, size_t *psz)
{
    if (beyond>first) {
        *pd=kdistribution(first,beyond,pk,firstk,beyondk);
        *psort=do_sort(*pd,first,beyond,pk,firstk,beyondk
#if SAVE_PARTIAL
            ,ppeq,ppgt
#endif
        );
    }
    if (0U==*psort) {
        switch (*pd) {
            case 4U : /*FALLTHROUGH*/
            case 6U : /* left */
                if (NULL!=psz)
                    *psz=sizeof(separated_sampling_table)
                        /sizeof(separated_sampling_table[0]);
if (0<debug) (V)fprintf(stderr, "// %s line %d: sort=%u, d=%u; separated sampling table\n",__func__,__LINE__,*psort,*pd);
            return separated_sampling_table;
            case 1U : /*FALLTHROUGH*/
            case 3U : /* right */
                if (NULL!=psz)
                    *psz=sizeof(separated_sampling_table)
                        /sizeof(separated_sampling_table[0]);
if (0<debug) (V)fprintf(stderr, "// %s line %d: sort=%u, d=%u; separated sampling table\n",__func__,__LINE__,*psort,*pd);
            return separated_sampling_table;
            case 5U : /* separated */
                if (NULL!=psz)
                    *psz=sizeof(separated_sampling_table)
                        /sizeof(separated_sampling_table[0]);
if (0<debug) (V)fprintf(stderr, "// %s line %d: sort=%u, d=%u; separated sampling table\n",__func__,__LINE__,*psort,*pd);
            return separated_sampling_table;
            case 2U : /* middle */
                if (NULL!=psz)
                    *psz=sizeof(middle_sampling_table)
                        /sizeof(middle_sampling_table[0]);
if (0<debug) (V)fprintf(stderr, "// %s line %d: sort=%u, d=%u; middle sampling table\n",__func__,__LINE__,*psort,*pd);
            return middle_sampling_table;
            default : /* distributed */
            break;
        }
    }
    if (NULL!=psz)
        *psz=sizeof(sorting_sampling_table)
            /sizeof(sorting_sampling_table[0]);
if (0<debug) (V)fprintf(stderr, "// %s line %d: sort=%u, d=%u; sorting sampling table\n",__func__,__LINE__,*psort,*pd);
    return sorting_sampling_table;
}

/* repivot using median-of-medians */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *repivot(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void*),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    struct sampling_table_struct *pst, unsigned int table_index,
    size_t *pk,
    char *pv, size_t swaps,
    char **ppc, char **ppd, char **ppe, char **ppf)
{
    size_t n, nmemb;
    char *pa, *pivot, *pl;

    nmemb=beyond-first;
    pa=pl=base+first*size;
#if 1
    /* Finding a pivot with guaranteed intermediate rank. Ideally,
       median (50%).  Blum, Floyd, Pratt, Rivest, Tarjan
       median-of-medians using sets of 5 elements with recursion
       guarantees rank in (asymptotically) 30-70% range, often
       better; can guarantee linear median-finding, N log(N)
       sorting. Simplification ignores "leftover" elements with a
       slight increase in rank range.  Non-recursive method (using
       separate linear median finding) can use sets of 3 elements to
       provide a tighter 33.33% to 66.67% range (again, slightly
       wider if "leftover" elements are ignored) at lower
       computational cost.
    */
    /* Medians of sets of 3 elements. */
    n=nmemb/3UL;    /* number of complete sets */
# if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, nmemb=%lu, n=%lu\n// ",__func__,__LINE__,first,beyond,nmemb,n);
print_int_array(base,first,beyond-1UL);
}
# endif
    if (1UL<=n) {
        if (nmemb>3UL*n) pa+=size; /* cheaper than %3 test */
# if SWAP_FUNCTION
        pa=medians3(pa,size,compar,n*size,size,n,swapf);
# else
        pa=medians3(pa,size,compar,n*size,size,n,typeindex);
# endif
        A(pl<pa);
    }
# if 0 /* median-of-medians-of-medians */
    /* Chen & Dumitrescu use two iterations of median-of-3 followed by
       selection of the median of that 1/9 of the original array to yield
       about the same range. More work in medians3, less in selection.
       Overall more work to get a slightly inferior rank range (because of
       fewer comparisons).
    */
    if (3UL<=n) {
/* XXX something buggy here; fails against adversary */
        n/=3UL;
#  if SWAP_FUNCTION
        pa=medians3(pa,size,compar,n*size,size,n,swapf);
#  else
        pa=medians3(pa,size,compar,n*size,size,n,typeindex);
#  endif
    }
# endif
    *ppc=pa;
    if (1UL<n) { /* median of medians [of medians] */
        unsigned int d, srt;
        size_t karray[1], stsz;
        first=(pa-base)/size;
        karray[0]=first+(n>>1); /* upper-median for even size arrays */
        beyond=first+n;
        A(first<beyond);
if (karray[0]>=beyond) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, n=%lu, first=%lu, mid=%lu, beyond=%lu\n",__func__,__LINE__,nmemb,n,first,karray[0],beyond);
        A(first<=karray[0]);A(karray[0]<beyond);
        *ppf=base+beyond*size;
# if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, nmemb=%lu, n=%lu, karray[0]=%lu, *ppc=pa@%lu, *ppf=base+beyond*size@%lu\n// ",__func__,__LINE__,first,beyond,nmemb,n,karray[0],(pa-base)/size,(*ppf-base)/size);
print_int_array(base,first,beyond-1UL);
}
# endif
        pst=sampling_table(first,beyond,karray,0UL,1UL,
#if SAVE_PARTIAL
            ppd,ppe,
#endif
            &srt,&d,&stsz);
        A(0U==srt);A(2U==d);
        if (table_index>=stsz) table_index=stsz-1UL;
        else
            while (n>pst[table_index].min_nmemb) table_index++;
        while (n<pst[table_index].min_nmemb) table_index--;
#if ASSERT_CODE
        if (n<pst[table_index].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: n=%lu, table_index=%u, pst[%u].min_nmemb=%lu\n",
                __func__,__LINE__,n,table_index,table_index,
                pst[table_index].min_nmemb);
        A(pst[table_index].min_nmemb<=n);
        if (n>=pst[table_index+1UL].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: n=%lu, table_index=%u, pst[%u].min_nmemb=%lu\n",
                __func__,__LINE__,n,table_index,table_index+1UL,
                pst[table_index+1UL].min_nmemb);
        A(pst[table_index+1UL].min_nmemb>n);
#endif
# if DEBUG_CODE + ASSERT_CODE
#  if SAVE_PARTIAL
        *ppd = *ppe = NULL;
#  endif
# endif
        quickselect_loop(base,first,beyond,size,compar,karray,0UL,1UL,
# if SWAP_FUNCTION
            swapf
# else
            typeindex
# endif
            ,pst,table_index,pv,swaps,0
# if SAVE_PARTIAL
            ,ppd,ppe
# endif
        );
# if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, nmemb=%lu, n=%lu, pivot@karray[0]=%lu\n// ",__func__,__LINE__,first,beyond,nmemb,n,karray[0]);
print_int_array(base,first,beyond-1UL);
}
# endif
        pivot=base+karray[0]*size; /* pointer to median of medians */
# if SAVE_PARTIAL
        if (0U!=save_partial) {
            /* Middle third of array (medians) is partitioned. */
#  if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, karray[0]=%lu, pivot=%p[%lu], *ppd=%p[%lu], *ppe=%p[%lu]\n",__func__,__LINE__,first,beyond,karray[0],pivot,(pivot-base)/size,*ppd,(*ppd-base)/size,*ppe,(*ppe-base)/size);
}
#  endif
#if ASSERT_CODE
if ((*ppe<=pivot)||(*ppd>pivot))
(V)fprintf(stderr, "%s line %d: save_partial=%u, pivot=%p, ppd=%p, *ppd=%p, ppe=%p, *ppe=%p\n",__func__,__LINE__,save_partial,pivot,ppd,NULL!=ppd?*ppd:NULL,ppe,NULL!=ppe?*ppe:NULL);
#endif
            A(*ppd<=pivot);A(pivot<*ppe);
        } else {
            *ppd=pivot;
            *ppe=pivot+size;
#  if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, karray[0]=%lu, pivot=%p[%lu], *ppd=%p[%lu], *ppe=%p[%lu]\n",__func__,__LINE__,first,beyond,karray[0],pivot,(pivot-base)/size,*ppd,(*ppd-base)/size,*ppe,(*ppe-base)/size);
}
#  endif
        }
# else
        *ppd=pivot;
        *ppe=pivot+size;
#  if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, karray[0]=%lu, pivot=%p[%lu], *ppd=%p[%lu], *ppe=%p[%lu]\n",__func__,__LINE__,first,beyond,karray[0],pivot,(pivot-base)/size,*ppd,(*ppd-base)/size,*ppe,(*ppe-base)/size);
}
#  endif
# endif
    } else {
        *ppd=pivot=pa;
        *ppe=pivot+size;
        *ppf=pa+n*size;
#  if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, pivot=%p[%lu], *ppd=%p[%lu], *ppe=%p[%lu]\n",__func__,__LINE__,first,beyond,pivot,(pivot-base)/size,*ppd,(*ppd-base)/size,*ppe,(*ppe-base)/size);
}
#  endif
    }
#else
    /* (nearly) full remedian */
/* XXX something buggy here; fails against adversary */
    n=nmemb/3UL;
    *ppc=pl;
    *ppd=pivot=remedian(pl,size*n,size,n,compar);
    *ppe=pivot+size;
    *ppf=pa+n*size;
#endif
    return pivot;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int should_repivot(size_t nmemb, size_t n, unsigned int table_index, size_t *pk,
    int *pn2)
{
    size_t factor1, factor2, p, q, ratio;

    if (NULL==pk) { /* sorting */
        if (quickselect_small_array_cutoff+2UL>=n) return 0;
    } else { /* order statistic selection */
        if (8UL>=n) return 0; /* no repivot unless >= 3 medians-of-3 */
    }
    if (n>=nmemb) {
#if DEBUG_CODE
if ((2<debug)||(n>nmemb)) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, n=%lu\n",__func__,__LINE__,nmemb,n);
#endif
        ++*pn2;
        return 1;
    }
    q=nmemb-n;
    A(0UL<q);
    if (n<=q) return 0; /* ratio 0; return w/o incurring division cost */
    ratio=n/q;
    if (0<debug) /* for graphing worst-case partition ratios */
        if (ratio>stats_table[table_index].max_ratio) stats_table[table_index].max_ratio = ratio;
    /* aux tests, if not disabled */
    /* should be disabled for time vs. factor,limit graph */
    if (0==no_aux_repivot) {
        repivot_factors(table_index,pk,&factor1,&factor2);
        /* size-dependent count 1 limit test */
        if (ratio>=factor1) {
            if (0<debug) { /* for graphing worst-case partition ratios */
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, n=%lu, ratio>=%lu>=repivot_factor1=%lu\n",__func__,__LINE__,nmemb,n,ratio,factor1);
#endif
                stats_table[table_index].repivots++;
                stats_table[table_index].repivot_ratio = ratio;
            }
                return 1;
        }
        /* size-dependent count 2 limit test */
        if (ratio>=factor2) {
            ++*pn2;
            if (*pn2>=2UL) {
                if (0<debug) { /* for graphing worst-case partition ratios */
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, n=%lu, ratio>=%lu>=repivot_factor2=%lu, *pn2=%d, limit=%d\n",__func__,__LINE__,nmemb,n,ratio,factor2,*pn2,2UL);
#endif
                    stats_table[table_index].repivots++;
                    stats_table[table_index].repivot_ratio = ratio;
                }
                return 1;
            }
        }
    } else {
        /* global limit test */
        if (ratio>=repivot_factor) {
            if (0!=no_aux_repivot) {
                /* need limit here for time vs factor,limit graph */
                ++*pn2; /* use count for factor2 */
                if (*pn2>=lopsided_partition_limit) {
                    if (0<debug) { /* for graphing worst-case partition ratios */
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, n=%lu, ratio>=%lu>=repivot_factor=%lu, *pn2=%d, limit=%d\n",__func__,__LINE__,nmemb,n,ratio,repivot_factor,*pn2,lopsided_partition_limit);
#endif
                        stats_table[table_index].repivots++;
                        stats_table[table_index].repivot_ratio = ratio;
                    }
                    return 1;
                }
            } else {
                if (0<debug) { /* for graphing worst-case partition ratios */
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, n=%lu, ratio>=%lu>=repivot_factor=%lu\n",__func__,__LINE__,nmemb,n,ratio,repivot_factor);
#endif
                    stats_table[table_index].repivots++;
                    stats_table[table_index].repivot_ratio = ratio;
                }
                return 1;
            }
        }
    }
    return 0;
}

/* array partitioning */
/* partitioning based on Bentley & McIlroy's split-end partition, with
   type-independent deferral of pivot swapping to reduce introduction of
   disorder
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t partition(char *base, size_t first, size_t beyond, char *pivot,
    size_t size, int(*compar)(const void *,const void*),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    char *pv,
    size_t swaps, size_t *peq, size_t *pgt)
{
    char *pb, *pc, *pe, *pf, *pl, *pu;
    int c, d;
#if SWAP_FUNCTION
#if ASSERT_CODE
    int typeindex = type_index(base,size);
#endif
#endif

#if 0
fprintf(stderr, "%s line %d: base=%p, first=%lu, beyond=%lu, pivot=%p\n// ",__func__,__LINE__,base,first,beyond,pivot);
print_int_array(base,first,beyond-1UL);
#endif
#if DEBUG_CODE
if ((3<debug)||((2<debug)&&(0UL<swaps))) {
    (V)fprintf(stderr, "// %s: line %d: nmemb %lu, swaps %lu, pivot=%p[%lu]\n// ",__func__,__LINE__,beyond-first,swaps,pivot,(pivot-base)/size);
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
    pc=pl=base+size*first;
    pe=(pu=base+beyond*size)-size;
#if DEBUG_CODE
if ((pivot<pl)||(pivot>=pu)) {
    (V)fprintf(stderr, "// %s line %d: pivot=%p(%d), pl=%p, pc=%p, pe=%p, pu=%p\n// ",__func__,__LINE__,pivot,*((int *)pivot),pl,pc,pe,pu);
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
    A(pl<=pivot);A(pivot<pu); /* pivot in array */
    /* swap pivot to first or last position if 0 interchange swaps last partition */
    /* Counterintuitively, swapping to far end yields better results */
    /* But don't swap if the pivot is already at one end. */
    if (1UL>swaps) {
        if (pivot-pl<pu-pivot) { /* pivot is closer to first element */
#if SWAP_FUNCTION
            if (pl!=pivot) { swapf(pe,pivot,size); pivot=pe; }
#else
            if (pl!=pivot) { iswap(pe,pivot,size,typeindex); pivot=pe; }
#endif
        } else { /* pivot is closer to last element */
#if SWAP_FUNCTION
            if (pe!=pivot) { swapf(pl,pivot,size); pivot=pl; }
#else
            if (pe!=pivot) { iswap(pl,pivot,size,typeindex); pivot=pl; }
#endif
        }
    }
#if 1
    else
#endif
    if (0!=pivot_copy) { /* use a copy of the pivot */
        pivot=memcpy(pv,pivot,size);
    }
    /* +-----------------------------------------------------+ */
    /* |   =   |   <   |  ?               ?  |   >   |   =   | */
    /* +-----------------------------------------------------+ */
    /*  pl      b       c                   E e       f       u*/
#if DEBUG_CODE
if (2<debug) {
    (V)fprintf(stderr, "// %s line %d: pivot=%p(%d), pl=%p, pc=%p, pe=%p, pu=%p\n// ",__func__,__LINE__,pivot,*((int *)pivot),pl,pc,pe,pu);
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
    A(pc<=pe);

/* Kiwiel's algorithm L excluding cleanup as a macro */
/* use S1, S2 for pivot pointer update if required */
#if SWAP_FUNCTION
#define KIWIEL_L(S1,S2)                                                  \
    if (pivot==pc) pc+=size;                                             \
    else if (pivot==pe) pe-=size;                                        \
    /* I2(K) */                                                          \
    while ((pc<=pe)&&(0==(c=compar(pivot,pc)))) { pc+=size; }            \
    pb=pc;                                                               \
    if ((pc<=pe)&&(0<c)) {                                               \
        /* I3(K) */                                                      \
        for (pc+=size; (pc<=pe)&&(0<=(c=compar(pivot,pc))); pc+=size) {  \
            if (0==c) { A(pb!=pc); swapf(pb,pc,size); S1; pb+=size; } \
        }                                                                \
    }                                                                    \
    /* I4(K) */                                                          \
    while ((pc<pe)&&(0==(d=compar(pivot,pe)))) { pe-=size; }             \
    pf=pe+size; /* different semantics from Kiwiel's q */                \
    if ((pc<pe)&&(0>d)) {                                                \
        /* I5(K) */                                                      \
        for (pe-=size; (pc<pe)&&(0>=(d=compar(pivot,pe))); pe-=size) {   \
            if (0==d) { pf-=size; A(pe!=pf); swapf(pe,pf,size); S2; } \
        }                                                                \
    }                                                                    \
    while (pc<pe) {                                                      \
        /* I7 (there is no I6) */ A(0>c);A(0<d);                         \
        swapf(pc,pe,size);                                            \
        /* I8(J) */  A(pl<=pb);A(pb<=pc);A(pe<pf);A(pf<=pu);             \
        pc+=size, pe-=size;                                              \
        /* I9(J) */                                                      \
        for (; (pc<=pe)&&(0<=(c=compar(pivot,pc))); pc+=size) {          \
            if (0==c) { A(pb!=pc); swapf(pb,pc,size); S1; pb+=size; } \
        }                                                                \
        /* I10(J) */                                                     \
        for (; (pc<pe)&&(0>=(d=compar(pivot,pe))); pe-=size) {           \
            if (0==d) { pf-=size; A(pe!=pf); swapf(pe,pf,size); S2; } \
        }                                                                \
        /* I11(J): loop */                                               \
    }                                                                    \
    pe = pc-size /* terminating semicolon after macro call */
#else
#define KIWIEL_L(S1,S2)                                                  \
    if (pivot==pc) pc+=size;                                             \
    else if (pivot==pe) pe-=size;                                        \
    /* I2(K) */                                                          \
    while ((pc<=pe)&&(0==(c=compar(pivot,pc)))) { pc+=size; }            \
    pb=pc;                                                               \
    if ((pc<=pe)&&(0<c)) {                                               \
        /* I3(K) */                                                      \
        for (pc+=size; (pc<=pe)&&(0<=(c=compar(pivot,pc))); pc+=size) {  \
            if (0==c) { A(pb!=pc); iswap(pb,pc,size,typeindex); S1; pb+=size; } \
        }                                                                \
    }                                                                    \
    /* I4(K) */                                                          \
    while ((pc<pe)&&(0==(d=compar(pivot,pe)))) { pe-=size; }             \
    pf=pe+size; /* different semantics from Kiwiel's q */                \
    if ((pc<pe)&&(0>d)) {                                                \
        /* I5(K) */                                                      \
        for (pe-=size; (pc<pe)&&(0>=(d=compar(pivot,pe))); pe-=size) {   \
            if (0==d) { pf-=size; A(pe!=pf); iswap(pe,pf,size,typeindex); S2; } \
        }                                                                \
    }                                                                    \
    while (pc<pe) {                                                      \
        /* I7 (there is no I6) */                                        \
        A(0>c);A(0<d);                                                   \
        iswap(pc,pe,size,typeindex); swaps++;                                   \
        /* I8(J) */                                                      \
        A(pl<=pb);A(pb<=pc);A(pe<pf);A(pf<=pu);                          \
        pc+=size, pe-=size;                                              \
        /* I9(J) */                                                      \
        for (; (pc<=pe)&&(0<=(c=compar(pivot,pc))); pc+=size) {          \
            if (0==c) { A(pb!=pc); iswap(pb,pc,size,typeindex); S1; pb+=size; } \
        }                                                                \
        /* I10(J) */                                                     \
        for (; (pc<pe)&&(0>=(d=compar(pivot,pe))); pe-=size) {           \
            if (0==d) { pf-=size; A(pe!=pf); iswap(pe,pf,size,typeindex); S2; } \
        }                                                                \
        /* I11(J): loop */                                               \
    }                                                                    \
    pe = pc-size /* terminating semicolon after macro call */
#endif

/* Bentley&McIlroy partitioning algorithm w/ self-swapping guards as a macro */
/* use S1, S2 for pivot pointer update if required */
#if SWAP_FUNCTION
#define BM_PARTITION(S1,S2)                                                   \
    if (pivot==pc) pc+=size;                                        \
    else if (pivot==pe) pe-=size;                                   \
    for (pb=pc,pf=pe+size;;) { /* loop - partitioning */                                      \
        for (; (pc<=pe)&&(0<=(c=compar(pivot,pc))); pc+=size) {               \
            if (0==c) { if (pa!=pc) { swapf(pa,pc,size); S1; } pa+=size; } \
        }                                                                     \
        for (; (pc<=pe)&&(0>=(d=compar(pivot,pe))); pe-=size) {               \
            if (0==d) { pf-=size; if (pe!=pf) { swapf(pe,pf,size); S2; } } \
        }                                                                     \
        if (pc>pe) break;                                                     \
        if (pc!=pe) { swapf(pc,pe,size);          }                        \
        pc+=size; pe-=size;                                                   \
    }
#else
#define BM_PARTITION(S1,S2)                                                   \
    if (pivot==pc) pc+=size;                                                  \
    else if (pivot==pe) pe-=size;                                             \
    for (pb=pc,pf=pe+size;;) { /* loop - partitioning */                      \
        for (; (pc<=pe)&&(0<=(c=compar(pivot,pc))); pc+=size) {               \
            if (0==c) { if (pb!=pc) { iswap(pb,pc,size,typeindex); S1; } pb+=size; } \
        }                                                                     \
        for (; (pc<=pe)&&(0>=(d=compar(pivot,pe))); pe-=size) {               \
            if (0==d) { pf-=size; if (pe!=pf) { iswap(pe,pf,size,typeindex); S2; } } \
        }                                                                     \
        if (pc>pe) break;                                                     \
        if (pc!=pe) { iswap(pc,pe,size,typeindex); swaps++; }                        \
        pc+=size; pe-=size;                                                   \
    }
#endif

#if 1
    if ((1UL>swaps)||(0!=pivot_copy)) { /* no pivot update: compact loop */
        KIWIEL_L(/**/,/**/);
    } else { /* update pivot pointer when pivot is moved */
        KIWIEL_L(if(pc==pivot)pivot=pb,if(pe==pivot)pivot=pf);
    }
#else
    if ((1UL>swaps)||(0!=pivot_copy)) { /* no pivot update: compact loop */
        BM_PARTITION(/**/,/**/)
    } else { /* update pivot pointer when pivot is moved */
        BM_PARTITION(if(pc==pivot)pivot=pb,if(pe==pivot)pivot=pf)
    }
#endif
    pe+=size; /* now start of > region */
#if DEBUG_CODE
if ((2<debug)||(pe>pf)) {
    (V)fprintf(stderr, "// %s line %d: pivot=%p, pl=%p[%lu], pb=%p[%lu], pc=%p[%lu], pe=%p[%lu], pf=%p[%lu], pu=%p[%lu]\n// ",__func__,__LINE__,pivot,pl,first,pb,(pb-base)/size,pc,(pc-base)/size,pe,(pe-base)/size,pf,(pf-base)/size,pu,(pu-base)/size);
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
    A(pl<=pb);A(pb<=pc);/*A(pe<=pf);*/A(pf<=pu);A(pc<=pe);
#endif
    A(pb<=pe);
    A(pc==pe);
    /* I12: [Cleanup.] Set a := l + i - p and b := r - q + j. Exchange
       x[l: p - 1] <-> x[p: j] and x[i: q] <-> x[q + 1: r].
    */
    /* |   =   |   <              |              >   |   =   | */
    /*  pl      b                  e                  f       u*/
    A(pl<=pb);A(pb<=pe);
#if SWAP_FUNCTION
    pb=blockmove(pl,pb,pe,swapf);
#else
    pb=blockmove(pl,pb,pe,typeindex);
#endif
    A(pe<=pf);A(pf<=pu);
#if SWAP_FUNCTION
    pe=blockmove(pe,pf,pu,swapf);
#else
    pe=blockmove(pe,pf,pu,typeindex);
#endif
    /* |       <            |         =           |      >      | */
    /*  pl                   b                     e             u*/
    A(pl<=pb);A(pe<=pu);
#if DEBUG_CODE
if ((2<debug)&&(pb>=pe)) {
    (V)fprintf(stderr, "// %s line %d: pivot=%p[%lu], pl=%p[%lu], pb=%p, pe=%p, pu=%p\n// ",__func__,__LINE__,pivot,(pivot-base)/size,pl,(pl-base)/size,pb,pe,pu);
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
    A(pb<pe);
    if (NULL!=peq) *peq=(pb-base)/size;
    if (NULL!=pgt) *pgt=(pe-base)/size;
#if DEBUG_CODE
if ((3<debug)||((2<debug)&&(0UL<swaps))) {
    (V)fprintf(stderr, "// %s: line %d: nmemb %lu, swaps %lu, pb=%p[%lu], pe=%p[%lu]\n// ",__func__,__LINE__,beyond-first,swaps,pb,(pb-base)/size,pe,(pe-base)/size);
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
#if 0
fprintf(stderr, "%s line %d: pb=%p[%lu], pe=%p[%lu]\n// ",__func__,__LINE__,pb,(pb-base)/size,pe,(pe-base)/size);
print_int_array(base,first,beyond-1UL);
#endif
    return swaps;
}

#if SAVE_PARTIAL
/* repartition: like partition, but handle partially partitioned array */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void repartition(char *base, size_t first, size_t beyond, char *pc, char *pd,
    char *pivot, char *pe, char *pf, size_t size,
    int(*compar)(const void *,const void*),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    size_t *peq, size_t *pgt)
{
    char *pa, *pb, *pg, *ph, *pl, *pu;
    int c, d;
    size_t n, p;
#if SWAP_FUNCTION
#if ASSERT_CODE
    int typeindex = type_index(base,size);
#endif
#endif

    /* McGeoch & Tygar suggest that partial partition information
       from median-of-medians might be used to avoid recomparisons
       during repartitioning.
    */
    /* Skip over some already-partitioned sections. */
    /* Separate partitioning loop here to avoid overhead when not
       preserving partial partition. Subtleties involved: empty
       regions (pc-pd, pe-pf, pg-ph, pl-pa, pa-pb), whether pb or pg
       reaches the partially partitioned region first. No initial pivot swap
       because that would put an unknown status element in the already-
       partitioned region.
    */
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, pc@%lu, pd@%lu, pivot@%lu, pe@%lu, pf@%lu, beyond=%lu\n// ",__func__,__LINE__,first,(pc-base)/size,(pd-base)/size,(pivot-base)/size,(pe-base)/size,(pf-base)/size,beyond);
if (3==typeindex)
print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
/*
Kiwiel alg. L code; no initial test for pivot at pc or pg (not applicable here).
Final canonicalization not included.  Initialization would have to be performed
first. For repartitionig pc becomes pb, stage 1 should end w/o comparisons when
pb==pc (maybe then skip to pd, blockmove equals, then stop at pf).  Upper
pointer pg needs to stop w/o comparisons after pf (maybe skip to pe, blockmove,
stop at pd-size). Stage 2 loops also need to test for already-partitioned range.
*/
    /* +-----------------------------------------------------+ */
    /* |   =   |   <   |  ?  : < :=: > :  ?  |   >   |   =   | */
    /* +-----------------------------------------------------+ */
    /*  pl      a       b     c   d e   f   G g       h       u*/
    pb=pl=base+size*first, pu=base+size*beyond, pg=pu-size;
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
logfile=start_log("log");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/* |   =   |   <   |  ?  : < :=: > :  ?  |   >   |   =   | */\n");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/*  pl      a       b     c   d e   f   G g       h       u*/\n");
fprintf(logfile, "// %s line %d: pl=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,(pl-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    while ((pb<pc)&&(0==(c=compar(pivot,pb)))) { pb+=size; }
    pa=pb;
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pb over equals at left\n");
fprintf(logfile, "// %s line %d: c=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    A((pa==pl)||(0==compar(pivot,pa-size)));
    A(pl<=pa);A(pa==pb);A(pb<=pc);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=pu);
    if ((pb<pc)&&(0<c)) {
        for (pb+=size; (pb<pc)&&(0<=(c=compar(pivot,pb))); pb+=size) {
#if SWAP_FUNCTION
            if (0==c) { A(pa!=pb); swapf(pa,pb,size); pa+=size; }
#else
            if (0==c) { A(pa!=pb); iswap(pa,pb,size,typeindex); pa+=size; }
#endif
        }
    }
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pb over less-than and equals at left\n");
fprintf(logfile, "// %s line %d: c=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=pu);
    /* 2. Within partial partition; no recomparisons. */
    if ((pc<=pb)&&(pb<pf)) { /* partitioned */
        /* First skip over < elements, possibly past pg. */
        if (pd<=pg) { /* If pg didn't, move = block. */
            if (pb<pe) { /* move = elements as a block */
                A(pa<=pd);A(pd<=pe);
#if SWAP_FUNCTION
                pa=blockmove(pivot=pa,pd,pb=pe,swapf);
#else
                pa=blockmove(pivot=pa,pd,pb=pe,typeindex);
#endif
                pb=pe, c=-2;
            }
        } else pb=pg+size; /* < elements */
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pb over already-partitioned less-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    }
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=pu);
    while ((pb<=pg)&&((pb<pc)||(pf<=pb))&&(0<=(c=compar(pivot,pb)))) {
#if SWAP_FUNCTION
        if (0==c) { if (pa!=pb) swapf(pa,pb,size); pa+=size; }
#else
        if (0==c) { if (pa!=pb) iswap(pa,pb,size,typeindex); pa+=size; }
#endif
        pb+=size;
    }
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pb over less-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=pu);
    d=3;
    while ((pb<=pg)&&(pf<=pg)&&(0==(d=compar(pivot,pg)))) { pg-=size; }
    ph=pg+size;
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pg over equals at right\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    A(pb-size<=pg);A(pf-size<=pg);
    A((ph==pu)||(0==compar(pivot,ph)));
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
    if ((pb<=pf)&&(pf<=pg)&&(0>d)) {
        for (pg-=size; (pf<=pg)&&(0>=(d=compar(pivot,pg))); pg-=size) {
#if SWAP_FUNCTION
            if (0==d) { ph-=size; if (pg!=ph) swapf(pg,ph,size); }
#else
            if (0==d) { ph-=size; if (pg!=ph) iswap(pg,ph,size,typeindex); }
#endif
        }
    }
    /* check for already-partitioned region here (but no comparisons) */
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
char *px;
int e;
fprintf(logfile,"\n// skipped pg over greater-than and equals at right\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
# if ASSERT_CODE > 1
for (px=pg+size; px<ph; px+=size) {
    e=compar(pivot,px);
    (V)fprintf(logfile, "// %s line %d: compar(pivot=%lu,px=%lu)=%d\n",__func__,__LINE__,(pivot-base)/size,(px-base)/size,e);
    A(0>e);
}
# endif
}
#endif
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
    /* 4. Within partial partition; no recomparisons. */
    if ((pb<=pg)&&(pc<=pg)&&(pg<pf)) { /* partitioned */
        /* First skip over > elements, possibly past pb. */
        if (pb<pe) { /* If pb didn't, move = block. */
            if (pd<=pg) { /* move = elements as a block */
                A(pd<=pe);A(pe<=ph);
#if SWAP_FUNCTION
                pivot=ph=blockmove(pd,pe,ph,swapf); pg=pd-size; d=2;
#else
                pivot=ph=blockmove(pd,pe,ph,typeindex); pg=pd-size; d=2;
#endif
            }
        } else pg=pb-size; /* > elements */
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pg over already-partitioned greater-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    }
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
    while ((pb<=pg)&&((pf<=pg)||(pg<pc))&&(0>=(d=compar(pivot,pg)))) {
#if SWAP_FUNCTION
        if (0==d) { ph-=size; if (pg!=ph) swapf(pg,ph,size); }
#else
        if (0==d) { ph-=size; if (pg!=ph) iswap(pg,ph,size,typeindex); }
#endif
        pg-=size;
    }
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pg over greater-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
    while (pb<pg) {
#if ASSERT_CODE /* XXX */
if (((0<=c)||(0>=d))&&(REPARTITION_DEBUG_LEVEL<=debug)) {
fprintf(logfile,"\n// ERROR: pb,pg should be ready for swap\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
        A(0>c);A(0<d);
#if SWAP_FUNCTION
        swapf(pb,pg,size);
#else
        iswap(pb,pg,size,typeindex);
#endif
        pb+=size, pg-=size;
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// swapped pb,pg and updated pointers\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
        while ((pb<=pg)&&((pb<pc)||(pf<=pb))&&(0<=(c=compar(pivot,pb)))) {
#if SWAP_FUNCTION
            if (0==c) { if (pa!=pb) swapf(pa,pb,size); pa+=size; }
#else
            if (0==c) { if (pa!=pb) iswap(pa,pb,size,typeindex); pa+=size; }
#endif
            pb+=size;
        }
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pb over less-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
        A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
        /* 6. Within partial partition; no recomparisons. */
        if ((pc<=pb)&&(pb<pf)) { /* partitioned */
            /* First skip over < elements, possibly past pg. */
            if (pd<=pg) { /* If pg didn't, move = block. */
                if (pb<pe) { /* move = elements as a block */
                    A(pa<=pd);A(pd<=pe);
#if SWAP_FUNCTION
                    pa=blockmove(pivot=pa,pd,pb=pe,swapf);
#else
                    pa=blockmove(pivot=pa,pd,pb=pe,typeindex);
#endif
                    pb=pe, c=-2;
                }
            } else pb=pg+size; /* < elements */
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pb over already-partitioned less-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
        }
        A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
        while ((pb<=pg)&&((pf<=pg)||(pg<pc))&&(0>=(d=compar(pivot,pg)))) {
#if SWAP_FUNCTION
            if (0==d) { ph-=size; if (pg!=ph) swapf(pg,ph,size); }
#else
            if (0==d) { ph-=size; if (pg!=ph) iswap(pg,ph,size,typeindex); }
#endif
            pg-=size;
        }
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pg over greater-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
        A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
        /* 8. Within partial partition; no recomparisons. */
        if ((pc<=pg)&&(pg<pf)) { /* partitioned */
            /* First skip over > elements, possibly past pb. */
            if (pb<pe) { /* If pb didn't, move = block. */
                if (pd<=pg) { /* move = elements as a block */
                    A(pd<=pe);A(pe<=ph);
#if SWAP_FUNCTION
                    pivot=ph=blockmove(pd,pe,ph,swapf); pg=pd-size;
#else
                    pivot=ph=blockmove(pd,pe,ph,typeindex); pg=pd-size;
#endif
                    d=2;
                }
            } else pg=pb-size; /* > elements */
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// skipped pg over already-partitioned greater-than and equals\n");
fprintf(logfile, "// %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu\n// ",__func__,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,(ph-base)/size,(pu-base)/size,(pivot-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
        }
        A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
    }
    /* pb is start of > region */
    /* |    =  |      <             |             >    |   =    | */
    /*  pl      a                    b                  h        u*/
#if ASSERT_CODE /* XXX */
if ((pb<pa)||(pb>ph)) {
    (V)fprintf(stderr,
       "// %s line %d: pl=%p[%lu], pa=%p[%lu], pb=%p[%lu], pc=%p[%lu], pd=%p[%lu], pe=%p[%lu], pf=%p[%lu], pg=%p[%lu], ph=%p[%lu], pu=%p[%lu]\n// ",
       __func__,__LINE__,
       pl,(pl-base)/size, pa,(pa-base)/size, pb,(pb-base)/size,
       pc,(pc-base)/size, pd,(pd-base)/size, pe,(pe-base)/size,
       pf,(pf-base)/size, pg,(pg-base)/size, ph,(ph-base)/size,
       pu,(pu-base)/size
    );
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
    A(pl<=pa); A(pa<=pb); A(pb<=ph); A(ph<=pu);
#if SWAP_FUNCTION
    pc=blockmove(pl,pa,pb,swapf);
    pf=blockmove(pb,ph,pu,swapf);
#else
    pc=blockmove(pl,pa,pb,typeindex);
    pf=blockmove(pb,ph,pu,typeindex);
#endif
    A(pl<=pc); A(pc<pf); A(pf<=pu);
    /* |       <            |         =           |      >      | */
    /*  pl                   c                     f             u*/
#if ASSERT_CODE /* XXX */
if ((pf<=pc)||(pc<pl)) {
    (V)fprintf(stderr,
       "// %s line %d: pb=%p[%lu], pc=%p[%lu], pf=%p[%lu], pg=%p[%lu], pl=%p[%lu], pa=%p[%lu], ph=%p[%lu], pu=%p[%lu]\n// ",
       __func__,__LINE__,
       pb,(pb-base)/size,
       pc,(pc-base)/size,
       pf,(pf-base)/size,
       pg,(pg-base)/size,
       pl,(pl-base)/size,
       pa,(pa-base)/size,
       ph,(ph-base)/size,
       pu,(pu-base)/size
    );
if (3==typeindex)
    print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
if (REPARTITION_DEBUG_LEVEL<=debug) {
fprintf(logfile,"\n// blockmoves to restore canonical partition\n");
fprintf(logfile, "// %s line %d: pl=%lu, pc(pivot)=%lu, pf=%lu, pu=%lu\n// ",__func__,__LINE__,(pl-base)/size,(pc-base)/size,(pf-base)/size,(pu-base)/size);
if (3==typeindex) fprint_int_array(logfile,base,first,beyond-1UL);
else if (0==typeindex) fprint_double_array(logfile,base,first,beyond-1UL);
}
#endif
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, pc@%lu, pf@%lu\n// ",__func__,__LINE__,first,beyond,(pc-base)/size,(pf-base)/size);
if (3==typeindex)
print_int_array(base,first,beyond-1UL);
else if (0==typeindex)
    print_double_array(base,first,beyond-1UL);
}
#endif
#if ASSERT_CODE /* XXX */
if (REPARTITION_DEBUG_LEVEL<=debug) close_log(logfile);
#endif
    if (NULL!=peq) *peq=(pc-base)/size;
    if (NULL!=pgt) *pgt=(pf-base)/size;
}
#endif

/* Modified Bentley&McIlroy qsort:
      ternary median-of-3
      optional type-independent deferred pivot swap
      improved sampling quality for median-of-3, ninther
      recurse on small region, iterate large region
      avoid self-swapping pb,pc
      improved swapping sizes
      don't repeat initialization during iteration
      uses quickselect_small_array_cutoff rather than BM_INSERION_CUTOFF
      uses hybrid of network sort or insertion sort
   No remedian of samples (except equivalent to ninther)
   No break-glass mechanism to prevent quadratic behavior
   No order statistic selection
*/

void mbmqsort_internal(void *a, size_t n, size_t es,
    int (*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    unsigned int table_index,
    char *pv,
    size_t swaps)
{

    for (;;) {
        char *pe, *pm;
        size_t p, q, r;

        if (n <= quickselect_small_array_cutoff) {
            switch (n) {
                case 2UL :
#if NETWORK3
                case 3UL : /* already-sorted input slightly better w/ insertion sort */
#endif
#if NETWORK4
                case 4UL :  /* insertion sort 5% slower, only 1.6% lower average comparisons */
#endif
#if NETWORK5
                case 5UL : /* insertion sort reversed 0.86 @ 5 */
#endif
#if NETWORK6
                case 6UL : /* insertion sort reversed 0.97 @ 6 */
#endif
#if NETWORK7
                case 7UL : /* insertion sort reversed 1.07 @ 7 */
#endif
                case 8UL : /* insertion sort too costly */
                case 9UL : /* insertion sort too costly */
                case 10UL : /* insertion sort too costly */
#if SWAP_FUNCTION
                    networksort_internal(a,0UL,n,es,compar,swapf);
#else
                    networksort_internal(a,0UL,n,es,compar,typeindex);
#endif
                break;
                default :
#if SWAP_FUNCTION
                    isort_internal(a,0UL,n,es,compar,swapf);
#else
                    isort_internal(a,0UL,n,es,compar,typeindex);
#endif
                break;
            }
            return;
        }
        pm = select_pivot(a, 0UL, n, es, compar, table_index, NULL);
        swaps=partition(a,0UL,n,pm,es,compar,
#if SWAP_FUNCTION
            swapf,
#else
            typeindex,
#endif
            pv,0!=pivot_swap?0UL:swaps,&q,&p);
        npartitions++;
        pe=a+p*es;
        if (n>p) r=n-p; else r=0UL;  /* size of the > region */
        if (q<r) { /* > region is larger */
            if (1UL<q) {
                unsigned int idx=table_index;
                while (q<sorting_sampling_table[idx].min_nmemb)
                    idx--;
#if SWAP_FUNCTION
                mbmqsort_internal(a, q, es, compar, swapf, idx, pv, swaps+pivot_swap);
#else
                mbmqsort_internal(a, q, es, compar, typeindex, idx, pv, swaps+pivot_swap);
#endif
            }
            if (2UL>r) return;
            a=pe;
            n=r;
        } else { /* < region is larger, or regions are the same size */
            if (1UL<r) {
                unsigned int idx=table_index;
                while (r<sorting_sampling_table[idx].min_nmemb)
                    idx--;
#if SWAP_FUNCTION
                mbmqsort_internal(pe, r, es, compar, swapf, idx, pv, swaps+pivot_swap);
#else
                mbmqsort_internal(pe, r, es, compar, typeindex, idx, pv, swaps+pivot_swap);
#endif
            }
            if (2UL>q) return;
            n=q;
        }
        if (quickselect_small_array_cutoff<n)
            while (n<sorting_sampling_table[table_index].min_nmemb)
                table_index--;
    }
}

void mbmqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *))
{
    unsigned int table_index=2U; /* optimized for small nmemb */
    union aligned_union pv[1+(es-1UL)/sizeof(union aligned_union)];

    assert(a != NULL || n == 0 || es == 0);
    assert(compar != NULL);

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((table_index<22UL) /* sorting_sampling_table size */
    &&(n>sorting_sampling_table[table_index].min_nmemb)
    &&(9UL<sorting_sampling_table[table_index].samples) /* mbmqsort limited to max. 9 samples */
    )
        table_index++;
    while (n<sorting_sampling_table[table_index].min_nmemb)
        table_index--;
    A(table_index<22UL);

#if SWAP_FUNCTION
    mbmqsort_internal(a, n, es, compar, whichswap(a,es), table_index, pv, n);
#else
    mbmqsort_internal(a, n, es, compar, type_index(a,es), table_index, pv, n);
#endif
}

/* simplified and full-blown quickselect-based qsort:
   Like modified B&M qsort, except:
      add sampling and remedian of samples, with sampling table
      pivot selection moves elements
*/

/* simplified quickselect */
void sqsort_internal(void *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    unsigned int table_index,
    char *pv,
    size_t swaps)
{
    char *pivot;
    size_t nmemb, p, q, r, s;

    for (;;) {
        nmemb=beyond-first;
#if DEBUG_CODE
if (2<debug) (V) fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, nmemb=%lu, table_index=%u, swaps=%lu\n",__func__,__LINE__,first,beyond,nmemb,table_index,swaps);
#endif
        if (nmemb<=quickselect_small_array_cutoff) {
            switch (nmemb) {
                case 2UL :  /* insertion sort overhead too high */
#if NETWORK3
                case 3UL : /* already-sorted input slightly better w/ insertion sort */
#endif
#if NETWORK4
                case 4UL :  /* insertion sort 5% slower, only 1.6% lower average comparisons */
#endif
#if NETWORK5
                case 5UL : /* insertion sort reversed 0.86 @ 5 */
#endif
#if NETWORK6
                case 6UL : /* insertion sort reversed 0.97 @ 6 */
#endif
#if NETWORK7
                case 7UL : /* insertion sort reversed 1.07 @ 7 */
#endif
                case 8UL :  /* insertion sort too costly */
                case 9UL :  /* insertion sort too costly */
                case 10UL : /* insertion sort too costly */
#if SWAP_FUNCTION
                    networksort_internal(base,first,beyond,size,compar,swapf);
#else
                    networksort_internal(base,first,beyond,size,compar,typeindex);
#endif
                break;
                default :
#if SWAP_FUNCTION
                    isort_internal(base,first,beyond,size,compar,swapf);
#else
                    isort_internal(base,first,beyond,size,compar,typeindex);
#endif
                break;
            }
    /* <- */return; /* Done; */
        }
#if ASSERT_CODE
        if (nmemb<sorting_sampling_table[table_index].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: nmemb=%lu, table_index=%u, sorting_sampling_table[%u].min_nmemb=%lu\n",
                __func__,__LINE__,nmemb,table_index,table_index,
                sorting_sampling_table[table_index].min_nmemb);
        A(sorting_sampling_table[table_index].min_nmemb<=nmemb);
        A(sorting_sampling_table[table_index+1UL].min_nmemb>nmemb);
#endif
        pivot = select_pivot(base, first, beyond, size, compar, table_index, NULL);
#if SWAP_FUNCTION
        swaps=partition(base,first,beyond,pivot,size,compar,swapf,pv,
            0!=pivot_swap?0UL:swaps,&q,&p);
#else
        swaps=partition(base,first,beyond,pivot,size,compar,typeindex,pv,
            0!=pivot_swap?0UL:swaps,&q,&p);
#endif
        npartitions++;
        s=q-first;
        if (beyond>p) r=beyond-p; else r=0UL;  /* size of the > region */
        if (s<r) { /* > region is larger */
            if (1UL<s) {
                unsigned int idx=table_index;
                while (s<sorting_sampling_table[idx].min_nmemb) idx--;
#if SWAP_FUNCTION
                sqsort_internal(base, first, q, size, compar, swapf, idx, pv, swaps+pivot_swap);
#else
                sqsort_internal(base, first, q, size, compar, typeindex, idx, pv, swaps+pivot_swap);
#endif
            }
            if (2UL>r) return;
            first=p, nmemb=r;
        } else { /* < region is larger, or regions are the same size */
            if (1UL<r) {
                unsigned int idx=table_index;
                while (r<sorting_sampling_table[idx].min_nmemb) idx--;
#if SWAP_FUNCTION
                sqsort_internal(base, p, beyond, size, compar, swapf, idx, pv, swaps+pivot_swap);
#else
                sqsort_internal(base, p, beyond, size, compar, typeindex, idx, pv, swaps+pivot_swap);
#endif
            }
            if (2UL>s) return;
            beyond=q, nmemb=s;
        }
        if (quickselect_small_array_cutoff<nmemb)
            while (nmemb<sorting_sampling_table[table_index].min_nmemb) table_index--;
    }
}

void sqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    unsigned int table_index=2U; /* optimized for small nmemb */
    union aligned_union pv[1+(size-1UL)/sizeof(union aligned_union)];

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((table_index<22UL)
    &&(nmemb>sorting_sampling_table[table_index].min_nmemb))
        table_index++;
    while (nmemb<sorting_sampling_table[table_index].min_nmemb)
        table_index--;
    A(table_index<22UL);

#if SWAP_FUNCTION
    sqsort_internal(base,0UL,nmemb,size,compar,whichswap(base,size), table_index, pv, nmemb);
#else
    sqsort_internal(base,0UL,nmemb,size,compar,type_index(base,size), table_index, pv, nmemb);
#endif
}

/* modified quickselect sort; always get worst-case pivot for given
   repivot_factor and repivot_cutoff that would just avoid repivoting
*/
void wqsort_internal(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    unsigned int table_index,
    char *pv,
    size_t swaps)
{
    char *pl;  /* left (or lowest) end of array being processed */
    char *po;  /* origin of array (maintained so that ranks can be computed) */
    size_t s;  /* general size_t variable */
    int i;              /* general integer variables */
    int nlopsided=0;
    char *pb, *pe, *pivot;
    size_t first, p, q, r, t, karray[1];
#if 1
    double d, x;

    for (i=0,x=d=1.0+1.0/(double)repivot_factor; i<lopsided_partition_limit; i++,x*=d) ;
#endif

    pl=po=(char *)base;

#if 1
    s=repivot_cutoff*snlround(x+1.8,NULL,NULL);
#else
    s=repivot_cutoff+2UL*lopsided_partition_limit;
#endif

wloop: ;
    if (nmemb<=quickselect_small_array_cutoff) {
#if SWAP_FUNCTION
            if (0!=use_networks) networksort_internal(pl,0UL,nmemb,size,compar,swapf);
            else if (0!=use_shell) shellsort_internal(pl,0UL,nmemb,size,compar,swapf);
            else isort_internal(pl,0UL,nmemb,size,compar,swapf);
#else
        if (0!=use_networks) networksort_internal(pl,0UL,nmemb,size,compar,typeindex);
        else if (0!=use_shell) shellsort_internal(pl,0UL,nmemb,size,compar,typeindex);
        else isort_internal(pl,0UL,nmemb,size,compar,typeindex);
#endif
/* <- */return; /* Done; */
    }
    /* get pivot at rank that just avoids repivoting (minimum possible) */
    /* minimum possible rank is determined by number of samples used for pivot selection */
    /* 3^^p samples -> minimum rank is 2^^p-1 */
    for (p=0UL; p<22UL; p++)
        if (nmemb<sorting_sampling_table[p].min_nmemb)
            break; /* stop looking */
    if (0UL<p) p--; /* sampling table index */
    for (t=1UL,q=0UL; q<p; q++,t<<=1) ;
    /* XXX this does not take aux repivot parameters into account */
    if (nmemb<repivot_cutoff) {
        karray[0]=t-1UL; /* worst possible minimum rank */
    } else {
        if ((nlopsided<lopsided_partition_limit)&&(nmemb<=s)) {
            karray[0]=t-1UL; /* worst possible minimum rank */
            nlopsided++;
        } else {
            karray[0]=(nmemb-1UL)/(repivot_factor+1UL)+1UL;
            if (karray[0]<t-1UL) karray[0]=t-1UL;
        }
    }
    /* normal pivot selection (for comparison and swap counts) */
    first=(pl-po)/size;
    pivot = select_pivot(po, first, first+nmemb, size, compar, table_index, NULL);
    /* don't count comparisons or swaps for pivot selection here */
    p=neq, q=nlt, r=ngt, t=nsw;
    quickselect(pl, nmemb, size, compar, karray, 1UL);
    pivot=pl+size*karray[0];
    /* restore comparison and swap counts */
    neq=p, nlt=q, ngt=r, nsw=t;
#if SWAP_FUNCTION
    swaps=partition(po,first,first+nmemb,pivot,size,compar,swapf,pv,
        0!=pivot_swap?0UL:swaps,&q,&p);
#else
    swaps=partition(po,first,first+nmemb,pivot,size,compar,typeindex,pv,
        0!=pivot_swap?0UL:swaps,&q,&p);
#endif
    npartitions++;
    pb=po+q*size;
    pe=po+p*size;
    q-=first;
    if (first+nmemb>p) r=first+nmemb-p; else r=0UL;  /* size of the > region */
    /* |       <            |         =           |      >      | */
    /*  pl                   b                     e             u*/
    if (q<r) { /* > region is larger */
        if (1UL<q) {
            unsigned int idx=table_index;
            while (q<sorting_sampling_table[idx].min_nmemb)
                idx--;
#if SWAP_FUNCTION
            wqsort_internal(pl, q, size, compar, swapf,idx, pv,swaps+pivot_swap);
#else
            wqsort_internal(pl, q, size, compar, typeindex,idx, pv,swaps+pivot_swap);
#endif
        }
        if (r>1UL) {
            pl=pe;
            nmemb=r;
            if (quickselect_small_array_cutoff<nmemb)
                while (nmemb<sorting_sampling_table[table_index].min_nmemb)
                    table_index--;
            goto wloop;
        }
    } else { /* < region is larger, or regions are the same size */
        if (1UL<r) {
            unsigned int idx=table_index;
            while (r<sorting_sampling_table[idx].min_nmemb)
                idx--;
#if SWAP_FUNCTION
            wqsort_internal(pe, r, size, compar, swapf,idx, pv,swaps+pivot_swap);
#else
            wqsort_internal(pe, r, size, compar, typeindex,idx, pv,swaps+pivot_swap);
#endif
        }
        if (q>1UL) {
            nmemb=q;
            if (quickselect_small_array_cutoff<nmemb)
                while (nmemb<sorting_sampling_table[table_index].min_nmemb)
                    table_index--;
            goto wloop;
        }
    }
}

void wqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    unsigned int table_index=2U; /* optimized for small nmemb */
    union aligned_union pv[1+(size-1UL)/sizeof(union aligned_union)];

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((table_index<22UL)
    &&(nmemb>sorting_sampling_table[table_index].min_nmemb))
        table_index++;
    while (nmemb<sorting_sampling_table[table_index].min_nmemb)
        table_index--;
    A(table_index<22UL);

#if SWAP_FUNCTION
    wqsort_internal(base,nmemb,size,compar,whichswap(base,size),table_index, pv,nmemb);
#else
    wqsort_internal(base,nmemb,size,compar,type_index(base,size),table_index, pv,nmemb);
#endif
}

/* full-blown quickselect */
#define NREGIONS (LONG_BIT)+8

/* basic quickselect loop (using system stack) */
/* modular version of inlined code in quickselect.c */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_loop(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *),
    const size_t *pk, size_t firstk, size_t beyondk,
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    struct sampling_table_struct *pst, unsigned int table_index,
    char *pv, size_t swaps, int nr
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
   )
{
    size_t lk=beyondk, nmemb=beyond-first, rk=firstk, stsz;
    auto int c=nr;
    int do_repivot;
    size_t oldc, olds;

#if DEBUG_CODE
if (2<debug) (V) fprintf(stderr,"// %s line %d: first=%lu, beyond=%lu, pk=%p, firstk=%lu, beyondk=%lu, swaps=%lu, nr=%d\n",__func__,__LINE__,first,beyond,pk,firstk,beyondk,swaps,nr);
#endif
    while (((NULL!=pk)
#if SAVE_PARTIAL
          &&((0U!=save_partial)||(nmemb>5UL)) /* dedicated sort for nmemb <= 5 for selection */
#else
          &&(nmemb>5UL) /* dedicated sort for nmemb <= 5 for selection */
#endif
    )||(nmemb>quickselect_small_array_cutoff)) {
        size_t nk, p, q, r, s, t;
        char *pivot;
        struct sampling_table_struct *npst;
        unsigned int srt, d;

#if ASSERT_CODE
        if (nmemb<pst[table_index].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: nmemb=%lu, table_index=%u, pst[%u].min_nmemb=%lu\n",
                __func__,__LINE__,nmemb,table_index,table_index,
                pst[table_index].min_nmemb);
        A(pst[table_index].min_nmemb<=nmemb);
        if (nmemb>=pst[table_index+1UL].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: nmemb=%lu, table_index=%u, pst[%u].min_nmemb=%lu\n",
                __func__,__LINE__,nmemb,table_index,table_index+1UL,
                pst[table_index+1UL].min_nmemb);
        A(pst[table_index+1UL].min_nmemb>nmemb);
#endif
        pivot=select_pivot(base,first,beyond,size,compar,table_index, pk);
#if ASSERT_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr, "// %s line %d: pivot=%p(%d), pl=%p, pu=%p\n// ",__func__,__LINE__,pivot,*((int *)pivot),base+first*size,base+beyond*size);
    print_int_array(base,first,beyond-1UL);
}
#endif
        A(base+first*size<=pivot);A(pivot<base+beyond*size);
#if SWAP_FUNCTION
        swaps=partition(base,first,beyond,pivot,size,compar,swapf,pv,
            0!=pivot_swap?0UL:swaps,&p,&q);
#else
        swaps=partition(base,first,beyond,pivot,size,compar,typeindex,pv,
            0!=pivot_swap?0UL:swaps,&p,&q);
#endif
check_sizes: ;
        npartitions++;
#if ASSERT_CODE
if (p>=q) {
    (V)fprintf(stderr, "%s line %d: beyond=%lu, first=%lu, beyond-first=%lu, nmemb=%lu, q=%lu, p=%lu, q-p=%lu, pk=%p, s=%lu, t=%lu, q-p+s+t=%lu\n",__func__,__LINE__,beyond,first,beyond-first,nmemb,q,p,q-p,pk,s,t,q-p+s+t);
}
#endif
        /* sizes of < and > regions (elements) */
        s=p-first, t=beyond-q;
#if ASSERT_CODE
if (q-p+s+t!=nmemb) {
    /* should always have pivot plus at least one other element excluded from large region */
    (V)fprintf(stderr, "%s line %d: beyond=%lu, first=%lu, beyond-first=%lu, nmemb=%lu, q=%lu, p=%lu, q-p=%lu, pk=%p, s=%lu, t=%lu, q-p+s+t=%lu\n",__func__,__LINE__,beyond,first,beyond-first,nmemb,q,p,q-p,pk,s,t,q-p+s+t);
}
#endif
        A(q-p+s+t==nmemb);
        /* revised range of order statistic ranks */
        /* < region indices [first,p), order statistics [firstk,lk) */
        /* > region indices [q,beyond), order statistics [rk,beyondk) */
        if (NULL!=pk) { /* selection only */
            klimits(p,q,pk,firstk,beyondk,&lk,&rk);
            A((lk==beyondk)||(pk[lk]>=p));A((rk==beyondk)||(pk[rk]>=q));
#if SAVE_PARTIAL
            if (0U!=save_partial) {
                /* only applicable for median-of-medians */
                if ((0UL==firstk)&&(1UL==beyondk)&&(NULL!=ppeq)&&(NULL!=ppgt)) {
                    int set=0;
                    A(p<q);
                    /* Update range pointers if median (of medians) is [in] one
                       of 3 regions.
                    */
                    r=pk[0];
                    if ((p<=r)&&(r<q)) *ppeq=base+size*p, *ppgt=base+size*q, set++;
                    else if ((1UL==s)&&(first<=r)&&(r<p))
                        *ppeq=base+size*first, *ppgt=*ppeq+size, set++;
                    else if ((1UL==t)&&(q<=r)&&(r<beyond))
                        *ppgt=base+size*beyond, *ppeq=*ppgt-size, set++;
# if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) if (0!=set) (V)fprintf(stderr, "// %s line %d: *ppeq@%lu, *ppgt@%lu, pk[0]=%lu\n",__func__,__LINE__,(*ppeq-base)/size,(*ppgt-base)/size,r);
# endif
                }
            }
#endif
        }
        A(lk>=firstk);A(rk<=beyondk);A(lk<=rk);

        if (s<t) { /* > region is larger */
            /* recurse on small region, if effective size > 1 */
            if ((1UL<s)&&((NULL==pk)||(lk>firstk))) {
                unsigned int idx=table_index;
                npst=sampling_table(first,beyond,pk,firstk,lk,
#if SAVE_PARTIAL
                    ppeq,ppgt,
#endif
                    &srt,&d,&stsz);
                if (idx>=stsz) idx=stsz-1UL;
                else
                    while (s>npst[idx].min_nmemb) idx++;
                while (s<npst[idx].min_nmemb) idx--;
#if ASSERT_CODE
                if (s<npst[idx].min_nmemb)
                    (V)fprintf(stderr,
                        "%s line %d: s=%lu, idx=%u, npst[%u].min_nmemb=%lu\n",
                        __func__,__LINE__,s,idx,idx,
                        npst[idx].min_nmemb);
                A(npst[idx].min_nmemb<=s);
                if (s>=npst[idx+1UL].min_nmemb)
                    (V)fprintf(stderr,
                        "%s line %d: s=%lu, idx=%u, npst[%u].min_nmemb=%lu\n",
                        __func__,__LINE__,s,idx,idx+1UL,
                        npst[idx+1UL].min_nmemb);
                A(npst[idx+1UL].min_nmemb>s);
#endif
                /* sort if nmemb < 5 * (lk-firstk) */
                /* sort if nmemb-lk+firstk < 4 * (lk-firstk) */
                /* sort if nmemb-lk+firstk / 4 < (lk-firstk) */
                /* sort if ((nmemb-lk+firstk)>>2) < (lk-firstk) */
                /* but no sort if finding median of medians with SAVE_PARTIAL */
                if (0U!=srt) {
                    quickselect_loop(base,first,p,size,compar,NULL,0UL,0UL,
#if SWAP_FUNCTION
                        swapf
#else
                        typeindex
#endif
                        ,npst,idx,pv,swaps+pivot_swap,0
#if SAVE_PARTIAL
                        ,ppeq,ppgt
#endif
                    );
                } else {
                    quickselect_loop(base,first,p,size,compar,pk,firstk,lk,
#if SWAP_FUNCTION
                        swapf
#else
                        typeindex
#endif
                        ,npst,idx,pv,swaps+pivot_swap,0
#if SAVE_PARTIAL
                        ,ppeq,ppgt
#endif
                    );
                }
            }
            /* large region [q,beyond) size t, order statistics [rk,beyondk) */
            first=q, r=t, firstk=rk;
        } else { /* < region is larger, or regions are same size */
            /* recurse on small region, if effective size > 1 */
            if ((1UL<t)&&((NULL==pk)||(beyondk>rk))) {
                unsigned int idx=table_index;
                npst=sampling_table(first,beyond,pk,rk,beyondk,
#if SAVE_PARTIAL
                    ppeq,ppgt,
#endif
                    &srt,&d,&stsz);
                if (idx>=stsz) idx=stsz-1UL;
                else
                    while (t>npst[idx].min_nmemb) idx++;
                while (t<npst[idx].min_nmemb) idx--;
#if ASSERT_CODE
                if (t<npst[idx].min_nmemb)
                    (V)fprintf(stderr,
                        "%s line %d: t=%lu, idx=%u, npst[%u].min_nmemb=%lu\n",
                        __func__,__LINE__,t,idx,idx,
                        npst[idx].min_nmemb);
                A(npst[idx].min_nmemb<=t);
                if (t>=npst[idx+1UL].min_nmemb)
                    (V)fprintf(stderr,
                        "%s line %d: t=%lu, idx=%u, npst[%u].min_nmemb=%lu\n",
                        __func__,__LINE__,t,idx,idx+1UL,
                        npst[idx+1UL].min_nmemb);
                A(npst[idx+1UL].min_nmemb>t);
#endif
                /* sort if nmemb < 5 * (beyondk-rk) */
                /* sort if nmemb-beyondk+rk < 4 * (beyondk-rk) */
                /* sort if nmemb-beyondk+rk / 4 < (beyondk-rk) */
                /* sort if ((nmemb-beyondk+rk)>>2) < (beyondk-rk) */
                /* but no sort if finding median of medians with SAVE_PARTIAL */
                if (0U!=srt) {
                    quickselect_loop(base,q,beyond,size,compar,NULL,0UL,0UL,
#if SWAP_FUNCTION
                        swapf
#else
                        typeindex
#endif
                        ,npst,idx,pv,swaps+pivot_swap,0
#if SAVE_PARTIAL
                        ,ppeq,ppgt
#endif
                    );
                } else {
                    quickselect_loop(base,q,beyond,size,compar,pk,rk,beyondk,
#if SWAP_FUNCTION
                        swapf
#else
                        typeindex
#endif
                        ,npst,idx,pv,swaps+pivot_swap,0
#if SAVE_PARTIAL
                        ,ppeq,ppgt
#endif
                    );
                }
            }
            /* large region [first,p) size s, order statistics [firstk,lk) */
            beyond=p, r=s, beyondk=lk;
        }
        /* iterate on large region (size r), if effective size > 1 */
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) (V)fprintf(stderr,"// %s line %d: p=%lu, q=%lu, first=%lu, beyond=%lu, r=%lu, firstk=%lu, beyondk=%lu: %s\n",__func__,__LINE__,p,q,first,beyond,r,firstk,beyondk,((2UL>r)||((NULL!=pk)&&(firstk>=beyondk)))?"done":"continuing");
#endif
        if (1<debug) { /* for graphing ratios during sorting */
            A(nmemb>r);
            (V)fprintf(stderr, "// %s line %d: large region from partition of nmemb=%lu, s=%lu, e=%lu, t=%lu, ratio=%lu\n", __func__, __LINE__, nmemb, s, q-p, t, r/(nmemb-r));
        }
        if ((2UL>r)||((NULL!=pk)&&(firstk>=beyondk))) return;
        /* sort if nmemb < 5 * (beyondk-rk) */
        /* sort if nmemb-beyondk+rk < 4 * (beyondk-rk) */
        /* sort if nmemb-beyondk+rk / 4 < (beyondk-rk) */
        /* sort if ((nmemb-beyondk+rk)>>2) < (beyondk-rk) */
        /* but no sort if finding median of medians with SAVE_PARTIAL */
        pst=sampling_table(first,beyond,pk,firstk,beyondk,
#if SAVE_PARTIAL
            ppeq,ppgt,
#endif
            &srt,&d,&stsz);
        /* Update table index for region size. Updated index is
           required for sampling if not repivoting, and for next round of
           repivoting decisions whether or not this region is repivoted.
        */
        if (table_index>=stsz) table_index=stsz-1UL;
        else
            while (r>pst[table_index].min_nmemb) table_index++;
        while (r<pst[table_index].min_nmemb) table_index--;
#if ASSERT_CODE
        if (r<pst[table_index].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: r=%lu, table_index=%u, pst[%u].min_nmemb=%lu\n",
                __func__,__LINE__,r,table_index,table_index,
                pst[table_index].min_nmemb);
        A(pst[table_index].min_nmemb<=r);
        if (r>=pst[table_index+1UL].min_nmemb)
            (V)fprintf(stderr,
                "%s line %d: r=%lu, table_index=%u, pst[%u].min_nmemb=%lu\n",
                __func__,__LINE__,r,table_index,table_index+1UL,
                pst[table_index+1UL].min_nmemb);
        A(pst[table_index+1UL].min_nmemb>r);
#endif
        A(table_index<stsz);
        if (0U!=srt) {
            pk=NULL, lk=rk=firstk=beyondk=0UL;
        }
        /* should large region be repivoted? */
#if ASSERT_CODE
if (r>=nmemb) {
    /* should always have pivot plus at least one other element excluded from large region */
    (V)fprintf(stderr, "// %s line %d: nmemb=%lu, q=%lu, p=%lu, q-p=%lu, pk=%p, s=%lu, t=%lu, q-p+s+t=%lu\n",__func__,__LINE__,nmemb,q,p,q-p,pk,s,t,q-p+s+t);
}
#endif
        do_repivot=should_repivot(nmemb,r,table_index,pk,&c);
#if DEBUG_CODE
if ((0!=do_repivot)&&(2<debug)) (V) fprintf(stderr,"// %s line %d: repivoting large region %lu / %lu, first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu\n",__func__,__LINE__,r,nmemb,first,beyond,firstk,beyondk);
#endif
        nmemb=r;
        if(0!=do_repivot) {
            char *pc, *pd, *pe, *pf;

            /* yes: do it and check resulting sizes */
#if SWAP_FUNCTION
            pivot=repivot(base,first,beyond,size,compar,swapf,pst,table_index,
                pk,pv,swaps+pivot_swap,&pc,&pd,&pe,&pf);
#else
            pivot=repivot(base,first,beyond,size,compar,typeindex,pst,table_index,
                pk,pv,swaps+pivot_swap,&pc,&pd,&pe,&pf);
#endif
            A(pc<=pd);A(pd<pe);A(pe<=pf);
            nrepivot++;
#if ASSERT_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr, "// %s line %d: pivot=%p(%d), pl=%p, pc=%p, pe=%p, pu=%p\n// ",__func__,__LINE__,pivot,*((int *)pivot),base+first*size,pc,pe,base+beyond*size);
    print_int_array(base,first,beyond-1UL);
}
#endif
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, pc@%lu, pd@%lu, pivot@%lu, pe@%lu, pf@%lu, beyond=%lu\n// ",__func__,__LINE__,first,(pc-base)/size,(pd-base)/size,(pivot-base)/size,(pe-base)/size,(pf-base)/size,beyond);
print_int_array(base,first,beyond-1UL);
}
#endif
            A(pd<=pivot);A(pivot<pe);
#if SAVE_PARTIAL
            if (0U!=save_partial) {
                repartition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,
# if SWAP_FUNCTION
                    swapf
# else
                    typeindex
# endif
                    ,&p,&q);
            } else {
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, pc@%lu, pd@%lu, pivot@%lu, pe@%lu, pf@%lu, beyond=%lu, save_partial=%u\n// ",__func__,__LINE__,first,(pc-base)/size,(pd-base)/size,(pivot-base)/size,(pe-base)/size,(pf-base)/size,beyond,save_partial);
print_int_array(base,first,beyond-1UL);
}
#endif
# if SWAP_FUNCTION
                swaps=partition(base,first,beyond,pivot,size,compar,swapf,pv,
                    0!=pivot_swap?0UL:swaps,&p,&q);
# else
                swaps=partition(base,first,beyond,pivot,size,compar,typeindex,pv,
                    0!=pivot_swap?0UL:swaps,&p,&q);
# endif
            }
#else
#if DEBUG_CODE
if (REPARTITION_DEBUG_LEVEL<=debug) {
(V)fprintf(stderr,"// %s line %d: first=%lu, pc@%lu, pd@%lu, pivot@%lu, pe@%lu, pf@%lu, beyond=%lu, SAVE_PARTIAL=%u\n// ",__func__,__LINE__,first,(pc-base)/size,(pd-base)/size,(pivot-base)/size,(pe-base)/size,(pf-base)/size,beyond,SAVE_PARTIAL);
print_int_array(base,first,beyond-1UL);
}
#endif
# if SWAP_FUNCTION
            swaps=partition(base,first,beyond,pivot,size,compar,swapf,pv,
                0!=pivot_swap?0UL:swaps,&p,&q);
# else
            swaps=partition(base,first,beyond,pivot,size,compar,typeindex,pv,
                0!=pivot_swap?0UL:swaps,&p,&q);
# endif
#endif
#if ASSERT_CODE /* for debugging of broken repivot/repartition */
if (p>=q) {
    (V)fprintf(stderr, "%s line %d: beyond=%lu, first=%lu, beyond-first=%lu, nmemb=%lu, q=%lu, p=%lu, q-p=%lu, pk=%p, s=%lu, t=%lu, q-p+s+t=%lu\n",__func__,__LINE__,beyond,first,beyond-first,nmemb,q,p,q-p,pk,s,t,q-p+s+t);
}
/* sizes of < and > regions (elements) */
s=p-first, t=beyond-q;
if (q-p+s+t!=nmemb) {
    /* should always have pivot plus at least one other element excluded from large region */
    (V)fprintf(stderr, "%s line %d: beyond=%lu, first=%lu, beyond-first=%lu, nmemb=%lu, q=%lu, p=%lu, q-p=%lu, pk=%p, s=%lu, t=%lu, q-p+s+t=%lu\n",__func__,__LINE__,beyond,first,beyond-first,nmemb,q,p,q-p,pk,s,t,q-p+s+t);
}
if (s>t) r=s; else r=t;
if (r/(nmemb-r)>3) {
(V)fprintf(stderr, "%s line %d: repivot/repartition failed (r/(nmemb-r) is too large): nmemb=%lu, first=%lu, beyond=%lu, p=%lu, q=%lu, s=%lu, e=%lu, t=%lu, r=%lu\n",__func__,__LINE__,nmemb,first,beyond,p,q,s,q-p,t,r);
    print_int_array(base,first,beyond-1UL);
    A(1<0);
}
#endif
            c=0; /* clean slate */
            goto check_sizes;
        }
        /* no repivot: just process large region as usual */
    }
    if (0U<do_histogram) {
        oldc=nlt+neq+ngt, olds=nsw;
        if (0!=use_networks) {
#if SWAP_FUNCTION
            networksort_internal(base,first,beyond,size,compar,swapf);
#else
            networksort_internal(base,first,beyond,size,compar,typeindex);
#endif
        } else {
#if SWAP_FUNCTION
            isort_internal(base,first,beyond,size,compar,swapf);
#else
            isort_internal(base,first,beyond,size,compar,typeindex);
#endif
        }
        if (nmemb>=HISTOGRAM_INDEX1_OFFSET) {
            oldc=nlt+neq+ngt-oldc; /* added comparisons (insertion sort) */
            histogram[nmemb-HISTOGRAM_INDEX1_OFFSET][oldc+1UL-nmemb]++;
            olds=nsw-olds;
            histoswap[nmemb-HISTOGRAM_INDEX1_OFFSET]+=olds;
        }
        return;
    }
    switch (nmemb) {
        case 2UL :  /* insertion sort overhead too high */
#if NETWORK3
        case 3UL : /* already-sorted input slightly better w/ insertion sort */
#endif
#if NETWORK4
        case 4UL :  /* insertion sort 5% slower, only 1.6% lower average comparisons */
#endif
#if NETWORK5
        case 5UL : /* insertion sort reversed 0.86 @ 5 */
#endif
#if NETWORK6
        case 6UL : /* insertion sort reversed 0.97 @ 6 */
#endif
#if NETWORK7
        case 7UL : /* insertion sort reversed 1.07 @ 7 */
#endif
        case 8UL :  /* insertion sort too costly */
        case 9UL :  /* insertion sort too costly */
        case 10UL : /* insertion sort too costly */
#if SWAP_FUNCTION
            networksort_internal(base,first,beyond,size,compar,swapf);
#else
            networksort_internal(base,first,beyond,size,compar,typeindex);
#endif
        break;
        default :
#if SWAP_FUNCTION
            isort_internal(base,first,beyond,size,compar,swapf);
#else
            isort_internal(base,first,beyond,size,compar,typeindex);
#endif
        break;
    }
}

/* comparison function for sorting order statistic ranks */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int size_t_cmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const size_t a = *((const size_t *)p1), b = *((const size_t *)p2);

        if (a > b) return 1;
        if (a < b) return -1;
    }
    return 0;
}

/* reverse the order of elements in p between l and r inclusive */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void reverse_size_t(size_t *p, size_t l, size_t r)
{
    size_t t;

    for (;l<r; l++,r--) {
        t=p[l]; p[l]=p[r]; p[r]=t;
    }
}

/* rotate the elements in p (n elements) to the left by i positions */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void rotate_size_t(size_t *p, size_t n, size_t i)
{
#if 0 /* no need to rotate duplicates */
    reverse_size_t(p, 0UL, i-1UL);
#endif
    reverse_size_t(p, i, n-1UL);
    reverse_size_t(p, 0UL, n-1UL);
}

/* quickselect definition (internal interface) */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_internal(char *base, size_t nmemb,
    const size_t size, int (*compar)(const void *,const void *),
    size_t *pk, size_t nk
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
   )
{
    union aligned_union pv[1+(size-1UL)/sizeof(union aligned_union)];
    unsigned int d, srt;
    unsigned int table_index=2U; /* optimized for small nmemb */
    size_t onk=nk, p, q, r=0UL, stsz;
    struct sampling_table_struct *pst;

    if (2UL>nmemb) return; /* Return early if there's nothing to do. */

    /* Make sure pk,nk are consistent; order statistics are sorted, unique. */
    if (0UL==nk) pk=NULL;
    else if (NULL==pk) onk=nk=0UL;
    else if (1UL<nk) {
        for (p=0UL,q=1UL; q<nk; p++,q++) /* verify sorted */
            if (pk[p]>pk[q]) break; /* not sorted */
        if (q<nk) /* fix (sort) iff broken (not sorted) */
            quickselect_internal((void *)pk,nk,sizeof(size_t),size_t_cmp,NULL,
                0UL
#if SAVE_PARTIAL
                ,NULL,NULL
#endif
            );
        /* verify, fix uniqueness */
        for (p=0UL,q=1UL; q<=nk; q++) {
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    r=q-p;
if (2<debug) { (V)fprintf(stderr, "%s line %d: %lu duplicate order statistic%s [%lu,%lu):\n",__func__,__LINE__,r,r==1?"":"s",p,q);
print_size_t_array(pk, 0UL, nk-1UL);
}
                    rotate_size_t(&(pk[p]),nk-p,r);
                    nk-=r;
if (2<debug) { (V)fprintf(stderr, "%s line %d: duplicate order statistic%s rotated to end:\n",__func__,__LINE__,r==1?"":"s");
print_size_t_array(pk, 0UL, nk-1UL);
}
                    q=p;
                }
            }
        }
    }

    pst=sampling_table(0UL,nmemb,pk,0UL,nk,
#if SAVE_PARTIAL
        ppeq,ppgt,
#endif
        &srt,&d,&stsz);
if (0<debug) (V)fprintf(stderr, "// %s line %d: d=%u\n",__func__,__LINE__,d);
if (0<debug) (V)fprintf(stderr, "// %s line %d: %s\n",__func__,__LINE__,0U!=srt?"sort":"select");
if (0<debug) (V)fprintf(stderr, "// %s line %d: %s sampling table\n",__func__,__LINE__,(pst==middle_sampling_table)?"middle":(pst==separated_sampling_table)?"separated":"sorting");
    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smaller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    if (table_index>=stsz) table_index=stsz-1UL;
    else
        while (nmemb>pst[table_index].min_nmemb) table_index++;
    while (nmemb<pst[table_index].min_nmemb) table_index--;
    A(table_index<stsz);
if (0<debug) (V)fprintf(stderr, "// %s line %d: nmemb=%lu, min_nmemb=%lu, %lu samples\n",__func__,__LINE__,nmemb,pst[table_index].min_nmemb,pst[table_index].samples);

    if (0<debug) { /* for graphing worst-case partition ratios */
        for (q=0UL; q<stsz; q++)
            stats_table[q].max_ratio=stats_table[q].repivot_ratio=1UL,
                stats_table[q].repivots=0UL;
    }
    if (0U!=srt) {
        quickselect_loop(base,0UL,nmemb,size,compar,NULL,0UL,0UL,
#if SWAP_FUNCTION
            whichswap(base,size),pst,table_index,pv,nmemb,0
#else
            type_index(base,size),pst,table_index,pv,nmemb,0
#endif
#if SAVE_PARTIAL
            ,ppeq,ppgt
#endif
        );
    } else {
        quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,
#if SWAP_FUNCTION
            whichswap(base,size),pst,table_index,pv,nmemb,0
#else
            type_index(base,size),pst,table_index,pv,nmemb,0
#endif
#if SAVE_PARTIAL
            ,ppeq,ppgt
#endif
        );
    }
    if (0<debug) { /* for graphing worst-case partition ratios */
/* XXX because of switching among tables, min_nmemb in statistics output may be meaningless */
        for (q=0UL; q<stsz; q++) {
            if (1UL<stats_table[q].max_ratio)
                (V)fprintf(stderr,
                    "%s: min_nmemb=%lu, samples=%lu, max_ratio=%lu, repivots=%lu@%lu\n",
                    __func__,pst[q].min_nmemb,
                    pst[q].samples,stats_table[q].max_ratio,
                    stats_table[q].repivots,
                    stats_table[q].repivot_ratio);
        }
    }
    /* Restore pk to full sorted (non-unique) order. */
    if (0UL!=r) { /* there were duplicates */
        quickselect_internal((void *)pk,onk,sizeof(size_t),size_t_cmp,NULL,
            0UL
#if SAVE_PARTIAL
            ,NULL,NULL
#endif
        );
if (2<debug) { (V)fprintf(stderr, "%s line %d: full order statistics array:\n",__func__,__LINE__);
print_size_t_array(pk, 0UL, onk-1UL);
}
    }
}


/* dual-pivot sorting */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void sort5(char *pa, char *pb, char *pc, char *pd, char *pe, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    /* Batcher sorting network: 9 comparisons, 0-9 exchanges */
#if SWAP_FUNCTION
    if (0>compar(pa,pe)) swapf(pa,pe,size);
    if (0>compar(pa,pc)) swapf(pa,pc,size);
    if (0>compar(pb,pd)) swapf(pb,pd,size);
    if (0>compar(pa,pb)) swapf(pa,pb,size);
    if (0>compar(pc,pe)) swapf(pc,pe,size);
    if (0>compar(pc,pd)) swapf(pc,pd,size);
    if (0>compar(pb,pe)) swapf(pb,pe,size);
    if (0>compar(pb,pc)) swapf(pb,pc,size);
    if (0>compar(pd,pe)) swapf(pd,pe,size);
#else
    if (0>compar(pa,pe)) iswap(pa,pe,size,typeindex);
    if (0>compar(pa,pc)) iswap(pa,pc,size,typeindex);
    if (0>compar(pb,pd)) iswap(pb,pd,size,typeindex);
    if (0>compar(pa,pb)) iswap(pa,pb,size,typeindex);
    if (0>compar(pc,pe)) iswap(pc,pe,size,typeindex);
    if (0>compar(pc,pd)) iswap(pc,pd,size,typeindex);
    if (0>compar(pb,pe)) iswap(pb,pe,size,typeindex);
    if (0>compar(pb,pc)) iswap(pb,pc,size,typeindex);
    if (0>compar(pd,pe)) iswap(pd,pe,size,typeindex);
#endif
}

static size_t dp_cutoff = 26UL; /* 26 seems to be optimal */

/* dual-pivot qsort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void dpqsort_internal(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t))
#else
    int typeindex)
#endif
{
    char *pa, *pb, *pivot1, *pivot2, *pm, *pn, *ps, *pt, *pu;
    size_t n, q, r, s, t, u, v, w, karray[2];

#define DPDEBUG 0
/* SAMPLE_SELECTION 0: use sort of 5 elements; 1: use quickselect to obtain pivots from desired ranks */
#define SAMPLE_SELECTION 1
/* ADAPTIVE_SAMPLING 0: always use 5 samples; 1: use ~sqrt(nmemb) samples */
#define ADAPTIVE_SAMPLING 1
/* sanity */
#if SAMPLE_SELECTION == 0
# undef ADAPTIVE_SAMPLING
# define ADAPTIVE_SAMPLING SAMPLE_SELECTION
#endif
#if ADAPTIVE_SAMPLING != 0
# undef SAMPLE_SELECTION
# define SAMPLE_SELECTION ADAPTIVE_SAMPLING
#endif
    for (;;) {
        pu=base+(nmemb-1UL)*size; /* last element */
        if (nmemb<=dp_cutoff) { /* small-array sort */
#if SWAP_FUNCTION
            if (0!=use_networks) networksort_internal(base,0UL,nmemb,size,compar,swapf);
            else if (0!=use_shell) shellsort_internal(base,0UL,nmemb,size,compar,swapf);
            else isort_internal(base,0UL,nmemb,size,compar,swapf);
#else
            if (0!=use_networks) networksort_internal(base,0UL,nmemb,size,compar,typeindex);
            else if (0!=use_shell) shellsort_internal(base,0UL,nmemb,size,compar,typeindex);
            else isort_internal(base,0UL,nmemb,size,compar,typeindex);
#endif
    /* <- */return; /* Done; */
        }
#if ADAPTIVE_SAMPLING
        /* number of samples for pivot selection */
        for(s=30UL,q=900UL; q<nmemb; s+=30UL,q=s*s) ; /* XXX worry about overflow iff dual-pivot shows potential ... */
        if (q>nmemb) s-=30UL;
        if (s<5UL) s=5UL;
#else
        s=5UL;
#endif
        w=nmemb>>1;
        pm=base+w*size; /* middle element */
        pu+=size; /* past last element */
#if SAMPLE_SELECTION
        /* swap sample elements to middle of array for selection */
        /* XXX quick hack */
        for (r=((s+1UL)>>1),q=1UL,t=nmemb/s*size; q<r; q++) {
            if (t!=size) {
                u=q*t;
                v=q*size;
#if SWAP_FUNCTION
                if (pm-u>=base) swapf(pm-u,pm-v,size);
                if (pm+u<pu) swapf(pm+u,pm+v,size);
#else
                if (pm-u>=base) iswap(pm-u,pm-v,size,typeindex);
                if (pm+u<pu) iswap(pm+u,pm+v,size,typeindex);
#endif
            }
        }
        r=s>>1;
# if 1
        q=(s+1UL)/6UL;
        karray[0]=r-q;
        karray[1]=r+q;
# else
        /* Aumuller & Dietzfelbinger suggest s/2 and s/4 */
        /* but it's slower in practice */
        q=r>>1;
        karray[0]=q;
        karray[1]=r;
# endif
        pn=pm-r*size;
        n=s;
        pivot1=pn+karray[0]*size;
        pivot2=pn+karray[1]*size;
        /* XXX selecting pivots partitions the samples around the pivots and
           costs ~ 2s.  Subsequent partitioning of the array costs > 2n.
           Using full array indices (rather than relative to first sample)
           and selecting the pivots based on those ranks from the entire
           array would partition the array around the pivots w/o a separate
           partitioning step, with total cost ~ 2n.
           That would be an extension of the basic quicksort divide-and-conquer
           method, extended to multiple pivots.  Instead of selecting the single
           median as the single pivot, select n pivots to divide the array into
           n+1 regions requiring further processing (and n regions equal to the
           n pivots), with the optimum n ~ sqrt(nmemb).  But to implement that,
           multiple selection would have to somehow indicate the equal-to-pivot
           ranges for each of the pivots, or left+right scans after selection
           would have to determine the extents of those ranges.  And it would
           produce ~ sqrt(nmemb) smaller partitions which would have to be
           processed.
        */
        quickselect_internal(pn,n,size,compar,karray,2UL
# if SAVE_PARTIAL
            ,NULL,NULL
# endif
        ); /* bootstrap with quickselect */
#else
        /* for 5 samples only, simply identify pivots using sort5 */
        t=nmemb/s*size; /* sample spacing (chars) */
        pivot1=pm-t;
        pa=pivot1-t;
        pivot2=pm+t;
        ps=pivot2+t;
#if SWAP_FUNCTION
        sort5(pa,pivot1,pm,pivot2,ps,size,compar,swapf);
#else
        sort5(pa,pivot1,pm,pivot2,ps,size,compar,typeindex);
#endif
#endif
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n// ",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
print_int_array(base,0UL,nmemb-1UL);
#endif
        /* partition array around pivot elements */
#if 1
        /* This implementation puts equals in the outer regions, then separates
           them into position in a quick single-comparison pass over each of the
           two outer regions for a total expected comparison count of 7/3 N. An
           alternative is to put them in the middle region, then separate
           them into position using a one- to two-comparison pass over the
           middle region for a total expected comparison count of 2 N (for
           pivot1,pivot2 same value, else also 7/3 N).  Either has at least
           double the comparisons of a single-pivot partition. This one has less
           overhead, so runs faster on typical input sequences.
        */
        /* | =P1 | <P1 | P1<x<P2 |  ?            | >P2 | =P2 | */
        /*  |     |     |         |               |     |     | */
        /*  base  pa    pb        pm              ps    pt    pu */
        for (pb=pm=base,ps=pu; pm<ps;) {
            A(base<=pm);A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0<=compar(pivot1,pm)) {
#if SWAP_FUNCTION
                if (pb!=pm) swapf(pb,pm,size);
#else
                if (pb!=pm) iswap(pb,pm,size,typeindex);
#endif
                if (pivot1==pm) pivot1=pb; else if (pivot2==pm) pivot2=pb;
                else if (pivot1==pb) pivot1=pm; else if (pivot2==pb) pivot2=pm;
                pb+=size;
            } else if (0>=compar(pivot2,pm)) {
                do ps-=size; while ((pm<ps)&&(0>=compar(pivot2,ps)));
                if (pm>=ps) break;
#if SWAP_FUNCTION
                swapf(pm,ps,size);
#else
                iswap(pm,ps,size,typeindex);
#endif
                if (pivot1==pm) pivot1=ps; else if (pivot2==pm) pivot2=ps;
                else if (pivot1==ps) pivot1=pm; else if (pivot2==ps) pivot2=pm;
                continue; /* new unknown at pm */
            }
            pm+=size;
        }
        /* |      <=P1 | P1<x<P2 |      >=P2 | */
        /*  |           |         |           | */
        /*  base        pb        ps          pu */
        /* group equals */
        /* | <P1 | =P1 | P1<x<P2 | =P2 | >P2 | */
        /*  |     |     |         |     |     | */
        /*  base  pa    pb        ps    pt    pu */
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pb=%p[%lu](%d), ps=%p[%lu](%d)\n// ",__func__,__LINE__,pb,(pb-base)/size,*((int *)pb),ps,(ps-base)/size,*((int *)ps));
print_int_array(base,0UL,nmemb-1UL);
#endif
        for (pa=pb,pm=base; pm<pa; pm+=size) {
            if (0==compar(pivot1,pm)) {
                pa-=size;
#if SWAP_FUNCTION
                if (pm!=pa) swapf(pm,pa,size);
#else
                if (pm!=pa) iswap(pm,pa,size,typeindex);
#endif
                if (pivot1==pm) pivot1=pa; else if (pivot2==pm) pivot2=pa;
            }
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n// ",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pb,(pb-base)/size,*((int *)pb));
print_int_array(base,0UL,nmemb-1UL);
#endif
        for (pt=ps,pm=pu-size; pm>=pt; pm-=size) {
            if (0==compar(pivot2,pm)) {
#if SWAP_FUNCTION
                if (pm!=pt) swapf(pm,pt,size);
#else
                if (pm!=pt) iswap(pm,pt,size,typeindex);
#endif
                if (pivot1==pm) pivot1=pt; else if (pivot2==pm) pivot2=pt;
                pt+=size;
            }
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pb,(pb-base)/size,*((int *)pb));
fprintf(stderr, "//%s line %d: ps=%p[%lu](%d), pt=%p[%lu](%d)\n// ",__func__,__LINE__,ps,(ps-base)/size,*((int *)ps),pt,(pt-base)/size,*((int *)pt));
print_int_array(base,0UL,nmemb-1UL);
#endif
#else
        for (pa=pm=base,pt=pu; pm<pt;) {
            A(base<=pm);A(base<=pa);A(pa<=pm);A(pt<=pu);
            A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0<compar(pivot1,pm)) {
#if SWAP_FUNCTION
                swapf(pa,pm,size);
#else
                iswap(pa,pm,size,typeindex);
#endif
                if (pivot1==pa) pivot1=pm; else if (pivot2==pa) pivot2=pm;
                else if (pivot2==pm) pivot2=pa;
                pa+=size;
            } else if (0>compar(pivot2,pm)) {
                do pt-=size; while ((pm<pt)&&(0>compar(pivot2,pt)));
                if (pm>=pt) break;
#if SWAP_FUNCTION
                swapf(pm,pt,size);
#else
                iswap(pm,pt,size,typeindex);
#endif
                if (pivot1==pt) pivot1=pm; else if (pivot2==pt) pivot2=pm;
                else if (pivot1==pm) pivot1=pt;
                continue; /* new unknown at pm */
            }
            pm+=size;
        }
        /* | <P1 |      P1<=x<=P2      | >P2 | */
        /*  |     |                     |     | */
        /*  base  pa                    pt    pu */
#if DPDEBUG
fprintf(stderr, "//%s line %d: base=%p, pivot1=%p[%lu](%d), pivot2=%p[%lu](%d), pu=%p\n",__func__,__LINE__,base,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2),pu);
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pt=%p[%lu](%d)\n// ",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pt,(pt-base)/size,*((int *)pt));
print_int_array(base,0UL,nmemb-1UL);
#endif
        /* group equals */
        /* | <P1 | =P1 | P1<x<P2 | =P2 | >P2 | */
        /*  |     |     |         |     |     | */
        /*  base  pa    pb        ps    pt    pu */
        for (pm=pb=pa,ps=pt; pm<ps;) {
            A(base<=pm);A(base<=pa);A(pa<=pb);A(ps<=pt);A(pt<=pu);
            A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0==compar(pivot1,pm)) {
#if SWAP_FUNCTION
                swapf(pb,pm,size);
#else
                iswap(pb,pm,size,typeindex);
#endif
                if (pivot1==pm) pivot1=pb; else if (pivot1==pb) pivot1=pm;
                if (pivot2==pm) pivot2=pb; else if (pivot2==pb) pivot2=pm;
                pb+=size;
            } else if (0==compar(pivot2,pm)) {
                do ps-=size; while ((pm<ps)&&(0==compar(pivot2,ps)));
                if (pm>=ps) break;
#if SWAP_FUNCTION
                swapf(pm,ps,size);
#else
                iswap(pm,ps,size,typeindex);
#endif
                if (pivot1==pm) pivot1=ps; else if (pivot1==ps) pivot1=pm;
                if (pivot2==pm) pivot2=ps; else if (pivot2==ps) pivot2=pm;
                continue; /* test moved element at pm */
            }
            pm+=size;
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: base=%p, pivot1=%p[%lu](%d), pivot2=%p[%lu](%d), pu=%p\n",__func__,__LINE__,base,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2),pu);
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pb,(pb-base)/size,*((int *)pb));
fprintf(stderr, "//%s line %d: ps=%p[%lu](%d), pt=%p[%lu](%d)\n// ",__func__,__LINE__,ps,(ps-base)/size,*((int *)ps),pt,(pt-base)/size,*((int *)pt));
print_int_array(base,0UL,nmemb-1UL);
#endif
#endif
        npartitions++;
        /* process [base,pa), [pb,ps), [pt,pu) */
        /* process 3 regions */
        /* region sizes */
        /* should process small regions recursively, large iteratively */
        /* XXX quick hack: 2nd, 3rd regions recursively, 1st iteratively */
        if (pa>base) q=(pa-base)/size; else q=0UL;
        if (ps>pb) r=(ps-pb)/size; else r=0UL;
        if (pu>pt) s=(pu-pt)/size; else s=0UL;
#if SWAP_FUNCTION
        if (r>1UL) dpqsort_internal(pb,r,size,compar,swapf);
        if (s>1UL) dpqsort_internal(pt,s,size,compar,swapf);
#else
        if (r>1UL) dpqsort_internal(pb,r,size,compar,typeindex);
        if (s>1UL) dpqsort_internal(pt,s,size,compar,typeindex);
#endif
        if (q<2UL) return;
        nmemb=q;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void dpqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
    if (4UL>dp_cutoff) dp_cutoff=4UL; /* need at least 5 elements for samples */
#if SWAP_FUNCTION
    dpqsort_internal(base, nmemb, size, compar, whichswap(base,size));
#else
    dpqsort_internal(base, nmemb, size, compar, type_index(base,size));
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t ulsqrt(size_t n)
{
    if (1UL<n) {
        size_t q, r, s, t;

        q=(n>>2);
        r=ulsqrt(q);
        t=(r<<1);
        s=t++;
#if DEBUG_CODE
if (3<debug) (V)fprintf(stderr, "%s line %d: n=%lu, q=%lu, r=%lu, s=%lu, t=%lu t*t=%lu\n",__func__,__LINE__,n,q,r,s,t,t*t);
#endif
        if (t*t>n) return s;
        else return t;
    }
    return n;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t floor_lg(size_t n)
{
    size_t r=0UL;

    while (1UL<n) n>>=1, r++;
    return r;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void logsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    unsigned int table_index)
{
    size_t nmemb=beyond-first;
    A(first<beyond);
    if (8UL<nmemb) {
        size_t k, o, p, s, xnk=floor_lg(nmemb-5UL)-1UL;
        size_t xpk[xnk];
        union aligned_union pv[1+(size-1UL)/sizeof(union aligned_union)];
        unsigned int idx;

        s=nmemb/xnk;
        o=first+(s>>1);
        for (k=0UL,p=o; k<xnk; k++,p+=s) {
            xpk[k] = p;
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: xpk[%lu]=%lu\n",__func__,__LINE__,k,p);
#endif
        }
#if ASSERT_CODE
if ((xpk[0]<=first)||(xpk[xnk-1UL]>=beyond-1UL))
(V)fprintf(stderr, "%s line %d: first=%lu, beyond=%lu, xnk=%lu, o=%lu, xpk[0]=%lu, xpk[%lu]=%lu\n",__func__,__LINE__,first,beyond,xnk,o,xpk[0],xnk-1UL,xpk[xnk-1UL]);
#endif
        A(xpk[0]>first);A(xpk[xnk-1UL]<beyond-1UL);

        quickselect_loop(base,first,beyond,size,compar,xpk,0UL,xnk,
#if SWAP_FUNCTION
            swapf,
#else
            typeindex,
#endif
            sorting_sampling_table,table_index,pv,nmemb,0
#if SAVE_PARTIAL
            ,NULL,NULL
#endif
        );
        /* xnk+1 regions partitioned by xpk ranks; recursively sort them. */
/* XXX for selection (future), check for desired order statistic ranks for each region */
        for (idx=table_index; o<sorting_sampling_table[idx].min_nmemb; idx--) ;
#if SWAP_FUNCTION
        logsort_internal(base,first,xpk[0UL],size,compar,swapf,idx);
#else
        logsort_internal(base,first,xpk[0UL],size,compar,typeindex,idx);
#endif
        for (idx++,s--; s<sorting_sampling_table[idx].min_nmemb; idx--) ;
        for (k=0UL,--xnk; k<xnk; k++) {
            A(xpk[k]+1UL<xpk[k+1UL]);
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: xpk[%lu]=%lu, xpk[%lu]=%lu\n",__func__,__LINE__,k,xpk[k],k+1UL,xpk[k+1UL]);
#endif
#if SWAP_FUNCTION
            logsort_internal(base,xpk[k]+1UL,xpk[k+1UL],size,compar,swapf,idx);
#else
            logsort_internal(base,xpk[k]+1UL,xpk[k+1UL],size,compar,typeindex,idx);
#endif
        }
        for (p=beyond-xpk[k]-1UL; p<sorting_sampling_table[idx].min_nmemb; idx--) ;
#if SWAP_FUNCTION
        logsort_internal(base,xpk[k]+1UL,beyond,size,compar,swapf,idx);
#else
        logsort_internal(base,xpk[k]+1UL,beyond,size,compar,typeindex,idx);
#endif
    } else {
#if SWAP_FUNCTION
        if (0!=use_networks) networksort_internal(base,first,beyond,size,compar,swapf);
        else if (0!=use_shell) shellsort_internal(base,first,beyond,size,compar,swapf);
        else isort_internal(base,first,beyond,size,compar,swapf);
#else
        if (0!=use_networks) networksort_internal(base,first,beyond,size,compar,typeindex);
        else if (0!=use_shell) shellsort_internal(base,first,beyond,size,compar,typeindex);
        else isort_internal(base,first,beyond,size,compar,typeindex);
#endif
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void logsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
    unsigned int table_index=2U; /* optimized for small nmemb */

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((table_index<22UL)
    &&(nmemb>sorting_sampling_table[table_index].min_nmemb))
        table_index++;
    while (nmemb<sorting_sampling_table[table_index].min_nmemb)
        table_index--;
    A(table_index<22UL);

#if SWAP_FUNCTION
    logsort_internal(base,0UL,nmemb,size,compar,whichswap(base,size),table_index);
#else
    logsort_internal(base,0UL,nmemb,size,compar,type_index(base,size),table_index);
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void sqrtsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t nmemb=beyond-first;
#if DEBUG_CODE
if (1<debug) {
fprintf(stderr, "%s line %d: base=%p, first=%lu, beyond=%lu\n",__func__,__LINE__,base,first,beyond);
print_int_array(base,first,beyond-1UL);
}
#endif
    A(first<beyond);
    if (8UL<nmemb) {
        size_t k, o, p, s, xnk=ulsqrt(nmemb-5UL)-1UL;
        size_t xpk[xnk];
        union aligned_union pv[1+(size-1UL)/sizeof(union aligned_union)];
        unsigned int table_index=2U; /* optimized for small nmemb */

        s=nmemb/xnk;
        o=first+(s>>1);
        for (k=0UL,p=o; k<xnk; k++,p+=s) {
            xpk[k] = p;
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: xpk[%lu]=%lu\n",__func__,__LINE__,k,p);
#endif
        }
#if ASSERT_CODE
if ((xpk[0]<=first)||(xpk[xnk-1UL]>=beyond-1UL))
(V)fprintf(stderr, "%s line %d: first=%lu, beyond=%lu, xnk=%lu, o=%lu, xpk[0]=%lu, xpk[%lu]=%lu\n",__func__,__LINE__,first,beyond,xnk,o,xpk[0],xnk-1UL,xpk[xnk-1UL]);
#endif
        A(xpk[0]>first);A(xpk[xnk-1UL]<beyond-1UL);

        /* Initialize the sampling table index for the array size. Sub-array
           sizes will be smalller, and this step ensures that the main loop won't
           have to traverse much of the table during recursion and iteration.
        */
        while ((table_index<22UL)
        &&(nmemb>sorting_sampling_table[table_index].min_nmemb))
            table_index++;
        while (nmemb<sorting_sampling_table[table_index].min_nmemb)
            table_index--;
        A(table_index<22UL);

        quickselect_loop(base,first,beyond,size,compar,xpk,0UL,xnk,
#if SWAP_FUNCTION
            swapf,
#else
            typeindex,
#endif
            sorting_sampling_table,table_index,pv,nmemb,0
#if SAVE_PARTIAL
            ,NULL,NULL
#endif
        );
        /* xnk+1 regions partitioned by xpk ranks; recursively sort them. */
/* XXX for selection (future), check for desired order statistic ranks for each region */
#if SWAP_FUNCTION
        sqrtsort_internal(base,first,xpk[0UL],size,compar,swapf);
#else
        sqrtsort_internal(base,first,xpk[0UL],size,compar,typeindex);
#endif
        for (k=0UL,--xnk; k<xnk; k++) {
            A(xpk[k]+1UL<xpk[k+1UL]);
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: xpk[%lu]=%lu, xpk[%lu]=%lu\n",__func__,__LINE__,k,xpk[k],k+1UL,xpk[k+1UL]);
#endif
#if SWAP_FUNCTION
            sqrtsort_internal(base,xpk[k]+1UL,xpk[k+1UL],size,compar,swapf);
#else
            sqrtsort_internal(base,xpk[k]+1UL,xpk[k+1UL],size,compar,typeindex);
#endif
        }
#if SWAP_FUNCTION
        sqrtsort_internal(base,xpk[k]+1UL,beyond,size,compar,swapf);
#else
        sqrtsort_internal(base,xpk[k]+1UL,beyond,size,compar,typeindex);
#endif
    } else {
#if SWAP_FUNCTION
        if (0!=use_networks) networksort_internal(base,first,beyond,size,compar,swapf);
        else if (0!=use_shell) shellsort_internal(base,first,beyond,size,compar,swapf);
        else isort_internal(base,first,beyond,size,compar,swapf);
#else
        if (0!=use_networks) networksort_internal(base,first,beyond,size,compar,typeindex);
        else if (0!=use_shell) shellsort_internal(base,first,beyond,size,compar,typeindex);
        else isort_internal(base,first,beyond,size,compar,typeindex);
#endif
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void sqrtsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
#if SWAP_FUNCTION
    sqrtsort_internal(base, 0UL, nmemb, size, compar, whichswap(base,size));
#else
    sqrtsort_internal(base, 0UL, nmemb, size, compar, type_index(base,size));
#endif
}

/* Yaroslavskiy's dual-pivot sort */
/* Ref.: Yaroslavskiy, V. "Dual-pivot Quicksort algorithm" September 11, 2009 */
static size_t y_cutoff = 27UL; /* 11 seems to be optimal */
#if 0
/* Yaroslavskiy apparently keeps changing his mind about the magic number 13... */
static size_t y_cutoff2 = 13UL; /* infinite is best */
#endif

/* qsort-like interface, translated from Java-nese,
   magic numbers scaled to tunable y_cutoff (11 works best),
   assertions and protection tests added,
   optimized (initial pivots, cached pivot comparison, busy-work avoidance)
   best available swap
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void yqsort_internal(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), size_t y_div,
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    int pcmp;
    char *pg, *pivot1, *pivot2, *pk, *pl, *pr;
    size_t dist, t;

#define YQSORT_DEBUG 0
#if YQSORT_DEBUG
fprintf(stderr,"%s(%p,%lu,%lu,%p,%lu)\n",__func__,base,nmemb,size,compar,y_div);
#endif
    pr=base+(nmemb-1UL)*size;
    if (nmemb<y_cutoff) { /* small-array sort */
#if SWAP_FUNCTION
        if (0!=use_networks) networksort_internal(base,0UL,nmemb,size,compar,swapf);
        else if (0!=use_shell) shellsort_internal(base,0UL,nmemb,size,compar,swapf);
        else isort_internal(base,0UL,nmemb,size,compar,swapf);
#else
        if (0!=use_networks) networksort_internal(base,0UL,nmemb,size,compar,typeindex);
        else if (0!=use_shell) shellsort_internal(base,0UL,nmemb,size,compar,typeindex);
        else isort_internal(base,0UL,nmemb,size,compar,typeindex);
#endif
/* <- */return; /* Done; */
    }
    /* ideally, pivots should split array into 3 equal-sized regions */
    /* indices should be (nmemb-2UL)/3 and nmemb-1-((nmemb-2UL)/3) */
    A(nmemb>=2UL); A(0UL!=y_div); A(0UL!=size);
#if 0 /* original, unequal spacing */
    t = (nmemb / y_div) * size;
#else /* improved, more equal spacing */
    t = ((nmemb-2UL)/y_div) * size;
#endif
    if (0UL==t) t++;
    /* "medians", actually pivot elements */
    pivot1=base+t;
    pivot2=pr-t;
    pcmp=compar(pivot1,pivot2); /* cached comparison */
    if (0<pcmp) {
#if SWAP_FUNCTION
        swapf(base,pivot2,size);
        swapf(pivot1,pr,size);
#else
        iswap(base,pivot2,size,typeindex);
        iswap(pivot1,pr,size,typeindex);
#endif
    } else {
#if SWAP_FUNCTION
        swapf(base,pivot1,size);
        swapf(pivot2,pr,size);
#else
        iswap(base,pivot1,size,typeindex);
        iswap(pivot2,pr,size,typeindex);
#endif
    }
    /* pivot elements (moved) */
    pivot1=base, pivot2=pr;
    /* pointers */
    pl=base+size, pg=pr-size;
    /* sorting */
    for (pk=pl; pk<=pg; pk+=size) {
#if SWAP_FUNCTION
        if (0>compar(pk,pivot1)) { if (pk!=pl) swapf(pk,pl,size); pl+=size; }
#else
        if (0>compar(pk,pivot1)) { if (pk!=pl) iswap(pk,pl,size,typeindex); pl+=size; }
#endif
        else if (0<compar(pk,pivot2)) {
            while ((pk<pg)&&(0<compar(pg,pivot2))) pg-=size;
#if SWAP_FUNCTION
            if (pk!=pg) swapf(pk,pg,size);
#else
            if (pk!=pg) iswap(pk,pg,size,typeindex);
#endif
            pg-=size;
#if SWAP_FUNCTION
            if (0>compar(pk,pivot1)) { if (pk!=pl) swapf(pk,pl,size); pl+=size; }
#else
            if (0>compar(pk,pivot1)) { if (pk!=pl) iswap(pk,pl,size,typeindex); pl+=size; }
#endif
        }
    }
    npartitions++;
    /* swaps */
    if (pg>pl) dist=(pg-pl)/size; else dist=0UL; /* avoid underflow */
#if 0 /* avoid changing y_div (performance improves w/o change) */
    /* magic number 13 replaced by y_cutoff2 */
    /* dist is the size of the center region (only; ignores other sizes) */
    if (dist < y_cutoff2) y_div++;
#endif
    pivot1=pl-size; /* new location for pivot1 */
#if SWAP_FUNCTION
    if (pivot1!=base) swapf(pivot1,base,size);
#else
    if (pivot1!=base) iswap(pivot1,base,size,typeindex);
#endif
    pivot2=pg+size; /* new location for pivot2 */
#if SWAP_FUNCTION
    if (pivot2!=pr) swapf(pivot2,pr,size);
#else
    if (pivot2!=pr) iswap(pivot2,pr,size,typeindex);
#endif
    /* subarrays */
    t=(pivot1-base)/size; /* save size for protection comparison and recursion */
    A((2UL>t)||(base+(t-1UL)*size<=pr));
#if SWAP_FUNCTION
    if (1UL<t) yqsort_internal(base,t,size,compar,y_div,swapf);
#else
    if (1UL<t) yqsort_internal(base,t,size,compar,y_div,typeindex);
#endif
    t=(pr-pivot2)/size; /* save size for protection comparison and recursion */
    A((2UL>t)||(pivot2+t*size<=pr));
#if SWAP_FUNCTION
    if (1UL<t) yqsort_internal(pivot2+size,t,size,compar,y_div,swapf);
#else
    if (1UL<t) yqsort_internal(pivot2+size,t,size,compar,y_div,typeindex);
#endif
    /* equal elements */
    /* cheap (cached) condition first */
    if ((0!=pcmp)
#if 0 /* always separate equals if pivots differ */
    /* magic number 13 replaced by y_cutoff2 */
    &&(y_cutoff2>nmemb-dist) /* avoid overflow/underflow */
#endif
    ) { /* avoid overflow/underflow */
        for (pk=pl; pk<=pg; pk+=size) {
#if SWAP_FUNCTION
            if (0==compar(pk,pivot1)) { if (pk!=pl) swapf(pk,pl,size); pl+=size; }
#else
            if (0==compar(pk,pivot1)) { if (pk!=pl) iswap(pk,pl,size,typeindex); pl+=size; }
#endif
            else if (0==compar(pk,pivot2)) {
#if SWAP_FUNCTION
                if (pk!=pg) swapf(pk,pg,size);
#else
                if (pk!=pg) iswap(pk,pg,size,typeindex);
#endif
                pg-=size;
#if SWAP_FUNCTION
                if (0==compar(pk,pivot1)) { if (pk!=pl) swapf(pk,pl,size); pl+=size; }
#else
                if (0==compar(pk,pivot1)) { if (pk!=pl) iswap(pk,pl,size,typeindex); pl+=size; }
#endif
            }
        }
        /* revised "dist" */
        if (pg>pl) dist=(pg-pl)/size; else dist=0UL; /* avoid underflow */
    }
    /* subarray */
    A(pl+dist*size<=pr);
    /* avoid unnecessary work for all-equal region; cached condition first */
#if SWAP_FUNCTION
    if ((0!=pcmp)&&(0UL<dist)) yqsort_internal(pl,dist+1UL,size,compar,y_div,swapf);
#else
    if ((0!=pcmp)&&(0UL<dist)) yqsort_internal(pl,dist+1UL,size,compar,y_div,typeindex);
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void yqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
    if (4UL>y_cutoff) y_cutoff=4UL; /* require po<pivot1<pivot2<pr */
#if SWAP_FUNCTION
    yqsort_internal(base, nmemb, size, compar, 3UL,whichswap(base,size));
#else
    yqsort_internal(base, nmemb, size, compar, 3UL,type_index(base,size));
#endif
}
/* ************************************************************** */

/* introsort, loosely based on "Introspective Sorting and Selection Algorithms"
   by David R. Musser. Also insertion sort and heapsort implementations.
*/
/* generalized to take lower(root) and upper indices of heap array */
/* for debugging: not performance-optimized */
#if 0
#define DEBUG_PRINT_INT 1
static void print_int_heap(int *heap, size_t l, size_t u)
{
    int c, columns[15] = { 7, 3, 11, 1, 5, 9, 13, 0, 2, 4, 6, 8, 10, 12, 14 }, d, digits=2;
    size_t j, k, m, row;

    /* first pass: find maximum absolute value (for number of digits) */
    for (d=1,j=l; j<=u; j++) {
        c = heap[j];
        if (0 > c)
            c = 0 - c;
        if (c > d)
            d = c;
    }
    for (c=1,digits=1; c<d; digits++,c*=10) ;
#if DEBUG_PRINT_INT
(V)fprintf(stderr, "%s: %s line %d: %s(..., l=%lu, u=%lu), %d digit%s\n", __func__, source_file, __LINE__, __func__, l, u, digits, digits==1?"":"s");
#endif
    for (row=0UL,m=2UL,k=l; row<4UL; m*=2UL,row++) {
        for (d=0,j=m/2UL; (k<=u)&&(j<m); d+=digits,j++,k++) {
            for(c=(digits+1)*columns[j-1UL]; d<c; d++)
                (V)fprintf(stderr, " ");
            (V)fprintf(stderr, "%*d", digits, heap[k]);
        }
        (V)fprintf(stderr, "\n");
        if (k > u) {
#if DEBUG_PRINT_INT
(V)fprintf(stderr, "%s: %s line %d: %s(..., l=%lu, u=%lu), %d digit%s: k=%lu, j=%lu, m=%lu\n", __func__, source_file, __LINE__, __func__, l, u, digits, digits==1?"":"s", k, j, m);
#endif
            break; /* out of outer loop also */
        }
    }
}
#endif

/* heap parent/child node index calculations */
/* generalized for root at any position in array */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t left_child(size_t l, size_t p)
{
    return l + ((p-l)<<1) + 1UL;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t right_child(size_t l, size_t p)
{
    return l + (((p-l) + 1UL)<<1);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t parent(size_t l, size_t c)
{
    return l + ((c-l-1UL)>>1);
}

/* make a max-heap */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t heap_siftdown(char *base, size_t l, size_t p, size_t u, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t c, rc;

    /* pre-condition: heap(p+1..u) */
    for (; (c = left_child(l,p)) <= u; p = c) {
        rc=right_child(l,p);
        if (rc <= u) {          /* compare parent (i.e. node being sifted down) to farthest child */
            if (0 > compar(base+size*c, base+size*rc)) /* left child is farther from parent than right child */
                c= rc; /* compare p with largest (in max-heap) child; swap will maintain heap order */
        }
        if (0 <= compar(base+size*p, base+size*c))        /* proper p vs. c relationship achieved */
            return p; /* nothing else to do */
#if SWAP_FUNCTION
        swapf(base+size*p,base+size*c,size);
#else
        iswap(base+size*p,base+size*c,size,typeindex);
#endif
    }
    return p;
}

/* generalized to take root and upper indices of heap array */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void heapify_array(char *base, size_t r, size_t u, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t p;

    for (p=parent(r,u); 1; p--) {
#if SWAP_FUNCTION
        (V)heap_siftdown(base, r, p, u, size, compar, swapf);
#else
        (V)heap_siftdown(base, r, p, u, size, compar, typeindex);
#endif
        if (p == r)
            break;
    }

}

/* generalized to take root and upper indices of heap array */
/* Moves the element at the root (minimum in a min-heap, maximum in a max-heap)
      to the lowest index in the array beyond the end of the reduced heap.
   Return the (new) largest index in the heap.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t heap_delete_root(char *base, size_t r, size_t n, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
#if SWAP_FUNCTION
    if (r != n) { swapf(base+size*r,base+size*n,size); /* swap root and last */ }
#else
    if (r != n) { iswap(base+size*r,base+size*n,size,typeindex); /* swap root and last */ }
#endif
    --n;
#if SWAP_FUNCTION
    (V)heap_siftdown(base, r, r, n, size, compar, swapf);
#else
    (V)heap_siftdown(base, r, r, n, size, compar, typeindex);
#endif
    return n;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void heap_sort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t n;

#if SWAP_FUNCTION
    heapify_array(base, first, beyond-1UL, size, compar, swapf);
#else
    heapify_array(base, first, beyond-1UL, size, compar, typeindex);
#endif
    for (n=beyond-1UL; first < n;)
#if SWAP_FUNCTION
        n = heap_delete_root(base, first, n, size, compar, swapf);
#else
        n = heap_delete_root(base, first, n, size, compar, typeindex);
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void heap_sort(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *))
{
#if SWAP_FUNCTION
    heap_sort_internal(base, 0UL, nmemb, size, compar, whichswap(base,size));
#else
    heap_sort_internal(base, 0UL, nmemb, size, compar, type_index(base,size));
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void introsort_loop(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t),
#else
    int typeindex,
#endif
    unsigned int table_index,
    char *pv,
    size_t depth_limit, size_t swaps)
{
    size_t eq, gt, s;
    char *pivot;

    A(beyond>first);
    while (beyond-first>introsort_small_array_cutoff) {
        if (0UL==depth_limit) {
#if SWAP_FUNCTION
            heap_sort_internal(base,first,beyond,size,compar,swapf);
#else
            heap_sort_internal(base,first,beyond,size,compar,typeindex);
#endif
            return;
        }
        depth_limit--;
#if 0
        /* Introsort can be improved by improving the quality and
           quality of samples and pivot selection methods.  The following is
           a simple median-of-3 of the first, middle, and last elements, which
           causes issues with some input sequences (median-of-3 killer,
           rotated, organ-pipe, etc.). It does however use ternary median-of-3.
        */
        pivot = fmed3(base+size*first,base+size*(first+(beyond-first)>>1),base+size*(beyond-1UL),compar);
#else
        /* adaptive sampling for pivot selection as in quickselect */
        pivot = select_pivot(base, first, beyond, size, compar, table_index, NULL);
#endif
        /* partition is modified Bentley&McIlroy split-end partition returning
           indices of equal-to and greater-than region starts.  This efficient
           partition function also avoids further processing of elements
           comparing equal to the pivot.
        */
        swaps=partition(base,first,beyond,pivot,size,compar,
#if SWAP_FUNCTION
            swapf,
#else
            typeindex,
#endif
            pv,0!=pivot_swap?0UL:swaps,&eq,&gt);
        npartitions++;
        s=beyond-gt;
        if (1UL<s) {
            unsigned int idx=table_index;
            while (s<sorting_sampling_table[idx].min_nmemb)
                idx--;
#if SWAP_FUNCTION
            introsort_loop(base,gt,beyond,size,compar,swapf,idx,pv,depth_limit,swaps);
#else
            introsort_loop(base,gt,beyond,size,compar,typeindex,idx,pv,depth_limit,swaps);
#endif
        }
        beyond=eq;
        if (first+1UL<beyond) {
            while (beyond-first<sorting_sampling_table[table_index].min_nmemb)
                table_index--;
        }
    }
    if (0U==introsort_final_insertion_sort) {
#if SWAP_FUNCTION
        if (0!=use_networks) networksort_internal(base,first,beyond,size,compar,swapf);
        else if (0!=use_shell) shellsort_internal(base,first,beyond,size,compar,swapf);
        else isort_internal(base,first,beyond,size,compar,swapf);
#else
        if (0!=use_networks) networksort_internal(base,first,beyond,size,compar,typeindex);
        else if (0!=use_shell) shellsort_internal(base,first,beyond,size,compar,typeindex);
        else isort_internal(base,first,beyond,size,compar,typeindex);
#endif
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void introsort(char *base, size_t nmemb, size_t size, int(*compar)(const void *,const void *))
{
    size_t depth_limit;
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t);
#else
    int typeindex;
#endif
    unsigned int table_index=2U; /* optimized for small nmemb */
    union aligned_union pv[1+(size-1UL)/sizeof(union aligned_union)];

#if SWAP_FUNCTION
    swapf=whichswap(base,size);
#else
    typeindex=type_index(base,size);
#endif
    depth_limit=introsort_recursion_factor*floor_lg(nmemb);

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((table_index<22UL)
    &&(nmemb>sorting_sampling_table[table_index].min_nmemb))
        table_index++;
    while (nmemb<sorting_sampling_table[table_index].min_nmemb)
        table_index--;
    A(table_index<22UL);

#if SWAP_FUNCTION
    introsort_loop(base, 0UL, nmemb, size, compar, swapf, table_index, pv, depth_limit,nmemb);
#else
    introsort_loop(base, 0UL, nmemb, size, compar, typeindex, table_index, pv, depth_limit,nmemb);
#endif
    if (0U!=introsort_final_insertion_sort) {
#if SWAP_FUNCTION
        isort_internal(base,0UL,nmemb,size,compar,swapf);
#else
        isort_internal(base,0UL,nmemb,size,compar,typeindex);
#endif
    }
}
/* ************************************************************** */

/* ************************************************************** */
/* Smoothsort implementation from musl
    http://git.musl-libc.org/cgit/musl/tree/src/stdlib/qsort.c
   redundant header inclusion ifdef'ed, qsort->smoothsort, add swap counts
*/
/* Copyright (C) 2011 by Valentin Ochs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Minor changes by Rich Felker for integration in musl, 2011-04-27. */

#if 0
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "atomic.h"
#else
#define a_ctz_l a_ctz_l
static inline int a_ctz_l(unsigned long x)
{
	__asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
	return x;
}
#endif
#define ntz(x) a_ctz_l((x))

typedef int (*cmpfun)(const void *, const void *);

static inline int pntz(size_t p[2]) {
	int r = ntz(p[0] - 1);
	if(r != 0 || (r = 8*sizeof(size_t) + ntz(p[1])) != 8*sizeof(size_t)) {
		return r;
	}
	return 0;
}

static void cycle(size_t width, unsigned char* ar[], int n)
{
	unsigned char tmp[256];
	size_t l;
	int i;

	if(n < 2) {
		return;
	}

	ar[n] = tmp;
	while(width) {
		l = sizeof(tmp) < width ? sizeof(tmp) : width;
		memcpy(ar[n], ar[0], l);
		for(i = 0; i < n; i++) {
			memcpy(ar[i], ar[i + 1], l);
			ar[i] += l;
		}
		width -= l;
	}
}

/* shl() and shr() need n > 0 */
static inline void shl(size_t p[2], int n)
{
	if(n >= 8 * sizeof(size_t)) {
		n -= 8 * sizeof(size_t);
		p[1] = p[0];
		p[0] = 0;
	}
	p[1] <<= n;
	p[1] |= p[0] >> (sizeof(size_t) * 8 - n);
	p[0] <<= n;
}

static inline void shr(size_t p[2], int n)
{
	if(n >= 8 * sizeof(size_t)) {
		n -= 8 * sizeof(size_t);
		p[0] = p[1];
		p[1] = 0;
	}
	p[0] >>= n;
	p[0] |= p[1] << (sizeof(size_t) * 8 - n);
	p[1] >>= n;
}

static void sift(unsigned char *head, size_t width, cmpfun cmp, int pshift, size_t lp[])
{
	unsigned char *rt, *lf;
	unsigned char *ar[14 * sizeof(size_t) + 1];
	int i = 1;

	ar[0] = head;
	while(pshift > 1) {
		rt = head - width;
		lf = head - width - lp[pshift - 2];

		if((*cmp)(ar[0], lf) >= 0 && (*cmp)(ar[0], rt) >= 0) {
			break;
		}
		if((*cmp)(lf, rt) >= 0) {
			ar[i++] = lf;
			head = lf;
			pshift -= 1;
		} else {
			ar[i++] = rt;
			head = rt;
			pshift -= 2;
		}
	}
	cycle(width, ar, i);
}

static void trinkle(unsigned char *head, size_t width, cmpfun cmp, size_t pp[2], int pshift, int trusty, size_t lp[])
{
	unsigned char *stepson,
	              *rt, *lf;
	size_t p[2];
	unsigned char *ar[14 * sizeof(size_t) + 1];
	int i = 1;
	int trail;

	p[0] = pp[0];
	p[1] = pp[1];

	ar[0] = head;
	while(p[0] != 1 || p[1] != 0) {
		stepson = head - lp[pshift];
		if((*cmp)(stepson, ar[0]) <= 0) {
			break;
		}
		if(!trusty && pshift > 1) {
			rt = head - width;
			lf = head - width - lp[pshift - 2];
			if((*cmp)(rt, stepson) >= 0 || (*cmp)(lf, stepson) >= 0) {
				break;
			}
		}

		ar[i++] = stepson;
		head = stepson;
		trail = pntz(p);
		shr(p, trail);
		pshift += trail;
		trusty = 0;
	}
	if(!trusty) {
		cycle(width, ar, i);
		sift(head, width, cmp, pshift, lp);
	}
}

void smoothsort(void *base, size_t nel, size_t width, cmpfun cmp)
{
	size_t lp[12*sizeof(size_t)];
	size_t i, size = width * nel;
	unsigned char *head, *high;
	size_t p[2] = {1, 0};
	int pshift = 1;
	int trail;

	if (!size) return;

	head = base;
	high = head + size - width;

	/* Precompute Leonardo numbers, scaled by element width */
	for(lp[0]=lp[1]=width, i=2; (lp[i]=lp[i-2]+lp[i-1]+width) < size; i++);

	while(head < high) {
		if((p[0] & 3) == 3) {
			sift(head, width, cmp, pshift, lp);
			shr(p, 2);
			pshift += 2;
		} else {
			if(lp[pshift - 1] >= high - head) {
				trinkle(head, width, cmp, p, pshift, 0, lp);
			} else {
				sift(head, width, cmp, pshift, lp);
			}
			
			if(pshift == 1) {
				shl(p, 1);
				pshift = 0;
			} else {
				shl(p, pshift - 1);
				pshift = 1;
			}
		}
		
		p[0] |= 1;
		head += width;
	}

	trinkle(head, width, cmp, p, pshift, 0, lp);

	while(pshift != 1 || p[0] != 1 || p[1] != 0) {
		if(pshift <= 1) {
			trail = pntz(p);
			shr(p, trail);
			pshift += trail;
		} else {
			shl(p, 2);
			pshift -= 2;
			p[0] ^= 7;
			shr(p, 1);
			trinkle(head - lp[pshift] - width, width, cmp, p, pshift + 1, 1, lp);
			shl(p, 1);
			p[0] |= 1;
			trinkle(head - width, width, cmp, p, pshift, 1, lp);
		}
		head -= width;
	}
}
/* ************************************************************** */

/* ************************************************************** */
/* Plan 9 qsort; name changed */

/*
 * qsort -- simple quicksort
 */

#if 0
#include <u.h>
#else
typedef unsigned long long uintptr;
#endif

typedef
struct
{
	int	(*cmp)(void*, void*);
	void	(*swap)(char*, char*, long);
	long	es;
} Sort;

static	void
swapb(char *i, char *j, long es)
{
	char c;

	do {
		c = *i;
		*i++ = *j;
		*j++ = c;
		es--;
	} while(es != 0);

}

static	void
swapi(char *ii, char *ij, long es)
{
	long *i, *j, c;

	i = (long*)ii;
	j = (long*)ij;
	do {
		c = *i;
		*i++ = *j;
		*j++ = c;
		es -= sizeof(long);
	} while(es != 0);
}

static	char*
pivot(char *a, long n, Sort *p)
{
	long j;
	char *pi, *pj, *pk;

	j = n/6 * p->es;
	pi = a + j;	/* 1/6 */
	j += j;
	pj = pi + j;	/* 1/2 */
	pk = pj + j;	/* 5/6 */
	if(p->cmp(pi, pj) < 0) {
		if(p->cmp(pi, pk) < 0) {
			if(p->cmp(pj, pk) < 0)
				return pj;
			return pk;
		}
		return pi;
	}
	if(p->cmp(pj, pk) < 0) {
		if(p->cmp(pi, pk) < 0)
			return pi;
		return pk;
	}
	return pj;
}

static	void
qsorts(char *a, long n, Sort *p)
{
	long j, es;
	char *pi, *pj, *pn;

	es = p->es;
	while(n > 1) {
		if(n > 10) {
			pi = pivot(a, n, p);
		} else
			pi = a + (n>>1)*es;

		p->swap(a, pi, es);
		pi = a;
		pn = a + n*es;
		pj = pn;
		for(;;) {
			do
				pi += es;
			while(pi < pn && p->cmp(pi, a) < 0);
			do
				pj -= es;
			while(pj > a && p->cmp(pj, a) > 0);
			if(pj < pi)
				break;
			p->swap(pi, pj, es);
		}
		p->swap(a, pj, es);
		j = (pj - a) / es;

		n = n-j-1;
		if(j >= n) {
			qsorts(a, j, p);
			a += (j+1)*es;
		} else {
			qsorts(a + (j+1)*es, n, p);
			n = j;
		}
	}
}

void
p9qsort(void *va, long n, long es, int (*cmp)(void*, void*))
{
	Sort s;

	s.cmp = cmp;
	s.es = es;
	s.swap = swapi;
	if(((uintptr)va | es) % sizeof(long))
		s.swap = swapb;
	qsorts((char*)va, n, &s);
}
/* ************************************************************** */

/* test pattern sequences */
/* reasonable modulus for sawtooth pattern for array of size n */
static size_t sawtooth_modulus(size_t n, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    size_t m, r;

    if (n < MAXIMUM_SAWTOOTH_MODULUS * 2UL) {
        if (n < 5UL) {
            m =  2UL;
        } else {
            m = n >> 1; /* n >> 1 is a fast version of n / 2 */
            if (0 != is_even(m))
                m++;
        }
    } else {
        m = MAXIMUM_SAWTOOTH_MODULUS;
    }
    for (r=n%m; 5UL<=m; m-=2UL,r=n%m) {
        if (0 != is_even(r))
            break;
    }
    if (NULL != f)
        f(LOG_INFO, log_arg, "%s: %s line %d: %s(%lu,...) returns %lu, r=%lu", __func__, source_file, __LINE__, __func__, n, m, r);
    return m;
}

static const char *sequence_name(unsigned int testnum)
{
    switch (testnum) {
        case TEST_SEQUENCE_STDIN :
        return "input sequence";
        case TEST_SEQUENCE_SORTED :
        return "increasing sorted sequence";
        case TEST_SEQUENCE_REVERSE :
        return "decreasing sorted sequence";
        case TEST_SEQUENCE_ROTATED :
        return "rotated sequence";
        case TEST_SEQUENCE_ORGAN_PIPE :
        return "organ pipe sequence";
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
        return "inverted organ pipe sequence";
        case TEST_SEQUENCE_SAWTOOTH :
        return "sawtooth sequence";
        case TEST_SEQUENCE_TERNARY :
        return "random ternary values";
        case TEST_SEQUENCE_BINARY :
        return "random zeros and ones";
        case TEST_SEQUENCE_CONSTANT :
        return "equal values";
        case TEST_SEQUENCE_MEDIAN3KILLER :
        return "median-of-3 killer";
        case TEST_SEQUENCE_RANDOM_DISTINCT :
        return "randomly ordered distinct values";
        case TEST_SEQUENCE_RANDOM_VALUES :
        return "random values (full range)";
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
        return "random values (limited range)";
        case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
        return "random values (restricted range)";
        case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
        return "random values (approximately normal distribution)";
        case TEST_SEQUENCE_HISTOGRAM :
        return "histogram data";
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :
        return "many equal (left)";
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
        return "many equal (middle)";
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
        return "many equal (right)";
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
        return "many equal (shuffled)";
        case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
        return "dual-pivot killer";
        case TEST_SEQUENCE_JUMBLE :
        return "jumble";
        case TEST_SEQUENCE_PERMUTATIONS :
        return "distinct permutations";
        case TEST_SEQUENCE_COMBINATIONS :
        return "0/1 combinations";
        case TEST_SEQUENCE_ADVERSARY :
        return "McIlroy adversary";
        default:
            (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
        break;
    }
    return "invalid testnum";
}

/* replace elements of p between l and r inclusive with element value modulo modulus */
static void modulo(long *p, size_t l, size_t r, long modulus)
{
    for (;l<=r; l++)
        p[l] %= modulus;
}

/* reverse the order of elements in p between l and r inclusive */
static void reverse_long(long *p, size_t l, size_t r)
{
    long t;

    for (;l<r; l++,r--) {
        t=p[l]; p[l]=p[r]; p[r]=t;
    }
}

/* fill long array p between l and r inclusive with integers beginning with n */
static void increment(long *p, size_t l, size_t r, long n)
{
    for (;l<=r; l++)
        p[l] = n++;
}

/* fill long array p between l and r inclusive with constant value n */
static void constant_fill(long *p, size_t l, size_t r, long n)
{
    for (;l<=r; l++)
        p[l] = n;
}

/* fill long array p with random values in [0,modulus) */
static void random_fill(long *p, size_t l, size_t r, uint64_t modulus)
{
    for (;l<=r; l++)
        p[l] = (long)random64n(modulus);
}

/* add random values in [0,modulus) to values in long array p */
static void random_add(long *p, size_t l, size_t r, uint64_t modulus)
{
    for (;l<=r; l++)
        p[l] += (long)random64n(modulus);
}

/* permute array p by swapping one pair of elements in range l..r */
/* Based on Algorithm 4a (Alternate Ives) in Sedgewick "Permutation Generation Methods" */
/* caller provides pointers to array c and variable t, both type size_t */
static void permute(long *p, size_t l, size_t r, size_t *c, size_t *pt)
{
    size_t t=*pt;
    A(r>=t);
    for (;;) {
        if (c[t] < r - t) {
            if (0 == is_even(t)) { /* t is odd */
                A(c[t]<r);
                exchange(p, c[t], c[t]+1UL, 1UL, sizeof(long));
            } else { /* t is even */
                exchange(p, c[t], r-t, 1UL, sizeof(long));
            }
            c[t]++;
            *pt = l;
            return;
        } else {
            c[t] = l;
            *pt = ++t;
        }
    }
}

/* worst-case for sorting network */
#if SWAP_FUNCTION
#define REVERSE_COMPARE_EXCHANGE(ma,mb) if(0>compar(ma,mb))swapf(ma,mb,size)
#else
#define REVERSE_COMPARE_EXCHANGE(ma,mb) if(0>compar(ma,mb))swapper(ma,mb,size,typeindex)
#endif

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void jumble(char *base, size_t first, size_t last, size_t size,
    int(*compar)(const void *, const void *),
#if SWAP_FUNCTION
    void (*swapf)(char *, char *, size_t)
#else
    int typeindex
#endif
    )
{
    size_t nmemb, oldneq, oldnlt, oldngt, oldnsw;

    oldnlt=nlt, oldneq=neq, oldngt=ngt, oldnsw=nsw;
    if ((2UL<=(nmemb=last+1UL-first))&&((MAX_NETWORK)>=nmemb)) {
        char *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9;

#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: first=%lu, last=%lu, nmemb=%lu, size=%lu, nsw=%lu\n",__func__,__LINE__,first,last,nmemb,size,nsw);
#endif
        p0=base+first*size;
        p1=p0+size;
        switch (nmemb) {
            case 2UL : /* 1 comparison */
                REVERSE_COMPARE_EXCHANGE(p0,p1);
            break;;
            case 3UL : /* 3 comparisons */
                p2=p1+size;
                REVERSE_COMPARE_EXCHANGE(p1,p2); /* 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p1); /* 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2); /* 1 */
            break;;
            case 4UL : /* 5 comparisons */
                p2=p1+size;
                p3=p2+size;
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p2);
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
            break;;
            case 5UL : /* 9 comparisons */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                /* parallel group 5 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                /* parallel group 4 */
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p2,p4);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p0,p2);
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p4);
            break;;
            case 6UL : /* 12 comparisons */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                /* parallel group 6 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                /* parallel group 5 */
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                /* parallel group 4 */
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                    REVERSE_COMPARE_EXCHANGE(p4,p5);
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p2,p4);
                    REVERSE_COMPARE_EXCHANGE(p3,p5);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p0,p2);
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p4);
                    REVERSE_COMPARE_EXCHANGE(p1,p5);
            break;;
            case 7UL : /* 16 comparisons */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                /* parallel group 6 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p6);
                /* parallel group 5 */
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                    REVERSE_COMPARE_EXCHANGE(p3,p6);
                /* parallel group 4 */
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                    REVERSE_COMPARE_EXCHANGE(p4,p5);
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p2,p4);
                    REVERSE_COMPARE_EXCHANGE(p3,p5);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p0,p2);
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
                    REVERSE_COMPARE_EXCHANGE(p4,p6);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p4);
                    REVERSE_COMPARE_EXCHANGE(p1,p5);
                    REVERSE_COMPARE_EXCHANGE(p2,p6);
            break;;
            case 8UL : /* 19 comparisons */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                p7=p6+size;
                /* parallel group 6 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p6);
                /* parallel group 5 */
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                    REVERSE_COMPARE_EXCHANGE(p3,p6);
                /* parallel group 4 */
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                    REVERSE_COMPARE_EXCHANGE(p4,p5);
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p2,p4);
                    REVERSE_COMPARE_EXCHANGE(p3,p5);
                    REVERSE_COMPARE_EXCHANGE(p6,p7);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p0,p2);
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
                    REVERSE_COMPARE_EXCHANGE(p4,p6);
                    REVERSE_COMPARE_EXCHANGE(p5,p7);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p4);
                    REVERSE_COMPARE_EXCHANGE(p1,p5);
                    REVERSE_COMPARE_EXCHANGE(p2,p6);
                    REVERSE_COMPARE_EXCHANGE(p3,p7);
            break;;
            case 9UL : /* 25 comparisons */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                p7=p6+size;
                p8=p7+size;
                /* parallel group 9 */
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                /* parallel group 8 */
                    REVERSE_COMPARE_EXCHANGE(p2,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p6);
                /* parallel group 7 */
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
                    REVERSE_COMPARE_EXCHANGE(p4,p6);
                /* parallel group 6 */
                    REVERSE_COMPARE_EXCHANGE(p0,p3);
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p7);
                    REVERSE_COMPARE_EXCHANGE(p2,p6);
                /* parallel group 5 */
                    REVERSE_COMPARE_EXCHANGE(p3,p6);
                    REVERSE_COMPARE_EXCHANGE(p4,p7);
                    REVERSE_COMPARE_EXCHANGE(p2,p5);
                /* parallel group 4 */
                    REVERSE_COMPARE_EXCHANGE(p0,p3);
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p8);
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                    REVERSE_COMPARE_EXCHANGE(p6,p7);
                    REVERSE_COMPARE_EXCHANGE(p2,p5);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                    REVERSE_COMPARE_EXCHANGE(p4,p5);
                    REVERSE_COMPARE_EXCHANGE(p7,p8);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                    REVERSE_COMPARE_EXCHANGE(p6,p7);
            break;;
            case 10UL : /* 29 comparisons */
                p2=p1+size;
                p3=p2+size;
                p4=p3+size;
                p5=p4+size;
                p6=p5+size;
                p7=p6+size;
                p8=p7+size;
                p9=p8+size;
                /* parallel group 9 */
                    REVERSE_COMPARE_EXCHANGE(p4,p5);
                /* parallel group 8 */
                    REVERSE_COMPARE_EXCHANGE(p3,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p6);
                /* parallel group 7 */
                    REVERSE_COMPARE_EXCHANGE(p2,p3);
                    REVERSE_COMPARE_EXCHANGE(p6,p7);
                /* parallel group 6 */
                    REVERSE_COMPARE_EXCHANGE(p1,p3);
                    REVERSE_COMPARE_EXCHANGE(p2,p5);
                    REVERSE_COMPARE_EXCHANGE(p4,p7);
                    REVERSE_COMPARE_EXCHANGE(p6,p8);
                /* parallel group 5 */
                    REVERSE_COMPARE_EXCHANGE(p1,p2);
                    REVERSE_COMPARE_EXCHANGE(p3,p5);
                    REVERSE_COMPARE_EXCHANGE(p4,p6);
                    REVERSE_COMPARE_EXCHANGE(p7,p8);
                /* parallel group 4 */
                    REVERSE_COMPARE_EXCHANGE(p0,p1);
                    REVERSE_COMPARE_EXCHANGE(p2,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p7);
                    REVERSE_COMPARE_EXCHANGE(p8,p9);
                /* parallel group 3 */
                    REVERSE_COMPARE_EXCHANGE(p0,p2);
                    REVERSE_COMPARE_EXCHANGE(p3,p6);
                    REVERSE_COMPARE_EXCHANGE(p7,p9);
                /* parallel group 2 */
                    REVERSE_COMPARE_EXCHANGE(p0,p3);
                    REVERSE_COMPARE_EXCHANGE(p1,p4);
                    REVERSE_COMPARE_EXCHANGE(p5,p8);
                    REVERSE_COMPARE_EXCHANGE(p6,p9);
                /* parallel group 1 */
                    REVERSE_COMPARE_EXCHANGE(p0,p5);
                    REVERSE_COMPARE_EXCHANGE(p1,p6);
                    REVERSE_COMPARE_EXCHANGE(p2,p7);
                    REVERSE_COMPARE_EXCHANGE(p3,p8);
                    REVERSE_COMPARE_EXCHANGE(p4,p9);
            break;
            default :
                /* Fallback in case size is out-of-bounds. */
                reverse_long(base, first, last); /* worst-case for insertion sort */
            break;
        }
    }
    nlt=oldnlt, neq=oldneq, ngt=oldngt, nsw=oldnsw;
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: first=%lu, last=%lu, nmemb=%lu, size=%lu, nsw=%lu\n",__func__,__LINE__,first,last,nmemb,size,nsw);
#endif
}

/* Generate specified test sequence in long array at p, index 0UL through n-1UL
      with maximum value limited to max_val.
*/
static int generate_long_test_arrays(long *p, size_t n, unsigned int testnum, uint64_t max_val, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret = -1;

    if (NULL != p) {
        size_t h, j, k, u, x, y, z;
        size_t permutation_limit;
        long i;

        u = n - 1UL;
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :
                for(j=0UL; j<=u; j++)
                    p[j] = input_data[j].val;
            break;
            case TEST_SEQUENCE_SORTED :
                increment(p,0UL,u,0L);
            break;
            case TEST_SEQUENCE_REVERSE :
                increment(p,0UL,u,0L);
                reverse_long(p,0UL,u);
            break;
            case TEST_SEQUENCE_ROTATED :
                if (0UL<u) increment(p,0UL,u-1UL,1L);
                p[u]=0UL;
            break;
            case TEST_SEQUENCE_ORGAN_PIPE :
                increment(p,0UL,n>>1,0L);
                increment(p,(n+1UL)>>1,u,0L);
                reverse_long(p,(n+1UL)>>1,u);
            break;
            case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
                increment(p,n>>1,u,0L);
                increment(p,0UL,(n-1UL)>>1,0L);
                reverse_long(p,0UL,(n-1UL)>>1);
            break;
            case TEST_SEQUENCE_SAWTOOTH :
                increment(p,0UL,u,0L);
                k = sawtooth_modulus(j,NULL,log_arg);
                modulo(p,0UL,u,k);
            break;
            case TEST_SEQUENCE_TERNARY :
                random_fill(p,0UL,u,3);
            break;
            case TEST_SEQUENCE_BINARY :
                random_fill(p,0UL,u,2);
            break;
            case TEST_SEQUENCE_CONSTANT :
                constant_fill(p,0UL,u,CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MEDIAN3KILLER :
                /* start with increasing sorted sequence */
                increment(p,0UL,u,0L);
                k = n;
                /* median-of-3-killer sequence */
                if (1UL < k) {
                    if (0 == is_even(k))
                        k--;
                    /* k is the greatest even number <= n */
                    for(h = n-2UL; 1; h -= 2UL) {
                        j = ((k - h) >> 1); /* >> 1 is a fast version of / 2 */
                        exchange(p, h+1UL, j, 1UL, sizeof(long));
                        if (2UL > h)  /* don't allow h to iterate below 2 */
                            break;
                    }
                }
            break;
            case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                /* start with increasing sorted sequence */
                increment(p,0UL,u,0L);
                /* swap extreme elements to 1/3 and 2/3 positions */
                for (x=((n-4UL)>>1); ; x--) { /*x=1st index @ yqsort*/
                    A(n>(x<<1));
                    h=n-(x<<1); /* elements when yqsort sees subarray */
                    A(2UL<=h); A(h<=n);
                    j = (h-2UL)/3UL; /* yqsort pivot offset from x */
                    if (0UL==j) j++;
                    k = x+j; /* swap x,k */
                    A(k<u); A(k>x);
                    A(u==n-1UL);
                    z = u-x; /* index of last element seen by yqsort */
                    A(z<=u);
                    y = z-j; /* swap y,z */
                    A(y<z); A(y>k);
                    exchange(p, x, k, 1UL, sizeof(long));
                    exchange(p, y, z, 1UL, sizeof(long));
                    if (0UL==x) break;
                }
            break;
            case TEST_SEQUENCE_JUMBLE :
                /* start with increasing sorted sequence */
                A(0UL<=u);
                increment(p,0UL,u,0L);
#if SWAP_FUNCTION
                jumble(p,0UL,u,sizeof(long),longcmp,whichswap(p,sizeof(long)));
#else
                jumble(p,0UL,u,sizeof(long),longcmp,type_index(p,sizeof(long)));
#endif
            break;
            case TEST_SEQUENCE_RANDOM_DISTINCT :
                increment(p,0UL,u,0L);
                (V)fisher_yates_shuffle(p, n, sizeof(long), random64n, f, log_arg);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
                random_fill(p,0UL,u,max_val);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                random_fill(p,0UL,u,n);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
                h=snlround(sqrt((double)n),NULL,NULL);
                random_fill(p,0UL,u,h);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
                constant_fill(p,0UL,u,0L);
                /* Ref: R.W.Hamming, "Numerical Methods for Scientists and Engineers",
                   2nd Ed., Sect. 8.6, ISBN 978-0-486-65241-2
                */
                for (h=0UL,k=max_val/12UL; h<12UL; h++)
                    random_add(p,0UL,u,k);
            break;
            case TEST_SEQUENCE_HISTOGRAM :
                constant_fill(p,0UL,u,0L);
                if (max_val / n < n) x=n*n; else x=max_val/n;
                if (10000000UL<x) x=10000000UL; /* time limit */
                z=12UL*u+1UL;
                for (j=0UL; j<x; j++) {
                    for (i=0L,h=0UL; h<12UL; h++) {
                        i += (long)random64n(n);
                    }
                    p[(n*i)/z]++;
                }
            break;
            case TEST_SEQUENCE_MANY_EQUAL_LEFT :
                random_fill(p,0UL,u,n);
                constant_fill(p,0UL,n>>2,CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
                random_fill(p,0UL,u,n);
                constant_fill(p,(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
                random_fill(p,0UL,u,n);
                constant_fill(p,(n>>1)+(n>>2),u,CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                random_fill(p,0UL,u,n);
                constant_fill(p,(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
                (V)fisher_yates_shuffle(p, n, sizeof(long), random64n, f, log_arg);
            break;
            case TEST_SEQUENCE_PERMUTATIONS :
                /* initialized and updated in test functions */
            break;
            case TEST_SEQUENCE_COMBINATIONS :
                /* initialized and updated in test functions */
            break;
            case TEST_SEQUENCE_ADVERSARY :
                /* handled by initialize_antiqsort */
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
                ret = 7;
            break;
        }
        if (0 > ret) ret = 0;
    }
    return ret;
}

/* macros to unify function calls */
#define BMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) bmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define IBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) ibmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define MBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) mbmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define GLQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) glibc_quicksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define NBQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) nbqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define WQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) wqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#if SAVE_PARTIAL
# define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_internal((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,NULL,NULL)
#else
# define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_internal((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl)
#endif
#define YQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) yqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define DPQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) dpqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define HEAPSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) heap_sort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define INTROSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) introsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define ISORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) isort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SELSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) selsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SHELLSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) shellsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define NETWORKSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) networksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SQRTSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqrtsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define LOGSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) logsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SMOOTHSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) smoothsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define P9QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) p9qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)

#define FUNCTION_WQSORT         1U
#define FUNCTION_YQSORT         2U
#define FUNCTION_SQSORT         3U
#define FUNCTION_MBMQSORT       4U
#define FUNCTION_BMQSORT        5U
#define FUNCTION_NBQSORT        6U
#define FUNCTION_GLQSORT        7U
#define FUNCTION_QSORT          8U
#define FUNCTION_QSELECT        9U
#define FUNCTION_QSORT_WRAPPER 10U
#define FUNCTION_DPQSORT       11U
#define FUNCTION_HEAPSORT      12U
#define FUNCTION_INTROSORT     13U
#define FUNCTION_ISORT         14U
#define FUNCTION_SELSORT       15U
#define FUNCTION_SHELLSORT     16U
#define FUNCTION_NETWORKSORT   17U
#define FUNCTION_SQRTSORT      18U
#define FUNCTION_LOGSORT       19U
#define FUNCTION_SMOOTHSORT    20U
#define FUNCTION_P9QSORT       21U
#define FUNCTION_IBMQSORT      22U

/* type of test */
#define TEST_TYPE_MEDIAN       0x002U
#define TEST_TYPE_PARTITION    0x004U
#define TEST_TYPE_TRIPARTITION 0x008U
#define TEST_TYPE_ADVERSARY    0x010U
#define TEST_TYPE_SORT         0x020U
#define TEST_TYPE_THOROUGH     0x040U
#define TEST_TYPE_CORRECTNESS  0x080U
#define TEST_TYPE_TIMING       0x100U

#if 1
#define strlcat(mb,ms,ml) strcat(mb,ms)
#endif
#if 0
/* a name for the type (not function) of test */
static void test_names(char *buf, size_t sz, unsigned int tests)
{
    if (NULL != buf) {
        buf[0] = '\0';
        if (TEST_TYPE_PARTITION == (TEST_TYPE_PARTITION & tests)) {
            if ('\0' != buf[0]) (V)strlcat(buf, ",", sz);
            (V)strlcat(buf, "partition", sz);
        }
        if (TEST_TYPE_MEDIAN == (TEST_TYPE_MEDIAN & tests)) {
            if ('\0' != buf[0]) (V)strlcat(buf, ",", sz);
            (V)strlcat(buf, "selection", sz);
        }
        if (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests)) {
            if ('\0' != buf[0]) (V)strlcat(buf, ",", sz);
            (V)strlcat(buf, "sort", sz);
        }
        if (TEST_TYPE_THOROUGH == (TEST_TYPE_THOROUGH & tests)) {
            if ('\0' != buf[0]) (V)strlcat(buf, ",", sz);
            (V)strlcat(buf, "thorough", sz);
        }
        if (TEST_TYPE_ADVERSARY == (TEST_TYPE_ADVERSARY & tests)) {
            if ('\0' != buf[0]) (V)strlcat(buf, ",", sz);
            (V)strlcat(buf, "adversary", sz);
        }
        if ('\0' == buf[0]) (V)strlcat(buf, "(none)", sz);
    }
}
#endif

/* expected time factor for tests: N log(N) for sorting, N for others (pivot, partition, median) */
static double test_factor(unsigned int tests, size_t n)
{
    double d = (double)n;

#if 0
    if (TEST_TYPE_ADVERSARY == (TEST_TYPE_ADVERSARY & tests)) {
        d *= d;
    }
#endif
    if (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests)) {
        d *= log2(d);
    }
    return d;
}

/* expected time factor for tests: N log(N) for sorting, N for others (pivot, partition, median) */
static const char *test_size(unsigned int tests)
{
#if 0
    if (TEST_TYPE_ADVERSARY == (TEST_TYPE_ADVERSARY & tests)) return "N^2";
#endif
    if (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests)) return "N log(N)";
    return "N";
}

/* name of the function to be tested */
static const char *function_name(unsigned int func, size_t n)
{
    const char *ret=NULL;

    switch (func) {
        case FUNCTION_WQSORT :        ret = "lopsided quickselect";
        break;
        case FUNCTION_SQSORT :        ret = "simplified quickselect";
        break;
        case FUNCTION_QSELECT :       ret = "quickselect";
        break;
        case FUNCTION_QSORT :         ret = "library qsort";
        break;
        case FUNCTION_IBMQSORT :      /*FALLTHROUGH*/
        case FUNCTION_BMQSORT :       ret = "Bentley&McIlroy qsort";
        break;
        case FUNCTION_NBQSORT :       ret = "NetBSD qsort";
        break;
        case FUNCTION_MBMQSORT :      ret = "modified Bentley&McIlroy qsort";
        break;
        case FUNCTION_GLQSORT :       ret = "glibc qsort";
        break;
        case FUNCTION_QSORT_WRAPPER : ret = "qsort_wrapper";
        break;
        case FUNCTION_YQSORT :        ret = "Yaroslavskiy's dual-pivot sort";
        break;
        case FUNCTION_DPQSORT :       ret = "dual-pivot qsort";
        break;
        case FUNCTION_HEAPSORT :      ret = "heapsort";
        break;
        case FUNCTION_INTROSORT :     ret = "introsort";
        break;
        case FUNCTION_ISORT :         ret = "insertion sort";
        break;
        case FUNCTION_SELSORT :       ret = "selection sort";
        break;
        case FUNCTION_SHELLSORT :     ret = "Shell sort";
        break;
        case FUNCTION_NETWORKSORT :   ret = "network sort";
        break;
        case FUNCTION_SQRTSORT :      ret = "sqrt sort";
        break;
        case FUNCTION_LOGSORT :       ret = "log sort";
        break;
        case FUNCTION_SMOOTHSORT :    ret = "smoothsort";
        break;
        case FUNCTION_P9QSORT :       ret = "plan9 qsort";
        break;
        default :
            errno = EINVAL;
        break;
    }
    return ret;
}

/* name of the type of test for a particular function
   side-effect: sets appropriate tests, unsets inappropriate tests
*/
static const char *function_type(unsigned int func, unsigned int *ptests)
{
    const char *ret=NULL;

    switch (func) {
        case FUNCTION_QSELECT :
            *ptests |= ( TEST_TYPE_PARTITION | TEST_TYPE_MEDIAN );
            if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests))  ret = "sort"; else ret = "selection";
        break;
        case FUNCTION_WQSORT :        /*FALLTHROUGH*/
        case FUNCTION_SQSORT :        /*FALLTHROUGH*/
        case FUNCTION_IBMQSORT :      /*FALLTHROUGH*/
        case FUNCTION_BMQSORT :       /*FALLTHROUGH*/
        case FUNCTION_MBMQSORT :      /*FALLTHROUGH*/
        case FUNCTION_NBQSORT :       /*FALLTHROUGH*/
        case FUNCTION_GLQSORT :       /*FALLTHROUGH*/
        case FUNCTION_DPQSORT :       /*FALLTHROUGH*/
        case FUNCTION_YQSORT :        /*FALLTHROUGH*/
        case FUNCTION_HEAPSORT :      /*FALLTHROUGH*/
        case FUNCTION_INTROSORT :     /*FALLTHROUGH*/
        case FUNCTION_ISORT :         /*FALLTHROUGH*/
        case FUNCTION_SELSORT :       /*FALLTHROUGH*/
        case FUNCTION_SHELLSORT :     /*FALLTHROUGH*/
        case FUNCTION_NETWORKSORT :   /*FALLTHROUGH*/
        case FUNCTION_SQRTSORT :      /*FALLTHROUGH*/
        case FUNCTION_LOGSORT :       /*FALLTHROUGH*/
        case FUNCTION_SMOOTHSORT :    /*FALLTHROUGH*/
        case FUNCTION_P9QSORT :       /*FALLTHROUGH*/
        case FUNCTION_QSORT :         ret = "sort";
            *ptests |= ( TEST_TYPE_PARTITION | TEST_TYPE_MEDIAN | TEST_TYPE_SORT );
        break;
        case FUNCTION_QSORT_WRAPPER : ret = "sort";
            *ptests |= ( TEST_TYPE_PARTITION | TEST_TYPE_MEDIAN | TEST_TYPE_SORT );
        break;
        default :
            errno = EINVAL;
        break;
    }
    return ret;
}

/* return non-zero if test testnum is valid for test_type at size n */
static int valid_test(unsigned int test_type, unsigned int testnum, size_t n)
{
    if (1UL>n) return 0; /* nothing to test! */
    /* same conditions for correctness and timing */
    switch (testnum) {
        /* size-limited */
        case TEST_SEQUENCE_ADVERSARY :
            if (2UL > n) return 0;
        break;
        case TEST_SEQUENCE_ORGAN_PIPE :               /*FALLTHROUGH*/
        case TEST_SEQUENCE_HISTOGRAM :                /*FALLTHROUGH*/
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
            if (3UL > n) return 0;
        break;
        case TEST_SEQUENCE_DUAL_PIVOT_KILLER :        /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :        /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :         /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :      /*FALLTHROUGH*/
        case TEST_SEQUENCE_SAWTOOTH :
            if (5UL > n) return 0;
        break;
        case TEST_SEQUENCE_MEDIAN3KILLER :
            if (7UL > n) return 0;
        break;
        /* not size-limited */
        case TEST_SEQUENCE_STDIN :                    /*FALLTHROUGH*/
        case TEST_SEQUENCE_SORTED :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_ROTATED :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_REVERSE :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_BINARY :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_TERNARY :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_COMBINATIONS :             /*FALLTHROUGH*/
        case TEST_SEQUENCE_PERMUTATIONS :             /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_DISTINCT :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_CONSTANT :                 /*FALLTHROUGH*/
        case TEST_SEQUENCE_JUMBLE :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :    /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED : /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES :
        break;
        default:
            (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n",
                __func__, source_file, __LINE__, testnum);
        return 0;
    }
#if 0
    /* conditions specific to correctness or timing */
    switch (test_type) {
        case TEST_TYPE_CORRECTNESS :
            switch (testnum) {
                /* always invalid for correctness */
                case TEST_SEQUENCE_ADVERSARY :
                return 0; /* no correctness test */
            }
        break;
        case TEST_TYPE_TIMING :
            switch (testnum) {
                /* size-limited */
                case TEST_SEQUENCE_ADVERSARY :
                    if (2UL > n) return 0;
                break;
            }
        break;
        default :
            (V)fprintf(stderr, "%s: %s line %d: unrecognized test_type %u\n",
                __func__, source_file, __LINE__, test_type);
        return 0;
    }
#endif
    return 1;
}

/* duplicate (as far as possible) long data in larray to other arrays */
/* n items; source starts at index 0UL, copies at index o */
static void duplicate_test_data(long *refarray, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, size_t o, size_t n)
{
    long l;
    int i;
    size_t j;
    double d;

    for (j=0UL; j<n; j++) {
        l = refarray[j];
        i = l % INT_MAX;
        d = (double)l;
        if (NULL!=larray) {
            larray[j+o] = l;
        }
        if (NULL!=big_array) {
            big_array[j+o].l = l;
            big_array[j+o].d = d;
            (V)snl(big_array[j+o].string,BIG_STRUCT_STRING_SIZE,NULL,NULL,l,36,
                '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
            big_array[j+o].val = i;
            if (NULL!=parray) {
                parray[j+o] = &(big_array[j+o]);
            }
        }
        if (NULL!=darray) {
            darray[j+o] = d;
        }
        if (NULL!=array) {
            array[j+o] = i;
        }
    }
}

static void restore_test_data(size_t o, size_t n, long *refarray, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray)
{
    duplicate_test_data(refarray, larray, array, darray, big_array, parray, o, n);
}

static unsigned int correctness_test(int type, size_t size, long *refarray, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, int (*comparison_function)(const void *, const void *), const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    char buf[256], buf2[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="";
    const char *comment="";
    const char *psize = test_size(*ptests);
    int c, c1, c2, emin, emax, ik, odebug;
    unsigned int distinct=0U, errs=0U, rpt=flags['d'];
    size_t carray[MAX_PERMUTATION_SIZE], ct, eql, equ, k, *karray=NULL, m, nc, nk, t, u, x;
    size_t erro
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , errt = 0UL
        , j
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , o
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        ;
    FILE *fp
#if SILENCE_WHINEY_COMPILERS
        = NULL
#endif
        ;
    double d, d2;
    void *pv;
    uint64_t max_val;

    if (0U == ((0x01U << testnum) & *psequences)) return ++errs;
    pcc = function_name(func, n);
    if (NULL == pcc) {
        errs++;
    } else {
        pfunc = function_type(func, ptests);
        if (NULL == pfunc) {
            errs++;
        }
    }
    if (0U == errs) {
        switch (type) {
            case DATA_TYPE_LONG :
                pv=larray;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_INT :
                pv=array;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_DOUBLE :
                pv=darray;
                max_val = INT_MAX;
            break;
            case DATA_TYPE_POINTER :
                pv=parray;
                max_val = INT_MAX;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                pv=big_array;
                max_val = INT_MAX;
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized data type %u\n", __func__, source_file, __LINE__, type);
            return ++errs;
        }
        ptest = sequence_name(testnum);
        u = n - 1UL;
        k = u>>1;
        /* generated test sequences */
        emin = emax = (int)k;
        if (0 != is_even(n)) {
            if (0 < emin) emin--;
            emax++;
        }
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :                /*FALLTHROUGH*/
            case TEST_SEQUENCE_SORTED :               /*FALLTHROUGH*/
            case TEST_SEQUENCE_ROTATED :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_REVERSE :              /*FALLTHROUGH*/
                count = 1UL;
            break;
            case TEST_SEQUENCE_ORGAN_PIPE :           /*FALLTHROUGH*/
            case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
                count = 1UL;
                emin = (int)u / 4 - 1;
                if (0 > emin) emin = 0;
                emax = n - emin;
            break;
            case TEST_SEQUENCE_SAWTOOTH :
                count = 1UL;
                m = sawtooth_modulus(n,NULL,log_arg);
                emin = (int)m / 2 - 1;
                if (0 > emin) emin = 0;
                emax = n - emin;
            break;
            case TEST_SEQUENCE_BINARY :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_TERNARY :             /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
                emin = 0; emax = n;                  /*FALLTHROUGH*/
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH*/
            case TEST_SEQUENCE_JUMBLE :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_DISTINCT :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_ADVERSARY :
            break;
            case TEST_SEQUENCE_MANY_EQUAL_LEFT :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_RIGHT :    /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                count = 1UL;                         /*FALLTHROUGH*/
                emin = 0; emax = n;                  /*FALLTHROUGH*/
            break;
            case TEST_SEQUENCE_CONSTANT :
                emin = 0; emax = n;
            break;
            case TEST_SEQUENCE_DUAL_PIVOT_KILLER :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MEDIAN3KILLER :
                count = 1UL;
                emin--; emin--; emax++; emax++;
                if (0 > emin) emin = 0;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_HISTOGRAM :                /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                emin = 0 ; emax = (int)u;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED : /*FALLTHROUGH*/
                emin = 0 ; emax = (int)snlround(sqrt((double)u),NULL,NULL);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
                emin = EXPECTED_RANDOM_MIN; emax = EXPECTED_RANDOM_MAX; /* XXX might not work for long */
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
        if (0U == errs) {
            /* special count numbers, etc. */
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    nc=MAX_PERMUTATION_SIZE;
                    if (n>nc) return ++errs;
                    if (nc>n) nc=n;
                    count = factorial(nc);
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    nc=MAX_COMBINATION_SIZE;
                    if (n>=nc) return ++errs;
                    if (nc>n) nc=n;
                    count = 0x01UL << nc;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
            }
            /* run tests */
            d=d2=0.0;
            for (m=o=0UL; m<count; m++) {
                switch (testnum) {
                    case TEST_SEQUENCE_ADVERSARY :
                        if (0UL==m) {
                            /* generate test sequence */
                            odebug=debug, debug=0;
                            initialize_antiqsort(n, pv, type, size, refarray);
                            /* run sorting function (even if selection is specified)
                               against adversary to generate adverse sequence in
                               refarray, without instrumented comparisons
                            */
                            switch (func) {
                                case FUNCTION_QSELECT :
                                    QSEL(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_BMQSORT :
                                    BMQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_IBMQSORT :
                                    IBMQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_MBMQSORT :
                                    MBMQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_NBQSORT :
                                    NBQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_GLQSORT :
                                    GLQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_WQSORT :
                                    WQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SQSORT :
                                    SQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_QSORT :
                                    QSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_QSORT_WRAPPER :
                                    qsort_wrapper((char *)pv+size*o,u+1UL,size,aqcmp);
                                break;
                                case FUNCTION_YQSORT :
                                    YQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_DPQSORT :
                                    DPQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_HEAPSORT :
                                    HEAPSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_INTROSORT :
                                    INTROSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_ISORT :
                                    ISORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SELSORT :
                                    SELSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SHELLSORT :
                                    SHELLSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_NETWORKSORT :
                                    NETWORKSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SQRTSORT :
                                    SQRTSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_LOGSORT :
                                    LOGSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SMOOTHSORT :
                                    SMOOTHSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_P9QSORT :
                                    P9QSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                            }
                            if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
                            debug=odebug;
                        } else
                            restore_test_data(0UL,n,refarray,larray,array,darray,
                                big_array,parray);
                            /* refarray should now contain an appropriate adverse sequence */
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,n);
                    break;
                    case TEST_SEQUENCE_PERMUTATIONS :
                        if (0UL==m) {
                            /* initial array is sorted sequence */
                            c = generate_long_test_arrays(refarray,
                                n, TEST_SEQUENCE_SORTED, max_val, f, log_arg);
                            if (0 > c) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                    __func__, source_file, __LINE__, c);
                                return ++errs;
                            } else if (0 < c) {
                                return ++errs;
                            }
                            /* initialize for permutations */
                            for (ct=nc; 0UL<ct; carray[--ct]=0UL) ;
                        } else {
                            permute(refarray,0UL,u,carray,&ct);
                        }
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,n);
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* combinations of 0, 1 elements for this test */
                        for (ct=0UL; ct<nc; ct++) {
                            refarray[ct] = (long)((m >> (nc-ct-1UL)) & 0x01UL);
                        }
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,n);
                    break;
                    default :
                        /* generate new  test sequence */
                        c=generate_long_test_arrays(refarray,
                            n, testnum, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,
                            n);
                    break;
                }
                /* long test data is in long array larray */
                /* test sequence has been copied to others */
                /* verify correctness of test sequences */
                switch (testnum) {
                    case TEST_SEQUENCE_SORTED :               /*FALLTHROUGH*/
                        distinct=1U;
#if GENERATOR_TEST
                        t = test_array_sort(pv, o, o+u, size, comparison_function, distinct, logger, log_arg);
                        if (t != o+u) {
                            (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", ptest);
                            errs++;
                            erro = (unsigned int)o;
                            if (t <= o+u) errt = t;
                        }
                        if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
#endif /* GENERATOR_TEST */
                    case TEST_SEQUENCE_JUMBLE :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_REVERSE :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ROTATED :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_DISTINCT :         /*FALLTHROUGH*/
                    case TEST_SEQUENCE_PERMUTATIONS :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_DUAL_PIVOT_KILLER :       /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MEDIAN3KILLER :           /*FALLTHROUGH*/
                        distinct=1U;
#if GENERATOR_TEST
                        if (1000UL > u) {
                            t = test_array_distinctness(pv, o, o+u, size, comparison_function, f, log_arg);
                            if (t != o+u) {
                                (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_distinctness) for %s ^^^^^ ERROR!!!! line %d\n", ptest, __LINE__);
                                errs++;
                                erro = (unsigned int)o;
                                if (t <= o+u) errt = t;
                            }
                        }
                        if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
#endif /* GENERATOR_TEST */
                    break;
                    case TEST_SEQUENCE_STDIN :                   /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ORGAN_PIPE :              /*FALLTHROUGH*/
                    case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :     /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SAWTOOTH :                /*FALLTHROUGH*/
                    case TEST_SEQUENCE_BINARY :                  /*FALLTHROUGH*/
                    case TEST_SEQUENCE_TERNARY :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_CONSTANT :                /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES :           /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :   /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :/*FALLTHROUGH*/
                    case TEST_SEQUENCE_HISTOGRAM :                /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_LEFT :         /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :       /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_RIGHT :        /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :     /*FALLTHROUGH*/
                    case TEST_SEQUENCE_COMBINATIONS :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ADVERSARY :               /*FALLTHROUGH*/
                        distinct=0U;
                    break;
                    default:
                        (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
                    return ++errs;
                }
                if ((2UL > count) && (MAX_ARRAY_PRINT > n)) {
                    (V)fprintf(stderr, "initial %s array:\n", ptest);
                    switch (type) {
                        case DATA_TYPE_LONG :
                            print_long_array(pv, o, o+u);
                        break;
                        case DATA_TYPE_INT :
                            print_int_array(pv, o, o+u);
                        break;
                        case DATA_TYPE_DOUBLE :
                            print_double_array(pv, o, o+u);
                        break;
                        case DATA_TYPE_POINTER :
                            print_indirect_keys(pv, o, o+u);
                        break;
                        case DATA_TYPE_STRUCT :
                        /*FALLTHROUGH*/
                        case DATA_TYPE_STRING :
                            print_big_array_keys(pv, o, o+u);
                        break;
                    }
                }
                eql = equ = j = o+k;
                if (0UL==m) {
                        (V)fprintf(stdout, "%s vs. %s %s: ", pcc, typename, ptest);
                        fflush(stdout);
                }
                switch (testnum) {
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* test number */
                        if (NULL != fp) {
                            c = snul(buf, sizeof(buf), NULL, NULL, m, 2, '0', (int)n, f, log_arg);
                            (V)fprintf(fp, "%s", buf);
                            fflush(fp);
                        }
                    break;
                }
do_test:
                /* reset counters every time, as they are altered by correctness test comparisons */
                if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
                switch (func) {
                    case FUNCTION_QSELECT :
                        if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests)) {
                            QSEL(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                        } else {
                            karray = malloc(selection_nk*sizeof(size_t));
                            if (NULL==karray) return ++errs;
                            if ((1UL==selection_nk)&&(flags['m']=='m')) { /* special-case: 1 or 2 median(s) */
                                karray[0] = k; /* lower-median */
                                karray[1] = ((u+1UL)>>1); /* upper-median */
                                nk = 2UL;
                            } else {
                                nk = selection_nk;
                                switch (flags['m']) {
                                    case 'b' : /* bottom */ /*FALLTHROUGH*/
                                    case 'l' : /* left or low */
                                        for (t=0UL; t<nk; t++)
                                            karray[t] = (t<n)?t:u;
                                    break;
                                    case 'm' : /* middle */
                                        x = (nk>>1);
                                        for (t=0UL; t<nk; t++)
                                            karray[t] = (x<k+t)?((k+t<n+x)?k+t-x:u):0UL;
                                    break;
                                    case 'r' : /* right */ /*FALLTHROUGH*/
                                    case 't' : /* top */
                                        for (t=0UL; t<nk; t++)
                                            karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                    break;
                                    case 's' : /* separated */
                                        x = (nk>>1);
                                        for (t=0UL; t<x; t++)
                                            karray[t] = (t<n)?t:u;
                                        for (; t<nk; t++)
                                            karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                    break;
                                    case 'd' : /*FALLTHROUGH*/
                                    default : /* distributed */
                                        for (t=0UL; t<nk; t++)
                                            karray[t] = n * (t+1UL) / (nk+1UL);
                                    break;
                                }
                            }
#if DEBUG_CODE
if ((0UL==m)&&(0<debug)) {
(V)fprintf(stderr, "order statistic ranks: %lu", karray[0]);
for (t=1UL; t<nk; t++)
(V)fprintf(stderr, ", %lu", karray[t]);
(V)fprintf(stderr, "\n");
}
#endif
                            QSEL(pv,o,o+u,size,comparison_function,karray,0UL,nk,rpt);
                            free(karray);
                            karray = NULL;
                        }
#if DEBUG_CODE
if (2<debug) (V)fprintf(stderr, "%s line %d: quickselect returned\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_IBMQSORT :
                        IBMQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MBMQSORT :
                        MBMQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_WQSORT :
                        WQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQSORT :
                        SQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
                        qsort_wrapper((char *)pv+size*o,u+1UL,size,comparison_function);
                    break;
                    case FUNCTION_YQSORT :
                        YQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_DPQSORT :
                        DPQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_HEAPSORT :
                        HEAPSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_INTROSORT :
                        INTROSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_ISORT :
                        ISORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SELSORT :
                        SELSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SHELLSORT :
                        SHELLSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NETWORKSORT :
                        NETWORKSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQRTSORT :
                        SQRTSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_LOGSORT :
                        LOGSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SMOOTHSORT :
                        SMOOTHSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_P9QSORT :
                        P9QSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                }
                if (0U!=flags['i']) {
                    double factor;

                    t=nlt+neq+ngt;
#if 0
fprintf(stderr, "%s line %d: *ptests=0x%x\n",__func__,__LINE__,*ptests);
#endif
                    factor = test_factor(*ptests, n);
                    d += (double)t / (double)count / factor;
                    if (0UL<nsw) {
                        d2 += (double)nsw / (double)count / factor;
                    }
                }
                if (j > o+u) { errs++; }
                if ((0U == errs) && (TEST_TYPE_PARTITION == (TEST_TYPE_PARTITION & *ptests))) {
                    t = test_array_partition(pv, o, eql, equ, o+u, size, comparison_function, f, log_arg);
                    if (t != eql) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_partition) for %s ^^^^^ ERROR!!!! line %d\n", pcc, ptest, __LINE__);
                        errs++;
                        erro = (unsigned int)o;
                        if (t <= o+u) errt = t;
                    }
                }
                if ((0U == errs) && (TEST_TYPE_MEDIAN == (TEST_TYPE_MEDIAN & *ptests))) {
                    t = test_array_median(pv, o, j, o+u, size, comparison_function, f, log_arg);
                    if (t != j) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_median) for %s ^^^^^ ERROR!!!! line %d\n", pcc, ptest, __LINE__);
                        errs++;
                        erro = o;
                        if (t <= o+u) errt = t;
                    } else if (j != k) k = j-o; /* some median functions do not move data */
#if 0 /* XXX would need to compare  data */
                    if ((emin > k) || (emax < k)) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s failed for %s: emin %d !<= array[o %lu + k %lu] %d !<= emax %d ^^^^^ ERROR!!!! line %d\n", pcc, ptest, emin, o, k, ik, emax, __LINE__);
                        if (0U == errs) errs++;
                        erro = o;
                    }
#endif
                }
                if ((0U == errs) && (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests))) {
                    t = test_array_sort(pv, o, o+u, size, comparison_function, distinct, logger, log_arg);
                    if (t != o+u) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", pcc, ptest);
                        errs++;
                        erro = (unsigned int)o;
                        if (t <= o+u) errt = t;
                    }
                }
                if (0U < errs) break;
                switch (testnum) {
                    case TEST_SEQUENCE_COMBINATIONS :
                        if (NULL != fp) { for (t=0UL; t<n; t++) { fputc('\b', fp); fflush(fp); } }
                    break;
                }
            }
            switch (testnum) {
                case TEST_SEQUENCE_COMBINATIONS :
                    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); fp=NULL; }
                break;
            }
            if (0U < errs) { /* error diagnostics */
                (V)fprintf(stderr, "%s returned index %lu, range %lu through %lu\n", pcc, j, eql, equ);
                if (eql < o)
                    (V)fprintf(stderr, "ERROR ^^^^^: %s failed (eql %lu < o %lu) for %s ^^^^^ ERROR!!!!\n", pcc, eql, o, ptest);
                if (equ > o+u)
                    (V)fprintf(stderr, "ERROR ^^^^^: %s failed (equ %lu < o+u %lu) for %s ^^^^^ ERROR!!!!\n", pcc, equ, o+u, ptest);
                if (MAX_ARRAY_PRINT > n) {
                    switch (type) {
                        case DATA_TYPE_LONG :
                            if (0UL != errt) print_long_array(pv, errt-1UL, errt+1UL);
                            print_long_array(pv, erro, erro+u);
                        break;
                        case DATA_TYPE_INT :
                            if (0UL != errt) print_int_array(pv, errt-1UL, errt+1UL);
                            print_int_array(pv, erro, erro+u);
                        break;
                        case DATA_TYPE_DOUBLE :
                            if (0UL != errt) print_double_array(pv, errt-1UL, errt+1UL);
                            print_double_array(pv, erro, erro+u);
                        break;
                        case DATA_TYPE_POINTER :
                            if (0UL != errt) print_indirect_keys(pv, errt-1UL, errt+1UL);
                            else print_indirect_keys(pv, erro, erro+u);
                        break;
                        case DATA_TYPE_STRUCT :
                        /*FALLTHROUGH*/
                        case DATA_TYPE_STRING :
                            if (0UL != errt) print_big_array_keys(pv, errt-1UL, errt+1UL);
                            print_big_array_keys(pv, erro, erro+u);
                        break;
                    }
                } else {
                    switch (type) {
                        case DATA_TYPE_LONG :
                            if (0UL != errt) print_long_array(pv, errt-1UL, errt+1UL);
                            else print_long_array(pv, 0UL, 2UL);
                            print_long_array(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                        case DATA_TYPE_INT :
                            if (0UL != errt) print_int_array(pv, errt-1UL, errt+1UL);
                            else print_int_array(pv, 0UL, 2UL);
                            print_int_array(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                        case DATA_TYPE_DOUBLE :
                            if (0UL != errt) print_double_array(pv, errt-1UL, errt+1UL);
                            else print_double_array(pv, 0UL, 2UL);
                            print_double_array(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                        case DATA_TYPE_POINTER :
                            if (0UL != errt) print_indirect_keys(pv, errt-1UL, errt+1UL);
                            else print_indirect_keys(pv, 0UL, 2UL);
                            print_indirect_keys(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                        case DATA_TYPE_STRUCT :
                        /*FALLTHROUGH*/
                        case DATA_TYPE_STRING :
                            if (0UL != errt) print_big_array_keys(pv, errt-1UL, errt+1UL);
                            else print_big_array_keys(pv, 0UL, 2UL);
                            print_big_array_keys(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                    }
                }
                if (0U == rpt) {
                    restore_test_data(o,n,refarray,larray,array,darray,big_array,parray);
                    if (MAX_ARRAY_PRINT > n) {
                        (V)fprintf(stderr, "initial %s array:\n", ptest);
                        switch (type) {
                            case DATA_TYPE_LONG :
                                print_long_array(pv, o, o+u);
                            break;
                            case DATA_TYPE_INT :
                                print_int_array(pv, o, o+u);
                            break;
                            case DATA_TYPE_DOUBLE :
                                print_double_array(pv, o, o+u);
                            break;
                            case DATA_TYPE_POINTER :
                                print_indirect_keys(pv, o, o+u);
                            break;
                            case DATA_TYPE_STRUCT :
                            /*FALLTHROUGH*/
                            case DATA_TYPE_STRING :
                                print_big_array_keys(pv, o, o+u);
                            break;
                        }
                    }
                    errs = 0U;
                    rpt++;
                    goto do_test;
                }
            } else {
                (V)fprintf(stdout, "passed\n");
                if (0U!=flags['i']) {
                    if (0U != flags['K']) comment="#";
                    if (0U!=flags['R']) {
                        c1 = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                        (V)printf("%s%s%*.*s%.6G %s comparisons\n", comment, buf, col-c1, col-c1, " ", d, psize);
                    } else {
                        c1 = 1 + snprintf(buf, sizeof(buf), "%s %s vs. %lu %s %s", pcc, pfunc, n, typename, ptest);
                        (V)printf("%s%s%*.*stotal %lu: %.6G %s\n", comment, buf, col-c1, col-c1, " ", d, psize);
                    }
                    if (0UL<nsw) {
                        c2 = 1 + snprintf(buf2, sizeof(buf2), "%lu", n);
                        (V)printf("%s%s%*.*s%.6G %s swaps\n", comment, buf2, col-c2, col-c2, " ", d2, psize);
                    }
                }
                fflush(stdout);
            }
        } /* run test if no errors */
    } /* generate test sequence and run test if no errors */
    return errs;
}

static unsigned int timing_test(int type, size_t size, long *refarray, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, int (*comparison_function)(const void *, const void *), const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count,  unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags, double **pwarray, double **puarray, double **psarray, size_t *marray, size_t *pdn)
{
    char buf[256], buf2[256], buf3[256], buf4[256], buf5[256], buf6[256],
        buf7[256], buf8[256], buf9[256], buf10[256], buf11[256], buf12[256],
        buf13[256], buf14[256], buf15[256], buf16[256], buf17[256], buf18[256],
        buf19[256], buf20[256], buf21[256], buf22[256], buf23[256], buf24[256],
        buf25[256], buf26[256], buf27[256], buf28[256], buf29[256], buf30[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="", *psize;
    const char *comment="";
    int c, emin, emax, i, odebug;
    unsigned int errs=0U, rpt=flags['d'];
    size_t b[10], k, *karray=NULL, m, nk, ocount=count, t, u, x, y;
    size_t best_m
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , j
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , o
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , worst_m
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        ;
    FILE *fp
#if SILENCE_WHINEY_COMPILERS
        = NULL
#endif
        ;
    size_t carray[MAX_PERMUTATION_SIZE], ct, nc;
    int *pint;
    long l, *plong;
    struct big_struct *pbs;
    auto struct rusage rusage_start, rusage_end;
    auto struct timespec timespec_start, timespec_end, timespec_diff;
    double best_s=9.9e99, best_u=9.9e99, best_w=9.9e99, d, factor, test_s,
        tot_s=0.0, test_u, tot_u=0.0, test_w, tot_w=0.0, worst_s=0.0,
        worst_u=0.0, worst_w=0.0;
    void *pv;
    uint64_t max_val;

    if (0U == ((0x01U << testnum) & *psequences)) return ++errs;
    pcc = function_name(func, n);
    if (NULL == pcc) {
        errs++;
    } else {
        pfunc = function_type(func, ptests);
        if (NULL == pfunc) {
            errs++;
        }
    }
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
    if (0U == errs) {
        switch (type) {
            case DATA_TYPE_LONG :
                pv=larray;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_INT :
                pv=array;
                max_val = INT_MAX;
            break;
            case DATA_TYPE_DOUBLE :
                pv=darray;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_POINTER :
                pv=parray;
                max_val = INT_MAX;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                pv=big_array;
                max_val = INT_MAX;
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized data type %u\n", __func__, source_file, __LINE__, type);
            return ++errs;
        }
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
        ptest = sequence_name(testnum);
        u = n - 1UL;
        k = u >>1;
        /* generated test sequences */
        emin = emax = (int)k;
        if (0 != is_even(u)) {
            emin--; emax++;
        }
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_SORTED :             /*FALLTHROUGH*/
            case TEST_SEQUENCE_ROTATED :            /*FALLTHROUGH*/
            case TEST_SEQUENCE_REVERSE :
            break;
            case TEST_SEQUENCE_ORGAN_PIPE :          /*FALLTHROUGH*/
            case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
                emin = (int)u / 4 - 1; emax = (int)u / 4 + 1;
            break;
            case TEST_SEQUENCE_SAWTOOTH :
                m = sawtooth_modulus(n,NULL,log_arg);
                emin = (int)m / 2 - 1; emax = (int)m / 2 + 1;
            break;
            case TEST_SEQUENCE_CONSTANT :
                emin = CONSTANT_VALUE; emax = CONSTANT_VALUE;
            break;
            case TEST_SEQUENCE_DUAL_PIVOT_KILLER :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MEDIAN3KILLER :
                emin--; emin--; emax++; emax++;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_HISTOGRAM :                /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                emin = 0 ; emax = (int)u;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED : /*FALLTHROUGH*/
                emin = 0 ; emax = (int)snlround(sqrt((double)u),NULL,NULL);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
                emin = EXPECTED_RANDOM_MIN; emax = EXPECTED_RANDOM_MAX; /* XXX might not work for long */
            break;
            case TEST_SEQUENCE_MANY_EQUAL_LEFT :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_RIGHT :    /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                if (0 != is_even(n)) { emin = CONSTANT_VALUE ; } else { emin = CONSTANT_VALUE + 10; }
                emax = CONSTANT_VALUE + 10 + (int)u / 4 + 1;
            break;
            case TEST_SEQUENCE_BINARY :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_TERNARY :             /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
                emin = 0; emax = 1;
            /*FALLTHROUGH */
            case TEST_SEQUENCE_RANDOM_DISTINCT :     /*FALLTHROUGH */
            case TEST_SEQUENCE_JUMBLE :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH */
            case TEST_SEQUENCE_ADVERSARY :
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
        if (0U==errs) {
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    nc=MAX_PERMUTATION_SIZE;
                    if (n>nc) return ++errs;
                    if (nc>n) nc=n;
                    count = factorial(nc);
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    nc=MAX_COMBINATION_SIZE;
                    if (n>=nc) return ++errs;
                    if (nc>n) nc=n;
                    count = 0x01UL << nc;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
            }
            if (100000000000UL<count) return ++errs; /* malloc will probably fail for statistics arrays */
            /* preparation */
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
            if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
            *psarray=realloc(*psarray, sizeof(double)*count);
            if (NULL==*psarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                return errs;
            }
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
            *puarray=realloc(*puarray, sizeof(double)*count);
            if (NULL==*puarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                return errs;
            }
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
            *pwarray=realloc(*pwarray, sizeof(double)*count);
            if (NULL==*pwarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                return errs;
            }
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
#if DEBUG_CODE
            if (2<debug) if (NULL!=f) f(LOG_INFO, log_arg, "%s %s line %d: %s %s %s", __func__, source_file, __LINE__, pcc, typename, pfunc);
#endif /* DEBUG_CODE */
            /* prepare karray before timing for selection */
            switch (func) {
                case FUNCTION_QSELECT :
                    if (TEST_TYPE_SORT != (TEST_TYPE_SORT & *ptests)) {
                        karray = malloc(selection_nk*sizeof(size_t));
                        if (NULL==karray) return ++errs;
                        if ((1UL==selection_nk)&&(flags['m']=='m')) { /* special-case: 1 or 2 median(s) */
                            karray[0] = k; /* lower-median */
                            karray[1] = ((u+1UL)>>1); /* upper-median */
                            nk = 2UL;
                        } else {
                            nk = selection_nk;
                            switch (flags['m']) {
                                case 'b' : /* bottom */ /*FALLTHROUGH*/
                                case 'l' : /* left or low */
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = (t<n)?t:u;
                                break;
                                case 'm' : /* middle */
                                    x = (nk>>1);
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = (x<k+t)?((k+t<n+x)?k+t-x:u):0UL;
                                break;
                                case 'r' : /* right */ /*FALLTHROUGH*/
                                case 't' : /* top */
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                break;
                                case 's' : /* separated */
                                    x = (nk>>1);
                                    for (t=0UL; t<x; t++)
                                        karray[t] = (t<n)?t:u;
                                    for (; t<nk; t++)
                                        karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                break;
                                case 'd' : /*FALLTHROUGH*/
                                default : /* distributed */
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = n * (t+1UL) / (nk+1UL);
                                break;
                            }
                        }
#if DEBUG_CODE
if (0<debug) {
(V)fprintf(stderr, "order statistic ranks: %lu", karray[0]);
for (t=1UL; t<nk; t++)
(V)fprintf(stderr, ", %lu", karray[t]);
(V)fprintf(stderr, "\n");
}
#endif
                    }
                break;
            }
            /* run tests */
            nrepivot=npartitions=0UL;
            for (m=o=*pdn=0UL; m<count; m++) {
                test_s = test_u = test_w = 0.0;
                switch (testnum) {
                    case TEST_SEQUENCE_ADVERSARY :
                        if (0UL==m) {
                            /* generate new test sequence */
                            odebug=debug, debug=0;
                            initialize_antiqsort(n, pv, type, size, refarray);
                            /* run sorting function (even if selection is specified)
                               against adversary to generate adverse sequence in
                               refarray, without instrumented comparisons
                            */
                            switch (func) {
                                case FUNCTION_QSELECT :
#if 1
                                    if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests)) {
                                        QSEL(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                    } else { /* SELECTION */
                                        QSEL(pv,o,o+u,size,aqcmp,karray,0UL,nk,0U);
                                    }
#else
                                    QSEL(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
#endif
                                break;
                                case FUNCTION_BMQSORT :
                                    BMQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_IBMQSORT :
                                    IBMQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_MBMQSORT :
                                    MBMQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_NBQSORT :
                                    NBQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_GLQSORT :
                                    GLQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_WQSORT :
                                    WQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SQSORT :
                                    SQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_QSORT :
                                    QSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_QSORT_WRAPPER :
                                    qsort_wrapper((char *)pv+size*o,u+1UL,size,aqcmp);
                                break;
                                case FUNCTION_YQSORT :
                                    YQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_DPQSORT :
                                    DPQSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_HEAPSORT :
                                    HEAPSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_INTROSORT :
                                    INTROSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_ISORT :
                                    ISORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SELSORT :
                                    SELSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SHELLSORT :
                                    SHELLSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_NETWORKSORT :
                                    NETWORKSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SQRTSORT :
                                    SQRTSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_LOGSORT :
                                    LOGSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SMOOTHSORT :
                                    SMOOTHSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_P9QSORT :
                                    P9QSORT(pv,o,o+u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                            }
                            if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
                            debug=odebug;
                        } else
                            restore_test_data(0UL,n,refarray,larray,array,darray,
                                big_array,parray);
                            /* refarray should now contain an appropriate adverse sequence */
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,n);
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* test number */
                        if (NULL != fp) {
                            c = snul(buf, sizeof(buf), NULL, NULL, m, 2, '0',
                                (int)n, f, log_arg);
                            (V)fprintf(fp, "%s", buf);
                            fflush(fp);
                        }
                        /* combinations of 0, 1 elements for this test */
                        for (ct=0UL; ct<nc; ct++) {
                            refarray[ct] = (long)((m >> (nc-ct-1UL)) & 0x01UL);
                        }
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,n);
                    break;
                    case TEST_SEQUENCE_PERMUTATIONS :
                        if (0UL==m) {
                            /* initial array is sorted sequence */
                            c = generate_long_test_arrays(refarray,
                                n, TEST_SEQUENCE_SORTED, max_val, f, log_arg);
                            if (0 > c) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                    __func__, source_file, __LINE__, c);
                                return ++errs;
                            } else if (0 < c) {
                                return ++errs;
                            }
                            /* initialize for permutations */
                            for (ct=nc; 0UL<ct; carray[--ct]=0UL) ;
                        } else {
                            permute(refarray,0UL,u,carray,&ct);
                        }
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,n);
                    break;
                    case TEST_SEQUENCE_STDIN :               /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SORTED :              /*FALLTHROUGH*/
                    case TEST_SEQUENCE_REVERSE :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ORGAN_PIPE :          /*FALLTHROUGH*/
                    case TEST_SEQUENCE_INVERTED_ORGAN_PIPE : /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ROTATED :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SAWTOOTH :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_CONSTANT :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MEDIAN3KILLER :       /*FALLTHROUGH*/
                    case TEST_SEQUENCE_DUAL_PIVOT_KILLER :   /*FALLTHROUGH*/
                    case TEST_SEQUENCE_JUMBLE :
                        /* computed, non-randomized sequences never change */
                        if (0UL==m) {
                            c=generate_long_test_arrays(refarray,
                                n, testnum, max_val, f, log_arg);
                            if (0 > c) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                    __func__, source_file, __LINE__, c);
                                return ++errs;
                            } else if (0 < c) {
                                return errs;
                            }
                        } else
                            restore_test_data(0UL,n,refarray,larray,array,darray,
                                big_array,parray);
                        /* copy test sequence to alternates */
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,
                            n);
                    break;
                    default :
                        /* generate new test sequence */
                        c=generate_long_test_arrays(refarray,
                            n, testnum, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        duplicate_test_data(refarray,larray,array,darray,big_array,parray, 0UL,
                            n);
                    break;
                }
                /* run and time the test */
                (V)getrusage(RUSAGE_SELF,&rusage_start);
                (V)clock_gettime(CLOCK_REALTIME,&timespec_start);
                switch (func) {
                    case FUNCTION_QSELECT :
                        if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests)) {
                            QSEL(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                        } else { /* SELECTION */
                            QSEL(pv,o,o+u,size,comparison_function,karray,0UL,nk,rpt);
                        }
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_IBMQSORT :
                        IBMQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MBMQSORT :
                        MBMQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_WQSORT :
                        WQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQSORT :
                        SQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
                        qsort_wrapper((char *)pv+size*o,u+1UL,size,comparison_function);
                    break;
                    case FUNCTION_YQSORT :
                        YQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_DPQSORT :
                        DPQSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_HEAPSORT :
                        HEAPSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_INTROSORT :
                        INTROSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_ISORT :
                        ISORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SELSORT :
                        SELSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SHELLSORT :
                        SHELLSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NETWORKSORT :
                        NETWORKSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQRTSORT :
                        SQRTSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_LOGSORT :
                        LOGSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SMOOTHSORT :
                        SMOOTHSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_P9QSORT :
                        P9QSORT(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                    break;
                }
                (V)getrusage(RUSAGE_SELF,&rusage_end);
                (V)clock_gettime(CLOCK_REALTIME,&timespec_end);
                switch (testnum) {
                    case TEST_SEQUENCE_COMBINATIONS :
                        if (NULL != fp) { for (t=0UL; t<n; t++) { fputc('\b', fp); fflush(fp); } }
                    break;
                }
                test_u += (double)(rusage_end.ru_utime.tv_sec - rusage_start.ru_utime.tv_sec) + 1.0e-6 * (double)(rusage_end.ru_utime.tv_usec - rusage_start.ru_utime.tv_usec);
                if (test_u < 0.0) test_u = 0.0;
                tot_u += test_u;
                if (test_u > worst_u) worst_u = test_u, worst_m = m;
                if (test_u < best_u) best_u = test_u, best_m = m;
                test_s += (double)(rusage_end.ru_stime.tv_sec - rusage_start.ru_stime.tv_sec) + 1.0e-6 * (double)(rusage_end.ru_stime.tv_usec - rusage_start.ru_stime.tv_usec);
                if (test_s < 0.0) test_s = 0.0;
                tot_s += test_s;
                if (test_s > worst_s) worst_s = test_s;
                if (test_s < best_s) best_s = test_s;
                timespec_subtract(&timespec_end, &timespec_start, &timespec_diff);
                test_w = timespec_to_double(&timespec_diff);
                if (test_w < 0.0) test_w = 0.0;
                tot_w += test_w;
                if (test_w > worst_w) worst_w = test_w;
                if (test_w < best_w) best_w = test_w;
//(V)fprintf(stderr, "%s line %d: errs=%u\n",__func__,__LINE__,errs);
                if (0UL<count) {
//(V)fprintf(stderr, "%s line %d: test_u=%G, dn=%lu\n",__func__,__LINE__,test_u,*pdn);
                    A(*pdn<count);
                    (*puarray)[*pdn] = test_u;
                    (*psarray)[*pdn] = test_s;
                    (*pwarray)[*pdn] = test_w;
                    (*pdn)++;
//(V)fprintf(stderr, "%s line %d: test_u=%G, dn=%lu\n",__func__,__LINE__,test_u,*pdn);
                }
                if (0U < errs) break;
                if ((tot_w > timeout)&&(m+1<count)) count = ++m; /* horrible performance; break out of loop */
            } /* execute test count times */
            /* cleanup */
            if (NULL != karray) {
                free(karray);
                karray = NULL;
            }
            switch (testnum) {
                case TEST_SEQUENCE_COMBINATIONS :
                    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); fp=NULL; }
                /*FALLTHROUGH*/
                case TEST_SEQUENCE_PERMUTATIONS :
                    ocount=count; /* avoid "terminated early" warning */
                break;
            }
            /* find order statistics for times */
            if (0UL<*pdn) {
                /* marray may be modified by quickselect (sorting, duplicate removal) */
                b[0] = marray[0] = 0UL;                  /* min */
                b[1] = marray[1] = *pdn/50UL;            /* 2%ile */
                b[2] = marray[2] = *pdn/11UL;            /* 9%ile */
                b[3] = marray[3] = (*pdn)>>2;            /* 1st quartile */
//(V)fprintf(stderr, "%s line %d: dn=%lu, n=%lu, u=%lu, marray[3]=%lu, count=%lu\n", __func__, __LINE__, *pdn, n, u, marray[3], count);
                b[4] = marray[4] = (*pdn-1UL)>>1;        /* lower median */
                b[5] = marray[5] = (*pdn>>1);            /* upper median */
                b[6] = marray[6] = *pdn-1UL-((*pdn)>>2); /* 3rd quartile */
                b[7] = marray[7] = *pdn-1UL-(*pdn/11UL); /* 91%ile */
                b[8] = marray[8] = *pdn-1UL-(*pdn/50UL); /* 98%ile */
                b[9] = marray[9] = *pdn-1UL;             /* max */
#if DEBUG_CODE
if (3<debug) (V)fprintf(stderr, "%s line %d: dn=%lu, n=%lu, u=%lu, marray[4]=%lu, marray[5]=%lu\n", __func__, __LINE__, *pdn, n, u, marray[4], marray[5]);
#endif /* DEBUG_CODE */
#if 0
                /* use uninstrumented quickselect with uninstrumented comparison function */
                quickselect((void *)(*puarray),*pdn,sizeof(double),doublecmp,
                    marray,10UL);
                quickselect((void *)(*psarray), *pdn, sizeof(double), doublecmp,
                    marray, 10UL);
                quickselect((void *)(*pwarray),*pdn,sizeof(double),doublecmp,
                    marray, 10UL);
#else
                /* use internal quickselect with uninstrumented comparison function */
                t=nsw,x=npartitions,y=nrepivot;
                quickselect_internal((void *)(*puarray),*pdn,sizeof(double),doublecmp,
                    marray,10UL
#if SAVE_PARTIAL
                   ,NULL,NULL
#endif
                    );
                quickselect_internal((void *)(*psarray), *pdn, sizeof(double), doublecmp,
                    marray, 10UL
#if SAVE_PARTIAL
                   ,NULL,NULL
#endif
                    );
                quickselect_internal((void *)(*pwarray),*pdn,sizeof(double),doublecmp,
                    marray, 10UL
#if SAVE_PARTIAL
                   ,NULL,NULL
#endif
                    );
                nsw=t,npartitions=x,nrepivot=y;
#endif
#if DEBUG_CODE
if (3<debug) (V)fprintf(stderr, "%s line %d: quickselect returned\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                if (0U==flags[':']) { /* silent for optimization runs */
                    if (0U != flags['K']) comment="#";
                    if ((0U == errs)) {
                        /* summarize test results */
                        if (count == ocount) {
                            factor = test_factor(~(TEST_TYPE_ADVERSARY) & *ptests, n);
                            psize = test_size(~(TEST_TYPE_ADVERSARY) & *ptests);
                        } else {
                            factor = test_factor(*ptests, n);
                            psize = test_size(*ptests);
                        }
                        buf29[0] = '\0';
                        if (0UL<npartitions)
                            (V)snul(buf29, sizeof(buf29), NULL, 1UL==npartitions/count?" partition":" partitions", npartitions/count, 10, '0', 1, f, log_arg);
                        buf30[0] = '\0';
                        if (0UL<nrepivot)
                            (V)snul(buf30, sizeof(buf30), NULL, 1UL==nrepivot/count?" repivot":" repivots", nrepivot/count, 10, '0', 1, f, log_arg);
                        /* mean times */
                        test_u = tot_u / (double)m;
                        (V)sng(buf, sizeof(buf), NULL, NULL, test_u / factor, -6, 3, f, log_arg);
                        (V)sng(buf6, sizeof(buf6), NULL, NULL, test_u, -6, 3, f, log_arg);
#if DEBUG_CODE
if (3<debug) (V)fprintf(stderr, "%s line %d: tot_u=%G, factor=%G, count=%lu, mean=%G (%s)\n", __func__, __LINE__, tot_u, factor, count, test_u, buf);
#endif /* DEBUG_CODE */
                        test_s = tot_s / (double)m;
                        (V)sng(buf7, sizeof(buf7), NULL, NULL, test_s / factor, -4, 3, f, log_arg);
                        (V)sng(buf8, sizeof(buf8), NULL, NULL, test_s, -4, 3, f, log_arg);
                        test_w = tot_w / (double)m;
                        (V)sng(buf9, sizeof(buf9), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
                        (V)sng(buf10, sizeof(buf10), NULL, NULL, test_w, -4, 3, f, log_arg);
                        c = 1 + snprintf(buf3, sizeof(buf3), "%s %s of %lu %s%s: %s: (%lu * %lu)", pcc, pfunc, n, typename, (n==1)?"":"s", ptest, n, count);
                        (V)sng(buf4, sizeof(buf4), NULL, NULL, tot_u, -4, 3, f, log_arg);
                        (V)sng(buf15, sizeof(buf15), NULL, NULL, tot_s, -4, 3, f, log_arg);
                        (V)sng(buf16, sizeof(buf16), NULL, NULL, tot_w, -4, 3, f, log_arg);
                        test_u = ((*puarray)[b[4]] + (*puarray)[b[5]]) / 2.0; /* median user */
                        (V)sng(buf5, sizeof(buf5), NULL, NULL, test_u, -4, 3, f, log_arg);
                        (V)sng(buf17, sizeof(buf17), NULL, NULL, (*puarray)[b[0]], -4, 3, f, log_arg); /* min user */
                        (V)sng(buf18, sizeof(buf18), NULL, NULL, (*puarray)[b[9]], -4, 3, f, log_arg); /* max user */
                        test_s = ((*psarray)[b[4]] + (*psarray)[b[5]]) / 2.0; /* median sys */
                        (V)sng(buf11, sizeof(buf11), NULL, NULL, test_s, -4, 3, f, log_arg);
                        (V)sng(buf19, sizeof(buf19), NULL, NULL, (*psarray)[b[0]], -4, 3, f, log_arg); /* min sys */
                        (V)sng(buf20, sizeof(buf20), NULL, NULL, (*psarray)[b[9]], -4, 3, f, log_arg); /* max sys */
                        test_w = ((*pwarray)[b[4]] + (*pwarray)[b[5]]) / 2.0; /* median wall */
                        (V)sng(buf13, sizeof(buf13), NULL, NULL, test_w, -4, 3, f, log_arg);
                        (V)sng(buf21, sizeof(buf21), NULL, NULL, (*pwarray)[b[0]], -4, 3, f, log_arg); /* min wall */
                        (V)sng(buf22, sizeof(buf22), NULL, NULL, (*pwarray)[b[9]], -4, 3, f, log_arg); /* max wall */
                        (V)sng(buf2, sizeof(buf2), NULL, NULL, test_u / factor, -4, 3, f, log_arg);
                        (V)sng(buf12, sizeof(buf12), NULL, NULL, test_s / factor, -4, 3, f, log_arg);
                        (V)sng(buf14, sizeof(buf14), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
#if DEBUG_CODE
if (3<debug) (V)fprintf(stderr, "%s line %d: factor=%G, count=%lu, dn=%lu, median=%G (%s)\n", __func__, __LINE__, factor, count, *pdn, test_u, buf2);
#endif /* DEBUG_CODE */
                        if (0U!=flags['B']) {
                            /* wall-clock box plot data: 2%, 9%, 1/4, median, 3/4, 91%, 98% */
                            (V)sng(buf23, sizeof(buf23), NULL, NULL, (*pwarray)[b[1]], -4, 3, f, log_arg); /* 2% wall */
                            (V)sng(buf24, sizeof(buf24), NULL, NULL, (*pwarray)[b[2]], -4, 3, f, log_arg); /* 9% wall */
                            (V)sng(buf25, sizeof(buf25), NULL, NULL, (*pwarray)[b[3]], -4, 3, f, log_arg); /* 1/4 wall */
                            (V)sng(buf26, sizeof(buf26), NULL, NULL, (*pwarray)[b[6]], -4, 3, f, log_arg); /* 3/4 wall */
                            (V)sng(buf27, sizeof(buf27), NULL, NULL, (*pwarray)[b[7]], -4, 3, f, log_arg); /* 91% wall */
                            (V)sng(buf28, sizeof(buf28), NULL, NULL, (*pwarray)[b[8]], -4, 3, f, log_arg); /* 98% wall */
                            (V)printf("%s%s %s %s box plot: %s %s %s %s %s %s %s\n",comment,pcc,pfunc,ptest,buf23,buf24,buf25,buf13,buf26,buf27,buf28);
                        }
                        if (0U!=flags['R']) {
                            c = 1 + snprintf(buf2, sizeof(buf2), "%lu", n);
                            (V)printf("%s%s%*.*s%s user seconds\n", comment, buf2, col-c, col-c, " ", 0.0!=test_u?buf5:buf6);
                            (V)printf("%s%s%*.*s%s system seconds\n", comment, buf2, col-c, col-c, " ", 0.0!=test_s?buf11:buf8);
                            (V)printf("%s%s%*.*s%s wall-clock seconds\n", comment, buf2, col-c, col-c, " ", 0.0!=test_w?buf13:buf10);
                        } else {
                            (V)printf("%s%s%*.*snormalized unit user times: %s (mean), %s (median), per %s, total user time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf, buf2, psize, buf4, (count!=ocount)?" (terminated early)":"", buf6, buf5);
                            if (0UL<*pdn) {
                                /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                                d = tot_u / test_u / (double)m; /* ratio of mean to median */
                                if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                                if (d > DISCREPANCY_THRESHOLD) (V)fprintf(stderr,"%s%s %s User time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",comment,pcc,ptest,buf17,buf6,buf18,buf5,100.0*(d-1.0));
                            }
                            (V)printf("%s%s%*.*snormalized unit system times: %s (mean), %s (median), per %s, total system time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf7, buf12, psize, buf15, (count!=ocount)?" (terminated early)":"", buf8, buf11);
                            (V)printf("%s%s%*.*snormalized unit wall clock times: %s (mean), %s (median), per %s, total wall clock time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf9, buf14, psize, buf16, (count!=ocount)?" (terminated early)":"", buf10, buf13);
                            /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                            d = tot_w / test_w / (double)m; /* ratio of mean to median */
                            if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                            if (d > DISCREPANCY_THRESHOLD) (V)fprintf(stderr,"%s%s %s Wall-clock time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",comment,pcc,ptest,buf21,buf10,buf22,buf13,100.0*(d-1.0));
                            switch (testnum) {
                                case TEST_SEQUENCE_PERMUTATIONS : /*FALLTHROUGH*/
                                case TEST_SEQUENCE_COMBINATIONS :
                                    (V)snprintf(buf5, sizeof(buf5), " s/ %s %s\n", psize, typename);
                                    (V)sng(buf, sizeof(buf), NULL, buf5, best_u / factor, -4, 3, f, log_arg);
                                    c = 1 + snprintf(buf2, sizeof(buf2), "%s %s of %lu %s%s: %s: %s(%lu)", pcc, pfunc, n, typename, (n==1)?"":"s", ptest, "best", best_m);
                                    (V)printf("%s%s%*.*s%s", comment, buf2, col-c, col-c, " ", buf);
                                    (V)sng(buf, sizeof(buf), NULL, buf5, worst_u / factor, -4, 3, f, log_arg);
                                    c = 1 + snprintf(buf2, sizeof(buf2), "%s %s of %lu %s%s: %s: %s(%lu)", pcc, pfunc, n, typename, (n==1)?"":"s", ptest, "worst", worst_m);
                                    (V)printf("%s%s%*.*s%s", comment, buf2, col-c, col-c, " ", buf);
                                break;
                                case TEST_SEQUENCE_ADVERSARY :
                                    c = snul(buf, sizeof(buf), NULL, " comparisons", antiqsort_ncmp, 10, '0', 1, f, log_arg);
                                    c = snul(buf2, sizeof(buf2), NULL, " solid", (unsigned long)antiqsort_nsolid, 10, '0', 1, f, log_arg);
                                    c = 1 + snprintf(buf3, sizeof(buf3), "%s %s vs. %s:", pcc, pfunc, ptest);
                                    (V)printf("%s%s%*.*s%s, %s: %.6G %s\n", comment, buf3, col-c, col-c, " ", buf, buf2, (double)antiqsort_ncmp / factor, psize);
                                break;
                            }
                        }
                    }
                    if (0U != flags['i']) {
                        c = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                        t=nlt+neq+ngt;
                        d = (double)t / (double)count / factor;
                        if (0U!=flags['R']) {
                            (V)printf("%s%s%*.*s%.6G %s comparisons (%.0f)\n", comment, buf, col-c, col-c, " ", d, psize, (double)t/(double)count);
                        } else {
                            i = 1 + snprintf(buf3, sizeof(buf3), "%s %s vs. %lu %s %s comparison counts:", pcc, pfunc, n, typename, ptest);
                            (V)printf("%s%s%*.*s%lu < (%.1f%%), %lu == (%.1f%%), %lu > (%.1f%%), total %lu: %.6G %s\n",
                                comment, buf3, col-i, col-i, " ", nlt, 100.0*(double)nlt/(double)t, neq,
                                100.0*(double)neq/(double)t, ngt, 100.0*(double)ngt/(double)t,
                                t, d, psize);
                        }
                        if (0UL<nsw) {
                            d = (double)nsw / (double)count / factor;
                            (V)printf("%s%s%*.*s%.6G %s swaps (%.0f)\n", comment, buf, col-c, col-c, " ", d, psize, (double)nsw/(double)count);
                        }
                        if ('\0'!=buf29[0])
                            (V)printf("%s%s%*.*s%s\n",comment,buf,col-c,col-c," ",buf29);
                        if ('\0'!=buf30[0])
                            (V)printf("%s%s%*.*s%s (%.3f%%)\n",comment,buf,col-c,col-c," ",buf30,100.0*(double)nrepivot/(double)npartitions);
                    }
                    if (0U != flags['K']) {
                        switch (testnum) {
                            case TEST_SEQUENCE_ADVERSARY :
                                (V)printf("#Adversary sequence:\n");
                                for(j=0UL; j<n; j++) {
                                    (V)printf("%ld\n", refarray[j]);
                                }
                            break;
                            case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                                (V)printf("#Dual-pivot killer sequence:\n");
                                for(j=0UL; j<n; j++) {
                                    /* take copied data from unsorted data set */
                                    switch (type) {
                                        case DATA_TYPE_LONG :
                                            l = big_array[j].l;
                                        break;
                                        default :
                                            l = larray[j];
                                        break;
                                    }
                                    (V)printf("%ld\n", l);
                                }
                            break;
                            case TEST_SEQUENCE_MEDIAN3KILLER :
                                (V)printf("#Median-of-3 killer sequence:\n");
                                for(j=0UL; j<n; j++) {
                                    /* take copied data from unsorted data set */
                                    switch (type) {
                                        case DATA_TYPE_LONG :
                                            l = big_array[j].l;
                                        break;
                                        default :
                                            l = larray[j];
                                        break;
                                    }
                                    (V)printf("%ld\n", l);
                                }
                            break;
                        }
                    }
                    fflush(stdout);
                }
            } /* test result summary if no errors */
        } /* test if no errors */
    } /* generate test sequence and test if no errors */
    return errs;
}

static unsigned int correctness_tests(int type, size_t size, long *refarray, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, const char *prog, unsigned int func, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags, double **pwarray, double **puarray, double **psarray, size_t *marray, size_t *pdn)
{
    unsigned int errs=0U, testnum;
    int (*comparison_function)(const void *, const void *);
    const char *fname = function_name(func, n), *ftype = function_type(func, ptests);

    /* generated test sequences */
    for (testnum=0U; TEST_SEQUENCE_COUNT>testnum; testnum++) {
        /* MEDIAN3KILLER only for n >= 7 */
        /* SAWTOOTH, MANY_EQUAL only for n >= 5 */
        /* permutations, combinations only for thorough tests */
        /* no adversary test */
        if (0==valid_test(TEST_TYPE_CORRECTNESS, testnum, n)) continue;
        switch (testnum) {
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
            break;
        }
        if (0U == ((0x01U << testnum) & *psequences)) continue;
        switch (type) {
            case DATA_TYPE_LONG :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = ilongcmp;
                else
                    comparison_function = longcmp;
            break;
            case DATA_TYPE_INT :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = iintcmp;
                else
                    comparison_function = intcmp;
            break;
            case DATA_TYPE_POINTER :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = iindcmp;
                else
                    comparison_function = indcmp;
            break;
            case DATA_TYPE_STRING :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = ibig_struct_strcmp;
                else
                    comparison_function = big_struct_strcmp;
            break;
            case DATA_TYPE_STRUCT :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = ibig_struct_intcmp;
                else
                    comparison_function = big_struct_intcmp;
            break;
            case DATA_TYPE_DOUBLE :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = idoublecmp;
                else
                    comparison_function = doublecmp;
            break;
        }
        if (NULL!=f) {
            if (0U!=flags['K']) fprintf(stderr, "// ");
            f(LOG_INFO, log_arg, "%s: %s %s %s correctness test: %lu %lu", prog, fname, ftype, typename, n, count);
        }
        errs += correctness_test(type, size, refarray, larray, array, darray, big_array, parray, typename, comparison_function, prog, func, testnum, n, count, psequences, ptests, col, f, log_arg, flags);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}

static unsigned int timing_tests(int type, size_t size, long *refarray, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, const char *prog, unsigned int func, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags, double **pwarray, double **puarray, double **psarray, size_t *marray, size_t *pdn)
{
    unsigned int errs=0U, testnum;
    int (*comparison_function)(const void *, const void *);
    const char *fname = function_name(func, n), *ftype = function_type(func, ptests);

    /* generated test sequences */
    for (testnum=0U; TEST_SEQUENCE_COUNT>testnum; testnum++) {
        /* permutations, combinations only for thorough tests */
        if (0==valid_test(TEST_TYPE_TIMING, testnum, n)) continue;
        if (0U == ((0x01U << testnum) & *psequences)) continue;
        switch (type) {
            case DATA_TYPE_LONG :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = ilongcmp;
                else
                    comparison_function = longcmp;
            break;
            case DATA_TYPE_INT :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = iintcmp;
                else
                    comparison_function = intcmp;
            break;
            case DATA_TYPE_POINTER :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = iindcmp;
                else
                    comparison_function = indcmp;
            break;
            case DATA_TYPE_STRING :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = ibig_struct_strcmp;
                else
                    comparison_function = big_struct_strcmp;
            break;
            case DATA_TYPE_STRUCT :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = ibig_struct_intcmp;
                else
                    comparison_function = big_struct_intcmp;
            break;
            case DATA_TYPE_DOUBLE :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = idoublecmp;
                else
                    comparison_function = doublecmp;
            break;
        }
        if ((NULL!=f)&&(0u==flags[':'])) {
            if (0U!=flags['K']) fprintf(stderr, "// ");
            f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: %lu %lu", prog, fname, ftype, typename, sequence_name(testnum), n, count);
        }
        errs += timing_test(type, size, refarray, larray, array, darray, big_array, parray, typename, comparison_function, prog, func, testnum, n, count, psequences, ptests, col, timeout, f, log_arg, flags, pwarray, puarray, psarray, marray, pdn);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}

static
unsigned int test_function(const char *prog, long *refarray, int *array, struct big_struct *big_array, double *darray, long *larray, struct big_struct **parray, unsigned int func, size_t n, size_t count, unsigned int *pcsequences, unsigned int *ptsequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags, double **pwarray, double **puarray, double **psarray, size_t *marray, size_t *pdn)
{
    unsigned char c;
    unsigned int errs=0U;
    int type;
    size_t size;
#if DEBUG_CODE
    const char *fname = function_name(func, n);
#endif
    const char *ftype = function_type(func, ptests), *typename;

#if 0
    if (NULL != big_array) {
        /* test swap functions */
        (big_array[0]).val = 0;
        strcpy((big_array[0]).string, "abracadabra");
        (big_array[1]).val = 1;
        strcpy((big_array[1]).string, "mississippi");
        (big_array[2]).val = 2;
        strcpy((big_array[2]).string, "zygomorphic");
        (V)fprintf(stderr, "swap test: original structures:\n");
        (V)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
        (V)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
        (V)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
        swap2(&(big_array[0]), &(big_array[1]), sizeof(struct big_struct));
        (V)fprintf(stderr, "swap test: swap2:\n");
        (V)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
        (V)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
        (V)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
        swap(&(big_array[0]), &(big_array[2]), 1UL, sizeof(struct big_struct));
        (V)fprintf(stderr, "swap test: swap:\n");
        (V)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
        (V)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
        (V)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
        exchange(big_array, 1UL, 2UL, 1UL, sizeof(struct big_struct));
        (V)fprintf(stderr, "swap test: exchange:\n");
        (V)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
        (V)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
        (V)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
    }
#endif

#if DEBUG_CODE
    if (2<debug) if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: %s %s start: array=%p, big_array=%p, darray=%p", prog, fname, ftype,array,big_array,darray);
#endif
    /* check flags, modify tests as requested */
    /* first pass for test types */
    for (c=0U; 1; c++) {
        /* supported flags */
        switch (c) {
            case 'a' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_ADVERSARY ;
            break;
            case 'C' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_CORRECTNESS ;
            break;
            case 't' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_THOROUGH ;
            break;
            case 'T' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_TIMING ;
            break;
        }
        if (c == UCHAR_MAX) break; /* That's All Folks! */
    }
#if DEBUG_CODE
    if (2<debug) if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: %s %s flags 1st pass completed", prog, fname, ftype);
#endif
    /* second pass for tests with data types */
    for (c=0U; 1; c++) {
        /* supported flags */
        switch (c) {
            case 'A' :
                if (0U != flags[c]) { /* test requested */
                    typename = "string";
                    type = DATA_TYPE_STRING;
                    size = sizeof(struct big_struct);
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'F' :
                if (0U != flags[c]) { /* test requested */
                    typename = "double";
                    type = DATA_TYPE_DOUBLE;
                    size = sizeof(double);
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'I' :
                if (0U != flags[c]) { /* test requested */
                    typename = "integer";
                    type = DATA_TYPE_INT;
                    size = sizeof(int);
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'L' :
                if (0U != flags[c]) { /* test requested */
                    typename = "long";
                    type = DATA_TYPE_LONG;
                    size = sizeof(long);
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'P' :
                if (0U != flags[c]) { /* test requested */
                    typename = "pointer";
                    type = DATA_TYPE_POINTER;
                    size = sizeof(struct big_struct *);
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'S' :
                if (0U != flags[c]) { /* test requested */
                    typename = "structure";
                    type = DATA_TYPE_STRUCT;
                    size = sizeof(struct big_struct);
                } else { /* test not requested */
                    continue;
                }
            break;
            default : /* not a relevant flag */
                if (c != UCHAR_MAX) continue;
            break;
        }
        if (c == UCHAR_MAX) break; /* That's All Folks! */
#if DEBUG_CODE
        if (2<debug) if (NULL!=f) f(LOG_INFO, log_arg, "%s: %s %s flag %c tests", prog, fname, ftype, c);
#endif
        if (0U == errs) {
            if (TEST_TYPE_CORRECTNESS == (TEST_TYPE_CORRECTNESS & *ptests)) {
                errs +=  correctness_tests(type, size, refarray, larray, array, darray, big_array, parray, typename, prog, func, n, count, pcsequences, ptests, col, timeout, f, log_arg, flags, pwarray, puarray, psarray, marray, pdn);
            }
        }
        if (0U == errs) {
            if (TEST_TYPE_TIMING == (TEST_TYPE_TIMING & *ptests)) {
                errs +=  timing_tests(type, size, refarray, larray, array, darray, big_array, parray, typename, prog, func, n, count, ptsequences, ptests, col, timeout, f, log_arg, flags, pwarray, puarray, psarray, marray, pdn);
            }
        }
    }
#if DEBUG_CODE
    if (2<debug) if ((0U==flags['h'])&&(NULL!=f)) {
        if (0U!=flags['K']) fprintf(stderr, "// ");
        f(LOG_INFO, log_arg, "%s: %s %s %s tests completed: errs %u", prog, fname, ftype, typename, errs);
    }
#endif
    return errs;
}

/* print some size statistics to stdout */
static
void print_sizes(const char *prefix, const char *prog)
{
    /* information about thing that may affect sizes: */
    (V)fprintf(stdout, "%s%s: %s (%s %s %s) COMPILER_USED=\"%s\"\n", prefix, prog, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED);
    /* common standard (as of C99) types */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    (V)fprintf(stdout, "%ssizeof(_Bool) = %lu\n", prefix, sizeof(_Bool));
#endif /* C99 */
    (V)fprintf(stdout, "%ssizeof(char) = %lu\n", prefix, sizeof(char));
    (V)fprintf(stdout, "%ssizeof(short) = %lu\n", prefix, sizeof(short));
    (V)fprintf(stdout, "%ssizeof(int) = %lu\n", prefix, sizeof(int));
    (V)fprintf(stdout, "%ssizeof(long) = %lu\n", prefix, sizeof(long));
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    (V)fprintf(stdout, "%ssizeof(long long) = %lu\n", prefix, sizeof(long long));
#endif /* C99 */
    (V)fprintf(stdout, "%ssizeof(float) = %lu\n", prefix, sizeof(float));
    (V)fprintf(stdout, "%ssizeof(double) = %lu\n", prefix, sizeof(double));
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    (V)fprintf(stdout, "%ssizeof(long double) = %lu\n", prefix, sizeof(long double));
    (V)fprintf(stdout, "%ssizeof(float _Complex) = %lu\n", prefix, sizeof(float _Complex));
    (V)fprintf(stdout, "%ssizeof(double _Complex) = %lu\n", prefix, sizeof(double _Complex));
    (V)fprintf(stdout, "%ssizeof(long double _Complex) = %lu\n", prefix, sizeof(long double _Complex));
#endif /* C99 */
    (V)fprintf(stdout, "%ssizeof(void *) = %lu\n", prefix, sizeof(void *));
    (V)fprintf(stdout, "%ssizeof(size_t) = %lu\n", prefix, sizeof(size_t));
    /* structure specific to this test program */
    (V)fprintf(stdout, "%ssizeof(struct big_struct) = %lu\n", prefix, sizeof(struct big_struct));
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int nocmp(void *unused1, void *unused2)
{
    return 0;
}

/* time basic operations and print summary table */
static
void op_tests(const char *prefix, const char *prog, void (*f)(int, void *, const char *,...), void *log_arg)
{
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define OPTYPES 18
#else
# define OPTYPES 11
#endif /* C99 */
#define NOPS   34
    char buf[256], numbuf[64];
    char testmatrix[OPTYPES][NOPS] = {      /* applicability matrix */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnny", /* _Bool */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* char */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnny", /* unsigned char */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* short */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnny", /* unsigned short */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* int */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnny", /* unsigned */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* unsigned long */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnny", /* long long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnny", /* unsigned long long */
#endif /* C99 */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnnny", /* float */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnyyyy", /* double */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnnny", /* long double */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnny", /* float _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnny", /* double _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnny", /* long double _Complex */
#endif /* C99 */
        "ynnnnnnyyyyyyyyynnnnnnnnnnnnnnnnny", /* pointer */
    };
    const char *pcc, *pcc2;
    const char *typename[OPTYPES][2] = {
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "", "_Bool" },
#endif /* C99 */
       { "", "char" }, { "unsigned", "char" }, { "short", "int" },
       { "unsigned", "short" }, { "plain", "int"}, { "unsigned", "int" },
       { "long", "int" }, { "unsigned", "long" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "long", "long" }, { "unsigned", "long long" },
#endif /* C99 */
       { "", "float" }, { "", "double" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "long", "double" }, { "float", "_Complex" },
       { "double", "_Complex" }, { "long double", "_Complex" },
#endif /* C99 */
       { "", "pointer" }
    };
    unsigned int testnum, typenum;
    size_t i, j, k, l;
    double timing[OPTYPES][NOPS];
    FILE *fp;
    auto struct rusage rusage_start, rusage_end;

    /* flush output streams before writing to /dev/tty */
    fflush(stdout); fflush(stderr);
    /* Progress indication to /dev/tty */
    fp = fopen("/dev/tty", "w");
    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
    pcc = "Running tests, please be patient.  ";
    k = 1UL;
    pcc2="\b-\b\\\b|\b/"; /* "spinner" */
    l = 1UL;

    /* run tests, collect timing data */

/* macro to run one operation timing test */
/* Run (if possible) long enough to get sufficient accuracy with resolution of
   one microsecond (from struct rusage).
*/
#define MIN_OPS_TIME     0.02 /* 2*0.000001*10000 */
#define DESIRED_OPS_TIME 2.0*(MIN_OPS_TIME)
#define M(op) timing[typenum][testnum]=0.0;                               \
    if ('y' == testmatrix[typenum][testnum]) {                            \
        double t;                                                         \
        register size_t count, n;                                         \
        for (count=65536UL; 1;) {  /* 2^^16 */                            \
            if (NULL != fp) {                                             \
                for (j=0UL; j<k; j++) {                                   \
                    fputc(*pcc, fp); fflush(fp);                          \
                    if (pcc[1] != '\0') {                                 \
                        pcc++;                                            \
                    } else {                                              \
                        pcc = pcc2;                                       \
                        k = l;                                            \
                    }                                                     \
                }                                                         \
            }                                                             \
            (V)getrusage(RUSAGE_SELF,&rusage_start);                   \
            for (n=0UL; n<count; n++) {                                   \
                op ;                                                      \
            }                                                             \
            (V)getrusage(RUSAGE_SELF,&rusage_end);                     \
            t = ((double)(rusage_end.ru_utime.tv_sec -                    \
            rusage_start.ru_utime.tv_sec) + 1.0e-6 *                      \
            (double)(rusage_end.ru_utime.tv_usec -                        \
            rusage_start.ru_utime.tv_usec));                              \
            if (((MIN_OPS_TIME)<=t)||((SIZE_T_MAX)==count)) {             \
                timing[typenum][testnum]=t/(double)count;                 \
                break;                                                    \
            } else {                                                      \
                if (0.0>t) t=0.0;                                         \
                if (0.0==t) {                                             \
                    if ((SIZE_T_MAX)/256UL>count) count *= 256UL;         \
                    else count=(SIZE_T_MAX);                              \
                } else {                                                  \
                    n=snlround((DESIRED_OPS_TIME)/t,f,log_arg);           \
                    if (2UL>n) n=2UL; /* *1 is NG */                      \
                    if ((SIZE_T_MAX)/n>count) count *= n;                 \
                    else count=(SIZE_T_MAX);                              \
                }                                                         \
            }                                                             \
        }                                                                 \
    }

    /* Integer types: */

/* macro to run tests applicable to integer types */
#define I(type,v1,v2,v3,fcmp) { volatile type x=v1;       \
        volatile type y=v2; volatile type z=v3;           \
        size_t sz=sizeof(type);                           \
        if (0 == z) z++; /* z must not be zero! */        \
        testnum=0U;  M(x=y)                               \
        testnum=1U;  M(x=(y==z))                          \
        testnum=2U;  M(x=(y!=z))                          \
        testnum=3U;  M(x=(y<z))                           \
        testnum=4U;  M(x=(y>=z))                          \
        testnum=5U;  M(x=(y<=z))                          \
        testnum=6U;  M(x=(y>z))                           \
        testnum=7U;  M(x=(y==z)?y:z)                      \
        testnum=8U;  M(x=(y!=z)?y:z)                      \
        testnum=9U;  M(x=(y<z)?y:z)                       \
        testnum=10U; M(x=(y>=z)?y:z)                      \
        testnum=11U; M(x=(y<=z)?y:z)                      \
        testnum=12U; M(x=(y>z)?y:z)                       \
        testnum=13U; M(x++)                               \
        testnum=14U; M(x+=1)                              \
        testnum=15U; M(x=x+1)                             \
        testnum=16U; M(x+=y)                              \
        testnum=17U; M(x=y+z)                             \
        testnum=18U; M(x=y-z)                             \
        testnum=19U; M(x=y*z)                             \
        testnum=20U; M(x=y/z)                             \
        testnum=21U; M(x=y%z)                             \
        testnum=22U; M(x=!y)                              \
        testnum=23U; M(x=~y)                              \
        testnum=24U; M(x=y&z)                             \
        testnum=25U; M(x=y|z)                             \
        testnum=26U; M(x=y^z)                             \
        testnum=27U; M(x=y<<z)                            \
        testnum=28U; M(x=y>>z)                            \
        testnum=29U; M(x=&y-&z)                           \
        testnum=30U; M((x=(type)fcmp(&y,&z)))             \
        testnum=31U; M((swap2(&y,&z,sz)))                 \
        testnum=32U; M((swapper(&y,&z,1UL,sz)))           \
        testnum=33U; M(x=y;y=z;z=x)                       \
    }

    typenum=0U;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    /* _Bool type can be treated as an integer, if careful */
    I(_Bool,                0,    1,    1,    nocmp)    typenum++;
#endif /* C99 */
    I(char,                '0',  '1',  '2',   charcmp)  typenum++;
    I(unsigned char,       '0',  '1',  '2',   nocmp)    typenum++;
    I(short,                0,    1,    2,    shortcmp) typenum++;
    I(unsigned short,       0U,   1U,   2U,   nocmp)    typenum++;
    I(int,                  0,    1,    2,    intcmp)   typenum++;
    I(unsigned int,         0U,   1U,   2U,   nocmp)    typenum++;
    I(long,                 0L,   1L,   2L,   longcmp)  typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(unsigned long,        0UL,  1UL,  2UL,  ulcmp)    typenum++;
    I(long long,            0LL,  1LL,  2LL,  nocmp)    typenum++;
    I(unsigned long long,   0ULL, 1ULL, 2ULL, nocmp)    typenum++;
#endif /* C99 */

/* macro to run tests applicable to floating-point types */
#define F(type,v1,v2,v3,fcmp) { volatile type x=v1;   \
        volatile type y=v2; volatile type z=v3;       \
        size_t sz=sizeof(type);                       \
        if (0.0 == z) z = 1.0; /* z non-zero! */      \
        testnum=0U;  M(x=y)                           \
        testnum=1U;  M(x=(y==z))                      \
        testnum=2U;  M(x=(y!=z))                      \
        testnum=3U;  M(x=(y<z))                       \
        testnum=4U;  M(x=(y>=z))                      \
        testnum=5U;  M(x=(y<=z))                      \
        testnum=6U;  M(x=(y>z))                       \
        testnum=7U;  M(x=(y==z)?y:z)                  \
        testnum=8U;  M(x=(y!=z)?y:z)                  \
        testnum=9U;  M(x=(y<z)?y:z)                   \
        testnum=10U; M(x=(y>=z)?y:z)                  \
        testnum=11U; M(x=(y<=z)?y:z)                  \
        testnum=12U; M(x=(y>z)?y:z)                   \
        testnum=14U; M(x+=1)                          \
        testnum=15U; M(x=x+1)                         \
        testnum=16U; M(x+=y)                          \
        testnum=17U; M(x=y+z)                         \
        testnum=18U; M(x=y-z)                         \
        testnum=19U; M(x=y*z)                         \
        testnum=20U; M(x=y/z)                         \
        testnum=30U; M((x=(type)fcmp(&y,&z)))         \
        testnum=31U; M((swap2(&y,&z,sz)))             \
        testnum=32U; M((swapper(&y,&z,1UL,sz)))       \
        testnum=33U; M(x=y;y=z;z=x)                   \
    }

    /* Floating-point types: */
    F(float,                0.0, 1.0, 2.0, nocmp)     typenum++;
    F(double,               0.0, 1.0, 2.0, doublecmp) typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    F(long double,          0.0, 1.0, 2.0, nocmp)     typenum++;
#endif /* C99 */

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
/* macro to run tests applicable to complex types */
#define C(type,v1,v2,v3) { volatile type x=v1;        \
        volatile type y=v2; volatile type z=v3;       \
        if (0.0 == z) z = 1.0; /* z non-zero! */      \
        testnum=0U;  M(x=y)                           \
        testnum=1U;  M(x=(y==z)?y:z)                  \
        testnum=2U;  M(x=(y!=z)?y:z)                  \
        testnum=8U;  M(x+=1)                          \
        testnum=9U;  M(x=x+1)                         \
        testnum=10U; M(x=(y==z))                      \
        testnum=11U; M(x=(y!=z))                      \
        testnum=16U; M(x=y+z)                         \
        testnum=17U; M(x=y-z)                         \
        testnum=18U; M(x=y*z)                         \
        testnum=19U; M(x+=y)                          \
        testnum=20U; M(x=y/z)                         \
        testnum=33U; M(x=y;y=z;z=x)                   \
    }

    /* Complex types: */
    C(float _Complex,       0.0, 1.0, 2.0) typenum++;
    C(double _Complex,      0.0, 1.0, 2.0) typenum++;
    C(long double _Complex, 0.0, 1.0, 2.0) typenum++;
#endif /* C99 */

    /* Last, but not least: pointer */
    {
        volatile char array[3];
        volatile char volatile *x=array, *y=array+1, *z=array+2;

        /* Last test, clean up. */
        pcc = " \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b";
        k = 12UL;
        pcc2 = pcc;
        l = k;
        testnum=0U;  M(x=y)
        testnum=7U;  M(x=(y==z)?y:z)
        testnum=8U;  M(x=(y!=z)?y:z)
        testnum=9U;  M(x=(y<z)?y:z)
        testnum=10U; M(x=(y>=z)?y:z)
        testnum=11U; M(x=(y<=z)?y:z)
        testnum=12U; M(x=(y>z)?y:z)
        testnum=13U; M(x++)
        testnum=14U; M(x+=1)
        testnum=15U; M(x=x+1)
        testnum=33U; M(x=y;y=z;z=x)
    }

    /* Finished with /dev/tty */
    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); }

    /* print headings for results table */
    (V)fprintf(stdout, "%s%s: %s (%s %s %s) COMPILER_USED=\"%s\"\n", prefix, prog, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED);
    (V)fprintf(stdout, "%sOperations: mean time per operation\n", prefix);
#define OPWIDTH  14
#define NUMWIDTH 11
    for (i=0UL; 2UL>i; i++) {
        memset(buf, ' ', sizeof(buf));
        if (1UL == i) {
            pcc = "operation";
            k = strlen(pcc);
            j = (OPWIDTH - k) / 2UL;
            while ('\0' != *pcc) { buf[j++] = *pcc++; }
        }
        for (typenum=0U; typenum<OPTYPES; typenum++) {
            pcc = typename[typenum][i];
            k = strlen(pcc);
            j = OPWIDTH + NUMWIDTH * typenum + (NUMWIDTH - k) / 2UL;
            while ('\0' != *pcc) { buf[j++] = *pcc++; }
        }
        j = OPWIDTH + NUMWIDTH * OPTYPES;
        while (' ' == buf[--j]) { buf[j] = '\0'; }
        (V)fprintf(stdout, "%s%s\n", prefix, buf);
    }
    /* print operation name and timing data */

/* macro to print one row of data, corresponding to one test operation */
#define P(op) pcc = buildstr(op);                                 \
    memset(buf, ' ', sizeof(buf));                                \
    k = strlen(pcc);                                              \
    j = (OPWIDTH - k) / 2UL;                                      \
    while ('\0' != *pcc) { buf[j++] = *pcc++; }                   \
    for (typenum=0U; typenum<OPTYPES; typenum++) {                \
        if ('y' != testmatrix[typenum][testnum]) {                \
            pcc = "N/A";                                          \
        } else {                                                  \
            (V)sng(numbuf, sizeof(numbuf), NULL, NULL,         \
                timing[typenum][testnum], -4, 3, logger, log_arg);\
            pcc = numbuf;                                         \
        }                                                         \
        k = strlen(pcc);                                          \
        j = OPWIDTH + NUMWIDTH * typenum + (NUMWIDTH - k) / 2UL;  \
        while ('\0' != *pcc) { buf[j++] = *pcc++; }               \
    }                                                             \
    j = OPWIDTH + NUMWIDTH * OPTYPES;                             \
    while (' ' == buf[--j]) { buf[j] = '\0'; }                    \
    (V)fprintf(stdout, "%s%s\n", prefix, buf);

    testnum=0U;  P(x=y)
    testnum=1U;  P(x=(y==z))
    testnum=2U;  P(x=(y!=z))
    testnum=3U;  P(x=(y<z))
    testnum=4U;  P(x=(y>=z))
    testnum=5U;  P(x=(y<=z))
    testnum=6U;  P(x=(y>z))
    testnum=7U;  P(x=(y==z)?y:z)
    testnum=8U;  P(x=(y!=z)?y:z)
    testnum=9U;  P(x=(y<z)?y:z)
    testnum=10U; P(x=(y>=z)?y:z)
    testnum=11U; P(x=(y<=z)?y:z)
    testnum=12U; P(x=(y>z)?y:z)
    testnum=13U; P(x++)
    testnum=14U; P(x+=1)
    testnum=15U; P(x=x+1)
    testnum=16U; P(x+=y)
    testnum=17U; P(x=y+z)
    testnum=18U; P(x=y-z)
    testnum=29U; P(x=y*z)
    testnum=10U; P(x=y/z)
    testnum=21U; P(x=y%z)
    testnum=22U; P(x=!y)
    testnum=23U; P(x=~y)
    testnum=24U; P(x=y&z)
    testnum=25U; P(x=y|z)
    testnum=26U; P(x=y^z)
    testnum=27U; P(x=y<<z)
    testnum=28U; P(x=y>>z)
    testnum=39U; P(x=&y-&z)
    testnum=20U; P(compar)
    testnum=31U; P(swap2)
    testnum=32U; P(swapper)
    testnum=33U; P(z=x;x=y;y=z)
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned long mult_k_ki(int p)
{
    unsigned long n=1000UL;

    switch (p) {
        case 'i' : /*FALLTHROUGH*/
        case 'I' :
            n = 1024UL;
        break;
    }
    return n;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned long parse_num(char *p, char **pendptr, int base)
{
    unsigned long n;

if (3<debug) (V)fprintf(stderr, "%s line %d: %s(\"%s\", %p, %d)\n", __func__,__LINE__,__func__,p,pendptr,base);
    n = strtoul(p, pendptr, base);
if (3<debug) (V)fprintf(stderr, "%s line %d: n=%lu, *pendptr=\"%s\"\n", __func__,__LINE__,n,*pendptr);
    switch (**pendptr) {
        case 't' : /*FALLTHROUGH*/
        case 'T' :
            n *= mult_k_ki((*pendptr)[1]);
        /*FALLTHROUGH*/
        case 'g' : /*FALLTHROUGH*/
        case 'G' :
            n *= mult_k_ki((*pendptr)[1]);
        /*FALLTHROUGH*/
        case 'm' : /*FALLTHROUGH*/
        case 'M' :
            n *= mult_k_ki((*pendptr)[1]);
        /*FALLTHROUGH*/
        case 'k' : /*FALLTHROUGH*/
        case 'K' :
            n *= mult_k_ki((*pendptr)[1]);
            switch ((*pendptr)[1]) {
                case 'i' : /*FALLTHROUGH*/
                case 'I' :
                    (*pendptr)++;
                break;
            }
            (*pendptr)++;
        break;
        default :
        break;
    }
if (3<debug) (V)fprintf(stderr, "%s line %d: n=%lu, *pendptr=\"%s\"\n", __func__,__LINE__,n,*pendptr);
    return n;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned long parse_expr(char *p, char **pendptr, int base)
{
    unsigned long n, n2;

if (3<debug) (V)fprintf(stderr, "%s line %d: %s(\"%s\", %p, %d)\n", __func__,__LINE__,__func__,p,pendptr,base);
    n = parse_num(p, pendptr, base);
if (3<debug) (V)fprintf(stderr, "%s line %d: n=%lu, *pendptr=\"%s\"\n", __func__,__LINE__,n,*pendptr);
    while ('\0' != **pendptr) {
        switch (**pendptr) {
            case '+' : 
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
if (3<debug) (V)fprintf(stderr, "%s line %d: n2=%lu\n", __func__,__LINE__,n2);
                n += n2;
            break;
            case '-' : 
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
if (3<debug) (V)fprintf(stderr, "%s line %d: n2=%lu\n", __func__,__LINE__,n2);
                if (n2<n) n -= n2;
                if (1UL > n) n = 1UL;
            break;
            case '*' : 
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
if (3<debug) (V)fprintf(stderr, "%s line %d: n2=%lu\n", __func__,__LINE__,n2);
                if (0<n2) n *= n2;
            break;
            case '/' : 
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
if (3<debug) (V)fprintf(stderr, "%s line %d: n2=%lu\n", __func__,__LINE__,n2);
                if (0<n2) n /= n2;
                if (1UL > n) n = 1UL;
            break;
            case '%' : 
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
if (3<debug) (V)fprintf(stderr, "%s line %d: n2=%lu\n", __func__,__LINE__,n2);
                if (0<n2) n %= n2;
                if (1UL > n) n = 1UL;
            break;
            default :
if (3<debug) (V)fprintf(stderr, "%s line %d: n=%lu, *pendptr=\"%s\"\n", __func__,__LINE__,n,*pendptr);
            return n;
        }
    }
if (3<debug) (V)fprintf(stderr, "%s line %d: n=%lu, *pendptr=\"%s\"\n", __func__,__LINE__,n,*pendptr);
    return n;
}

extern const char *quickselect_build_options;

int main(int argc, const char * const *argv)
{
    char buf[4096], buf2[256], buf3[256];  /* RATS: ignore (big enough) */
    char prog[PATH_MAX];
    char host[MAXHOSTNAMELEN];  /* RATS: ignore (big enough) */
    char iplist[1024];          /* RATS: ignore (size is checked) */
    char logbuf[65536];         /* RATS: ignore (it's big enough) */
    char loghost[MAXHOSTNAMELEN] = "localhost";  /* RATS: ignore (big enough) */
    char procid[32];            /* RATS: ignore (used with sizeof()) */
    const char *pcc, *pcc2;
    char *comment="", *endptr=NULL;
    int bufsz, c, col= RESULTS_COLUMN , i, optind;
    int maxlen = 1024;
    int sockfd = -1;
    int cflags = REG_EXTENDED | REG_ICASE ;
    int eflags = 0;
    unsigned int seqid = 1U;
    unsigned int tzknown = 1U;
    unsigned int csequences=0U, errs=0U, p, tsequences=0U, tests;
    unsigned char flags[256], oldi, oldY, oldZ, var_count='n';
    volatile unsigned long count, incr=1UL, n, div=1UL, mult=1UL, startn=0UL, ul, z;
    pid_t pid;
    size_t sz, q, x, y, stsz;
    size_t marray[10], dn;
    void (*f)(int, void *, const char *, ...) = logger;
    void *log_arg;
    struct logger_struct ls;
    double d, timeout = TEST_TIMEOUT;
    double *sarray=NULL, *uarray=NULL, *warray=NULL;
    int *array=NULL;
    struct big_struct *big_array=NULL, **parray=NULL;
    double *darray=NULL;
    long *larray=NULL, *refarray=NULL;
    uint64_t s[16];
    struct sampling_table_struct *pst;
    regex_t re;
    regmatch_t match[3]; /* line_buf, name, value */
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "main";
#endif

    /* I/O initialization */
    (V) setvbuf(stdout, NULL, (int)_IONBF, BUFSIZ);
    (V) setvbuf(stderr, NULL, (int)_IONBF, BUFSIZ);
    initialize_median_test();
    /* variable initialization */
    host[0] = '\0';
    iplist[0] = '\0';
    procid[0] = '\0';
    (V)path_basename(argv[0], prog, sizeof(prog));
    pid = getpid();
    /* initial logging configuration */
    memset(&ls, 0, sizeof(struct logger_struct));
    ls.options = LOG_PID /* | LOG_PERROR | LOG_CONS */ ;
    ls.logfac = LOG_DAEMON ;
    ls.logmask = LOG_UPTO(LOG_INFO) ;
    ls.location = LOGGER_LOCATION_STREAM | LOGGER_LOCATION_SYSLOG_HOST ;
#if 0
    ls.format = LOGGER_FORMAT_RFC5424_UTC ;
#else
    ls.format = LOGGER_FORMAT_PLAIN ;
#endif
    ls.ext_msg_start = LOGGER_MESSAGE_ALL ;
    ls.psysUpTime = NULL;
    ls.psyncAccuracy = NULL;
    ls.ptzKnown = &tzknown;
    ls.pisSynced = NULL;
    ls.stream = stderr;
    ls.sysloghost = loghost;
    ls.fqdn = host;             /* filled in below (uses logger for error reporting) */
    ls.ident = prog;
    ls.msgid = "median_test";
    ls.procid = procid;         /* filled in below (uses logger for error reporting) */
    ls.enterpriseId = NULL;
    ls.software = prog;
    ls.swVersion = "2.10";       /* maintained by version control system */
    ls.language = NULL;
    ls.pip = iplist;            /* filled in below (uses logger for error reporting) */
    ls.func = NULL;
    ls.pmax_msg_len = &maxlen;
    bufsz = sizeof(logbuf);
    ls.pbufsz = &bufsz;
    ls.buf = logbuf;
    ls.psockfd = &sockfd;
    ls.psequenceId = &seqid;
    log_arg = &ls;
    (V)get_host_name(host, sizeof(host), logger, log_arg);
    /* host address(es) */
    (V)get_ips(iplist, sizeof(iplist), host, AF_UNSPEC, AI_ADDRCONFIG, logger, log_arg);
    (V)snul(procid, sizeof(procid), NULL, NULL, (unsigned long)pid, 10, '0', 1, logger, log_arg); /* logging of snn OK from logger caller after initialization */
    /* process command-line arguments */
    /* initialize flags */
    for (q=0UL; q<sizeof(flags); q++) flags[q] = (unsigned char)0U;
    /* emulate getopt (too many implementation differences to rely on it) */
    for (optind = 1; (optind < argc) && (argv[optind][0] == '-'); optind++) {
        if (0 == strcmp(argv[optind], "--")) {
            optind++;
            break;
        }
        for (pcc = argv[optind] + 1; (int)'\0' != (c = (int)(*pcc)); pcc++) {
            if ((':' == (char)c) || (NULL == strchr(OPTSTRING, (char)c)))
                c = (int)'?';
            switch (c) {
#if SILENCE_WHINEY_COMPILERS
                /* this program has no options (that's what a compiler diagnostic for the loop is about) */
                case ':': /* clang! STFU!! */ /* cannot happen here, but clang is unable to figure that out */
                break;
#endif
                case 'b' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    instrumented_bmqsort = atoi(pcc);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'C' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            for (p=0U; p<TEST_SEQUENCE_COUNT; p++) {
                                pcc2 = sequence_name(p);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__, source_file, __LINE__, pcc, pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__, match[0].rm_so,
                                                match[0].rm_eo - 1, n, n, pcc2 + match[0].rm_so);
                                            tests=0x01U<<p;
                                            /* assume adequate size (15UL) */
                                            if (0!=valid_test(TEST_TYPE_TIMING, p, 15UL))
                                                csequences |= tests;
                                        }
                                    }
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        csequences |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'd' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no debug level) */
                            pcc--;
                            debug++;
                        } else {
                            pcc = argv[++optind]; /* debug level */
                            debug = strtoul(pcc, &endptr, 10);
                            pcc=endptr;
                            for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                            pcc--;
                        }
                    } else {
                        debug = strtoul(pcc, &endptr, 10);
                        pcc=endptr;
                        for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                        pcc--;
                    }
                    if (4<debug) ls.logmask = LOG_UPTO(LOG_DEBUG) ;
                break;
                case 'D' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    dp_cutoff = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'G' :
                    flags[c] = 1U;
                    use_shell++;
                break;
                case 'J' :
                    flags[c] = 1U;
                    endptr = pcc;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no gaps) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* gap value(s) */
                    }
                    for (x=1UL; x<ngaps; x++) {
                        y = strtoul(pcc, &endptr, 10);
                        if (2UL>y) x=0UL; /* in case user gives gap of 1 */
                        gaps[x]=y;
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    pcc=endptr;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'i' :
                    flags[c] = instrumented = 1U;
                break;
                case 'k' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    col = atoi(pcc);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'K' : /*FALLTHROUGH*/
                    flags[c] = 1U;
                    comment="#";
                break;
                case 'm' :
                    flags[c] = 'm'; /* middle for default median(s) */
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no options) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* option value(s) */
                    }
                    if (isalpha(*pcc)) { /* placement of order statistics */
                        flags[c] = tolower((unsigned char)(*pcc));
                    }
                    while (('\0'!=*pcc)&&(!(isdigit(*pcc)))) ++pcc;
                    selection_nk = parse_expr(pcc, &endptr, 10); /* number of order statistics */
                    if (1UL>selection_nk) selection_nk = 1UL;
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        no_aux_repivot=1;
                        repivot_factor = strtoul(pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        lopsided_partition_limit = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'M' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
#if 1
                        pcc = argv[++optind]; /* cutoff value(s) */
#else
                        (V)fprintf(stderr, "ERROR: -%c optional arguments no longer supported\n", c);
#endif
                    }
                    quickselect_small_array_cutoff = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'O' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = strtod(pcc, NULL);
                    if (0.0<d) {
                        double e, g;
                        for (x=2UL; x<22UL; x++) {
                            e=(double)(sorting_sampling_table[x].samples-1UL);
                            g=d*e*e;
                            if (g<(double)(SIZE_T_MAX))
                                sorting_sampling_table[x].min_nmemb=
                                    (unsigned long)snlround(g,f,log_arg);
                            else
                                sorting_sampling_table[x].min_nmemb=(SIZE_T_MAX);
                        }
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'p' :
                    flags[c] = 1U;
                    pivot_swap++;
                break;
                case 'q' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    quickselect_small_array_cutoff = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        no_aux_repivot=1;
                        repivot_factor = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        lopsided_partition_limit = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'Q' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    timeout = strtod(pcc, NULL);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'r' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    introsort_final_insertion_sort = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        introsort_small_array_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        introsort_recursion_factor = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'T' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            for (p=0U; p<TEST_SEQUENCE_COUNT; p++) {
                                pcc2 = sequence_name(p);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__, source_file, __LINE__, pcc, pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__, match[0].rm_so,
                                                match[0].rm_eo - 1, n, n, pcc2 + match[0].rm_so);
                                            tests=0x01U<<p;
                                            /* assume adequate size (15UL) */
                                            if (0!=valid_test(TEST_TYPE_TIMING, p, 15UL))
                                                tsequences |= tests;
                                        }
                                    }
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        tsequences |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'u' :
                    flags[c] = 1U;
                    pivot_copy++;
                break;
                case 'U' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = strtod(pcc, NULL);
                    if (0.0<d) {
                        double e, g;
                        for (x=2UL; x<22UL; x++) {
                            e=(double)(sorting_sampling_table[x].samples);
                            g=d*e*e;
                            if (g<(double)(SIZE_T_MAX))
                                sorting_sampling_table[x].min_nmemb=
                                    (unsigned long)snlround(g,f,log_arg);
                            else
                                sorting_sampling_table[x].min_nmemb=(SIZE_T_MAX);
                        }
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'V' :
                    flags[c] = 1U;
                    use_networks++;
                break;
                case 'W' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    no_aux_repivot=1;
                    repivot_factor = parse_expr(pcc, &endptr, 10);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        repivot_cutoff = parse_expr(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        lopsided_partition_limit = (int)parse_expr(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot = (int)parse_expr(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'y' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoff) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value */
                    }
                    y_cutoff = parse_expr(pcc, &endptr, 10);
                    pcc=endptr;
#if 0
                    if ('\0' != *pcc) {
                        y_cutoff2 = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
#endif
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'Y' : /*FALLTHROUGH*/
                case 'Z' :
                    flags[c] = 't';
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no specifier) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* specifier value(s) */
                    }
                    for (; '\0'!=*pcc; pcc++) {
                        switch (*pcc) {
                            case 'c' :
                                if ('s'==flags[c]) flags[c]='o'; else flags[c]=*pcc;
                            break;
                            case 'o':
                                flags[c]=*pcc;
                            break;
                            case 's' :
                                if ('c'==flags[c]) flags[c]='o'; else flags[c]=*pcc;
                            break;
                            case 't':
                                flags[c]='x';
                            break;
                            case '-':
                                switch (flags[c]) {
                                    case 'c' : /*FALLTHROUGH*/
                                    case 'd' : /*FALLTHROUGH*/
                                    case 'o' : /*FALLTHROUGH*/
                                    case 'p' : /*FALLTHROUGH*/
                                    case 's' : /*FALLTHROUGH*/
                                    case 't' : /*FALLTHROUGH*/
                                    case 'x' : /*FALLTHROUGH*/
                                    case 'y' :
                                        flags[c]--;
                                    break;
                                }
                            break;
                            case '+':
                                switch (flags[c]) {
                                    case 'b' : /*FALLTHROUGH*/
                                    case 'c' : /*FALLTHROUGH*/
                                    case 'n' : /*FALLTHROUGH*/
                                    case 'o' : /*FALLTHROUGH*/
                                    case 'r' : /*FALLTHROUGH*/
                                    case 's' : /*FALLTHROUGH*/
                                    case 'w' : /*FALLTHROUGH*/
                                    case 'x' :
                                        flags[c]++;
                                    break;
                                }
                            break;
                            default :
                            break;
                        }
                    }
                    pcc--;
                break;
                case '0' :
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no table name) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* table name */
                    }
                    flags[c]=*pcc; /* first character of table name */
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                    /* set up requested (or default) sampling table */
                    switch (flags[c]) {
                        case 'a' : /* aggressive */
                            sorting_repivot_table_name = "aggressive";
                            sorting_repivot_table = sorting_repivot_table_aggressive;
                        break;
                        case 'd' : /* disabled */
                            sorting_repivot_table_name = "disabled";
                            sorting_repivot_table = sorting_repivot_table_disabled;
                        break;
                        case 'e' : /* experimental */
                            sorting_repivot_table_name = "experimental";
                            sorting_repivot_table = sorting_repivot_table_experimental;
                        break;
                        case 'l' : /* loose */
                            sorting_repivot_table_name = "loose";
                            sorting_repivot_table = sorting_repivot_table_loose;
                        break;
                        case 'r' : /* relaxed */
                            sorting_repivot_table_name = "relaxed";
                            sorting_repivot_table = sorting_repivot_table_relaxed;
                        break;
                        case 't' : /* transparent */
                            sorting_repivot_table_name = "transparent";
                            sorting_repivot_table = sorting_repivot_table_transparent;
                        break;
                        default : /* no match */
                            sorting_repivot_table_name = "experimental";
                            sorting_repivot_table = sorting_repivot_table_experimental;
                        break;
                    }
                    if ((0U!=flags['1'])||(0U!=flags['2'])||(0U!=flags['3']))
                        (V)fprintf(stderr, "You had better set -0 BEFORE -1 or -2 or -3...\n");
                break;
                case '1' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no factors) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    for (x=1UL; x<=repivot_table_size; x++) {
                        y = parse_expr(pcc, &endptr, 10);
                        set_factor1(x,NULL,y);
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case '2' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no factors) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    for (x=1UL; x<=repivot_table_size; x++) {
                        y = parse_expr(pcc, &endptr, 10);
                        set_factor2(x,NULL,y);
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case '3' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    oldY = 0U, oldZ = 7U;
                    if (isalpha(*pcc)) { /* sampling table specifier */
                        switch (tolower((unsigned char)(*pcc))) {
                            case 'm' : /* middle */
                                oldZ = 2U;
                                pst=sampling_table(0UL,0UL,NULL,0UL,0UL,
#if SAVE_PARTIAL
                                    NULL,NULL,
#endif
                                    &oldY,&oldZ,&stsz);
                            break;
                            case 's' : /* separated */
                                oldZ = 5U;
                                pst=sampling_table(0UL,0UL,NULL,0UL,0UL,
#if SAVE_PARTIAL
                                    NULL,NULL,
#endif
                                    &oldY,&oldZ,&stsz);
                            break;
                            default : /* distributed=sorting */
                                pst=sampling_table(0UL,0UL,NULL,0UL,0UL,
#if SAVE_PARTIAL
                                    NULL,NULL,
#endif
                                    &oldY,&oldZ,&stsz);
                            break;
                        }
                        while (!isdigit(*pcc)) pcc++;
                    } else
                        pst=sampling_table(0UL,0UL,NULL,0UL,0UL,
#if SAVE_PARTIAL
                            NULL,NULL,
#endif
                            &oldY,&oldZ,&stsz);
                    for (x=1UL; x<stsz; x++) {
                        y = parse_expr(pcc, &endptr, 10);
                        if ((1UL==x)&&(3UL>y)) x=0UL; /* in case user gives cutoff for 1 sample */
                        pst[x].min_nmemb=y;
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case '4' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoff) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    quickselect_small_array_cutoff = parse_expr(pcc, &endptr, 10);
                    pcc=endptr;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
#if SAVE_PARTIAL
                case '5' :
                    flags[c] = save_partial = 1U;
                break;
#endif
                case '6' :
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no table name) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* table name */
                    }
                    flags[c]=*pcc; /* first character of table name */
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                    /* set up requested (or default) sampling table */
                    switch (flags[c]) {
                        case 'a' : /* aggressive */
                            selection_repivot_table_name = "aggressive";
                            selection_repivot_table = selection_repivot_table_aggressive;
                        break;
                        case 'd' : /* disabled */
                            selection_repivot_table_name = "disabled";
                            selection_repivot_table = selection_repivot_table_disabled;
                        break;
                        case 'e' : /* experimental */
                            selection_repivot_table_name = "experimental";
                            selection_repivot_table = selection_repivot_table_experimental;
                        break;
                        case 'l' : /* loose */
                            selection_repivot_table_name = "loose";
                            selection_repivot_table = selection_repivot_table_loose;
                        break;
                        case 'r' : /* relaxed */
                            selection_repivot_table_name = "relaxed";
                            selection_repivot_table = selection_repivot_table_relaxed;
                        break;
                        case 't' : /* transparent */
                            selection_repivot_table_name = "transparent";
                            selection_repivot_table = selection_repivot_table_transparent;
                        break;
                        default : /* no match */
                            selection_repivot_table_name = "experimental";
                            selection_repivot_table = selection_repivot_table_experimental;
                        break;
                    }
                    if ((0U!=flags['7'])||(0U!=flags['8']))
                        (V)fprintf(stderr, "You had better set -6 BEFORE -7 or -8...\n");
                break;
                case '7' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no factors) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    for (x=1UL; x<=repivot_table_size; x++) {
                        y = parse_expr(pcc, &endptr, 10);
                        set_factor1(x,&y,y);
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case '8' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no factors) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    for (x=1UL; x<=repivot_table_size; x++) {
                        y = parse_expr(pcc, &endptr, 10);
                        set_factor2(x,&y,y);
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case '#' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no threswhold) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    sort_threshold = strtoul(pcc, &endptr, 10); /* number of order statistics */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case '?' :
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: unrecognized option %s",
                        prog, __func__, source_file, __LINE__, argv[optind]);
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: OPTSTRING is %s",
                        prog, __func__, source_file, __LINE__, OPTSTRING);
usage: 
                    logger(LOG_ERR, log_arg,
                        "%s: usage: %s %s",
                        prog, prog, USAGE_STRING);
                    fprintf(stderr, "test sequences:\n");
                    for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                        fprintf(stderr, "%s %s\n", buf, sequence_name(tests));
                    }
                return 1;
                default : /* recognized option with no special processing */
                    flags[c] = 1U;
                break;
            }
        }
    }
    if (argc<2) goto usage;
    /* repivot parameter sanity */
    if (repivot_cutoff<repivot_factor+2UL) repivot_cutoff=repivot_factor+2UL;
    /* print configured parameters in header */
    if (0U==flags['h']) {
        /* command line */
        (V)fprintf(stdout, "%s%s", comment, prog);
        for (i = 1; i < argc; i++) { (V)fprintf(stdout, " %s", argv[i]); }
        (V)fprintf(stdout, ":\n");
        /* now output settings made by options */
        for (z=n=0U,ul=sizeof(flags); n<ul; n++) {
            if (0U!=flags[n]) {
                switch (n) {
                    case 'b' :
                        (V)fprintf(stdout, "%sinstrumented_bmqsort = %d\n", comment, instrumented_bmqsort);
                    break;
                    case 'd' :
                        (V)fprintf(stdout, "%sdebug = %d\n", comment, debug);
                    break;
                    case 'D' :
                        (V)fprintf(stdout, "%sdp_cutoff = %lu\n", comment, dp_cutoff);
                    break;
                    case 'G' :
                        (V)fprintf(stdout, "%suse_shell = %d\n", comment, use_shell);
                    break;
                    case 'J' :
                        for (x=0UL; x<ngaps; x++)
                            (V)fprintf(stdout, "%sShell sort gap[%lu] %lu\n", comment, x, gaps[x]);
                    break;
                    case 'k' :
                        (V)fprintf(stdout, "%stiming column = %d\n", comment, col);
                    break;
                    case 'M' :
                        (V)fprintf(stdout,
                            "%squickselect small-array sort cutoff = %lu\n",
                            comment, quickselect_small_array_cutoff);
                    break;
                    case 'p' :
                        (V)fprintf(stdout, "%spivot_swap = %d\n", comment, pivot_swap);
                    break;
                    case '0' : /*FALLTHROUGH*/
                    case '1' : /*FALLTHROUGH*/
                    case '2' : /*FALLTHROUGH*/
                    case '3' : /*FALLTHROUGH*/
                    case '4' : /*FALLTHROUGH*/
                    case '6' : /*FALLTHROUGH*/
                    case '7' : /*FALLTHROUGH*/
                    case '8' : /*FALLTHROUGH*/
                    case 'c' : /*FALLTHROUGH*/
                    case 'm' : /*FALLTHROUGH*/
                    case 'O' : /*FALLTHROUGH*/
                    case 'U' : /*FALLTHROUGH*/
                    case 'q' :
                        if (0U==z) {
                            (V)fprintf(stdout,
                                "%squickselect small-array sort cutoff = %lu, no_aux_repivot = %d, repivot_factor = %lu, repivot_cutoff = %lu, lopsided_partition_limit = %d\n",
                                comment, quickselect_small_array_cutoff,
                                no_aux_repivot, repivot_factor, repivot_cutoff, lopsided_partition_limit);
                            if ((0<debug)||(0U==flags['h'])) {
                                (V)fprintf(stdout,"sorting sampling table:\n%s\n",
                                    "                  min_nmemb,     samples"
                                    );
                                for (x=1UL,y=sizeof(sorting_sampling_table)/sizeof(sorting_sampling_table[0]); x<y; x++) {
                                    i = snul(buf,sizeof(buf),NULL,"UL,",sorting_sampling_table[x].min_nmemb, 10, ' ', 24, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",sorting_sampling_table[x].samples, 10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,"   { %s %s },\n",buf,buf2);
                                }
                                (V)fprintf(stdout,"middle selection sampling table:\n%s\n",
                                    "                  min_nmemb,     samples"
                                    );
                                for (x=1UL,y=sizeof(middle_sampling_table)/sizeof(middle_sampling_table[0]); x<y; x++) {
                                    i = snul(buf,sizeof(buf),NULL,"UL,",middle_sampling_table[x].min_nmemb, 10, ' ', 24, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",middle_sampling_table[x].samples, 10, ' ', 13, f, log_arg);
                                    (V)fprintf(stdout,"   { %s %s },\n",buf,buf2);
                                }
                                (V)fprintf(stdout,"separated selection sampling table:\n%s\n",
                                    "                  min_nmemb,     samples"
                                    );
                                for (x=1UL,y=sizeof(separated_sampling_table)/sizeof(separated_sampling_table[0]); x<y; x++) {
                                    i = snul(buf,sizeof(buf),NULL,"UL,",separated_sampling_table[x].min_nmemb, 10, ' ', 24, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",separated_sampling_table[x].samples, 10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,"   { %s %s },\n",buf,buf2);
                                }
                                (V)fprintf(stdout,"sorting repivot table (%s):\n%s\n",
                                    sorting_repivot_table_name,
                                    " samples, factor1, factor2"
                                    );
                                for (x=1UL; x<=repivot_table_size; x++) {
                                    size_t factor1, factor2;
                                    i = snul(buf,sizeof(buf),"/* "," */ { ",sorting_sampling_table[x].samples, 10, ' ', 3, f, log_arg);
                                    repivot_factors(x,NULL,&factor1,&factor2);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL,",factor1, 10, ' ', 3, f, log_arg);
                                    i = snul(buf3,sizeof(buf3),NULL,"UL",factor2, 10, ' ', 3, f, log_arg);
                                    (V)fprintf(stdout,"%s %s %s },\n",buf,buf2,buf3);
                                }
                                (V)fprintf(stdout,"selection repivot table (%s):\n%s\n",
                                    selection_repivot_table_name,
                                    " samples, factor1, factor2"
                                    );
                                for (x=1UL; x<=repivot_table_size; x++) {
                                    size_t factor1, factor2;
                                    /* #samples is uniform for all sampling tables */
                                    i = snul(buf,sizeof(buf),"/* "," */ { ",sorting_sampling_table[x].samples, 10, ' ', 3, f, log_arg);
                                    repivot_factors(x,&factor1,&factor1,&factor2);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL,",factor1, 10, ' ', 3, f, log_arg);
                                    i = snul(buf3,sizeof(buf3),NULL,"UL",factor2, 10, ' ', 3, f, log_arg);
                                    (V)fprintf(stdout,"%s %s %s },\n",buf,buf2,buf3);
                                }
                                if (0U!=flags['m']) {
                                    (V)fprintf(stdout, "%lu order statistic%s, %c\n",
                                        selection_nk, selection_nk==1UL?"":"s",
                                        flags['m']);
                                }
                            }
                            z++;
                        }
                    break;
                    case 'Q' :
                        (V)fprintf(stdout, "%stest timeout = %0.2f\n", comment, timeout);
                    break;
                    case 'r' :
                        /* introsort small-array sort cutoff */
                        if (0UL==introsort_small_array_cutoff) {
                            if (0U==introsort_final_insertion_sort) introsort_small_array_cutoff=8UL;
                            else introsort_small_array_cutoff=16UL;
                        }
                        (V)fprintf(stdout, "%sintrosort_final_insertion_sort = %lu, introsort small-array sort cutoff = %lu, recursion factor = %lu\n", comment, introsort_final_insertion_sort, introsort_small_array_cutoff, introsort_recursion_factor);
                    break;
                    case 'u' :
                        (V)fprintf(stdout, "%spivot_copy = %d\n", comment, pivot_copy);
                    break;
                    case 'V' :
                        (V)fprintf(stdout, "%suse_networks = %d\n", comment, use_networks);
                    break;
                    case 'W' :
                        (V)fprintf(stdout,
                            "%slopsided quickselect no_aux_repivot = %d, repivot_factor = %lu, repivot_cutoff = %lu, lopsided_partition_limit = %d\n",
                            comment, no_aux_repivot, repivot_factor, repivot_cutoff, lopsided_partition_limit);
                    break;
                    case 'y' :
                        (V)fprintf(stdout, "%sYaroslavskiy's dual-pivot small-array sort cutoff = %lu\n", comment, y_cutoff);
                    break;
#if SAVE_PARTIAL
                    case '5' :
                        (V)fprintf(stdout, "%ssave_partial = %u\n", comment, save_partial);
                    break;
#endif
                    case '!' :
                        do_histogram=1U;
                        (V)fprintf(stdout, "%sdo_histogram = %u\n", comment, do_histogram);
                    break;
                    case '#' :
                        (V)fprintf(stdout, "%ssort_threshold = %u\n", comment, sort_threshold);
                    break;
                }
            }
        }
        if (0U!=flags['H']) {
            (V)fprintf(stdout, "%s%s ips %s\n", comment, host, iplist);
        }
        (V)fprintf(stdout, "%s%s (%s %s %s) COMPILER_USED=\"%s\"\n", comment, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED);
#if SILENCE_WHINEY_COMPILERS
        (V)fprintf(stdout, "%sSILENCE_WHINEY_COMPILERS = %d\n", comment, SILENCE_WHINEY_COMPILERS);
#endif
        (V)fprintf(stdout, "%s%s\n", comment, build_options+4);
        (V)fprintf(stdout, "%s%s\n", comment, quickselect_build_options+4);
    }

    if (0U < flags['s']) { /* print type sizes requested */
        print_sizes(comment,prog);
    }

    if (0U < flags['o']) { /* operation timing tests requested */
        op_tests(comment, prog, logger, log_arg); /* 64-bit machines */
    }

#if DEBUG_CODE
if (2<debug) fprintf(stderr,"%s line %d: argc=%d, optind=%d\n",__func__,__LINE__,argc,optind);
#endif
    if (argc > optind+2) {
        startn = parse_expr(argv[optind++], &endptr, 10);
        if (2UL > startn) startn = 2UL;
        pcc = argv[optind++];
        while ('\0' != *pcc) {
            switch (*pcc) {
                case '*' :
                    mult *= parse_num(++pcc, &endptr, 10);
                    if (1UL > mult) mult = 1UL;
                    if (1UL<mult) incr=0UL;
                break;
                case '/' :
                    div *= parse_num(++pcc, &endptr, 10);
                    if (1UL > div) div = 1UL;
                    if (1UL<div) incr=0UL;
                break;
                case '+' :
                    pcc++; /*FALLTHROUGH*/
                default :
                    incr = parse_num(pcc, &endptr, 10);
                    if (0UL<incr) div=mult=1UL;
                    endptr = "";
                break;
            }
            pcc=endptr;
        }
        if (0U==flags['h']) (V)fprintf(stdout, "%sstartn = %lu, div = %lu, incr = %lu, mult = %lu\n", comment, startn, div, incr, mult);
    }

    if (optind >= argc) {
        ul = 0UL;
    } else {
        ul = parse_expr(argv[optind++], &endptr, 10);
        if (0U==flags['h']) (V)fprintf(stdout, "%sul = %lu\n", comment, ul);
    }
    if (1UL > ul) ul = 10000UL;
    if (2UL > ul) ul = 2UL;
#if DEBUG_CODE
if (3<debug) fprintf(stdout, "%s line %d: ul = %lu, startn = %lu, div = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, ul, startn, div, incr, mult);
#endif
    if ((ul<startn)||(0UL==startn)) startn = ul;
#if DEBUG_CODE
if (3<debug) fprintf(stdout, "%s line %d: startn = %lu, div = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, startn, div, incr, mult);
#endif

    if (optind >= argc) {
        count = 0UL;
    } else {
        count = parse_expr(argv[optind++], &endptr, 10);
        switch (*(pcc=endptr)) {
            case 'r' : /*FALLTHROUGH*/
            case 'R' : /*FALLTHROUGH*/
            case 's' : /*FALLTHROUGH*/
            case 'S' :
                var_count = *pcc;
                endptr= ++pcc;
            break;
        }
        if (0U==flags['h']) (V)fprintf(stdout, "%scount = %lu, var_count = %c\n", comment, count, var_count);
    }
    if (1UL > count)
        count = 10UL;

#if DEBUG_CODE
if (3<debug) fprintf(stdout, "%s line %d: startn = %lu, div = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, startn, div, incr, mult);
if (3<debug)     if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s%s: args processed", comment, prog);
#endif

    q=1UL;
    if (0U==flags['h']) (V)fprintf(stdout, "%s%s: %s line %d: startn = %lu, div = %lu, incr = %lu, mult = %lu, ul = %lu, count = %lu, var_count = %c, ul*count = %lu, q = %lu\n", comment, __func__, source_file, __LINE__, startn, div, incr, mult, ul, count, var_count, ul*count, q);

    /* data types */
    /* Test all data types if none specified. */ 
    if (
        (0U==flags['A'])
      &&(0U==flags['F'])
      &&(0U==flags['I'])
      &&(0U==flags['L'])
      &&(0U==flags['P'])
      &&(0U==flags['S'])
    )
        flags['A']=
        flags['F']=
        flags['I']=
        flags['L']=
        flags['P']=
        flags['S']=
        1U;

    /* test types */
    /* Test correctness and timing if neither specified. */ 
    if ((0U==flags['C']) && (0U==flags['T'])) flags['C']=flags['T']=1U;
    else if ((0U==flags['C'])&&(0U!=flags['T'])) csequences=0U;
    else if ((0U!=flags['C'])&&(0U==flags['T'])) tsequences=0U;

    /* sequences vs. flags vs. array size consistency */
    p = ((0x01U << TEST_SEQUENCE_COMBINATIONS) | (0x01U << TEST_SEQUENCE_PERMUTATIONS));
    if (0U != flags['t']) { csequences |= p; tsequences |= p; }
    if ((p==(csequences&p))&&(p==(tsequences&p))) flags['t']=1U;
    p = (0x01U << TEST_SEQUENCE_ADVERSARY);
    if (0U != flags['a']) tsequences |= p;
    if (p==(tsequences&p)) flags['a']=1U;

    /* initialize default sequences if none specified */
    for (p=1U; p<TEST_SEQUENCE_COUNT; p++) {
        tests=0x01U<<p;
        /* default is all valid test sequences except stdin */
        /* assume adequate size */
        if ((0U==flags['C'])&&(0!=valid_test(TEST_TYPE_CORRECTNESS, p, startn)))
            csequences |= tests;
        if ((0U==flags['T'])&&(0!=valid_test(TEST_TYPE_TIMING, p, startn)))
            tsequences |= tests;
    }

    if (0U==flags['h']) {
        if ((0U!=csequences)||(0U!=tsequences)) {
            (V)fprintf(stdout, "%stest sequences:\n", comment);
            if ((0U!=flags['C'])&&(0U!=csequences)) {
                (V)fprintf(stdout, "%s correctness 0x%lx: ", comment, csequences);
                for (errs=0U,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                    if (0U != (csequences & (0x01U << tests))) {
                        (V)snul(buf, sizeof(buf), 0U==errs?"0x":", 0x", NULL,
                            0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4, 
                            logger, log_arg);
                        (V)fprintf(stdout,"%s %s",buf,sequence_name(tests));
                        errs++;
                    }
                }
                (V)fprintf(stdout, "\n");
            }
            if ((0U!=flags['T'])&&(0U!=tsequences)) {
                (V)fprintf(stdout, "%s timing 0x%lx: ", comment, tsequences);
                for (errs=0U,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                    if (0U != (tsequences & (0x01U << tests))) {
                        (V)snul(buf, sizeof(buf), 0U==errs?"0x":", 0x", NULL,
                            0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4, 
                            logger, log_arg);
                        (V)fprintf(stdout,"%s %s",buf,sequence_name(tests));
                        errs++;
                    }
                }
                (V)fprintf(stdout, "\n");
            }
        }
    }

    errs = 0U;

    /* initialize (seed) random number generator and save state */
    if (0U != flags['z']) {
        unsigned int z2;

        p = 0U;
        for (z2=0U; z2<16U; z2++)
            s[z2]=z2+1U;
        i = seed_random64n(s, p);
        if (0 != i) {
            logger(LOG_ERR, log_arg, 
                "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                prog, __func__, source_file, __LINE__,
                (unsigned long)s, p, i
            );
            errs++;
        }
    }
    if (0U<errs) {
        return errs;
    }

    /* allocate test arrays */
    /* Size should be large enough for 1 set of arrays at largest count, except
       when testing with permutations or combinations.
    */
    sz = 0UL;
    q = sizeof(long); /* refarray */
    if (0UL!=flags['I']) q+=sizeof(int);
    if ((0U!=flags['A'])||(0U!=flags['S'])||(0U!=flags['P'])) q+=sizeof(struct big_struct);
    if (0UL!=flags['P']) q+=sizeof(struct big_struct *);
    if (0UL!=flags['F']) q+=sizeof(double);
    if (0UL!=flags['L']) q+=sizeof(long);
#if 0
    p = (0x01U << TEST_SEQUENCE_COMBINATIONS);
    if ( ( ((0U!=flags['C'])&&(0U!=(csequences&p)))
         ||((0U!=flags['T'])&&(0U!=(tsequences&p)))
         )
    ) {
        /* big: ul*2^ul */
        if ((ul>=LONG_BIT)||((((SIZE_T_MAX)/q)>>ul)<=ul)) {
            for (n=1UL; n<LONG_BIT; n++)
                if (((SIZE_T_MAX)>>n)/q <= n) break;
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %s %d",
                prog, __func__, source_file, __LINE__,
                "maximum size possible for combinations is",
                n-1UL);
#if 0
            return ++errs;
#else
            csequences &= ~p;
            tsequences &= ~p;
#endif
        } else
            sz=ul*(0x01<<ul);
    }
    p = (0x01U << TEST_SEQUENCE_PERMUTATIONS);
    if ( ( ((0U!=flags['C'])&&(0U!=(csequences&p)))
         ||((0U!=flags['T'])&&(0U!=(tsequences&p)))
         )
    &&   ((3UL<ul)||(0UL==sz))
    ) {
        /* bigger: ul*ul! (for ul>=4, 2^4=16 < 4!=24; s^3=8 > 3!=6) */
        if (7UL<ul) {
            /* run-time test of feasibility */
            for (n=8UL; n<=ul; n++) {
                sz=n*factorial(n);
                if (((SIZE_T_MAX)/sz/q <= (n+1UL)*(n+1UL))&&(n<ul)) {
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: %s %lu",
                        prog, __func__, source_file, __LINE__,
                        "maximum size possible for permutations is",
                        n);
#if 0
                    return ++errs;
#else
                    csequences &= ~p;
                    tsequences &= ~p;
                    break;
#endif
                }
            }
        } else /* 7*7! will fit even 16-bit words */
            sz=ul*factorial(ul);
    }
    p = ((0x01U << TEST_SEQUENCE_COMBINATIONS) | (0x01U << TEST_SEQUENCE_PERMUTATIONS));
    if ((0UL==sz)||((0U==(csequences&p)&&(0U==(tsequences&p))))) {
        /* not testing with permutations or combinations */
        sz=ul;
    }
#else
    sz=ul;
#endif
    if (SIZE_T_MAX/q <= sz) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %s %lu",
            prog, __func__, source_file, __LINE__,
            "malloc will certainly fail; try again with size <",
            ul);
        return ++errs;
    }

    if (0U < flags['n']) /* do-nothing flag */
        return 0;

    if (0UL!=flags['I']) {
        array=malloc(sizeof(int)*sz);
        if (NULL==array) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            return errs;
        }
    }
    p = (0x01U << TEST_SEQUENCE_STDIN);
    if ((0U!=(csequences&p))||(0U!=(tsequences&p))||(0U!=flags['A'])||(0U!=flags['S'])||(0U!=flags['P'])) {
        big_array=malloc(sizeof(struct big_struct)*sz);
        if (NULL==big_array) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            if (NULL!=array) { free(array); array=NULL; }
            return errs;
        }
        if (0UL!=flags['P']) {
            parray=malloc(sizeof(struct big_struct *)*sz);
            if (NULL==parray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                if (NULL!=array) { free(array); array=NULL; }
                if (NULL!=big_array) { free(big_array); big_array=NULL; }
                return errs;
            }
            /* pointers in parray point to structures in big_array */
            for (q=0UL; q<sz; q++)
                parray[q]=&(big_array[q]);
        }
    }
    if (0UL!=flags['F']) {
        darray=malloc(sizeof(double)*sz);
        if (NULL==darray) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            if (NULL!=array) { free(array); array=NULL; }
            if (NULL!=big_array) { free(big_array); big_array=NULL; }
            if (NULL!=parray) { free(parray); parray=NULL; }
            return errs;
        }
    }
    if (0U!=flags['L']) {
        larray=malloc(sizeof(long)*sz);
        if (NULL==larray) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            if (NULL!=array) { free(array); array=NULL; }
            if (NULL!=big_array) { free(big_array); big_array=NULL; }
            if (NULL!=parray) { free(parray); parray=NULL; }
            if (NULL!=darray) { free(darray); darray=NULL; }
            return errs;
        }
    }
    /* refarray is the basis for all types */
    refarray=malloc(sizeof(long)*sz);
    if (NULL==refarray) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        if (NULL!=array) { free(array); array=NULL; }
        if (NULL!=big_array) { free(big_array); big_array=NULL; }
        if (NULL!=parray) { free(parray); parray=NULL; }
        if (NULL!=darray) { free(darray); darray=NULL; }
        if (NULL!=larray) { free(larray); larray=NULL; }
        return errs;
    }

#if DEBUG_CODE
if (3<debug)     if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s%s %s: %s line %d: arrays allocated, sz=%lu", comment, prog, __func__, source_file, __LINE__, sz);
#endif

    /* read data from stdin if requested */
    p = (0x01U << TEST_SEQUENCE_STDIN);
    if ((0U!=(csequences&p))||(0U!=(tsequences&p))) {
        f(LOG_INFO, log_arg, "%s%s %s: %s line %d: reading data sequence from stdin", comment, prog, __func__, source_file, __LINE__);
        for (i=0; NULL != fgets(buf,sizeof(buf),stdin); i++) {
            /* elide trailing newline */
            endptr=strchr(buf,'\n');
            if (NULL != endptr) *endptr='\0';
            else { /* line too long */ --i; continue; }
            /* skip leading whitespace (comment detection; not strtol/strtod) */
            for (endptr=buf; (*endptr!='\0')&&(0!=isspace(*endptr)); endptr++) ;
            /* skip comments, empty lines */
            switch (*endptr) {
                case '/' : /* C-type and C ploose ploose comments */
                /*FALLTHROUGH*/
                case '#' : /* shell-style comments */
                /*FALLTHROUGH*/
                case ';' : /* named.conf-style comments */
                    (V)fprintf(stderr, "%s line %d: i=%d: ignoring comment line %s\n",__func__,__LINE__,i,buf);
                /*FALLTHROUGH*/
                case '\0' : /* empty line */
                    --i; /* don't count this input line */
                continue; /* next line */
            }
            /* don't try to write past end of arrays! */
            if (i+0UL>=sz) break;
            if ((0==i)&&(0U!=flags['d'])) (V)fprintf(stderr, "%s line %d: first non-comment input line %s\n",__func__,__LINE__,buf);
            /* save line as long, integer, double, string */
            big_array[i].d=strtod(endptr,NULL);
            big_array[i].val=big_array[i].l=snlround(big_array[i].d,f,log_arg);
            (V)snprintf(big_array[i].string, (BIG_STRUCT_STRING_SIZE), "%*.*s", (BIG_STRUCT_STRING_SIZE)-1, (BIG_STRUCT_STRING_SIZE)-1, endptr);
#if DEBUG_CODE
if (3<debug) if (0==i) (V)fprintf(stderr, "// %s line %d: val %d, d %G, string \"%s\"\n",__func__,__LINE__,big_array[i].val,big_array[i].d,big_array[i].string);
#endif
        }
        /* allocate structures to save input data for reuse */
        input_data=malloc(sizeof(struct big_struct)*i);
        if (NULL==input_data) errs++; /* oops, malloc failed; bail out */
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            if (NULL!=array) { free(array); array=NULL; }
            if (NULL!=big_array) { free(big_array); big_array=NULL; }
            if (NULL!=parray) { free(parray); parray=NULL; }
            if (NULL!=darray) { free(darray); darray=NULL; }
            if (NULL!=larray) { free(larray); larray=NULL; }
            if (NULL!=refarray) { free(refarray); refarray=NULL; }
            return errs;
        }
        /* save input data for reuse */
        /* reset counts to correspond to number of data read */
        for (ul=0UL; ul<(size_t)i; ul++)
            input_data[ul]=big_array[ul];
        startn=ul;
        /* emit revised counts if header is not suppressed */
        if (0U==flags['h']) (V)fprintf(stdout, "%s: %s line %d: startn = %lu, div = %lu, incr = %lu, mult = %lu, ul = %lu, count = %lu, ul*count = %lu, q = %lu\n", __func__, source_file, __LINE__, startn, div, incr, mult, ul, count, ul*count, q);
    }

    /* restore (re-seed) random number generator(s) before each call to test_function() */
    if (0U == errs) {
        i = save_random64n_state(s, &p);
        if (0 != i) {
            logger(LOG_ERR, log_arg, 
                "%s: %s: %s line %d: save_random64n_state(0x%lx, 0x%lx) returned %d: %m",
                prog, __func__, source_file, __LINE__,
                (unsigned long)s, (unsigned long)(&p), i
            );
            errs++;
        }
#if DEBUG_CODE
          else logger(LOG_INFO, log_arg, "%s: random generator initialized, state saved", prog);
#endif
    }

    if ((0U!=flags['Y'])||(0U!=flags['Z'])) { /* optimize quickselect sampling table breakpoints */
        breakpoint_table[0].min_min_bp=breakpoint_table[0].max_min_bp=1UL;
        breakpoint_table[0].min_max_bp=breakpoint_table[0].max_max_bp=1UL;
        breakpoint_table[0].optimum_bp=sorting_sampling_table[0].min_nmemb;
        breakpoint_table[1].min_min_bp=breakpoint_table[1].max_min_bp=3UL;
        breakpoint_table[1].min_max_bp=breakpoint_table[1].max_max_bp=3UL;
        breakpoint_table[1].optimum_bp=sorting_sampling_table[1].min_nmemb;
        for (x=2UL; x<22UL; x++) {
            n=sorting_sampling_table[x].samples;
            breakpoint_table[x].min_min_bp=breakpoint_table[x].min_max_bp=n*n;
            breakpoint_table[x].max_min_bp=breakpoint_table[x].max_max_bp=n;
            breakpoint_table[x].optimum_bp=sorting_sampling_table[x].min_nmemb;
        }
    }
    oldi = flags['i']; oldY = flags['Y']; oldZ = flags['Z'];
    for (q=ul*count,n=startn; n<=ul; n=z) {
        flags['Y'] = oldY; flags['Z'] = oldZ;
        switch (var_count) {
            case 'r' : /*FALLTHROUGH*/
            case 'R' :
                count = q / n; /* XXX for now... */
            break;
            case 's' : /*FALLTHROUGH*/
            case 'S' :
                count = q / n;
            break;
        }

        flags[':']=flags['i']=1U; /* turn on magic silence flag; enable counters */
        while ((0U==errs)&&((0U!=flags['Y'])||(0U!=flags['Z']))) { /* optimize quickselect sampling table breakpoints */
            unsigned int cs=0U,
                ts=0x01U<<TEST_SEQUENCE_RANDOM_DISTINCT
            ;
            size_t minn, maxn, maxm, minm, m, orig, prev, step, r, r2;
            int func;

            tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION ;
            if (0U!=flags['Z']) {
                tests |= TEST_TYPE_SORT ;
                func = FUNCTION_SQSORT ;
                c = 'Z';
            } else {
                func = FUNCTION_QSELECT ;
                c = 'Y';
            }
            flags['i']=1U;
            for (x=2UL; x<21UL; x++) {
                orig = prev = minm = maxm = m = sorting_sampling_table[x].min_nmemb; /* start with existing values */
                minn=(SIZE_T_MAX), maxn=0UL;
                if (m>n) break;
#if 0
                if (m < (n>>4)) continue;
                if (n>=sorting_sampling_table[x+1UL].min_nmemb) continue;
#endif
                i = seed_random64n(s, p);
                (V)test_function(prog, refarray, array, big_array, darray, larray, parray, func, n, count, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
//(V)fprintf(stderr, "%s line %d: 1st quartile user time is %lu microseconds\n",__func__,__LINE__,snlround(1.0e6*uarray[marray[3]],f,log_arg));
                switch (flags[c]) {
                    case 'b' : /*FALLTHROUGH*/
                    case 'c' : /*FALLTHROUGH*/
                    case 'd' :
                        r2 = neq+nlt+ngt;
                    break;
                    case 'n' : /*FALLTHROUGH*/
                    case 'o' : /*FALLTHROUGH*/
                    case 'p' :
                        r2 = neq+nlt+ngt+nsw;
                    break;
                    case 'r' : /*FALLTHROUGH*/
                    case 's' : /*FALLTHROUGH*/
                    case 't' :
                        r2 = nsw;
                    break;
                    default :
                        r2 = snlround(1.0e6*uarray[marray[3]],f,log_arg);
                    break;
                }
                r = r2;
                (V)fprintf(stderr, "baseline breakpoint %lu for %lu samples: %lu metric\n", m, sorting_sampling_table[x].samples, r);
                step=(m>>1);
                if (m+step>n) step=n-m;
                /* try larger cutoff for lower cost */
                do {
                    if (r2<=r) {
                        if (r2==r) {
                            if (m<minm) {
                                if (n<minn) minn=n;
                                minm=m;
                            } else if(m>maxm) {
                                if (n>maxn) maxn=n;
                                maxm=m;
                            }
                            prev = m;
                        }
                        if (r2<r) {
                            /* prefer higher cutoff with same cost for selection */
                            minm = maxm = m; /* best so far */
                            if (n<minn) minn=n; else if (n>maxn) maxn=n;
                            r = r2;
                            (V)fprintf(stderr, "%lu samples for new candidate nmemb >= %lu (%lu metric)\n", sorting_sampling_table[x].samples, m, r);
                        }
                        m+=step;
                        if (m>n) m=n;
                        if ((m==orig)||(m==prev)||(m==minm)||(m==maxm)) step>>=1, m-=step;
                    } else {
                        step>>=1;
                        if (1UL>step) break;
                        if (m<=step) break;
                        m-=step;
                        if ((m==orig)||(m==prev)||(m==minm)||(m==maxm)) step>>=1, m+=step;
                    }
                    if (m>n) break;
                    if (m<=orig) break;
#if 0
                    if (m<sorting_sampling_table[x].samples) break;
                    if (m>=sorting_sampling_table[x+1UL].min_nmemb) break;
#endif
                    if (1UL>step) break;
                    sorting_sampling_table[x].min_nmemb = m;
                    (V)fprintf(stderr, "trying breakpoint %lu for %lu samples, step=%lu\n", m, sorting_sampling_table[x].samples,step);
                    i = seed_random64n(s, p);
                    (V)test_function(prog, refarray, array, big_array, darray, larray, parray, func, n, count, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
//(V)fprintf(stderr, "%s line %d: 1st quartile user time is %lu microseconds\n",__func__,__LINE__,snlround(1.0e6*uarray[marray[3]],f,log_arg));
                    switch (flags[c]) {
                        case 'b' : /*FALLTHROUGH*/
                        case 'c' : /*FALLTHROUGH*/
                        case 'd' :
                            r2 = neq+nlt+ngt;
                        break;
                        case 'n' : /*FALLTHROUGH*/
                        case 'o' : /*FALLTHROUGH*/
                        case 'p' :
                            r2 = neq+nlt+ngt+nsw;
                        break;
                        case 'r' : /*FALLTHROUGH*/
                        case 's' : /*FALLTHROUGH*/
                        case 't' :
                            r2 = nsw;
                        break;
                        default :
                            r2 = snlround(1.0e6*uarray[marray[3]],f,log_arg);
                        break;
                    }
                } while ((r2<=r)||(step>1UL));
                m = orig;
                step=(m>>2);
                m -= step;
                /* try smaller cutoff for lower cost */
                do {
                    if (r2<=r) {
                        if (r2==r) {
                            if (m<minm) {
                                if (n<minn) minn=n;
                                minm=m;
                            } else if(m>maxm) {
                                if (n>maxn) maxn=n;
                                maxm=m;
                            }
                            prev = m;
                        }
                        if (r2<r) {
                            /* prefer lower cutoff with same cost for sorting
                               or when optimizing small-sample cutoffs for large
                               arrays
                            */
                            minm = maxm = m; /* best so far */
                            if (n<minn) minn=n; else if (n>maxn) maxn=n;
                            r = r2;
                            (V)fprintf(stderr, "%lu samples for new candidate nmemb >= %lu (%lu metric)\n", sorting_sampling_table[x].samples, m, r);
                        }
                        if (m<=step) break;
                        m-=step;
                        if ((m==orig)||(m==prev)||(m==minm)||(m==maxm)) step>>=1, m+=step;
                    } else {
                        step>>=1;
                        if (1UL>step) break;
                        m+=step;
                        if ((m==orig)||(m==prev)||(m==minm)||(m==maxm)) step>>=1, m-=step;
                    }
                    if ((2UL<x)&&(m<=sorting_sampling_table[x-1UL].min_nmemb)) break;
                    if (m>=orig) break;
                    if (m<sorting_sampling_table[x].samples) break;
                    if (1UL>step) break;
                    sorting_sampling_table[x].min_nmemb = m;
                    (V)fprintf(stderr, "trying breakpoint %lu for %lu samples, step=%lu\n", m, sorting_sampling_table[x].samples,step);
                    i = seed_random64n(s, p);
                    (V)test_function(prog, refarray, array, big_array, darray, larray, parray, func, n, count, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
//(V)fprintf(stderr, "%s line %d: 1st quartile user time is %lu microseconds\n",__func__,__LINE__,snlround(1.0e6*uarray[marray[3]],f,log_arg));
                    switch (flags[c]) {
                        case 'b' : /*FALLTHROUGH*/
                        case 'c' : /*FALLTHROUGH*/
                        case 'd' :
                            r2 = neq+nlt+ngt;
                        break;
                        case 'n' : /*FALLTHROUGH*/
                        case 'o' : /*FALLTHROUGH*/
                        case 'p' :
                            r2 = neq+nlt+ngt+nsw;
                        break;
                        case 'r' : /*FALLTHROUGH*/
                        case 's' : /*FALLTHROUGH*/
                        case 't' :
                            r2 = nsw;
                        break;
                        default :
                            r2 = snlround(1.0e6*uarray[marray[3]],f,log_arg);
                        break;
                    }
                } while ((r2<=r)||(step>1UL));
                if ((orig>=minm)&&(orig<=maxm)) sorting_sampling_table[x].min_nmemb=orig;
                else
                    switch (flags[c]) {
                        case 'b' : /*FALLTHROUGH*/
                        case 'n' : /*FALLTHROUGH*/
                        case 'r' : /*FALLTHROUGH*/
                        case 'w' :
                            sorting_sampling_table[x].min_nmemb = minm;
                        break;
                        case 'c' : /*FALLTHROUGH*/
                        case 'o' : /*FALLTHROUGH*/
                        case 's' : /*FALLTHROUGH*/
                        case 'x' :
                            sorting_sampling_table[x].min_nmemb = maxm-((maxm-minm)>>2); /* 3/4 of spread */
                        break;
                        case 'd' : /*FALLTHROUGH*/
                        case 'p' : /*FALLTHROUGH*/
                        case 't' : /*FALLTHROUGH*/
                        case 'y' :
                            sorting_sampling_table[x].min_nmemb = maxm;
                        break;
                    }
                (V)fprintf(stderr, "%lu samples for nmemb >= %lu (range %lu through %lu) (%lu metric)\n", sorting_sampling_table[x].samples, sorting_sampling_table[x].min_nmemb, minm, maxm, r);
                if (minm<breakpoint_table[x].min_min_bp) {
                    breakpoint_table[x].min_min_bp=minm;
                    breakpoint_table[x].min_min_nmemb=minn;
                }
                if (minm>breakpoint_table[x].max_min_bp) {
                    breakpoint_table[x].max_min_bp=minm;
                    breakpoint_table[x].max_min_nmemb=minn;
                }
                if (maxm<breakpoint_table[x].min_max_bp) {
                    breakpoint_table[x].min_max_bp=maxm;
                    breakpoint_table[x].min_max_nmemb=maxn;
                }
                if (maxm>breakpoint_table[x].max_max_bp) {
                    breakpoint_table[x].max_max_bp=maxm;
                    breakpoint_table[x].max_max_nmemb=maxn;
                }
#if 0
                if (breakpoint_table[x].max_min_nmemb<=breakpoint_table[x].min_max_nmemb) {
                    if ((orig>=breakpoint_table[x].max_min_nmemb)
                    && (orig<=breakpoint_table[x].min_max_nmemb))
                        breakpoint_table[x].optimum_bp=orig;
                    else
                        breakpoint_table[x].optimum_bp=breakpoint_table[x].min_max_nmemb;
                } else
#endif
                {
#if 0
27 samples for nmemb >= 734 (range 460 through 741) (12082 metric)
trying breakpoints 813 through 813 for 27 samples, from nmemb=1219 through 1219 by 1
trying breakpoint 813 for 27 samples, from nmemb=1219 through 1219 by 1
#endif
                    /* need to compare overall number of comparisons at breakpoints between min_max and max_min over nmemb from min(min_max,max_min) through max(min_max,max_min)) */
                    minm=breakpoint_table[x].min_max_bp;
                    if (breakpoint_table[x].max_min_bp<minm) minm=breakpoint_table[x].max_min_bp;
                    m=sorting_sampling_table[x-1UL].min_nmemb*3UL;
                    if (minm<m) minm=m;
                    maxm=breakpoint_table[x].max_min_bp;
                    if (breakpoint_table[x].min_max_bp>maxm) maxm=breakpoint_table[x].min_max_bp;
                    if (maxm>startn) maxm=startn;
                    if (maxm<minm) maxm=minm;
                    if ((orig>=minm)&&(orig<=maxm)) breakpoint_table[x].optimum_bp=orig;
                    else breakpoint_table[x].optimum_bp=maxm-((maxm-minm)>>2);
                    maxn=minn=breakpoint_table[x].min_max_nmemb;
                    if (breakpoint_table[x].max_min_nmemb<minn) minn=breakpoint_table[x].max_min_nmemb;
                    if (breakpoint_table[x].max_min_nmemb>maxn) maxn=breakpoint_table[x].max_min_nmemb;
                    if (minn<startn) minn=startn;
                    m=maxm+(maxm>>1);
                    if (minn<m) minn=m;
                    if (maxn>n) maxn=n;
                    if (minn>maxn) maxn=minn;
                    /* get baseline cost measure at present min_nmemb */
                    breakpoint_table[x].optimum_bp=sorting_sampling_table[x].min_nmemb;
                    r=0UL;
                    step=(maxn-minn)/32UL;
                    if (1UL>step) step=1UL;
                    for (y=minn; y<=maxn; y+=step) {
                        i = seed_random64n(s, p);
                        (V)test_function(prog, refarray, array, big_array, darray, larray, parray, func, y, count, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
                        switch (flags[c]) {
                            case 'b' : /*FALLTHROUGH*/
                            case 'c' : /*FALLTHROUGH*/
                            case 'd' :
                                r += neq+nlt+ngt;
                            break;
                            case 'n' : /*FALLTHROUGH*/
                            case 'o' : /*FALLTHROUGH*/
                            case 'p' :
                                r += neq+nlt+ngt+nsw;
                            break;
                            case 'r' : /*FALLTHROUGH*/
                            case 's' : /*FALLTHROUGH*/
                            case 't' :
                                r += nsw;
                            break;
                            default :
                                r += snlround(1.0e6*uarray[marray[3]],f,log_arg);
                            break;
                        }
                    }
                    if (0<debug) (V)fprintf(stderr, "cost metric at breakpoint %lu for %lu samples from nmemb=%lu through %lu by %lu is %lu\n", sorting_sampling_table[x].min_nmemb, sorting_sampling_table[x].samples,minn,maxn,step,r);
                    (V)fprintf(stderr, "trying breakpoints %lu through %lu for %lu samples, from nmemb=%lu through %lu by %lu\n", minm, maxm, sorting_sampling_table[x].samples,minn,maxn,step);
                    for (m=minm; m<=maxm; m++) {
                        sorting_sampling_table[x].min_nmemb = m;
                        r2=0UL;
                        if (0<debug) (V)fprintf(stderr, "trying breakpoint %lu for %lu samples, from nmemb=%lu through %lu by %lu\n", m, sorting_sampling_table[x].samples,minn,maxn,step);
                        for (y=minn; y<=maxn; y+=step) {
                            i = seed_random64n(s, p);
                            (V)test_function(prog, refarray, array, big_array, darray, larray, parray, func, y, count, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
                            switch (flags[c]) {
                                case 'b' : /*FALLTHROUGH*/
                                case 'c' : /*FALLTHROUGH*/
                                case 'd' :
                                    r2 += neq+nlt+ngt;
                                break;
                                case 'n' : /*FALLTHROUGH*/
                                case 'o' : /*FALLTHROUGH*/
                                case 'p' :
                                    r2 += neq+nlt+ngt+nsw;
                                break;
                                case 'r' : /*FALLTHROUGH*/
                                case 's' : /*FALLTHROUGH*/
                                case 't' :
                                    r2 += nsw;
                                break;
                                default :
                                    r2 += snlround(1.0e6*uarray[marray[3]],f,log_arg);
                                break;
                            }
                        }
                        if (r2<=r) {
                            r=r2;
                            breakpoint_table[x].optimum_bp=m;
                            if (0<debug) (V)fprintf(stderr, "%lu samples for new candidate optimum breakpoint >= %lu (%lu metric)\n", sorting_sampling_table[x].samples, m, r);
                        }
                    }
                }
                (V)fprintf(stderr, "%lu samples optimum breakpoint min_nmemb = %lu\n",sorting_sampling_table[x].samples,breakpoint_table[x].optimum_bp);
                sorting_sampling_table[x].min_nmemb = breakpoint_table[x].optimum_bp;
                if (1<debug) sleep(8);
            }
            for (x=1UL; x<22UL; x++) {
                if (((SIZE_T_MAX)==sorting_sampling_table[x].min_nmemb)
                && ((SIZE_T_MAX) / sorting_sampling_table[x].samples > sorting_sampling_table[x].samples))
                    sorting_sampling_table[x].min_nmemb = sorting_sampling_table[x].samples * sorting_sampling_table[x].samples;
                i = snul(buf,sizeof(buf),NULL,"UL,",breakpoint_table[x].optimum_bp, 10, ' ', 21, f, log_arg);
                i = snul(buf2,sizeof(buf2),NULL,"UL",sorting_sampling_table[x].samples, 10, ' ', 11, f, log_arg);
                (V)fprintf(stdout,"   { %s %s },\n",buf,buf2);
            }
            if (0U!=flags['Y']) flags['Y']=0U;
            else if (0U!=flags['Z']) flags['Z']=0U;
        }
#if 0
        if (breakpoint_table[2].max_min_bp!=sorting_sampling_table[2].samples) {
            for (x=2UL; x<22UL; x++) {
                if (breakpoint_table[x].max_min_bp==sorting_sampling_table[x].samples) break;
                (V)fprintf(stdout,
                    "%lu samples, breakpoint ranges: wide %lu@%lu-%lu@%lu, narrow %lu@%lu-%lu@%lu\n",
                    sorting_sampling_table[x].samples,
                    breakpoint_table[x].min_min_bp,breakpoint_table[x].min_min_nmemb,
                    breakpoint_table[x].max_max_bp,breakpoint_table[x].max_max_nmemb,
                    breakpoint_table[x].max_min_bp,breakpoint_table[x].max_min_nmemb,
                    breakpoint_table[x].min_max_bp,breakpoint_table[x].min_max_nmemb);
            }
        }
#endif
        flags['i']=oldi;
        flags[':']=0U; /* turn off magic silence flag */

        if ((0U == errs) && (0U != flags['b'])) { /* test Bentley&McIlroy qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                if (0==instrumented_bmqsort)
                    errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_BMQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
                else
                    errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_IBMQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['M'])) { /* test modified Bentley&McIlroy qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_MBMQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['N'])) { /* test NetBSD qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_NBQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['w'])) { /* test qsort_wrapper */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_QSORT_WRAPPER, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['c'])) { /* test simplified quickselect sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_SQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['W'])) { /* test lopsided quickselect sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_WQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['m'])) { /* test quickselect selection */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION ;
                if (0U<do_histogram) {
                    for (q=0UL; q<6UL; q++) {
                        size_t r;
                        for (r=0UL; r<22UL; r++) {
                            histogram[q][r]=0UL;
                        }
                        histoswap[q]=0UL;
                    }
                }
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_QSELECT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
                if (0U<do_histogram) {
                    for (q=0UL; q<6UL; q++) {
                        size_t r, t, u;
                        for (r=t=u=0UL; r<22UL; r++) {
                            if (0UL<histogram[q][r]) {
                                (V)fprintf(stderr,
                                    "sub-array %lu samples, %lu comparisons: %lu\n",
                                    q+HISTOGRAM_INDEX1_OFFSET,
                                    q+HISTOGRAM_INDEX1_OFFSET-1UL+r, histogram[q][r]);
                                t+=histogram[q][r];
                                u+=histogram[q][r]*(q+HISTOGRAM_INDEX1_OFFSET-1UL+r);
                            }
                        }
                        if (0UL<u) {
                            (V)fprintf(stderr,
                                "sub-array %lu samples: %lu instance%s, average %.3G comparisons, %.3G swaps, %.3G operations\n",
                                q+HISTOGRAM_INDEX1_OFFSET, t, t==1UL?"":"s",
                                (double)u/(double)t, (double)(histoswap[q])/(double)t,
                                (double)(u+histoswap[q])/(double)t);
                        }
                    }
                }
            }
        }

        if ((0U == errs) && (0U != flags['q'])) { /* test quickselect sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                if (0U<do_histogram) {
                    for (q=0UL; q<6UL; q++) {
                        size_t r;
                        for (r=0UL; r<22UL; r++) {
                            histogram[q][r]=0UL;
                        }
                        histoswap[q]=0UL;
                    }
                }
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_QSELECT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
                if (0U<do_histogram) {
                    for (q=0UL; q<6UL; q++) {
                        size_t r, t, u;
                        for (r=t=u=0UL; r<22UL; r++) {
                            if (0UL<histogram[q][r]) {
                                (V)fprintf(stderr,
                                    "sub-array %lu samples, %lu comparisons: %lu\n",
                                    q+HISTOGRAM_INDEX1_OFFSET,
                                    q+HISTOGRAM_INDEX1_OFFSET-1UL+r, histogram[q][r]);
                                t+=histogram[q][r];
                                u+=histogram[q][r]*(q+HISTOGRAM_INDEX1_OFFSET-1UL+r);
                            }
                        }
                        if (0UL<u) {
                            (V)fprintf(stderr,
                                "sub-array %lu samples: %lu instance%s, average %.3G comparisons, %.3G swaps, %.3G operations\n",
                                q+HISTOGRAM_INDEX1_OFFSET, t, t==1UL?"":"s",
                                (double)u/(double)t, (double)(histoswap[q])/(double)t,
                                (double)(u+histoswap[q])/(double)t);
                        }
                    }
                }
            }
        }

        if ((0U == errs) && (0U != flags['g'])) { /* test glibc qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_GLQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['l'])) { /* test system library qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_QSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['j'])) { /* test insertion sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_ISORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['J'])) { /* test Shell sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_SHELLSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['E'])) { /* test selection sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_SELSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['v'])) { /* test network sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_NETWORKSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['e'])) { /* test heapsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_HEAPSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['r'])) { /* test introsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_INTROSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['y'])) { /* test Yaroslavskiy's dual-pivot qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_YQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['D'])) { /* test dual-pivot qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_DPQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['f'])) { /* test sqrt sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_SQRTSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['x'])) { /* test log sort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_LOGSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['X'])) { /* test smoothsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_SMOOTHSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        if ((0U == errs) && (0U != flags['9'])) { /* test plan9 qsort */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else {
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                errs += test_function(prog, refarray, array, big_array, darray, larray, parray, FUNCTION_P9QSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags, &warray, &uarray, &sarray, marray, &dn);
            }
        }

        z=snlround((double)(n+incr)*(double)mult/(double)div,f,log_arg);
        if (z<=n) z=++n;
    }

#if DEBUG_CODE
    if (2<debug) if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: tests ended", prog);
#endif

    if (NULL!=input_data) free(input_data);
    if (NULL!=warray) { free(warray); warray=NULL; }
    if (NULL!=warray) { free(uarray); uarray=NULL; }
    if (NULL!=warray) { free(sarray); sarray=NULL; }
    free(parray);
    free(larray);
    free(darray);
    free(big_array);
    if (NULL!=array) { free(array); array=NULL; }

#if DEBUG_CODE
    if (2<debug) if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: arrays freed", prog);
#endif

    if (2<debug) if ((0U==flags['h'])&&(NULL!=f)&&(0UL!=errs)) f(LOG_INFO, log_arg, "%s: errs=%lu", prog, errs);
    return (0U==errs?0:1);

#undef buildstr
#undef xbuildstr
}
