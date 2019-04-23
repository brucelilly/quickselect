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
* $Id: ~|^` @(#)    introsort.c copyright 2016-2019 Bruce Lilly.   \ introsort.c $
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
/* $Id: ~|^` @(#)   This is introsort.c version 1.23 dated 2019-04-18T02:02:40Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.introsort.c */

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
#define ID_STRING_PREFIX "$Id: introsort.c ~|^` @(#)"
#define SOURCE_MODULE "introsort.c"
#define MODULE_VERSION "1.23"
#define MODULE_DATE "2019-04-18T02:02:40Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define __STDC_WANT_LIB_EXT1__ 0
#define LIBMEDIAN_TEST_CODE 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#include "indirect.h" /* floor_lg */

#include "insertion_sort_src.h" /* inline code: isort_ls */

#if QUICKSELECT_BUILD_FOR_SPEED
# include "pivot_src.h"
#endif

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

static
QUICKSELECT_INLINE
void introsort_loop(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int table_index, size_t cache_sz, size_t depth_limit,
    size_t pbeyond, unsigned int options)
{
    size_t eq, gt, s;
    char *pc, *pd, *pe, *pf, *pivot;
    int method = QUICKSELECT_PIVOT_REMEDIAN_SAMPLES;

    A(beyond>first);
    A(3UL<introsort_small_array_cutoff);
    while (beyond-first>introsort_small_array_cutoff) {
        if (0UL==depth_limit) {
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG))
                (V)fprintf(stderr,
                    "/* %s: %s line %d: recursion depth limit reached */\n",
                    __func__,source_file,__LINE__);
#endif
            nfrozen=0UL, pivot_minrank=beyond-first;
            heap_sort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
            return;
        }
        depth_limit--;
        /* Adaptive uniformly-spaced sampling for pivot selection as in
           quickselect.
        */
        /* freeze low-address samples which will be used for pivot selection */
        if (aqcmp==compar)
            (V)freeze_some_samples(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,table_index,options);
        pivot=
#if LIBMEDIAN_TEST_CODE
            d_select_pivot
#else
            select_pivot
#endif
            (base,first,beyond,size,compar,swapf,
            alignsize,size_ratio,0U,NULL,0UL,0UL,cache_sz,pbeyond,
            options,&pc,&pd,&pe,&pf,&method,NULL);
        pivot_minrank=beyond-first;
        /* partition is modified Bentley&McIlroy split-end partition returning
           indices of equal-to and greater-than region starts.  This efficient
           partition function also avoids further processing of elements
           comparing equal to the pivot.
        */
        /* Stable sorting not supported (heapsort is not stable) */
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,swapf,
            alignsize,size_ratio,cache_sz,options,&eq,&gt);
        /* This processes the > region recursively, < region iteratively as
           described by Musser.  Could process smaller region recursively,
           larger iteratively, but Musser says it's not necessary due to
           recursion depth limit.
        */
        s=beyond-gt;
        if (1UL<s) {
            unsigned int idx=table_index;
            while ((0U<idx)&&(s<=sorting_sampling_table[idx-1U].max_nmemb))
                idx--;
            nrecursions++;
            introsort_loop(base,gt,beyond,size,compar,swapf,alignsize,
                size_ratio,idx,cache_sz,depth_limit,pbeyond,options);
        }
        beyond=eq;
        if (first+1UL<beyond) {
            while ((0U<table_index)
            &&(beyond-first<=sorting_sampling_table[table_index-1U].max_nmemb)
            )
                table_index--;
        }
    }
    if (0U==introsort_final_insertion_sort) {
        nfrozen=0UL, pivot_minrank=beyond-first;
        if (0!=use_networks)
            networksort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,options);
        else if (0!=use_shell)
            shellsort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
        else if (first+1UL<beyond)
            /* d_quickselect_loop (for the small array sizes used here) probably
               calls dedicated_sort, which uses the best sorting method for the
               array size and options.
            */
            (V)d_quickselect_loop(base,first,beyond,size,compar,NULL,0UL,0UL,
                swapf,alignsize,size_ratio,cache_sz,pbeyond,options,NULL,NULL);
    }
}

void introsort(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *), unsigned int options)
{
    size_t depth_limit;
    void (*swapf)(char *, char *, size_t);
    size_t alignsize, size_ratio;
    unsigned int table_index;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    depth_limit=introsort_recursion_factor*floor_lg(nmemb);

    table_index=nmemb<=
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

    nfrozen=0UL, pivot_minrank=nmemb;
    introsort_loop(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        table_index,quickselect_cache_size,depth_limit,0UL,options);
    if (0U!=introsort_final_insertion_sort) {
#if LIBMEDIAN_TEST_CODE
        d_isort_ls
#else
        isort_ls
#endif
        (base,0UL,nmemb,size,compar,swapf,alignsize,
            size_ratio,options);
    }
}
