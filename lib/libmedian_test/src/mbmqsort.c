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
* $Id: ~|^` @(#)    mbmqsort.c copyright 2016-2019 Bruce Lilly.   \ mbmqsort.c $
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
/* $Id: ~|^` @(#)   This is mbmqsort.c version 1.24 dated 2019-04-18T02:02:40Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.mbmqsort.c */

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
#define ID_STRING_PREFIX "$Id: mbmqsort.c ~|^` @(#)"
#define SOURCE_MODULE "mbmqsort.c"
#define MODULE_VERSION "1.24"
#define MODULE_DATE "2019-04-18T02:02:40Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */

#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#define LIBMEDIAN_TEST_CODE 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#include "dedicated_sort_src.h"

#if QUICKSELECT_BUILD_FOR_SPEED
# include "pivot_src.h"
#endif

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

/* Modified Bentley&McIlroy qsort:
   Very little original Bentley&McIlroy code
   Performance modifications to qsort implementation:
      ternary median-of-3
      improved sampling quality for median-of-3, ninther
      Kiwiel's Algorithm L to prevent self-swapping during partitioning
      no single-sample pivot selection (disastrous for some inputs)
      improved sampling quantity beyond ninther via remedian of samples,
         table-based sample size adjusted according to array size
      recurse on small region, iterate large region
      avoid self-swapping (in d_partition)
      improved swapping sizes (via swapf, determined in wrapper function)
      don't repeat initialization during iteration (wrapper function only)
      uses hybrid of network sort or in-place merge sort or indirect mergesort
         instead of inefficient insertion sort for small-array sorting
      options provide limited support for stable sorting and internal
         indirection (neither fully tested, and probably not working);
         support for minimizing the number of comparisons (e.g. for cases
         where comparisons are known to be expensive) is tested and functional
      cache-aware to efficiently support options and small-array sorting
   No break-glass mechanism to prevent quadratic behavior
   No order statistic selection
   Limitations on support for partial order stability and internal indirection
      (see above)
   The purpose is to verify performance enhancement w/o break-glass or selection
      resulting from the modifications above (primarily improved sampling
      quality, improved small-array sorting, and increased sample quantity for
      large arrays).  Sampling quality and Kiwiel's Algorithm L improvements and
      recursion on the smaller partitioned region first and size-related swap
      function determined once and ternary median-of-3 are always incorporated.
      Improved sampling quantity and replacement of linear-search waltzing
      insertion sort by linear-search insertion sort with insertion by rotation,
      binary-search insertion sort (with same or increased cutoff) or by
      dedicated_sort using optimal sorting networks, binary-search insertion
      sort, in-place mergesort, and indirect mergesort can be enabled by setting
      appropriate bits in the options argument.
*/

/* base array doesn't change (unlike Bentley&McIlroy's qsort); first and beyond
   indices delimit the region to be sorted
*/
static void mbmqsort_internal(char *base, size_t first, size_t beyond,
    size_t size, int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t pbeyond,
    unsigned int options)
{
    int method = QUICKSELECT_PIVOT_REMEDIAN_SAMPLES;
    size_t nmemb=beyond-first;
    size_t cache_limit, lneq=0UL, lnne=0UL;
    if (size>sizeof(char*)) /* direct sorting in data cache */
        cache_limit=(quickselect_cache_size<<1)/size/3UL; /* 1.5*size within cache */
    else /* indirect mergesort */
        cache_limit=quickselect_cache_size/pointer_and_a_half; /* 1.5 pointers within cache */
    for (;;) {
        char *pc, *pd, *pe, *pf, *pm;
        size_t p, q;

#if LIBMEDIAN_TEST_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,
               "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, size=%lu, "
               "options=0x%x */\n",__func__,source_file,
               __LINE__,first,beyond,nmemb,size,options);
#endif
        /* Multiple tests take some time, but only the comparison count is of
           interest here, not run-time.  Run-time can be (and is) optimized in
           quickselect, which incorporates all of the improvements.
        */
        if ((0U!=(options&(MOD_DEDICATED_SORT))) 
            && ((5UL>nmemb) /* small sub-array; network no worse than best-case D&C */
/* stable and optimize comparison methods only work within cache limits */
            ||((nmemb<=cache_limit)&&
              ((0U!=(options&(QUICKSELECT_STABLE))) /* stable methods */
              ||(0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS))) /* minimize comparisons */
              ||(
#if 0
              (nel>nmemb)&& /* XXX not useful unless nel is a parameter set by wrapper (or internal stack is used instead of recursion) */
              /* not first partition; D&C 1st time in case constant or binary */
              /* can only determine non-constant, non-binary, non-ternary after first partition */
#endif
              (lneq<=(lnne>>2)))) /* not constant, binary, ternary */
              )
            )) {
            int ret;
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG))
                (V)fprintf(stderr,
                   "/* %s: %s line %d: d_dedicated_sort: first=%lu, beyond=%lu, "
                   "size=%lu, options=0x%x */\n",__func__,
                   source_file,__LINE__,first,beyond,size,options);
