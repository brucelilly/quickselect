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
* $Id: ~|^` @(#)    sqsort.c copyright 2016-2018 Bruce Lilly.   \ sqsort.c $
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
/* $Id: ~|^` @(#)   This is sqsort.c version 1.15 dated 2018-05-10T02:22:27Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "sqsort" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.sqsort.c */

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
#define ID_STRING_PREFIX "$Id: median_test.c ~|^` @(#)"
#define SOURCE_MODULE "sqsort.c"
#define MODULE_VERSION "1.15"
#define MODULE_DATE "2018-05-10T02:22:27Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define QUICKSELECT_LOOP d_quickselect_loop

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* dedicated_sort[_s] declaration */
#if ! defined(DEDICATED_SORT_DECLARED)
static
# include "dedicated_sort_decl.h"
;
# define DEDICATED_SORT_DECLARED 1
#endif /* DEDICATED_SORT_DECLARED */

#include "dedicated_sort_src.h"

#include "pivot_src.h"

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

/* simplified and full-blown quickselect-based qsort:
   Like modified B&M qsort, except:
      add sampling and remedian of samples, with sampling table
*/

/* simplified quickselect */
static void sqsort_internal(void *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, size_t pbeyond, unsigned int options,
    unsigned int table_index)
{
    char *pc, *pd, *pe, *pf, *pivot;
    size_t nmemb, p, q, r, s;

    for (;;) {
        nmemb=beyond-first;
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb="
                "%lu, table_index=%u */\n",__func__,source_file,__LINE__,first,
                beyond,nmemb,table_index);
#endif
        if (2UL<=nmemb) {
            /* Special-case (not used in production code) to permit disabling or
               limiting dedicated_sort for small sub-arrays; no dedicated_sort
               if nmemb is above quickselect_small_array_cutoff.
            */
            if (nmemb<=quickselect_small_array_cutoff) {
                int ret= DEDICATED_SORT(base,first,beyond,size,COMPAR_ARGS,
                    swapf,alignsize,size_ratio,table_index,
                    quickselect_cache_size,0UL,options);
                switch (ret) {
                    case 0 : /* sorting is complete */
    /* <- */        return; /* Done; */
                    case EINVAL : /* error */
#if ASSERT_CODE + DEBUG_CODE
                        fflush(stderr); fflush(stdout);
                        (V)fprintf(stderr,
                            "/* %s: line %d: EINVAL return %d from "
                            "dedicated_sort */\n",
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
                            "dedicated_sort */\n",
                            __func__,__LINE__,ret);
                        A(ret==0);
#endif
                    return ;
                }
            }
        } else return; /* Done because a single element is a sorted array. */
        table_index=d_sample_index(sorting_sampling_table,table_index,nmemb);
#if ASSERT_CODE
        if (nmemb>sorting_sampling_table[table_index].max_nmemb)
            (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, table_index=%u, "
                "sorting_sampling_table[%u].max_nmemb=%lu */\n",__func__,
                source_file,__LINE__,nmemb,table_index,table_index,
                sorting_sampling_table[table_index].max_nmemb);
        A(sorting_sampling_table[table_index].max_nmemb>=nmemb);
        A((0U==table_index)
        ||(sorting_sampling_table[table_index-1UL].max_nmemb<nmemb));
#endif
        /* freeze low-address samples which will be used for pivot selection */
        if (aqcmp==compar)
            (V)freeze_some_samples(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,table_index,options);
        pivot=d_select_pivot(base,first,beyond,size,compar,swapf,alignsize,
            size_ratio,table_index,NULL,quickselect_cache_size,pbeyond,options,
            &pc,&pd,&pe,&pf);
        pivot_minrank=nmemb;
        /* no support for efficient stable sorting */
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,swapf,
            alignsize,size_ratio,quickselect_cache_size,options,&q,&p);
        s=q-first;
        if (beyond>p) r=beyond-p; else r=0UL;  /* size of the > region */
        if (s<r) { /* > region is larger */
            if (1UL<s) {
                unsigned int idx=table_index;
                while ((0U<idx)&&(s<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                sqsort_internal(base,first,q,size,compar,swapf,alignsize,
                    size_ratio,pbeyond,options,idx);
            }
            if (2UL>r) return;
            first=p, nmemb=r;
        } else { /* < region is larger, or regions are the same size */
            if (1UL<r) {
                unsigned int idx=table_index;
                while ((0U<idx)&&(r<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                sqsort_internal(base,p,beyond,size,compar,swapf,alignsize,
                    size_ratio,pbeyond,options,idx);
            }
            if (2UL>s) return;
            beyond=q, nmemb=s;
        }
        while ((0U<table_index)
        &&(nmemb<=sorting_sampling_table[table_index-1U].max_nmemb)
        )
            table_index--;
    }
}

void sqsort(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *), unsigned int options)
{
    unsigned int table_index;
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

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
    options |= QUICKSELECT_NO_REPIVOT;
    sqsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        0UL,options,table_index);
}
