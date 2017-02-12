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
/* 64-bit machine: struct size is 32 bytes for string size 13-20
                                  40 for string size 21-28
                                  48 for string size 29-36
*/
/* 32-bit machine: struct size is 24 bytes for string size 5-8
                                  24 for string size 9-12
                                  28 for string size 13-16
                                  32 for string size 17-20
                                  36 for string size 21-24
                                  40 for string size 25-28
                                  44 for string size 29-32
                                  48 for string size 33-36
*/
/* should be at least 8 to hold base-36 32-bit integers */
/* should be at least 15 to hold base-36 64-bit integers */
#define BIG_STRUCT_STRING_SIZE      33

#define ASSERT_CODE                 0
#define DEBUG_CODE                  0
#define GENERATOR_TEST              0
#define SILENCE_WHINEY_COMPILERS    0

#define TEST_TIMEOUT                5.0      /* seconds (default value) */

#define RESULTS_COLUMN              72

#define MAX_ARRAY_PRINT             90UL

#define QUICKSELECT_ARG_CHECKS      DEBUG_CODE

/* for even-sized arrays, return lower- (else upper-) median */
#define LOWER_MEDIAN                0

#define BM_INSERTION_CUTOFF 7UL /* 7UL in original B&M code */
// #define BM_INSERTION_CUTOFF 17UL /* 7UL in original B&M code */
#define BM_SWAP_CODE        1    /* 0 uses exchange.h swap */
#define BM_MEDIAN_CODE      1    /* 0 preferred because redundant comparisons can be eliminated */
#define BM_PVINIT           1    /* 0 preferred because PVINIT screws up pointer differences */
#define BM_RECURSE          1    /* 0 preferred because recursion on both regions causes the stack to grow too much */
#define GL_SWAP_CODE        1    /* 0 uses exchange.h swap */

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
# define _XOPEN_SOURCE 500
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
#include <ctype.h>              /* isdigit isspace */
#include <errno.h>
#include <limits.h>             /* *_MAX *_BIT */
#include <math.h>               /* log2 */
#include <stdio.h>
#include <stdlib.h>             /* strtol strtoul */
#include <string.h>             /* strrchr */
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

#define OPTSTRING "aAbBC:dFghHiIk:KlL:mnNoqQ:RsStT:wz"
#define USAGE_STRING     "[-a] [-A] [-b] [-B] [-C sequences] [-d] [-F] [-g] [-h] [-H] [-i] [-I] [-k col] [-l] [-L lim] [-m] [-n] [-N] [-o] [-q] [-Q timeout] [-R] [-s] [-S] [-t] [-T sequences] [-w] [-z] [[start incr]] [size [count]]\n\
-a\ttest with McIlroy quicksort adversary\n\
-A\talphabetic (string) data tests\n\
-b\ttest Bentley&McIlroy qsort\n\
-B\toutput data for drawing box plots\n\
-C sequences\tinclude correctness tests for specified sequences\n\
-d\tturn on debugging output\n\
-F\tfloating-point (double) tests\n\
-g\ttest glibc qsort code\n\
-h\tomit program headings (args, parameters, machine)\n\
-H\toutput host information\n\
-i\tuse instrumented comparison functions and report results\n\
-I\tinteger tests\n\
-k col\t align timing numbers at column col\n\
-K\twrite integer sequence generated by adversary (if given with -a)\n\
-l\ttest library qsort\n\
-L lim\tset upper limit for pivot larger part ratio to whole\n\
-m\ttest quickselect median\n\
-n\tdo nothing except as specified by option flags\n\
-N\ttest NetBSD qsort code\n\
-o\tprint execution costs of operations on basic types\n\
-q\ttest quickselect sort\n\
-Q timeout\ttime out tests after timeout seconds\n\
-R\traw timing output (median time (and comparison counts if requested) only)\n\
-s\tprint sizes of basic types\n\
-S\tstructure tests\n\
-t\tthorough tests\n\
-T sequences\tinclude timing tests for specified sequences\n\
-w\ttest qsort_wrapper\n\
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
#define DATA_TYPE_INT    2
#define DATA_TYPE_DOUBLE 3
#define DATA_TYPE_STRUCT 4
#define DATA_TYPE_STRING 5

/* test sequence macros (could be an enum) */
#define TEST_SEQUENCE_STDIN                 0U
#define TEST_SEQUENCE_SORTED                1U
#define TEST_SEQUENCE_REVERSE               2U
#define TEST_SEQUENCE_RANDOM_DISTINCT       3U
#define TEST_SEQUENCE_ORGAN_PIPE            4U
#define TEST_SEQUENCE_INVERTED_ORGAN_PIPE   5U
#define TEST_SEQUENCE_SAWTOOTH              6U
#define TEST_SEQUENCE_BINARY                7U
#define TEST_SEQUENCE_CONSTANT              8U
#define TEST_SEQUENCE_MEDIAN3KILLER         9U
#define TEST_SEQUENCE_MANY_EQUAL_LEFT       10U
#define TEST_SEQUENCE_MANY_EQUAL_MIDDLE     11U
#define TEST_SEQUENCE_MANY_EQUAL_RIGHT      12U
#define TEST_SEQUENCE_MANY_EQUAL_SHUFFLED   13U
#define TEST_SEQUENCE_RANDOM_VALUES         14U
#define TEST_SEQUENCE_RANDOM_VALUES_LIMITED 15U
#define TEST_SEQUENCE_PERMUTATIONS          16U  /* via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_COMBINATIONS          17U  /* via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_ADVERSARY             18U  /* via TEST_TYPE_ADVERSARY */ /* must be last (comparison function) */

#define TEST_SEQUENCE_COUNT                 19U


#define EXPECTED_RANDOM_MIN           (0)
#define EXPECTED_RANDOM_MAX           (INT_MAX)

#define MAXIMUM_SAWTOOTH_MODULUS 3
#define CONSTANT_VALUE           3

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
                  ", TEST_TIMEOUT=" xbuildstr(TEST_TIMEOUT)
                  ;
static struct big_struct *input_data=NULL;
/* for instrumented comparisons and swaps: */
static size_t nlt, neq, ngt, nsw;

/* initialize at run-time */
static void initialize_median_test(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    else
        s++;
    median_test_initialized = register_build_strings(build_options, &source_file, s);
}

static inline int big_struct_intcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;
        int a=pa->val, b=pb->val;

        if (a > b)
            return 1;
        if (a < b)
            return -1;
    }
    return 0;
}

static inline int ibig_struct_intcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;
        int a=pa->val, b=pb->val;

        if (a > b) {
            ngt++;
            return 1;
        }
        if (a < b) {
            nlt++;
            return -1;
        }
        neq++;
    }
    return 0;
}

static inline int big_struct_strcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;

        return strcmp(pa->string, pb->string);
    }
    return 0;
}

static inline int ibig_struct_strcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct big_struct *pa = (const struct big_struct *)p1,
            *pb = (const struct big_struct *)p2;
        int c;

        c = strcmp(pa->string, pb->string);
        if (0 < c) ngt++;
        else if (0 > c) nlt++;
        else neq++;
        return c;
    }
    return 0;
}

static inline int iintcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const int *pa = (const int *)p1, *pb = (const int *)p2;
        int a=*pa, b=*pb;

        if (a > b) {
            ngt++;
            return 1;
        }
        if (a < b) {
            nlt++;
            return -1;
        }
        neq++;
    }
    return 0;
}

static inline int idoublecmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const double *pa = (const double *)p1, *pb = (const double *)p2;
        double a=*pa, b=*pb;

        if (a > b) {
            ngt++;
            return 1;
        }
        if (a < b) {
            nlt++;
            return -1;
        }
        neq++;
    }
    return 0;
}

/* Print elements of integer array with indices l through u inclusive. */
static void print_int_array(int *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (void)fprintf(stderr, ", ");
        (void)fprintf(stderr, "%lu:%d", i, target[i]);
    }
    (void)fprintf(stderr, "\n");
}

static void print_big_array_keys(struct big_struct *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (void)fprintf(stderr, ", ");
        (void)fprintf(stderr, "%lu:%d", i, target[i].val);
    }
    (void)fprintf(stderr, "\n");
}

static void print_double_array(double *target, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (void)fprintf(stderr, ", ");
        (void)fprintf(stderr, "%lu:%G", i, target[i]);
    }
    (void)fprintf(stderr, "\n");
}

/* optimized for & less expensive than % (esp. for unsigned long) */
static
inline
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

static double test_array_pivot(const void *pv, size_t l, size_t p, size_t u, size_t size, int (*comparison_function)(const void *, const void *), void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, eq, gt, lt, lim, n;
    double ratio = 1.0;

    if (l > p) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > p (%lu)", __func__, source_file, __LINE__, l, p);
        return ratio;
    }
    if (p > u) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: p (%lu) > u (%lu)", __func__, source_file, __LINE__, p, u);
        return ratio;
    }
    if (NULL == pv) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return ratio;
    }
    n = (u + 1UL - l);
    if (0UL == n) return ratio;
    lim = l + n - 1UL;
    gt=lt=0UL,eq=1UL;
    if (p >= l+1UL)
        for (i=p-1UL; 1; i--) {
            c = comparison_function((const char *)pv+i*size, (const char *)pv+p*size);
            if (0 < c) gt++; else if (0 > c) lt++; else eq++;
            if (i < l+1UL) break;
        }
    for (i=p+1UL; i<=lim; i++) {
        c = comparison_function((const char *)pv+i*size, (const char *)pv+p*size);
        if (0 < c) gt++; else if (0 > c) lt++; else eq++;
    }
    if (eq + gt + lt != n) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: eq %lu + gt %lu + lt %lu != n %lu", __func__, source_file, __LINE__, eq, gt, lt, n);
        return ratio;
    }
    if (gt > lt) ratio = (double)gt / (double)n; else ratio = (double)lt / (double)n;
    return ratio;
}

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
static int antiqsort_nsolid; /* number of solid items */
static int pivot_candidate; /* pivot candidate */
static int antiqsort_gas; /* gas value */
/* qsort uses one array, antiqsort another */
static char *antiqsort_base, *qsort_base;
static size_t antiqsort_typsz, qsort_typsz;
static int antiqsort_type, qsort_type;

