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
/* $Id: ~|^` @(#)   This is dual.c version 1.18 dated 2018-04-20T11:58:17Z. \ $ */
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
#define MODULE_VERSION "1.18"
#define MODULE_DATE "2018-04-20T11:58:17Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define QUICKSELECT_LOOP d_quickselect_loop

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* quickselect_loop declaration */
#if ! defined(QUICKSELECT_LOOP_DECLARED)
QUICKSELECT_EXTERN
# include "quickselect_loop_decl.h"
;
# define QUICKSELECT_LOOP_DECLARED 1
#endif /* QUICKSELECT_LOOP_DECLARED */

/* turn on (or off) debugging code */
#define DPDEBUG 0

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

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
    char *pa, *pb, *pc, *pd, *pe, *pu;
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
    /*     a                                         c  u*/
    /*     b                                         d   */
    /*                                               e   */
    /* intermediate: */
    /* +-----------------------------------------------+ */
    /* |=p1|<p1|     ?                 |p1<x<p2|>p2|=p2| */
    /* +-----------------------------------------------+ */
    /*      b   a                       c       d   e   u*/
    /* all categorized: */
    /* +-----------------------------------------------+ */
    /* |=p1|    <p1    |    p1<x<p2    |    >p2    |=p2| */
    /* +-----------------------------------------------+ */
    /*      b           c               d           e   u*/
    /* final, canonical: */
    /* +-----------------------------------------------+ */
    /* |    <p1    |=p1|    p1<x<p2    |=p2|    >p2    | */
    /* +-----------------------------------------------+ */
    /*  base        b   c               d   e           u*/
    /* Expected comparisons (uniformly distributed random input):
          N/3 * 1 + 2N/3 * 2 = 5N/3
          67% more than single pivot partition (N-1)
       Expected swaps (uniformly distributed distinct input):
          2 + N/3 * 0 + N/3 * 1 + N/3 * 2 + 2 = N + 4
          300% more than single-pivot partition (N/4+2)
    */
    /* Initialize */
    if (*ppivot1!=base) {
        EXCHANGE_SWAP(swapf,base,*ppivot1,size,alignsize,size_ratio,nsw++);
        if (*ppivot2==base) *ppivot2=*ppivot1; /* it moved */
    }
    pa=pb=base+size;
    pc=pd=pe=(pu=base+size*nmemb)-size;
    if (*ppivot2!=pe) {
        EXCHANGE_SWAP(swapf,*ppivot2,pe,size,alignsize,size_ratio,nsw++);
    }
#if DPDEBUG
    if (DEBUGGING(DPQSORT_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d) */\n",
            __func__,__LINE__,(void *)base,0UL,*((int *)base),
            (void *)pe,(pe-base)/size,*((int *)pe));
        print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
    }
#endif /* DPDEBUG */
    /* partition element at pa while pa<pc */
    while (pa<pc) {
        c=OPT_COMPAR(pa,base,options); /* pivot1 at base */
        if (0>c) { /* element at pa is < pivot1 */
            pa+=size;
        } else if (0==c) { /* element at pa compares equal to pivot1 */
            if (pa!=pb) {
                EXCHANGE_SWAP(swapf,pb,pa,size,alignsize,size_ratio,nsw++);
            }
            pa+=size,pb+=size;
        } else { /* element at pa is greater than pivot1 */
            A(0<c);
            /* element will be moved to a section starting at pc, pd, or pe */
            pc-=size;
            if (pa!=pc) {
                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
            }
            A(pc!=pe);
            d=OPT_COMPAR(pc,pe,options); /* pe is same as pivot2 */
            /* if pivot1<*pc<pivot2 (0>d), element is now in place */
            if (0<=d) { /* element >= pivot2 */
                pd-=size;
                if (pc!=pd) {
                    EXCHANGE_SWAP(swapf,pc,pd,size,alignsize,size_ratio,nsw++);
                }
                if (0==d) { /* element compares equal to pivot2 */
                    pe-=size;
                    if (pd!=pe) {
                        EXCHANGE_SWAP(swapf,pd,pe,size,alignsize,size_ratio,
                            nsw++);
                    }
                }
            }
        }
    }
    /* blockmoves to canonicalize */
    pb=blockmove(base,pb,pc,swapf);
    pe=blockmove(pd,pe,pu,swapf);
    *ppivot1=pb,*pmid=pc,*ppivot2=pd,*pgt=pe;
