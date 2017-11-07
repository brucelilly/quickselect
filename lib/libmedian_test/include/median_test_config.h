/*INDENT OFF*/

/* Description: header file for configuration and declarations for median_test */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    median_test_config.h copyright 2017 Bruce Lilly.   \ median_test_config.h $
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
/* $Id: ~|^` @(#)   This is median_test_config.h version 1.9 dated 2017-11-07T18:51:15Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/include/s.median_test_config.h */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* configuration options */
#define ASSERT_CODE              1 /* N.B. ASSERT_CODE > 1 may affect comparison and swap counts */
#define DEBUG_CODE               1
#define GENERATOR_TEST           1
#define SILENCE_WHINEY_COMPILERS 0

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
/* string size should be at least 8 to hold base-36 32-bit integers */
/* string size should be at least 15 to hold base-36 64-bit integers */
/* string size should be at least 20 to hold CCSDS 301.4 date-time string (w/o fraction) */
#define STRING_SIZE              22
/* base 36 feasible for string size >= 15 */
/* base 16 feasible for string size >= 17 */
/* base 10 feasible for string size >= 21 */
#define STRING_BASE              36

#define TEST_TIMEOUT             600.0  /* seconds (default value) */

#define RESULTS_COLUMN           2

#define MAX_ARRAY_PRINT          290UL

#define QUICKSELECT_ARG_CHECKS   DEBUG_CODE

#define DISCREPANCY_THRESHOLD    1.05

/* for even-sized arrays, return lower- (else upper-) median */
#define LOWER_MEDIAN             0

/* place medians at start of array instead of middle */
#define MEDIANS_FIRST            1

#define BM_INSERTION_CUTOFF      7UL  /* 7UL in original B&M code */

/* When selecting a relatively large number of order statistics (where "large"
   depends on the sub-array size and the distribution of the desired order
   statistic ranks within the sub-array's ranks), it may be more efficient to
   sort the sub-array, avoiding the overhead of order statistic rank evaluation.
   An alternative is to avoid the overhead of evaluating the distribution of
   order statistic ranks, and just use selection (caller is responsible for
   specifying a sort for a large number of widely distributed order statistics).
*/
#define SORT_FOR_SELECTION       1

#define USE_FMED5                0 /* using median-of-5 from 25 <= nmemb < 81 raises sorting and selection cost */

/* When sorting by merge sort, comparing the last element of the first sorted
   sub-array to the first element of the second sorted sub-array adds 1
   comparison for all inputs to save n/2-1 comparisons for already-sorted
   input.
*/
#define COMPARE1                 0

/* 0 to use separate sampling tables for sorting, middle, ends, distributed,
      extended, separated rank distributions.  Selection vs. sorting for
      separated, ends, middle, distributed.
   1 consolidates into sorting, middle, ends, distributed.  Selection vs.
      sorting for middle, ends, distributed.
   2 further consolidates into sorting, middle, ends.  Selection vs. sorting
      for distributed, others.
*/
#define CONSOLIDATED_TABLES     2

/* nothing (much) to configure below this line */

/* for assertions */
#if ! DEBUG_CODE
# if ! ASSERT_CODE
#  define NDEBUG 1
# endif
#endif

