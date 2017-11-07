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
* $Id: ~|^` @(#)    quickselect_s.c copyright 2016-2017 Bruce Lilly.   \ quickselect_s.c $
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
/* $Id: ~|^` @(#)   This is quickselect_s.c version 1.2 dated 2017-11-07T18:52:02Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.quickselect_s.c */

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
#define ID_STRING_PREFIX "$Id: quickselect_s.c ~|^` @(#)"
#define SOURCE_MODULE "quickselect_s.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2017-11-07T18:52:02Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

#define __STDC_WANT_LIB_EXT1__ 1
/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#ifdef RSIZE_MAX
/* Preliminary support for 9899:201x draft N1570 qsort_s w/ "context".
   "context" is similar to glibc qsort_r; N1570 also has provision for
   runtime-constraint violation detection and handling.
   Consequences of "context" are wide-ranging: any operation that requires
   comparisons (pivot selection, partitioning, dedicated sorting and/or
   selection, and support functions for any of those) has to be rewritten.
   The only operations unaffected are sampling, basic swaps, blockmoves using
   swaps, rank comparisons, and partition size evaluation; sampling and
   repivoting tables may also be reused without change.
   Executable code size is roughly doubled, not including external
   constraint-handler function code.
*/
# if QUICKSELECT_PROVIDE_HANDLER
#  include <stdio.h>            /* fprintf stderr */
static void default_handler(const char * restrict msg, void * restrict ptr,
    errno_t error)
{
    (V)fprintf(stderr, "%s\n",msg);
    errno=error;
}
static constraint_handler_t the_handler = default_handler;
static constraint_handler_t set_constraint_handler_s(constraint_handler_t handler)
{
    constraint_handler_t old_handler=the_handler;
    if (NULL==handler) the_handler=default_handler; else the_handler=handler;
    return old_handler;
}
# endif /* PROVIDE_HANDLER */
/* static functions supporting comparison "context" and runtime-constraint
   violation detection/handling
*/
/* medians of sets of 3 elements */
/* 3 elements:
   Optimized ternary median-of-3: 1-3 comparisons.
   Minimum number of comparisons (because in the qsort model (external function
   for comparison), comparisons are expensive).  If any two elements (of the 3)
   compare equal, either can be chosen for the median (the value is the median
   of the 3, regardless of the value of the third element).  Likewise, the
   median may be determined by the first two comparisons, e.g. a < b and b < c.
   In the worst case, 3 comparisons are required.
   Return value is biased to pb, then pa.
*/
/* called from remedian_s and select_pivot_s */
static inline
char *fmed3_s(char *pa,char *pb,char *pc,
    int(*compar)(const void *,const void *,void *), void *context)
{
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(MEDIAN_DEBUG)) {
(V)fprintf(stderr, "/* %s: %s line %d: pa=%p, pb=%p, pc=%p */\n",
__func__,source_file,__LINE__,(void *)pa,(void *)pb,(void *)pc);
}
#endif
    int c=compar(pa,pb,context);
    if (0!=c) {
        int d=compar(pb,pc,context);
        if (0!=d) {
            if ((0<d)&&(0>c)) {
                if (0>compar(pa,pc,context)) return pc;
                else return pa;
            } else if ((0>d)&&(0<c)) {
                if (0<compar(pa,pc,context)) return pc;
                else return pa;
            }
        }
    }
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(MEDIAN_DEBUG)) {
(V)fprintf(stderr, "/* %s: %s line %d: pb=%p */\n",
__func__,source_file,__LINE__,(void *)pb);
}
#endif
    return pb;
}

/* Remedian (base 3) of samples, recursive implementation. */
/* top-level call from select_pivot_s */
/* idx is the index into the sampling table (with number of samples equal to
      the idx'th power of 3), which provides a cheap way of determining when to
      end the recursion
*/
/* row_spacing and sample_spacing are measured in elements, converted to chars
      here for offsets to middle element
*/
/* Uniform sample spacing is maintained, recursively. */
static inline
char *remedian_s(char *middle, size_t row_spacing, size_t sample_spacing,
    size_t size, unsigned int idx, int(*compar)(const void*,const void*,void *),
    void *context)
{
    size_t o;
    A((SAMPLING_TABLE_SIZE)>idx);
    if (0U < --idx) {
        char *pa, *pb, *pc;
        size_t s=sample_spacing/3UL;

        o=s*size;
        pa=remedian_s(middle-o,row_spacing,s,size,idx,compar,context);
        pb=remedian_s(middle,row_spacing,s,size,idx,compar,context);
        pc=remedian_s(middle+o,row_spacing,s,size,idx,compar,context);
        return fmed3_s(pa,pb,pc,compar,context);
    }
    o=row_spacing*size;
    return fmed3_s(middle-o,middle,middle+o,compar,context);
}

/* quickselect_loop_s forward declaration (for select_pivot_s) */
static inline
errno_t quickselect_loop_s(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *,void *),
    void *context, const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, unsigned int options, char **ppeq, char **ppgt);

