#ifndef MEDIAN_TEST_CONFIG_H_INCLUDED
# define MEDIAN_TEST_CONFIG_H_INCLUDED 1
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
* $Id: ~|^` @(#)    median_test_config.h copyright 2017-2020 Bruce Lilly.   \ median_test_config.h $
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
/* $Id: ~|^` @(#)   This is median_test_config.h version 1.45 dated 2020-02-03T20:36:29Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/include/s.median_test_config.h */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* configuration options */
#ifndef ASSERT_CODE
# define ASSERT_CODE             0 /* N.B. ASSERT_CODE > 1 may affect comparison and swap counts */
#endif
#undef DEBUG_CODE
#define DEBUG_CODE               1 /* enable debugging */
#define GENERATOR_TEST           1 /* validate generated test sequences */
#define SILENCE_WHINEY_COMPILERS 1

/* string size should be at least 34 to hold CCSDS 301.4 date-time string w/ picosecond resolution fraction */
#define STRING_SIZE              34

#define TEST_TIMEOUT             600.0  /* seconds (default value) */

#define MAX_ARRAY_PRINT          290UL

#define QUICKSELECT_ARG_CHECKS   DEBUG_CODE

#define DISCREPANCY_THRESHOLD    1.05

/* place medians at start of array instead of middle */
#define MEDIANS_FIRST            1

#define BM_INSERTION_CUTOFF      7UL  /* 7UL in original B&M code */

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
#if defined(__STDC__) && (__STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
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

/* size_t is needed for nsw and nrotations declarations */
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */

/* some macros need to be defined before quickselect_config is included */
/* gcc barfs if nsw is not declared before macro defining text using nsw */
extern size_t nsw;

/* gcc barfs if nrotations is not declared before macro defining text using nrotations */
/* gcc barfs if MAXROTATION is not declared before macro defining text using MAXROTATION */
extern size_t nrotations[];
#define MAXROTATION 20

/* count swaps using nsw */
#if LIBMEDIAN_TEST_CODE
# ifndef SWAP_COUNT_STATEMENT
#  define SWAP_COUNT_STATEMENT nsw++
# endif
/* count rotations */
# define QUICKSELECT_COUNT_ROTATIONS(md,mn) nrotations[(md<MAXROTATION)?md:0]+=mn /* caller supplies terminating semicolon */
#endif

/* stringification (don't change!) */
#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

/* debugging codes (need to be defined before DEBUGGING macro) */
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
#define MERGE_DEBUG              0x00040000U
#define DPQSORT_DEBUG            0x00080000U
#define CACHE_DEBUG              0x00100000U
#define MEMORY_DEBUG             0x00200000U
#define PIVOT_METHOD_DEBUG       0x00400000U
#define CORRECTNESS_DEBUG        0x00800000U
#define HISTOGRAM_DEBUG          0x01000000U

#define DEBUG_VALUE_COUNT 25
#if DEBUG_VALUE_COUNT > 32
# error "DEBUG_VALUE_COUNT " xbuildstr(DEBUG_VALUE_COUNT) " is incompatible with 32-bit unsigned integers"
#endif

/* gcc barfs if debug is not declared before macro defining text using debug */
extern unsigned int debug;

/* shorthand for use in conditionals; definition is also used to enable
   debugging features in common code blocks (e.g. inline functions in
   quickselect_config.h); must be defined before including quickselect_config.h
*/
#define DEBUGGING(mx) (0U!=((mx)&debug))

/* more gcc barf prevention */
static const char *source_file;
extern unsigned int instrumented;
extern size_t nlt, neq, ngt, nmoves; /* nsw already declared */
extern size_t nmerges, npartitions, nrecursions, nrepivot;
extern size_t npcopies, npderefs, npiconversions;
extern int forced_pivot_selection_method;
    /* print.c */
extern void print_some_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix,unsigned int options);

/* local header files needed */
#include "civil_time.h"         /* civil_time_cmp parse_civil_time_text sn_civil_time */
#include "compare.h"            /* charcmp ... */
#include "exchange.h"           /* alignment_size blockmove reverse irotate protate swapn EXCHANGE_SWAP */
#include "get_host_name.h"      /* get_host_name */
#include "get_ips.h"            /* get_ips */
/* include indirect.h only where required */
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
#if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )
# include <stdint.h>            /* (header not standardized pre-C99) SIZE_MAX
                                   (maybe RSIZE_MAX (C11))
                                */
