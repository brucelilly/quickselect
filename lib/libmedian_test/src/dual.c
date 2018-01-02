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
* $Id: ~|^` @(#)    dual.c copyright 2016-2017 Bruce Lilly.   \ dual.c $
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
/* $Id: ~|^` @(#)   This is dual.c version 1.5 dated 2017-12-15T21:54:06Z. \ $ */
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
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2017-12-15T21:54:06Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* dual-pivot qsort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void dpqsort_internal(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    char *pa, *pb, *pivot1, *pivot2, *pm, *pn, *ps, *pt, *pu;
    size_t n, q, r, s, u, v, w, x, karray[2];

#define DPDEBUG 0
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
        for(s=30UL,q=900UL; q<nmemb; s+=30UL,q=s*s) ; /* XXX worry about overflow iff dual-pivot shows potential ... */
        if (q>nmemb) s-=30UL;
        if (s<5UL) s=5UL;
#else
        s=5UL;
#endif
        w=nmemb>>1;
        pm=base+w*size; /* middle element */
        pu+=size; /* past last element */
#if SAMPLE_SELECTION
        /* swap sample elements to middle of array for selection */
        /* XXX quick hack */
        for (r=((s+1UL)>>1),q=1UL,x=nmemb/s*size; q<r; q++) {
            if (x!=size) {
                u=q*x;
                v=q*size;
                if (pm-u>=base) {
                    EXCHANGE_SWAP(swapf,pm-u,pm-v,size,alignsize,size_ratio,
                        nsw++);
                }
                if (pm+u<pu) {
                    EXCHANGE_SWAP(swapf,pm+u,pm+v,size,alignsize,size_ratio,
                        nsw++);
                }
            }
        }
        r=s>>1;
# if 1
        q=(s+1UL)/6UL;
        karray[0]=r-q;
        karray[1]=r+q;
# else
        /* Aumuller & Dietzfelbinger suggest s/2 and s/4 */
        /* but it's slower in practice */
        q=r>>1;
        karray[0]=q;
        karray[1]=r;
# endif
        pn=pm-r*size;
        n=s;
        pivot1=pn+karray[0]*size;
        pivot2=pn+karray[1]*size;
        /* XXX selecting pivots partitions the samples around the pivots and
           costs ~ 2s.  Subsequent partitioning of the array costs > 2n.
           Using full array indices (rather than relative to first sample)
           and selecting the pivots based on those ranks from the entire
           array would partition the array around the pivots w/o a separate
           partitioning step, with total cost ~ 2n.
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
        quickselect_internal(pn,n,size,compar,karray,2UL,
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
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2));
    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
#endif
        /* partition array around pivot elements */
#if 1
        /* This implementation puts equals in the outer regions, then separates
           them into position in a quick single-comparison pass over each of the
           two outer regions for a total expected comparison count of 7/3 N. An
           alternative is to put them in the middle region, then separate
           them into position using a one- to two-comparison pass over the
           middle region for a total expected comparison count of 2 N (for
           pivot1,pivot2 same value, else also 7/3 N).  Either has about
           double the comparisons of a single-pivot partition. This one has less
           overhead, so runs faster on typical input sequences.
        */
        /* | =P1 | <P1 | P1<x<P2 |  ?            | >P2 | =P2 | */
        /*  |     |     |         |               |     |     | */
        /*  base  pa    pb        pm              ps    pt    pu */
        npartitions++;
        for (pb=pm=base,ps=pu; pm<ps;) {
            A(base<=pm);A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0<=compar(pivot1,pm)) {
                if (pb!=pm) {
                    EXCHANGE_SWAP(swapf,pb,pm,size,alignsize,size_ratio,
                        nsw++);
                }
                if (pivot1==pm) pivot1=pb; else if (pivot2==pm) pivot2=pb;
                else if (pivot1==pb) pivot1=pm; else if (pivot2==pb) pivot2=pm;
                pb+=size;
            } else if (0>=compar(pivot2,pm)) {
                do ps-=size; while ((pm<ps)&&(0>=compar(pivot2,ps)));
                if (pm>=ps) break;
                EXCHANGE_SWAP(swapf,pm,ps,size,alignsize,size_ratio,nsw++);
                if (pivot1==pm) pivot1=ps; else if (pivot2==pm) pivot2=ps;
                else if (pivot1==ps) pivot1=pm; else if (pivot2==ps) pivot2=pm;
                continue; /* new unknown at pm */
            }
            pm+=size;
        }
        /* |      <=P1 | P1<x<P2 |      >=P2 | */
        /*  |           |         |           | */
        /*  base        pb        ps          pu */
        /* group equals */
        /* | <P1 | =P1 | P1<x<P2 | =P2 | >P2 | */
        /*  |     |     |         |     |     | */
        /*  base  pa    pb        ps    pt    pu */
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pb=%p[%lu](%d), ps=%p[%lu](%d)\n",__func__,__LINE__,(void *)pb,(pb-base)/size,*((int *)pb),(void *)ps,(ps-base)/size,*((int *)ps));
    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
#endif
        npartitions++;
        for (pa=pb,pm=base; pm<pa; pm+=size) {
            if (0==compar(pivot1,pm)) {
                pa-=size;
                if (pm!=pa) {
                    EXCHANGE_SWAP(swapf,pm,pa,size,alignsize,size_ratio,
                        nsw++);
                }
                if (pivot1==pm) pivot1=pa; else if (pivot2==pm) pivot2=pa;
            }
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,(void *)pa,(pa-base)/size,*((int *)pa),(void *)pb,(pb-base)/size,*((int *)pb));
    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
#endif
        npartitions++;
        for (pt=ps,pm=pu-size; pm>=pt; pm-=size) {
            if (0==compar(pivot2,pm)) {
                if (pm!=pt) {
                    EXCHANGE_SWAP(swapf,pm,pt,size,alignsize,size_ratio,
                        nsw++);
                }
                if (pivot1==pm) pivot1=pt; else if (pivot2==pm) pivot2=pt;
                pt+=size;
            }
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: pivot1=%p[%lu](%d), pivot2=%p[%lu](%d)\n",__func__,__LINE__,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2));
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,(void *)pa,(pa-base)/size,*((int *)pa),(void *)pb,(pb-base)/size,*((int *)pb));
fprintf(stderr, "//%s line %d: ps=%p[%lu](%d), pt=%p[%lu](%d)\n",__func__,__LINE__,(void *)ps,(ps-base)/size,*((int *)ps),(void *)pt,(pt-base)/size,*((int *)pt));
    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
#endif
#else
        npartitions++;
        for (pa=pm=base,pt=pu; pm<pt;) {
            A(base<=pm);A(base<=pa);A(pa<=pm);A(pt<=pu);
            A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0<compar(pivot1,pm)) {
                EXCHANGE_SWAP(swapf,pa,pm,size,alignsize,size_ratio,nsw++);
                if (pivot1==pa) pivot1=pm; else if (pivot2==pa) pivot2=pm;
                else if (pivot2==pm) pivot2=pa;
                pa+=size;
            } else if (0>compar(pivot2,pm)) {
                do pt-=size; while ((pm<pt)&&(0>compar(pivot2,pt)));
                if (pm>=pt) break;
                EXCHANGE_SWAP(swapf,pm,pt,size,alignsize,size_ratio,nsw++);
                if (pivot1==pt) pivot1=pm; else if (pivot2==pt) pivot2=pm;
                else if (pivot1==pm) pivot1=pt;
                continue; /* new unknown at pm */
            }
            pm+=size;
        }
        /* | <P1 |      P1<=x<=P2      | >P2 | */
        /*  |     |                     |     | */
        /*  base  pa                    pt    pu */
