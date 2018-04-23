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
* $Id: ~|^` @(#)    logsort.c copyright 2016-2018 Bruce Lilly.   \ logsort.c $
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
/* $Id: ~|^` @(#)   This is logsort.c version 1.10 dated 2018-04-23T05:16:06Z. \ $ */
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
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2018-04-23T05:16:06Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define QUICKSELECT_LOOP d_quickselect_loop

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#include "indirect.h" /* floor_lg */

/* quickselect_loop declaration */
#if ! defined(QUICKSELECT_LOOP_DECLARED)
QUICKSELECT_EXTERN
# include "quickselect_loop_decl.h"
;
# define QUICKSELECT_LOOP_DECLARED 1
#endif /* QUICKSELECT_LOOP_DECLARED */

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

static
QUICKSELECT_INLINE
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

        s=nmemb/xnk;
        o=first+(s>>1);
        for (k=0UL,p=o; k<xnk; k++,p+=s) {
            xpk[k] = p;
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: xpk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,k,p);
#endif
        }
#if ASSERT_CODE
        if ((xpk[0]<=first)||(xpk[xnk-1UL]>=beyond-1UL))
            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, xnk="
                "%lu, o=%lu, xpk[0]=%lu, xpk[%lu]=%lu */\n",__func__,
                source_file,__LINE__,first,beyond,xnk,o,xpk[0],xnk-1UL,
                xpk[xnk-1UL]);
#endif
        A(xpk[0]>first);A(xpk[xnk-1UL]<beyond-1UL);

        /* efficient stable partition is not supported */
        (V)QUICKSELECT_LOOP(base,first,beyond,size,compar,xpk,0UL,xnk,
            swapf,alignsize,size_ratio,table_index,quickselect_cache_size,0UL,
            options,NULL,NULL);
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
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: xpk[%lu]=%lu, xpk[%lu]=%lu */"
                "\n",__func__,source_file,__LINE__,k,xpk[k],k+1UL,xpk[k+1UL]);
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
        /* Handle divide-and-conquer for <= 8 elements */
        /* table index very small (probably zero) for <= 8 elements;
           dedicated sort may be used, so don't waste time caculating
           table_index
        */
        (V)QUICKSELECT_LOOP(base,first,beyond,size,COMPAR_ARGS,NULL,0UL,0UL,
            swapf,alignsize,size_ratio,table_index,quickselect_cache_size,0UL,
            options,NULL,NULL);
    }
}

void logsort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), unsigned int options)
{
    unsigned int table_index;
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    table_index=(nmemb<=
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
    ); /* to be refined... */

    logsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        options,table_index);
}