#endif /* C99 or later */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# include <limits.h>           /* ULONG_MAX */
# define SIZE_MAX ULONG_MAX
#endif /* SIZE_MAX */
#include <float.h>              /* DBL_MAX_10_EXP */
#include <stdio.h>              /* *printf (maybe errno_t rsize_t [N1570 K3.5]) */
#include <stdlib.h>             /* free malloc realloc strtol strtoul
                                   (maybe errno_t rsize_t constraint_handler_t [N1570 K3.6]
                                */
#include <string.h>             /* strrchr strstr */
#include <time.h>               /* CLOCK_* clock_gettime */
#include <unistd.h>             /* getpid */

/* allocation of time bits between fractional seconds and date-time */
/* for 32-bit compatibility, smallest fraction should be no smaller than a nanosecond */
#if LONG_MAX > 0x7fffffffL
# define FRACTION_COUNT 50000000L
#else
# define FRACTION_COUNT 2L
#endif

#undef MEDIAN_STDC_VERSION
#if defined(__STDC__) && (__STDC__ == 1) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L)
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

/* space-saving */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* maybe volatitle qualifier to preclude excessive optimization */
#undef VOL
#define VOL volatile

/* data types */
/* types capable of holding large values first */
#define DATA_TYPE_STRUCT                       0U /* 0x0000001 */
#define DATA_TYPE_STRING                       1U /* 0x0000002 */
#define DATA_TYPE_POINTER                      2U /* 0x0000004 */
#define DATA_TYPE_DOUBLE                       3U /* 0x0000008 */
#define DATA_TYPE_UINT_LEAST64_T               4U /* 0x0000010 */
#define DATA_TYPE_FLOAT                        5U /* 0x0000020 */ /* not directly used */
#define DATA_TYPE_LONG                         6U /* 0x0000040 */
#define DATA_TYPE_INT                          7U /* 0x0000080 */
#define DATA_TYPE_UINT_LEAST32_T               8U /* 0x0000100 */
#define DATA_TYPE_UINT_LEAST16_T               9U /* 0x0000200 */
#define DATA_TYPE_SHORT                       10U /* 0x0000400 */
#define DATA_TYPE_CHAR                        11U /* 0x0000800 */ /* not used */
#define DATA_TYPE_UINT_LEAST8_T               12U /* 0x0001000 */

#define DATA_TYPE_COUNT                       13U
#if DATA_TYPE_COUNT > 32
# error "DATA_TYPE_COUNT " xbuildstr(DATA_TYPE_COUNT) " is incompatible with 32-bit unsigned integers"
#endif

/* test sequence macros (could be an enum) */
#define TEST_SEQUENCE_STDIN                    0U  /* 0x00000001 */ /* must be first (some loops start at 1 to avoid STDIN) */
#define TEST_SEQUENCE_SORTED                   1U  /* 0x00000002 */
#define TEST_SEQUENCE_REVERSE                  2U  /* 0x00000004 */
#define TEST_SEQUENCE_ORGAN_PIPE               3U  /* 0x00000008 */
#define TEST_SEQUENCE_INVERTED_ORGAN_PIPE      4U  /* 0x00000010 */
#define TEST_SEQUENCE_ROTATED                  5U  /* 0x00000020 */ /* after organ-pipe for graphing */
#define TEST_SEQUENCE_SHIFTED                  6U  /* 0x00000040 */ /* rotated opposite direction */
#define TEST_SEQUENCE_SAWTOOTH                 7U  /* 0x00000080 */ /* 3 distinct values, repeated sequence */
#define TEST_SEQUENCE_TERNARY                  8U  /* 0x00000100 */ /* 3 distinct values, random */
#define TEST_SEQUENCE_BINARY                   9U  /* 0x00000200 */ /* 2 distinct values */
#define TEST_SEQUENCE_CONSTANT                 10U /* 0x00000400 */ /* only 1 value */
#define TEST_SEQUENCE_MANY_EQUAL_LEFT          11U /* 0x00000800 */
#define TEST_SEQUENCE_MANY_EQUAL_MIDDLE        12U /* 0x00001000 */
#define TEST_SEQUENCE_MANY_EQUAL_RIGHT         13U /* 0x00002000 */
#define TEST_SEQUENCE_MANY_EQUAL_SHUFFLED      14U /* 0x00004000 */
#define TEST_SEQUENCE_RANDOM_DISTINCT          15U /* 0x00008000 */
#define TEST_SEQUENCE_RANDOM_VALUES            16U /* 0x00010000 */
#define TEST_SEQUENCE_RANDOM_VALUES_LIMITED    17U /* 0x00020000 */
#define TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED 18U /* 0x00040000 */
#define TEST_SEQUENCE_RANDOM_VALUES_NORMAL     19U /* 0x00080000 */
#define TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL 20U /* 0x00100000 */
#define TEST_SEQUENCE_HISTOGRAM                21U /* 0x00200000 */
#define TEST_SEQUENCE_MEDIAN3KILLER            22U /* 0x00400000 */
#define TEST_SEQUENCE_DUAL_PIVOT_KILLER        23U /* 0x00800000 */
#define TEST_SEQUENCE_JUMBLE                   24U /* 0x01000000 */ /* worst-case swaps for sorting networks */
#define TEST_SEQUENCE_PERMUTATIONS             25U /* 0x02000000 */
#define TEST_SEQUENCE_COMBINATIONS             26U /* 0x04000000 */
#define TEST_SEQUENCE_ADVERSARY                27U /* 0x08000000 */
#define TEST_SEQUENCE_WORST                    28U /* 0x10000000 */

#define TEST_SEQUENCE_COUNT                    29U
#if TEST_SEQUENCE_COUNT > 32
# error "TEST_SEQUENCE_COUNT " xbuildstr(TEST_SEQUENCE_COUNT) " is incompatible with 32-bit unsigned integers"
#endif

#define MAX_PERMUTATION_SIZE                   20  /* 20! is huge */
#define MAX_COMBINATION_SIZE                   ((LONG_BIT)-1) /* avoid sign bit */

#define MAXIMUM_SAWTOOTH_MODULUS               3  /* ensures maximum 3 distinct values */
#define CONSTANT_VALUE                         3  /* a more-or-less random value */

/* macros to unify function calls */
#undef ISORT
#define BMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) bmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define DEDSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) dedsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define DPQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) dpqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define GLQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) glibc_quicksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define HEAPSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) heap_sort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define IBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) ibmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define IMERGESORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) d_indirect_mergesort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define INTROSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) introsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define ISORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) isort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define LOGSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) logsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define MBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) mbmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define MERGESORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) xmergesort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define MMSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) minmaxsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define NBQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) nbqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define NETWORKSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) networksort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define P9QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) p9qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define ILLUMOSQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) illumos_qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define D_QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) d_quickselect((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,options&(QUICKSELECT_USER_OPTIONS_MASK|QUICKSELECT_STRICT_SELECTION|QUICKSELECT_NO_REPIVOT))
#define QSEL(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,options&(QUICKSELECT_USER_OPTIONS_MASK|QUICKSELECT_STRICT_SELECTION|QUICKSELECT_NO_REPIVOT))
#define D_QSEL_S(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) d_quickselect_s((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,NULL,mpk,mku-mkl,options&(QUICKSELECT_USER_OPTIONS_MASK|QUICKSELECT_STRICT_SELECTION|QUICKSELECT_NO_REPIVOT))
#define QSEL_S(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) quickselect_s((void*)((char*)marray+mts*mstart),mend+1UL-mstart,mts,mcf,NULL,mpk,mku-mkl,options&(QUICKSELECT_USER_OPTIONS_MASK|QUICKSELECT_STRICT_SELECTION|QUICKSELECT_NO_REPIVOT))
#define QSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) qsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define RUNSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) runsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define SELSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) selsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SHELLSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) shellsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SMOOTHSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) smoothsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define SQRTSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqrtsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define SQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) sqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)
#define SYSMERGESORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) mergesort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define WBMQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) wbmqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf)
#define WQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) wqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,mpk,mku-mkl,options)
#define YQSORT(marray,mstart,mend,mts,mcf,mpk,mkl,mku,mdbg) yqsort((void *)((char *)marray+mts*mstart),mend+1UL-mstart,mts,mcf,options)

/* IBMQSORT, BMQSORT, WBMQSORT are grouped together for adversary generation */
#define FUNCTION_IBMQSORT             0U
#define FUNCTION_BMQSORT              1U
#define FUNCTION_WBMQSORT             2U /* only for adversary generation */
#define FUNCTION_NBQSORT              3U
#define FUNCTION_MBMQSORT             4U
/* SQSORT, QSELECT, QSELECT_S, QSELECT_SORT, QSORT_WRAPPER, WQSORT are grouped together for adversary generation */
#define FUNCTION_SQSORT               5U
#define FUNCTION_QSELECT_SORT         6U
#define FUNCTION_WQSORT               7U
#define FUNCTION_QSORT_WRAPPER        8U
#define FUNCTION_QSELECT              9U
#define FUNCTION_QSELECT_S           10U
#define FUNCTION_GLQSORT             11U
#define FUNCTION_QSORT               12U
#define FUNCTION_ISORT               13U
#define FUNCTION_SHELLSORT           14U
#define FUNCTION_MINMAXSORT          15U
#define FUNCTION_SELSORT             16U
#define FUNCTION_NETWORKSORT         17U
#define FUNCTION_HEAPSORT            18U
#define FUNCTION_INTROSORT           19U
#define FUNCTION_YQSORT              20U
#define FUNCTION_DPQSORT             21U
#define FUNCTION_SQRTSORT            22U
#define FUNCTION_LOGSORT             23U
#define FUNCTION_SMOOTHSORT          24U
#define FUNCTION_P9QSORT             25U
#define FUNCTION_ILLUMOSQSORT        26U
#define FUNCTION_MERGESORT           27U
#define FUNCTION_DEDSORT             28U
#define FUNCTION_SYSMERGESORT        29U
#define FUNCTION_INDIRECT_MERGESORT  30U
#define FUNCTION_RUNSORT             31U

#define FUNCTION_COUNT               32U
#if FUNCTION_COUNT > 32
# error "FUNCTION_COUNT " xbuildstr(FUNCTION_COUNT) " is incompatible with 32-bit unsigned integers"
#endif

/* type of test */
#define TEST_TYPE_CORRECTNESS  0x01U
#define TEST_TYPE_TIMING       0x02U
#define TEST_TYPE_PARTITION    0x04U
#define TEST_TYPE_SORT         0x08U

#define HISTOGRAM_INDEX1_OFFSET 3

/* options for modified Bentley&McIlroy qsort */
/* Improved sampling quality, Kiwiel Algorithm L, and recursion on the smaller
   partitioned region are always incorporated.
*/
#define MOD_SAMPLE_QUALITY     0x0040U
#define MOD_SAMPLE_QUANTITY    0x0080U
#define MOD_ISORT_LS           0x0100U
#define MOD_ISORT_BS           0x0200U /* overrides MBM_ISORT_LS */
#define MOD_DEDICATED_SORT     0x0400U /* overrides MBM_ISORT_BS, MBM_ISORT_LS */
#define MOD_TERNARY            0x0800U

#define MOD_OPTIONS ((MOD_SAMPLE_QUALITY)|(MOD_SAMPLE_QUANTITY)|(MOD_ISORT_LS)|(MOD_ISORT_BS)|(MOD_DEDICATED_SORT)|(MOD_TERNARY))

/* worst-case for sorting network; force swaps for network sort */
#if SILENCE_WHINEY_COMPILERS /* no ambiguity here... */
# define REVERSE_COMPARE_EXCHANGE(ma,mb,mopts,msize,mswapf,malignsize,  \
   msize_ratio)                                                            \
   if(0>OPT_COMPAR(ma,mb,mopts)) {                                      \
       EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,/**/); }
   /* Programmer's terminating semicolon is now an excess statement.
      And this precludes a following "else".
      Thanks :-/ gcc.
   */
