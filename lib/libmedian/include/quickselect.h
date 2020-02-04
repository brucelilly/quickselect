/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining function quickselect
*/
#ifndef	QUICKSELECT_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    quickselect.h copyright 2016-2020 Bruce Lilly. \ quickselect.h $
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
/* $Id: ~|^` @(#)   This is quickselect.h version 1.31 dated 2020-02-03T15:32:08Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect.h */

/* version-controlled header file version information */
#define QUICKSELECT_H_VERSION "quickselect.h 1.31 2020-02-03T15:32:08Z"

/* FreeBSD requires feature test macros to properly define type errno_t */
#ifndef __STDC_WANT_LIB_EXT1__
# define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <stddef.h>             /* size_t (maybe rsize_t [N1570 K3.3]) */
/* preliminary support for 9899:201x draft N1570 */
#if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 201001L )
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
# include <errno.h>             /* (maybe errno_t [N1570 K3.2]) */
# include <stdint.h>            /* (maybe RSIZE_MAX [N1570 K3.4]) */
# include <stdio.h>             /* (maybe errno_t rsize_t [N1570 K3.5]) */
# include <stdlib.h>            /* (maybe errno_t rsize_t constraint_handler_t
                                   [N1570 K3.6]) */
#endif /* N1570 */

/* separated from __STDC_VERSION__ test to permit testing in pre-C11 context */
#if __STDC_WANT_LIB_EXT1__
  /* C11 RSIZE_MAX, errno_t, rsize_t, constraint_handler_t are presumed to be
     all or none
  */
# ifndef RSIZE_MAX /* provide definitions if lacking in compilation environment */
    typedef int errno_t;
    typedef size_t rsize_t;
    typedef void(*constraint_handler_t)(const char *
#  if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 201001L )
        restrict
#  endif
        msg,
        void *
#  if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 201001L )
       	restrict
#  endif
       	ptr, errno_t error);
#  if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )
#   include <stdint.h>           /* (header not standardized pre-C99) SIZE_MAX */
#  endif /* C99 or later */
#  ifndef SIZE_MAX /* not standardized pre-C99 */
#  include <limits.h>           /* ULONG_MAX */
#   define SIZE_MAX ULONG_MAX
#  endif /* SIZE_MAX */
#  define RSIZE_MAX ((SIZE_MAX)>>1)
#  define QUICKSELECT_PROVIDE_HANDLER 1 /* provide static implementation */
#  define QUICKSELECT_HANDLER_PROVIDED 0
# endif /* RSIZE_MAX */
#endif /* __STDC_WANT_LIB_EXT1__ */

/* qsort wrapper function name */
#ifndef QSORT_FUNCTION_NAME
# define QSORT_FUNCTION_NAME qsort_wrapper
#endif

/* quickselect option and processing flag bitmap values */

  /* User-accessible (if supported at compile-time) options: */
  /* non-stable vs. stable partial ordering */
    /* faster (but not stable) if bit is not set */
  /* Quickselect can sort or select preserving partial order stability. However,
     code size approximately doubles, and there is a significant run-time cost.
     Partial order stability is a non-issue for scalar data, and multivariate
     data is most efficiently sorted using multivariate comparisons.  It is
     therefore recommended to define QUICKSELECT_STABLE as 0 (zero) for general-
     purpose use.
  */
  /* if QUICKSELECT_STABLE is pre-defined as zero, the option will be
     unavailable and object code size will be reduced
  */
#ifndef QUICKSELECT_STABLE
# define QUICKSELECT_STABLE              0x01U
#endif
/* sanity check for override */
#if ((QUICKSELECT_STABLE) != 0) && ((QUICKSELECT_STABLE) != 0x01U)
# error "unreasonable definition for QUICKSELECT_STABLE"
#endif

  /* minimize comparisons vs. minimize run-time for simple comparisons */
    /* optimize for speed with simple comparisons if bit is not set */
    /* in-place mergesort for small sub-arrays uses fewer comparisons on
       average than insertion sort (even with binary search), but is recursive
       and moves more data on average
    */
#define QUICKSELECT_OPTIMIZE_COMPARISONS 0x02U

  /* Indirect sorting via an array of pointers to elements. */
  /* Indirection (i.e. initially moving pointers to the data instead of the data
     elements, followed by a rearrangement of the data elements after their
     final positions have been determined) is used internally for modest-sizes
     arrays of large elements (when moving pointers is less costly than moving
     data elements) and is available for all arrays as a user-visible option in
     quickselect.  There is no guarantee of partial order stability for qsort.
  */
