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
* $Id: ~|^` @(#)    mbmqsort.c copyright 2016-2017 Bruce Lilly.   \ mbmqsort.c $
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
/* $Id: ~|^` @(#)   This is mbmqsort.c version 1.4 dated 2017-11-03T19:59:07Z. \ $ */
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
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-11-03T19:59:07Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* Modified Bentley&McIlroy qsort:
      ternary median-of-3
      optional type-independent deferred pivot swap
      improved sampling quality for median-of-3, ninther
      recurse on small region, iterate large region
      avoid self-swapping pb,pc
      improved swapping sizes
      don't repeat initialization during iteration
      uses quickselect_small_array_cutoff rather than BM_INSERTION_CUTOFF
      uses hybrid of network sort or insertion sort or in-place merge sort
      options support stable sorting
   Maybe no remedian of samples (except equivalent to ninther)
   No break-glass mechanism to prevent quadratic behavior
   No order statistic selection
*/
#define NO_REMEDIAN 0

static void mbmqsort_internal(char *a, size_t n, size_t es,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, size_t cutoff, unsigned int table_index,
    unsigned int options)
{

    for (;;) {
        char *pb, *pc, *pd, *pe, *pf, *pm;
        size_t p, q, r;

        if (n <= cutoff) {
            dedicated_sort(a,0UL,n,es,compar,swapf,alignsize,size_ratio,options);
            return;
        }
        /* freeze low-address samples which will be used for pivot selection */
        if (aqcmp==compar) {
            (V)freeze_some_samples(a,0UL,n,es,compar,swapf,alignsize,
                size_ratio,table_index,options);
        }
        pm=d_select_pivot(a,0UL,n,es,compar,swapf,alignsize,size_ratio,
            table_index,NULL,options,&pc,&pd,&pe,&pf);
        pivot_minrank=n;
        d_partition(a,0UL,n,pc,pd,pm,pe,pf,es,compar,swapf,alignsize,size_ratio,
            options,&q,&p);
        pe=a+p*es;
        if (n>p) r=n-p; else r=0UL;  /* size of the > region */
        if (q<r) { /* > region is larger */
            if (1UL<q) {
                unsigned int idx=table_index;
                while ((0U<idx)&&(q<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                mbmqsort_internal(a,q,es,compar,swapf,alignsize,size_ratio,
                    cutoff,idx,options);
            }
            if (2UL>r) return;
            a=pe;
            n=r;
        } else { /* < region is larger, or regions are the same size */
            if (1UL<r) {
                unsigned int idx=table_index;
                while ((0U<idx)&&(r<=sorting_sampling_table[idx-1U].max_nmemb))
                    idx--;
                nrecursions++;
                mbmqsort_internal(pe,r,es,compar,swapf,alignsize,size_ratio,
                    cutoff,idx,options);
            }
            if (2UL>q) return;
            n=q;
        }
        if (cutoff<n)
            while ((0U<table_index)
            &&(n<=sorting_sampling_table[table_index-1U].max_nmemb)
            )
                table_index--;
    }
}

void mbmqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *),unsigned int options)
{
    unsigned int table_index=2U; /* optimized for small nmemb */
    size_t alignsize=alignment_size((char *)a,es);
    size_t cutoff;
    size_t size_ratio=es/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    assert(a != NULL || n == 0 || es == 0);
    assert(compar != NULL);

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((0U<table_index)
    &&(n<=sorting_sampling_table[table_index-1U].max_nmemb)
    )
        table_index--;
    while ((n>sorting_sampling_table[table_index].max_nmemb)
#if NO_REMEDIAN
    &&(9UL>sorting_sampling_table[table_index].samples) /* max. 9 samples */
#endif
    )
        table_index++;

    if (0UL!=quickselect_small_array_cutoff)
        cutoff=quickselect_small_array_cutoff;
    else cutoff=cutoff_value(size_ratio,options);

    nfrozen=0UL, pivot_minrank=n;
    mbmqsort_internal(a,n,es,compar,swapf,alignsize,size_ratio,cutoff,
        table_index,options);
}