#if DPDEBUG
    if (DEBUGGING(DPQSORT_DEBUG)) {
        /* verify correct partitioning */
        (V)fprintf(stderr,
            "/* %s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d) */\n",
            __func__,__LINE__,(void *)pb,(pb-base)/size,*((int *)pb),
            (void *)pd,(pd-base)/size,*((int *)pd));
        print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
        for (pa=base; pa<pb; pa+=size) {
            c=OPT_COMPAR(pa,pb,options);
            if (0<=c)
                (V)fprintf(stderr,
                    "/* %s: %s line %d: partitioning error at %lu vs. pivot 1 "
                    "at %lu/\n", __func__,source_file,__LINE__,
                    (pa-base)/size,(pb-base)/size);
            A(0>c);
        }
        for (pa=pb+size; pa<pc; pa+=size) {
            c=OPT_COMPAR(pa,pb,options);
            if (0!=c)
                (V)fprintf(stderr,
                    "/* %s: %s line %d: partitioning error at %lu vs. pivot 1 "
                    "at %lu/\n", __func__,source_file,__LINE__,
                    (pa-base)/size,(pb-base)/size);
            A(0==c);
        }
        for (pa=pc; pa<pd; pa+=size) {
            c=OPT_COMPAR(pa,pb,options);
            d=OPT_COMPAR(pa,pd,options);
            if ((0>=c)||(0<=d))
                (V)fprintf(stderr,
                    "/* %s: %s line %d: partitioning error at %lu vs. pivot 1 "
                    "at %lu, pivot 2 at %lu/\n", __func__,source_file,__LINE__,
                    (pa-base)/size,(pb-base)/size,(pd-base)/size);
            A((0<c)&&(0>d));
        }
        for (pa=pd+size; pa<pe; pa+=size) {
            d=OPT_COMPAR(pa,pd,options);
            if (0!=d)
                (V)fprintf(stderr,
                    "/* %s: %s line %d: partitioning error at %lu vs. pivot 2 "
                    "at %lu/\n", __func__,source_file,__LINE__,
                    (pa-base)/size,(pd-base)/size);
            A(0==d);
        }
        for (pa=pe; pa<pu; pa+=size) {
            d=OPT_COMPAR(pa,pd,options);
            if (0>=d)
                (V)fprintf(stderr,
                    "/* %s: %s line %d: partitioning error at %lu vs. pivot 2 "
                    "at %lu/\n", __func__,source_file,__LINE__,
                    (pa-base)/size,(pd-base)/size);
            A(0<d);
        }
    }
#endif /* DPDEBUG */
}

/* Regions to process after partitioning. */
struct region_struct {
    char *base;
    size_t nmemb;
};

static size_t regionsize = sizeof(struct region_struct);

/* compare regions by size */
static int regioncmp(const struct region_struct *r1,
    const struct region_struct *r2)
{
    size_t s1=r1->nmemb, s2=r2->nmemb;
    if (s1>s2) return 1;
    if (s1<s2) return -1;
    return 0;
}