#else
# define REVERSE_COMPARE_EXCHANGE(ma,mb,mopts,msize,mswapf,malignsize,  \
   msize_ratio)                                                            \
   if(0>OPT_COMPAR(ma,mb,mopts))                                        \
       EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,/**/)
#endif

#define RCX(ma,mb) REVERSE_COMPARE_EXCHANGE((ma),(mb),options,size,swapf,alignsize,size_ratio)

/* structure definitions */
struct data_struct {
    /* fields similar to struct tm, but different sizes and offsets */
    /* field sizes compatible with CCSDS 301.0-B-4 sect. 3.4.1.1, 3.4.1.2 CCS */
    /* signed values allow for arithmetic and normalization */
    /* yday and month+mday packed into a union; conversion between
       variations is required.
    */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    int_least16_t year;  /* no (1900) offset */
    union {
        int_least16_t yday;  /* no (1) offset */
        /* xxx.u_var.yday */
        struct {
            int_least8_t  month; /* no (1) offset */
            /* xxx.u_var.s_md.month */
            int_least8_t  mday;
            /* xxx.u_var.s_md.mday */
        } s_md;
    } u_var;
    int_least8_t  hour;
    int_least8_t  minute;
    int_least8_t  second;
    uint_least8_t  fractional[6]; /* actually 12 BCD digits */
    uint_least8_t  preamble[1];
    /* preserve alignment by padding to a multiple of 2 bytes */
    uint_least8_t  pad; /* could be used for wday */
#else
    signed short year; /* no (1900) offset */
    union {
        signed short yday; /* no (1) offset */
        /* xxx.u_var.yday */
        struct {
            signed char  month /* no (1) offset */
            /* xxx.u_var.s_md.month */
            signed char  mday;
            /* xxx.u_var.s_md.mday */
        } s_md;
    } u_var;
    signed char  hour;
    signed char  minute;
    signed char  second;
    unsigned char  fractional[6]; /* actually 12 BCD digits */
    unsigned char  preamble[1];
    /* preserve alignment by padding to a multiple of 2 bytes */
    unsigned char  pad; /* could be used for wday */
#endif /* C99 */
    /* CCSDS 301.0-B-4 sect. 3.5.1.1 ASCII TIME CODE A subset (no fractional seconds) */
    char string[STRING_SIZE]; /* for string comparison */
};