/* feature test macros defined before any header files are included */
/* Minimum _XOPEN_SOURCE version for C99 (else illumos compilation fails) */
#undef MAX_XOPEN_SOURCE_VERSION
#undef MIN_XOPEN_SOURCE_VERSION
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) && defined(MIN_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && defined(MAX_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE 1
#endif

/* for AI_ADDRCONFIG on Linux */
#undef _BSD_SOURCE
#define _BSD_SOURCE 1
#undef _SVID_SOURCE
#define _SVID_SOURCE 1

#if 0 /* defined only in quickselect_s.c */
/* test preliminary qsort_s, quickselect_s support */
#ifndef __STDC_WANT_LIB_EXT1__
# define __STDC_WANT_LIB_EXT1__ 1
#endif
#endif

/* size_t is needed for nsw declaration */
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */

/* gcc barfs if nsw is not declared before macro defining text using nsw */
extern size_t nsw;

/* count swaps using nsw */
#define SWAP_COUNT_STATEMENT nsw++

/* local header files needed */
#include "civil_time.h"         /* civil_time_cmp parse_civil_time_text sn_civil_time */
#include "compare.h"            /* charcmp ... */
#include "cutoff_value_decl.h"
;
#include "exchange.h"           /* alignment_size blockmove reverse irotate protate swapn EXCHANGE_SWAP */
#include "get_host_name.h"      /* get_host_name */
#include "get_ips.h"            /* get_ips */
#include "logger.h"             /* logger */
#include "paths_decl.h"         /* path_basename */
#include "quickselect_config.h" 
#include "random64n.h"          /* random64n */
#include "shuffle.h"            /* fisher_yates_shuffle */
#include "snn.h"                /* snf sng snl snlround snul */
#include "timespec.h"           /* timespec_subtract timespec_to_double */
#include "utc_mktime.h"         /* utc_mktime */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <sys/param.h>          /* MAXHOSTNAMELEN */
#include <sys/resource.h>       /* RUSAGE_* struct rusage */
#include <sys/time.h>           /* struct timeval */
#include <sys/times.h>          /* times */
#include <sys/types.h>          /* *_t */
#include <assert.h>             /* assert */
#include <ctype.h>              /* isalpha isdigit ispunct isspace tolower */
#include <errno.h>              /* errno E* */
#include <limits.h>             /* *_MAX *_BIT */
#include <math.h>               /* log2 */
#include <regex.h>              /* regcomp regerror regexec regfree */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t (maybe RSIZE_MAX) */
# if __STDC_VERSION__ >= 201001L
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
# endif /* N1570 */
#else
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 */
#include <stdio.h>              /* *printf (maybe errno_t rsize_t [N1570 K3.5]) */
#include <stdlib.h>             /* free malloc realloc strtol strtoul
                                   (maybe errno_t rsize_t constraint_handler_t [N1570 K3.6]
                                */
#include <string.h>             /* memcpy strrchr strstr */
#include <time.h>               /* CLOCK_* clock_gettime */
#include <unistd.h>             /* getpid */

#ifndef SIZE_MAX /* not standardized pre-C99 */
# define SIZE_MAX ULONG_MAX
#endif

#undef MEDIAN_STDC_VERSION
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MEDIAN_STDC_VERSION __STDC_VERSION__ + 0L
#elif defined(__STDC__)
# define MEDIAN_STDC_VERSION __STDC__ + 0L
#else
# define MEDIAN_STDC_VERSION 0L
#endif /* C99 */

  /* C11 RSIZE_MAX, errno_t, rsize_t, constraint_handler_t are presumed to be all or none */
#ifndef RSIZE_MAX /* provide definitions if lacking in compilation environment */
    typedef int errno_t;
    typedef size_t rsize_t;
    typedef void(*constraint_handler_t)(const char * restrict msg, void * restrict ptr, errno_t error);
#  define RSIZE_MAX ((SIZE_MAX)>>1)
#  define QUICKSELECT_PROVIDE_HANDLER 1 /* provide static implementation of set_constraint_handler_s */
#  define QUICKSELECT_HANDLER_PROVIDED 0
#endif /* RSIZE_MAX */

#undef optind
#undef DISABLED
#undef TRANSPARENT
#undef LOOSE
#undef RELAXED
#undef AGGRESSIVE
#undef EXPERIMENTAL

#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

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

/* maybe volatitle qualifier to preclude excessive optimization */
#undef VOL
#define VOL volatile

/* data types */
#define DATA_TYPE_LONG                         1
#define DATA_TYPE_INT                          2
#define DATA_TYPE_DOUBLE                       3
#define DATA_TYPE_STRUCT                       4
#define DATA_TYPE_STRING                       5
#define DATA_TYPE_POINTER                      6
#define DATA_TYPE_UINT_LEAST8_T                7
#define DATA_TYPE_UINT_LEAST16_T               8
#define DATA_TYPE_UINT_LEAST32_T               9
#define DATA_TYPE_UINT_LEAST64_T               10

/* test sequence macros (could be an enum) */
#define TEST_SEQUENCE_STDIN                    0U  /* 0x0000001 */ /* must be first (some loops start at 1 to avoid STDIN) */
#define TEST_SEQUENCE_SORTED                   1U  /* 0x0000002 */
#define TEST_SEQUENCE_REVERSE                  2U  /* 0x0000004 */
#define TEST_SEQUENCE_ORGAN_PIPE               3U  /* 0x0000008 */
#define TEST_SEQUENCE_INVERTED_ORGAN_PIPE      4U  /* 0x0000010 */
#define TEST_SEQUENCE_ROTATED                  5U  /* 0x0000020 */ /* after organ-pipe for graphing */
#define TEST_SEQUENCE_SHIFTED                  6U  /* 0x0000040 */ /* rotated opposite direction */
#define TEST_SEQUENCE_SAWTOOTH                 7U  /* 0x0000080 */ /* 3 distinct values, repeated sequence */
#define TEST_SEQUENCE_TERNARY                  8U  /* 0x0000100 */ /* 3 distinct values, random */
#define TEST_SEQUENCE_BINARY                   9U  /* 0x0000200 */ /* 2 distinct values */
#define TEST_SEQUENCE_CONSTANT                 10U /* 0x0000400 */ /* only 1 value */
#define TEST_SEQUENCE_MANY_EQUAL_LEFT          11U /* 0x0000800 */
#define TEST_SEQUENCE_MANY_EQUAL_MIDDLE        12U /* 0x0001000 */
#define TEST_SEQUENCE_MANY_EQUAL_RIGHT         13U /* 0x0002000 */
#define TEST_SEQUENCE_MANY_EQUAL_SHUFFLED      14U /* 0x0004000 */
#define TEST_SEQUENCE_RANDOM_DISTINCT          15U /* 0x0008000 */
#define TEST_SEQUENCE_RANDOM_VALUES            16U /* 0x0010000 */
#define TEST_SEQUENCE_RANDOM_VALUES_LIMITED    17U /* 0x0020000 */
#define TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED 18U /* 0x0040000 */
#define TEST_SEQUENCE_RANDOM_VALUES_NORMAL     19U /* 0x0080000 */
#define TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL 20U /* 0x0100000 */
#define TEST_SEQUENCE_HISTOGRAM                21U /* 0x0200000 */
#define TEST_SEQUENCE_MEDIAN3KILLER            22U /* 0x0400000 */
#define TEST_SEQUENCE_DUAL_PIVOT_KILLER        23U /* 0x0800000 */
#define TEST_SEQUENCE_JUMBLE                   24U /* 0x1000000 */ /* worst-case swaps for sorting networks */
#define TEST_SEQUENCE_ANTIQUICKSELECT          25U /* 0x2000000 */ /* worst-case for quickselect */
#define TEST_SEQUENCE_PERMUTATIONS             26U  /* also via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_COMBINATIONS             27U  /* also via TEST_TYPE_THOROUGH */
#define TEST_SEQUENCE_ADVERSARY                28U  /* also via TEST_TYPE_ADVERSARY */

#define TEST_SEQUENCE_COUNT                    29U

#define MAX_PERMUTATION_SIZE                   20  /* 20! is huge */
#define MAX_COMBINATION_SIZE                   ((LONG_BIT)-1) /* avoid sign bit */

#define MAXIMUM_SAWTOOTH_MODULUS               3  /* ensures maximum 3 distinct values */
#define CONSTANT_VALUE                         3  /* a more-or-less random value */

/* 32-bit signed integers in base-36 uses at least 8 chars (incl. '\0') */
/* 64-bit signed integers in base-36 uses at least 15 chars (incl. '\0') */
/* CCSDS 301.4 date-time w/o fraction uses at least 20 chars (incl. '\0') */
/* for 64-bit integers: */
/* base 36 feasible for string size >= 15 */
/* base 16 feasible for string size >= 17 */
/* base 10 feasible for string size >= 21 */
#if ULONG_MAX > 0xffffffffUL
# if STRING_SIZE < 20
#  undef STRING_SIZE
#  define STRING_SIZE                          20
# endif
# if STRING_SIZE >= 21
#  if STRING_BASE < 10
#   undef STRING_BASE
#   define STRING_BASE                         10
#  endif
# elif STRING_SIZE >= 17
#  if STRING_BASE < 16
#   undef STRING_BASE
#   define STRING_BASE                         16
#  endif
# elif STRING_SIZE >= 15
#  if STRING_BASE < 36
#   undef STRING_BASE
#   define STRING_BASE                         36
#  endif
# endif
#else
# if STRING_SIZE < 20
#  undef STRING_SIZE
#  define STRING_SIZE                          20
# endif
# if STRING_SIZE >= 11
#  if STRING_BASE < 10
#   undef STRING_BASE
#   define STRING_BASE                         10
#  endif
# elif STRING_SIZE >= 9
#  if STRING_BASE < 16
#   undef STRING_BASE
#   define STRING_BASE                         16
#  endif
# elif STRING_SIZE >= 8
#  if STRING_BASE < 36
#   undef STRING_BASE
#   define STRING_BASE                         36
#  endif
# endif
#endif

/* debugging */
#define AQCMP_DEBUG              0x00000001U
#define COMPARE_DEBUG            0x00000002U
#define MEDIAN_DEBUG             0x00000004U
#define PARTITION_ANALYSIS_DEBUG 0x00000008U
#define PARTITION_DEBUG          0x00000010U
#define PIVOT_SELECTION_DEBUG    0x00000020U
#define PK_ADJUSTMENT_DEBUG      0x00000040U
#define RATIO_GRAPH_DEBUG        0x00000080U
#define RECURSION_DEBUG          0x00000100U
#define REMEDIAN_DEBUG           0x00000200U
#define REPARTITION_DEBUG        0x00000400U
#define REPIVOT_DEBUG            0x00000800U
#define SAMPLING_DEBUG           0x00001000U
#define SHOULD_REPIVOT_DEBUG     0x00002000U
#define SORT_SELECT_DEBUG        0x00004000U
#define SUPPORT_DEBUG            0x00008000U
#define SWAP_DEBUG               0x00010000U
#define WQSORT_DEBUG             0x00020000U

#define DEBUG_VALUE_COUNT 18

#define DEBUGGING(mx) (0U!=((mx)&debug))

/* macros to unify function calls */
#undef ISORT
#define BMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) bmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define DEDSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) dedsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define DPQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) dpqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define GLQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) glibc_quicksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define HEAPSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) heap_sort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define IBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) ibmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define INTROSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) introsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define ISORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) isort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define LOGSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) logsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define MBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) mbmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define MERGESORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) xmergesort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define MMSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) minmaxsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define NBQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) nbqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define NETWORKSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) networksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define P9QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) p9qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_internal((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,options,NULL,NULL)
#define QSEL_S(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_s_internal((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,NULL,mpk,mku-mkl,options)
#define QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SELSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) selsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SHELLSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) shellsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SMOOTHSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) smoothsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SQRTSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqrtsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define SQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define SYSMERGESORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) mergesort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define WBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) wbmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define WQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) wqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,options)
#define YQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) yqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)

#define FUNCTION_IBMQSORT       0U
#define FUNCTION_BMQSORT        1U
#define FUNCTION_MBMQSORT       2U
#define FUNCTION_NBQSORT        3U
#define FUNCTION_SQSORT         4U
#define FUNCTION_QSELECT_SORT   5U
#define FUNCTION_WQSORT         6U
#define FUNCTION_QSORT_WRAPPER  7U
#define FUNCTION_QSELECT        8U
#define FUNCTION_GLQSORT        9U
#define FUNCTION_QSORT         10U
#define FUNCTION_ISORT         11U
#define FUNCTION_SHELLSORT     12U
#define FUNCTION_MINMAXSORT    13U
#define FUNCTION_SELSORT       14U
#define FUNCTION_NETWORKSORT   15U
#define FUNCTION_HEAPSORT      16U
#define FUNCTION_INTROSORT     17U
#define FUNCTION_YQSORT        18U
#define FUNCTION_DPQSORT       19U
#define FUNCTION_SQRTSORT      20U
#define FUNCTION_LOGSORT       21U
#define FUNCTION_SMOOTHSORT    22U
#define FUNCTION_P9QSORT       23U
#define FUNCTION_MERGESORT     24U
#define FUNCTION_DEDSORT       25U
#define FUNCTION_QSELECT_S     26U
#define FUNCTION_SYSMERGESORT  27U

#define FUNCTION_COUNT         28U

/* type of test */
#define TEST_TYPE_MEDIAN       0x002U
#define TEST_TYPE_PARTITION    0x004U
#define TEST_TYPE_TRIPARTITION 0x008U
#define TEST_TYPE_ADVERSARY    0x010U
#define TEST_TYPE_SORT         0x020U
#define TEST_TYPE_THOROUGH     0x040U
#define TEST_TYPE_CORRECTNESS  0x080U
#define TEST_TYPE_TIMING       0x100U

#define HISTOGRAM_INDEX1_OFFSET 3

#define ADD_PADDING 0

#define MAXROTATION 20

/* compare-exchange primitive for sorting networks */
#undef COMPARE_EXCHANGE
#define COMPARE_EXCHANGE(ma,mb,mc,msize,mswapf,malignsize,msize_ratio) \
   if(0<COMPAR(ma,mb,mc))                                              \
       EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,nsw++)

/* worst-case for sorting network; force swaps for network sort */
#define REVERSE_COMPARE_EXCHANGE(ma,mb,mc,msize,mswapf,malignsize,msize_ratio) \
   if(0>COMPAR(ma,mb,mc))                                                      \
       EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,/**/)

/* generalized adaptive-search insertion sort:
      preset mtype, mbase, mn, mfirst, ma, mb, msize, msize_ratio, mu, mpu, mcf
      modifies mn, ma, mb
   Adaptive search would use linear search for a sorted region of 3 or fewer
      elements and binary search over 4 or more elements.  Linear search in
      3 sorted elements requires 1-3 comparisons (average 2) whereas binary
      search requires 2 comparisons always, but with slightly higher overhead.
      At 4 elements, linear search requires 1-4 comparisons (average 2.5) and
      binary search requires 2-3 comparisons (average log2(5) ~ 2.322) which
      is a bit better.  But eliminating the test for the sorted region size and
      always using binary search runs faster; eliminating the test compensates
      for the overhead of binary search.
   N.B. This works well for small arrays, but not for large nearly-sorted
      arrays, where linear search is better due to the expectation that the
      insertion point is closer than half the size of the sorted region.
*/
#if 0
            } else if (mu-l<2UL) { /* linear search <= 3 sorted elements */    \
                for (pd=mb+msize; pd<=mpu; pd+=size)                           \
                    if (0>=mcf(pa,pd)) break;                                  \
            } else { /* binary search in region of 4 or more sorted elements */\

#endif
#define ISORT_AS(mtype,mbase,mn,mfirst,ma,mb,msize,msize_ratio,mu,mpu,mcf)     \
    while (mn>mfirst) {                                                        \
        --mn;                                                                  \
        if (0<mcf(ma,mb)) {                                                    \
            register mtype *px, *py, *pz, t;                                   \
            register size_t l=mn+2UL;                                          \
            register char *pc, *pd;                                            \
            if (l>mu) {                                                        \
                pd=mb+msize; /* a simple swap */                               \
            } else { /* binary search in region of sorted elements */          \
                register size_t h=mu;                                          \
                /* Binary search for insertion position: l->index beyond       \
                   elements to rotate.  Fewer comparisons than linear search,  \
                   slightly higher overhead.                                   \
                */                                                             \
                while (l<=h) {                                                 \
                    register size_t m=l+((h-l)>>1);                            \
                    /* >= important for stable sorting, rotation distance */   \
                    if (0>=mcf(ma,mbase+m*msize)) h=m-1UL;                     \
                    else l=m+1UL;                                              \
                }                                                              \
                A(mn!=l);                                                      \
                pd=mbase+l*msize;                                              \
            }                                                                  \
            /* Insertion by rotating elements [n,l) left by 1.  Same number of \
               swaps as ripple/bubble swapping if rotation by 1 position is    \
               implemented by reversals as for general rotation, but the       \
               specific case of 1 position can be implemented more efficiently \
               using the temporary variable once per iteration of the basic    \
               type size rather than swapping, which uses the temporary        \
               variable multiple times (unless the element size is exactly the \
               same size as the basic type and the rotation distance is 1 (i.e.\
               a simple swap)).                                                \
            */                                                                 \
            pc=ma, px=(mtype *)pc, pc+=msize;                                  \
            if (0U!=instrumented) {                                            \
                size_t r=(pd-pc)/msize+1UL;                                    \
                if (MAXROTATION<=r) r=0UL;                                     \
                nrotations[r]+=msize_ratio;                                    \
            }                                                                  \
            while ((char *)px<pc) {                                            \
                py=px,t=*py,pz=py+msize_ratio;                                 \
                while ((char *)pz<pd)                                          \
                    *py=*pz,py=pz,pz+=msize_ratio;                             \
                *py=t,px++;                                                    \
            }                                                                  \
        }                                                                      \
        mb=ma,ma-=msize;                                                       \
    }

/* macro for insertion sorting two pieces (split for mergesort)
      preset all
      modifies ma, mb, mn, mu
*/
#define ISORT_PIECES(mtype,mbase,mn,mfirst,mbeyond,mna,mnb,ma,mb,msize,        \
 msize_ratio,mu,mpu,mcf)                                                       \
    /* first piece */                                                          \
    mu=mfirst+mna-1UL, mpu=mb=mbase+mu*msize, ma=mb-msize;                     \
    mn=mu; /* now index of last element */                                     \
    ISORT_AS(mtype,mbase,mn,mfirst,ma,mb,msize,msize_ratio,mu,mpu,mcf)         \
    /* second piece */                                                         \
    mu=mbeyond-1UL, mpu=mb=mbase+mu*msize, ma=mb-msize;                        \
    mn=mu; /* now index of last element */                                     \
    ISORT_AS(mtype,mbase,mn,mfirst+mna,ma,mb,msize,msize_ratio,mu,mpu,mcf)

