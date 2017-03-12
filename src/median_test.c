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
#define BIG_STRUCT_STRING_SIZE      17

#define ASSERT_CODE                 0
#define DEBUG_CODE                  0
#define GENERATOR_TEST              0
#define SILENCE_WHINEY_COMPILERS    0

#define TEST_TIMEOUT                5.0      /* seconds (default value) */

#define RESULTS_COLUMN              72

#define MAX_ARRAY_PRINT             290UL

#define QUICKSELECT_ARG_CHECKS      DEBUG_CODE

/* for even-sized arrays, return lower- (else upper-) median */
#define LOWER_MEDIAN                0

#define BM_INSERTION_CUTOFF 7UL /* 7UL in original B&M code */
#define BM_SWAP_CODE        1    /* 0 uses exchange.h swap */
#define BM_MEDIAN_CODE      1    /* 0 preferred because redundant comparisons can be eliminated */
#define BM_PVINIT           1    /* 0 preferred because PVINIT screws up pointer differences */
#define BM_RECURSE          1    /* 0 preferred because recursion on both regions causes the stack to grow too much */
#define GL_SWAP_CODE        1    /* 0 uses exchange.h swap */
#define MBM_M3_CUTOFF       12UL /* median-of-3 above */
#define MBM_N_CUTOFF        88UL /* ninther above */
#define INSERTION_CUTOFF    6UL /* insertion sort arrays this size or smaller (like B&M) */

#define SAVE_PARTIAL        1

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
#include "compare.h"            /* doublecmp intcmp ulcmp */
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
#include <ctype.h>              /* isalpha isdigit ispunct isspace */
#include <errno.h>
#include <limits.h>             /* *_MAX *_BIT */
#include <math.h>               /* log2 */
#include <regex.h>              /* regcomp regerror regexec regfree */
#include <stdio.h>
#include <stdlib.h>             /* strtol strtoul */
#include <string.h>             /* strrchr strstr */
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

#define OPTSTRING "aAbBc:C:dD:eFghHiIjk:KlLm:M:nNoOPq:Q:r:RsStT:wW:y:z"
#define USAGE_STRING     "[-a] [-A] [-b] [-B] [-c [c1[,c2[,c3[...]]]]] [-C sequences] [-d] [-D [n]] [-e] [-F] [-g] [-h] [-H] [-i] [-I] [-j] [-k col] [-l] [-L] [-m [f[,c[,l]]]] [-M [m3,n]] [-n] [-N] [-o] [-O] [-P] [-q [n[,f[,c[,l]]]]] [-Q timeout] [-r [i[,n[,f]]]] [-R] [-s] [-S] [-t] [-T sequences] [-w] [-W [f,c[,l]]] [-y [n]] [-z] -- [[start incr]] [size [count]]\n\
-a\ttest with McIlroy quicksort adversary\n\
-A\talphabetic (string) data type tests\n\
-b\ttest Bentley&McIlroy qsort\n\
-B\toutput data for drawing box plots\n\
-c [c1[,c2[,...]]]\ttest simplified sort-only quickselect with optional cutoff values\n\
-C sequences\tinclude correctness tests for specified sequences\n\
-d\tturn on debugging output\n\
-D [n]\ttest dual-pivot qsort code with optional insertion sort cutoff\n\
-e\ttest heapsort code\n\
-F\tfloating-point (double) tests\n\
-g\ttest glibc qsort code\n\
-h\tomit program headings (args, parameters, machine)\n\
-H\toutput host information\n\
-i\tuse instrumented comparison functions and report results\n\
-I\tinteger data type tests\n\
-j\ttest insertion sort\n\
-k col\t align timing numbers at column col\n\
-K\twrite integer sequence generated by adversary (if given with -a)\n\
-l\ttest library qsort\n\
-L\tlong integer data type tests\n\
-m [f[,c]]\ttest quickselect median with optional repivot_factor and repivot_cutoff and lopsided_partition_limit\n\
-M [m3,n]\ttest modified Bentley&McIlroy qsort with optional cutoff values for median-of-3 and ninther\n\
-n\tdo nothing except as specified by option flags\n\
-N\ttest NetBSD qsort code\n\
-o\tprint execution costs of operations on basic types\n\
-O\toptimize sampling table for quickselect\n\
-P\tpointer to structure data type tests\n\
-q [n[,f[,c]]]\ttest quickselect sort with optional insertion sort cutoff, repivot_factor, and repivot_cutoff and lopsided_partition_limit\n\
-Q timeout\ttime out tests after timeout seconds\n\
-r [i[,n[,f]]]\ttest introsort sort with optional final insertion sort flag, insertion sort cutoff and recursion depth factor\n\
-R\traw timing output (median time (and comparison counts if requested) only)\n\
-s\tprint sizes of basic types\n\
-S\tstructure data type tests\n\
-t\tthorough tests\n\
-T sequences\tinclude timing tests for specified sequences\n\
-w\ttest qsort_wrapper\n\
-w [f,c]\ttest lopsided partitions in quickselect with optional repivot_factor and repivot_cutoff and lopsided_partition_limit\n\
-y [n]\tYaroslavskiy's dual-pivot sort with optional insertion sort cutoff\n\
-z\tset repeatable random number generator state\n\
start\tbegin testing with array size\n\
incr\tincrement array size (prefix with * for geometric sequence)\n\
size\tnumber of items in each test (maximum size if start is given) (default 10000)\n\
count\tnumber of times to run each test (default 10)"

/* space-saving */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* data types */
#define DATA_TYPE_LONG    1
#define DATA_TYPE_INT     2
#define DATA_TYPE_DOUBLE  3
#define DATA_TYPE_STRUCT  4
#define DATA_TYPE_STRING  5
#define DATA_TYPE_POINTER 6

/* test sequence macros (could be an enum) */
#define TEST_SEQUENCE_STDIN                    0U  /* 0x000001 */ /* must be first (some lopps start at 1 to avoid STDIN) */
#define TEST_SEQUENCE_SORTED                   1U  /* 0x000002 */
#define TEST_SEQUENCE_REVERSE                  2U  /* 0x000004 */
#define TEST_SEQUENCE_ORGAN_PIPE               3U  /* 0x000008 */
#define TEST_SEQUENCE_INVERTED_ORGAN_PIPE      4U  /* 0x000010 */
#define TEST_SEQUENCE_SAWTOOTH                 5U  /* 0x000020 */ /* 3 distinct values */
#define TEST_SEQUENCE_BINARY                   6U  /* 0x000040 */ /* 2 distinct values */
#define TEST_SEQUENCE_CONSTANT                 7U  /* 0x000080 */ /* only 1 value */
#define TEST_SEQUENCE_MANY_EQUAL_LEFT          8U  /* 0x000100 */
#define TEST_SEQUENCE_MANY_EQUAL_MIDDLE        9U  /* 0x000200 */
#define TEST_SEQUENCE_MANY_EQUAL_RIGHT         10U /* 0x000400 */
#define TEST_SEQUENCE_MANY_EQUAL_SHUFFLED      11U /* 0x000800 */
#define TEST_SEQUENCE_RANDOM_DISTINCT          12U /* 0x001000 */
#define TEST_SEQUENCE_RANDOM_VALUES            13U /* 0x002000 */
#define TEST_SEQUENCE_RANDOM_VALUES_LIMITED    14U /* 0x004000 */
#define TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED 15U /* 0x008000 */
#define TEST_SEQUENCE_RANDOM_VALUES_NORMAL     16U /* 0x010000 */
#define TEST_SEQUENCE_HISTOGRAM                17U /* 0x020000 */
#define TEST_SEQUENCE_MEDIAN3KILLER            18U /* 0x040000 */
#define TEST_SEQUENCE_DUAL_PIVOT_KILLER        19U /* 0x080000 */
#define TEST_SEQUENCE_PERMUTATIONS             20U  /* also via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_COMBINATIONS             21U  /* also via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_ADVERSARY                22U  /* also via TEST_TYPE_ADVERSARY */ /* must be last (comparison function changes) */

#define TEST_SEQUENCE_COUNT                    23U


#define EXPECTED_RANDOM_MIN           (0)
#define EXPECTED_RANDOM_MAX           (INT_MAX)

#define MAXIMUM_SAWTOOTH_MODULUS 3  /* ensures maximum 3 distinct values */
#define CONSTANT_VALUE           3  /* a more-or-less random value */

/* 32-bit signed integers in base-36 uses at least 8 chars (incl. '\0') */
/* 64-bit signed integers in base-36 uses at least 15 chars (incl. '\0') */
#if ULONG_MAX > 0xffffffffUL
# if BIG_STRUCT_STRING_SIZE < 15
#  undef BIG_STRUCT_STRING_SIZE
#  define BIG_STRUCT_STRING_SIZE      15
# endif
#else
# if BIG_STRUCT_STRING_SIZE < 8
#  undef BIG_STRUCT_STRING_SIZE
#  define BIG_STRUCT_STRING_SIZE      8
# endif
#endif

struct big_struct {
    double d;
    long l;
    int val;
    char string[BIG_STRUCT_STRING_SIZE];
};

/* definitions:
   size is the size of the element type in bytes as returned by sizeof()
*/

static char median_test_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
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
                  ", BM_SWAP_CODE=" xbuildstr(BM_SWAP_CODE)
                  ", BM_MEDIAN_CODE=" xbuildstr(BM_MEDIAN_CODE)
                  ", BM_PVINIT=" xbuildstr(BM_PVINIT)
                  ", BM_RECURSE=" xbuildstr(BM_RECURSE)
                  ", GL_SWAP_CODE=" xbuildstr(GL_SWAP_CODE)
                  ", MBM_M3_CUTOFF=" xbuildstr(MBM_M3_CUTOFF)
                  ", MBM_N_CUTOFF=" xbuildstr(MBM_N_CUTOFF)
                  ", TEST_TIMEOUT=" xbuildstr(TEST_TIMEOUT)
                  ;
static struct big_struct *input_data=NULL;
/* for instrumented comparisons and swaps: */
static size_t nlt, neq, ngt, nsw;

/* Element swapping code depends on the size and alignment of elements. */
/* Assume basic types sizeof(foo) etc. are powers of 2. */
#define NTYPES 6 /* double, pointer, long, int, short, char */

/* alignment; no fiddly bits */
#define is_aligned(var,shift) (0U==(((unsigned long)(var))&bitmask[(shift)]))

/* insertion sort parameters */
static unsigned int introsort_final_insertion_sort = 1U;
static size_t introsort_insertion_cutoff = 0UL; /* sentinel value */
static size_t introsort_recursion_factor = 2UL;

/* for repivoting */
static size_t npartitions, nrepivot;
static size_t repivot_factor = 9UL;
static size_t repivot_cutoff = 17UL;
static int lopsided_partition_limit = 2;

/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
*/
struct sampling_table_struct {
    size_t min_nmemb;
    size_t samples;
};
#if (SIZE_T_MAX) > 4294967295
# define SAMPLING_TABLE_SIZE 22
#else
# define SAMPLING_TABLE_SIZE 12
#endif
static struct sampling_table_struct sampling_table[SAMPLING_TABLE_SIZE] = {
   {                    1UL,           1UL }, /* single sample, 1/4 position */
   {                   13UL,           3UL }, /* median-of-3, 1/4,1/2,3/4
                                              maybe samples!=sqrt(min_nmemb)
                                          useful min_nmemb range 7UL to 13UL */
   {                   81UL,           9UL }, /* remedian of samples */
   {                  729UL,          27UL },
   {                 6561UL,          81UL },
   {                59049UL,         243UL },
   {               531441UL,         729UL },
   {              4782969UL,        2187UL },
   {             41701511UL,        6561UL },
   {            387420489UL,       19683UL },
   {           3486784401UL,       59049UL },
#if (SIZE_T_MAX) > 4294967295
   {          31381059609UL,      177147UL }, /* silly to include large sizes?
                                              check again in a decade or two */
   {         282429536481UL,      531441UL },
   {        2541865828329UL,     1594323UL },
   {       22876792454961UL,     4782969UL },
   {      205891132094649UL,    14348907UL },
   {     1853020188851841UL,    43046721UL },
   {    16677181699666569UL,   129140163UL },
   {   150094635296999121UL,   387420489UL },
   {  1350851717672992089UL,  1162261467UL },
   { 12157665459056928801UL,  3486784401UL },
   {   (SIZE_T_MAX),         10460353203UL }
#else
   {   (SIZE_T_MAX),              177147UL }
#endif
};

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
int big_struct_intcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;
        return intcmp(&(pa->val),&(pb->val));
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
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;

        return strcmp(pa->string, pb->string);
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
        const struct big_struct **pa = (const struct big_struct **)p1,
            **pb = (const struct big_struct **)p2;

        /* compare as strings (intentionally slow) */
        return big_struct_strcmp(*pa, *pb);
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
fprintf(stderr, "%s line %d: p1=%p, p2=%p\n",__func__,__LINE__,p1,p2);
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

/* Array element swaps: */
/* count is in chars */
/* versions to swap by char, short, int, double */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void charswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        char t;
        if (NULL==pc) {
            do {
                t=*pa, *pa=*pb, *pb=t;
                if (0UL==--count) break;
                pa++, pb++;
            } while (1);
        } else {
            do {
                t=*pa, *pa=*pb, *pb=*pc, *pc=t;
                if (0UL==--count) break;
                pa++, pb++, pc++;
            } while (1);
        }
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void icharswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        char t;
        nsw+=count;
        if (NULL==pc) {
            do {
                t=*pa, *pa=*pb, *pb=t;
                if (0UL==--count) break;
                pa++, pb++;
            } while (1);
        } else {
            do {
                t=*pa, *pa=*pb, *pb=*pc, *pc=t;
                if (0UL==--count) break;
                pa++, pb++, pc++;
            } while (1);
        }
    }
}

#undef GENERIC_SWAP
# define GENERIC_SWAP(type)                 \
    type *px=(type *)pa, *py=(type *)pb, t; \
    count /= sizeof(type);                  \
    if (NULL==pc) {                         \
        do {                                \
            t=*px, *px=*py, *py=t;          \
            if (0UL==--count) break;        \
            px++, py++;                     \
        } while (1);                        \
    } else {                                \
        type *pz=(type *)pc;                \
        do {                                \
            t=*px, *px=*py, *py=*pz, *pz=t; \
            if (0UL==--count) break;        \
            px++, py++, *pz++;              \
        } while (1);                        \
    }


#undef COUNTING_SWAP
# define COUNTING_SWAP(type)                \
    type *px=(type *)pa, *py=(type *)pb, t; \
    count /= sizeof(type);                  \
    nsw+=count;                             \
    if (NULL==pc) {                         \
        do {                                \
            t=*px, *px=*py, *py=t;          \
            if (0UL==--count) break;        \
            px++, py++;                     \
        } while (1);                        \
    } else {                                \
        type *pz=(type *)pc;                \
        do {                                \
            t=*px, *px=*py, *py=*pz, *pz=t; \
            if (0UL==--count) break;        \
            px++, py++, *pz++;              \
        } while (1);                        \
    }

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shortswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        GENERIC_SWAP(short)
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void ishortswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        COUNTING_SWAP(short)
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void intswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        GENERIC_SWAP(int)
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void iintswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        COUNTING_SWAP(int)
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void doubleswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        GENERIC_SWAP(double)
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void idoubleswap(char *pa, char *pb, char *pc, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        COUNTING_SWAP(double)
    }
}

/* determine and return a pointer to an appropriate swap function */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void (*whichswap(char *base, size_t size, unsigned int instrumented))
    (char *, char *, char *, size_t)
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
    switch (t) {
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

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void iswap(char *pa, char *pb, size_t count)
{
    nsw+=count;
    swap2(pa,pb,count);
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
        (V)fprintf(stderr, "%lu:%G", i, target[i]);
    }
    (V)fprintf(stderr, "\n");
}

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
static char *antiqsort_base, *qsort_base;
static size_t antiqsort_typsz, qsort_typsz;
static size_t antiqsort_type, qsort_type;

/* handle reversed-sense of antiqsort array as it arises, or before output */
#define REVERSE_LATER 1

/* freeze implemented as a function */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t freeze(long z)
{
    long l;
#if DEBUG_CODE
(V)fprintf(stderr,"// %s: z=%ld\n",__func__,z);
#endif
    l=antiqsort_nsolid;
    switch (antiqsort_type) {
        case DATA_TYPE_LONG :
            {
                long *plong=(long *)(antiqsort_base + z*antiqsort_typsz);
                *plong = l;
            }
        break;
        case DATA_TYPE_INT :
            {
                int *pint=(int *)(antiqsort_base + z*antiqsort_typsz);
                *pint = l;
            }
        break;
        default :
            (V)fprintf(stderr, "%s: %s line %d: antiqsort_type %d is not valid\n",__func__,source_file,__LINE__,antiqsort_type);
        break;
    }
    antiqsort_nsolid++;
    return l;
}

