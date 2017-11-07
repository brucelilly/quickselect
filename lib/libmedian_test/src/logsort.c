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
* $Id: ~|^` @(#)    logsort.c copyright 2016-2017 Bruce Lilly.   \ logsort.c $
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
/* $Id: ~|^` @(#)   This is logsort.c version 1.3 dated 2017-11-03T19:33:29Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.logsort.c */

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
#define ID_STRING_PREFIX "$Id: logsort.c ~|^` @(#)"
#define SOURCE_MODULE "logsort.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2017-11-03T19:33:29Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void logsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, unsigned int table_index)
{
    size_t nmemb=beyond-first;
    A(first<beyond);
    if (8UL<nmemb) {
        size_t k, o, p, s, xnk=floor_lg(nmemb-5UL)-1UL;
        size_t xpk[xnk];
        unsigned int idx;
        size_t cutoff=cutoff_value(size_ratio,options);

        s=nmemb/xnk;
        o=first+(s>>1);
        for (k=0UL,p=o; k<xnk; k++,p+=s) {
            xpk[k] = p;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s: %s line %d: xpk[%lu]=%lu */\n",
__func__,source_file,__LINE__,k,p);
#endif
        }
#if ASSERT_CODE
if ((xpk[0]<=first)||(xpk[xnk-1UL]>=beyond-1UL))
(V)fprintf(stderr, "/* %s: %s line %d: first=%lu, beyond=%lu, xnk=%lu, o=%lu, xpk[0]=%lu, xpk[%lu]=%lu */\n",
__func__,source_file,__LINE__,first,beyond,xnk,o,xpk[0],xnk-1UL,xpk[xnk-1UL]);
#endif
        A(xpk[0]>first);A(xpk[xnk-1UL]<beyond-1UL);

        d_quickselect_loop(base,first,beyond,size,compar,xpk,0UL,xnk,
            swapf,alignsize,size_ratio,cutoff,options,NULL,NULL);
        /* xnk+1 regions partitioned by xpk ranks; recursively sort them. */
/* XXX for selection (future), check for desired order statistic ranks for each region */
        for (idx=table_index;
        (0U<idx)&&(o>=sorting_sampling_table[idx-1U].max_nmemb);
        idx--)
            ;
        nrecursions++;
        logsort_internal(base,first,xpk[0UL],size,compar,swapf,alignsize,
            size_ratio,options,idx);
        for (idx++,s--;
        (0U<idx)&&(s>=sorting_sampling_table[idx-1U].max_nmemb);
        idx--)
            ;
        for (k=0UL,--xnk; k<xnk; k++) {
            A(xpk[k]+1UL<xpk[k+1UL]);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s: %s line %d: xpk[%lu]=%lu, xpk[%lu]=%lu */\n",
__func__,source_file,__LINE__,k,xpk[k],k+1UL,xpk[k+1UL]);
#endif
            nrecursions++;
            logsort_internal(base,xpk[k]+1UL,xpk[k+1UL],size,compar,swapf,
                alignsize,size_ratio,options,idx);
        }
        for (p=beyond-xpk[k]-1UL;
        (0U<idx)&&(p>=sorting_sampling_table[idx-1U].max_nmemb);
        idx--)
            ;
        nrecursions++;
        logsort_internal(base,xpk[k]+1UL,beyond,size,compar,swapf,alignsize,
            size_ratio,options,idx);
    } else {
        if (0!=use_networks)
            networksort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,options);
        else if (0!=use_shell)
            shellsort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
        else
#if 0
            isort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
#else
            dedicated_sort(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,options);
#endif
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void logsort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), unsigned int options)
{
    unsigned int table_index=2U; /* optimized for small nmemb */
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

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
    A(table_index<SAMPLING_TABLE_SIZE);

    logsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        options,table_index);
}