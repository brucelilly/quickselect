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
* $Id: ~|^` @(#)    yaroslavskiy.c copyright 2016-2017 Bruce Lilly.   \ yaroslavskiy.c $
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
/* $Id: ~|^` @(#)   This is yaroslavskiy.c version 1.5 dated 2017-12-15T21:34:10Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.yaroslavskiy.c */

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
#define ID_STRING_PREFIX "$Id: yaroslavskiy.c ~|^` @(#)"
#define SOURCE_MODULE "yaroslavskiy.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2017-12-15T21:34:10Z"
#define COPYRIGHT_HOLDER "V. Yaroslavskiy"
#define COPYRIGHT_DATE "2009"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* Yaroslavskiy's dual-pivot sort */
/* Ref.: Yaroslavskiy, V. "Dual-pivot Quicksort algorithm" September 11, 2009 */
#if 0
/* Yaroslavskiy apparently keeps changing his mind about the magic number 13... */
static size_t y_cutoff2 = 13UL; /* infinite is best */
#endif

/* qsort-like interface, translated from Java-nese,
   magic numbers scaled to tunable y_cutoff (11 works best),
   assertions and protection tests added,
   optimized (initial pivots, cached pivot comparison, busy-work avoidance)
   best available swap
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void yqsort_internal(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), size_t y_div,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    int pcmp;
    char *pg, *pivot1, *pivot2, *pk, *pl, *pr;
    size_t dist, temp;

#define YQSORT_DEBUG 0
#if YQSORT_DEBUG
fprintf(stderr,"%s(%p,%lu,%lu,%p,%lu)\n",__func__,(void *)base,nmemb,size,compar,y_div);
#endif
    pr=base+(nmemb-1UL)*size;
    if (nmemb<y_cutoff) { /* small-array sort */
        if (0!=use_networks)
            networksort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio,options);
        else if (0!=use_shell)
            shellsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio);
        else
#if 0
            isort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio);
#else
            d_dedicated_sort(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio,options);
#endif
/* <- */return; /* Done; */
    }
    /* ideally, pivots should split array into 3 equal-sized regions */
    /* indices should be (nmemb-2UL)/3 and nmemb-1-((nmemb-2UL)/3) */
    A(nmemb>=2UL); A(0UL!=y_div); A(0UL!=size);
#if 0 /* original, unequal spacing */
    temp = (nmemb / y_div) * size;
#else /* improved, more equal spacing */
    temp = ((nmemb-2UL)/y_div) * size;
#endif
    if (0UL==temp) temp++;
    /* "medians", actually pivot elements */
    pivot1=base+temp;
    pivot2=pr-temp;
    pcmp=compar(pivot1,pivot2); /* cached comparison */
    if (0<pcmp) {
        EXCHANGE_SWAP(swapf,base,pivot2,size,alignsize,size_ratio,nsw++);
        EXCHANGE_SWAP(swapf,pivot1,pr,size,alignsize,size_ratio,nsw++);
    } else {
        EXCHANGE_SWAP(swapf,base,pivot1,size,alignsize,size_ratio,nsw++);
        EXCHANGE_SWAP(swapf,pivot2,pr,size,alignsize,size_ratio,nsw++);
    }
    /* pivot elements (moved) */
    pivot1=base, pivot2=pr;
    /* pointers */
    pl=base+size, pg=pr-size;
    /* sorting */
    for (pk=pl; pk<=pg; pk+=size) {
        if (0>compar(pk,pivot1)) {
            if (pk!=pl) {
                EXCHANGE_SWAP(swapf,pk,pl,size,alignsize,size_ratio,nsw++);
            }
            pl+=size;
        } else if (0<compar(pk,pivot2)) {
            while ((pk<pg)&&(0<compar(pg,pivot2))) pg-=size;
            if (pk!=pg) {
                EXCHANGE_SWAP(swapf,pk,pg,size,alignsize,size_ratio,nsw++);
            }
            pg-=size;
            if (0>compar(pk,pivot1)) {
                if (pk!=pl) {
                    EXCHANGE_SWAP(swapf,pk,pl,size,alignsize,size_ratio,nsw++);
                }
                pl+=size;
            }
        }
    }
    npartitions++;
    /* swaps */
    if (pg>pl) dist=(pg-pl)/size; else dist=0UL; /* avoid underflow */
#if 0 /* avoid changing y_div (performance improves w/o change) */
    /* magic number 13 replaced by y_cutoff2 */
    /* dist is the size of the center region (only; ignores other sizes) */
    if (dist < y_cutoff2) y_div++;
#endif
    pivot1=pl-size; /* new location for pivot1 */
    if (pivot1!=base) {
        EXCHANGE_SWAP(swapf,pivot1,base,size,alignsize,size_ratio,nsw++);
    }
    pivot2=pg+size; /* new location for pivot2 */
    if (pivot2!=pr) {
        EXCHANGE_SWAP(swapf,pivot2,pr,size,alignsize,size_ratio,nsw++);
    }
    /* subarrays */
    temp=(pivot1-base)/size; /* save size for protection comparison & recursion */
    A((2UL>temp)||(base+(temp-1UL)*size<=pr));
    if (1UL<temp) {
        nrecursions++;
        yqsort_internal(base,temp,size,compar,y_div,swapf,alignsize,size_ratio,
            options);
    }
    temp=(pr-pivot2)/size; /* save size for protection comparison and recursion */
    A((2UL>temp)||(pivot2+temp*size<=pr));
    if (1UL<temp) {
        nrecursions++;
        yqsort_internal(pivot2+size,temp,size,compar,y_div,swapf,alignsize,
            size_ratio,options);
    }
    /* equal elements */
    /* cheap (cached) condition first */
    if ((0!=pcmp)
#if 0 /* always separate (partition) equals if pivots differ */
    /* magic number 13 replaced by y_cutoff2 */
    &&(y_cutoff2>nmemb-dist) /* avoid overflow/underflow */
#endif
    ) { /* avoid overflow/underflow */
        npartitions++;
        for (pk=pl; pk<=pg; pk+=size) {
            if (0==compar(pk,pivot1)) {
                if (pk!=pl) {
                    EXCHANGE_SWAP(swapf,pk,pl,size,alignsize,size_ratio,nsw++);
                }
                pl+=size;
            } else if (0==compar(pk,pivot2)) {
                if (pk!=pg) {
                    EXCHANGE_SWAP(swapf,pk,pg,size,alignsize,size_ratio,nsw++);
                }
                pg-=size;
                if (0==compar(pk,pivot1)) {
                    if (pk!=pl) {
                        EXCHANGE_SWAP(swapf,pk,pl,size,alignsize,size_ratio,
                            nsw++);
                    }
                    pl+=size;
                }
            }
        }
        /* revised "dist" */
        if (pg>pl) dist=(pg-pl)/size; else dist=0UL; /* avoid underflow */
    }
    /* subarray */
    A(pl+dist*size<=pr);
    /* avoid unnecessary work for all-equal region; cached condition first */
    if ((0!=pcmp)&&(0UL<dist)) {
        nrecursions++;
        yqsort_internal(pl,dist+1UL,size,compar,y_div,swapf,alignsize,
            size_ratio,options);
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void yqsort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), unsigned int options)
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    if (4UL>y_cutoff) y_cutoff=4UL; /* require po<pivot1<pivot2<pr */
    yqsort_internal(base,nmemb,size,compar,3UL,swapf,alignsize,size_ratio,
        options);
}
/* ************************************************************** */