static long lastx, lasty;

/* comparison function called by qsort; values corresponding to pointers are
   used as indices into the antiqsort array
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int cmp(const void *px, const void *py) /* per C standard */
{
    long a, b, x, y;
    const struct big_struct *pbs;
    const long *plong;

    antiqsort_ncmp++;
    switch (qsort_type) {
        case DATA_TYPE_LONG :
            x = *((const long *)px);
            y = *((const long *)py);
        break;
        case DATA_TYPE_INT :
            x = *((const int *)px);
            y = *((const int *)py);
#if DEBUG_CODE
(V)fprintf(stderr,"// %s: x=%ld, y=%ld\n",__func__,x,y);
#endif
        break;
        case DATA_TYPE_DOUBLE :
            x = (long)(*((const double *)px));
            y = (long)(*((const double *)py));
#if DEBUG_CODE
(V)fprintf(stderr,"// %s: *px=%G, x=%ld, *py=%G, y=%ld\n",__func__,*((const double *)px),x,*((const double *)py),y);
#endif
        break;
        case DATA_TYPE_POINTER :
            pbs = *((const struct big_struct **)px);
            x = pbs->l;
            pbs = *((const struct big_struct **)py);
            y = pbs->l;
        break;
        case DATA_TYPE_STRUCT :
        /*FALLTHROUGH*/
        case DATA_TYPE_STRING :
            pbs = (const struct big_struct *)px;
            x = pbs->l;
            pbs = (const struct big_struct *)py;
            y = pbs->l;
        break;
    }
    /* Compare pointers used to each other and to previously used pointers.
       The plan is to detect the pivot element, which is presumed to be used
       for successive comparisons.   If a different pivot had been assumed,
       change the ordering used (such that previously returned comparison
       results remain valid).
    */
    if (x == y) {
        pivot_candidate = x;
    } else if (x == lastx) {
        pivot_candidate = x;
    } else if (y == lasty) {
        pivot_candidate = y;
    } else if (x == lasty) {
        pivot_candidate = x;
    } else if (y == lastx) {
        pivot_candidate = y;
    }
#if DEBUG_CODE
(V)fprintf(stderr,"// %s line %d: lastx=%ld, x=%ld, pivot_candidate=%ld, lasty=%ld, y=%ld\n",__func__,__LINE__,lastx,x,pivot_candidate,lasty,y);
#endif
    A(x<antiqsort_n);A(y<antiqsort_n);
    switch (antiqsort_type) {
        case DATA_TYPE_LONG :
            plong = (const long *)(antiqsort_base + x*antiqsort_typsz);
            a = *plong;
            plong = (const long *)(antiqsort_base + y*antiqsort_typsz);
            b = *plong;
        break;
        case DATA_TYPE_INT :
            plong = (const long *)(antiqsort_base + x*antiqsort_typsz);
            a = *plong;
            plong = (const long *)(antiqsort_base + y*antiqsort_typsz);
            b = *plong;
        break;
        default :
            (V)fprintf(stderr, "%s: %s line %d: antiqsort_type %d is not valid\n",__func__,source_file,__LINE__,antiqsort_type);
        break;
    }
#if DEBUG_CODE
(V)fprintf(stderr,"// %s line %d: a=%ld, b=%ld\n",__func__,__LINE__,a,b);
#endif
    /* brackets added BL */
    if ((a==antiqsort_gas) && (b==antiqsort_gas)) {
        if (x==y) {
            /* must return 0 for cmp(x,x) no change to a */
            (void)freeze(x);
        } else {
            if (x == pivot_candidate) {
                a = freeze(x);
            } else if (y == pivot_candidate) {
                b = freeze(y);
            } else { /* x!=y */
                a = freeze(x); /* might pick wrong element as pivot; may have to reverse */
            }
        }
    }

    lastx=x, lasty=y;
    /* must return 0 for cmp(x,x) */
    if ((x==y)&&(a!=b)) (V)fprintf(stderr, "// %s line %d: ERROR: x==y=%ld, a=%ld, b=%ld, antiqsort_gas=%ld: will return non-zero\n",__func__,__LINE__,x,a,b,antiqsort_gas);
    /* initialized to distinct values; must not return 0 for cmp(x,y), x!=y */
    if ((x!=y)&&(a==b)) (V)fprintf(stderr, "// %s line %d: ERROR: x=%ld, y=%ld, a==b=%ld, antiqsort_gas=%ld: will return zero\n",__func__,__LINE__,x,y,b,antiqsort_gas);
#if REVERSE_LATER
    if (a > b) return 1; else if (a < b) return -1; else return 0;
#else
#if DEBUG_CODE
(V)fprintf(stderr,"// %s line %d: a=%d, b=%d, will return %d\n",__func__,__LINE__,a,b,((a>b)?1:(a<b)?-1:0));
#endif
    if (a > b) return 1; else if (a < b) return 1; else return 0;
#endif
 /* only the sign matters */
}

/* instrumented version */
static int icmp(const void *px, const void *py) /* per C standard */
{
    int c;

    switch ((c=cmp(px,py))) { /* only -1, 0, 1 */
        case -1 : nlt++;
        break;
        case 0 :  neq++;
        break;
        case 1 :  ngt++;
        break;
    }
    return c;
}
/* ********** modified into a separate function for intitalization ************ */
/* added args: pv points to array used by qsort, alt to array used by antiqsort; sz, type, altsz, alt_type correspond to those */
static void initialize_antiqsort(size_t n, char *pv, const size_t sz, char *alt, const size_t altsz, int type, int alt_type)
{
    size_t i;

    A(NULL!=pv);A(NULL!=alt);A(pv!=alt);
    qsort_base = pv;
    qsort_type = type;
    qsort_typsz = sz;
    antiqsort_n = n;
    antiqsort_base = alt;
    antiqsort_type = alt_type;
    antiqsort_typsz = altsz;
    antiqsort_gas = n - 1UL;
    antiqsort_nsolid = 0;
    pivot_candidate = n;
    antiqsort_ncmp = 0UL;
    lastx = lasty = n;
#if DEBUG_CODE
(V)fprintf(stderr,"// %s: n=%lu, pv=%p, sz=%lu, alt=%p, altsz=%lu, type=%d, alt_type=%d\n",__func__,n,pv,sz,alt,altsz,type,alt_type);
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
                    (V)snl(pbs->string, BIG_STRUCT_STRING_SIZE, NULL, NULL, (long)i, 36, '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
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
                    (V)snl(pbs->string, BIG_STRUCT_STRING_SIZE, NULL, NULL, (long)i, 36, '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
                    pbs->d = (double)i;
                }
            break;
        }
        /* initialize the array that antiqsort uses */
        switch (antiqsort_type) {
            case DATA_TYPE_LONG :
                {
                    long *plong;
                    plong = (long *)(antiqsort_base + antiqsort_typsz*i);
                    *plong = (long)antiqsort_gas;
                }
            break;
            case DATA_TYPE_INT :
                {
                    int *pint;
                    pint = (int *)(antiqsort_base + antiqsort_typsz*i);
                    *pint = (int)antiqsort_gas;
                }
            break;
            default :
                (V)fprintf(stderr, "%s: %s line %d: antiqsort_type %d is not valid\n",__func__,source_file,__LINE__,antiqsort_type);
            break;
        }
    }
}
/* ***************************************************************************** */

/* ******************** Bentley & McIlroy qsort ******************************** */
/* namespace changed */
/* qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

#if BM_SWAP_CODE
    /*assume sizeof(long) is a power of 2 */
#define SWAPINIT(a, es) swaptype =         \
    (a-(char*)0 | es) % sizeof(long) ? 2 : es > sizeof(long);
#define swapcode(TYPE, parmi, parmj, n) {  \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    nsw += n/sizeof(TYPE);                 \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while ((n -= sizeof(TYPE)) > 0);     \
}
#include <stddef.h>
static void swapfunc(char *a, char *b, size_t n, int swaptype)
{   if (swaptype <= 1) swapcode(long, a, b, n)
    else swapcode(char, a, b, n)
}
#define bmswap(a, b)                       \
    if (swaptype == 0) {                   \
        long t = *(long*)(a);              \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
        nsw++;                             \
    } else                                 \
        swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)
#else
#define SWAPINIT(a,b)      swaptype=1
#define bmswap(a,b) iswap(a,b,1UL,es)
#define vecswap(a,b,n) iswap(a,b,n,1UL)
#endif

#if BM_PVINIT
# define PVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; bmswap(pv, pm); }    \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }
#else
# define PVINIT(pv,pm)   pv=a; bmswap(pv, pm);
#endif /* BM_PVINIT */

#define bmmin(x, y) ((x)<=(y) ? (x) : (y))

static char *med3(char *a, char *b, char *c, int (*compar)(const void *, const void *))
{
#if BM_MEDIAN_CODE
        return compar(a, b) < 0 ?
                  (compar(b, c) < 0 ? b : compar(a, c) < 0 ? c : a)
                : (compar(b, c) > 0 ? b : compar(a, c) > 0 ? c : a);
#else
    char *pp, *pn;
    int q;

    q = compar(a,b);
    if (0 != q) {
        if (0>q) pp=b; else pp=a; /* max */
        q = compar(b,c);
        if (0 != q) {
            if (0>q) pn=b; else pn=c; /* min */
            if ((pp!=pn) && ((pp==b) || (pn==b))) {
                q=compar(a,c);
                if (pp==b) { if (0>q) return c; else return a; /* max */
                } else if (pn==b) { if (0>q) return a; else return c; /* min */ }
            }
        }
    }
    return b;
#endif
}

void bmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s; /* added BL */

bmloop:   SWAPINIT(a, es);
        if (n < (BM_INSERTION_CUTOFF)) {         /* Insertion sort on smallest arrays */
                for (pm = a + es; pm < a + n*es; pm += es)
                        for (pl = pm; pl > a && compar(pl-es, pl) > 0; pl -= es)
                                bmswap(pl, pl-es);
                return;
        }
        pm = a + (n/2)*es;    /* Small arrays, middle element */
        if (n > 7) {
                pl = a; /* moved outside of pivot selection guard */
                pn = a + (n-1)*es; /* moved outside of pivot selection guard */
                if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                        s = (n/8)*es;
                        pl = med3(pl, pl+s, pl+2*s, compar);
                        pm = med3(pm-s, pm, pm+s, compar);
                        pn = med3(pn-2*s, pn-s, pn, compar);
                }
                pm = med3(pl, pm, pn, compar); /* Mid-size, med of 3 */
                pl = a; /* reset (BL) */
        }
        pn = a + n*es;
        PVINIT(pv, pm);       /* pv points to partition value */
        pa = pb = a; /* original code; redundant comparison pa vs. pa */
        pc = pd = a + (n-1)*es;
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
        npartitions++;
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
#if BM_RECURSE
        if ((s = pd-pc) > es) bmqsort(pn-s, s/es, es, compar);
#else
        if ((s = pd-pc) > es) {
                /* Iterate rather than recurse to save stack space */
                a = pn - s;
                n = s / es;
                goto bmloop;
        }
#endif
}
/* ************************************************************************** */

#ifndef bmmin
#define bmmin(x, y) ((x)<=(y) ? (x) : (y))
#endif

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
                                bmswap(pl, pl - es);
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
        bmswap(a, pm);
        pa = pb = (char *) a + es;

        pc = pd = (char *) a + (n - 1) * es;
        for (;;) {
                while (pb <= pc && (cmp_result = compar(pb, a)) <= 0) {
                        if (cmp_result == 0) {
                                bmswap(pa, pb);
                                pa += es;
                        }
                        pb += es;
                }
                while (pb <= pc && (cmp_result = compar(pc, a)) >= 0) {
                        if (cmp_result == 0) {
                                bmswap(pc, pd);
                                pd -= es;
                        }
                        pc -= es;
                }
                if (pb > pc)
                        break;
                bmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        npartitions++;

        pn = (char *) a + n * es;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower == region */
        /* pb-pa is the number of chars in the < block */
        r = bmmin(pa - (char *) a, pb - pa);
        vecswap(a, pb - r, r);
        /* after the swap (above), the start of the == section is at pb-r */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper == region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper == block */
        /* pd-pc is the number of chars in the > block */
        r = bmmin((size_t)(pd - pc), pn - pd - es);
        vecswap(pb, pn - r, r);

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

/* Modified Bentley&McIlroy qsort:
      ternary median-of-3
      type-independent deferred pivot swap
      improved sampling for singleton, median-of-3, ninther
      recurse on small region, iterate large region
      avoid self-swapping pb,pc
      improved swapping
      don't repeat initialization during iteration
      uses INSERTION_CUTOFF rather than BM_INSERION_CUTOFF (normally equivalent)
   No remedian of samples
   No break-glass mechanism to prevent quadratic behavior
   No order statistic selection
*/
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

/* programmable cutoffs; a bit slower than compile-time constants */
static size_t mbm_m3_cutoff = MBM_M3_CUTOFF;
static size_t mbm_n_cutoff = MBM_N_CUTOFF;

void mbmqsort_internal(void *a, size_t n, size_t es,
    int (*compar)(const void *, const void *),
    void (*swapfunc)(char *, char *, char *, size_t))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
        size_t d, r;
        int cmp_result;

        assert(a != NULL || n == 0 || es == 0);
        assert(compar != NULL);

loop:   if (n <= (INSERTION_CUTOFF)) {
                for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
                        for (pl = pm; pl > (char *) a && compar(pl - es, pl) > 0;
                             pl -= es)
                                swapfunc(pl, pl - es, NULL, es);
                return;
        }
        d = es * (n / 4UL);
        pm = (char *) a + d; /* 1/4 */
        if (n > mbm_m3_cutoff) {
                pl = (char *)a + es * (n / 2UL); /* 1/2 */
                pn = (char *) a + (n - 1UL) * es - d; /* 3/4 */
                if (n > mbm_n_cutoff) {
                        d = (n / 12UL) * es; /* twelfths */
                        r = 2UL*d;
                        pm = fmed3(pm-r, pm-d, pm, compar); /* 1,2,3 */
                        pl = fmed3(pl-d, pl, pl+d, compar); /* 5,6,7 */
                        pn = fmed3(pn, pn+d, pn+r, compar); /* 9,10,11 */
                }
                pm = fmed3(pl, pm, pn, compar);
        }
        pa = pb = (char *) a;
        pc = pd = (char *) a + (n - 1) * es;
        for (;;) {
                while (pb <= pc && (cmp_result = compar(pb, pm)) <= 0) {
                        if (cmp_result == 0) {
                                swapfunc(pa, pb, NULL, es);
                                pm=pa;
                                pa += es;
                        }
                        pb += es;
                }
                while (pb <= pc && (cmp_result = compar(pc, pm)) >= 0) {
                        if (cmp_result == 0) {
                                swapfunc(pc, pd, NULL, es);
                                pm=pd;
                                pd -= es;
                        }
                        pc -= es;
                }
                if (pb >= pc)
                        break;
                swapfunc(pb, pc, NULL, es);
                pb += es;
                pc -= es;
        }
        npartitions++;

        pn = (char *) a + n * es;
        /* | = |  <  |  >  | = | */
        /*    a     b     d     n*/
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower == region */
        /* pb-pa is the number of chars in the < block */
        r = bmmin(pa - (char *) a, pb - pa);
        if (0<r) swapfunc(a, pb-r, NULL, r);
        /* after the swap (above), the start of the == section is at pb-r */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper == region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper == block */
        /* pd-pc is the number of chars in the > block */
        r = bmmin((size_t)(pd - pc), pn - pd - es);
        if (0<r) swapfunc(pb, pn-r, NULL, r);

        r = pb - pa; /* size (only) of < region */
        d = pd - pc; /* size (only) of > region */
        if (r<=d) {
                /* recurse on small pa..pb, iterate large */
                if (r>es) mbmqsort_internal(a, r/es, es, compar, swapfunc);
                if (d>es) {
                    a = pn-d;
                    n = d/es;
                    goto loop;
                }
        } else {
                if (d>es) mbmqsort_internal(pn-d, d/es, es, compar, swapfunc);
                if (r>es) {
                    n = r/es;
                    goto loop;
                }
        }
}

void mbmqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *))
{
    mbmqsort_internal(a, n, es, compar, whichswap(a,es,1U));
}

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
#if GL_SWAP_CODE
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
#else
# define SWAP(a,b,size)   iswap(a,b,1UL,size)
#endif /* GL_SWAP_CODE */

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
    void (*swapfunc)(char *, char *, char *, size_t))
{
    char *pa, *pb, *pc, *pl, *pu;
    size_t nmemb=beyond-first;

#if 0
fprintf(stderr, "%s line %d: base=%p, first=%lu, beyond=%lu\n",__func__,__LINE__,base,first,beyond);
print_int_array(base,first,beyond-1UL);
#endif
    if (2UL>nmemb) return;
    for (pa=pl=base+first*size,pu=base+(beyond-1UL)*size; pa<pu; pa+=size)
        for (pb=pa; (pb>=pl)&&(0<compar(pb,pc=pb+size)); pb-=size)
            swapfunc(pb,pc,NULL,size);
#if 0
fprintf(stderr, "%s line %d:\n",__func__,__LINE__);
print_int_array(base,first,beyond-1UL);
#endif
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void isort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    isort_internal(base,0UL,nmemb,size,compar,whichswap(base,size,1U));
}

/* simplified and full-blown quickselect-based qsort:
   Like modified B&M qsort, except:
      add sampling and remedian of samples, with sampling table
      pivot selection moves elements
*/
/* medians of sets of 3 elements */
/* Given base, size, and compar as for qsort, plus a row offset and sample
   offset, both in chars, find medians of sets of 3 elements where the three
   elements for each median are taken from three rows starting with base, and
   each subsequent set of three elements is offset from the previous set by
   sample_offset. As each median is determined, it is placed in the position of
   the middle element of the set.
   The return value is a pointer to the first median (which has been moved as
   described above).
   N.B. base is not necessarily the first element in the array passed to qsort.
*/
/* called from select_pivot() and repivot() */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *medians3(char *base, size_t size,
    int(*compar)(const void *, const void *), size_t row_offset,
    size_t sample_offset, size_t row_samples,
    void (*swapfunc)(char *, char *, char *, size_t))
{
    int c;
    char *pa, *pb;
    size_t n, o;

    for (n=o=0UL; n<row_samples; n++,o+=sample_offset) {
        pa=base+o;
        pb=pa+row_offset; /* middle element */
        /* Optimized ternary median-of-3: 1-3 comparisons.
           Minimum number of comparisons (because in the qsort model (external
           function for comparison), comparisons are expensive).  If any two
           elements (of the 3) compare equal, either can be chosen for the
           median (the value is the median of the 3, regardless of the value of
           the third element).  Likewise, the median may be determined by the
           first two comparisons, e.g. a < b and b < c.  In the worst case, 3
           comparisons are required. The median element is swapped to the middle
           position.
        */
        c=compar(pa,pb);
        if (0!=c) {
            char *pc=pb+row_offset;
            int d=compar(pb,pc);
            if (0!=d) {
                if ((0<d)&&(0>c)) { /* pc<pb, pa<pb; pb is max. */
                    if (0>=compar(pa,pc)) swapfunc(pb,pc,NULL,size); /* OK */
                    else swapfunc(pc,pb,pa,size);
                } else if ((0>d)&&(0<c)) { /* pb is min. */
                    if (0<compar(pa,pc)) swapfunc(pa,pb,pc,size);
                    else swapfunc(pa,pb,NULL,size); /* OK */
                }
            }
        }
    }
    return base+row_offset; /* middle row */
}

/* fast pivot selection using a single sample, median-of-3, or remedian of samples */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *select_pivot(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void*), size_t *pk,
    void (*swapfunc)(char *, char *, char *, size_t))
{
    size_t n, nmemb, p, q, r, s, t;
    char *pivot;

    nmemb=beyond-first;
    for (q=0UL; q<SAMPLING_TABLE_SIZE; q++)
        if (nmemb<sampling_table[q].min_nmemb)
            break; /* stop looking */
    if (0UL<q) q--; /* sampling table index */
    n=sampling_table[q].samples; /* total samples (a power of 3) */
#if 0 /* should decrease, but increases comparison count */
    /* exception; when sorting (NULL==pk), use minimum 3 samples */
    if ((NULL==pk)&&(3UL>n)) n=sampling_table[++q].samples;
#endif
#if 0
fprintf(stderr, "%s line %d: base=%p, first=%lu, beyond=%lu, nmemb=%lu, q=%lu, n=%lu\n",__func__,__LINE__,base,first,beyond,nmemb,q,n);
#endif
    switch (n) {
        case 1UL :
            /* best for organ-pipe, OK others */
            pivot=base+(first+(nmemb>>2))*size;
            s=r=0UL;
        break;
        case 3UL :
            s=r=(nmemb>>2)*size;                        /* 1/4 spacing */
            pivot=base+size*(first+(nmemb>>1))-r;      /* first sample */
        break;
        default : /* > 3, power of 3 */
            t = n/3UL;                              /* samples per row */
            if (0UL<t) s=(p=nmemb>>2)/t; else s=0UL; /* sample spacing */
            if (1UL>=t) r=((nmemb+2UL)>>2); else r=p+s; /* row spacing */
            s*=size, r*=size;                      /* spacing in chars */
            pivot=base+size*(first+(nmemb>>1))       /* middle element */
                -r                              /* -> middle first row */
                -(t>>1)*s;                       /* - half row samples */
        break;
    }
    switch (n) {
        case 1UL :
        break;
#if 1
        case 3UL :
            pivot = fmed3(pivot,pivot+s,pivot+(s<<1),compar);
        break;
        case 9UL :
            {
                char *pa, *pb;

                t=(s<<1);
                p=(r<<1);
                pa = fmed3(pivot,pivot+s,pivot+t,compar);
                pb = fmed3(pivot+r,pivot+r+s,pivot+r+t,compar);
                pivot = fmed3(pivot+p,pivot+p+s,pivot+p+t,compar);
                pivot = fmed3(pa,pb,pivot,compar);
            }
        break;
        case 27UL :
            {
                char *px[27];

                for (p=0UL; p<9UL; p++) { /* 9 columns */
                    /* 3 rows per column */
                    for (t=0UL; t<3UL; t++)
                       px[3UL*p+t] = pivot+p*s+t*r;
                    /* pointers to column medians are first 9 pointers */
                    px[p] = fmed3(px[3UL*p],px[3UL*p+1UL],px[3UL*p+2UL],compar);
                }
                px[0] = fmed3(px[0],px[3],px[6],compar);
                px[1] = fmed3(px[1],px[4],px[7],compar);
                px[2] = fmed3(px[2],px[5],px[8],compar);
                pivot = fmed3(px[0],px[1],px[2],compar);
            }
        break;
#endif
        default : /* power of 3, in-place sort3 */
            n=sampling_table[--q].samples;
            pivot=medians3(pivot,size,compar,r,s,n,swapfunc);
            while (0UL<q) {
                n=sampling_table[--q].samples;
                pivot=medians3(pivot,size,compar,n*s,s,n,swapfunc);
            }
        break;
    }
#if 0
fprintf(stderr, "%s line %d: pivot=%p[%lu], r=%lu, s=%lu\n",__func__,__LINE__,pivot,(pivot-base)/size,r,s);
print_int_array(base,first,beyond-1UL);
#endif
    return pivot;
}

/* forward declaration */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_loop(char *base, size_t first, size_t beyond, const size_t size,
    int(*compar)(const void *,const void *), const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapfunc)(char *, char *, char *, size_t)
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
   );

/* repivot using median-of-medians */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *repivot(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void*),
    void (*swapfunc)(char *, char *, char *, size_t),
    char **ppc, char **ppd, char **ppe, char **ppf)
{
    size_t n, nmemb, mid;
    char *pa, *pivot, *pl;

    nmemb=beyond-first;
    mid=first+(nmemb>>1);
    pl=base+first*size;
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
    n=nmemb, pa=pl;
    /* Medians of sets of 3 elements. */
    n/=3UL;    /* number of complete sets */
    if (1UL<=n)
        pa=medians3(pl,size,compar,n*size,size,n,swapfunc);
    *ppc=pa;
    if (1UL<n) { /* median of medians */
        size_t karray[1];
        karray[0]=mid; /* upper-median for even size arrays */
        first=(pa-base)/size;
        beyond=first+n;
        *ppf=base+beyond*size;
        quickselect_loop(base,first,beyond,size,compar,karray,0UL,1UL,swapfunc
#if SAVE_PARTIAL
            ,ppd,ppe
#endif
        );
#if SAVE_PARTIAL
        /* Middle third of array (medians) is partitioned. */
        pivot=*ppd;
#else
        *ppe=*ppd=pivot=base+karray[0]*size; /* pointer to median of medians */
#endif
    } else {
        *ppf=pa+n*size;
    }
    return pivot;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int should_repivot(size_t nmemb, size_t n)
{
    if (((repivot_cutoff)<=nmemb)&&((n/(repivot_factor))>=nmemb-n-1UL))
        return 1;
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
void partition(char *base, size_t first, size_t beyond, char *pivot,
    size_t size, int(*compar)(const void *,const void*),
    void (*swapfunc)(char *, char *, char *, size_t),
    size_t *peq, size_t *pgt)
{
    char *pa, *pb, *pc, *pe, *pf, *pl, *pu;
    int c, d;
    size_t n, p;

#if 0
fprintf(stderr, "%s line %d: base=%p, first=%lu, beyond=%lu, pivot=%p\n",__func__,__LINE__,base,first,beyond,pivot);
print_int_array(base,first,beyond-1UL);
#endif
    pl=base+size*first;
    pa=pb=pc=pl;
    pe=(pf=pu=base+beyond*size)-size;
    while (1) { /* loop - partitioning */
        while ((pc<=pe)&&(0<=(c=compar(pivot,pc)))) { /* scan -> */
            if (c==0) { swapfunc(pivot=pb,pc,NULL,size); pb+=size; }
            pc+=size;
        }
        while ((pc<=pe)&&(0>=(d=compar(pivot,pe)))) { /* scan <- */
            if (d==0) { pf-=size; swapfunc(pe,pivot=pf,NULL,size); }
            pe-=size;
        }
        if (pc>pe) break;
        swapfunc(pc,pe,NULL,size); /* <, > */
        pc+=size; pe-=size;
    }
    pe+=size; /* now start of > region */
    if (pf<pu) { /* pe unchanged if pf>=pu */
        if (pe<pf) { /* pe= original pu if pf>=pe */
            p=pu-pf, n=pf-pe; if (p<n) n=p;
            swapfunc(pe,pu-n,NULL,n); pe+=p;
        } else pe=pu;
    }
    if (pb<pc) { /* pb= original pl if pb>=pc */
        if (pl<pb) { /* pb= original pc if pb>=pa */
            p=pc-pb, n=pb-pl; if (p<n) n=p;
            swapfunc(pl,pc-n,NULL,n); pb=pl+p;
        } else pb=pc;
    } else pb=pl;
    /* |       <            |         =           |      >      | */
    /*  pl                   b                     e             u*/
    if (NULL!=peq) *peq=(pb-base)/size;
    if (NULL!=pgt) *pgt=(pe-base)/size;
#if 0
fprintf(stderr, "%s line %d: pb=%p[%lu], pe=%p[%lu]\n",__func__,__LINE__,pb,(pb-base)/size,pe,(pe-base)/size);
print_int_array(base,first,beyond-1UL);
#endif
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
    void (*swapfunc)(char *, char *, char *, size_t),
    size_t *peq, size_t *pgt)
{
    char *pa, *pb, *pg, *ph, *pl, *pu;
    int c, d;
    size_t n, p;

    pa=pb=pl=base+size*first;
    ph=pu=base+size*beyond;
    pg=ph-size;
    /* |   ?   : < :=p=: > :   ?   | */
    /*          c   d   e   f       u*/
    /* reinitialize for repartitioning */
    /* |   ?   : < :=p=: > :   ?   | */
    /*  pl      c   d   e   f       u*/
    /*  a=b                       g h*/
    /* McGeoch & Tygar suggest that partial partition information
       from median-of-medians might be used to avoid recomparisons
       during repartitioning.
    */
    /* Skip over some already-partitioned sections. */
    /* Separate partitioning loop here to avoid overhead when not
       preserving partial partition. Subtleties involved: empty
       regions (pc-pd, pe-pf, pg-ph, pl-pa, pa-pb), whether pb or pg
       reaches the partially partitioned region first.
    */
    while (1) { /* partitioning loop */
        /* +------------------------------------------------------------+ */
        /* |    =    |    <    |    ? : < :=: > :   ?   |   >   |   =   | */
        /* +------------------------------------------------------------+ */
        /*  pl        a         b      c   d e   f     G g       h       u*/
        A(pl<=pa);A(pa<=pb);A(pg<=ph);A(ph<=pu);
        A(pc<=pd);A(pd<pe);A(pe<=pf);
        while (pb<=pg) { /* scan -> */
            /* 1. Like regular partitioning except when within
                  partial partition from selection.
            */
            while (((pb<pc)||(pf<=pb))&&(0<=(c=compar(pivot,pb)))) {
                if (0==c) { swapfunc(pivot=pa,pb,NULL,size); pa+=size; }
                pb+=size;
            }
            A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
            /* 2. Within partial partition; no recomparisons. */
            if ((pc<=pb)&&(pb<pf)) { /* partitioned */
                /* First skip over < elements, possibly past pg. */
                if (pd<=pg) { /* If pg didn't, move = block. */
                    if (pb<pe) { /* move = elements as a block */
                        if (pa<pd) { /* pa=pe if pd<=pa */
                            p=pe-pd, n=pd-pa; if (p<n) n=p;
                            swapfunc(pa,pe-n,NULL,n); pa+=p;
                        } else pa=pe;
                        pivot=pa-size;
                        pb=pe;
                    }
                } else pb=pg+size; /* < elements */
                if ((pb>=pe)&&(pb<pf)) break; /* at > element */
            } else break; /* not partitioned region; break loop */
        }
        A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
        while (pb<=pg) { /* scan <- */
            /* 3. Like regular partitioning except when within partial
                  partition from selection.
            */
            while (((pf<=pg)||(pg<pc))&&(0>=(d=compar(pivot,pg)))) {
                if (0==d) { ph-=size; swapfunc(pg,pivot=ph,NULL,size); }
                pg-=size;
            }
            A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
            /* 4. Within partial partition; no recomparisons. */
            if ((pc<=pg)&&(pg<pf)) { /* partitioned */
                /* First skip over > elements, possibly past pb. */
                if (pb<pe) { /* If pb didn't, move = block. */
                    if (pd<=pg) { /* move = elements as a block */
                        if (pe<ph) { /* ph= original pd if pe>=ph */
                            p=ph-pe, n=pe-pd; if (p<n) n=p;
                            swapfunc(pd,ph-n,NULL,n); ph-=n;
                        } else ph=pd;
                        pivot=ph;
                        pg=pd-size;
                    }
                } else pg=pb-size; /* > elements */
                if ((pg<pd)&&(pc<=pg)) break; /* at < element */
            } else break; /* not partitioned region; break loop */
        }
        A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
        if (pb>pg) break;
        A(pb!=pg); /* no self-swapping */
        swapfunc(pb,pg,NULL,size);
        pb+=size, pg-=size;
    }
    pg+=size; /* now start of > region */
    /* |    =  |      <             |             >    |   =    | */
    /*  pl      a                    g                  h        u*/
    /* swap > and upper = regions, set pg to start of > */
    pb=pg; /* = region will start here */
    A(pl<=pa); A(pb==pg); A(pg<=ph); A(ph<=pu);
    if (ph<pu) { /* pg unchanged if ph>=pu */
        if (pg<ph) { /* pg= pu if ph<=pg */
            p=pu-ph, n=ph-pg; if (p<n) n=p;
            swapfunc(pg,pu-n,NULL,n); pg+=p;
        } else pg=pu;
    }
    /* |    =  |      <           |        =      |      >      | */
    /*  pl      a                  b               g             u*/
    /* swap left = and < regions */
    A(pl<=pa); A(pa<=pb); A(pg<=pu);
    if (pa<pb) { /* pa= original pl if pa>=pb */
        if (pl<pa) { /* pa= original pb if pa<=pl */
            p=pb-pa, n=pa-pl; if (p<n) n=p;
            swapfunc(pl,pb-n,NULL,n); pa=pl+p;
        } else pa=pb;
    } else pa=pl;
    A(pl<=pa); A(pa<=pg); A(pg<=pu);
    /* |       <            |         =           |      >      | */
    /*  pl                   a                     g             u*/
    if (NULL!=peq) *peq=(pa-base)/size;
    if (NULL!=pgt) *pgt=(pg-base)/size;
}
#endif

/* simplified quickselect */
void sqsort_internal(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *),
    void(*swapfunc)(char *, char *, char *, size_t))
{
    char *pl;  /* left (or lowest) end of array being processed */
    char *po;  /* origin of array (maintained so that ranks can be computed) */
    char *pb, *pe, *pivot;
    size_t n, p, q, r;

    pl=po=(char *)base;

sloop: ;
    if (nmemb<=(INSERTION_CUTOFF)) {
        isort_internal(pl,0UL,nmemb,size,compar,swapfunc);
/* <- */return; /* Done; */
    }
    n=(pl-po)/size;
    pivot = select_pivot(po, n, n+nmemb, size, compar, NULL, swapfunc);
    partition(po,n,n+nmemb,pivot,size,compar,swapfunc,&q,&p);
    npartitions++;
    pb=po+q*size;
    pe=po+p*size;
    q-=n;
    if (n+nmemb>p) r=n+nmemb-p; else r=0UL;  /* size of the > region */
    if (q<r) { /* > region is larger */
        if (q>1UL) sqsort_internal(pl, q, size, compar, swapfunc);
        if (r>1UL) {
            pl=pe;
            nmemb=r;
            goto sloop;
        }
    } else { /* < region is larger, or regions are the same size */
        if (r>1UL) sqsort_internal(pe, r, size, compar, swapfunc);
        if (q>1UL) {
            nmemb=q;
            goto sloop;
        }
    }
}

void sqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    sqsort_internal(base,nmemb,size,compar,whichswap(base,size,1U));
}