/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
*/
#include "tables.h"

#if defined(__STDC__) && (__STDC__ == 1) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L)
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
extern int use_networks;
extern int use_shell;
extern unsigned int do_histogram;
extern unsigned int introsort_final_insertion_sort;
extern unsigned int save_partial;
extern unsigned int mbm_options;
#if defined(__STDC__) && (__STDC__ == 1) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L)
extern size_t ausz;
#endif /* C99 */
extern size_t dp_cutoff;
extern size_t global_count;
extern size_t global_ratio;
extern size_t global_sz;
extern size_t histogram[8][37];
extern size_t histoswap[8];
extern size_t introsort_recursion_factor;
extern size_t introsort_small_array_cutoff;
extern size_t merge_cutoff;
extern size_t nfrozen, pivot_minrank;
extern size_t quickselect_small_array_cutoff;
extern size_t repivot_cutoff;
extern size_t repivot_factor;
extern size_t selection_nk;
extern size_t sort_threshold;
extern size_t y_cutoff;
extern short *global_sharray;
extern int *global_iarray;
extern long *global_larray;
extern long *input_data;
extern long *global_refarray;
extern float *global_uarray;
extern float *global_sarray;
extern float *global_warray;
extern double *global_darray;
extern struct data_struct **global_parray;
extern struct data_struct *global_data_array;
# if ASSERT_CODE + DEBUG_CODE
extern FILE *logfile;
#endif /* ASSERT_CODE */

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
extern void initialize_antiqsort(size_t n, char *pv, unsigned int type, size_t ratio, const size_t sz, long *alt);
extern size_t antiqsort_ncmp; /* number of comparisons */
extern size_t antiqsort_nsolid; /* number of solid items */