#define QUICKSELECT_INDIRECT             0x04U /* visible, used internally */

  /* Internal use flags (possibly subject to compile-time definition): */

  /* restrict pivot rank by using all array elements for pivot selection */
    /* use only a sample of array elements for pivot selection if not set */
    /* This flag is used by the ``break-glass'' mechanism to avoid performance
       degradation from linearithmic to polynomial with adverse inputs.  The
       ``break-glass'' mechanism uses little additional code and is critial for
       preventing poor performance; therefore it is not subject to being
       disabled at compile-time.
    */
#define QUICKSELECT_RESTRICT_RANK        0x08U /* internal */

/* mask for user-accessible option bits (don't allow caller-supplied options
   to interfere with internal flags); internal flags are high order bits
*/
#define QUICKSELECT_INTERNAL_FLAGS 0xFFFFFFF8U
#define QUICKSELECT_USER_OPTIONS_MASK (( QUICKSELECT_STABLE \
        | QUICKSELECT_OPTIMIZE_COMPARISONS | QUICKSELECT_INDIRECT ) \
        & ~( QUICKSELECT_INTERNAL_FLAGS ))
/* end of option bits */

/* defined values for pivot_method */
  /* no data movement */
#define QUICKSELECT_PIVOT_REMEDIAN_SAMPLES  1
#define QUICKSELECT_PIVOT_REMEDIAN_FULL     2
  /* alters data order (cannot be used for stable sort/selection) */
#define QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES 3
#define QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS 4

/* defined values for partition_method */
  /* partial order not preserved (cannot be used for stable sort/selection) */
#define QUICKSELECT_PARTITION_FAST   0
  /* partial order preserved */
#define QUICKSELECT_PARTITION_STABLE 1

/* function declarations */
#if defined(__cplusplus)
# define QUICKSELECT_EXTERN extern "C"
#else
# define QUICKSELECT_EXTERN extern
#endif

/* options.c */
QUICKSELECT_EXTERN unsigned int quickselect_options(void);

/* quickselect.c */
QUICKSELECT_EXTERN int quickselect(/*const*/ void *, size_t, /*const*/ size_t,
    int (*)(const void *, const void *),
    /*const*/ size_t /*const*/ *, /*const*/ size_t, unsigned int);
QUICKSELECT_EXTERN void QSORT_FUNCTION_NAME(/*const*/ void *, size_t,
    /*const*/ size_t, int (*)(const void *, const void *));
#if __STDC_WANT_LIB_EXT1__
# ifndef QSORT_S_FUNCTION_NAME
#  define QSORT_S_FUNCTION_NAME qsort_s_wrapper
# endif
QUICKSELECT_EXTERN errno_t quickselect_s(/*const*/ void *, rsize_t,
    /*const*/ rsize_t, int (*)(const void *, const void *, void *),
    /*const*/ void *, /*const*/ size_t /*const*/ *, /*const*/ size_t,
    unsigned int);
QUICKSELECT_EXTERN errno_t QSORT_S_FUNCTION_NAME(/*const*/ void *, rsize_t,
    /*const*/ rsize_t, int (*)(const void *, const void *, void *),
    /*const*/ void *);
/* QSORT_S_FUNCTION_NAME remains defined for use in quickselect.c */
#endif

/* QSORT_FUNCTION_NAME and QSORT_S_FUNCTION_NAME remain defined for use in
   quickselect.c
*/

/* data structures (needed for function argument declarations) */
/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
   McGeoch & Tygar, in "Optimal Sampling Strategies for Quicksort" also
   suggest use of a table, and determine the optimal sample size as proportional
   to the square root of the array size.
*/
struct sampling_table_struct {
    size_t max_nmemb;    /* smallest sub-array for this number of samples */
    size_t samples;      /* the number of samples used for pivot selection */
};

