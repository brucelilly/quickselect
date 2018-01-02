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
* $Id: ~|^` @(#)    sqsort.c copyright 2016-2017 Bruce Lilly.   \ sqsort.c $
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
/* $Id: ~|^` @(#)   This is sqsort.c version 1.5 dated 2017-12-06T23:17:47Z. \ $ */
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
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2017-12-06T23:17:47Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* simplified and full-blown quickselect-based qsort:
   Like modified B&M qsort, except:
      add sampling and remedian of samples, with sampling table
*/

/* simplified quickselect */
static void sqsort_internal(void *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, size_t cutoff,
    unsigned int options, unsigned int table_index)
{
    char *pc, *pd, *pe, *pf, *pivot;
    size_t nmemb, p, q, r, s;

    for (;;) {
        nmemb=beyond-first;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, table_index=%u */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,table_index);
#endif
        if (nmemb<=cutoff) {
            if (2UL<=nmemb)
                d_dedicated_sort(base,first,beyond,size,compar,swapf,alignsize,
                    size_ratio,options);
    /* <- */return; /* Done; */
        }
#if ASSERT_CODE
        if (nmemb>sorting_sampling_table[table_index].max_nmemb)
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, table_index=%u, sorting_sampling_table[%u].max_nmemb=%lu */\n",
                __func__,source_file,__LINE__,nmemb,table_index,table_index,
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
            size_ratio,table_index,NULL,options,&pc,&pd,&pe,&pf);
        pivot_minrank=nmemb;
        /* no support for efficient stable sorting */
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,swapf,
            alignsize,size_ratio,options,NULL,NULL,NULL,&q,&p);
        s=q-first;
        if (beyond>p) r=beyond-p; else r=0UL;  /* size of the > region */
        if (s<r) { /* > region is larger */
            if (1UL<s) {
                unsigned int idx=table_index;
                while ((0U<idx)&&(s<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                sqsort_internal(base,first,q,size,compar,swapf,alignsize,
                    size_ratio,cutoff,options,idx);
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
                    size_ratio,cutoff,options,idx);
            }
            if (2UL>s) return;
            beyond=q, nmemb=s;
        }
        if (cutoff<nmemb)
            while ((0U<table_index)
            &&(nmemb<=sorting_sampling_table[table_index-1U].max_nmemb)
            )
                table_index--;
    }
}

void sqsort(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *), unsigned int options)
{
    unsigned int table_index=2U; /* optimized for small nmemb */
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);
    size_t cutoff;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((0U<table_index)
    &&(nmemb<=sorting_sampling_table[table_index-1U].max_nmemb)
    )
        table_index--;
    while (nmemb>sorting_sampling_table[table_index].max_nmemb)
        table_index++;

    if (0UL!=quickselect_small_array_cutoff)
        cutoff=quickselect_small_array_cutoff;
    else cutoff=cutoff_value(size_ratio,options);

    nfrozen=0UL, pivot_minrank=nmemb;
    sqsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        cutoff,options,table_index);
}
