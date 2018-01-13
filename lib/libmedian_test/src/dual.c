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
* $Id: ~|^` @(#)    dual.c copyright 2016-2018 Bruce Lilly.   \ dual.c $
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
/* $Id: ~|^` @(#)   This is dual.c version 1.9 dated 2018-01-13T02:47:57Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.dual.c */

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
#define ID_STRING_PREFIX "$Id: dual.c ~|^` @(#)"
#define SOURCE_MODULE "dual.c"
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2018-01-13T02:47:57Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* Partition array starting at base with nmemb elements of size size around
   2 pivots at pivot1, pivot2 (0>=compar(pivot1,pivot2), pivot1!=pivot2) using
   comparison function compar and swap function swapf, observing options.  Cost
   is 5/3 * nmemb - 2 comparisons.  On return *ppivot1 points to the first
   element comparing equal to pivot1, *pmid points to the first element greater
   than pivot1 and less than pivot2, *ppivot2 points to the first element
   comparing equal to pivot2, and *pgt points to the first element greater than
   pivot2.
*/
static QUICKSELECT_INLINE void dp_partition(char *base, size_t nmemb,
    size_t size, int(*compar)(const void*,const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, char **ppivot1, char **pmid, char **ppivot2,
    char **pgt)
{
    char *pa, *pb, *pc, *pd, *pe;
    int c, d;

    /* at entry: */
    /* +-----------------------------------------------+ */
    /* |        ?     |p1|       ?        |p2|    ?    | */
    /* +-----------------------------------------------+ */
    /*  base                                             */
    /* N.B. order of p1,p2 may be reversed */
    /* initialization: */
    /* +-----------------------------------------------+ */
    /* |p1|                     ?                   |p2| */
    /* +-----------------------------------------------+ */
    /*  b  a                                         d  e*/
    /*                                               c   */
    /* intermediate: */
    /* +-----------------------------------------------+ */
    /* |<p1|=p1|     ?                 |p1<x<p2|=p2|>p2| */
    /* +-----------------------------------------------+ */
    /*      b   a                       c       d   e    */
    /* final: */
    /* +-----------------------------------------------+ */
    /* |    <p1    | =p1 |  p1<x<p2  | =p2 |    >p2    | */
    /* +-----------------------------------------------+ */
    /*  base        b     c           d     e             */
    /* Initialize */
    if (*ppivot1!=base) {
        EXCHANGE_SWAP(swapf,base,*ppivot1,size,alignsize,size_ratio,nsw++);
        if (*ppivot2==base) *ppivot2=*ppivot1; /* it moved */
    }
    pc=pd=(pe=base+size*nmemb)-size;
    if (*ppivot2!=pd) {
        EXCHANGE_SWAP(swapf,*ppivot2,pd,size,alignsize,size_ratio,nsw++);
    }
    pa=(pb=base)+size;
    /* partition element at pa while pa<pc */
    while (pa<pc) {
/* XXX indirection is not necessarily handled correctly here! */
        c=OPT_COMPAR(pa,pb,options,foo); /* pb has same value as pivot1 */
        if (0>c) { /* element at pa is < pivot1 */
            EXCHANGE_SWAP(swapf,pb,pa,size,alignsize,size_ratio,nsw++);
            pa+=size,pb+=size;
        } else if (0==c) { /* element at pa compares equal to pivot1 */
            pa+=size;
        } else { /* element at pa is greater than pivot1 */
            d=OPT_COMPAR(pa,pd,options,foo); /* pd has same value as pivot2 */
            if (0>d) { /* pivot1 < element at pa < pivot2 */
                pc-=size;
                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
            } else if (0==d) { /* element at pa compares equal to pivot2 */
                pc-=size;
                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                pd-=size;
                EXCHANGE_SWAP(swapf,pc,pd,size,alignsize,size_ratio,nsw++);
            } else { /* element at pa > pivot2 */
                pc-=size;
                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                pd-=size;
                EXCHANGE_SWAP(swapf,pc,pd,size,alignsize,size_ratio,nsw++);
                pe-=size;
                EXCHANGE_SWAP(swapf,pd,pe,size,alignsize,size_ratio,nsw++);
            }
        }
    }
    *ppivot1=pb,*pmid=pc,*ppivot2=pd,*pgt=pe;
}

/* dual-pivot qsort */
/* Expected number of comparisons is 5/3 N log3 N ~ 1.05155 N log2 N.  */
static QUICKSELECT_INLINE void dpqsort_internal(char *base, size_t nmemb,
    size_t size, int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    char *pa, *pivot1, *pivot2, *ps, *pu;
    size_t q, r, s, u, karray[2];

#define DPDEBUG 1
/* SAMPLE_SELECTION 0: use sort of 5 elements; 1: use quickselect to obtain pivots from desired ranks */
#define SAMPLE_SELECTION 1
/* ADAPTIVE_SAMPLING 0: always use 5 samples; 1: use ~sqrt(nmemb) samples */
#define ADAPTIVE_SAMPLING 1
/* sanity */
#if SAMPLE_SELECTION == 0
# undef ADAPTIVE_SAMPLING
# define ADAPTIVE_SAMPLING SAMPLE_SELECTION
#endif
#if ADAPTIVE_SAMPLING != 0
# undef SAMPLE_SELECTION
# define SAMPLE_SELECTION ADAPTIVE_SAMPLING
#endif
    for (;;) {
        pu=base+(nmemb-1UL)*size; /* last element */
        if (nmemb<=dp_cutoff) { /* small-array sort */
            nfrozen=0UL, pivot_minrank=nmemb;
            if (0!=use_networks) networksort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,options);
            else if (0!=use_shell) shellsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio);
            else
#if 0
                isort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio);
#else
                d_dedicated_sort(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,options);
#endif
    /* <- */return; /* Done; */
        }
#if ADAPTIVE_SAMPLING
        /* number of samples for pivot selection */
        /* Variable r represents the number of samples in each of the 3
           partitioned regions (if all goes perfectly) and affects the total
           number of samples s=3r+2. 0-based pivot ranks within the s samples
           are at k and 2k+1.
        */
        /* Variable u is a multiplier for some power of the number of
           samples and affects the array size q where the number of samples
           is increased.
        */
        u=4UL;
        /* Samples proportional to square root, cube root, or fourth root of
           the number of elements.
        */
#define SAMPLE_ROOT 2
#if SAMPLE_ROOT == 4
        for (r=1UL,s=5UL,q=u*625UL; q<nmemb; r++,s+=3UL,q=u*s*s*s*s) ;
#elif SAMPLE_ROOT == 3
        for (r=1UL,s=5UL,q=u*125UL; q<nmemb; r++,s+=3UL,q=u*s*s*s) ;
#else
        for (r=1UL,s=5UL,q=u*25UL; q<nmemb; r++,s+=3UL,q=u*s*s) ;
#endif
        if (q>nmemb) r--,s--;
        if (s<5UL) r=1UL,s=5UL;
        if (DEBUGGING(DPQSORT_DEBUG)) {
            (V)fprintf(stderr,"%s: %s line %d: %lu elements, %lu samples, r=%lu\n",__func__,source_file,__LINE__,nmemb,s,r);
        }
#else
        s=5UL;
#endif
        pu+=size; /* past last element */
#if SAMPLE_SELECTION
# if 1
        karray[0]=r;
        karray[1]=(r<<1)+1UL;
        if (DEBUGGING(DPQSORT_DEBUG)) {
            (V)fprintf(stderr,"%s: %s line %d: karray[0]=%lu, karray[1]=%lu\n",__func__,source_file,__LINE__,karray[0],karray[1]);
        }
# else
        /* Aumuller & Dietzfelbinger suggest s/2 and s/4 */
        /* but it's slower in practice */
        q=r>>1;
        karray[0]=q;
        karray[1]=r;
# endif
        /* swap sample elements to beginning of array for selection */
        /* XXX quick hack */
        for (pa=base,ps=base+r*size,q=0UL; q<s; pa+=size,ps+=r*size,q++) {
            if (pa!=ps) {
                swapf(pa,ps,size);
                nsw+=size;
            }
        }
        pivot1=base+karray[0]*size;
        pivot2=base+karray[1]*size;
        /* Selecting 2 pivots partitions the samples around the pivots and
           costs ~ 3s.  Subsequent partitioning of the array costs 5/3 n.
           Using full array indices (rather than relative to first sample)
           and selecting the pivots based on those ranks from the entire
           array would partition the array around the pivots w/o a separate
           partitioning step, with total cost ~ 3n.
           That would be an extension of the basic quicksort divide-and-conquer
           method, extended to multiple pivots.  Instead of selecting the single
           median as the single pivot, select n pivots to divide the array into
           n+1 regions requiring further processing (and n regions equal to the
           n pivots), with the optimum n ~ sqrt(nmemb).  But to implement that,
           multiple selection would have to somehow indicate the equal-to-pivot
           ranges for each of the pivots, or left+right scans after selection
           would have to determine the extents of those ranges.  And it would
           produce ~ sqrt(nmemb) smaller partitions which would have to be
           processed.
        */
        quickselect_internal(base,s,size,compar,karray,2UL,
            QUICKSELECT_NETWORK_MASK,NULL,NULL); /* bootstrap with quickselect */
        /* XXX samples are partitioned, and could be moved to the appropriate
           regions w/o recomparisons
        */
#else
        /* for 5 samples only, simply identify pivots using sort5 */
        x=nmemb/s*size; /* sample spacing (chars) */
        pivot1=pm-x;
        pa=pivot1-x;
        pivot2=pm+x;
        ps=pivot2+x;
        sort5(pa,pivot1,pm,pivot2,ps,size,compar,swapf,alignsize,size_ratio,options);
#endif
#if DPDEBUG
        if (DEBUGGING(DPQSORT_DEBUG)) {
            (V)fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2));
            print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
        }