#endif
            ret= d_dedicated_sort(base,first,beyond,size,compar,
                swapf,alignsize,size_ratio,quickselect_cache_size,0UL,
                options);
            switch (ret) {
                case 0 : /* sorting is complete */
    /* <- */    return; /* Done; */
                case EINVAL : /* error */
#if ASSERT_CODE + DEBUG_CODE
                    fflush(stderr); fflush(stdout);
                    (V)fprintf(stderr,
                        "/* %s: line %d: EINVAL return %d from "
                        "d_dedicated_sort */\n",
                        __func__,__LINE__,ret);
                    A(ret!=EINVAL);
#endif
                return ;
                case EAGAIN : /* continue with divide-and-conquer */
                break;
                default : /* ? */
#if ASSERT_CODE + DEBUG_CODE
                    fflush(stderr); fflush(stdout);
                    (V)fprintf(stderr,
                        "/* %s: line %d: unexpected return %d from "
                        "d_dedicated_sort */\n",
                        __func__,__LINE__,ret);
                    A(ret==0);
#endif
                return ;
            }
        } else if (0U!=(options&(MOD_ISORT_BS))) {
#if 0
            if (8UL>nmemb) /* cutoff > Bentley&McIlroy; isort_bs @ size 7 */
                       /* reversed, rotated, shifted inputs get better,
                          bitonic inputs (esp. inverted organ-pipe) get worse
                       */
#else
            if (7UL>nmemb) /* same cutoff but median-of-3 @ size 7 */
                       /* reversed input now same as already-sorted */
#endif
            {
#if LIBMEDIAN_TEST_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG))
                    (V)fprintf(stderr,
                       "/* %s: %s line %d: d_isort_bs: first=%lu, beyond=%lu, "
                       "size=%lu, options=0x%x */\n",__func__,
                       source_file,__LINE__,first,beyond,size,options);
#endif
#if LIBMEDIAN_TEST_CODE
                d_isort_bs
#else
                isort_bs
#endif
                (base,first,beyond,size,compar,swapf,alignsize,size_ratio,
                    options);
                return; /* done; else continue divide-and-conquer */
            }
        } else if (0U!=(options&(MOD_ISORT_LS))) {
            if (BM_INSERTION_CUTOFF>nmemb) {
                /* use median-of-3 pivot selection @ size 7 */
#if LIBMEDIAN_TEST_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG))
                    (V)fprintf(stderr,
                       "/* %s: %s line %d: d_isort_ls: first=%lu, beyond=%lu, "
                       "size=%lu, options=0x%x */\n",__func__,
                       source_file,__LINE__,first,beyond,size,options);
#endif
#if LIBMEDIAN_TEST_CODE
                d_isort_ls
#else
                isort_ls
#endif
                (base,first,beyond,size,compar,swapf,alignsize,size_ratio,
                    options);
                return; /* done; else continue divide-and-conquer */
            }
        } else if (BM_INSERTION_CUTOFF>nmemb) {
            /* uses median-of-3 pivot selection @ size 7; never single sample */
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG))
                (V)fprintf(stderr,
                   "/* %s: %s line %d: inline waltzing insertion sort: nmemb="
                   "%lu, size=%lu, options=0x%x */\n",
                   __func__,source_file,__LINE__,nmemb,size,options);