/* select_pivot_s using remedian_s or median-of-medians */
/* Called from both loop functions. */
/* return NULL if median-of-medians median selection fails */
static inline
char *select_pivot_s(char *base, size_t first, size_t beyond,
    register size_t size,int (*compar)(const void *,const void*),void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    struct sampling_table_struct *pst, unsigned int table_index,
    const size_t *pk, unsigned int options, char **ppc, char **ppd, char **ppe,
    char **ppf)
{
    size_t nmemb=beyond-first;
    register size_t n, r=nmemb/3UL;     /* 1/3 #elements */
    register char *pivot;

#if DEBUG_CODE
if (DEBUGGING(PIVOT_SELECTION_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) fprintf(stderr, "/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, nmemb=%lu, table_index=%u, options=0x%x */\n",
__func__,source_file,__LINE__,(void *)base,first,beyond,nmemb,table_index,options);
#endif
    switch (options&((QUICKSELECT_RESTRICT_RANK)|(QUICKSELECT_STABLE))) {
#if QUICKSELECT_STABLE
        case ((QUICKSELECT_RESTRICT_RANK)|(QUICKSELECT_STABLE)) :
            /* almost full remedian */
            while (table_index<(SAMPLING_TABLE_SIZE)-1U) {
                n=sorting_sampling_table[table_index].samples;
                if (n>r) break;
                table_index++;
            }
            while ((n=sorting_sampling_table[table_index].samples)>r)
                table_index--;
            if (table_index==(SAMPLING_TABLE_SIZE)-1U) {
                for (; n<r; table_index++)
                    n*=3UL;
                if (n>r) table_index--;
            }
        /*FALLTHROUGH*/
#endif /* QUICKSELECT_STABLE */
        default: /* remedian of samples */
            /*Fast pivot selection:1 sample, median-of-3, remedian of samples.*/
            pivot=base+size*(first+(nmemb>>1));     /* [upper-]middle element */
            if (0U<table_index) {       /* 3 or more samples */
                A(base+first*size<=pivot);A(pivot<base+beyond*size);
                pivot=remedian_s(pivot,r,r,size,table_index,compar,context);
            }
            *ppc=*ppd=pivot, *ppe=*ppf=pivot+size;
        break;
        case (QUICKSELECT_RESTRICT_RANK) : /* median-of-medians */
            {
                size_t karray[1];
                register size_t o;
                register char *pa, *pb, *pc, *pm;

                A(NULL!=base);A(NULL!=compar);
                A((SAMPLING_TABLE_SIZE)>table_index);
                A(NULL!=ppc);A(NULL!=ppd);A(NULL!=ppe);A(NULL!=ppf);
                if ((char)0==file_initialized) initialize_file(__FILE__);
                /* Finding a pivot with guaranteed intermediate rank. Ideally, median
                   (50%).  Blum, Floyd, Pratt, Rivest, Tarjan median-of-medians using
                   sets of 5 elements with recursion guarantees rank in (asymptotically)
                   30-70% range, often better; can guarantee linear median-finding,
                   N log(N) sorting. Simplification ignores "leftover" elements with a
                   slight increase in rank range.  Non-recursive method (using separate
                   linear median finding) can use sets of 3 elements to provide a
                   tighter 33.33% to 66.67% range (again, slightly wider if "leftover"
                   elements are ignored) at lower computational cost.
                */
                pc=base+first*size;
                A(9UL<=nmemb); /* never repivot for nmemb < 9 */
                /* Medians of sets of 3 elements. */
                A(1UL<r); /* never repivot for nmemb<9 */
                /* 3 element sets (columns); medians -> 1st row, ignore leftovers */
                for (o=0UL,n=r*size; o<n; o+=size) {
                    pa=pc+o;
                    pb=pa+n; /* middle element */
                    A(pb+n<base+beyond*size);
                    pm=fmed3_s(pb,pa,pb+n,compar,context);/* first element (pa) bias */
                    if (pm!=pa) EXCHANGE_SWAP(swapf,pm,pa,size,alignsize,size_ratio,nsw++); /* medians at start of sub-array */
                }
                /* medians in first row */
                A(pc<=base+beyond*size-n-n);
                *ppc=pc; /* first median */
                /* median of medians */
                karray[0]=first+(r>>1); /* upper-median for even size arrays */
                A(first<=karray[0]);A(karray[0]<first+r);
                *ppf=base+(first+r)*size; /* past last median */
# if ASSERT_CODE
                A((NULL!=ppd)&&(NULL!=ppe));
                *ppd=*ppe=NULL; /* clear before quickselect to avoid random values */
#endif
                /* select median of medians; partitions medians */
                /* cutoff (5UL used here) is unimportant for selection */
                pst=d_sampling_table(first,first+r,karray,0UL,1UL,ppd,NULL,
                    &table_index,r);
                if (0U!=save_partial)
                    (V)quickselect_loop_s(base,first,first+r,size,compar,
                        context,karray,0UL,1UL,swapf,alignsize,size_ratio,5UL,
                        0x07F8U,ppd,ppe);
                else
                    (V)quickselect_loop_s(base,first,first+r,size,compar,
                        context,karray,0UL,1UL,swapf,alignsize,size_ratio,5UL,
                        0x07F8U,NULL,NULL);
                pivot=base+karray[0]*size; /* pointer to median of medians */
                /* First third of array (medians) is partitioned. */
                if (0U==save_partial) *ppe=(*ppd=pivot)+size;
            }
        break;
    }
    A(*ppd<=pivot);A(pivot<*ppe);
    return pivot;
}

static inline errno_t dedicated_sort_s(char *base, size_t first, size_t beyond,
    /*const*/ size_t size, int (*compar)(const void *,const void *,void *),
    void *context, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options)
{
    errno_t ret=0;
    if (beyond>first) {
        size_t n=beyond-first, na;
        char *pa, *pb, *pu;

        if ((12UL<n)||((2UL<n)&&(0U==(options&((QUICKSELECT_STABLE)<<n))))) {
            /* split array into two (nearly) equal-size pieces */
            na=(n>>1);
            n=0UL; /* force merge sort */
        }
        switch (n) {
            case 1UL : /* shouldn't happen... */
                A(2UL<=n);
                errno=ret=EINVAL;
            return ret;
            case 2UL : /* 1 comparison, <=1 swap; less overhead than default */
                pa=base+first*size;
                pb=pa+size;
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio);
            break;
            case 3UL : /* optimized sort of 3 */
#if FAVOR_SORTED /* favor already-sorted, reverse-sorted input sequences */
                {   /* Optimized comparison-based sort:
                       average 32/13 comparisons and 11/13 swaps over all
                       possible inputs (including duplicate and all-equal
                       values), 8/3 comparisons and 7/6 swaps averaged
                       over distinct-valued inputs.  Already-sorted and
                       reversed inputs use 2 comparisons.  Bitonic and rotated
                       inputs require 3 comparisons.
                    */
                    char *pc;
                    int c, d;
                    pa=base+first*size;
                    pb=pa+size;
                    pc=pb+size;
                    c=compar(pa,pb,context), d=compar(pb,pc,context);
                    if (0>=c) { /*b>=a*/
                        if (0<d) { /*b>c*/
                            if (0==c) { /*a==b*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                            } else { /*b>a,b>c*/
                                if (0<compar(pa,pc,context)) /*a>c*/
                                    EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,nsw++);
                            }
                        }
                    } else { /*a>b*/
                        if (0<=d) /*b>=c*/
                            EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                        else { /*c>b*/
                            if (0<compar(pa,pc,context)) /*a>c*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++);
                        }
                    }
                }
#else /* ! FAVOR_SORTED */
                {   /* Optimized comparison-based sort:
                       average 32/13 comparisons and 11/13 swaps over all
                       possible inputs (including duplicate and all-equal
                       values), 8/3 comparisons and 7/6 swaps averaged
                       over distinct-valued inputs.  Bitonic and right-rotated
                       inputs use 2 comparisons.  Already-sorted, left-rotated,
                       and reversed inputs require 3 comparisons.  Favoring
                       bitonic inputs here slightly offsets the disadvantage
                       for those inputs of uniform sampling, which favors
                       reversed inputs.
                    */
                    char *pc;
                    int c, d;
                    pa=base+first*size;
                    pb=pa+size;
                    pc=pb+size;
                    c=compar(pa,pc,context), d=compar(pb,pc,context);
                    if (0>=c) { /*c>=a*/
                        if (0<d) { /*b>c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,nsw++);
                        } else if (0>d) { /*b<c*/
                            if ((0==c)||(0<compar(pa,pb,context)))/*a==c||a>b*/
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++);
                        }
                    } else { /*a>c*/
                        if (0>d) /*b<c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,nsw++);
                        else if ((0<d)&&(0>compar(pa,pb,context))) /*c<a<b*/
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++);
                        EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                    }
                }
#endif /* FAVOR_SORTED */
            break;
            default :
                {
                    /* in-place merge sort using rotation */
                    register size_t o;
                    /* split array into two (nearly) equal-size pieces */
                    /* sort pieces */
                    /* use recursion to sort pieces */
                    ret=dedicated_sort_s(base,first,first+na,size,compar,
                        context,swapf,alignsize,size_ratio,options);
                    A(0==ret); /* shouldn't get an error for internal calls! */
                    if (0!=ret) return ret;
                    ret=dedicated_sort_s(base,first+na,beyond,size,compar,
                        context,swapf,alignsize,size_ratio,options);
                    A(0==ret); /* shouldn't get an error for internal calls! */
                    if (0!=ret) return ret;
                    /* merge pieces */
                    pb=base+(first+na)*size;
                    switch (alignsize) {
                        case 8UL : /* uint64_t */
                            MERGE(int_least64_t,base,first,beyond,pa,pb,size,
                                pu,context,swapf,alignsize,size_ratio)
                        break;
                        case 4UL : /* uint32_t */
                            MERGE(int_least32_t,base,first,beyond,pa,pb,size,
                                pu,context,swapf,alignsize,size_ratio)
                        break;
                        case 2UL : /* uint16_t */
                            MERGE(int_least16_t,base,first,beyond,pa,pb,size,
                                pu,context,swapf,alignsize,size_ratio)
                        break;
                        default : /* uint8_t */
                            MERGE(char,base,first,beyond,pa,pb,size,
                                pu,context,swapf,alignsize,size_ratio)
                        break;
                    }
                    nmerges++;
                }
            break;
        }
    }
    return ret;
}