/* merge macro:
      set mtype, mbase, mfirst, mbeyond, mb, msize, msize_ratio, mcf before call
      modifies ma, mb, mpu
*/
/* avoid test mb==pc in outer loop by splitting into multiple loops */
#undef MERGE
#define MERGE(mtype,mbase,mfirst,mbeyond,ma,mb,msize,mpu,mc,mswapf,malignsize,\
    msize_ratio)                                                              \
    {                                                                         \
        register mtype *px, *py, *pz, t;                                      \
        register char *pc, *pd;                                               \
        ma=mbase+mfirst*msize, pc=mb, mpu=mbase+mbeyond*msize;                \
        while (mb<mpu) {                                                      \
            A(pc==mb);                                                        \
            while (ma<pc) { /* first loop while pc==mb */                     \
                if (0<COMPAR(ma,pc,mc)) {                                     \
                    EXCHANGE_SWAP(swapf,ma,pc,msize,malignsize,msize_ratio,   \
                        nsw++);                                               \
                    pc+=msize; ma+=msize; break;                              \
                }                                                             \
                ma+=msize;                                                    \
            }                                                                 \
            if (pc==ma) break; /* done */                                     \
            while (ma<mb) { /* second loop; pc>mb */                          \
                if ((pc<mpu)&&(0<COMPAR(mb,pc,mc))) {                         \
                    EXCHANGE_SWAP(swapf,ma,pc,msize,malignsize,msize_ratio,   \
                        nsw++);                                               \
                    pc+=msize;                                                \
                } else {                                                      \
                    EXCHANGE_SWAP(swapf,ma,mb,msize,malignsize,msize_ratio,   \
                        nsw++);                                               \
                    if (pc>mb+msize) { /* rotation */                         \
                        px=(mtype *)mb, pd=mb+msize;                          \
                        if (0U!=instrumented) {                               \
                            size_t r=(pc-pd)/msize+1UL;                       \
                            if (MAXROTATION<=r) r=0UL;                        \
                            nrotations[r]+=msize_ratio;                       \
                        }                                                     \
                        while ((char *)px<pd) {                               \
                            py=px,t=*py,pz=py+msize_ratio;                    \
                            while ((char *)pz<pc)                             \
                                *py=*pz,py=pz,pz+=msize_ratio;                \
                            *py=t,px++;                                       \
                        }                                                     \
                    }                                                         \
                }                                                             \
                ma+=msize;                                                    \
            }                                                                 \
            A(ma==mb);                                                        \
            mb=pc; /* done or new merge */                                    \
        }                                                                     \
    }