#endif
            /* This snippet mostly copied from Bentley&McIlroy's code, with
               necessary variable name changes and swap function replacement.
               Performance is therefore expected to be similar to
               Bentley&McIlroy's qsort; specifically, the number of comparisons
               should be identical, and the number of swaps will be lower for
               types with size and alignment different from long or char.
            */
            for (pm = base + first*size; pm < base + beyond*size; pm += size)
                for (pc = pm; pc > base && compar(pc-size, pc) > 0; pc -= size)
                    EXCHANGE_SWAP(swapf,pc,pc-size,size,alignsize,size_ratio,
                        nsw++);
            return; /* done; else continue divide-and-conquer */
        }
#if LIBMEDIAN_TEST_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,
               "/* %s: %s line %d: divide-and-conquer: first=%lu, beyond=%lu, "
               "nmemb=%lu, size=%lu, options=0x%x */\n",
               __func__,source_file,__LINE__,first,beyond,nmemb,size,options);
#endif
        pm=
#if LIBMEDIAN_TEST_CODE
            d_select_pivot
#else
            select_pivot
#endif
            (base,first,beyond,size,compar,swapf,alignsize,
            size_ratio,0U,NULL,0UL,0UL,quickselect_cache_size,pbeyond,
            options,&pc,&pd,&pe,&pf,&method,NULL);
        /* no provision for efficient stable sorting */
        d_partition(base,first,beyond,pc,pd,pm,pe,pf,size,compar,swapf,
            alignsize,size_ratio,quickselect_cache_size,options,&q,&p);
        lneq=p-q; lnne=nmemb-lneq;
        if (q-first<beyond-p) { /* > region is larger */
            if (first+1UL<q) { /* at least 2 elements */
                unsigned int idx=table_index;
                while ((0U<idx)
                &&(q-first<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                mbmqsort_internal(base,first,q,size,compar,swapf,alignsize,
                    size_ratio,idx,pbeyond,options);
            }
            if (2UL>beyond-p) return;
            first=p;
        } else { /* < region is larger, or regions are the same size */
            if (p+1UL<beyond) { /* at least 2 elements */
                unsigned int idx=table_index;
                while ((0U<idx)
                &&(beyond-p<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                mbmqsort_internal(base,p,beyond,size,compar,swapf,alignsize,size_ratio,
                    idx,pbeyond,options);
            }
            if (2UL>q-first) return;
            beyond=q;
        }
        nmemb=beyond-first;
        while ((0U<table_index)
        &&(nmemb<=sorting_sampling_table[table_index-1U].max_nmemb)
        )
            table_index--;
    }
}

void mbmqsort(void *a, size_t n, size_t es,
    int (*compar)(const void *, const void *),unsigned int options)
{
    unsigned int table_index;
    size_t alignsize=alignment_size((char *)a,es);
    size_t size_ratio=es/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    assert(a != NULL || n == 0 || es == 0);
    assert(compar != NULL);

#if LIBMEDIAN_TEST_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stderr,
           "/* %s: %s line %d: n=%lu, es=%lu, options=0x%x "
           "*/\n",__func__,source_file,__LINE__,
           n,es,options);
#endif

    table_index=n<=
#if ( SIZE_MAX < 65535 )
# error "SIZE_MAX < 65535 [C11 draft N1570 7.20.3]"
#elif ( SIZE_MAX == 65535 ) /* 16 bits */
        sorting_sampling_table[2].max_nmemb?1UL:3UL
#elif ( SIZE_MAX == 4294967295 ) /* 32 bits */
        sorting_sampling_table[5].max_nmemb?2UL:7UL
#elif ( SIZE_MAX == 18446744073709551615UL ) /* 64 bits */
        sorting_sampling_table[10].max_nmemb?5UL:15UL
#else
# error "strange SIZE_MAX " SIZE_MAX
#endif /* word size */
    ; /* starting point; refined by sample_index() */
    table_index=d_sample_index(sorting_sampling_table,table_index,n);

    nfrozen=0UL, pivot_minrank=n;
    options |= QUICKSELECT_NO_REPIVOT;
    mbmqsort_internal(a,0UL,n,es,compar,swapf,alignsize,size_ratio,
        table_index,0UL,options);
}