/* modified quickselect sort; always get worst-case pivot for given
   repivot_factor and repivot_cutoff that would just avoid repivoting
*/
void wqsort_internal(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapfunc)(char *, char *, char *, size_t))
{
    char *pl;  /* left (or lowest) end of array being processed */
    char *po;  /* origin of array (maintained so that ranks can be computed) */
    size_t s;  /* general size_t variable */
    int i;              /* general integer variables */
    int nlopsided=0;
    char *pb, *pe, *pivot;
    size_t n, p, q, r, t, karray[1];
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
    if (nmemb<=(INSERTION_CUTOFF)) {
        isort_internal(pl,0UL,nmemb,size,compar,swapfunc);
/* <- */return; /* Done; */
    }
    /* normal pivot selection (for comparison and swap counts) */
    n=(pl-po)/size;
    pivot = select_pivot(po, n, n+nmemb, size, compar, NULL, swapfunc);
    /* get pivot at rank that just avoids repivoting (minimum possible) */
    /* minimum possible rank is determined by number of samples used for pivot selection */
    /* 3^^p samples -> minimum rank is 2^^p-1 */
    for (p=0UL; p<SAMPLING_TABLE_SIZE; p++)
        if (nmemb<sampling_table[p].min_nmemb)
            break; /* stop looking */
    if (0UL<p) p--; /* sampling table index */
    for (t=1UL,q=0UL; q<p; q++,t<<=1) ;
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
    /* don't count comparisons or swaps for pivot selection here */
    p=neq, q=nlt, r=ngt, t=nsw;
    quickselect(pl, nmemb, size, compar, karray, 1UL);
    pivot=pl+size*karray[0];
    /* restore comparison and swap counts */
    neq=p, nlt=q, ngt=r, nsw=t;
    partition(po,n,n+nmemb,pivot,size,compar,swapfunc,&q,&p);
    npartitions++;
    pb=po+q*size;
    pe=po+p*size;
    q-=n;
    if (n+nmemb>p) r=n+nmemb-p; else r=0UL;  /* size of the > region */
    /* |       <            |         =           |      >      | */
    /*  pl                   b                     e             u*/
    if (q<r) { /* > region is larger */
        if (q>1UL) wqsort_internal(pl, q, size, compar, swapfunc);
        if (r>1UL) {
            pl=pe;
            nmemb=r;
            goto wloop;
        }
    } else { /* < region is larger, or regions are the same size */
        if (r>1UL) wqsort_internal(pe, r, size, compar, swapfunc);
        if (q>1UL) {
            nmemb=q;
            goto wloop;
        }
    }
}

void wqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    wqsort_internal(base,nmemb,size,compar,whichswap(base,size,1U));
}

/* full-blown quickselect */
static size_t quickselect_insertion_cutoff = 6UL;

#define NREGIONS (LONG_BIT)+8

/* basic quickselect loop (using system stack) */
/* modular version of inlined code in quickselect.c */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_loop(char *base, size_t first, size_t beyond, const size_t size,
    int(*compar)(const void *,const void *),
    const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapfunc)(char *, char *, char *, size_t)
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
   )
{
    size_t nmemb=beyond-first;
    int c=0;

    while ((NULL!=pk)||(nmemb>quickselect_insertion_cutoff)) {
        size_t lk, p, q, r, rk, s, t;
        char *pivot=select_pivot(base,first,beyond,size,compar,pk,swapfunc);
        partition(base,first,beyond,pivot,size,compar,swapfunc,&p,&q);
check_sizes: ;
        if (NULL==pk) npartitions++;
#if SAVE_PARTIAL
        /* only applicable for median-of-medians */
        if ((0UL==firstk)&&(1UL==beyondk)&&(NULL!=ppeq)&&(NULL!=ppgt))
            *ppeq=base+size*p, *ppgt=base+size*q;
#endif
        /* sizes of < and > regions (elements) */
        s=p-first, t=beyond-q;
        /* revised range of order statistic ranks */
        /* < region indices [first,p), order statistics [firstk,lk) */
        /* > region indices [q,beyond), order statistics [rk,beyondk) */
        if (NULL==pk) { /* sorting */
            lk=beyondk, rk=firstk;
        } else {
            size_t ll, lr, rl, rr;

            /* binary search through pk to find limits for each region */
            ll=rl=firstk, lr=rr=beyondk;
            lk=rk=ll+((lr+1UL-ll)>>1);
            while (ll<lr) {
                if (pk[lk]>=p) { if (0UL<lk) lr=lk-1UL; else lr=lk; }
                else ll=lk;
                lk=ll+((lr+1UL-ll)>>1);
                if (lk==beyondk) break;
                A(lk>=firstk);A(rk<=beyondk);
            }
            if ((lk<beyondk)&&(pk[lk]<p)) lk++;
            A(lk>=firstk);A(rk<=beyondk);
            while (rl<rr) {
                if (pk[rk]<q) { if (rk<beyondk) rl=rk+1UL; else rl=rk; }
                else rr=rk;
                rk=rl+((rr-rl)>>1);
                if (rk==beyondk) break;
                A(lk>=firstk);A(rk<=beyondk);
            }
            A((lk==beyondk)||(pk[lk]>=p));A((rk==beyondk)||(pk[rk]>=q));
        }
        A(lk>=firstk);A(rk<=beyondk);A(lk<=rk);

        if (s<t) { /* > region is larger */
            /* recurse on small region, if effective size > 1 */
            if ((1UL<s)&&((NULL==pk)||(lk>firstk)))
                quickselect_loop(base,first,p,size,compar,pk,firstk,lk,swapfunc
#if SAVE_PARTIAL
                ,ppeq,ppgt
#endif
            );
            /* large region [q,beyond) size t, order statistics [rk,beyondk) */
            first=q, r=t, firstk=rk;
        } else { /* < region is larger, or regions are same size */
            /* recurse on small region, if effective size > 1 */
            if ((1UL<t)&&((NULL==pk)||(beyondk>rk)))
               quickselect_loop(base,q,beyond,size,compar,pk,rk,beyondk,swapfunc
#if SAVE_PARTIAL
                ,ppeq,ppgt
#endif
            );
            /* large region [first,p) size s, order statistics [firstk,lk) */
            beyond=p, r=s, beyondk=lk;
        }
        /* iterate on large region (size r), if effective size > 1 */
        if ((2UL>r)||((NULL!=pk)&&(firstk>=beyondk))) return;
        /* should large region be repivoted? */
        c+=should_repivot(nmemb,r);
        if (lopsided_partition_limit<=c) { /* wait for enough poor partitions */
            char *pc, *pd, *pe, *pf;

            /* yes: do it and check resulting sizes */
            pivot=repivot(base,first,beyond,size,compar,swapfunc,&pc,&pd,
                      &pe,&pf);
            if (NULL==pk) nrepivot++;
#if SAVE_PARTIAL
            repartition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,
                swapfunc,&p,&q);
#else
            partition(base,first,beyond,pivot,size,compar,swapfunc,&p,&q);
#endif
            if (NULL==pk) npartitions++;
            c=0; /* clean slate */
            goto check_sizes;
        }
        /* no repivot: just process large region as usual */
        nmemb=r;
    }
    isort_internal(base,first,beyond,size,compar,swapfunc);
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
    if (2UL>nmemb) return; /* Return early if there's nothing to do. */

    if (0UL==nk) pk=NULL;
    else if (NULL==pk) nk=0UL;
    else if (1UL<nk) quickselect_internal((void *)pk,nk,sizeof(size_t),
        size_t_cmp,NULL,0UL
#if SAVE_PARTIAL
            ,NULL,NULL
#endif
        );
    quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,
        whichswap(base,size,1U)
#if SAVE_PARTIAL
        ,ppeq,ppgt
#endif
    );
}


/* dual-pivot sorting */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void sort5(char *pa, char *pb, char *pc, char *pd, char *pe, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapfunc)(char *, char *, char *, size_t))
{
    /* Batcher sorting network: 9 comparisons, 0-9 exchanges */
    if (0>compar(pa,pe)) swapfunc(pa,pe,NULL,size);
    if (0>compar(pa,pc)) swapfunc(pa,pc,NULL,size);
    if (0>compar(pb,pd)) swapfunc(pb,pd,NULL,size);
    if (0>compar(pa,pb)) swapfunc(pa,pb,NULL,size);
    if (0>compar(pc,pe)) swapfunc(pc,pe,NULL,size);
    if (0>compar(pc,pd)) swapfunc(pc,pd,NULL,size);
    if (0>compar(pb,pe)) swapfunc(pb,pe,NULL,size);
    if (0>compar(pb,pc)) swapfunc(pb,pc,NULL,size);
    if (0>compar(pd,pe)) swapfunc(pd,pe,NULL,size);
}

static size_t dp_cutoff = 26UL; /* 26 seems to be optimal */