/* structure definitions */
struct data_struct {
#if ADD_PADDING
    double pad[14];
    int_least64_t pad2[1];
#endif
    /* fields similar to struct tm, but different sizes and offsets */
    /* field sizes compatible with CCSDS 301.0-B-4 sect. 3.4.1.1, 3.4.1.2 CCS */
    /* signed values allow for arithmetic and normalization */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    int_least16_t year;  /* no (1900) offset */
//    int_least16_t yday;  /* no (1) offset */
    int_least8_t  month; /* no (1) offset */
    int_least8_t  mday;
    int_least8_t  hour;
    int_least8_t  minute;
    int_least8_t  second;
    /* field similar to struct tm */
//    int_least8_t  wday; /* unused */
#else
    signed short year; /* no (1900) offset */
    /* signed short yday; */ /* no (1) offset */
    signed char  month /* no (1) offset */
    signed char  mday;
    signed char  hour;
    signed char  minute;
    signed char  second;
    /* field similar to struct tm */
    /* signed char  wday; */ /* unused */
#endif /* C99 */
    /* CCSDS 301.0-B-4 sect. 3.5.1.1 ASCII TIME CODE A subset (no fractional seconds) */
    char string[STRING_SIZE]; /* for string comparison */
#if ADD_PADDING
    char pad3[4];
#endif
};

