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
* $Id: ~|^` @(#)    logsort.c copyright 2016-2019 Bruce Lilly.   \ logsort.c $
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
/* $Id: ~|^` @(#)   This is logsort.c version 1.13 dated 2019-03-16T15:37:11Z. \ $ */
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
#define MODULE_VERSION "1.13"
#define MODULE_DATE "2019-03-16T15:37:11Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define LIBMEDIAN_TEST_CODE 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#include "indirect.h" /* floor_lg */

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

static
QUICKSELECT_INLINE
void logsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    size_t nmemb=beyond-first;
    A(first<beyond);
    if (8UL<nmemb) {
        size_t k, o, p, s, xnk=floor_lg(nmemb-5UL)-1UL;
        size_t xpk[xnk];

        s=nmemb/xnk;
        o=first+(s>>1);
        for (k=0UL,p=o; k<xnk; k++,p+=s) {
            xpk[k] = p;
#if LIBMEDIAN_TEST_CODE
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
        (V)d_quickselect_loop(base,first,beyond,size,compar,xpk,0UL,xnk,
            swapf,alignsize,size_ratio,quickselect_cache_size,0UL,
            options,NULL,NULL);
        /* xnk+1 regions partitioned by xpk ranks; recursively sort them. */
/* XXX for selection (future), check for desired order statistic ranks for each region */
        nrecursions++;
        logsort_internal(base,first,xpk[0UL],size,compar,swapf,alignsize,
            size_ratio,options);
        for (k=0UL,--xnk; k<xnk; k++) {
            A(xpk[k]+1UL<xpk[k+1UL]);
#if LIBMEDIAN_TEST_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: xpk[%lu]=%lu, xpk[%lu]=%lu */"
                "\n",__func__,source_file,__LINE__,k,xpk[k],k+1UL,xpk[k+1UL]);
#endif
            nrecursions++;
            logsort_internal(base,xpk[k]+1UL,xpk[k+1UL],size,compar,swapf,
                alignsize,size_ratio,options);
        }
        nrecursions++;
        logsort_internal(base,xpk[k]+1UL,beyond,size,compar,swapf,alignsize,
            size_ratio,options);
    } else {
        /* Handle divide-and-conquer for <= 8 elements */
        (V)d_quickselect_loop(base,first,beyond,size,compar,NULL,0UL,0UL,
            swapf,alignsize,size_ratio,quickselect_cache_size,0UL,
            options,NULL,NULL);
    }
}

void logsort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), unsigned int options)
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    logsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        options);
}