#endif
        /* partition array around pivot elements */
        dp_partition(base,nmemb,size,compar,swapf,alignsize,size_ratio,options,
            &pivot1,&pa,&pivot2,&ps);
        npartitions++;
        /* process [base,pivot1), [pa,pivot2), [ps,pu) */
        /* process 3 regions */
        /* region sizes */
        if (pivot1>base) q=(pivot1-base)/size; else q=0UL;
        if (pivot2>pa) r=(pivot2-pa)/size; else r=0UL;
        if (pu>ps) s=(pu-ps)/size; else s=0UL;
        /* should process small regions recursively, large iteratively */
        /* XXX quick hack: 2nd, 3rd regions recursively, 1st iteratively */
        if (r>1UL) {
            nrecursions++;
            dpqsort_internal(pa,r,size,compar,swapf,alignsize,size_ratio,options);
        }
        if (s>1UL) {
            nrecursions++;
            dpqsort_internal(ps,s,size,compar,swapf,alignsize,size_ratio,options);
        }
        if (q<2UL) return;
        nmemb=q;
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void dpqsort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), unsigned int options)
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    if (4UL>dp_cutoff) dp_cutoff=4UL; /* need at least 5 elements for samples */
    nfrozen=0UL, pivot_minrank=nmemb;
    dpqsort_internal(base,nmemb,size,compar,swapf,alignsize,size_ratio,
        options);
}