/* quickselect_loop and variants are always externally visible. Support
   functions might be static inline (for speed) or external functions (to
   save space).
*/
#if LIBMEDIAN_TEST_CODE
# define QUICKSELECT_D_QUICKSELECT_LOOP int d_quickselect_loop (char *base, size_t first, size_t beyond, const size_t size, int(*compar)(const void *,const void *), const size_t *pk, size_t firstk, size_t beyondk, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, size_t pbeyond, unsigned int options, char **ppeq, char **ppgt)
QUICKSELECT_EXTERN QUICKSELECT_D_QUICKSELECT_LOOP ;
#endif
#define QUICKSELECT_QUICKSELECT_LOOP int quickselect_loop (char *base, size_t first, size_t beyond, const size_t size, int(*compar)(const void *,const void *), const size_t *pk, size_t firstk, size_t beyondk, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, size_t pbeyond, unsigned int options, char **ppeq, char **ppgt)
QUICKSELECT_EXTERN QUICKSELECT_QUICKSELECT_LOOP ;
#if __STDC_WANT_LIB_EXT1__
# if LIBMEDIAN_TEST_CODE
# define QUICKSELECT_QUICKSELECT_LOOP_S errno_t d_quickselect_loop_s (char *base, size_t first, size_t beyond, const size_t size, int(*compar)(const void *,const void *,void *), void *context, const size_t *pk, size_t firstk, size_t beyondk, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, size_t pbeyond, unsigned int options, char **ppeq, char **ppgt)
# else
# define QUICKSELECT_QUICKSELECT_LOOP_S errno_t quickselect_loop_s (char *base, size_t first, size_t beyond, const size_t size, int(*compar)(const void *,const void *,void *), void *context, const size_t *pk, size_t firstk, size_t beyondk, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, size_t pbeyond, unsigned int options, char **ppeq, char **ppgt)
# endif
QUICKSELECT_EXTERN QUICKSELECT_QUICKSELECT_LOOP_S ;
#endif

/* The default for user code calls is to declare suport functions as extern.
   Internal functions might sometimes use static or static inline declarations.
   QUICKSELECT_STATIC and QUICKSELECT_INLINE are defined in quickselect_config.h
      as included by library code.
   LIBMEDIAN_TEST_CODE is defined (in library source files) for
      debugging-enabled variants of functions.
*/
#ifndef QUICKSELECT_STATIC
# define QUICKSELECT_STATIC extern
#endif
#ifndef QUICKSELECT_INLINE
# define QUICKSELECT_INLINE /* */
#endif