/* freeze implemented as a function */
static inline int freeze(int z)
{
    struct big_struct *pbs;
    int *pint;

    switch (antiqsort_type) {
        case DATA_TYPE_INT :
            pint = (const int *)(antiqsort_base + z*antiqsort_typsz);
            *pint = antiqsort_nsolid++;
        break;
        case DATA_TYPE_STRUCT :
        /*FALLTHROUGH*/
        case DATA_TYPE_STRING :
            pbs = (const struct big_struct *)(antiqsort_base + z*antiqsort_typsz);
            pbs->val = antiqsort_nsolid++;
        break;
    }
    return antiqsort_nsolid;
}

/* comparison function called by qsort; values corresponding to pointers are
   used as indices into the antiqsort array
*/
static inline int cmp(const void *px, const void *py) /* per C standard */
{
    int a, b, x, y;
    const struct big_struct *pbs;
    const int *pint;

    antiqsort_ncmp++;
    switch (qsort_type) {
        case DATA_TYPE_INT :
            x = *((const int *)px);
            y = *((const int *)py);
#if DEBUG_CODE
(void)fprintf(stderr,"// %s: x=%d, y=%d\n",__func__,x,y);
#endif
        break;
        case DATA_TYPE_DOUBLE :
            x = (int)(*((const double *)px));
            y = (int)(*((const double *)py));
#if DEBUG_CODE
(void)fprintf(stderr,"// %s: *px=%G, x=%d, *py=%G, y=%d\n",__func__,*((const double *)px),x,*((const double *)py),y);
#endif
        break;
        case DATA_TYPE_STRUCT :
        /*FALLTHROUGH*/
        case DATA_TYPE_STRING :
            pbs = (const struct big_struct *)px;
            x = pbs->val;
            pbs = (const struct big_struct *)py;
            y = pbs->val;
        break;
    }
    switch (antiqsort_type) {
        case DATA_TYPE_INT :
            pint = (const int *)(antiqsort_base + x*antiqsort_typsz);
            a = *pint;
            pint = (const int *)(antiqsort_base + y*antiqsort_typsz);
            b = *pint;
        break;
        case DATA_TYPE_STRUCT :
        /*FALLTHROUGH*/
        case DATA_TYPE_STRING :
            pbs = (const struct big_struct *)(antiqsort_base + x*antiqsort_typsz);
            a = pbs->val;
            pbs = (const struct big_struct *)(antiqsort_base + y*antiqsort_typsz);
            b = pbs->val;
        break;
    }
#if DEBUG_CODE
(void)fprintf(stderr,"// %s line %d: a=%d, b=%d\n",__func__,__LINE__,a,b);
#endif
    /* brackets added BL */
    if ((a==antiqsort_gas) && (b==antiqsort_gas)) {
        if (x == pivot_candidate) {
            a = freeze(x);
        } else {
            b = freeze(y);
        }
    }
#if DEBUG_CODE
(void)fprintf(stderr,"// %s line %d: a=%d, b=%d\n",__func__,__LINE__,a,b);
#endif
    if (a == antiqsort_gas) { pivot_candidate = x; }
    else if (b == antiqsort_gas) { pivot_candidate = y; }
    if (a > b) return 1; else if (a < b) return -1; else return 0;
 /* only the sign matters */
}

/* instrumented version */
static int icmp(const void *px, const void *py) /* per C standard */
{
    int c;

    switch ((c=cmp(px,py))) {
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
    struct big_struct *pbs;
    double *pd;
    int *pint;

    A(NULL!=pv);A(NULL!=alt);A(pv!=alt);
    qsort_base = pv;
    qsort_type = type;
    qsort_typsz = sz;
    antiqsort_base = alt;
    antiqsort_type = alt_type;
    antiqsort_typsz = altsz;
    antiqsort_gas = n - 1UL;
    antiqsort_nsolid = pivot_candidate = 0;
    antiqsort_ncmp = 0UL;
#if DEBUG_CODE
(void)fprintf(stderr,"// %s: n=%lu, pv=%p, sz=%lu, alt=%p, altsz=%lu, type=%d, alt_type=%d\n",__func__,n,pv,sz,alt,altsz,type,alt_type);
#endif
    for(i=0UL; i<n; i++) {
        /* initialize the array that qsort uses */
        switch (qsort_type) {
            case DATA_TYPE_INT :
                pint = (int *)(qsort_base + qsort_typsz*i);
                *pint = (int)i;
            break;
            case DATA_TYPE_DOUBLE :
                pd = (double *)(qsort_base + qsort_typsz*i);
                *pd = (double)i;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                pbs = (struct big_struct *)(qsort_base + qsort_typsz*i);
                pbs->val = (int)i;
                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE, NULL, NULL, (long)i, 36, '0', BIG_STRUCT_STRING_SIZE-1, NULL, NULL);
            break;
        }
        /* initialize the array that antiqsort uses */
        switch (antiqsort_type) {
            case DATA_TYPE_INT :
                pint = (int *)(antiqsort_base + antiqsort_typsz*i);
                *pint = (int)antiqsort_gas;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                pbs = (struct big_struct *)(antiqsort_base + antiqsort_typsz*i);
                pbs->val = (int)antiqsort_gas;
                /* antiqsort uses only integer data */
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
        long t = *(long*)(a);                   \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
    } else                                 \
        swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)
#else
#define SWAPINIT(a,b)      swaptype=1
#define bmswap(a,b) swap(a,b,1UL,es)
#define vecswap(a,b,n) swap(a,b,n,1UL)
#endif

#if BM_PVINIT
# define PVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; bmswap(pv, pm); }    \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }
#else
# define PVINIT(pv,pm)   pv=a; bmswap(pv, pm);
#endif /* BM_PVINIT */

#define bmmin(x, y) ((x)<=(y) ? (x) : (y))

static char *med3(char *a, char *b, char *c, int (*compar)())
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

void bmqsort(char *a, size_t n, size_t es, int (*compar)(), unsigned int debug)
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s; /* added BL */

loop:   SWAPINIT(a, es);
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
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower == region */
        /* pa-a (a.k.a. pa-pl) is the number of chars in the lower == block */
        /* pb-pa is the number of chars in the < block */
        s = bmmin(pa-a,  pb-pa   ); vecswap(a,  pb-s, s);
        /* after the swap (above), the start of the == section is at pb-s */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper == region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper == block */
        /* pd-pc is the number of chars in the > block */
        s = bmmin(pd-pc, pn-pd-es); vecswap(pb, pn-s, s);

        if ((s = pb-pa) > es) bmqsort(a,    s/es, es, compar, debug);
#if BM_RECURSE
        if ((s = pd-pc) > es) bmqsort(pn-s, s/es, es, compar, debug);
#else
        if ((s = pd-pc) > es) {
                /* Iterate rather than recurse to save stack space */
                a = pn - s;
                n = s / es;
                goto loop;
        }
#endif
}
/* ************************************************************************** */

#ifndef bmmin
#define bmmin(x, y) ((x)<=(y) ? (x) : (y))
#endif

/* NetBSD qsort code (derived from Bentley&McIlroy code) */
/* _DIAGASSERT -> assert */
void nbqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *), unsigned int debug)
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
        size_t d, r;
        int swaptype, cmp_result;

        assert(a != NULL || n == 0 || es == 0);
        assert(compar != NULL);

loop:        SWAPINIT((char *)a, es);
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
                nbqsort(a, r / es, es, compar, debug);
        if ((r = pd - pc) > es) { 
                /* Iterate rather than recurse to save stack space */
                a = pn - r;
                n = r / es;
                goto loop;
        }
/*                nbqsort(pn - r, r / es, es, compar, debug);*/
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
#if GL_SWAP_CODE
#define SWAP(a, b, size) \
do { \
    size_t __size = (size); \
    char *__a = (a), *__b = (b); \
    do { \
        char __tmp = *__a; \
        *__a++ = *__b; \
        *__b++ = __tmp; \
    } while (--__size > 0); \
} while (0)
#else
# define SWAP(a,b,size)   swap(a,b,1UL,size)
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
#define PUSH(low, high) ((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define POP(low, high) ((void) (--top, (low = top->lo), (high = top->hi)))
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
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :
        return "many equal (left)";
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
        return "many equal (middle)";
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
        return "many equal (right)";
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
        return "many equal (shuffled)";
        case TEST_SEQUENCE_PERMUTATIONS :
        return "distinct permutations";
        case TEST_SEQUENCE_COMBINATIONS :
        return "0/1 combinations";
        case TEST_SEQUENCE_ADVERSARY :
        return "McIlroy adversary";
        default:
            (void)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
        break;
    }
    return "invalid testnum";
}

/* replace elements of p between l and r inclusive with element value modulo modulus */
static void modulo(int *p, size_t l, size_t r, int modulus)
{
    for (;l<=r; l++)
        p[l] %= modulus;
}

/* reverse the order of elements in p between l and r inclusive */
static void reverse(int *p, size_t l, size_t r)
{
    int i;

    for (;l<r; l++,r--) {
        i=p[l]; p[l]=p[r]; p[r]=i;
    }
}

/* fill integer array p between l and r inclusive with integers beginning with n */
static void increment(int *p, size_t l, size_t r, int n)
{
    for (;l<=r; l++)
        p[l] = n++;
}

/* fill integer array p between l and r inclusive with constant value n */
static void constant_fill(int *p, size_t l, size_t r, int n)
{
    for (;l<=r; l++)
        p[l] = n;
}

/* fill integer array p with random values in [0,modulus) */
static void random_fill(int *p, size_t count, int modulus)
{
    size_t j;

    for (j=0UL; j<count; j++)
        *p++ = (int)random64n(modulus);
}