/* bmqsort.c */
extern void bmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *));

/* comparators.c */
extern int (*uninstrumented_comparator(int (*compar)(const void *,const void *,...)))(const void *, const void *,...);
extern const char *comparator_name(int (*compar)(const void *, const void *,...));
extern int (*type_context_comparator(unsigned int type, unsigned char *flags))(const void *, const void *, void *);

/* compare.c */
extern int ilongcmp(const void *p1, const void *p2);
extern int longcmp_s(const void *p1, const void *p2, void *unused);
extern int ilongcmp_s(const void *p1, const void *p2, void *unused);
extern int ishortcmp(const void *p1, const void *p2);
extern int shortcmp_s(const void *p1, const void *p2, void *unused);
extern int ishortcmp_s(const void *p1, const void *p2, void *unused);
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
extern unsigned int correctness_tests(unsigned int sequences, unsigned int functions,
    unsigned int types, unsigned int options, const char *prog, size_t n,
    size_t ratio, size_t count, int col, double timeout, uint_least64_t *s,
    unsigned int p, unsigned int *plast_adv,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags);

/* db.c */
extern int db_compare_double(const long *pindx1, const long *pindx2);
extern int db_compare_int(const long *pindx1, const long *pindx2);
extern int db_compare_long(const long *pindx1, const long *pindx2);
extern int db_compare_short(const long *pindx1, const long *pindx2);
extern int db_compare_string(const long *pindx1, const long *pindx2);
extern int db_compare_struct(const long *pindx1, const long *pindx2);
extern void index_to_path(unsigned long indx, const char *prefix, char *path, unsigned int sz);
extern void initialize_indices(size_t n);
extern void read_text_file(const char *path, char *buf, unsigned int sz);
extern void ref_to_text(long l, unsigned int data_type, char *buf, unsigned int sz);
extern void text_to_datum(const char *buf, unsigned int data_type, void *datum);
extern int (*type_comparator(unsigned int type, unsigned char *flags))(const void *, const void *);
extern void write_database_files(long *p, size_t n, unsigned int data_type);