/* array partitioning */
/* called from both loop functions */
static inline
void partition_s(char *base, size_t first, size_t beyond, char *pc, char *pd,
    char *pivot, char *pe, char *pf, size_t size,
    int(*compar)(const void *,const void*, void *), void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, size_t *peq, size_t *pgt)
{
    char *pa, *pb, *pg, *ph, *pl, *pu;
    int c=0, d=0;
#if ASSERT_CODE + DEBUG_CODE
    size_t max_ratio=((pc+size<pf)?3UL:beyond-first), ratio=0UL;
#endif

    if ((char)0==file_initialized) initialize_file(__FILE__);
    npartitions++;
    switch (options&(QUICKSELECT_STABLE)) {
        default:
            /* McGeoch & Tygar suggest that partial partition information
               from median-of-medians might be used to avoid recomparisons
               during repartitioning.
            */
            pa=pb=pl=base+size*first, pg=(ph=pu=base+size*beyond)-size;
            /* return early if already fully partitioned */
            if ((pc==pl)&&(pf==pu)) goto done;
            if (pc+size==pf) { /* pc==pd&&pd+size==pf (pivot only) */
                A(pl<=pc);A(pl<=pd);A(pe<=pu); /* reasonable bounds, pivot in region */
                if (pivot-pl<pu-pivot) { /* pivot @ middle or closer to first element */
                    if (pl!=pivot) { ph=pg;
                        EXCHANGE_SWAP(swapf,ph,pivot,size,alignsize,size_ratio,nsw++);
                        pg=(pivot=ph)-size;
                    }
                    else pa+=size, pb=pa;
                } else { /* pivot is closer to last element */
                    if (pg!=pivot) {
                        EXCHANGE_SWAP(swapf,pl,pivot,size,alignsize,size_ratio,nsw++);
                        pa=pb=(pivot=pl)+size;
                    } else pivot=ph=pg, pg-=size;
                }
                A((pivot==pl)||((pivot==ph)&&(ph+size==pu)));
                A((pl<pa)||(ph<pu)); /* must have pivot somewhere */
                A(pa==pb);A(pg+size==ph);
            } else {
                /* Rearrange blocks for split-end partition */
                A(pc<=pd);A(pd<pe);
                pd=blockmove(pc,pd,pe,swapf);
                pivot=pc;
                if (0U!=save_partial) {
                    A(pe<=pf);A(pf<=ph);
                    pg=blockmove(pe,pf,ph,swapf)-size; /* N.B. -size */
                    A(pl<=pc);A(pc<=pd);
                    /* swap = block to far left; N.B. for pl==pc, this sets pa=pd */
                    pa=blockmove(pl,pc,pd,swapf);
                    pivot=pl;
                    A(pa<=pd);A(pd<=pe);
                    /* swap already-partitioned < leftward; N.B. for pa==pd, pb=pe */
                    pb=blockmove(pa,pd,pe,swapf);
                    if (pa>pl) pivot=pa-size; else pivot=pl;
                } else {
                    pa=pb=pd; /* already-partitioned < */
                    pivot=pa-size;
                }
            }
            if (pa==pb) {
                while ((pb<=pg)&&(0==(c=compar(pivot,pb,context)))) { pb+=size; }
                pa=pb;
                if (0<c)
                    for (pb+=size; (pb<=pg)&&(0<=(c=compar(pivot,pb,context))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
            } else {
                for (; (pb<=pg)&&(0<=(c=compar(pivot,pb,context))); pb+=size)
                    if (0==c) { A(pa!=pb);
                        EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                    }
            }
            if (pg==ph-size) {
                while ((pb<pg)&&(0==(d=compar(pivot,pg,context)))) { pg-=size; }
                ph=pg+size;
                if (0>d)
                    for (pg-=size; (pb<pg)&&(0>=(d=compar(pivot,pg,context))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
            } else {
                for (; (pb<pg)&&(0>=(d=compar(pivot,pg,context))); pg-=size)
                    if (0==d) { A(pg!=ph-size);
                        ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                    }
            }
            while (pb<pg) {
                EXCHANGE_SWAP(swapf,pb,pg,size,alignsize,size_ratio,nsw++); pb+=size, pg-=size;
                for (;(pb<=pg)&&(0<=(c=compar(pivot,pb,context))); pb+=size)
                    if (0==c) { A(pa!=pb);
                        EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                    }
                for (;(pb<pg)&&(0>=(d=compar(pivot,pg,context))); pg-=size)
                    if (0==d) { A(pg!=ph-size);
                        ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                    }
            }
            if (pb>=pa) pd=blockmove(pl,pa,pb,swapf); else pd=pb;
            pe=blockmove(pb,ph,pu,swapf);
            A(pl<=pd); A(pd<pe); A(pe<=pu);
done: ;
            *peq=(pd-base)/size;
            *pgt=(pe-base)/size;
            A(pe>pd);
        break;
        case QUICKSELECT_STABLE :
            /* divide-and-conquer partition */
            pl=base+size*first, pu=base+size*beyond;
            /* Recursively partitions unpartitioned regions down to a partition
               containing a single element, then merges partitions to fully
               partition the array.
            */
            if (pl+size==pu) { /* 1 element */
                if (pivot!=pl) { /* external pivot */
                    c=compar(pl,pivot,context);
                    if (0>c) *peq=*pgt=beyond;
                    else if (0<c) *peq=*pgt=first;
                    else *peq=first, *pgt=beyond;
                } else *peq=first, *pgt=beyond; /* pivot */
            } else if ((pl==pc)&&(pf==pu)) { /* already fully partitioned */
                *peq=(pd-base)/size;
                *pgt=(pe-base)/size;
            } else { /* more than 1 element, not fully partitioned */
                /* already-partitioned region is 1 partition; 1 or 2 others */
                if ((pl<=pc)&&(pf<=pu)) { /* pivot is in region */
                    size_t ipc, ipd, ipe, ipf;

                    ipc=(pc-base)/size, ipd=(pd-base)/size, ipe=(pe-base)/size,
                        ipf=(pf-base)/size;
                    if (pl==pc) { /* already-partitioned at left end */
                        A(pf<pu);
                        /* partition the right unpartitioned region and merge */
                        partition_s(base,ipf,beyond,pc,pd,pivot,pe,pf,size,
                            compar,context,swapf,alignsize,size_ratio,options,
                            peq,pgt);
                        A(*peq<=*pgt);A(ipf<=*peq);A(*pgt<=beyond);
                        merge_partitions(base,first,ipd,ipe,ipf,*peq,*pgt,
                            beyond,size,swapf,alignsize,size_ratio,options,peq,pgt);
                    } else if (pu==pf) { /* already-partitioned at right end */
                        /* partition unpartitioned left region and merge */
                        A(pl<pc);
                        partition_s(base,first,ipc,pc,pd,pivot,pe,pf,size,
                            compar,context,swapf,alignsize,size_ratio,options,
                            peq,pgt);
                        A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipc);
                        merge_partitions(base,first,*peq,*pgt,ipc,ipd,ipe,
                            beyond,size,swapf,alignsize,size_ratio,options,peq,pgt);
                    } else { /* already-partitioned in middle */
                        /* partition unpartitioned regions and merge */
                        size_t eq2, gt2;
                        A(pl<pc);
                        partition_s(base,first,ipc,pc,pd,pivot,pe,pf,size,
                            compar,context,swapf,alignsize,size_ratio,options,
                            peq,pgt);
                        A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipc);
                        merge_partitions(base,first,*peq,*pgt,ipc,ipd,ipe,ipf,
                            size,swapf,alignsize,size_ratio,options,peq,pgt);
                        A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipf);
                        A(pf<pu);
                        partition_s(base,ipf,beyond,pl,base+*peq*size,
                            base+*peq*size,base+*pgt*size,pf,size,compar,
                            context,swapf,alignsize,size_ratio,options,
                            &eq2,&gt2);
                        A(eq2<=gt2);A(ipf<=eq2);A(gt2<=beyond);
                        merge_partitions(base,first,*peq,*pgt,ipf,eq2,gt2,
                            beyond,size,swapf,alignsize,size_ratio,options,peq,pgt);
                    }
                } else { /* external pivot; split, partition, & merge */
                    /* External pivot arises whenpartitioning a part of an
                       unpartitioned sub-array.
                    */
                    size_t mid=first+((beyond-first)>>1), eq2, gt2;
                    partition_s(base,first,mid,pc,pd,pivot,pe,pf,size,compar,
                            context,swapf,alignsize,size_ratio,options,peq,pgt);
                    A(first<mid);A(mid<beyond);
                    partition_s(base,mid,beyond,pc,pd,pivot,pe,pf,size,compar,
                            context,swapf,alignsize,size_ratio,options,
                            &eq2,&gt2);
                    merge_partitions(base,first,*peq,*pgt,mid,eq2,gt2,beyond,
                            size,swapf,alignsize,size_ratio,options,peq,pgt);
                }
            }
        break;
    }
}

/* Special-case selection of minimum, maximum, or both. */
/* called from quickselect_loop */
/* Keeps track of (and relocates if necessary) elements comparing equal to the
   desired (minimum or maximum) element if ppeq,ppgt are not NULL.
*/
static
inline
errno_t select_min_s(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *,void *), void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, char **ppeq, char **ppgt)
{
    errno_t ret=0;
    if (beyond>first+1UL) {
        char *mn, *p, *q, *r;
        mn=p=base+first*size,q=p+size,r=base+beyond*size;
        /* both ppeq and ppgt are NULL, or both are non-NULL */
        A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
        if (NULL==ppeq) { /* non-partitioning */
            for (; q<r; q+=size)
                if (0<compar(mn,q,context)) mn=q;
            if (mn!=p) {
                if (0U==((QUICKSELECT_STABLE)&options)) {
                    EXCHANGE_SWAP(swapf,p,mn,size,alignsize,size_ratio,nsw++);
                } else {
                    /* |      mn      | */
                    protate(p,mn-size,mn,size,swapf,alignsize,size_ratio);
                }
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            for (; q<r; q+=size) {
                int c=compar(mn,q,context);
                if (0>c) continue;
                if (0==c) {
                    mn+=size;
                    if (q==mn) continue;
                } else /* 0<c */ mn=p; /* new min */
                A(mn!=q);
                EXCHANGE_SWAP(swapf,mn,q,size,alignsize,size_ratio,nsw++);
            }
            if (NULL!=ppeq) *ppeq=p; /* leftmost min */
            if (NULL!=ppgt) *ppgt=mn+size; /* beyond rightmost min */
        }
    } else { /* beyond==first+1UL; 1 element */
        if (NULL!=ppeq) *ppeq=base+first*size;
        if (NULL!=ppgt) *ppgt=base+beyond*size;
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

static
inline
errno_t select_max_s(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *,void *), void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, char **ppeq, char **ppgt)
{
    errno_t ret=0;
    if (beyond>first+1UL) {
        char *mx, *p, *r;
        p=base+first*size,mx=r=base+(beyond-1UL)*size;
        /* both ppeq and ppgt are NULL, or both are non-NULL */
        A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
        if ((NULL==ppeq)&&(NULL==ppgt)) { /* non-partitioning */
            for (; p<r; p+=size)
                if (0>compar(mx,p,context)) mx=p;
            if (mx!=r) {
                if (0U==((QUICKSELECT_STABLE)&options)) {
                    EXCHANGE_SWAP(swapf,r,mx,size,alignsize,size_ratio,nsw++);
                } else {
                    /* |      mx      | */
                    protate(mx,mx+size,r,size,swapf,alignsize,size_ratio);
                }
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            char *q=r-size;
            for (q=r-size; q>=p; q-=size) {
                int c=compar(mx,q,context);
                if (0<c) continue;
                if (0==c) {
                    mx-=size;
                    if (q==mx) continue;
                } else /* 0>c */ mx=r; /* new max */
                A(mx!=q);
                EXCHANGE_SWAP(swapf,mx,q,size,alignsize,size_ratio,nsw++);
            }
            if (NULL!=ppeq) *ppeq=mx; /* leftmost max */
            if (NULL!=ppgt) *ppgt=r+size; /* beyond rightmost max */
        }
    } else { /* beyond==first+1UL; 1 element */
        if (NULL!=ppeq) *ppeq=base+first*size;
        if (NULL!=ppgt) *ppgt=base+beyond*size;
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

/* Recursive divide-and-conquer method to find both minimum and maximum.
   Split into two parts; (recursively) find minimum and maximum of each part.
   Overall minimum is the smaller of the two minima; overall maximum is the
   larger of the two maxima.
*/
static
inline
errno_t find_minmax_s(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *,const void *,void *), void *context, char **pmn,
    char **pmx)
{
    errno_t ret=0;
    A(beyond>first);
    {
        size_t nmemb=beyond-first;
        if (1UL<nmemb) {
            char *mn, *mx;
            if (2UL<nmemb) {
                /* split; divide-and-conquer saves comparisons */
                size_t na=(nmemb>>1);
                char *mna, *mnb, *mxa, *mxb;
                /* find min and max of both parts */
                find_minmax_s(base,first,first+na,size,compar,context,&mna,&mxa);
                find_minmax_s(base,first+na,beyond,size,compar,context,&mnb,&mxb);
                /* overall min is smaller of *mna, *mnb; similarly for max */
                /* stability requires choosing mna if mna compares equal to mnb */
                if (0<compar(mna,mnb,context)) mn=mnb; else mn=mna;
                /* stability requires choosing mxb if mxa compares equal to mxb */
                if (0<compar(mxa,mxb,context)) mx=mxa; else mx=mxb;
            } else { /* nmemb==2UL */
                char *a, *z;
                /* first and last (i.e. second) elements */
                a=base+first*size,z=a+size;
                if (0<compar(a,z,context)) mn=z,mx=a; else mn=a,mx=z; /* stable */
            }
            *pmn=mn, *pmx=mx;
        } else /* 1 element; min==max */
            *pmn=*pmx=base+first*size;
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

/* Selection of both minimum and maximum using recursive find_minmax. */
static
inline
errno_t select_minmax_s(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *,void *), void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    errno_t ret=0;
    char *mn, *mx, *a, *z;
    A(beyond>first);
    /* first and last elements */
    a=base+first*size,z=base+(beyond-1UL)*size;
    ret=find_minmax_s(base,first,beyond,size,compar,context,&mn,&mx);
    if (0!=ret) return ret; /* shouldn't happen */
    /* put min and max in place with at most 2 swaps */
    if ((mn!=a)||(mx!=z)) {
        /* special case: mx==a and mn==z */
        if ((mx==a)&&(mn==z)) {
            EXCHANGE_SWAP(swapf,a,z,size,alignsize,size_ratio,nsw++);
        } else { /* rotations required to preserve stability */
            if (0U!=((QUICKSELECT_STABLE)&options)) {
                if (a!=mn) {
                    protate(a,mn,mn+size,size,swapf,alignsize,size_ratio);
                    if (mx<mn) mx+=size; /* mx was moved by rotation */
                }
                if (z!=mx)
                    protate(mx,mx+size,z+size,size,swapf,alignsize,size_ratio);
            } else {
                if (a!=mn)
                    EXCHANGE_SWAP(swapf,a,mn,size,alignsize,size_ratio,nsw++);
                if (z!=mx) /* beware case where a was mx before above swap! */
                    EXCHANGE_SWAP(swapf,z,a==mx?mn:mx,size,alignsize,size_ratio,
                        nsw++);
            }
        }
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

/* function to consolidate logic and calls to special-case selection functions */
/* also sets sampling table and index, determines whether to sort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int special_cases_s(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void*,const void*,void*), void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, size_t nmemb, const size_t *pk, size_t firstk,
    size_t beyondk, unsigned int options, char **ppeq, char **ppgt,
    struct sampling_table_struct **ppst, unsigned int *psort,
    unsigned int *pindex)
{
    int ret=0; /* caller continues */
    if (NULL!=pk) { /* selection */
        /* Check for special-case selection: 1-2 order statistic ranks */
        size_t nk=beyondk-firstk;
        if ((3UL>nk)&&((2UL>nk)||(NULL==ppeq))) {
            size_t fk=pk[firstk],  /* first (smallest) rank requested */
                ek=pk[beyondk-1UL],/* end rank requested */
                sp=first+1UL,      /* second element rank */
                lp=beyond-1UL,     /* last element rank */
                pp=beyond-2UL;     /* penultimate element rank */
            A(fk>=first);A(ek<beyond); /* arg sanity */
            A((beyondk==firstk+1UL)||(fk!=ek)); /* no duplicate rank requests */
            switch (nk) {
                case 1UL : /* any of smallest 2 or largest 2 */
                    if ((fk==first)||(fk==sp)) {
                        select_min_s(base,first,beyond,size,compar,context,
                            swapf,alignsize,size_ratio,options,ppeq,ppgt);
                        if (fk==sp) {
                            select_min_s(base,sp,beyond,size,compar,context,
                                swapf,alignsize,size_ratio,options,ppeq,ppgt);
                            if ((NULL!=ppeq/*)&&(NULL!=ppgt)*/)
                            &&(0==compar(*ppeq-size,*ppeq,context)))
                                (*ppeq)-=size;
                        }
                        return 1 ;
                    } else if ((ek==lp)||(ek==pp)) {
                        select_max_s(base,first,beyond,size,compar,context,
                            swapf,alignsize,size_ratio,options,ppeq,ppgt);
                        if (ek==pp) {
                            select_max_s(base,first,lp,size,compar,context,
                                swapf,alignsize,size_ratio,options,ppeq,ppgt);
                            if ((NULL!=ppeq/*)&&(NULL!=ppgt)*/)
                            &&(0==compar(*ppeq,*ppgt,context)))
                                (*ppgt)+=size;
                        }
                        return 1 ;
                    }
                break;
                case 2UL : /* 2 smallest, 2 largest, or smallest & largest */
                    A(NULL==ppeq);
                    if (fk==first) {
                        if (ek==lp) { /* smallest & largest */
                            select_minmax_s(base,first,beyond,size,compar,
                                context,swapf,alignsize,size_ratio,options);
                            return 1 ;
                        } else if (ek==sp) { /* 2 smallest */
                            select_min_s(base,first,beyond,size,compar,context,
                                swapf,alignsize,size_ratio,options,NULL,NULL);
                            select_min_s(base,sp,beyond,size,compar,context,
                                swapf,alignsize,size_ratio,options,NULL,NULL);
                            return 1 ;
                        }
                    } else if ((fk==pp)&&(ek==lp)) { /* 2 largest */
                        select_max_s(base,first,beyond,size,compar,context,
                            swapf,alignsize,size_ratio,options,NULL,NULL);
                        select_max_s(base,first,lp,size,compar,context,swapf,
                            alignsize,size_ratio,options,NULL,NULL);
                        return 1 ;
                    }
                break;
            }
        }
    }
    A(*pindex < (SAMPLING_TABLE_SIZE));
    *ppst=d_sampling_table(first,beyond,pk,firstk,beyondk,ppeq,psort,
        pindex,nmemb);
    return ret;
}

static inline errno_t quickselect_loop_s(char *base,size_t first,size_t beyond,
    const size_t size, int(*compar)(const void *,const void *,void *),
    void *context, const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, unsigned int options, char **ppeq, char **ppgt)
{
    errno_t ret=0;
    size_t lk=beyondk, nmemb=beyond-first, rk=firstk;
    auto int c=0; /* repivot factor2 count */
    unsigned int table_index=2U;

    A(2UL<=nmemb);
    A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
    /* both or neither NULL */
    for (;;) {
        size_t p, q, r, s, t;
        struct sampling_table_struct *pst;
        struct region_struct lt_region, gt_region, *ps_region, *pl_region;
        unsigned int sort;
        char *pc, *pd, *pe, *pf, *pivot;

        A(first<beyond);
        A(table_index < (SAMPLING_TABLE_SIZE));
        if (0!=special_cases_s(base,first,beyond,size,compar,context,swapf,
           alignsize,size_ratio,cutoff,nmemb,pk,firstk,beyondk,options,ppeq,
           ppgt,&pst,&sort,&table_index))
           return ret;
        A((SAMPLING_TABLE_SIZE)>table_index);
        if (0U!=sort) { /* switch to sorting */
            if (nmemb<=cutoff) {
                A(1UL<nmemb);
                ret=dedicated_sort_s(base,first,beyond,size,compar,context,
                    swapf,alignsize,size_ratio,options);
                return ret; /* done */
            }
            pk=NULL; /* sorting */ A(pst==sorting_sampling_table);
            /* proceed with pivot selection */
        }

        pivot=select_pivot_s(base,first,beyond,size,compar,context,swapf,
            alignsize,size_ratio,pst,table_index,pk,options,&pc,
            &pd,&pe,&pf);
        A(NULL!=pivot); /* shouldn't get an error for internal calls! */
        if (NULL==pivot) return ret=errno; /* errno was set to return from
                                              quickselect_loop_s */
        A(base+first*size<=pivot);A(pivot<base+beyond*size);
        A(pc<=pd);A(pd<pe);A(pe<=pf);
        A(pd<=pivot);A(pivot<pe);

        /* partition the array around the pivot element */
        lt_region.first=first, gt_region.beyond=beyond;
        partition_s(base,first,beyond,pc,pd,pivot,pe,pf,size,
            compar,context,swapf,alignsize,size_ratio,options,
            &(lt_region.beyond),&(gt_region.first));

        if (NULL!=pk) { /* selection only */
            rank_tests(base,first,lt_region.beyond,gt_region.first,beyond,size,
                pk,firstk,beyondk,&(lt_region.beyondk),&(gt_region.firstk),ppeq,
                ppgt);
            lt_region.firstk=firstk, gt_region.beyondk=beyondk;
        } else lt_region.firstk=lt_region.beyondk=gt_region.firstk
            =gt_region.beyondk=0UL;

        /* Which region(s) should be processed? */
        lt_region.process=(((lt_region.first+1UL<lt_region.beyond)
            &&((NULL==pk)||(lt_region.firstk<lt_region.beyondk)))?1U:0U);
        gt_region.process=(((gt_region.first+1UL<gt_region.beyond)
            &&((NULL==pk)||(gt_region.firstk<gt_region.beyondk)))?1U:0U);

        /* Categorize less-than and greater-than regions as small and large. */
        if (lt_region.beyond-lt_region.first<gt_region.beyond-gt_region.first) {
            ps_region=&lt_region, pl_region=&gt_region;
        } else {
            ps_region=&gt_region, pl_region=&lt_region;
        }

        if (0U!=((QUICKSELECT_RESTRICT_RANK)&options))
            c=0; /* reset factor2 count if pivot was median-of-medians */
        options&=~(QUICKSELECT_RESTRICT_RANK); /* default is array samples */

        /* Process less-than and/or greater-than regions by relative size. */
        if (0U!=pl_region->process) { /* Process large region. */
            if (0U!=ps_region->process) { /* Recurse on small region. */
                nrecursions++;
                (V)quickselect_loop_s(base,ps_region->first,ps_region->beyond,
                    size,compar,context,pk,ps_region->firstk,ps_region->beyondk,
                    swapf,alignsize,size_ratio,cutoff,options,ppeq,ppgt);
            }
            /* Dedicated sort for large region? */
            if ((NULL==pk)&&(pl_region->beyond<=pl_region->first+cutoff)) {
                A(pl_region->first+1UL<pl_region->beyond);
                ret=dedicated_sort_s(base,pl_region->first,pl_region->beyond,
                    size,compar,context,swapf,alignsize,size_ratio,options);
                return ret; /* done */
            }
            /* Iterate on large region. */
            /* Should large region be repivoted? */
            /* Determine whether or not to repivot/repartition region of size r
               elements (large region) resulting from a partition of nmemb
               elements.  Assumed that continued processing of the region w/o
               repivoting will yield a similarly poor split.  Repivot if the
               cost of repivoting plus processing the resulting regions is
               expected to be less than the cost of processing the region w/o
               repivoting.
            */
            first=pl_region->first, beyond=pl_region->beyond,
                firstk=pl_region->firstk, beyondk=pl_region->beyondk;
            if (0U!=((QUICKSELECT_RESTRICT_RANK)&options)) {
            }
            sort=
                d_should_repivot(nmemb,beyond-first,cutoff,pst,table_index,
                pk,&c,ppeq);
            options |= sort;
            if (0U!=((QUICKSELECT_RESTRICT_RANK)&options)) {
                nrepivot++;
            }
        } else { /* large region to be ignored; maybe iterate small region */
            if (0U!=ps_region->process) { /* Iterate on small region. */
                first=ps_region->first, beyond=ps_region->beyond,
                    firstk=ps_region->firstk, beyondk=ps_region->beyondk;
            } else {
                return ret; /* Process neither; nothing left to do. */
            }
        }
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}
/* public interfaces for N1570 draft extensions */
errno_t quickselect_s_internal(void *base, rsize_t nmemb, /*const*/ rsize_t size,
    int (*compar)(const void *,const void *,void *), void *context,
    size_t *pk, size_t nk, unsigned int options)
{
    errno_t ret=0;
    unsigned int sort;
    unsigned int table_index=2U; /* optimized for small nmemb */
    size_t onk=nk; /* original nk value */
    size_t s=0UL;  /* rotation amount */
    struct sampling_table_struct *pst;
    size_t alignsize, size_ratio;
    void (*swapf)(char *, char *, size_t);
    char **ppeq=NULL, **ppgt=NULL; /* gcc pukes w/o this */
    size_t cutoff;

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || ((0UL!=nk) && (NULL==pk))
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size) || (RSIZE_MAX<nk)
    ) {
        constraint_handler_t foo;
        /* N1570 K3.1.3 may set errno */
        errno=ret=EINVAL;
        /* Runtime-constraint violation handler to be called, but what is it?
           Only interface is set_constraint_handler_s, which CHANGES the
           handler.  N.B. determining the handler either before or after the
           violation might not represent the handler in effect AT THE TIME OF
           the violation, if the handler is not per-thread and/or in the absence
           of cooperating thread-based mutexes to constrain changes to the
           handler (or if even one thread fails to cooperate by failing to use a
           common mutex).
        */
        foo=set_constraint_handler_s(NULL); /* returns pointer to handler
                                               function, sets default handler
                                               (which might not be the current
                                               handler) */
        /* Quickly (N.B. there is a window of vulnerability here during which
           the handler has been changed.  It is undefined whether the handler is
           global, or per-process, or per-thread. Other threads or processes
           might therefore be affected.  Also, another process or thread might
           change the handler during this window; resetting the handler (which
           has been changed by the above call to set_contraint_handler_s) might
           undo any such change.  In combination, two (or more!) threads or
           processes calling set_constraint_handler_s to determine what the
           handler is might interfere with the other thread(s) or process(es),
           possibly resulting in incorrect inferences about the handler and/or
           unintended changes to the handler.) reset the handler to foo.
        */
        (void)set_constraint_handler_s(foo); /* reset to (presumed) previous
                                                handler */
        /* Finally, now call what might (or might not) have been the handler at
           the time of the runtime-constraint violation.
        */
        foo("invalid parameters",NULL,errno=ret); /* reset errno in case it was
                                                     changed during calls to
                                                     set_constraint_handler_s
                                                  */
        /* if the handler returns, return error code */
        return ret;
    }

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (2UL>nmemb) return ret; /* Return early if there's nothing to do. */

    /* Simplify tests for selection vs. sorting by ensuring a NULL pointer when
       sorting. Ensure consistency between pk and nk. Ensure sorted pk array
       with no duplicated order statistics.
    */
    if (0UL==nk) pk=NULL;
    else if (NULL==pk) onk=nk=0UL;
    else if (1UL<nk) { /* binary search requires nondecreasing pk */
        size_t p, q;
        for (p=0UL,q=1UL; q<nk; p++,q++) /* verify (linear scan with direct comparison) */
            if (pk[p]>pk[q]) break; /* not nondecreasing */
        if (q<nk) /* fix (sort) iff broken (not nondecreasing) */
            QSORT_FUNCTION_NAME ((void *)pk,nk,sizeof(size_t),size_tcmp);
        /* verify, fix uniqueness */
        alignsize=alignment_size((char *)pk,sizeof(size_t));
        size_ratio=sizeof(size_t)/alignsize;
        swapf=swapn(alignsize);
        for (p=0UL,q=1UL; q<=nk; q++) {
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    irotate((char *)pk,p,q,nk,sizeof(size_t),swapf,alignsize,size_ratio);
                    s=q-p, nk-=s, q=p;
                }
            }
        }
    }

    /* alignment size and swap function */
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smaller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    A(table_index < (SAMPLING_TABLE_SIZE));
    pst=d_sampling_table(0UL,nmemb,pk,0UL,nk,NULL,&sort,&table_index,nmemb);
    if (0UL!=quickselect_small_array_cutoff)
        cutoff=quickselect_small_array_cutoff;
    else cutoff=cutoff_value(size_ratio,options);

    ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,pk,0UL,nk,
        swapf,alignsize,size_ratio,cutoff,options,NULL,NULL);

    /* Restore pk to full sorted (non-unique) order for caller convenience. */
    /* This may be expensive if the caller supplied a large number of duplicate
       order statistic ranks.  So be it.  Caller: if it hurts, don't do it.
    */
restore_s: ;
    if (0UL!=s) /* there were duplicates */
        QSORT_FUNCTION_NAME ((void *)pk,onk,sizeof(size_t),size_tcmp);
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

errno_t qsort_s_internal (void *base, size_t nmemb, /*const*/ size_t size,
    int (*compar)(const void *,const void *, void *), void *context)
{
    errno_t ret=0;
    unsigned int table_index=1U; /* optimized for small nmemb */
    struct sampling_table_struct *pst=sorting_sampling_table;
    size_t alignsize, size_ratio;
    void (*swapf)(char *, char *, size_t);
    size_t cutoff;

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size)
    ) {
        constraint_handler_t foo;
        /* N1570 K3.1.3 may set errno */
        errno=ret=EINVAL;
        /* Runtime-constraint violation handler to be called, but what is it?
           Only interface is set_constraint_handler_s, which CHANGES the
           handler.  N.B. determining the handler either before or after the
           violation might not represent the handler in effect AT THE TIME OF
           the violation, if the handler is not per-thread and/or in the absence
           of cooperating thread-based mutexes to constrain changes to the
           handler (or if even one thread fails to cooperate by failing to use a
           common mutex).
        */
        foo=set_constraint_handler_s(NULL); /* returns pointer to handler
                                               function, sets default handler
                                               (which might not be the current
                                               handler) */
        /* Quickly (N.B. there is a window of vulnerability here during which
           the handler has been changed.  It is undefined whether the handler is
           global, or per-process, or per-thread. Other threads or processes
           might therefore be affected.  Also, another process or thread might
           change the handler during this window; resetting the handler (which
           has been changed by the above call to set_contraint_handler_s) might
           undo any such change.  In combination, two (or more!) threads or
           processes calling set_constraint_handler_s to determine what the
           handler is might interfere with the other thread(s) or process(es),
           possibly resulting in incorrect inferences about the handler and/or
           unintended changes to the handler.) reset the handler to foo.
        */
        (void)set_constraint_handler_s(foo); /* reset to (presumed) previous
                                                handler */
        /* Finally, now call what might (or might not) have been the handler at
           the time of the runtime-constraint violation.
        */
        foo("invalid parameters",NULL,errno=ret); /* reset errno in case it was
                                                     changed during calls to
                                                     set_constraint_handler_s
                                                  */
        /* if the handler returns, return error code */
        return ret;
    }

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (1UL<nmemb) { /* nothing to do for fewer than 2 elements */
#if 0
        /* Initialization of strings is performed here (rather than in
           quickselect_loop) so that quickselect_loop can be made inline.
        */
        if ((char)0==quickselect_initialized) initialize_quickselect();
#endif

        /* alignment size and swap function */
        alignsize=alignment_size(base,size);
        size_ratio=size/alignsize;
        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

        if (cutoff<nmemb) {
            /* Initialize the sampling table index for the array size. Sub-array
               sizes will be smaller, and this step ensures that the main loop
               won't have to traverse much of the table during recursion and
               iteration.
            */
            /* sampling table isn't used for dedicated sort */
            while (nmemb>pst[table_index].max_nmemb) table_index++;
            if (0UL!=quickselect_small_array_cutoff)
                cutoff=quickselect_small_array_cutoff;
            else cutoff=cutoff_value(size_ratio,0x07F8U);

            ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,NULL,0UL,0UL,swapf,
                alignsize,size_ratio,cutoff,0x07F8U,NULL,NULL);
        } else
            ret=dedicated_sort_s(base,0UL,nmemb,size,compar,context,swapf,
                alignsize,size_ratio,0x07F8U);
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}
#endif /* RSIZE_MAX */