/* externally-visible support function declarations */
/* invariant functions (no d_* or *_s variants) */
#define QUICKSELECT_FLOOR_LOG3 QUICKSELECT_STATIC QUICKSELECT_INLINE size_t floor_log3(register size_t n)
QUICKSELECT_FLOOR_LOG3 ;
/* special cases: qualifiers and function name depend on where code body is included */
#ifndef QUICKSELECT_FMED3_STATIC
# define QUICKSELECT_FMED3_STATIC extern
#endif
#ifndef QUICKSELECT_FMED3_INLINE
# define QUICKSELECT_FMED3_INLINE /* */
#endif
#define QUICKSELECT_FMED3 QUICKSELECT_FMED3_STATIC QUICKSELECT_FMED3_INLINE char *fmed3(register /*const*/char *pa, register /*const*/char *pb, register /*const*/char *pc, int(*compar)(const void *,const void *), unsigned int options)
QUICKSELECT_FMED3 ;
#if __STDC_WANT_LIB_EXT1__
# define QUICKSELECT_FMED3_S QUICKSELECT_FMED3_STATIC QUICKSELECT_FMED3_INLINE char *fmed3_s(register /*const*/char *pa, register /*const*/char *pb, register /*const*/char *pc, int(*compar)(const void *,const void *,const void *),const void *context, unsigned int options)
QUICKSELECT_FMED3_S ;
#endif
/* d_* variants when building libmedian_test objects */
#if LIBMEDIAN_TEST_CODE
# define QUICKSELECT_DEDICATED_SORT QUICKSELECT_STATIC QUICKSELECT_INLINE int d_dedicated_sort (char *base, size_t first, size_t beyond, /*const*/ size_t size, int(*compar)(const void *,const void *), void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cache_sz, size_t pbeyond, unsigned int options)
# define QUICKSELECT_FIND_MINMAX QUICKSELECT_STATIC QUICKSELECT_INLINE void d_find_minmax (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *), unsigned int options, char **pmn, char **pmx)
# define QUICKSELECT_KLIMITS QUICKSELECT_STATIC QUICKSELECT_INLINE void d_klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk, size_t beyondk, size_t *pfk, size_t *pbk)
# define QUICKSELECT_PARTITION QUICKSELECT_STATIC QUICKSELECT_INLINE void d_partition (char *base, size_t first, size_t beyond, char *pc, char *pd, char *pivot, char *pe, char *pf, size_t size, int(*compar)(const void *,const void *), void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, unsigned int options, size_t *peq, size_t *pgt)
# define QUICKSELECT_SAMPLE_INDEX QUICKSELECT_STATIC QUICKSELECT_INLINE unsigned int d_sample_index (const struct sampling_table_struct *psts, unsigned int idx, size_t nmemb)
# define QUICKSELECT_SAMPLES QUICKSELECT_STATIC QUICKSELECT_INLINE size_t d_samples (size_t nmemb, int method, unsigned int distribution, unsigned int options)
# define QUICKSELECT_SAMPLING_TABLE QUICKSELECT_STATIC QUICKSELECT_INLINE unsigned int d_sampling_table (size_t first, size_t beyond, const size_t *pk, size_t firstk, size_t beyondk, char **ppeq, const size_t **ppk, size_t nmemb, size_t size_ratio, int method, unsigned int options)
# define QUICKSELECT_SELECT_MAX QUICKSELECT_STATIC QUICKSELECT_INLINE void d_select_max (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *),void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MIN QUICKSELECT_STATIC QUICKSELECT_INLINE void d_select_min (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *),void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MINMAX QUICKSELECT_STATIC QUICKSELECT_INLINE void d_select_minmax (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *),void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options)
# define QUICKSELECT_SELECT_PIVOT QUICKSELECT_STATIC QUICKSELECT_INLINE char *d_select_pivot (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *), void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int distribution, const size_t *pk, size_t firstk, size_t beyondk, size_t cachesz, size_t pbeyond, unsigned int options, char **ppc, char **ppd, char **ppe, char **ppf, int *pmethod, size_t *psamples)
# define QUICKSELECT_SHOULD_REPIVOT QUICKSELECT_STATIC QUICKSELECT_INLINE unsigned int d_should_repivot (const size_t nmemb, const size_t n, const size_t samples, int method, unsigned int options, const size_t *pk, int *pn2)
# define QUICKSELECT_SIZE_T_SQRT QUICKSELECT_STATIC QUICKSELECT_INLINE size_t d_size_t_sqrt (size_t n)
#else
# define QUICKSELECT_DEDICATED_SORT QUICKSELECT_STATIC QUICKSELECT_INLINE int dedicated_sort (char *base, size_t first, size_t beyond, /*const*/ size_t size, int(*compar)(const void *,const void *), void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cache_sz, size_t pbeyond, unsigned int options)
# define QUICKSELECT_FIND_MINMAX QUICKSELECT_STATIC QUICKSELECT_INLINE void find_minmax (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *), unsigned int options, char **pmn, char **pmx)
# define QUICKSELECT_KLIMITS QUICKSELECT_STATIC QUICKSELECT_INLINE void klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk, size_t beyondk, size_t *pfk, size_t *pbk)
# define QUICKSELECT_PARTITION QUICKSELECT_STATIC QUICKSELECT_INLINE void partition (char *base, size_t first, size_t beyond, char *pc, char *pd, char *pivot, char *pe, char *pf, size_t size, int(*compar)(const void *,const void *), void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, unsigned int options, size_t *peq, size_t *pgt)
# define QUICKSELECT_SAMPLE_INDEX QUICKSELECT_STATIC QUICKSELECT_INLINE unsigned int sample_index (const struct sampling_table_struct *psts, unsigned int idx, size_t nmemb)
# define QUICKSELECT_SAMPLES QUICKSELECT_STATIC QUICKSELECT_INLINE size_t samples (size_t nmemb, int method, unsigned int distribution, unsigned int options)
# define QUICKSELECT_SAMPLING_TABLE QUICKSELECT_STATIC QUICKSELECT_INLINE unsigned int sampling_table (size_t first, size_t beyond, const size_t *pk, size_t firstk, size_t beyondk, char **ppeq, const size_t **ppk, size_t nmemb, size_t size_ratio, int method, unsigned int options)
# define QUICKSELECT_SELECT_MAX QUICKSELECT_STATIC QUICKSELECT_INLINE void select_max (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *),void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MIN QUICKSELECT_STATIC QUICKSELECT_INLINE void select_min (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *),void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MINMAX QUICKSELECT_STATIC QUICKSELECT_INLINE void select_minmax (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *),void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options)
# define QUICKSELECT_SELECT_PIVOT QUICKSELECT_STATIC QUICKSELECT_INLINE char *select_pivot (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *), void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int distribution, const size_t *pk, size_t firstk, size_t beyondk, size_t cachesz, size_t pbeyond, unsigned int options, char **ppc, char **ppd, char **ppe, char **ppf, int *pmethod, size_t *psamples)
# define QUICKSELECT_SHOULD_REPIVOT QUICKSELECT_STATIC QUICKSELECT_INLINE unsigned int should_repivot (const size_t nmemb, const size_t n, const size_t samples, int method, unsigned int options, const size_t *pk, int *pn2)
# define QUICKSELECT_SIZE_T_SQRT QUICKSELECT_STATIC QUICKSELECT_INLINE size_t size_t_sqrt (size_t n)
#endif
QUICKSELECT_DEDICATED_SORT ;
QUICKSELECT_FIND_MINMAX ;
QUICKSELECT_KLIMITS ;
QUICKSELECT_PARTITION ;
QUICKSELECT_PIVOT_METHOD ;
QUICKSELECT_SAMPLE_INDEX ;
QUICKSELECT_SAMPLES ;
QUICKSELECT_SAMPLING_TABLE ;
QUICKSELECT_SELECT_MAX ;
QUICKSELECT_SELECT_MIN ;
QUICKSELECT_SELECT_MINMAX ;
QUICKSELECT_SELECT_PIVOT ;
QUICKSELECT_SIZE_T_SQRT ;
/* additional support function declarations for qsort_s et al */
#if __STDC_WANT_LIB_EXT1__
# if LIBMEDIAN_TEST_CODE
#  define QUICKSELECT_DEDICATED_SORT_S QUICKSELECT_STATIC errno_t d_dedicated_sort_s (char *base, size_t first, size_t beyond, /*const*/ size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cache_sz, size_t pbeyond, unsigned int options)
#  define QUICKSELECT_FIND_MINMAX_S QUICKSELECT_STATIC QUICKSELECT_INLINE void d_find_minmax_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, unsigned int options, char **pmn, char **pmx)
# define QUICKSELECT_PARTITION_S QUICKSELECT_STATIC QUICKSELECT_INLINE void d_partition_s (char *base, size_t first, size_t beyond, char *pc, char *pd, char *pivot, char *pe, char *pf, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, unsigned int options, size_t *peq, size_t *pgt)
# define QUICKSELECT_SELECT_MAX_S QUICKSELECT_STATIC QUICKSELECT_INLINE void d_select_max_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MIN_S QUICKSELECT_STATIC QUICKSELECT_INLINE void d_select_min_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MINMAX_S QUICKSELECT_STATIC QUICKSELECT_INLINE void d_select_minmax_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options)
# define QUICKSELECT_SELECT_PIVOT_S QUICKSELECT_STATIC QUICKSELECT_INLINE char *d_select_pivot_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int distribution, const size_t *pk, size_t firstk, size_t beyondk, size_t cachesz, size_t pbeyond, unsigned int options, char **ppc, char **ppd, char **ppe, char **ppf, int *pmethod, size_t *psamples)
# else
#  define QUICKSELECT_DEDICATED_SORT_S QUICKSELECT_STATIC errno_t dedicated_sort_s (char *base, size_t first, size_t beyond, /*const*/ size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cache_sz, size_t pbeyond, unsigned int options)
#  define QUICKSELECT_FIND_MINMAX_S QUICKSELECT_STATIC QUICKSELECT_INLINE void find_minmax_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, unsigned int options, char **pmn, char **pmx)
# define QUICKSELECT_PARTITION_S QUICKSELECT_STATIC QUICKSELECT_INLINE void partition_s (char *base, size_t first, size_t beyond, char *pc, char *pd, char *pivot, char *pe, char *pf, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, size_t cachesz, unsigned int options, size_t *peq, size_t *pgt)
# define QUICKSELECT_SELECT_MAX_S QUICKSELECT_STATIC QUICKSELECT_INLINE void select_max_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MIN_S QUICKSELECT_STATIC QUICKSELECT_INLINE void select_min_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
# define QUICKSELECT_SELECT_MINMAX_S QUICKSELECT_STATIC QUICKSELECT_INLINE void select_minmax_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int options)
# define QUICKSELECT_SELECT_PIVOT_S QUICKSELECT_STATIC QUICKSELECT_INLINE char *select_pivot_s (char *base, size_t first, size_t beyond, size_t size, int(*compar)(const void *,const void *,const void *),const void *context, void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio, unsigned int distribution, const size_t *pk, size_t firstk, size_t beyondk, size_t cachesz, size_t pbeyond, unsigned int options, char **ppc, char **ppd, char **ppe, char **ppf, int *pmethod, size_t *psamples)
# endif
QUICKSELECT_DEDICATED_SORT_S ;
QUICKSELECT_FIND_MINMAX_S ;
QUICKSELECT_PARTITION_S ;
QUICKSELECT_SELECT_MAX_S ;
QUICKSELECT_SELECT_MIN_S ;
QUICKSELECT_SELECT_MINMAX_S ;
QUICKSELECT_SELECT_PIVOT_S ;
#endif

#define	QUICKSELECT_H_INCLUDED 1
#endif