/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
*/
#include "tables.h"
/* repivot factors are relatively small unsigned integers */
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

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
/* aligned data block */
union aligned_union {
    void *p;
    long l;
    double d;
    int_least64_t i;
};
#endif /* C99 */

/* globals.c */
extern int instrumented_bmqsort;
extern int lopsided_partition_limit;
extern int method_partition;
extern int method_pivot;
extern int no_aux_repivot;
extern int reset_selection_factor2_count;
extern int strict_selection;
extern int use_networks;
extern int use_shell;
extern unsigned int debug;
extern unsigned int do_histogram;
extern unsigned int instrumented;
extern unsigned int introsort_final_insertion_sort;
extern unsigned int network_mask;
extern unsigned int save_partial;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
extern size_t ausz;
#endif /* C99 */
extern size_t dp_cutoff;
extern size_t histogram[8][37];
extern size_t histoswap[8];
extern size_t introsort_recursion_factor;
extern size_t introsort_small_array_cutoff;
extern size_t merge_cutoff;
extern size_t nfrozen, pivot_minrank;
extern size_t nlt, neq, ngt;
extern size_t nmerges, npartitions, nrecursions, nrepivot;
extern size_t nrotations[];
extern size_t quickselect_small_array_cutoff;
extern size_t repivot_cutoff;
extern size_t repivot_factor;
extern size_t selection_nk;
extern size_t sort_threshold;
extern size_t y_cutoff;
extern int *global_iarray;
extern long *global_larray;
extern long *input_data;
extern long *global_refarray;
extern float *global_uarray;
extern float *global_sarray;
extern float *global_warray;
extern double *global_darray;
extern struct breakpoint_table_struct breakpoint_table[(SAMPLING_TABLE_SIZE)]; /* big enough to match biggest sampling_table_size */
extern struct data_struct **global_parray;
extern struct data_struct *global_data_array;
extern struct stats_table_struct stats_table[(SAMPLING_TABLE_SIZE)]; /* big enough to match biggest sampling_table_size */
# if ASSERT_CODE + DEBUG_CODE
extern FILE *logfile;
#endif /* ASSERT_CODE */