/* swap two regions by structure copy */
static void regionswap(struct region_struct *r1, struct region_struct *r2,
    size_t unused)
{
    struct region_struct t;
    t = *r1, *r1=*r2, *r2=t;
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
    struct region_struct regions[3];

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
#if DPDEBUG
    if (DEBUGGING(DPQSORT_DEBUG)) {
        (V)fprintf(stderr,
           "/* %s: %s line %d: %lu elements */\n",
           __func__,source_file,__LINE__,nmemb);
        print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
    }
#endif /* DPDEBUG */
    for (;;) {
        /* Use dedicated sort if array is small enough.
           d_dedicated sort uses the best sorting method based on the expected
           number of comparisons and swaps, taking size_ratio into account as a
           multiplier for the swap cost, and based on divide-and-conquer using
           single-pivot quicksort with 3 samples for pivot selection.  The
           choice might not be optimum for dual-pivot quicksort because dual-
           pivot divide-and-conquer uses about 5% more comparisons than single-
           pivot quicksort, and dual-pivot quicksort uses many more swaps than
           single-pivot quicksort. In particular, d_dedicated_sort may return
           non-zero to indicate continued divide-and-conquer in cases where some
           dedicated sorting method might yield better performance than dual-
           pivot quicksort (but not better than single-pivot quicksort).
           Because dual-pivot quicksort is inherently less efficient than
           single-pivot quicksort (dual-pivot uses at least 5% more comparisons
           and many more swaps) it's not worthwhile developing a special-case
           dedicated sort for dual-pivot quicksort (such a dedicated sort which
           selects the most efficient sorting method for array size and element
           size_ratio would use single-pivot quicksort rather than returning for
           continued dual-pivot processing; no point -- just use single-pivot
           quicksort in the first place).  Use of dedicated_sort is really a
           cheat here (in keeping with the dual-pivot approach, merge sort
           should use a 3-way merge, insertion sort should use some sort of
           ternary search, etc., all of which would likely also be more costly
           than single-choice methods), so it isn't used for nmemb > dp_cutoff.
        */
        if (nmemb<=dp_cutoff) {
            (V)QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,NULL,0UL,0UL,
                swapf,alignsize,size_ratio,0U,quickselect_cache_size,0UL,
                options,NULL,NULL);
    /* <- */    return; /* Done; else continue divide-and-conquer */
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
        /* With 2 pivots, it is not possible to simultaneously have uniform
           sampling and the expectation of equal partitioned region sizes.
           This implementation opts for uniform sampling to avoid anomalies
           with input patterns.  Worst case expected region size imbalance with
           uniformly distributed random input is expected to be 30%-40%-30%
           (ignoring the pivots themselves) with 5 samples; this improves to
           5/16-3/8-5/16 at 8 samples, 7/22-4/11-7/22 at 11 samples, etc.  The
           slight imbalance in partition sizes is expected to have only a tiny
           effect on performance.
        */
# define SAMPLE_ROOT 2
# if SAMPLE_ROOT == 4
        for (r=1UL,s=5UL,q=u*625UL; q<nmemb; r++,s+=3UL,q=u*s*s*s*s) ;
# elif SAMPLE_ROOT == 3
        for (r=1UL,s=5UL,q=u*125UL; q<nmemb; r++,s+=3UL,q=u*s*s*s) ;
# else
        for (r=1UL,s=5UL,q=u*25UL; q<nmemb; r++,s+=3UL,q=u*s*s) ;
# endif
        if (q>nmemb) r--,s--;
        if (s<5UL) r=1UL,s=5UL;
# if DPDEBUG
        if (DEBUGGING(DPQSORT_DEBUG)) {
            (V)fprintf(stderr,"%s: %s line %d: %lu elements, %lu samples, "
                "r=%lu\n",__func__,source_file,__LINE__,nmemb,s,r);
        }
# endif /* DPDEBUG */
#else
        s=5UL;
#endif
        pu=base+nmemb*size; /* past last element */
#if SAMPLE_SELECTION
        /* Aumuller & Dietzfelbinger suggest s/2 and s/4 */
        /* but it's slower in practice */
        karray[0]=r;
        karray[1]=(r<<1)+1UL;
        if (DEBUGGING(DPQSORT_DEBUG)) {
            (V)fprintf(stderr,"%s: %s line %d: karray[0]=%lu, karray[1]=%lu\n",
                __func__,source_file,__LINE__,karray[0],karray[1]);
        }
        /* swap sample elements to beginning of array for selection */
        /* There are s uniformly-spaced samples taken from nmemb elements.
           The samples are spaced nmemb/s elements apart with the first sample
           at 0-based index (nmemb-s)/s/2.  Variable r will be set to the
           sample spacing.  Variable q will be initially set to the index of the
           first sample, and will then be used to count samples.
           (nmemb-s)/s/2 = (nmemb/s-1)/2 = (r-1)/2
        */
        r=nmemb/s, q=(r-1UL)>>1, ps=base+q*size;
        for (pa=base,q=0UL; q<s; pa+=size,ps+=r*size,q++) {
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
        */ /* bootstrap with quickselect; select pivots from samples */
        /* table_index will be small (s samples, 2 ranks) */
        d_quickselect_loop(base,0UL,s,size,compar,karray,0UL,2UL,swapf,
            alignsize,size_ratio,2U,quickselect_cache_size,0UL,0U,NULL,NULL);
        /* Samples are partitioned, and one might think that they could be moved
           to the appropriate regions w/o recomparisons (except for the fact
           that some sample elements might compare equal to one (or both) of the
           pivots, and selection provides no indication of that, so recomparison
           would be necessary).
        */
#else
        /* for 5 samples only, simply identify pivots using sort5 */
        r=nmemb/s*size; /* sample spacing (chars) */
        pivot1=pm-r;
        pa=pivot1-r;
        pivot2=pm+r;
        ps=pivot2+r;
        sort5(pa,pivot1,pm,pivot2,ps,size,compar,swapf,alignsize,size_ratio,
            options);
#endif
#if DPDEBUG
        if (DEBUGGING(DPQSORT_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d) */\n",
                __func__,__LINE__,(void *)pivot1,(pivot1-base)/size,
               *((int *)pivot1),(void *)pivot2,(pivot2-base)/size,
               *((int *)pivot2));
            print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
        }
#endif
        /* partition array around pivot elements */
        dp_partition(base,nmemb,size,compar,swapf,alignsize,size_ratio,options,
            &pivot1,&pa,&pivot2,&ps);
        npartitions++;
        /* process 3 regions: [base,pivot1), [pa,pivot2), [ps,pu) */
        regions[0].base=base, regions[0].nmemb=(pivot1-base)/size;
        regions[1].base=pa, regions[1].nmemb=(pivot2-pa)/size;
        regions[2].base=ps, regions[2].nmemb=(pu-ps)/size;
        /* sort regions by size (3 regions; use quickselect_loop) */
        /* size 3 dedicated sort always completes sorting (ignore return) */
        (V)QUICKSELECT_LOOP((char *)regions,0UL,3UL,regionsize,regioncmp,NULL,
            0UL,0UL,regionswap,regionsize,1UL,0U,quickselect_cache_size,0UL,
            options,NULL,NULL);
        A(regions[0].nmemb<=regions[1].nmemb);
        A(regions[1].nmemb<=regions[2].nmemb);
        /* process small regions recursively, large iteratively */
        /* If the largest region is too small to process, so are the others. */
        if (regions[2].nmemb<2UL) return;
        for (q=0UL; q<2UL; q++) {
            if (regions[q].nmemb>1UL) {
                nrecursions++;
                dpqsort_internal(regions[q].base,regions[q].nmemb,size,compar,
                    swapf,alignsize,size_ratio,options);
            }
        }
        base=regions[2].base;
        nmemb=regions[2].nmemb;
    }
}

QUICKSELECT_INLINE
void dpqsort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *), unsigned int options)
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    if (4UL>dp_cutoff) dp_cutoff=4UL; /* need at least 5 elements for samples */
    nfrozen=0UL, pivot_minrank=nmemb;
    options |= QUICKSELECT_NO_REPIVOT;
    dpqsort_internal(base,nmemb,size,compar,swapf,alignsize,size_ratio,
        options);
}