static int generate_integer_test_arrays(void *pv, int type, size_t count, size_t n, unsigned int testnum, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret = -1;
    int *array = (int *)pv;

    if (NULL != pv) {
        size_t h, j, k, o, r, u;
        size_t permutation_limit;
        int i;
        int *p = (int *)pv;

        u = n - 1UL;
        for (o=0UL,r=0UL; r<count; r++,o+=n) {
            switch (testnum) {
                case TEST_SEQUENCE_STDIN :
                    for(i=0,j=o; j<=o+u; i++,j++)
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
                            exchange(pv, o+h+1UL, j, 1UL, sizeof(int));
                            if (2UL > h)  /* don't allow h to iterate below 2 */
                                break;
                        }
                    }
                break;
                case TEST_SEQUENCE_RANDOM_DISTINCT :
                    increment(&p[o],0UL,u,0);
                    (void)fisher_yates_shuffle(&(p[o]), n, sizeof(int), random64n, f, log_arg);
                break;
                case TEST_SEQUENCE_RANDOM_VALUES :
                    random_fill(&p[o],n,INT_MAX);
                break;
                case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                    random_fill(&p[o],n,(int)n);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_LEFT :
                    random_fill(&p[o],n,(int)n);
                    constant_fill(&p[o],0UL,n>>2,CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
                    random_fill(&p[o],n,(int)n);
                    constant_fill(&p[o],(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
                    random_fill(&p[o],n,(int)n);
                    constant_fill(&p[o],(n>>1)+(n>>2),u,CONSTANT_VALUE);
                break;
                case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                    random_fill(&p[o],n,(int)n);
                    constant_fill(&p[o],(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
                    (void)fisher_yates_shuffle(&(p[o]), n, sizeof(int), random64n, f, log_arg);
                break;
                case TEST_SEQUENCE_PERMUTATIONS :
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
                        (void)fprintf(stderr, "permutation test limit is %lu < %lu\n", permutation_limit, n);
                        r = count;     /* break out of loop */
                        ret = 2;
                    } else /* n <= 12 or 19 */ {
                        size_t b=n*sizeof(int), d[20], s, t;

                        if (0UL == r) { /* check size; reinitialize count; initialize d, and first set */
                            s = factorial(n);
                            if (s > count) { /* array is too small to hold all permutations */
                                (void)fprintf(stderr, "%lu! = %lu > %lu\n", n, s, count);
                                r = count;     /* break out of loop */
                                ret = 3;
                                break;
                            }
                            ret = generate_integer_test_arrays(pv, type, 1UL, n, TEST_SEQUENCE_SORTED, f, log_arg);
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
                        memcpy(&(array[o]), &(array[o-n]), b);
                        /* permute copy of array by swapping one pair of elements */
                        /* Based on Algorithm 4a (Alternate Ives) in Sedgewick "Permutation Generation Methods" */
                        for (t=0UL; t < n; ) {
                            if (d[t] < u - t) {
                                if (0 == is_even(t)) { /* t is odd */
                                    exchange(array, o+d[t], o+d[t]+1UL, 1UL, sizeof(int));
                                } else { /* t is even */
                                    exchange(array, o+d[t], o+u-t, 1UL, sizeof(int));
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
                        (void)fprintf(stderr, "Thorough test limit is %lu < %lu\n", 8 * sizeof(unsigned long), n);
                        r = count;     /* break out of loop */
                        ret = 5;
                    } else {
                        size_t b, s;

                        if (0UL == r) { /* check size; reinitialize count */
                            s = 0x01UL << n;
                            if (s > count) { /* array is too small to hold all combinations */
                                (void)fprintf(stderr, "2^%lu = %lu > %lu\n", n, s, count);
                                r = count;     /* break out of loop */
                                ret = 6;
                                break;
                            }
                            count = s;
                        }
                        /* combinations of 0, 1 elements for this test */
                        for (b=o; b<o+n; b++) {
                            array[b] = (int)((r >> b) & 0x01UL);
                        }
                    }
                break;
                case TEST_SEQUENCE_ADVERSARY :
                    /* handled by initialize_antiqsort */
                break;
                default:
                    (void)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
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
#define BMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) bmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mdbg)
#define GLQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) glibc_quicksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define NBQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) nbqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mdbg)
#define QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl+1UL)

#define FUNCTION_BMQSORT        5U
#define FUNCTION_NBQSORT        6U
#define FUNCTION_GLQSORT        7U
#define FUNCTION_QSORT          8U
#define FUNCTION_QSELECT        9U
#define FUNCTION_QSORT_WRAPPER 10U

/* type of test */
#define TEST_TYPE_PIVOT        0x001U
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
        if (TEST_TYPE_PIVOT == (TEST_TYPE_PIVOT & tests)) {
            if ('\0' != buf[0]) (void)strlcat(buf, ",", sz);
            (void)strlcat(buf, "pivot", sz);
        }
        if (TEST_TYPE_PARTITION == (TEST_TYPE_PARTITION & tests)) {
            if ('\0' != buf[0]) (void)strlcat(buf, ",", sz);
            (void)strlcat(buf, "partition", sz);
        }
        if (TEST_TYPE_MEDIAN == (TEST_TYPE_MEDIAN & tests)) {
            if ('\0' != buf[0]) (void)strlcat(buf, ",", sz);
            (void)strlcat(buf, "median", sz);
        }
        if (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests)) {
            if ('\0' != buf[0]) (void)strlcat(buf, ",", sz);
            (void)strlcat(buf, "sort", sz);
        }
        if (TEST_TYPE_THOROUGH == (TEST_TYPE_THOROUGH & tests)) {
            if ('\0' != buf[0]) (void)strlcat(buf, ",", sz);
            (void)strlcat(buf, "thorough", sz);
        }
        if (TEST_TYPE_ADVERSARY == (TEST_TYPE_ADVERSARY & tests)) {
            if ('\0' != buf[0]) (void)strlcat(buf, ",", sz);
            (void)strlcat(buf, "adversary", sz);
        }
        if ('\0' == buf[0]) (void)strlcat(buf, "(none)", sz);
    }
}
#endif

/* expected time factor for tests: N log(N) for sorting, N for others (pivot, partition, median) */
static double test_factor(unsigned int tests, size_t n)
{
    double d = (double)n;

    if (TEST_TYPE_ADVERSARY == (TEST_TYPE_ADVERSARY & tests)) {
        d *= d;
    }
    if (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests)) {
        d *= log2(d);
    }
    return d;
}

/* expected time factor for tests: N log(N) for sorting, N for others (pivot, partition, median) */
static const char *test_size(unsigned int tests)
{
    if (TEST_TYPE_ADVERSARY == (TEST_TYPE_ADVERSARY & tests)) return "N^2";
    if (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests)) return "N log(N)";
    return "N";
}

/* name of the function to be tested */
static const char *function_name(unsigned int func, size_t n)
{
    const char *ret=NULL;

    switch (func) {
        case FUNCTION_QSELECT :       ret = "quickselect";
        break;
        case FUNCTION_QSORT :         ret = "library qsort";
        break;
        case FUNCTION_BMQSORT :       ret = "Bentley&McIlroy qsort";
        break;
        case FUNCTION_NBQSORT :       ret = "NetBSD qsort";
        break;
        case FUNCTION_GLQSORT :       ret = "glibc qsort";
        break;
        case FUNCTION_QSORT_WRAPPER : ret = "qsort_wrapper";
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
            *ptests |= ( TEST_TYPE_PIVOT | TEST_TYPE_PARTITION | TEST_TYPE_MEDIAN );
            if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests))  ret = "sort"; else ret = "median";
        break;
        case FUNCTION_BMQSORT :       /*FALLTHROUGH*/
        case FUNCTION_NBQSORT :       /*FALLTHROUGH*/
        case FUNCTION_GLQSORT :       /*FALLTHROUGH*/
        case FUNCTION_QSORT :         ret = "sort";
            *ptests |= ( TEST_TYPE_PIVOT | TEST_TYPE_PARTITION | TEST_TYPE_MEDIAN | TEST_TYPE_SORT );
        break;
        case FUNCTION_QSORT_WRAPPER : ret = "sort";
            *ptests |= ( TEST_TYPE_PIVOT | TEST_TYPE_PARTITION | TEST_TYPE_MEDIAN | TEST_TYPE_SORT );
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
        case TEST_SEQUENCE_ORGAN_PIPE :            /*FALLTHROUGH*/
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
            if (3UL > n) return 0;
        break;
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :       /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :     /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :      /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :   /*FALLTHROUGH*/
        case TEST_SEQUENCE_SAWTOOTH :
            if (5UL > n) return 0;
        break;
        case TEST_SEQUENCE_MEDIAN3KILLER :
            if (7UL > n) return 0;
        break;
        /* not size-limited */
        case TEST_SEQUENCE_STDIN :                 /*FALLTHROUGH*/
        case TEST_SEQUENCE_SORTED :                /*FALLTHROUGH*/
        case TEST_SEQUENCE_REVERSE :               /*FALLTHROUGH*/
        case TEST_SEQUENCE_BINARY :                /*FALLTHROUGH*/
        case TEST_SEQUENCE_COMBINATIONS :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_PERMUTATIONS :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_DISTINCT :       /*FALLTHROUGH*/
        case TEST_SEQUENCE_CONSTANT :              /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED : /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES :
        break;
        /* placeholders: type-specific known (not unrecognized) tests */
        case TEST_SEQUENCE_ADVERSARY :
        break;
        default:
            (void)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n",
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
            (void)fprintf(stderr, "%s: %s line %d: unrecognized test_type %u\n",
                __func__, source_file, __LINE__, test_type);
        return 0;
    }
    return 1;
}

static unsigned int correctness_test(void *pv, const size_t size, void *alt, const size_t altsz, int type, int alt_type, double *darray, const char *typename, int (*comparison_function)(const void *, const void *), const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double lim, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
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
    int *pint;
    struct big_struct *pbs;
    double *pd;

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
            case TEST_SEQUENCE_MEDIAN3KILLER :
                count = 1UL;
                emin--; emin--; emax++; emax++;
                if (0 > emin) emin = 0;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                emin = 0 ; emax = (int)u;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
                emin = EXPECTED_RANDOM_MIN; emax = EXPECTED_RANDOM_MAX;
            break;
            case TEST_SEQUENCE_ADVERSARY :
            return ++errs; /* no correctness test */
            default:
                (void)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
        if (0U == errs) {
            /* special count numbers, etc. */
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    count = factorial(n);
                    /* generate all permutations */
                    c = generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                        type, count, n, testnum, f, log_arg);
                    if (0 > c) {
                        (void)fprintf(stderr,
                            "%s: %s line %d: generate_integer_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    if (size == sizeof(struct big_struct)) {
                        /* duplicate values for structure array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE,
                                    NULL, NULL, (long)(*pint), 36, '0',
                                    BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            }
                        }
                    } else if (size == sizeof(double)) {
                        double *pd;

                        /* duplicate values for double array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                *pd = (double)(*pint);
                            }
                        }
                    }
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    count = 0x01UL << n;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (void) setvbuf(fp, NULL, (int)_IONBF, 0);
                    /* generate all combinations */
                    c = generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                        type, count, n, testnum, f, log_arg);
                    if (0 > c) {
                        (void)fprintf(stderr,
                            "%s: %s line %d: generate_integer_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    if (size == sizeof(struct big_struct)) {
                        /* duplicate values for structure array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE,
                                    NULL, NULL, (long)(*pint), 36, '0',
                                    BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            }
                        }
                    } else if (size == sizeof(double)) {
                        double *pd;

                        /* duplicate values for double array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                *pd = (double)(*pint);
                            }
                        }
                    }
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
                        c=generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                            type, 1UL, n, testnum, f, log_arg);
                        if (0 > c) {
                            (void)fprintf(stderr,
                                "%s: %s line %d: generate_integer_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        if (size == sizeof(struct big_struct)) {
                            /* duplicate values for structure array */
                            for(j=0UL; j<n; j++) {
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE,
                                    NULL, NULL, (long)(*pint), 36, '0',
                                    BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            }
                        } else if (size == sizeof(double)) {
                            double *pd;

                            /* duplicate values for double array */
                            for(j=0UL; j<n; j++) {
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                *pd = (double)(*pint);
                            }
                        }
                    break;
                }
                /* integer test data is in integer array pv (type==DATA_TYPE_INT) or alt (others) */
                /* test sequence has been copied to others */
                /* verify correctness of test sequences */
                switch (testnum) {
                    case TEST_SEQUENCE_SORTED :               /*FALLTHROUGH*/
                        distinct=1U;
#if GENERATOR_TEST
                        t = test_array_sort(pv, o, o+u, size, comparison_function, distinct, logger, log_arg);
                        if (t != o+u) {
                            (void)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", ptest);
                            errs++;
                            erro = (unsigned int)o;
                            if (t <= o+u) errt = t;
                        }
                        if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
#endif /* GENERATOR_TEST */
                    case TEST_SEQUENCE_REVERSE :              /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_DISTINCT :      /*FALLTHROUGH*/
                    case TEST_SEQUENCE_PERMUTATIONS :         /*FALLTHROUGH*/
                        distinct=1U;
#if GENERATOR_TEST
                        if (1000UL > u) {
                            t = test_array_distinctness(pv, o, o+u, size, comparison_function, f, log_arg);
                            if (t != o+u) {
                                (void)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_distinctness) for %s ^^^^^ ERROR!!!! line %d\n", ptest, __LINE__);
                                errs++;
                                erro = (unsigned int)o;
                                if (t <= o+u) errt = t;
                            }
                        }
                        if (0U != flags['i']) ngt = nlt = neq = nsw = 0UL;
#endif /* GENERATOR_TEST */
                    break;
                    case TEST_SEQUENCE_STDIN :                /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ORGAN_PIPE :           /*FALLTHROUGH*/
                    case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :  /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SAWTOOTH :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_BINARY :               /*FALLTHROUGH*/
                    case TEST_SEQUENCE_CONSTANT :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MEDIAN3KILLER :        /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES :        /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :/*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_LEFT :      /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :    /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_RIGHT :     /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :  /*FALLTHROUGH*/
                    case TEST_SEQUENCE_COMBINATIONS :         /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ADVERSARY :            /*FALLTHROUGH*/
                        distinct=0U;
                    break;
                    default:
                        (void)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
                    return ++errs;
                }
                if ((2UL > count) && (MAX_ARRAY_PRINT > n)) {
                    (void)fprintf(stderr, "initial %s array:\n", ptest);
                    switch (type) {
                        case DATA_TYPE_INT :
                            print_int_array(pv, o, o+u);
                        break;
                        case DATA_TYPE_DOUBLE :
                            print_double_array(pv, o, o+u);
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
                        (void)fprintf(stdout, "%s vs. %s %s: ", pcc, typename, ptest);
                        fflush(stdout);
                }
                switch (testnum) {
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* test number */
                        if (NULL != fp) {
                            c = snul(buf, sizeof(buf), NULL, NULL, m, 2, '0', (int)n, f, log_arg);
                            (void)fprintf(fp, "%s", buf);
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
                            QSEL(pv,o,o+u,size,comparison_function,karray,0UL,1UL,rpt);
                        }
#if DEBUG_CODE
(void)fprintf(stderr, "%s line %d: quickselect returned\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
                        qsort_wrapper((char *)pv+size*o,u+1UL,size,comparison_function);
                    break;
                }
                if ((1UL==count)&&(0U != flags['i'])) {
                    const char *psize;
                    double factor;

                    t=nlt+neq+ngt;
                    factor = test_factor(*ptests, n);
                    psize = test_size(*ptests);
                    d = (double)t / factor;
                    if (0U != flags['K']) comment="#";
                    if (0U!=flags['R']) {
                        c = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                        (void)printf("%s%s%*.*s%.6G %s\n", comment, buf, col-c, col-c, " ", d, psize);
                    } else {
                        c = 1 + snprintf(buf, sizeof(buf), "%s %s vs. %lu %s comparison counts:", pcc, pfunc, n, ptest);
                        (void)printf("%s%s%*.*s%lu < (%.1f%%), %lu == (%.1f%%), %lu > (%.1f%%), total %lu: %.6G %s\n",
                            comment, buf, col-c, col-c, " ", nlt, 100.0*(double)nlt/(double)t, neq,
                            100.0*(double)neq/(double)t, ngt, 100.0*(double)ngt/(double)t,
                            t, d, psize);
                    }
                }
                if (j > o+u) { errs++; }
                if ((0U == errs) && (TEST_TYPE_PIVOT == (TEST_TYPE_PIVOT & *ptests))) {
                    d = test_array_pivot(pv, o, j, o+u, size, comparison_function, f, log_arg);
                    if (d > lim) {
                        (void)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_pivot) for %s ^^^^^ ERROR!!!! ratio %.2f%% > %.2f%%: line %d\n", pcc, ptest, 100.0*d, 100.0*lim, __LINE__);
                        errs++;
                        erro = (unsigned int)o;
                    }
                }
                if ((0U == errs) && (TEST_TYPE_PARTITION == (TEST_TYPE_PARTITION & *ptests))) {
                    t = test_array_partition(pv, o, eql, equ, o+u, size, comparison_function, f, log_arg);
                    if (t != eql) {
                        (void)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_partition) for %s ^^^^^ ERROR!!!! line %d\n", pcc, ptest, __LINE__);
                        errs++;
                        erro = (unsigned int)o;
                        if (t <= o+u) errt = t;
                    }
                }
                if ((0U == errs) && (TEST_TYPE_MEDIAN == (TEST_TYPE_MEDIAN & *ptests))) {
                    t = test_array_median(pv, o, j, o+u, size, comparison_function, f, log_arg);
                    if (t != j) {
                        (void)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_median) for %s ^^^^^ ERROR!!!! line %d\n", pcc, ptest, __LINE__);
                        errs++;
                        erro = o;
                        if (t <= o+u) errt = t;
                    } else if (j != k) k = j-o; /* some median functions do not move data */
                    if ((emin > k) || (emax < k)) {
                        (void)fprintf(stderr, "ERROR ^^^^^: %s failed for %s: emin %d !<= array[o %lu + k %lu] %d !<= emax %d ^^^^^ ERROR!!!! line %d\n", pcc, ptest, emin, o, k, ik, emax, __LINE__);
                        if (0U == errs) errs++;
                        erro = o;
                    }
                }
                if ((0U == errs) && (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests))) {
                    t = test_array_sort(pv, o, o+u, size, comparison_function, distinct, logger, log_arg);
                    if (t != o+u) {
                        (void)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", pcc, ptest);
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
                (void)fprintf(stderr, "%s returned index %lu, range %lu through %lu\n", pcc, j, eql, equ);
                if (eql < o)
                    (void)fprintf(stderr, "ERROR ^^^^^: %s failed (eql %lu < o %lu) for %s ^^^^^ ERROR!!!!\n", pcc, eql, o, ptest);
                if (equ > o+u)
                    (void)fprintf(stderr, "ERROR ^^^^^: %s failed (equ %lu < o+u %lu) for %s ^^^^^ ERROR!!!!\n", pcc, equ, o+u, ptest);
                if (MAX_ARRAY_PRINT > n) {
                    switch (type) {
                        case DATA_TYPE_INT :
                            if (0UL != errt) print_int_array(pv, errt-1UL, errt+1UL);
                            print_int_array(pv, erro, erro+u);
                        break;
                        case DATA_TYPE_DOUBLE :
                            if (0UL != errt) print_double_array(pv, errt-1UL, errt+1UL);
                            print_double_array(pv, erro, erro+u);
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
                        case DATA_TYPE_INT :
                            if (0UL != errt) print_int_array(pv, errt-1UL, errt+1UL);
                            print_int_array(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                        case DATA_TYPE_DOUBLE :
                            if (0UL != errt) print_double_array(pv, errt-1UL, errt+1UL);
                            print_double_array(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                        case DATA_TYPE_STRUCT :
                        /*FALLTHROUGH*/
                        case DATA_TYPE_STRING :
                            if (0UL != errt) print_big_array_keys(pv, errt-1UL, errt+1UL);
                            print_big_array_keys(pv, 0UL<eql?eql-1UL:eql, equ+1UL);
                        break;
                    }
                }
                if (0U == rpt) {
                    /* re-run test with debugging turned on */
                    for(j=o; j<o+n; j++) {
                        switch (type) {
                            case DATA_TYPE_INT :
                                pbs=(struct big_struct *)((char*)alt + j*altsz);
                                pint=(int *)((char *)pv + j*size);
                                *pint = pbs->val;
                            break;
                            case DATA_TYPE_DOUBLE :
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*altsz);
                                *pd = (double)(*pint);
                            break;
                            case DATA_TYPE_STRUCT :
                            /*FALLTHROUGH*/
                            case DATA_TYPE_STRING :
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*altsz);
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE, NULL, NULL, (long)(*pint), 36, '0', BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            break;
                        }
                    }
                    if (MAX_ARRAY_PRINT > n) {
                        (void)fprintf(stderr, "initial %s array:\n", ptest);
                        switch (type) {
                            case DATA_TYPE_INT :
                                print_int_array(pv, o, o+u);
                            break;
                            case DATA_TYPE_DOUBLE :
                                print_double_array(pv, o, o+u);
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
                (void)fprintf(stdout, "passed\n");
                fflush(stdout);
            }
        } /* run test if no errors */
    } /* generate test sequence and run test if no errors */
    return errs;
}

static unsigned int timing_test(void *pv, const size_t size, void *alt, const size_t altsz, int type, int alt_type, double *darray, const char *typename, int (*comparison_function)(const void *, const void *), const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count,  unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    char buf[256], buf2[256], buf3[256], buf4[256], buf5[256], buf6[256],
        buf7[256], buf8[256], buf9[256], buf10[256], buf11[256], buf12[256],
        buf13[256], buf14[256], buf15[256], buf16[256], buf17[256], buf18[256],
        buf19[256], buf20[256], buf21[256], buf22[256], buf23[256], buf24[256],
        buf25[256], buf26[256], buf27[256], buf28[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="", *psize;
    const char *comment="";
    int c, emin, emax;
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
    struct big_struct *pbs;
    auto struct rusage rusage_start, rusage_end;
    auto struct timespec timespec_start, timespec_end, timespec_diff;
    double *sarray, *uarray, *warray;
    double best_s=9.9e99, best_u=9.9e99, best_w=9.9e99, d, factor, test_s,
        tot_s=0.0, test_u, tot_u=0.0, test_w, tot_w=0.0, worst_s=0.0,
        worst_u=0.0, worst_w=0.0;

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
            case TEST_SEQUENCE_MEDIAN3KILLER :
                emin--; emin--; emax++; emax++;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                emin = 0 ; emax = (int)u;
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
                (void)fprintf(stderr, "%s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
        if (0U==errs) {
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    count = factorial(n);
                    /* generate all permutations */
                    c = generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                        type, count, n, testnum, f, log_arg);
                    if (0 > c) {
                        (void)fprintf(stderr,
                            "%s: %s line %d: generate_integer_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    if (size == sizeof(struct big_struct)) {
                        /* duplicate values for structure array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE,
                                    NULL, NULL, (long)(*pint), 36, '0',
                                    BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            }
                        }
                    } else if (size == sizeof(double)) {
                        double *pd;

                        /* duplicate values for double array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                *pd = (double)(*pint);
                            }
                        }
                    }
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    count = 0x01UL << n;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (void) setvbuf(fp, NULL, (int)_IONBF, 0);
                    /* generate all combinations */
                    c = generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                        type, count, n, testnum, f, log_arg);
                    if (0 > c) {
                        (void)fprintf(stderr,
                            "%s: %s line %d: generate_integer_test_arrays returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return ++errs;
                    } else if (0 < c) {
                        return ++errs;
                    }
                    /* copy test sequence to alternates */
                    if (size == sizeof(struct big_struct)) {
                        /* duplicate values for structure array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE,
                                    NULL, NULL, (long)(*pint), 36, '0',
                                    BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            }
                        }
                    } else if (size == sizeof(double)) {
                        double *pd;

                        /* duplicate values for double array */
                        for (m=o=0UL; m<count; m++,o+=n) {
                            for(j=o; j<o+n; j++) {
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                *pd = (double)(*pint);
                            }
                        }
                    }
                break;
            }
            /* preparation */
            if (0U != flags['i']) ngt = nlt = neq = 0UL;
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
            for (m=o=dn=0UL; m<count; m++) {
                test_s = test_u = test_w = 0.0;
                switch (testnum) {
                    case TEST_SEQUENCE_ADVERSARY :
                        /* generate new test sequence */
                        c=generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                            type, 1UL, n, TEST_SEQUENCE_SORTED, f, log_arg);
                        if (0 > c) {
                            (void)fprintf(stderr,
                                "%s: %s line %d: generate_integer_test_arrays returned %d\n",
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
                            (void)fprintf(fp, "%s", buf);
                            fflush(fp);
                        }
                    break;
                    case TEST_SEQUENCE_PERMUTATIONS :
                        /* nothing to do here */
                    break;
                    default :
                        /* generate new test sequence */
                        c=generate_integer_test_arrays(size==sizeof(int)?pv:alt,
                            type, 1UL, n, testnum, f, log_arg);
                        if (0 > c) {
                            (void)fprintf(stderr,
                                "%s: %s line %d: generate_integer_test_arrays returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        if (size == sizeof(struct big_struct)) {
                            /* duplicate values for structure array */
                            for(j=0UL; j<n; j++) {
                                pbs=(struct big_struct *)((char*)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                pbs->val = *pint;
                                (void)snl(pbs->string, BIG_STRUCT_STRING_SIZE,
                                    NULL, NULL, (long)(*pint), 36, '0',
                                    BIG_STRUCT_STRING_SIZE-1, f, log_arg);
                            }
                        } else if (size == sizeof(double)) {
                            double *pd;

                            /* duplicate values for double array */
                            for(j=0UL; j<n; j++) {
                                pd = (double *)((char *)pv + j*size);
                                pint=(int *)((char *)alt + j*sizeof(int));
                                *pd = (double)(*pint);
                            }
                        }
                    break;
                }
                /* run and time the test */
                (void)getrusage(RUSAGE_SELF,&rusage_start);
                (void)clock_gettime(CLOCK_REALTIME,&timespec_start);
                switch (func) {
                    case FUNCTION_QSELECT :
                        if (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests)) {
                            QSEL(pv,o,o+u,size,comparison_function,NULL,0UL,0UL,rpt);
                        } else {
                            karray[0] = karray[1] = k; /* 0-based rank */
                            QSEL(pv,o,o+u,size,comparison_function,karray,0UL,1UL,rpt);
                        }
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,o,o+u,size,comparison_function,karray,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
                        qsort_wrapper((char *)pv+size*o,u+1UL,size,comparison_function);
                    break;
                }
                (void)getrusage(RUSAGE_SELF,&rusage_end);
                (void)clock_gettime(CLOCK_REALTIME,&timespec_end);
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
            if (0U == errs) {
                /* summarize test results */
                if (count == ocount) {
                    factor = test_factor(~(TEST_TYPE_ADVERSARY) & *ptests, n);
                    psize = test_size(~(TEST_TYPE_ADVERSARY) & *ptests);
                } else {
                    factor = test_factor(*ptests, n);
                    psize = test_size(*ptests);
                }
                /* mean times */
                test_u = tot_u / (double)m;
                (void)sng(buf, sizeof(buf), NULL, NULL, test_u / factor, -6, 3, f, log_arg);
                (void)sng(buf6, sizeof(buf6), NULL, NULL, test_u, -6, 3, f, log_arg);
#if DEBUG_CODE
(void)fprintf(stderr, "%s line %d: tot_u=%G, factor=%G, count=%lu, mean=%G (%s)\n", __func__, __LINE__, tot_u, factor, count, test_u, buf);
#endif /* DEBUG_CODE */
                test_s = tot_s / (double)m;
                (void)sng(buf7, sizeof(buf7), NULL, NULL, test_s / factor, -4, 3, f, log_arg);
                (void)sng(buf8, sizeof(buf8), NULL, NULL, test_s, -4, 3, f, log_arg);
                test_w = tot_w / (double)m;
                (void)sng(buf9, sizeof(buf9), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
                (void)sng(buf10, sizeof(buf10), NULL, NULL, test_w, -4, 3, f, log_arg);
                c = 1 + snprintf(buf3, sizeof(buf3), "%s %s of %lu %s%s: %s: (%lu * %lu)", pcc, pfunc, n, typename, (n==1)?"":"s", ptest, n, count);
                (void)sng(buf4, sizeof(buf4), NULL, NULL, tot_u, -4, 3, f, log_arg);
                (void)sng(buf15, sizeof(buf15), NULL, NULL, tot_s, -4, 3, f, log_arg);
                (void)sng(buf16, sizeof(buf16), NULL, NULL, tot_w, -4, 3, f, log_arg);
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
(void)fprintf(stderr, "%s line %d: dn=%lu, n=%lu, u=%lu, dn-n-1UL=%lu, marray[4]=%lu, marray[5]=%lu\n", __func__, __LINE__, dn, n, u, dn-n-1UL, marray[4], marray[5]);
#endif /* DEBUG_CODE */
ustats:             quickselect((void *)(uarray), dn, sizeof(double), doublecmp,
                        marray, 10UL);
#if DEBUG_CODE
(void)fprintf(stderr, "%s line %d: quickselect returned\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                    test_u = (uarray[marray[4]] + uarray[marray[5]]) / 2.0; /* median user */
                    (void)sng(buf5, sizeof(buf5), NULL, NULL, test_u, -4, 3, f, log_arg);
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
                            (void)sng(buf6, sizeof(buf6), NULL, NULL, test_u, -6, 3, f, log_arg);
                            (void)sng(buf, sizeof(buf), NULL, NULL, test_u / factor, -6, 3, f, log_arg);
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
                    (void)sng(buf11, sizeof(buf11), NULL, NULL, test_s, -4, 3, f, log_arg);
                    (V)sng(buf19, sizeof(buf19), NULL, NULL, sarray[marray[0]], -4, 3, f, log_arg); /* min sys */
                    (V)sng(buf20, sizeof(buf20), NULL, NULL, sarray[marray[9]], -4, 3, f, log_arg); /* max sys */
wstats:             quickselect((void *)(warray), dn, sizeof(double), doublecmp,
                        marray, 10UL);
                    test_w = (warray[marray[4]] + warray[marray[5]]) / 2.0; /* median wall */
                    (void)sng(buf13, sizeof(buf13), NULL, NULL, test_w, -4, 3, f, log_arg);
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
                            (void)sng(buf10, sizeof(buf10), NULL, NULL, test_w, -4, 3, f, log_arg);
#if 0
                            (V)fprintf(stderr,"%s%s %s old mean %.G/%lu, revised mean %.G/%lu = %s\n",comment,pcc,ptest,tot_w+warray[marray[9]],m+1UL,tot_w,m,buf10);
#endif
                            (void)sng(buf9, sizeof(buf9), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
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
                (void)sng(buf2, sizeof(buf2), NULL, NULL, test_u / factor, -4, 3, f, log_arg);
                (void)sng(buf12, sizeof(buf12), NULL, NULL, test_s / factor, -4, 3, f, log_arg);
                (void)sng(buf14, sizeof(buf14), NULL, NULL, test_w / factor, -4, 3, f, log_arg);
#if DEBUG_CODE
(void)fprintf(stderr, "%s line %d: factor=%G, count=%lu, dn=%lu, median=%G (%s)\n", __func__, __LINE__, factor, count, dn, test_u, buf2);
#endif /* DEBUG_CODE */
                if (0U != flags['K']) comment="#";
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
                    (void)printf("%s%s%*.*s%s user\n", comment, buf2, col-c, col-c, " ", 0.0!=test_u?buf5:buf6);
                    (void)printf("%s%s%*.*s%s system\n", comment, buf2, col-c, col-c, " ", 0.0!=test_s?buf11:buf8);
                    (void)printf("%s%s%*.*s%s wall\n", comment, buf2, col-c, col-c, " ", 0.0!=test_w?buf13:buf10);
                } else {
                    (void)printf("%s%s%*.*snormalized unit user times: %s (mean), %s (median), per %s, total user time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf, buf2, psize, buf4, (count!=ocount)?" (terminated early)":"", buf6, buf5);
                    if (0UL<dn) {
                        /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                        d = tot_u / test_u / (double)m; /* ratio of mean to median */
                        if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                        if (d > DISCREPANCY_THRESHOLD) (V)fprintf(stderr,"%s%s %s User time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",comment,pcc,ptest,buf17,buf6,buf18,buf5,100.0*(d-1.0));
                    }
                    (void)printf("%s%s%*.*snormalized unit system times: %s (mean), %s (median), per %s, total system time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf7, buf12, psize, buf15, (count!=ocount)?" (terminated early)":"", buf8, buf11);
                    (void)printf("%s%s%*.*snormalized unit wall clock times: %s (mean), %s (median), per %s, total wall clock time %s%s, mean %s, median %s\n", comment, buf3, col-c, col-c, " ", buf9, buf14, psize, buf16, (count!=ocount)?" (terminated early)":"", buf10, buf13);
                    if (0UL<dn) {
                        /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                        d = tot_w / test_w / (double)m; /* ratio of mean to median */
                        if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                        if (d > DISCREPANCY_THRESHOLD) (V)fprintf(stderr,"%s%s %s Wall-clock time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",comment,pcc,ptest,buf21,buf10,buf22,buf13,100.0*(d-1.0));
                    }
                    switch (testnum) {
                        case TEST_SEQUENCE_PERMUTATIONS : /*FALLTHROUGH*/
                        case TEST_SEQUENCE_COMBINATIONS :
                            (void)snprintf(buf5, sizeof(buf5), " s/ %s %s\n", psize, typename);
                            (void)sng(buf, sizeof(buf), NULL, buf5, best_u / factor, -4, 3, f, log_arg);
                            c = 1 + snprintf(buf2, sizeof(buf2), "%s %s of %lu %s%s: %s: %s(%lu)", pcc, pfunc, n, typename, (n==1)?"":"s", ptest, "best", best_m);
                            (void)printf("%s%s%*.*s%s", comment, buf2, col-c, col-c, " ", buf);
                            (void)sng(buf, sizeof(buf), NULL, buf5, worst_u / factor, -4, 3, f, log_arg);
                            c = 1 + snprintf(buf2, sizeof(buf2), "%s %s of %lu %s%s: %s: %s(%lu)", pcc, pfunc, n, typename, (n==1)?"":"s", ptest, "worst", worst_m);
                            (void)printf("%s%s%*.*s%s", comment, buf2, col-c, col-c, " ", buf);
                        break;
                        case TEST_SEQUENCE_ADVERSARY :
                            c = snul(buf, sizeof(buf), NULL, " comparisons", antiqsort_ncmp, 10, '0', 1, f, log_arg);
                            c = snul(buf2, sizeof(buf2), NULL, " solid", (unsigned long)antiqsort_nsolid, 10, '0', 1, f, log_arg);
                            c = 1 + snprintf(buf3, sizeof(buf3), "%s %s vs. %s:", pcc, pfunc, ptest);
                            (void)printf("%s%s%*.*s%s, %s: %.6G %s\n", comment, buf3, col-c, col-c, " ", buf, buf2, (double)antiqsort_ncmp / factor, psize);
                        break;
                    }
                }
                if (0U != flags['i']) {
                    t=nlt+neq+ngt;
                    d = (double)t / (double)count / factor;
                    if (0U!=flags['R']) {
                        c = 1 + snprintf(buf2, sizeof(buf2), "%lu", n);
                        (void)printf("%s%s%*.*s%.6G %s\n", comment, buf2, col-c, col-c, " ", d, psize);
                    } else {
                        c = 1 + snprintf(buf3, sizeof(buf3), "%s %s vs. %lu %s comparison counts:", pcc, pfunc, n, ptest);
                        (void)printf("%s%s%*.*s%lu < (%.1f%%), %lu == (%.1f%%), %lu > (%.1f%%), total %lu: %.6G %s\n",
                            comment, buf3, col-c, col-c, " ", nlt, 100.0*(double)nlt/(double)t, neq,
                            100.0*(double)neq/(double)t, ngt, 100.0*(double)ngt/(double)t,
                            t, d, psize);
                    }
                }
                if (0U != flags['K']) {
                    switch (testnum) {
                        case TEST_SEQUENCE_ADVERSARY :
                            (void)printf("#Adversary sequence:\n");
                            for(j=0UL; j<n; j++) {
                                switch (antiqsort_type) {
                                    case DATA_TYPE_INT :
                                        pint = (int *)(antiqsort_base + antiqsort_typsz*j);
                                        c = *pint;
                                    break;
                                    case DATA_TYPE_STRUCT :
                                    /*FALLTHROUGH*/
                                    case DATA_TYPE_STRING :
                                        pbs = (struct big_struct *)(antiqsort_base + antiqsort_typsz*j);
                                        c = pbs->val;
                                        /* antiqsort uses only integer data */
                                    break;
                                }
                                (void)printf("%d\n", c);
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

static unsigned int correctness_tests(void *pv, size_t sz, void *alt, const size_t altsz, int type, int alt_type, double *darray, const char *typename, const char *prog, unsigned int func, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double lim, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
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
            case DATA_TYPE_INT :
                if (0U != flags['i']) /* comparison/swap counts */
                    comparison_function = iintcmp;
                else
                    comparison_function = intcmp;
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
        errs += correctness_test(pv, sz, alt, altsz, type, alt_type, darray, typename, comparison_function, prog, func, testnum, n, count, psequences, ptests, col, lim, f, log_arg, flags);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}

static unsigned int timing_tests(void *pv, size_t sz, void *alt, const size_t altsz, int type, int alt_type, double *darray, const char *typename, const char *prog, unsigned int func, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
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
                case DATA_TYPE_INT :
                    if (0U != flags['i']) /* comparison/swap counts */
                        comparison_function = iintcmp;
                    else
                        comparison_function = intcmp;
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
        if (NULL!=f) {
            if (0U!=flags['K']) fprintf(stderr, "// ");
            f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: %lu %lu", prog, fname, ftype, typename, sequence_name(testnum), n, count);
        }
        errs += timing_test(pv, sz, alt, altsz, type, alt_type, darray, typename, comparison_function, prog, func, testnum, n, count, psequences, ptests, col, timeout, f, log_arg, flags);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}

static
unsigned int test_function(const char *prog, int *array, struct big_struct *big_array, double *darray, unsigned int func, size_t n, size_t count, unsigned int *pcsequences, unsigned int *ptsequences, unsigned int *ptests, int col, double lim, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    unsigned char c;
    unsigned int errs=0U;
    int type, alt_type;
    size_t sz, altsz;
    void *pv, *alt;
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
    (void)fprintf(stderr, "swap test: original structures:\n");
    (void)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
    (void)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
    (void)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
    swap2(&(big_array[0]), &(big_array[1]), sizeof(struct big_struct));
    (void)fprintf(stderr, "swap test: swap2:\n");
    (void)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
    (void)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
    (void)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
    swap(&(big_array[0]), &(big_array[2]), 1UL, sizeof(struct big_struct));
    (void)fprintf(stderr, "swap test: swap:\n");
    (void)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
    (void)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
    (void)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
    exchange(big_array, 1UL, 2UL, 1UL, sizeof(struct big_struct));
    (void)fprintf(stderr, "swap test: exchange:\n");
    (void)fprintf(stderr, "0: %d: %s\n", (big_array[0]).val, (big_array[0]).string);
    (void)fprintf(stderr, "1: %d: %s\n", (big_array[1]).val, (big_array[1]).string);
    (void)fprintf(stderr, "2: %d: %s\n", (big_array[2]).val, (big_array[2]).string);
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
        switch (c) {
            case 'A' :
                if (0U != flags[c]) { /* test requested */
                    typename = "string";
                    pv = (void *)big_array;
                    sz = sizeof(big_array[0]);
                    type = DATA_TYPE_STRING;
                    alt = (void *)array;
                    altsz = sizeof(array[0]);
                    alt_type = DATA_TYPE_INT;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'F' :
                if (0U != flags[c]) { /* test requested */
                    typename = "double";
                    pv = (void *)darray;
                    sz = sizeof(darray[0]);
                    type = DATA_TYPE_DOUBLE;
                    alt = (void *)array;
                    altsz = sizeof(array[0]);
                    alt_type = DATA_TYPE_INT;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'I' :
                if (0U != flags[c]) { /* test requested */
                    typename = "integer";
                    pv = (void *)array;
                    sz = sizeof(array[0]);
                    type = DATA_TYPE_INT;
                    alt = (void *)big_array;
                    altsz = sizeof(big_array[0]);
                    alt_type = DATA_TYPE_STRUCT;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'S' :
                if (0U != flags[c]) { /* test requested */
                    typename = "structure";
                    pv = (void *)big_array;
                    sz = sizeof(big_array[0]);
                    type = DATA_TYPE_STRUCT;
                    alt = (void *)array;
                    altsz = sizeof(array[0]);
                    alt_type = DATA_TYPE_INT;
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
                errs +=  correctness_tests(pv, sz, alt, altsz, type, alt_type, darray, typename, prog, func, n, count, pcsequences, ptests, col, lim, timeout, f, log_arg, flags);
            }
        }
        if (0U == errs) {
            if (TEST_TYPE_TIMING == (TEST_TYPE_TIMING & *ptests)) {
                errs +=  timing_tests(pv, sz, alt, altsz, type, alt_type, darray, typename, prog, func, n, count, ptsequences, ptests, col, timeout, f, log_arg, flags);
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
    (void)fprintf(stdout, "%s%s: %s (%s %s %s) COMPILER_USED=\"%s\"\n", prefix, prog, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED);
    /* common standard (as of C99) types */
    (void)fprintf(stdout, "%ssizeof(_Bool) = %lu\n", prefix, sizeof(_Bool));
    (void)fprintf(stdout, "%ssizeof(char) = %lu\n", prefix, sizeof(char));
    (void)fprintf(stdout, "%ssizeof(short) = %lu\n", prefix, sizeof(short));
    (void)fprintf(stdout, "%ssizeof(int) = %lu\n", prefix, sizeof(int));
    (void)fprintf(stdout, "%ssizeof(long) = %lu\n", prefix, sizeof(long));
    (void)fprintf(stdout, "%ssizeof(long long) = %lu\n", prefix, sizeof(long long));
    (void)fprintf(stdout, "%ssizeof(float) = %lu\n", prefix, sizeof(float));
    (void)fprintf(stdout, "%ssizeof(double) = %lu\n", prefix, sizeof(double));
    (void)fprintf(stdout, "%ssizeof(long double) = %lu\n", prefix, sizeof(long double));
    (void)fprintf(stdout, "%ssizeof(float _Complex) = %lu\n", prefix, sizeof(float _Complex));
    (void)fprintf(stdout, "%ssizeof(double _Complex) = %lu\n", prefix, sizeof(double _Complex));
    (void)fprintf(stdout, "%ssizeof(long double _Complex) = %lu\n", prefix, sizeof(long double _Complex));
    (void)fprintf(stdout, "%ssizeof(void *) = %lu\n", prefix, sizeof(void *));
    (void)fprintf(stdout, "%ssizeof(size_t) = %lu\n", prefix, sizeof(size_t));
    /* structure specific to this test program */
    (void)fprintf(stdout, "%ssizeof(struct big_struct) = %lu\n", prefix, sizeof(struct big_struct));
}

static inline int nocmp(void *unused1, void *unused2)
{
    return 0;
}

/* time basic operations and print summary table */
static
void op_tests(const char *prefix, const char *prog, void (*f)(int, void *, const char *,...), void *log_arg)
{
#define NTYPES 18
#define NOPS   33
    char buf[256], numbuf[64];
    char testmatrix[NTYPES][NOPS] = {      /* applicability matrix */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynnn", /* _Bool */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* char */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned char */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* short */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned short */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* int */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* unsigned long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* long long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynny", /* unsigned long long */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnny", /* float */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnyyy", /* double */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnny", /* long double */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnn", /* float _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnn", /* double _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnnn", /* long double _Complex */
        "ynnnnnnyyyyyyyyynnnnnnnnnnnnnnnny", /* pointer */
    };
    const char *pcc, *pcc2;
    const char *typename[NTYPES][2] = {
       { "", "_Bool" }, { "", "char" }, { "unsigned", "char" },
       { "short", "int" }, { "unsigned", "short" }, { "plain", "int"},
       { "unsigned", "int" }, { "long", "int" }, { "unsigned", "long" },
       { "long", "long" }, { "unsigned", "long long" }, { "", "float" },
       { "", "double" }, { "long", "double" }, { "float", "_Complex" },
       { "double", "_Complex" }, { "long double", "_Complex" },
       { "", "pointer" }
    };
    unsigned int testnum, typenum;
    size_t i, j, k, l;
    double timing[NTYPES][NOPS];
    FILE *fp;
    auto struct rusage rusage_start, rusage_end;

    /* flush output streams before writing to /dev/tty */
    fflush(stdout); fflush(stderr);
    /* Progress indication to /dev/tty */
    fp = fopen("/dev/tty", "w");
    if (NULL != fp) (void) setvbuf(fp, NULL, (int)_IONBF, 0);
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
            (void)getrusage(RUSAGE_SELF,&rusage_start);                   \
            for (n=0UL; n<count; n++) {                                   \
                op ;                                                      \
            }                                                             \
            (void)getrusage(RUSAGE_SELF,&rusage_end);                     \
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
    /* _Bool type can be treated as an integer, if careful */
    I(_Bool,                0,    1,    1,    nocmp)    typenum++;
    I(char,                '0',  '1',  '2',   charcmp)  typenum++;
    I(unsigned char,       '0',  '1',  '2',   nocmp)    typenum++;
    I(short,                0,    1,    2,    shortcmp) typenum++;
    I(unsigned short,       0U,   1U,   2U,   nocmp)    typenum++;
    I(int,                  0,    1,    2,    intcmp)   typenum++;
    I(unsigned int,         0U,   1U,   2U,   nocmp)    typenum++;
    I(long,                 0L,   1L,   2L,   longcmp)  typenum++;
    I(unsigned long,        0UL,  1UL,  2UL,  ulcmp)    typenum++;
    I(long long,            0LL,  1LL,  2LL,  nocmp)    typenum++;
    I(unsigned long long,   0ULL, 1ULL, 2ULL, nocmp)    typenum++;

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
    F(long double,          0.0, 1.0, 2.0, nocmp)     typenum++;

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
    (void)fprintf(stdout, "%s%s: %s (%s %s %s) COMPILER_USED=\"%s\"\n", prefix, prog, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED);
    (void)fprintf(stdout, "%sOperations: mean time per operation\n", prefix);
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
        for (typenum=0U; typenum<NTYPES; typenum++) {
            pcc = typename[typenum][i];
            k = strlen(pcc);
            j = OPWIDTH + NUMWIDTH * typenum + (NUMWIDTH - k) / 2UL;
            while ('\0' != *pcc) { buf[j++] = *pcc++; }
        }
        j = OPWIDTH + NUMWIDTH * NTYPES;
        while (' ' == buf[--j]) { buf[j] = '\0'; }
        (void)fprintf(stdout, "%s%s\n", prefix, buf);
    }
    /* print operation name and timing data */

/* macro to print one row of data, corresponding to one test operation */
#define P(op) pcc = buildstr(op);                                     \
    memset(buf, ' ', sizeof(buf));                                \
    k = strlen(pcc);                                              \
    j = (OPWIDTH - k) / 2UL;                                      \
    while ('\0' != *pcc) { buf[j++] = *pcc++; }                   \
    for (typenum=0U; typenum<NTYPES; typenum++) {                 \
        if ('y' != testmatrix[typenum][testnum]) {                \
            pcc = "N/A";                                          \
        } else {                                                  \
            (void)sng(numbuf, sizeof(numbuf), NULL, NULL,         \
                timing[typenum][testnum], -4, 3, logger, log_arg); \
            pcc = numbuf;                                         \
        }                                                         \
        k = strlen(pcc);                                          \
        j = OPWIDTH + NUMWIDTH * typenum + (NUMWIDTH - k) / 2UL;  \
        while ('\0' != *pcc) { buf[j++] = *pcc++; }               \
    }                                                             \
    j = OPWIDTH + NUMWIDTH * NTYPES;                              \
    while (' ' == buf[--j]) { buf[j] = '\0'; }                    \
    (void)fprintf(stdout, "%s%s\n", prefix, buf);

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
    char buf[4096];             /* RATS: ignore (big enough) */
    char prog[PATH_MAX];
    char host[MAXHOSTNAMELEN];  /* RATS: ignore (big enough) */
    char iplist[1024];          /* RATS: ignore (size is checked) */
    char logbuf[65536];         /* RATS: ignore (it's big enough) */
    char loghost[MAXHOSTNAMELEN] = "localhost";  /* RATS: ignore (big enough) */
    char procid[32];            /* RATS: ignore (used with sizeof()) */
    const char *pcc;
    char *comment="", *endptr=NULL;
    int bufsz, c, col= RESULTS_COLUMN , i, optind;
    int maxlen = 1024;
    int sockfd = -1;
    unsigned int seqid = 1U;
    unsigned int tzknown = 1U;
    unsigned int csequences=0U, errs=0U, p, tsequences=0U, tests;
    unsigned char flags[256];
    volatile unsigned long count, incr=1UL, n, mult=1UL, startn=0UL, ul;
    pid_t pid;
    size_t size_limit, sz, q;
    void (*f)(int, void *, const char *, ...) = logger;
    void *log_arg;
    struct logger_struct ls;
    double lim = 1.00, timeout = TEST_TIMEOUT;
    int *array;
    struct big_struct *big_array;
    double *darray;
    uint64_t s[16];
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "main";
#endif

    /* I/O initialization */
    (void) setvbuf(stdout, NULL, (int)_IONBF, BUFSIZ);
    (void) setvbuf(stderr, NULL, (int)_IONBF, BUFSIZ);
    initialize_median_test();
    /* variable initialization */
    host[0] = '\0';
    iplist[0] = '\0';
    procid[0] = '\0';
    (void)path_basename(argv[0], prog, sizeof(prog));
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
    (void)get_host_name(host, sizeof(host), logger, log_arg);
    /* host address(es) */
    (void)get_ips(iplist, sizeof(iplist), host, AF_UNSPEC, AI_ADDRCONFIG, logger, log_arg);
    (void)snul(procid, sizeof(procid), NULL, NULL, (unsigned long)pid, 10, '0', 1, logger, log_arg); /* logging of snn OK from logger caller after initialization */
    /* process command-line arguments */
    /* initialize flags */
    for (q=0UL; q<sizeof(flags); q++) flags[q] = (unsigned char)0U;
    /* initialize default sequences */
    for (p=1U; p<TEST_SEQUENCE_COUNT; p++) {
        tests=0x01U<<p;
        /* default is all valid test sequences except stdin */
        /* assume adequate size */
        if (0!=valid_test(TEST_TYPE_CORRECTNESS, p, 15UL))
            csequences |= tests;
        if (0!=valid_test(TEST_TYPE_TIMING, p, 15UL))
            tsequences |= tests;
    }
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
                case 'd' : /*FALLTHROUGH*/
                case 'D' : /*FALLTHROUGH*/
                case 'e' : /*FALLTHROUGH*/
                case 'E' : /*FALLTHROUGH*/
                case 'f' : /*FALLTHROUGH*/
                case 'F' : /*FALLTHROUGH*/
                case 'g' : /*FALLTHROUGH*/
                case 'h' : /*FALLTHROUGH*/
                case 'H' : /*FALLTHROUGH*/
                case 'i' : /*FALLTHROUGH*/
                case 'I' : /*FALLTHROUGH*/
                case 'K' : /*FALLTHROUGH*/
                case 'l' : /*FALLTHROUGH*/
                case 'm' : /*FALLTHROUGH*/
                case 'M' : /*FALLTHROUGH*/
                case 'n' : /*FALLTHROUGH*/
                case 'N' : /*FALLTHROUGH*/
                case 'o' : /*FALLTHROUGH*/
                case 'p' : /*FALLTHROUGH*/
                case 'P' : /*FALLTHROUGH*/
                case 'q' : /*FALLTHROUGH*/
                case 'R' : /*FALLTHROUGH*/
                case 's' : /*FALLTHROUGH*/
                case 'S' : /*FALLTHROUGH*/
                case 't' : /*FALLTHROUGH*/
                case 'w' : /*FALLTHROUGH*/
                case 'z' : /*FALLTHROUGH*/
                    flags[c] = 1U;
                    if (0U!=flags['K']) comment="#";
                break;
                case 'C' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    csequences = strtoul(pcc, NULL, 0);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'k' :
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    col = atoi(pcc);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                    flags[c] = 1U;
                break;
                case 'L' :
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    lim = strtod(pcc, NULL);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                    flags[c] = 1U;
                break;
                case 'Q' :
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    timeout = strtod(pcc, NULL);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                    flags[c] = 1U;
                break;
                case 'T' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    tsequences = strtoul(pcc, NULL, 0);
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
                        (void)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                        fprintf(stderr, "%s %s\n", buf, sequence_name(tests));
                    }
                return 1;
            }
        }
    }
    if (0U==flags['h']) {
        /* now output settings made by options */
        for (n=0U,ul=sizeof(flags); n<ul; n++) {
            if (0U!=flags[n]) {
                switch (n) {
                    case 'k' :
                        (void)fprintf(stdout, "%stiming column = %d\n", comment, col);
                    break;
                    case 'L' :
                        (void)fprintf(stdout, "%spivot ratio limit = %0.2f\n", comment, lim);
                    break;
                    case 'Q' :
                        (void)fprintf(stdout, "%stest timeout = %0.2f\n", comment, timeout);
                    break;
                }
            }
        }
        (void)fprintf(stdout, "%s%s", comment, prog);
        for (i = 1; i < argc; i++) { (void)fprintf(stdout, " %s", argv[i]); }
        (void)fprintf(stdout, ":\n");
        if (0U!=flags['H']) {
            (void)fprintf(stdout, "%s%s ips %s\n", comment, host, iplist);
        }
        (void)fprintf(stdout, "%s%s (%s %s %s) COMPILER_USED=\"%s\"\n", comment, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED);
#if SILENCE_WHINEY_COMPILERS
        (void)fprintf(stdout, "%sSILENCE_WHINEY_COMPILERS = %d\n", comment, SILENCE_WHINEY_COMPILERS);
#endif
        (void)fprintf(stdout, "%s%s\n", comment, build_options+4);
        (void)fprintf(stdout, "%s%s\n", comment, quickselect_build_options+4);
    }

    if (0U < flags['s']) { /* print type sizes requested */
        print_sizes(comment,prog);
    }

    if (0U < flags['o']) { /* operation timing tests requested */
        op_tests(comment, prog, logger, log_arg); /* 64-bit machines */
    }

    if (0U < flags['n']) /* do-nothing flag */
        return 0;

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
        if (0U==flags['h']) (void)fprintf(stdout, "%sstartn = %lu, incr = %lu, mult = %lu\n", comment, startn, incr, mult);
    }

    if (optind >= argc) {
        ul = 0UL;
    } else {
        ul = strtoul(argv[optind++], NULL, 10);
        if (0U==flags['h']) (void)fprintf(stdout, "%sul = %lu\n", comment, ul);
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
        if (0U==flags['h']) (void)fprintf(stdout, "%scount = %lu\n", comment, count);
    }
    if (1UL > count)
        count = 10UL;

#if DEBUG_CODE
fprintf(stdout, "%s line %d: startn = %lu, incr = %lu, mult = %lu\n", __func__, __LINE__, startn, incr, mult);
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s%s: args processed", comment, prog);
#endif

    q=1UL;
    if (0U==flags['h']) (void)fprintf(stdout, "%s%s: %s line %d: startn = %lu, incr = %lu, mult = %lu, ul = %lu, count = %lu, ul*count = %lu, q = %lu\n", comment, __func__, source_file, __LINE__, startn, incr, mult, ul, count, ul*count, q);

    /* data types */
    /* Test all types if none specified. */ 
    if ((0U==flags['A'])&&(0U==flags['F'])&&(0U==flags['I'])&&(0U==flags['S']))
        flags['A']=flags['F']=flags['I']=flags['S']=1U;

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

    if (0U==flags['h']) {
        if ((0U!=csequences)||(0U!=tsequences)) {
            (void)fprintf(stdout, "%stest sequences:\n", comment);
            if (0U != csequences) {
                (void)fprintf(stdout, "%s correctness: ", comment);
                for (errs=0U,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                    if (0U != (csequences & (0x01U << tests))) {
                        (void)snul(buf, sizeof(buf), 0U==errs?"0x":", 0x", NULL,
                            0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4, 
                            logger, log_arg);
                        (void)fprintf(stdout,"%s %s",buf,sequence_name(tests));
                        errs++;
                    }
                }
                (void)fprintf(stdout, "\n");
            }
            if (0U != tsequences) {
                (void)fprintf(stdout, "%s timing: ", comment);
                for (errs=0U,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                    if (0U != (tsequences & (0x01U << tests))) {
                        (void)snul(buf, sizeof(buf), 0U==errs?"0x":", 0x", NULL,
                            0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4, 
                            logger, log_arg);
                        (void)fprintf(stdout,"%s %s",buf,sequence_name(tests));
                        errs++;
                    }
                }
                (void)fprintf(stdout, "\n");
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
    q = sizeof(int) + sizeof(struct big_struct) + sizeof(double);
    p = (0x01U << TEST_SEQUENCE_COMBINATIONS);
    if ((0U!=(csequences&p))||(0U!=(tsequences&p))) {
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
        }
        sz=ul*(0x01<<ul);
    }
    p = (0x01U << TEST_SEQUENCE_PERMUTATIONS);
    if ( ((0U!=(csequences&p))||(0U!=(tsequences&p)))
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
    if (SIZE_T_MAX/q <= sz) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %s %lu",
            prog, __func__, source_file, __LINE__,
            "malloc will certainly fail; try again with size <",
            ul);
        return ++errs;
    }
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

#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s%s: arrays allocated, sz=%lu", comment, prog, sz);
#endif

    /* read data from stdin if requested */
    p = (0x01U << TEST_SEQUENCE_STDIN);
    if ((0U!=(csequences&p))||(0U!=(tsequences&p))) {
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
                    (void)fprintf(stderr, "%s line %d: i=%d: ignoring comment line %s\n",__func__,__LINE__,i,buf);
                /*FALLTHROUGH*/
                case '\0' : /* empty line */
                    --i; /* don't count this input line */
                continue; /* next line */
            }
            /* don't try to write past end of arrays! */
            if (i+0UL>=sz) break;
            if ((0==i)&&(0U!=flags['d'])) (void)fprintf(stderr, "%s line %d: first non-comment input line %s\n",__func__,__LINE__,buf);
            /* save line as integer, double, string */
            big_array[i].d=strtod(endptr,NULL);
            big_array[i].val=snlround(big_array[i].d,f,log_arg);
            (void)snprintf(big_array[i].string, (BIG_STRUCT_STRING_SIZE), "%*.*s", (BIG_STRUCT_STRING_SIZE)-1, (BIG_STRUCT_STRING_SIZE)-1, endptr);
#if DEBUG_CODE
if (0==i) (void)fprintf(stderr, "// %s line %d: val %d, d %G, string \"%s\"\n",__func__,__LINE__,big_array[i].val,big_array[i].d,big_array[i].string);
#endif
        }
        /* allocate structures to save input data for reuse */
        input_data=malloc(sizeof(struct big_struct)*i);
        if (NULL==input_data) errs++; /* oops, malloc failed; bail out */
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
            free(darray);
            free(array);
            free(big_array);
            return errs;
        }
        /* save input data for reuse */
        for (sz=0UL; sz<(size_t)i; sz++)
            input_data[sz]=big_array[sz];
        /* reset counts to correspond to number of data read */
        startn=ul=sz;
        /* emit revised counts if header is not suppressed */
        if (0U==flags['h']) (void)fprintf(stdout, "%s: %s line %d: startn = %lu, incr = %lu, mult = %lu, ul = %lu, count = %lu, ul*count = %lu, q = %lu\n", __func__, source_file, __LINE__, startn, incr, mult, ul, count, ul*count, q);
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_QSELECT, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_QSELECT, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
            }
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_BMQSORT, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_NBQSORT, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_GLQSORT, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_QSORT, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
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
                errs += test_function(prog, array, big_array, darray, FUNCTION_QSORT_WRAPPER, n, count, &csequences, &tsequences, &tests, col, lim, timeout, f, log_arg, flags);
            }
        }
    }

#if DEBUG_CODE
    if ((0U==flags['h'])&&(NULL!=f)) f(LOG_INFO, log_arg, "%s: tests ended", prog);
#endif

    if (NULL!=input_data) free(input_data);
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