#if DPDEBUG
fprintf(stderr, "//%s line %d: base=%p, pivot1=%p[%lu](%d), pivot2=%p[%lu](%d), pu=%p\n",__func__,__LINE__,(void *)base,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2),(void *)pu);
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pt=%p[%lu](%d)\n",__func__,__LINE__,(void *)pa,(pa-base)/size,*((int *)pa),(void *)pt,(pt-base)/size,*((int *)pt));
    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
#endif
        /* group equals */
        /* | <P1 | =P1 | P1<x<P2 | =P2 | >P2 | */
        /*  |     |     |         |     |     | */
        /*  base  pa    pb        ps    pt    pu */
        npartitions++;
        for (pm=pb=pa,ps=pt; pm<ps;) {
            A(base<=pm);A(base<=pa);A(pa<=pb);A(ps<=pt);A(pt<=pu);
            A(base<=pivot1);A(pivot1<pu);A(base<=pivot2);A(pivot2<pu);
            if (0==compar(pivot1,pm)) {
                EXCHANGE_SWAP(swapf,pb,pm,size,alignsize,size_ratio,nsw++);
                if (pivot1==pm) pivot1=pb; else if (pivot1==pb) pivot1=pm;
                if (pivot2==pm) pivot2=pb; else if (pivot2==pb) pivot2=pm;
                pb+=size;
            } else if (0==compar(pivot2,pm)) {
                do ps-=size; while ((pm<ps)&&(0==compar(pivot2,ps)));
                if (pm>=ps) break;
                EXCHANGE_SWAP(swapf,pm,ps,size,alignsize,size_ratio,nsw++);
                if (pivot1==pm) pivot1=ps; else if (pivot1==ps) pivot1=pm;
                if (pivot2==pm) pivot2=ps; else if (pivot2==ps) pivot2=pm;
                continue; /* test moved element at pm */
            }
            pm+=size;
        }
#if DPDEBUG
fprintf(stderr, "//%s line %d: base=%p, pivot1=%p[%lu](%d), pivot2=%p[%lu](%d), pu=%p\n",__func__,__LINE__,(void *)base,(void *)pivot1,(pivot1-base)/size,*((int *)pivot1),(void *)pivot2,(pivot2-base)/size,*((int *)pivot2),(void *)pu);
fprintf(stderr, "//%s line %d: pa=%p[%lu](%d), pb=%p[%lu](%d)\n",__func__,__LINE__,(void *)pa,(pa-base)/size,*((int *)pa),(void *)pb,(pb-base)/size,*((int *)pb));
fprintf(stderr, "//%s line %d: ps=%p[%lu](%d), pt=%p[%lu](%d)\n",__func__,__LINE__,(void *)ps,(ps-base)/size,*((int *)ps),(void *)pt,(pt-base)/size,*((int *)pt));
    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
#endif
#endif
        npartitions++;
        /* process [base,pa), [pb,ps), [pt,pu) */
        /* process 3 regions */
        /* region sizes */
        if (pa>base) q=(pa-base)/size; else q=0UL;
        if (ps>pb) r=(ps-pb)/size; else r=0UL;
        if (pu>pt) s=(pu-pt)/size; else s=0UL;
        /* should process small regions recursively, large iteratively */
        /* XXX quick hack: 2nd, 3rd regions recursively, 1st iteratively */
        if (r>1UL) {
            nrecursions++;
            dpqsort_internal(pb,r,size,compar,swapf,alignsize,size_ratio,options);
        }
        if (s>1UL) {
            nrecursions++;
            dpqsort_internal(pt,s,size,compar,swapf,alignsize,size_ratio,options);
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