/* libmedian tables.c */
extern struct repivot_table_struct sorting_repivot_table_transparent[];
extern struct repivot_table_struct sorting_repivot_table_loose[];
extern struct repivot_table_struct sorting_repivot_table_relaxed[];
extern struct repivot_table_struct sorting_repivot_table_aggressive[];
extern struct repivot_table_struct sorting_repivot_table_disabled[];

/* aqsort.c */
extern int aqcmp(const void *px, const void *py);
extern long aqindex(const void *pv, void *base, size_t size);
extern long freeze(long z);
extern size_t count_frozen(char *base, size_t first, size_t beyond, size_t size);
extern size_t pivot_rank(char *base, size_t first, size_t beyond, size_t size,
    char *pivot);
extern void pivot_sample_rank(register char *middle, register size_t row_spacing,
    register size_t sample_spacing, register size_t size,
    register unsigned int idx, char *pivot, size_t *prank, char *base);
extern void initialize_antiqsort(size_t n, char *pv, int type, const size_t sz, long *alt);
extern size_t antiqsort_ncmp; /* number of comparisons */
extern size_t antiqsort_nsolid; /* number of solid items */

/* bmqsort.c */
extern void bmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *));

/* compare.c */
extern int ilongcmp(const void *p1, const void *p2);
extern int longcmp_s(const void *p1, const void *p2, void *unused);
extern int ilongcmp_s(const void *p1, const void *p2, void *unused);
extern int iintcmp(const void *p1, const void *p2);
extern int intcmp_s(const void *p1, const void *p2, void *unused);
extern int iintcmp_s(const void *p1, const void *p2, void *unused);
extern int idoublecmp(const void *p1, const void *p2);
extern int doublecmp_s(const void *p1, const void *p2, void *unused);
extern int idoublecmp_s(const void *p1, const void *p2, void *unused);
extern int data_struct_strcmp(const void *p1, const void *p2);
extern int idata_struct_strcmp(const void *p1, const void *p2);
extern int data_struct_strcmp_s(const void *p1, const void *p2, void *unused);
extern int idata_struct_strcmp_s(const void *p1, const void *p2, void *unused);
extern int timecmp(const void *p1, const void *p2);
extern int itimecmp(const void *p1, const void *p2);
extern int timecmp_s(const void *p1, const void *p2, void *unused);
extern int itimecmp_s(const void *p1, const void *p2, void *unused);
extern int indcmp(const void *p1, const void *p2);
extern int iindcmp(const void *p1, const void *p2);
extern int indcmp_s(const void *p1, const void *p2, void *unused);
extern int iindcmp_s(const void *p1, const void *p2, void *unused);
extern int nocmp(void *unused1, void *unused2);