/* debug.c */
extern const char *debug_name(unsigned int value);
extern const char *pivot_name(int method);
extern const char *repivot_table_name(struct repivot_table_struct *prts);
extern const char *sampling_table_name(struct sampling_table_struct *psts);

/* dual.c */
extern void dpqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

/* duplicate.c */
extern void duplicate_test_data(long *refarray, char *pv, int type, size_t r, size_t o, size_t n);
extern void restore_test_data(size_t r, size_t o, size_t n, long *refarray, char *pv, int type);

/* factorial.c */
extern size_t factorial(size_t n);

/* functions.c */
extern unsigned int ls_bit_number(unsigned int mask);
extern const char *function_type(unsigned int func, unsigned int *ptests);
extern const char *function_name(unsigned int func);
extern size_t type_alignment(unsigned int type);
extern void *type_array(unsigned int type);
extern uint_least64_t type_max(unsigned int type);
extern const char *type_name(unsigned int type);
extern size_t type_size(unsigned int type);

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

/* illumos.c */
extern void illumos_qsort(void *, size_t, size_t, int (*)(const void *, const void *),unsigned int options);

/* introsort.c */
extern void introsort(char *base, size_t nmemb, size_t size, int(*compar)(const void *,const void *),unsigned int options);
extern void wintrosort(char *base, size_t nmemb, size_t size, int(*compar)(const void *,const void *),unsigned int options);

/* is_even.c */
extern int is_even(size_t n);

/* isort.c */
extern void isort_internal(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options);
extern void isort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),
    unsigned int options);

/* logsort.c */
extern void logsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

/* mbmqsort.c */
extern void mbmqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *),unsigned int options);

/* mergesort.c */
extern void runsort(char *base, size_t nmemb, const size_t size,
    int (*compar)(const void *,const void *), unsigned int options);
extern void xmergesort(char *base, size_t nmemb, const size_t size,
    int (*compar)(const void *,const void *), unsigned int options);
extern int d_indirect_mergesort(char *base, size_t nmemb, const size_t size,
    int (*compar)(const void *,const void *), unsigned int options);

/* minmaxsort.c */
extern void minmaxsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),unsigned int options);

/* netbsd.c */
extern void nbqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *));

/* network.c */
extern void networksort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options);
extern void networksort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),
    unsigned int options);

/* operations.c */
extern void op_tests(const char *prefix, const char *prog,
    void (*f)(int, void *, const char *,...), void *log_arg);

/* parse.c */
extern double parse_expr(double n, const char *p, char **pendptr, int base);
extern double parse_num(const char *p, char **pendptr, int base);