/* dual-pivot qsort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void dpqsort_internal(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *),
    void (*swapfunc)(char *, char *, char *, size_t))
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
    while (1) {
        pu=base+(nmemb-1UL)*size; /* last element */
        if (nmemb<=dp_cutoff) { /* insertion sort */
            isort_internal(base,0UL,nmemb,size,compar,swapfunc);
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
            u=q*t;
            v=q*size;
            if (pm-u>=base) swapfunc(pm-u,pm-v,NULL,size);
            if (pm+u<pu) swapfunc(pm+u,pm+v,NULL,size);
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
        sort5(pa,pivot1,pm,pivot2,ps,size,compar,swapfunc);
#endif
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
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
                swapfunc(pb,pm,NULL,size);
                if (pivot1==pm) pivot1=pb; else if (pivot2==pm) pivot2=pb;
                else if (pivot1==pb) pivot1=pm; else if (pivot2==pb) pivot2=pm;
                pb+=size;
            } else if (0>=compar(pivot2,pm)) {
                do ps-=size; while ((pm<ps)&&(0>=compar(pivot2,ps)));
                if (pm>=ps) break;
                swapfunc(pm,ps,NULL,size);
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
fprintf(stderr, "//%s line %d: pb=%p[%lu](%d), ps=%p[%lu](%d)\n",__func__,__LINE__,pb,(pb-base)/size,*((int *)pb),ps,(ps-base)/size,*((int *)ps));
print_int_array(base,0UL,nmemb-1UL);
#endif
        for (pa=pb,pm=base; pm<pa; pm+=size) {
            if (0==compar(pivot1,pm)) {
                pa-=size;
                swapfunc(pm,pa,NULL,size);
                if (pivot1==pm) pivot1=pa; else if (pivot2==pm) pivot2=pa;
            }
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pb,(pb-base)/size,*((int *)pb));
print_int_array(base,0UL,nmemb-1UL);
#endif
        for (pt=ps,pm=pu-size; pm>=pt; pm-=size) {
            if (0==compar(pivot2,pm)) {
                swapfunc(pm,pt,NULL,size);
                if (pivot1==pm) pivot1=pt; else if (pivot2==pm) pivot2=pt;
                pt+=size;
            }
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pb,(pb-base)/size,*((int *)pb));
fprintf(stderr, "//%s line %d: ps=%p[%lu](%d), pt=%p[%lu](%d)\n",__func__,__LINE__,ps,(ps-base)/size,*((int *)ps),pt,(pt-base)/size,*((int *)pt));
print_int_array(base,0UL,nmemb-1UL);
#endif
#else
        for (pa=pm=base,pt=pu; pm<pt;) {
            A(base<=pm);A(base<=pa);A(pa<=pm);A(pt<=pu);
            A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0<compar(pivot1,pm)) {
                swapfunc(pa,pm,NULL,size);
                if (pivot1==pa) pivot1=pm; else if (pivot2==pa) pivot2=pm;
                else if (pivot2==pm) pivot2=pa;
                pa+=size;
            } else if (0>compar(pivot2,pm)) {
                do pt-=size; while ((pm<pt)&&(0>compar(pivot2,pt)));
                if (pm>=pt) break;
                swapfunc(pm,pt,NULL,size);
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
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pt=%p[%lu](%d)\n",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pt,(pt-base)/size,*((int *)pt));
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
                swapfunc(pb,pm,NULL,size);
                if (pivot1==pm) pivot1=pb; else if (pivot1==pb) pivot1=pm;
                if (pivot2==pm) pivot2=pb; else if (pivot2==pb) pivot2=pm;
                pb+=size;
            } else if (0==compar(pivot2,pm)) {
                do ps-=size; while ((pm<ps)&&(0==compar(pivot2,ps)));
                if (pm>=ps) break;
                swapfunc(pm,ps,NULL,size);
                if (pivot1==pm) pivot1=ps; else if (pivot1==ps) pivot1=pm;
                if (pivot2==pm) pivot2=ps; else if (pivot2==ps) pivot2=pm;
                continue; /* test moved element at pm */
            }
            pm+=size;
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: base=%p, pivot1=%p[%lu](%d), pivot2=%p[%lu](%d), pu=%p\n",__func__,__LINE__,base,pivot1,(pivot1-base)/size,*((int *)pivot1),pivot2,(pivot2-base)/size,*((int *)pivot2),pu);
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,pa,(pa-base)/size,*((int *)pa),pb,(pb-base)/size,*((int *)pb));
fprintf(stderr, "//%s line %d: ps=%p[%lu](%d), pt=%p[%lu](%d)\n",__func__,__LINE__,ps,(ps-base)/size,*((int *)ps),pt,(pt-base)/size,*((int *)pt));
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
        if (r>1UL) dpqsort_internal(pb,r,size,compar,swapfunc);
        if (s>1UL) dpqsort_internal(pt,s,size,compar,swapfunc);
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
    dpqsort_internal(base, nmemb, size, compar, whichswap(base,size,1U));
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
    void (*swapfunc)(char *, char *, char *, size_t))
{
    int pcmp;
    char *pg, *pivot1, *pivot2, *pk, *pl, *pr;
    size_t dist, t;

#define YQSORT_DEBUG 0
#if YQSORT_DEBUG
fprintf(stderr,"%s(%p,%lu,%lu,%p,%lu)\n",__func__,base,nmemb,size,compar,y_div);
#endif
    pr=base+(nmemb-1UL)*size;
    if (nmemb<y_cutoff) { /* insertion sort */
        isort_internal(base,0UL,nmemb,size,compar,swapfunc);
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
        swapfunc(base,pivot2,NULL,size);
        swapfunc(pivot1,pr,NULL,size);
    } else {
        swapfunc(base,pivot1,NULL,size);
        swapfunc(pivot2,pr,NULL,size);
    }
    /* pivot elements (moved) */
    pivot1=base, pivot2=pr;
    /* pointers */
    pl=base+size, pg=pr-size;
    /* sorting */
    for (pk=pl; pk<=pg; pk+=size) {
        if (0>compar(pk,pivot1)) swapfunc(pk,pl,NULL,size), pl+=size;
        else if (0<compar(pk,pivot2)) {
            while ((pk<pg)&&(0<compar(pg,pivot2))) pg-=size;
            swapfunc(pk,pg,NULL,size), pg-=size;
            if (0>compar(pk,pivot1)) swapfunc(pk,pl,NULL,size), pl+=size;
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
    swapfunc(pivot1,base,NULL,size);
    pivot2=pg+size; /* new location for pivot2 */
    swapfunc(pivot2,pr,NULL,size);
    /* subarrays */
    t=(pivot1-base)/size; /* save size for protection comparison and recursion */
    A((2UL>t)||(base+(t-1UL)*size<=pr));
    if (1UL<t) yqsort_internal(base,t,size,compar,y_div,swapfunc);
    t=(pr-pivot2)/size; /* save size for protection comparison and recursion */
    A((2UL>t)||(pivot2+t*size<=pr));
    if (1UL<t) yqsort_internal(pivot2+size,t,size,compar,y_div,swapfunc);
    /* equal elements */
    /* cheap (cached) condition first */
    if ((0!=pcmp)
#if 0 /* always separate equals if pivots differ */
    /* magic number 13 replaced by y_cutoff2 */
    &&(y_cutoff2>nmemb-dist) /* avoid overflow/underflow */
#endif
    ) { /* avoid overflow/underflow */
        for (pk=pl; pk<=pg; pk+=size) {
            if (0==compar(pk,pivot1)) swapfunc(pk,pl,NULL,size), pl+=size;
            else if (0==compar(pk,pivot2)) {
                swapfunc(pk,pg,NULL,size), pg-=size;
                if (0==compar(pk,pivot1)) swapfunc(pk,pl,NULL,size), pl+=size;
            }
        }
        /* revised "dist" */
        if (pg>pl) dist=(pg-pl)/size; else dist=0UL; /* avoid underflow */
    }
    /* subarray */
    A(pl+dist*size<=pr);
    /* avoid unnecessary work for all-equal region; cached condition first */
    if ((0!=pcmp)&&(0UL<dist)) yqsort_internal(pl,dist+1UL,size,compar,y_div,swapfunc);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void yqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *))
{
    if (4UL>y_cutoff) y_cutoff=4UL; /* require po<pivot1<pivot2<pr */
    yqsort_internal(base, nmemb, size, compar, 3UL,whichswap(base,size,1U));
}
/* ************************************************************** */

/* introsort, loosely based on "Introspective Sorting and Selection Algorithms"
   by David R. Musser. Also insertion sort and heapsort implementations.
*/
/* generalized to take lower(root) and upper indices of heap array */
/* for debugging: not performance-optimized */
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
    void (*swapfunc)(char *, char *, char *, size_t))
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
        swapfunc(base+size*p, base+size*c, NULL, size);
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
    void (*swapfunc)(char *, char *, char *, size_t))
{
    size_t p;

    for (p=parent(r,u); 1; p--) {
        (V)heap_siftdown(base, r, p, u, size, compar, swapfunc);
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
    void (*swapfunc)(char *, char *, char *, size_t))
{
    if (r != n) { swapfunc(base+size*r, base+size*n, NULL, size); /* swap root and last */ }
    --n;
    (V)heap_siftdown(base, r, r, n, size, compar, swapfunc);
    return n;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void heap_sort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapfunc)(char *, char *, char *, size_t))
{
    size_t n;

    heapify_array(base, first, beyond-1UL, size, compar, swapfunc);
    for (n=beyond-1UL; first < n;)
        n = heap_delete_root(base, first, n, size, compar, swapfunc);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void heap_sort(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *))
{
    heap_sort_internal(base, 0UL, nmemb, size, compar, whichswap(base,size,1U));
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void introsort_loop(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapfunc)(char *, char *, char *, size_t),
    size_t depth_limit)
{
    size_t eq, gt;
    char *pivot;

    while (beyond-first>introsort_insertion_cutoff) {
        if (0UL==depth_limit) {
            heap_sort_internal(base,first,beyond,size,compar,swapfunc);
            return;
        }
        depth_limit--;
#if 0
        /* Introsort can be improved by improving the quality and
           quality of samples and pivot selection methods.  The following is
           a simple median-of-3 of the first, middle, and last elements, which
           causes issues with some input sequences (median-of-3 killer,
           organ-pipe, etc.). It does however use ternary median-of-3.
        */
        pivot = fmed3(base+size*first,base+size*(first+(beyond-first)>>1),base+size*(beyond-1UL),compar);
#else
        /* adaptive sampling for pivot selection as in quickselect */
        pivot = select_pivot(base, first, beyond, size, compar, NULL, swapfunc);
#endif
        /* partition is modified Bentley&McIlroy split-end partition returning
           indices of equal-to and greater-than region starts.  This efficient
           partition function also avoids further processing of elements
           comparing equal to the pivot.
        */
        partition(base,first,beyond,pivot,size,compar,swapfunc,&eq,&gt);
        npartitions++;
        introsort_loop(base,gt,beyond,size,compar,swapfunc,depth_limit);
        beyond=eq;
    }
    if (0U==introsort_final_insertion_sort)
        isort_internal(base,first,beyond,size,compar,swapfunc);
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
void introsort(char *base, size_t nmemb, size_t size, int(*compar)(const void *,const void *))
{
    size_t depth_limit;
    void (*swapfunc)(char *,char *,char *, size_t);

    /* introsort insertion sort cutoff */
    if (0UL==introsort_insertion_cutoff) {
        if (0U==introsort_final_insertion_sort) introsort_insertion_cutoff=16UL;
        else introsort_insertion_cutoff=7UL;
    }
    swapfunc=whichswap(base,size,1U);
    depth_limit=introsort_recursion_factor*floor_lg(nmemb);
    introsort_loop(base, 0UL, nmemb, size, compar, swapfunc, depth_limit);
    if (0U!=introsort_final_insertion_sort)
        isort_internal(base,0UL,nmemb,size,compar,swapfunc);
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
        case TEST_SEQUENCE_ORGAN_PIPE :
        return "organ pipe sequence";
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
        return "inverted organ pipe sequence";
        case TEST_SEQUENCE_SAWTOOTH :
        return "sawtooth sequence";
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
static void reverse(long *p, size_t l, size_t r)
{
    int i;

    for (;l<r; l++,r--) {
        i=p[l]; p[l]=p[r]; p[r]=i;
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
static void random_fill(long *p, size_t count, uint64_t modulus)
{
    size_t j;

    for (j=0UL; j<count; j++)
        *p++ = (int)random64n(modulus);
}

/* add random values in [0,modulus) to values in long array p */
static void random_add(long *p, size_t count, uint64_t modulus)
{
    size_t j;

    for (j=0UL; j<count; j++)
        *p++ = (int)random64n(modulus);
}

static int generate_long_test_arrays(long *p, size_t count, size_t n, unsigned int testnum, uint64_t max_val, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret = -1;

    if (NULL != p) {
        size_t h, j, k, o, r, u, x, y, z;
        size_t permutation_limit;
        long i;

        u = n - 1UL;
        for (o=0UL,r=0UL; r<count; r++,o+=n) {
            switch (testnum) {
                case TEST_SEQUENCE_STDIN :
                    for(i=0L,j=o; j<=o+u; i++,j++)
                        p[j] = input_data[i].val;
                break;
                case TEST_SEQUENCE_SORTED :
                    increment(&p[o],0UL,u,0);
                break;
                case TEST_SEQUENCE_REVERSE :
                    increment(&p[o],0UL,n,0);
                    reverse(&p[o],0UL,u);
                break;
                case TEST_SEQUENCE_ORGAN_PIPE :
                    increment(&p[o],0UL,n>>1,0);
                    increment(&p[o],(n+1UL)>>1,u,0);
                    reverse(&p[o],(n+1UL)>>1,u);
                break;
                case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
                    increment(&p[o],n>>1,u,0);
                    increment(&p[o],0UL,(n-1UL)>>1,0);
                    reverse(&p[o],0UL,(n-1UL)>>1);
                break;
                case TEST_SEQUENCE_SAWTOOTH :
                    increment(&p[o],0UL,u,0);
                    k = sawtooth_modulus(j,NULL,log_arg);
                    modulo(&p[o],0UL,n,k);
                break;
                case TEST_SEQUENCE_BINARY :
                    random_fill(&p[o],n,2);
                break;
                case TEST_SEQUENCE_CONSTANT :
                    constant_fill(&p[o],0UL,u,CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MEDIAN3KILLER :
                    /* start with increasing sorted sequence */
                    increment(&p[o],0UL,u,0);
                    k = n;
                    /* median-of-3-killer sequence */
                    if (1UL < k) {
                        if (0 == is_even(k))
                            k--;
                        /* k is the greatest even number <= n */
                        for(h = n-2UL; 1; h -= 2UL) {
                            j = o + ((k - h) >> 1); /* >> 1 is a fast version of / 2 */
                            exchange(p, o+h+1UL, j, 1UL, sizeof(long));
                            if (2UL > h)  /* don't allow h to iterate below 2 */
                                break;
                        }
                    }
                break;
                case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                    /* start with increasing sorted sequence */
                    increment(&p[o],0UL,u,0);
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
                        exchange(p, o+x, o+k, 1UL, sizeof(long));
                        exchange(p, o+y, o+z, 1UL, sizeof(long));
                        if (0UL==x) break;
                    }
                break;
                case TEST_SEQUENCE_RANDOM_DISTINCT :
                    increment(&p[o],0UL,u,0);
                    (V)fisher_yates_shuffle(&(p[o]), n, sizeof(long), random64n, f, log_arg);
                break;
                case TEST_SEQUENCE_RANDOM_VALUES :
                    random_fill(&p[o],n,max_val);
                break;
                case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                    random_fill(&p[o],n,n);
                break;
                case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
                    h=snlround(sqrt((double)n),NULL,NULL);
                    random_fill(&p[o],n,h);
                break;
                case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
                    constant_fill(&p[o],0UL,u,0L);
                    /* Ref: R.W.Hamming, "Numerical Methods for Scientists and Engineers",
                       2nd Ed., Sect. 8.6, ISBN 978-0-486-65241-2
                    */
                    for (h=0UL,k=max_val/12UL; h<12UL; h++)
                        random_add(&p[o],n,k);
                break;
                case TEST_SEQUENCE_HISTOGRAM :
                    constant_fill(&p[o],0UL,u,0L);
                    if (max_val / n < n) x=n*n; else x=max_val/n;
                    if (10000000UL<x) x=10000000UL; /* time limit */
                    z=12UL*u+1UL;
                    for (j=0UL; j<x; j++) {
                        for (i=0L,h=0UL; h<12UL; h++) {
                            i += (long)random64n(n);
                        }
                        p[o+(n*i)/z]++;
                    }
                break;
                case TEST_SEQUENCE_MANY_EQUAL_LEFT :
                    random_fill(&p[o],n,n);
                    constant_fill(&p[o],0UL,n>>2,CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
                    random_fill(&p[o],n,n);
                    constant_fill(&p[o],(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
                    random_fill(&p[o],n,n);
                    constant_fill(&p[o],(n>>1)+(n>>2),u,CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                    random_fill(&p[o],n,n);
                    constant_fill(&p[o],(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
                    (V)fisher_yates_shuffle(&(p[o]), n, sizeof(long), random64n, f, log_arg);
                break;
                case TEST_SEQUENCE_PERMUTATIONS :
/* XXX shouldn't get here if there's no room */
#if SIZE_T_MAX > 2400000000000000000UL
                  permutation_limit = 19UL;
#else
# if SIZE_T_MAX > 5748019200UL
                  permutation_limit = 12UL;
# else
                  permutation_limit = 8UL;
# endif
#endif
                    if (n > permutation_limit) { /* n! would overflow */
                        (V)fprintf(stderr, "permutation test limit is %lu < %lu\n", permutation_limit, n);
                        r = count;     /* break out of loop */
                        ret = 2;
                    } else /* n <= 12 or 19 */ {
                        size_t b=n*sizeof(long), d[20], s, t;

                        if (0UL == r) { /* check size; reinitialize count; initialize d, and first set */
                            s = factorial(n);
                            if (s > count) { /* array is too small to hold all permutations */
                                (V)fprintf(stderr, "%lu! = %lu > %lu\n", n, s, count);
                                r = count;     /* break out of loop */
                                ret = 3;
                                break;
                            }
                            ret = generate_long_test_arrays(p, 1UL, n, TEST_SEQUENCE_SORTED, max_val, f, log_arg);
                            if (0 != ret) {
                                r = count;     /* break out of loop */
                                ret = 4;
                                break;
                            } else {
                                count = s;
                                for (t=0UL; t<n; t++) { d[t] = 0UL; }
                                continue;
                            }
                        }
                        /* copy previous array */
                        memcpy(&(p[o]), &(p[o-n]), b);
                        /* permute copy of array by swapping one pair of elements */
                        /* Based on Algorithm 4a (Alternate Ives) in Sedgewick "Permutation Generation Methods" */
                        for (t=0UL; t < n; ) {
                            if (d[t] < u - t) {
                                if (0 == is_even(t)) { /* t is odd */
                                    exchange(p, o+d[t], o+d[t]+1UL, 1UL, sizeof(long));
                                } else { /* t is even */
                                    exchange(p, o+d[t], o+u-t, 1UL, sizeof(long));
                                }
                                d[t]++;
                                t = 0UL;
                                break;
                            } else {
                                d[t] = 0UL;
                                t++;
                            }
                        }
                    }
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    if (n > 8 * sizeof(unsigned long)) {
/* XXX shouldn't get here if there's no room */
                        (V)fprintf(stderr, "Thorough test limit is %lu < %lu\n", 8 * sizeof(unsigned long), n);
                        r = count;     /* break out of loop */
                        ret = 5;
                    } else {
                        size_t b, s;

                        if (0UL == r) { /* check size; reinitialize count */
                            s = 0x01UL << n;
                            if (s > count) { /* array is too small to hold all combinations */
                                (V)fprintf(stderr, "2^%lu = %lu > %lu\n", n, s, count);
                                r = count;     /* break out of loop */
                                ret = 6;
                                break;
                            }
                            count = s;
                        }
                        /* combinations of 0, 1 elements for this test */
                        for (b=o; b<o+n; b++) {
                            p[b] = (long)((r >> b) & 0x01UL);
                        }
                    }
                break;
                case TEST_SEQUENCE_ADVERSARY :
                    /* handled by initialize_antiqsort */
                break;
                default:
                    (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
                    r = count;     /* break out of loop */
                    ret = 7;
                break;
            }
        }
        if (0 > ret) ret = 0;
    }
    return ret;
}

/* macros to unify function calls */
#define BMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) bmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define MBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) mbmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define GLQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) glibc_quicksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define NBQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) nbqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define WQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) wqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#if SAVE_PARTIAL
#define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_internal((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,NULL,NULL)
#else
#define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_internal((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl)
#endif
#define YQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) yqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define DPQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) dpqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define HEAPSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) heap_sort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define INTROSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) introsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define ISORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) isort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)

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
            (V)strlcat(buf, "median", sz);
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
            if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests))  ret = "sort"; else ret = "median";
        break;
        case FUNCTION_WQSORT :        /*FALLTHROUGH*/
        case FUNCTION_SQSORT :        /*FALLTHROUGH*/
        case FUNCTION_BMQSORT :       /*FALLTHROUGH*/
        case FUNCTION_MBMQSORT :      /*FALLTHROUGH*/
        case FUNCTION_NBQSORT :       /*FALLTHROUGH*/
        case FUNCTION_GLQSORT :       /*FALLTHROUGH*/
        case FUNCTION_DPQSORT :       /*FALLTHROUGH*/
        case FUNCTION_YQSORT :        /*FALLTHROUGH*/
        case FUNCTION_HEAPSORT :      /*FALLTHROUGH*/
        case FUNCTION_INTROSORT :     /*FALLTHROUGH*/
        case FUNCTION_ISORT :         /*FALLTHROUGH*/
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
        case TEST_SEQUENCE_REVERSE :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_BINARY :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_COMBINATIONS :             /*FALLTHROUGH*/
        case TEST_SEQUENCE_PERMUTATIONS :             /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_DISTINCT :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_CONSTANT :                 /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :    /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED : /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES :
        break;
        /* placeholders: type-specific known (not unrecognized) tests */
        case TEST_SEQUENCE_ADVERSARY :
        break;
        default:
            (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n",
                __func__, source_file, __LINE__, testnum);
        return 0;
    }
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
    return 1;
}

/* duplicate (as far as possible) long data in larray to other arrays */
static void duplicate_test_data(long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, size_t o, size_t n)
{
    long l;
    int i;
    size_t j;

    for (j=o; j<n; j++) {
        l = larray[j];
        big_array[j].l = l;
        darray[j]=big_array[j].d = (double)l;
        (V)snl(big_array[j].string, BIG_STRUCT_STRING_SIZE, NULL, NULL, l, 36,
            '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
        i = l % INT_MAX;
        big_array[j].val = array[j] = i;
        parray[j] = &(big_array[j]);
    }
}

static void restore_test_data(int type, size_t o, size_t n, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray)
{
    if (DATA_TYPE_LONG==type) {
        for (; o<n; o++)
            larray[o] = big_array[o].l;
    } else
        duplicate_test_data(larray, array, darray, big_array, parray, o, n);
}

static unsigned int correctness_test(int type, size_t size, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, int (*comparison_function)(const void *, const void *), const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    char buf[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="";
    const char *comment="";
    int c, emin, emax, ik;
    unsigned int distinct=0U, errs=0U, rpt=flags['d'];
    size_t eql, equ, k, karray[2], m, t, u;
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
    double d;
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
#if 0
/* XXX valid_test here should be redundant; it was checked in caller */
        if (0==valid_test(TEST_TYPE_CORRECTNESS, testnum, n)) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: test %s is not valid at size %lu",
                prog, __func__, source_file, __LINE__, ptest, n);
            return ++errs;
        }
#endif
        u = n - 1UL;
        k = u>>1;
        /* generated test sequences */
        emin = emax = (int)k;
        if (0 != is_even(n)) {
            if (0 < emin) emin--;
            emax++;
        }
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_SORTED :              /*FALLTHROUGH*/
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
            case TEST_SEQUENCE_COMBINATIONS :
                emin = 0; emax = n;                  /*FALLTHROUGH*/
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_DISTINCT :
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
                emin = EXPECTED_RANDOM_MIN; emax = EXPECTED_RANDOM_MAX;
            break;
            case TEST_SEQUENCE_ADVERSARY :
            return ++errs; /* no correctness test */
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
        if (0U == errs) {
            /* special count numbers, etc. */
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    count = factorial(n);
                    /* generate all permutations */
                    c = generate_long_test_arrays(larray,
                        count, n, testnum, max_val, f, log_arg);
                    if (0 > c) {
                        (V)fprintf(stderr,
                            "%s: %s line %d: generate_long_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    duplicate_test_data(larray,array,darray,big_array,parray, 0UL,n);
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    count = 0x01UL << n;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                    /* generate all combinations */
                    c = generate_long_test_arrays(larray,
                        count, n, testnum, max_val, f, log_arg);
                    if (0 > c) {
                        (V)fprintf(stderr,
                            "%s: %s line %d: generate_long_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    duplicate_test_data(larray,array,darray,big_array,parray, 0UL,n);
                break;
            }
            /* run tests */
            if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
            for (m=o=0UL; m<count; m++) {
                switch (testnum) {
                    case TEST_SEQUENCE_PERMUTATIONS :
                    /*FALLTHROUGH*/
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* nothing to do here */
                    break;
                    default :
                        /* generate new  test sequence */
                        c=generate_long_test_arrays(larray,
                            1UL, n, testnum, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        duplicate_test_data(larray,array,darray,big_array,parray, 0UL,
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
                    case TEST_SEQUENCE_REVERSE :                 /*FALLTHROUGH*/
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
                switch (func) {
                    case FUNCTION_QSELECT :
                        if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests)) {
                            QSEL(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                        } else {
                            karray[0] = karray[1] = k; /* 0-based rank */
                            QSEL(pv,o,o+u,size,comparison_function,karray,0UL,2UL,rpt);
                        }
#if DEBUG_CODE
(V)fprintf(stderr, "%s line %d: quickselect returned\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MBMQSORT :
                        MBMQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_WQSORT :
                        WQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQSORT :
                        SQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
                        qsort_wrapper((char *)pv+size*o,u+1UL,size,comparison_function);
                    break;
                    case FUNCTION_YQSORT :
                        YQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_DPQSORT :
                        DPQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_HEAPSORT :
                        HEAPSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_INTROSORT :
                        INTROSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_ISORT :
                        ISORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                }
                if ((1UL==count)&&(0U != flags['i'])) {
                    const char *psize;
                    double factor;

                    t=nlt+neq+ngt;
#if 0
fprintf(stderr, "%s line %d: *ptests=0x%x\n",__func__,__LINE__,*ptests);
#endif
                    factor = test_factor(*ptests, n);
                    psize = test_size(*ptests);
                    d = (double)t / factor;
                    if (0U != flags['K']) comment="#";
                    if (0U!=flags['R']) {
                        c = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                        (V)printf("%s%s%*.*s%.6G %s comparisons\n", comment, buf, col-c, col-c, " ", d, psize);
                    } else {
                        c = 1 + snprintf(buf, sizeof(buf), "%s %s vs. %lu %s comparison counts:", pcc, pfunc, n, ptest);
                        (V)printf("%s%s%*.*s%lu < (%.1f%%), %lu == (%.1f%%), %lu > (%.1f%%), total %lu: %.6G %s\n",
                            comment, buf, col-c, col-c, " ", nlt, 100.0*(double)nlt/(double)t, neq,
                            100.0*(double)neq/(double)t, ngt, 100.0*(double)ngt/(double)t,
                            t, d, psize);
                    }
                    if (0UL<nsw) {
                        c = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                        d = (double)nsw / (double)count / factor;
                        (V)printf("%s%s%*.*s%.6G %s swaps\n", comment, buf, col-c, col-c, " ", d, psize);
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
                switch (testnum) {
                    case TEST_SEQUENCE_PERMUTATIONS :
                    /*FALLTHROUGH*/
                    case TEST_SEQUENCE_COMBINATIONS :
                        o+=n;
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
                    restore_test_data(type,o,n,larray,array,darray,big_array,parray);
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
                fflush(stdout);
            }
        } /* run test if no errors */
    } /* generate test sequence and run test if no errors */
    return errs;
}

static unsigned int timing_test(int type, size_t size, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, int (*comparison_function)(const void *, const void *), const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count,  unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    char buf[256], buf2[256], buf3[256], buf4[256], buf5[256], buf6[256],
        buf7[256], buf8[256], buf9[256], buf10[256], buf11[256], buf12[256],
        buf13[256], buf14[256], buf15[256], buf16[256], buf17[256], buf18[256],
        buf19[256], buf20[256], buf21[256], buf22[256], buf23[256], buf24[256],
        buf25[256], buf26[256], buf27[256], buf28[256], buf29[256], buf30[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="", *psize;
    const char *comment="";
    int c, emin, emax, i;
    unsigned int errs=0U, rpt=flags['d'];
    size_t k, karray[2], m, marray[10], dn, ocount=count, t, u;
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
    int *pint;
    long l, *plong;
    struct big_struct *pbs;
    auto struct rusage rusage_start, rusage_end;
    auto struct timespec timespec_start, timespec_end, timespec_diff;
    double *sarray, *uarray, *warray;
    double best_s=9.9e99, best_u=9.9e99, best_w=9.9e99, d, factor, test_s,
        tot_s=0.0, test_u, tot_u=0.0, test_w, tot_w=0.0, worst_s=0.0,
        worst_u=0.0, worst_w=0.0;
    void *pv;
    uint64_t max_val;
    void *alt;
    size_t altsz;
    int alt_type;

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
                alt=big_array;
                altsz=sizeof(struct big_struct);
                alt_type=DATA_TYPE_STRUCT;
            break;
            case DATA_TYPE_INT :
                pv=array;
                max_val = INT_MAX;
                alt=larray;
                altsz=sizeof(long);
                alt_type=DATA_TYPE_LONG;
            break;
            case DATA_TYPE_DOUBLE :
                pv=darray;
                max_val = LONG_MAX;
                alt=larray;
                altsz=sizeof(long);
                alt_type=DATA_TYPE_LONG;
            break;
            case DATA_TYPE_POINTER :
                pv=parray;
                max_val = INT_MAX;
                alt=larray;
                altsz=sizeof(long);
                alt_type=DATA_TYPE_LONG;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                pv=big_array;
                max_val = INT_MAX;
                alt=larray;
                altsz=sizeof(long);
                alt_type=DATA_TYPE_LONG;
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized data type %u\n", __func__, source_file, __LINE__, type);
            return ++errs;
        }
        ptest = sequence_name(testnum);
#if 0
/* XXX valid_test here should be redundant; it was checked in timing_tests() */
        if (0==valid_test(TEST_TYPE_TIMING, testnum, n)) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: test %s is not valid at size %lu",
                prog, __func__, source_file, __LINE__, ptest, n);
            return ++errs;
        }
#endif
        u = n - 1UL;
        k = u >>1;
        /* generated test sequences */
        emin = emax = (int)k;
        if (0 != is_even(u)) {
            emin--; emax++;
        }
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_SORTED :
            break;
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
                emin = EXPECTED_RANDOM_MIN; emax = EXPECTED_RANDOM_MAX;
            break;
            case TEST_SEQUENCE_MANY_EQUAL_LEFT :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_RIGHT :    /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                if (0 != is_even(n)) { emin = CONSTANT_VALUE ; } else { emin = CONSTANT_VALUE + 10; }
                emax = CONSTANT_VALUE + 10 + (int)u / 4 + 1;
            break;
            case TEST_SEQUENCE_BINARY :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
                emin = 0; emax = 1;
            /*FALLTHROUGH */
            case TEST_SEQUENCE_RANDOM_DISTINCT :     /*FALLTHROUGH */
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH */
            case TEST_SEQUENCE_ADVERSARY :
            break;
            default:
                (V)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
        if (0U==errs) {
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    count = factorial(n);
                    /* generate all permutations */
                    c = generate_long_test_arrays(larray,
                        count, n, testnum, max_val, f, log_arg);
                    if (0 > c) {
                        (V)fprintf(stderr,
                            "%s: %s line %d: generate_long_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    duplicate_test_data(larray,array,darray,big_array,parray, 0UL,n);
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    count = 0x01UL << n;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                    /* generate all combinations */
                    c = generate_long_test_arrays(larray,
                        count, n, testnum, max_val, f, log_arg);
                    if (0 > c) {
                        (V)fprintf(stderr,
                            "%s: %s line %d: generate_long_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    duplicate_test_data(larray,array,darray,big_array,parray, 0UL,n);
                break;
            }
            /* preparation */
            if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
            sarray=malloc(sizeof(double)*count);
            if (NULL==sarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                return errs;
            }
            uarray=malloc(sizeof(double)*count);
            if (NULL==uarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                free(sarray);
                return errs;
            }
            warray=malloc(sizeof(double)*count);
            if (NULL==warray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
                free(uarray);
                free(sarray);
                return errs;
            }
#if DEBUG_CODE
            if (NULL!=f) f(LOG_INFO, log_arg, "%s %s line %d: %s %s %s", __func__, source_file, __LINE__, pcc, typename, pfunc);
#endif /* DEBUG_CODE */
            /* run tests */
            nrepivot=npartitions=0UL;
            for (m=o=dn=0UL; m<count; m++) {
                test_s = test_u = test_w = 0.0;
                switch (testnum) {
                    case TEST_SEQUENCE_ADVERSARY :
                        /* generate new test sequence */
                        c=generate_long_test_arrays(larray,
                            1UL, n, TEST_SEQUENCE_SORTED, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        j = k, o = 0UL;
                        initialize_antiqsort(n, pv, size, alt, altsz, type, alt_type);
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* test number */
                        if (NULL != fp) {
                            c = snul(buf, sizeof(buf), NULL, NULL, m, 2, '0',
                                (int)n, f, log_arg);
                            (V)fprintf(fp, "%s", buf);
                            fflush(fp);
                        }
                    break;
                    case TEST_SEQUENCE_PERMUTATIONS :
                        /* nothing to do here */
                    break;
                    default :
                        /* generate new test sequence */
                        c=generate_long_test_arrays(larray,
                            1UL, n, testnum, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        duplicate_test_data(larray,array,darray,big_array,parray, 0UL,
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
                        } else {
                            karray[0] = karray[1] = k; /* 0-based rank */
                            QSEL(pv,o,o+u,size,comparison_function,karray,0UL,2UL,rpt);
                        }
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MBMQSORT :
                        MBMQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_WQSORT :
                        WQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQSORT :
                        SQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
                        qsort_wrapper((char *)pv+size*o,u+1UL,size,comparison_function);
                    break;
                    case FUNCTION_YQSORT :
                        YQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_DPQSORT :
                        DPQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_HEAPSORT :
                        HEAPSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_INTROSORT :
                        INTROSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_ISORT :
                        ISORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
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
                if (1UL<count) {
                    uarray[dn] = test_u;
                    sarray[dn] = test_s;
                    warray[dn] = test_w;
                    dn++;
                }
                if (0U < errs) break;
                if ((tot_w > timeout)&&(m+1<count)) count = ++m; /* horrible performance; break out of loop */
                switch (testnum) {
                    case TEST_SEQUENCE_COMBINATIONS :
                    /*FALLTHROUGH*/
                    case TEST_SEQUENCE_PERMUTATIONS :
                        o+=n;
                    break;
                }
            } /* execute test count times */
            /* cleanup */
            switch (testnum) {
                case TEST_SEQUENCE_COMBINATIONS :
                    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); fp=NULL; }
                /*FALLTHROUGH*/
                case TEST_SEQUENCE_PERMUTATIONS :
                    ocount=count; /* avoid "terminated early" warning */
                break;
            }
            if (0U != flags['K']) comment="#";
            if ((0U == errs)&&(0U==flags['O'])) {
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
(V)fprintf(stderr, "%s line %d: tot_u=%G, factor=%G, count=%lu, mean=%G (%s)\n", __func__, __LINE__, tot_u, factor, count, test_u, buf);
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
                /* find order statistics for times */
                if (0UL<dn) {
                    marray[0] = 0UL;              /* min */
                    marray[1] = dn/50UL;          /* 2%ile */
                    marray[2] = dn/11UL;          /* 9%ile */
                    marray[3] = dn>>2;            /* 1st quartile */
                    marray[4] = (dn-1UL)>>1;      /* lower median */
                    marray[5] = dn>>1;            /* upper median */
                    marray[6] = dn-1UL-(dn>>2);   /* 3rd quartile */
                    marray[7] = dn-1UL-(dn/11UL); /* 91%ile */
                    marray[8] = dn-1UL-(dn/50UL); /* 98%ile */
                    marray[9] = dn-1UL;           /* max */
#if DEBUG_CODE
(V)fprintf(stderr, "%s line %d: dn=%lu, n=%lu, u=%lu, dn-n-1UL=%lu, marray[4]=%lu, marray[5]=%lu\n", __func__, __LINE__, dn, n, u, dn-n-1UL, marray[4], marray[5]);
#endif /* DEBUG_CODE */
ustats:             quickselect((void *)(uarray), dn, sizeof(double), doublecmp,
                        marray, 10UL);
#if DEBUG_CODE
(V)fprintf(stderr, "%s line %d: quickselect returned\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                    test_u = (uarray[marray[4]] + uarray[marray[5]]) / 2.0; /* median user */
                    (V)sng(buf5, sizeof(buf5), NULL, NULL, test_u, -4, 3, f, log_arg);
                    (V)sng(buf17, sizeof(buf17), NULL, NULL, uarray[marray[0]], -4, 3, f, log_arg); /* min user */
#define REMOVE_OUTLIERS 0
#define OUTLIER_THRESHOLD 1.2
#define DISCREPANCY_THRESHOLD 1.05
                    (V)sng(buf18, sizeof(buf18), NULL, NULL, uarray[marray[9]], -4, 3, f, log_arg); /* max user */
#if REMOVE_OUTLIERS
                    /* is there a mean-median discrepancy? */
                    d = tot_u / test_u / (double)m; /* ratio of mean to median */
                    if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                    if (d >= DISCREPANCY_THRESHOLD) {
                        /* is the maximum an outlier? (possibly) */
                        d = uarray[marray[9]] / test_u; /* ratio of max to median */
                        if (d >= OUTLIER_THRESHOLD) {
                            (V)fprintf(stderr,"%s%s %s User time maximum outlier: min %s, mean %s, max %s, median %s: max-median discrepancy %0.2f%%\n",comment,pcc,ptest,buf17,buf6,buf18,buf5,100.0*(d-1.0));
                            /* adjust mean */
                            tot_u -= uarray[marray[9]]; m--;
                            test_u = tot_u / (double)m; /* new mean */
                            (V)sng(buf6, sizeof(buf6), NULL, NULL, test_u, -6, 3, f, log_arg);
                            (V)sng(buf, sizeof(buf), NULL, NULL, test_u / factor, -6, 3, f, log_arg);
                            /* recompute order statistics */
                            dn--;
                            marray[4] = (dn-1UL)>>1; /* lower median */
                            marray[5] = dn>>1;       /* upper median */
                            marray[9]--;
                            goto ustats;
                        }
                    }
#endif
                    quickselect((void *)(sarray), dn, sizeof(double), doublecmp,
                        marray, 10UL);
                    test_s = (sarray[marray[4]] + sarray[marray[5]]) / 2.0; /* median sys */
                    (V)sng(buf11, sizeof(buf11), NULL, NULL, test_s, -4, 3, f, log_arg);
                    (V)sng(buf19, sizeof(buf19), NULL, NULL, sarray[marray[0]], -4, 3, f, log_arg); /* min sys */
                    (V)sng(buf20, sizeof(buf20), NULL, NULL, sarray[marray[9]], -4, 3, f, log_arg); /* max sys */
wstats:             quickselect((void *)(warray), dn, sizeof(double), doublecmp,
                        marray, 10UL);
                    test_w = (warray[marray[4]] + warray[marray[5]]) / 2.0; /* median wall */
                    (V)sng(buf13, sizeof(buf13), NULL, NULL, test_w, -4, 3, f, log_arg);
                    (V)sng(buf21, sizeof(buf21), NULL, NULL, warray[marray[0]], -4, 3, f, log_arg); /* min wall */
                    (V)sng(buf22, sizeof(buf22), NULL, NULL, warray[marray[9]], -4, 3, f, log_arg); /* max wall */
#if REMOVE_OUTLIERS
                    /* is there a mean-median discrepancy? */
                    d = tot_w / test_w / (double)m; /* ratio of mean to median */
                    if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                    if (d >= DISCREPANCY_THRESHOLD) {
                        /* is the maximum an outlier? (possibly) */
                        d = warray[marray[9]] / test_w; /* ratio of max to median */
                        if (d >= OUTLIER_THRESHOLD) {
                            (V)fprintf(stderr,"%s%s %s Wall-clock time maximum outlier: min %s, mean %s, max %s, median %s: max-median discrepancy %0.2f%%\n",comment,pcc,ptest,buf21,buf10,buf22,buf13,100.0*(d-1.0));
                            /* adjust mean */
                            tot_w -= warray[marray[9]]; m--;
                            test_w = tot_w / (double)m; /* new mean */
                            (V)sng(buf10, sizeof(buf10), NULL, NULL, test_w, -4, 3, f, log_arg);
#if 0
                            (V)fprintf(stderr,"%s%s %s old mean %.G/%lu, revised mean %.G/%lu = %s\n",comment,pcc,ptest,tot_w+warray[marray[9]],m+1UL,tot_w,m,buf10);
#endif
                            (V)sng(buf9, sizeof(buf9), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
                            /* recompute order statistics */
                            dn--;
                            marray[4] = (dn-1UL)>>1; /* lower median */
                            marray[5] = dn>>1;       /* upper median */
                            marray[9]--;
                            goto wstats;
                        }
                    }
#endif
                } else {
                    strncpy(buf5, buf6, sizeof(buf5));
                    strncpy(buf11, buf8, sizeof(buf11));
                    strncpy(buf13, buf10, sizeof(buf13));
                }
                (V)sng(buf2, sizeof(buf2), NULL, NULL, test_u / factor, -4, 3, f, log_arg);
                (V)sng(buf12, sizeof(buf12), NULL, NULL, test_s / factor, -4, 3, f, log_arg);
                (V)sng(buf14, sizeof(buf14), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
#if DEBUG_CODE
(V)fprintf(stderr, "%s line %d: factor=%G, count=%lu, dn=%lu, median=%G (%s)\n", __func__, __LINE__, factor, count, dn, test_u, buf2);
#endif /* DEBUG_CODE */
                if ((0UL<dn)&&(0U!=flags['B'])) {
                    /* wall-clock box plot data: 2%, 9%, 1/4, median, 3/4, 91%, 98% */
                    (V)sng(buf23, sizeof(buf23), NULL, NULL, warray[marray[1]], -4, 3, f, log_arg); /* 2% wall */
                    (V)sng(buf24, sizeof(buf24), NULL, NULL, warray[marray[2]], -4, 3, f, log_arg); /* 9% wall */
                    (V)sng(buf25, sizeof(buf25), NULL, NULL, warray[marray[3]], -4, 3, f, log_arg); /* 1/4 wall */
                    (V)sng(buf26, sizeof(buf26), NULL, NULL, warray[marray[6]], -4, 3, f, log_arg); /* 3/4 wall */
                    (V)sng(buf27, sizeof(buf27), NULL, NULL, warray[marray[7]], -4, 3, f, log_arg); /* 91% wall */
                    (V)sng(buf28, sizeof(buf28), NULL, NULL, warray[marray[8]], -4, 3, f, log_arg); /* 98% wall */
                    (V)printf("%s%s %s %s box plot: %s %s %s %s %s %s %s\n",comment,pcc,pfunc,ptest,buf23,buf24,buf25,buf13,buf26,buf27,buf28);
                }
                if (0U!=flags['R']) {
                    c = 1 + snprintf(buf2, sizeof(buf2), "%lu", n);
                    (V)printf("%s%s%*.*s%s user seconds\n", comment, buf2, col-c, col-c, " ", 0.0!=test_u?buf5:buf6);
                    (V)printf("%s%s%*.*s%s system seconds\n", comment, buf2, col-c, col-c, " ", 0.0!=test_s?buf11:buf8);
                    (V)printf("%s%s%*.*s%s wall-clock seconds\n", comment, buf2, col-c, col-c, " ", 0.0!=test_w?buf13:buf10);
                } else {
                    (V)printf("%s%s%*.*snormalized unit user times: %s (mean), %s (median), per %s, total user time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf, buf2, psize, buf4, (count!=ocount)?" (terminated early)":"", buf6, buf5);
                    if (0UL<dn) {
                        /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                        d = tot_u / test_u / (double)m; /* ratio of mean to median */
                        if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                        if (d > DISCREPANCY_THRESHOLD) (V)fprintf(stderr,"%s%s %s User time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",comment,pcc,ptest,buf17,buf6,buf18,buf5,100.0*(d-1.0));
                    }
                    (V)printf("%s%s%*.*snormalized unit system times: %s (mean), %s (median), per %s, total system time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf7, buf12, psize, buf15, (count!=ocount)?" (terminated early)":"", buf8, buf11);
                    (V)printf("%s%s%*.*snormalized unit wall clock times: %s (mean), %s (median), per %s, total wall clock time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf9, buf14, psize, buf16, (count!=ocount)?" (terminated early)":"", buf10, buf13);
                    if (0UL<dn) {
                        /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                        d = tot_w / test_w / (double)m; /* ratio of mean to median */
                        if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                        if (d > DISCREPANCY_THRESHOLD) (V)fprintf(stderr,"%s%s %s Wall-clock time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",comment,pcc,ptest,buf21,buf10,buf22,buf13,100.0*(d-1.0));
                    }
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
                if (0U != flags['i']) {
                    c = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                    t=nlt+neq+ngt;
                    d = (double)t / (double)count / factor;
                    if (0U!=flags['R']) {
                        (V)printf("%s%s%*.*s%.6G %s comparisons\n", comment, buf, col-c, col-c, " ", d, psize);
                    } else {
                        i = 1 + snprintf(buf3, sizeof(buf3), "%s %s vs. %lu %s comparison counts:", pcc, pfunc, n, ptest);
                        (V)printf("%s%s%*.*s%lu < (%.1f%%), %lu == (%.1f%%), %lu > (%.1f%%), total %lu: %.6G %s\n",
                            comment, buf3, col-i, col-i, " ", nlt, 100.0*(double)nlt/(double)t, neq,
                            100.0*(double)neq/(double)t, ngt, 100.0*(double)ngt/(double)t,
                            t, d, psize);
                    }
                    if (0UL<nsw) {
                        d = (double)nsw / (double)count / factor;
                        (V)printf("%s%s%*.*s%.6G %s swaps\n", comment, buf, col-c, col-c, " ", d, psize);
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
                                switch (antiqsort_type) {
                                    case DATA_TYPE_LONG :
                                        plong = (long *)(antiqsort_base + antiqsort_typsz*j);
                                        l = *plong;
                                    break;
                                    case DATA_TYPE_INT :
                                        pint = (int *)(antiqsort_base + antiqsort_typsz*j);
                                        l = *pint;
                                    break;
                                    case DATA_TYPE_STRUCT :
                                    /*FALLTHROUGH*/
                                    case DATA_TYPE_STRING :
                                        pbs = (struct big_struct *)(antiqsort_base + antiqsort_typsz*j);
                                        l = pbs->l;
                                        /* antiqsort uses only integer data */
                                    break;
                                }
                                (V)printf("%ld\n", l);
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
            } /* test result summary if no errors */
            free(warray);
            free(uarray);
            free(sarray);
        } /* test if no errors */
    } /* generate test sequence and test if no errors */
    return errs;
}

static unsigned int correctness_tests(int type, size_t size, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, const char *prog, unsigned int func, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
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
                if (TEST_TYPE_THOROUGH != (TEST_TYPE_THOROUGH & *ptests)) continue;
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
        errs += correctness_test(type, size, larray, array, darray, big_array, parray, typename, comparison_function, prog, func, testnum, n, count, psequences, ptests, col, f, log_arg, flags);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}

static unsigned int timing_tests(int type, size_t size, long *larray, int *array, double *darray, struct big_struct *big_array, struct big_struct **parray, const char *typename, const char *prog, unsigned int func, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    unsigned int errs=0U, testnum;
    int (*comparison_function)(const void *, const void *);
    const char *fname = function_name(func, n), *ftype = function_type(func, ptests);

    /* generated test sequences */
    for (testnum=0U; TEST_SEQUENCE_COUNT>testnum; testnum++) {
        /* permutations, combinations only for thorough tests */
        if (0==valid_test(TEST_TYPE_TIMING, testnum, n)) continue;
        switch (testnum) {
            case TEST_SEQUENCE_PERMUTATIONS : /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
                if (TEST_TYPE_THOROUGH != (TEST_TYPE_THOROUGH & *ptests)) continue;
            break;
            case TEST_SEQUENCE_ADVERSARY :
                if (TEST_TYPE_ADVERSARY != (TEST_TYPE_ADVERSARY & *ptests)) continue;
                if (0U != flags['i']) { /* comparison/swap counts */
                    comparison_function = icmp;
                } else {
                    comparison_function = cmp;
                }
            break;
        }
        if (0U == ((0x01U << testnum) & *psequences)) continue;
        if (TEST_SEQUENCE_ADVERSARY != testnum) {
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
        }
        if ((NULL!=f)&&(0U==flags['O'])) {
            if (0U!=flags['K']) fprintf(stderr, "// ");
            f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: %lu %lu", prog, fname, ftype, typename, sequence_name(testnum), n, count);
        }
        errs += timing_test(type, size, larray, array, darray, big_array, parray, typename, comparison_function, prog, func, testnum, n, count, psequences, ptests, col, timeout, f, log_arg, flags);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}

static
unsigned int test_function(const char *prog, int *array, struct big_struct *big_array, double *darray, long *larray, struct big_struct **parray, unsigned int func, size_t n, size_t count, unsigned int *pcsequences, unsigned int *ptsequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
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
#endif

#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: %s %s start: array=%p, big_array=%p, darray=%p", prog, fname, ftype,array,big_array,darray);
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
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: %s %s flags 1st pass completed", prog, fname, ftype);
#endif
    /* second pass for tests with data types */
    for (c=0U; 1; c++) {
        /* supported flags */
        /* alt_type is DATA_TYPE_LONG except when pv is DATA_TYPE_LONG */
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
        if (NULL!=f) f(LOG_INFO, log_arg, "%s: %s %s flag %c tests", prog, fname, ftype, c);
#endif
        if (0U == errs) {
            if (TEST_TYPE_CORRECTNESS == (TEST_TYPE_CORRECTNESS & *ptests)) {
                errs +=  correctness_tests(type, size, larray, array, darray, big_array, parray, typename, prog, func, n, count, pcsequences, ptests, col, timeout, f, log_arg, flags);
            }
        }
        if (0U == errs) {
            if (TEST_TYPE_TIMING == (TEST_TYPE_TIMING & *ptests)) {
                errs +=  timing_tests(type, size, larray, array, darray, big_array, parray, typename, prog, func, n, count, ptsequences, ptests, col, timeout, f, log_arg, flags);
            }
        }
    }
#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) {
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
#define NOPS   33
    char buf[256], numbuf[64];
    char testmatrix[OPTYPES][NOPS] = {      /* applicability matrix */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnn", /* _Bool */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* char */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned char */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* short */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned short */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* int */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* unsigned long */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* long long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned long long */
#endif /* C99 */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnny", /* float */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnyyy", /* double */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnny", /* long double */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnn", /* float _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnn", /* double _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnn", /* long double _Complex */
#endif /* C99 */
        "ynnnnnnyyyyyyyyynnnnnnnnnnnnnnnny", /* pointer */
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
        testnum=32U; M(x=y;y=z;z=x)                       \
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
        testnum=32U; M(x=y;y=z;z=x)                   \
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
        testnum=32U; M(x=y;y=z;z=x)
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
    testnum=32U; P(z=x;x=y;y=z)
}

extern const char *quickselect_build_options;

int main(int argc, const char * const *argv)
{
    char buf[4096], buf2[256];  /* RATS: ignore (big enough) */
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
    unsigned char flags[256];
    volatile unsigned long count, incr=1UL, n, mult=1UL, startn=0UL, ul;
    pid_t pid;
    size_t size_limit, sz, q, x, y;
    void (*f)(int, void *, const char *, ...) = logger;
    void *log_arg;
    struct logger_struct ls;
    double timeout = TEST_TIMEOUT;
    int *array;
    struct big_struct *big_array, **parray;
    double *darray;
    long *larray;
    uint64_t s[16];
    regex_t re;
    regmatch_t match[3]; /* line_buf, name, value */
    auto struct rusage rusage_start, rusage_end;
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
                /* this program has no options (that's what a compiler diagnostic for the loop is about) */
#if SILENCE_WHINEY_COMPILERS
                case ':': /* clang! STFU!! */ /* cannot happen here, but clang is unable to figure that out */
                break;
#endif
                case 'a' : /*FALLTHROUGH*/
                case 'A' : /*FALLTHROUGH*/
                case 'b' : /*FALLTHROUGH*/
                case 'B' : /*FALLTHROUGH*/
                case 'e' : /*FALLTHROUGH*/
                case 'E' : /*FALLTHROUGH*/
                case 'f' : /*FALLTHROUGH*/
                case 'F' : /*FALLTHROUGH*/
                case 'g' : /*FALLTHROUGH*/
                case 'h' : /*FALLTHROUGH*/
                case 'H' : /*FALLTHROUGH*/
                case 'i' : /*FALLTHROUGH*/
                case 'I' : /*FALLTHROUGH*/
                case 'j' : /*FALLTHROUGH*/
                case 'K' : /*FALLTHROUGH*/
                case 'l' : /*FALLTHROUGH*/
                case 'L' : /*FALLTHROUGH*/
                case 'n' : /*FALLTHROUGH*/
                case 'N' : /*FALLTHROUGH*/
                case 'o' : /*FALLTHROUGH*/
                case 'O' : /*FALLTHROUGH*/
                case 'p' : /*FALLTHROUGH*/
                case 'P' : /*FALLTHROUGH*/
                case 'R' : /*FALLTHROUGH*/
                case 's' : /*FALLTHROUGH*/
                case 'S' : /*FALLTHROUGH*/
                case 't' : /*FALLTHROUGH*/
                case 'w' : /*FALLTHROUGH*/
                case 'z' : /*FALLTHROUGH*/
                    flags[c] = 1U;
                    if (0U!=flags['K']) comment="#";
                break;
                case 'c' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    for (x=1UL; x<SAMPLING_TABLE_SIZE; x++) {
                        y = strtoul(pcc, &endptr, 10);
                        if (4UL>y) x=0UL; /* in case user gives cutoff for 1 sample */
                        sampling_table[x].min_nmemb=y;
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
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
                            (void)regerror(c, &re, buf, sizeof(buf));
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
                    ls.logmask = LOG_UPTO(LOG_DEBUG) ;
                break;
                case 'D' : /*FALLTHROUGH*/
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
                case 'k' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    col = atoi(pcc);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'm' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    repivot_factor = strtoul(pcc, &endptr, 0);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        lopsided_partition_limit = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
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
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    mbm_m3_cutoff = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        mbm_n_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
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
                    quickselect_insertion_cutoff = strtoul(pcc, &endptr, 0);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        repivot_factor = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        lopsided_partition_limit = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
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
                        introsort_insertion_cutoff = strtoul(++pcc, &endptr, 10);
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
                            (void)regerror(c, &re, buf, sizeof(buf));
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
                    repivot_factor = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        lopsided_partition_limit = (int)strtol(++pcc, &endptr, 10);
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
                    y_cutoff = strtoul(pcc, &endptr, 10);
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
                default:
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: unrecognized option %s",
                        prog, __func__, source_file, __LINE__, argv[optind]);
                    logger(LOG_ERR, log_arg,
                        "%s: usage: %s %s",
                        prog, prog, USAGE_STRING);
                    fprintf(stderr, "test sequences:\n");
                    for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                        fprintf(stderr, "%s %s\n", buf, sequence_name(tests));
                    }
                return 1;
            }
        }
    }
    if (0U==flags['h']) {
        /* command line */
        (V)fprintf(stdout, "%s%s", comment, prog);
        for (i = 1; i < argc; i++) { (V)fprintf(stdout, " %s", argv[i]); }
        (V)fprintf(stdout, ":\n");
        /* now output settings made by options */
        for (n=0U,ul=sizeof(flags); n<ul; n++) {
            if (0U!=flags[n]) {
                switch (n) {
                    case 'c' :
                        for (x=0UL; x<SAMPLING_TABLE_SIZE; x++)
                            (V)fprintf(stdout, "%snmemb>=%lu, %lu sample%s\n", comment, sampling_table[x].min_nmemb, sampling_table[x].samples, sampling_table[x].samples==1?"":"s");
                    break;
                    case 'k' :
                        (V)fprintf(stdout, "%stiming column = %d\n", comment, col);
                    break;
                    case 'm' :
                        (V)fprintf(stdout,
                            "%squickselect repivot_factor = %lu, repivot_cutoff = %lu, lopsided_partition_limit = %d\n",
                            comment, repivot_factor, repivot_cutoff, lopsided_partition_limit);
                    break;
                    case 'M' :
                        (V)fprintf(stdout,
                            "%smodified Bentley&McIlroy qsort median-of-3-cutoff = %lu, ninther_cutoff = %lu\n",
                            comment, mbm_m3_cutoff, mbm_n_cutoff);
                    break;
                    case 'q' :
                        (V)fprintf(stdout,
                            "%squickselect insertion sort cutoff = %lu, repivot_factor = %lu, repivot_cutoff = %lu, lopsided_partition_limit = %d\n",
                            comment, quickselect_insertion_cutoff,
                            repivot_factor, repivot_cutoff, lopsided_partition_limit);
                    break;
                    case 'Q' :
                        (V)fprintf(stdout, "%stest timeout = %0.2f\n", comment, timeout);
                    break;
                    case 'r' :
                        (V)fprintf(stdout, "%sintrosort_final_insertion_sort = %lu, introsort insertion sort cutoff = %lu, recursion factor = %lu\n", comment, introsort_final_insertion_sort, introsort_insertion_cutoff, introsort_recursion_factor);
                    break;
                    case 'W' :
                        (V)fprintf(stdout,
                            "%slopsided quickselect repivot_factor = %lu, repivot_cutoff = %lu, lopsided_partition_limit = %d\n",
                            comment, repivot_factor, repivot_cutoff, lopsided_partition_limit);
                    break;
                    case 'y' :
                        (V)fprintf(stdout, "%sdual-pivot insertion sort cutoff = %lu\n", comment, y_cutoff);
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
fprintf(stderr,"%s line %d: argc=%d, optind=%d\n",__func__,__LINE__,argc,optind);
#endif
    if (argc > optind+2) {
        startn = strtoul(argv[optind++], NULL, 10);
        if (2UL > startn) startn = 2UL;
        pcc = argv[optind++];
        switch (pcc[0]) {
            case '*' :
                mult = strtoul(++pcc, NULL, 10);
                if (1UL<mult) incr=0UL;
            break;
            case '+' :
                pcc++; /*FALLTHROUGH*/
            default :
                incr = strtoul(pcc, NULL, 10);
                if (0UL<incr) mult=1UL;
            break;
        }
        if (0U==flags['h']) (V)fprintf(stdout, "%sstartn = %lu, incr = %lu, mult = %lu\n", comment, startn, incr, mult);
    }

    if (optind >= argc) {
        ul = 0UL;
    } else {
        ul = strtoul(argv[optind++], NULL, 10);
        if (0U==flags['h']) (V)fprintf(stdout, "%sul = %lu\n", comment, ul);
    }
    if (1UL > ul) ul = 10000UL;
    if (2UL > ul) ul = 2UL;
#if DEBUG_CODE
fprintf(stdout, "%s line %d: ul = %lu, startn = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, ul, startn, incr, mult);
#endif
    if ((ul<startn)||(0UL==startn)) startn = ul;
#if DEBUG_CODE
fprintf(stdout, "%s line %d: startn = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, startn, incr, mult);
#endif

    if (optind >= argc) {
        count = 0UL;
    } else {
        count = strtoul(argv[optind++], NULL, 10);
        if (0U==flags['h']) (V)fprintf(stdout, "%scount = %lu\n", comment, count);
    }
    if (1UL > count)
        count = 10UL;

#if DEBUG_CODE
fprintf(stdout, "%s line %d: startn = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, startn, incr, mult);
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s%s: args processed", comment, prog);
#endif

    q=1UL;
    if (0U==flags['h']) (V)fprintf(stdout, "%s%s: %s line %d: startn = %lu, incr = %lu, mult = %lu, ul = %lu, count = %lu, ul*count = %lu, q = %lu\n", comment, __func__, source_file, __LINE__, startn, incr, mult, ul, count, ul*count, q);

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
                if (0U!=flags['a']) {
                    (V)fprintf(stdout,
                        "%sNote that adversary times may be high (comparison counts, etc. should be OK)\n",
                        comment);
                    if (0U!=flags['m']) {
                        if (0U!=flags['K']) {
                            (V)fprintf(stdout,
                                "%sNote that many sequence items will have the same value\n",
                                comment);
                           (V)fprintf(stdout,
                               "%sThe recommended way to get a complete sequence is to generate it using a sort rather than median selection\n",
                               comment);
                        }
                    }
                    if (0U==flags['K'])
                       (V)fprintf(stdout,
                           "%sThe recommended way to get accurate timing for adverse input is to save the sequence to a file using the -K option, then run the test on that sequence\n",
                           comment);
                }
            }
        }
    }

    errs = 0U;

    /* initialize (seed) random number generator and save state */
    if (0U != flags['z']) {
        unsigned int z;

        p = 0U;
        for (z=0U; z<16U; z++)
            s[z]=z+1U;
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
    q = sizeof(int)
      + sizeof(struct big_struct) 
      + sizeof(double) 
      + sizeof(long)
      + sizeof(struct big_struct *)
    ;
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
    if (0U!=flags['O']) {
        if (sz<10000000UL) sz=10000000UL;
    }
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

    array=malloc(sizeof(int)*sz);
    if (NULL==array) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        return errs;
    }
    big_array=malloc(sizeof(struct big_struct)*sz);
    if (NULL==big_array) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        free(array);
        return errs;
    }
    darray=malloc(sizeof(double)*sz);
    if (NULL==darray) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        free(array);
        free(big_array);
        return errs;
    }
    larray=malloc(sizeof(long)*sz);
    if (NULL==larray) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        free(darray);
        free(array);
        free(big_array);
        return errs;
    }
    parray=malloc(sizeof(struct big_struct *)*sz);
    if (NULL==parray) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        free(larray);
        free(darray);
        free(array);
        free(big_array);
        return errs;
    }
    /* pointers in parray point to structures in big_array */
    for (q=0UL; q<sz; q++)
        parray[q]=&(big_array[q]);

#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s%s %s: %s line %d: arrays allocated, sz=%lu", comment, prog, __func__, source_file, __LINE__, sz);
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
if (0==i) (V)fprintf(stderr, "// %s line %d: val %d, d %G, string \"%s\"\n",__func__,__LINE__,big_array[i].val,big_array[i].d,big_array[i].string);
#endif
        }
        /* allocate structures to save input data for reuse */
        input_data=malloc(sizeof(struct big_struct)*i);
        if (NULL==input_data) errs++; /* oops, malloc failed; bail out */
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            free(larray);
            free(darray);
            free(array);
            free(big_array);
            return errs;
        }
        /* save input data for reuse */
        /* reset counts to correspond to number of data read */
        for (ul=0UL; ul<(size_t)i; ul++)
            input_data[ul]=big_array[ul];
        startn=ul;
        /* emit revised counts if header is not suppressed */
        if (0U==flags['h']) (V)fprintf(stdout, "%s: %s line %d: startn = %lu, incr = %lu, mult = %lu, ul = %lu, count = %lu, ul*count = %lu, q = %lu\n", __func__, source_file, __LINE__, startn, incr, mult, ul, count, ul*count, q);
    }

    for (n=startn; n<=ul; n=(n+incr)*mult) {

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

        if ((0U == errs) && (0U != flags['O'])) { /* optimize quickselect sampling table */
            i = seed_random64n(s, p);
            if (0 != i) {
                logger(LOG_ERR, log_arg, 
                    "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                    prog, __func__, source_file, __LINE__,
                    (unsigned long)s, p, i
                );
                errs++;
            } else if (n==startn) {
                unsigned char oldi=flags['i'];
                unsigned int cs=0U,
#if 0
                    ts=0x01U<<TEST_SEQUENCE_MEDIAN3KILLER
                    ts=0x01U<<TEST_SEQUENCE_DUAL_PIVOT_KILLER
                    ts=0x01U<<TEST_SEQUENCE_RANDOM_VALUES
#else
                    ts=0x01U<<TEST_SEQUENCE_RANDOM_DISTINCT
#endif
                ;
                size_t m, step, value[3], result[3];
                int dir;
                double d;

#if 0
# define METRIC neq+nlt+ngt+nsw      /* comparisons + swaps */
# define METRIC neq+nlt+ngt+(nsw<<1) /* comparisons + 2*swaps */
# define METRIC nsw                  /* swaps */
# define METRIC neq+nlt+ngt          /* comparisons */
# define METRIC neq+nlt+ngt+(nsw>>1) /* comparisons + swaps/2 */
# define METRIC neq+nlt+ngt+(nsw>>2) /* comparisons + swaps/4 */
#else
# define METRIC 1000000UL*rusage_end.ru_utime.tv_sec+rusage_end.ru_utime.tv_usec-1000000UL*rusage_start.ru_utime.tv_sec-rusage_start.ru_utime.tv_usec /* run-time */
#endif
                tests = TEST_TYPE_MEDIAN | TEST_TYPE_PARTITION | TEST_TYPE_SORT ;
                flags['i']=1U;
                for (x=1UL; x<SAMPLING_TABLE_SIZE; x++)
                    sampling_table[x].min_nmemb=(SIZE_T_MAX);
                for (x=1UL; x<SAMPLING_TABLE_SIZE-1UL; x++) {
                    value[0] = sampling_table[x-1UL].min_nmemb + 1UL;
                    value[1] = sampling_table[x].samples * sampling_table[x].samples; /* start at samples^^2 (McGeock&Tygar, Martinez&Roura) */
                    value[2] = value[1] * 2UL;
                    if (value[1]>sz) break;
                    m = value[2]*2UL;
                    if (32768UL>m) m=32768UL;
                    if (m>sz) m=sz;
#if 1
                    /* many runs for randomized data */
                    d=test_factor(TEST_TYPE_SORT,m);
                    if (d!= 0.0) y=snlround(1.0e8/d,logger,log_arg); else y=499UL;
                    if (y>sz/m) y=sz/m;
                    if (499UL<y) y=499UL;
                    if (9UL>y) y=9UL;
#else
                    /* single run for fixed sequence data */
                    y=1UL;
#endif
                    sampling_table[x].min_nmemb = value[1];
fprintf(stderr, "testing for %lu samples at nmemb >= %lu\n", sampling_table[x].samples, sampling_table[x].min_nmemb);
                    (V)getrusage(RUSAGE_SELF,&rusage_start);
                    (V)test_function(prog, array, big_array, darray, larray, parray, FUNCTION_SQSORT, m, y, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags);
                    (V)getrusage(RUSAGE_SELF,&rusage_end);
                    result[1] = METRIC;
fprintf(stderr, "cutoff for %lu samples @ %lu -> %lu metric\n", sampling_table[x].samples, sampling_table[x].min_nmemb, result[1]);
                    step=(value[1]>>4);
                    if (1UL>step) step=1UL;
fprintf(stderr, "step=%lu\n",step);
                    for (dir=-1; 0!=dir;) {
fprintf(stderr, "testing %lu <= nmemb for %lu samples <= %lu\n",value[0], sampling_table[x].samples, value[2]);
                        switch (dir) {
                            case -1 :
                                value[0] = value[1]-step;
                                if (value[0]<=sampling_table[x-1UL].min_nmemb)
                                    value[0]=sampling_table[x-1UL].min_nmemb+1UL, step=1UL;
                                sampling_table[x].min_nmemb = value[0];
                                (V)seed_random64n(s, p);
fprintf(stderr, "testing for %lu samples at nmemb >= %lu\n", sampling_table[x].samples, sampling_table[x].min_nmemb);
                (V)getrusage(RUSAGE_SELF,&rusage_start);
                                (V)test_function(prog, array, big_array, darray, larray, parray, FUNCTION_SQSORT, m, y, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags);
                (V)getrusage(RUSAGE_SELF,&rusage_end);
                                result[0] = METRIC;
fprintf(stderr, "cutoff for %lu samples @ %lu -> %lu metric (best %lu@%lu)\n", sampling_table[x].samples, sampling_table[x].min_nmemb, result[0], result[1], value[1]);
                                if ((result[2]==result[1])&&(result[0]==result[1])) {
                                    value[1] = value[2];
                                    dir=0;
                                } else if (result[0]<result[1]) {
                                    value[1] = value[0];
                                    result[1] = result[0];
#if 0
                                    step += step>>1;
#endif
                                } else if ((result[0]>=result[1])&&(value[2]==value[1]+1UL)) {
                                    dir=0;
                                } else {
                                    dir=1;
                                    step -= step>>1;
                                    if (1UL>step) step=1UL;
                                }
                            break;
                            case 1 :
                                value[2] = value[1]+step;
                                sampling_table[x].min_nmemb = value[2];
                                (V)seed_random64n(s, p);
fprintf(stderr, "testing for %lu samples at nmemb >= %lu\n", sampling_table[x].samples, sampling_table[x].min_nmemb);
                (V)getrusage(RUSAGE_SELF,&rusage_start);
                                (V)test_function(prog, array, big_array, darray, larray, parray, FUNCTION_SQSORT, m, y, &cs, &ts, &tests, col, 9999.9, f, log_arg, flags);
                (V)getrusage(RUSAGE_SELF,&rusage_end);
                                result[2] = METRIC;
fprintf(stderr, "cutoff for %lu samples @ %lu -> %lu metric (best %lu@%lu)\n", sampling_table[x].samples, sampling_table[x].min_nmemb, result[2], result[1], value[1]);
                                if ((result[2]==result[1])&&(result[0]==result[1])) {
                                    value[1] = value[2];
                                    dir=0;
                                } else if (result[2]<result[1]) {
                                    value[1] = value[2];
                                    result[1] = result[2];
#if 0
                                    step += step>>1;
#endif
                                } else if ((result[2]>=result[1])&&(value[0]==value[1]-1UL)) {
                                    if (result[2]==result[1]) value[1]=value[2];
                                    dir=0;
                                } else {
                                    dir=-1;
                                    step -= step>>1;
                                    if (1UL>step) step=1UL;
                                }
                            break;
                        }
fprintf(stderr, "step=%lu\n",step);
                    }
                    sampling_table[x].min_nmemb = value[1];
fprintf(stderr, "%lu samples for nmemb >= %lu (%lu metric)\n", sampling_table[x].samples, sampling_table[x].min_nmemb, result[1]);
                }
                for (x=1UL; x<SAMPLING_TABLE_SIZE; x++) {
                    if (((SIZE_T_MAX)==sampling_table[x].min_nmemb)
                    && ((SIZE_T_MAX) / sampling_table[x].samples > sampling_table[x].samples))
                        sampling_table[x].min_nmemb = sampling_table[x].samples * sampling_table[x].samples;
                    i = snul(buf,sizeof(buf),NULL,"UL,",sampling_table[x].min_nmemb, 10, ' ', 21, f, log_arg);
                    i = snul(buf2,sizeof(buf2),NULL,"UL },",sampling_table[x].samples, 10, ' ', 12, f, log_arg);
                    printf("   {%s%s\n", buf, buf2);
                }
                flags['i']=oldi;
            }
            flags['O']=0U; /* reset to re-enable output from timing tests */
        }

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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_BMQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_MBMQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_NBQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_QSORT_WRAPPER, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_SQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_WQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
            }
        }

        if ((0U == errs) && (0U != flags['m'])) { /* test quickselect median */
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_QSELECT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_QSELECT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_GLQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
            }
        }

        if ((0U == errs) && (0U != flags['l'])) { /* test library qsort */
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_QSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_ISORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_HEAPSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_INTROSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_YQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, larray, parray, FUNCTION_DPQSORT, n, count, &csequences, &tsequences, &tests, col, timeout, f, log_arg, flags);
            }
        }

    }

#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: tests ended", prog);
#endif

    if (NULL!=input_data) free(input_data);
    free(parray);
    free(larray);
    free(darray);
    free(big_array);
    free(array);

#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: arrays freed", prog);
#endif

    if ((0U==flags['h'])&&(NULL!=f)&&(0UL!=errs)) f(LOG_INFO, log_arg, "%s: errs=%lu", prog, errs);
    return (0U==errs?0:1);

#undef buildstr
#undef xbuildstr
}