/* correctness.c */
extern size_t test_array_distinctness(const void *pv, size_t l, size_t u, size_t size, int(*compar)(const void *, const void *), void (*f)(int, void *, const char *, ...), void *log_arg);
extern size_t test_array_partition(const void *pv, size_t l, size_t pl, size_t pu, size_t u, size_t size, int(*compar)(const void *, const void *), void(*f)(int, void *, const char *, ...), void *log_arg);
extern size_t test_array_partition_s(const void *pv, size_t l, size_t pl, size_t pu, size_t u, size_t size, int(*compar)(const void *, const void *,void *), void *context, void(*f)(int, void *, const char *, ...), void *log_arg);
extern size_t test_array_median(const void *pv, size_t l, size_t m, size_t u, size_t size, int(*compar)(const void *, const void *), void(*f)(int, void *, const char *, ...), void *log_arg);
extern size_t test_array_sort(const void *pv, size_t l, size_t u, size_t size, int(*compar)(const void *, const void *), unsigned int distinct, void(*f)(int, void *, const char *, ...), void *log_arg);
extern unsigned int correctness_tests(int type, size_t size, long *refarray,
    long *larray, int *array, double *darray, struct data_struct *data_array,
    struct data_struct **parray, const char *typename,
    int (*compar)(const void *,const void *),
    int (*compar_s)(const void *,const void *,void *),
    void (*swapf)(void *,void *,size_t), unsigned int options, const char *prog,
    unsigned int func, size_t n, size_t count, unsigned int *psequences,
    unsigned int *ptests, int col, double timeout,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn);

/* debug.c */
extern const char *debug_name(unsigned int value);

/* dedicated_sort.c */
extern void dedicated_sort(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options);
extern void dedsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),unsigned int options);

/* dual.c */
extern void dpqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

/* duplicate.c */
extern void duplicate_test_data(long *refarray, long *larray, int *array, double *darray, struct data_struct *data_array, struct data_struct **parray, size_t o, size_t n);
extern void restore_test_data(size_t o, size_t n, long *refarray, long *larray, int *array, double *darray, struct data_struct *data_array, struct data_struct **parray);

/* factorial.c */
extern size_t factorial(size_t n);

/* floor_lg.c */
extern size_t floor_lg(size_t n);

/* fmed3.c */
extern char *fmed3(register /*const*/char *pa, register /*const*/char *pb, register /*const*/char *pc,
    int(*compar)(const void *,const void *), char *base, size_t size);

/* functions.c */
extern const char *function_type(unsigned int func, unsigned int *ptests);
extern const char *function_name(unsigned int func);
extern unsigned int test_function(const char *prog, long *refarray, int *array,
    struct data_struct *data_array, double *darray, long *larray,
    struct data_struct **parray, unsigned int func, size_t n, size_t count,
    unsigned int *pcsequences, unsigned int *ptsequences, unsigned int *ptests,
    unsigned int options, int col, double timeout,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn);

/* glibc.c */
extern void glibc_quicksort (void *const pbase, size_t total_elems, size_t size,
            int(*compar)(const void *, const void *));

/* heap.c */
extern void heapify_array(char *base, size_t r, size_t u, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio);
extern size_t heap_delete_root(char *base, size_t r, size_t n, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio);

/* heapsort.c */
extern void heap_sort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio);
extern void heap_sort(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *));

/* ibmqsort.c */
extern void ibmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *));
extern void wbmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *));

/* introsort.c */
extern void introsort(char *base, size_t nmemb, size_t size, int(*compar)(const void *,const void *),unsigned int options);
extern void wintrosort(char *base, size_t nmemb, size_t size, int(*compar)(const void *,const void *),unsigned int options);

/* is_even.c */
extern int is_even(size_t n);

/* isort.c */
extern void isort_internal(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio);
extern void isort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* logsort.c */
extern void logsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

/* mbmqsort.c */
extern void mbmqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *),unsigned int options);

/* mergesort.c */
extern void xmergesort(char *base, size_t nmemb,
    const size_t size, int (*compar)(const void *,const void *));

/* minmax.c */
extern void find_minmax(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *,const void *), char **pmn, char **pmx);
extern void select_minmax(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options);

/* minmaxsort.c */
extern void minmaxsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),unsigned int options);

/* netbsd.c */
extern void nbqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *));

/* network.c */
extern void networksort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options);
extern void networksort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* operations.c */
extern void op_tests(const char *prefix, const char *prog,
    void (*f)(int, void *, const char *,...), void *log_arg);

/* parse.c */
extern double parse_expr(const char *p, char **pendptr, int base);
extern double parse_num(const char *p, char **pendptr, int base);

/* partition.c */
extern void merge_partitions(char *base, size_t first, size_t eq1,
    size_t gt1, size_t mid, size_t eq2, size_t gt2,
    size_t beyond, size_t size, void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, unsigned int options, size_t *peq,
    size_t *pgt);