/* plan9.c */
extern void p9qsort(void *va, long n, long es, int (*cmp)(const void*, const void*));

/* print.c */
extern void fprint_some_array(FILE *f, char *target, size_t l, size_t u, const char *prefix, const char *suffix,unsigned int options);
extern void print_size_t_array(size_t *target, size_t ratio, size_t l, size_t u, const char *prefix, const char *suffix);
# if ASSERT_CODE + DEBUG_CODE
extern FILE *start_log(const char *path);
extern void close_log(FILE *f);
#endif /* ASSERT_CODE */

/* quickselect.c */
extern int d_quickselect(void *base, size_t nmemb,
    /*const*/ size_t size, int (*compar)(const void *,const void *),
    size_t *pk, size_t nk, unsigned int options);

/* quickselect_s.c */
extern errno_t d_quickselect_s(void *base, rsize_t nmemb, /*const*/ rsize_t size,
    int (*compar)(const void *,const void *,void *), void *context,
    size_t *pk, size_t nk, unsigned int options);
extern errno_t d_qsort_s_internal (void *base, size_t nmemb, /*const*/ size_t size,
    int (*compar)(const void *,const void *, void *), void *context);

/* selsort.c */
extern void selsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* sequences.c */
extern unsigned int sequence_is_randomized(unsigned int sequence);
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
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options);

/* sqrtsort.c */
extern void sqrtsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

/* sqsort.c */
extern void sqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),unsigned int options);

/* swap.c */
extern void (*iswapn(size_t alignsize))(char *, char *, size_t);
#if defined(__STDC__) && (__STDC__ == 1) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L)
extern void swapd(void * restrict p1, void * restrict p2, register size_t size);
extern void iswapd(void * restrict p1, void * restrict p2, register size_t size);
#endif /* C99 */
extern void swapstruct(register struct data_struct *p1, register struct data_struct *p2, register size_t count);
extern void iswapstruct(register struct data_struct *p1, register struct data_struct *p2, register size_t count);

/* test_array_partition.c */
extern size_t test_array_partition(const char *pv, size_t l, size_t pl,
    size_t pu, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    void(*f)(int, void *, const char *, ...), void *log_arg);

/* test_array_sort.c */
extern size_t test_array_sort(const char *pv, size_t l, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    unsigned int distinct, void(*f)(int, void *, const char *, ...),
    void *log_arg);

/* test_size.c */
extern const char *test_size(unsigned int tests);

/* test_factor.c */
extern double test_factor(unsigned int tests, size_t n);

/* timing.c */
extern void reset_counters(unsigned int totals_too);
extern void update_totals(size_t count, void (*f)(int, void *, const char *, ...),
    void *log_arg);
extern void update_counters(size_t size_ratio);
extern void print_counters(const char *comment, int col, size_t n, size_t count,
    double factor, const char *pcc, const char *pfunc, const char *psize,
    const char *typename, const char *ptest, unsigned char *flags,
    size_t size_ratio, void (*f)(int, void *, const char *, ...), void *log_arg);
extern unsigned int timing_tests(unsigned int sequences, unsigned int functions,
    unsigned int types, unsigned int options, const char *prog, size_t n,
    size_t ratio, size_t count, int col, double timeout, uint_least64_t *s,
    unsigned int p, unsigned int *plast_adv,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn);

/* ulsqrt.c */
extern size_t ulsqrt(size_t n);

/* valid.c */
extern int valid_test(unsigned int test_type, unsigned int testnum, size_t n);

/* wqsort.c */
extern void make_adverse(long *base, size_t first, size_t beyond, size_t *pk,
    size_t firstk, size_t beyondk, size_t size_ratio, unsigned int distribution,
    int method, unsigned int options);
extern size_t minimum_remedian_rank(unsigned int table_index);
extern char *freeze_some_samples(register char *base, register size_t first, size_t beyond,
    register size_t size, int (*compar)(const void *, const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    register unsigned int table_index, unsigned int options);
extern void wqsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *), size_t *pk, size_t nk, unsigned int options);

/* yaroslavskiy.c */
extern void yqsort(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *),unsigned int options);

#endif /* MEDIAN_TEST_CONFIG_H_INCLUDED */