extern void d_partition(char *base, size_t first, size_t beyond, char *pc, char *pd,
    register char *pivot, char *pe, char *pf, register size_t size,
    int(*compar)(const void *,const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, size_t *peq, size_t *pgt);

/* plan9.c */
extern void p9qsort(void *va, long n, long es, int (*cmp)(const void*, const void*));

/* print.c */
extern void print_int_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void print_long_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void print_data_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void print_indirect_keys(struct data_struct **target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void print_double_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void fprint_some_array(FILE *f, void *target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void print_size_t_array(size_t *target, size_t l, size_t u, const char *prefix, const char *suffix);
extern void print_some_array(void *target, size_t l, size_t u, const char *prefix, const char *suffix);
# if ASSERT_CODE + DEBUG_CODE
extern FILE *start_log(const char *path);
extern void close_log(FILE *f);
#endif /* ASSERT_CODE */

/* quickselect.c */
extern unsigned int d_sample_index(struct sampling_table_struct *psts,
    unsigned int idx, size_t nmemb);
extern void d_klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *pfk, size_t *pbk);
extern void rank_tests(char *base, size_t first, size_t p, size_t q,
    size_t beyond, size_t size, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *plk, size_t *prk, char **ppeq, char **ppgt);
extern struct sampling_table_struct *d_sampling_table(size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk, char **ppeq,
    unsigned int *psort, unsigned int *pindex, size_t nmemb);
extern void repivot_factors(unsigned int repivot_table_index, const size_t *pk,
    unsigned char *pf1, unsigned char *pf2);
extern unsigned int d_should_repivot(size_t nmemb, size_t n, size_t cutoff,
    struct sampling_table_struct *pst, unsigned int table_index,
    const size_t *pk, int *pn2, char **ppeq);
extern void d_quickselect_loop(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *),
    const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, unsigned int options, char **ppeq, char **ppgt);
extern void quickselect_internal(char *base, size_t nmemb,
    /*const*/ size_t size, int (*compar)(const void *,const void *),
    size_t *pk, size_t nk, unsigned int options, char **ppeq, char **ppgt);

/* quickselect_s.c */
extern errno_t quickselect_s_internal(void *base, rsize_t nmemb, /*const*/ rsize_t size,
    int (*compar)(const void *,const void *,void *), void *context,
    size_t *pk, size_t nk, unsigned int options);

/* select_pivot.c */
extern char *d_select_pivot(register char *base, register size_t first, size_t beyond, register size_t size,
    int (*compar)(const void *, const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    register unsigned int table_index, const size_t *pk, unsigned int options,
    char **ppc, char **ppd, char **ppe, char **ppf);

/* selsort.c */
extern void selsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* sequences.c */
extern void permute(long *p, size_t l, size_t r, size_t *c, size_t *pt);
extern int generate_long_test_array(long *p, size_t n, unsigned int testnum, long incr, uint64_t max_val, void(*f)(int, void *, const char *, ...), void *log_arg);
extern const char *sequence_name(unsigned int testnum);

/* shellsort.c */
extern size_t gaps[];
extern size_t ngaps;
extern void shellsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio);
extern void shellsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* sizes.c */
extern void print_sizes(const char *prefix, const char *prog);

/* smoothsort.c */
extern void smoothsort(void *base, size_t nel, size_t width, int(*cmp)(const void *,const void*));

/* sort5.c */
extern void sort5(char *pa, char *pb, char *pc, char *pd, char *pe, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio);

/* sqrtsort.c */
extern void sqrtsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

/* sqsort.c */
extern void sqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),unsigned int options);

/* swap.c */
extern void (*iswapn(size_t alignsize))(char *, char *, size_t);
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
extern void swapd(void * restrict p1, void * restrict p2, register size_t size);
extern void iswapd(void * restrict p1, void * restrict p2, register size_t size);
#endif /* C99 */
extern void swapstruct(register struct data_struct *p1, register struct data_struct *p2, register size_t count);
extern void iswapstruct(register struct data_struct *p1, register struct data_struct *p2, register size_t count);

/* test_size.c */
extern const char *test_size(unsigned int tests);

/* test_factor.c */
extern double test_factor(unsigned int tests, size_t n);

/* timing.c */
extern unsigned int timing_tests(int type, size_t size, long *refarray,
    long *larray, int *array, double *darray, struct data_struct *data_array,
    struct data_struct **parray, const char *typename,
    int (*compar)(const void *,const void *),
    int (*compar_s)(const void *,const void *,void *),
    void (*swapf)(void *,void *,size_t), unsigned int options, const char *prog,
    unsigned int func, size_t n, size_t count, unsigned int *psequences,
    unsigned int *ptests, int col, double timeout,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn);

/* ulsqrt.c */
extern size_t ulsqrt(size_t n);

/* valid.c */
extern int valid_test(unsigned int test_type, unsigned int testnum, size_t n);

/* wqsort.c */
extern size_t minimum_remedian_rank(unsigned int table_index);
extern char *freeze_some_samples(register char *base, register size_t first, size_t beyond,
    register size_t size, int (*compar)(const void *, const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    register unsigned int table_index, unsigned int options);
extern void wqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *), size_t *pk, size_t nk, unsigned int options);

/* yaroslavskiy.c */
extern void yqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

