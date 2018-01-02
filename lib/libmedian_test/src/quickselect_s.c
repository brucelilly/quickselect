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
/* $Id: ~|^` @(#)   This is quickselect_s.c version 1.9 dated 2017-12-22T14:01:59Z. \ $ */
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
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2017-12-22T14:01:59Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

#define __STDC_WANT_LIB_EXT1__ 1
/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* structures */
/* regions resulting from partitioning */
struct region_struct {
    size_t first;  /* base array */
    size_t beyond;
    size_t firstk; /* order statistics */
    size_t beyondk;
    unsigned char process; /* 0=false */
};

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
    int(*compar)(const void *,const void *,void *), void *context,
    unsigned int options)
{
    int c;

#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(MEDIAN_DEBUG)) {
(V)fprintf(stderr, "/* %s: %s line %d: pa=%p, pb=%p, pc=%p */\n",
__func__,source_file,__LINE__,(void *)pa,(void *)pb,(void *)pc);
}
#endif
    A(0U==(options&(QUICKSELECT_INDIRECT)));
    c=COMPAR(pa,pb,context);
    if (0!=c) {
        int d=COMPAR(pb,pc,context);
        if (0!=d) {
            if ((0<d)&&(0>c)) {
                if (0>COMPAR(pa,pc,context)) return pc;
                else return pa;
            } else if ((0>d)&&(0<c)) {
                if (0<COMPAR(pa,pc,context)) return pc;
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
    void *context, unsigned int options)
{
    size_t o;
    A((SAMPLING_TABLE_SIZE)>idx);
    if (0U < --idx) {
        char *pa, *pb, *pc;
        size_t s=sample_spacing/3UL;

        o=s*size;
        pa=remedian_s(middle-o,row_spacing,s,size,idx,compar,context,options);
        pb=remedian_s(middle,row_spacing,s,size,idx,compar,context,options);
        pc=remedian_s(middle+o,row_spacing,s,size,idx,compar,context,options);
#if QUICKSELECT_INDIRECT
        if (0U==(options&(QUICKSELECT_INDIRECT)))
#endif /* QUICKSELECT_INDIRECT */
            return fmed3_s(pa,pb,pc,compar,context,options);
#if QUICKSELECT_INDIRECT
        else {
            char *pr= fmed3_s(*((char **)pa),*((char **)pb),*((char **)pc),
                compar,context,options&~(QUICKSELECT_INDIRECT));
            if (pr==*((char **)pa)) return pa;
            if (pr==*((char **)pc)) return pc;
            return pb;
        }
#endif /* QUICKSELECT_INDIRECT */
    }
    o=row_spacing*size;
#if QUICKSELECT_INDIRECT
    if (0U==(options&(QUICKSELECT_INDIRECT)))
#endif /* QUICKSELECT_INDIRECT */
        return fmed3_s(middle-o,middle,middle+o,compar,context,options);
#if QUICKSELECT_INDIRECT
    else {
        char *pa, *pc, *pr;
        pa=middle-o, pc=middle+o;
        pr = fmed3_s(*((char **)pa),*((char **)middle),*((char **)pc),compar,
            context,options&~(QUICKSELECT_INDIRECT));
        if (pr==*((char **)pa)) return pa;
        if (pr==*((char **)pc)) return pc;
        return middle;
    }
#endif /* QUICKSELECT_INDIRECT */
}

/* quickselect_loop_s forward declaration (for select_pivot_s) */
static inline
errno_t quickselect_loop_s(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *,void *),
    void *context, const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, unsigned int options, char *conditions,
    size_t *indices, char *element, char **ppeq, char **ppgt);

/* select_pivot_s using remedian_s or median-of-medians */
/* Called from both loop functions. */
/* return NULL if median-of-medians median selection fails */
static inline
char *select_pivot_s(char *base, size_t first, size_t beyond,
    register size_t size,int (*compar)(const void *,const void*,void*),
    void *context, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, struct sampling_table_struct *pst,
    unsigned int table_index, const size_t *pk, unsigned int options,
    char **ppc, char **ppd, char **ppe, char **ppf)
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
                pivot=remedian_s(pivot,r,r,size,table_index,compar,context,options);
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
                /*3 element sets (columns); medians->1st row, ignore leftovers*/
                for (o=0UL,n=r*size; o<n; o+=size) {
                    pa=pc+o;
                    pb=pa+n; /* middle element */
                    A(pb+n<base+beyond*size);
                    /* first element (pa) bias */
#if QUICKSELECT_INDIRECT
                    if (0U==(options&(QUICKSELECT_INDIRECT))) {
#endif /* QUICKSELECT_INDIRECT */
                        pm=fmed3_s(pb,pa,pb+n,compar,context,options);
                        if (pm!=pa) {
                            EXCHANGE_SWAP(swapf,pm,pa,size,alignsize,size_ratio,
                                nsw++); /*medians at start of sub-array*/
                        }
#if QUICKSELECT_INDIRECT
                    } else {
                        pm=fmed3_s(*((char **)pb),*((char **)pa),
                            *((char **)(pb+n)),compar,context,
                            options&~(QUICKSELECT_INDIRECT));
                        /*medians at start of sub-array*/
                        /*compare returned data pointer,swap indirect pointers*/
                        if (pm!=*((char **)pa)) {
                            if (pm==*((char **)pb)) {
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                                    size_ratio,nsw++);
                            } else {
                                EXCHANGE_SWAP(swapf,pa,pb+n,size,alignsize,
                                    size_ratio,nsw++);
                            }
                        }
                    }
#endif /* QUICKSELECT_INDIRECT */
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
                /* options wrangling: QUICKSELECT_STABLE doesn't apply here
                   (can't use median-of-medians if stability is required),
                   reset QUICKSELECT_RESTRICT_RANK for initial partition;
                   remaining options are network bits, QUICKSELECT_INDIRECT,
                   and QUICKSELECT_OPTIMIZE_COMPARISONS
                */
                A(0U==(options&(QUICKSELECT_STABLE)));
                options &= ~(QUICKSELECT_STABLE | QUICKSELECT_RESTRICT_RANK);
                if (0U!=save_partial)
                    (V)quickselect_loop_s(base,first,first+r,size,compar,
                        context,karray,0UL,1UL,swapf,alignsize,size_ratio,5UL,
                        options,NULL,NULL,NULL,ppd,ppe);
                else
                    (V)quickselect_loop_s(base,first,first+r,size,compar,
                        context,karray,0UL,1UL,swapf,alignsize,size_ratio,5UL,
                        options,NULL,NULL,NULL,NULL,NULL);
                pivot=base+karray[0]*size; /* pointer to median of medians */
                /* First third of array (medians) is partitioned. */
                if (0U==save_partial) *ppe=(*ppd=pivot)+size;
            }
        break;
    }
    A(*ppd<=pivot);A(pivot<*ppe);
    return pivot;
}

/* Insertion sort using binary search to locate insertion position for
   out-of-order element, followed by rotation to insert the element in position.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void isort_bs(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *,const void *, void *), void *context,
    void (*swapf)(char *,char *,size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    register size_t l, m, h, n, u=beyond-1UL;
    char *pa, *pu=base+u*size;

#if QUICKSELECT_INDIRECT
    if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
#endif /* QUICKSELECT_INDIRECT */
        for (n=u,pa=pu-size; n>first; pa-=size) {
            --n;
            if (0<COMPAR(pa,pa+size,context)) { /* skip over in-order */
                l=n+2UL;
                if (l>u) l=beyond; /* simple swap */
                else { /* binary search for insertion position */
                    for (h=u,m=l+((h-l)>>1); l<=h; m=l+((h-l)>>1)) {
                        if (0>=COMPAR(pa,base+m*size,context)) h=m-1UL;
                        else l=m+1UL;
                    } A(n!=l);
                }
                /* Insert element now at n at position before l by rotating elements
                   [n,l) left by 1.
                */
                irotate(base,n,n+1UL,l,size,swapf,alignsize,size_ratio);
            }
        }
#if QUICKSELECT_INDIRECT
    } else { /* indirect */
        register char *p;
        for (n=u,pa=pu-size; n>first; pa-=size) {
            --n;
            p=*((char **)pa);
            if (0<COMPAR(p,*((char**)(pa+size)),context)) {/* skip over in-order */
                l=n+2UL;
                if (l>u) l=beyond; /* simple swap */
                else { /* binary search for insertion position */
                    for (h=u,m=l+((h-l)>>1); l<=h; m=l+((h-l)>>1)) {
                        if (0>=COMPAR(p,*((char**)(base+m*size)),context)) h=m-1UL;
                        else l=m+1UL;
                    } A(n!=l);
                }
                /* Insert element now at n at position before l by rotating elements
                   [n,l) left by 1.
                */
                irotate(base,n,n+1UL,l,size,swapf,alignsize,size_ratio);
            }
        }
    }
#endif /* QUICKSELECT_INDIRECT */
}

/* In-place merge of two adjacent sorted sub-arrays using rotation of displaced
   elements.
   --------------------     -------------------
   |  first  | second |  => | merged (sorted) |
   --------------------     -------------------
   _f___a_____b___c____u
   |   |     |   |    |
   --------------------
   [f,a) is already merged
   [a,b) is the remainder of the first sorted section
   [b,c) contains (sorted) elements displaced from first section
   [c,u) is the remainder of the second sorted section
   initially a=f, b=c
   While b==c (no elements displaced from first section), compare a,c.
   When at least one element has been displaced from the first section, the
      smallest displaced element is smaller than the element at a (because the
      first section was in sorted order), so compare b,c to determine which
      should be swapped with a.
   Rotations are required to maintain [b,c) in sorted order when a is swapped
      with b; this is expensive for large arrays.
   When a reaches b, start new merge of [a,c) with [c,u) by setting b=c.
   Merge is complete when a==c or b==u.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void inplace_merge(char *base, size_t l, size_t m, size_t u, size_t size,
    int (*compar)(const void *,const void *, void *), void *context,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    char *pl, *pm, *pr, *pu;  /* left, middle, right pointers to elements */
    int c;

    pl=base+l*size;
    pr=pm=base+m*size;
    pu=base+u*size;
#if QUICKSELECT_INDIRECT
    if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
#endif /* QUICKSELECT_INDIRECT */
        while (pr<pu) {
            A(l<=m);A(m<u);
            /* Find and swap the first element of the left side which is larger than
               the first element of the right side.  That element is almost always
               within the first few elements, so use a linear scan (binary search
               would be counterproductive).
            */
            for (; pl<pm; pl+=size) {
                A(pl>=base+l*size);A(pl<pm);
                c=COMPAR(pl,pm,context);
                A(c>-2);A(c<2);
                if (0<c) {
                    /* *pl>*pm: displace it */
                    EXCHANGE_SWAP(swapf,pl,pm,size,alignsize,size_ratio,/**/);
                    pr+=size;
                    break;
                }
            }
            if (pl==pm) break; /* all on left <= smallest on right (@m); done */
            pl+=size;
            /* displaced element @ pm, 2nd region starts @ pr, next to merge @ pl */
            while (pl<pm) {
                if ((pr<pu)&&(0<COMPAR(pm,pr,context))) {
                    EXCHANGE_SWAP(swapf,pl,pr,size,alignsize,size_ratio,/**/);
                    pr+=size;
                } else {
                    EXCHANGE_SWAP(swapf,pl,pm,size,alignsize,size_ratio,/**/);
                    if (pr>pm+size)
                        protate(pm,pm+size,pr,size,swapf,alignsize,size_ratio);
                }
                pl+=size;
            }
            pm=pr;
        }
#if QUICKSELECT_INDIRECT
    } else { /* indirect */
        while (pr<pu) {
            register char *p;
            p=*((char **)pm);
            A(l<=m);A(m<u);
            /* Find and swap the first element of the left side which is larger than
               the first element of the right side.  That element is almost always
               within the first few elements, so use a linear scan (binary search
               would be counterproductive).
            */
            for (; pl<pm; pl+=size) {
                A(pl>=base+l*size);A(pl<pm);
                c=COMPAR(*((char**)(pl)),p,context);
                A(c>-2);A(c<2);
                if (0<c) {
                    /* *pl>*pm: displace it */
                    EXCHANGE_SWAP(swapf,pl,pm,size,alignsize,size_ratio,/**/);
                    pr+=size;
                    break;
                }
            }
            if (pl==pm) break; /* all on left <= smallest on right (@m); done */
            pl+=size;
            /* displaced element @ pm, 2nd region starts @ pr, next to merge @ pl */
            while (pl<pm) {
                if ((pr<pu)&&(0<COMPAR(p,*((char**)(pr)),context))) {
                    EXCHANGE_SWAP(swapf,pl,pr,size,alignsize,size_ratio,/**/);
                    pr+=size;
                } else {
                    EXCHANGE_SWAP(swapf,pl,pm,size,alignsize,size_ratio,/**/);
                    if (pr>pm+size)
                        protate(pm,pm+size,pr,size,swapf,alignsize,size_ratio);
                }
                pl+=size;
            }
            pm=pr;
        }
    }
#endif /* QUICKSELECT_INDIRECT */
}

static inline errno_t dedicated_sort_s(char *base, size_t first, size_t beyond,
    /*const*/ size_t size, int (*compar)(const void *,const void *,void *),
    void *context, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options)
{
    errno_t ret=0;
    if (beyond>first) {
        size_t n=beyond-first;
        char *pa, *pb;

        if ((12UL<n)
        ||((2UL<n)
          &&((0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS)))
             ||(0U==(options&(0x01U<<n)&(QUICKSELECT_NETWORK_MASK)))
            )
          )
        ) {
            /* Insertion sort (for speed in some cases) or merge sort (for
               minimum number of comparisons); both are stable.
            */
            if ((155UL<=n)
               ||(0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS)))
            ) {
                /* in-place merge sort using rotation */
                size_t mid, na;
                /* split */
                na=(n>>1); mid=first+na;
                /* use recursion to sort pieces */
                if (1UL<na) {
                    ret= dedicated_sort_s(base,first,mid,size,compar,context,
                        swapf,alignsize, size_ratio, options);
                    if (0!=ret) return ret;
                }
                A(first+na+1UL<beyond);
                ret= dedicated_sort_s(base,mid,beyond,size,compar,context,swapf,
                    alignsize, size_ratio, options);
                if (0!=ret) return ret;
                /* merge pieces */
                inplace_merge(base,first,mid,beyond,size,compar,context,
                    swapf,alignsize,size_ratio,options);
            } else {
                /* Insertion sort iterative loop. */
                /* Implementation by insertion of first out-of-place element
                   into sorted remainder (except for trivial case).
                */
                /* Find extent of rightmost sorted run of elements. */
                /* n is the index of the out-of-place element immediately to
                   the left of the rightmost sorted run of elements
                */
                isort_bs(base,first,beyond,size,compar,context,swapf,alignsize,
                    size_ratio,options);
            }
            return ret ;
        }
        switch (n) {
            default : /* shouldn't happen... */
                A(2UL<=n);
                errno=ret=EINVAL;
            return ret;
            case 2UL : /* 1 comparison, <=1 swap; less overhead than default */
                pa=base+first*size;
                pb=pa+size;
                COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio);
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
                    c=OPT_COMPAR(pa,pb,options,context),
                        d=OPT_COMPAR(pb,pc,options,context);
                    if (0>=c) { /*b>=a*/
                        if (0<d) { /*b>c*/
                            if (0==c) { /*a==b*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,nsw++);
                            } else { /*b>a,b>c*/
                                if (0<OPT_COMPAR(pa,pc,options,context)) { /*a>c*/
                                    EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                        size_ratio,nsw++);
                                }
                                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,
                                    size_ratio,nsw++);
                            }
                        }
                    } else { /*a>b*/
                        if (0<=d) { /*b>=c*/
                            EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                        } else { /*c>b*/
                            if (0<OPT_COMPAR(pa,pc,options,context)) { /*a>c*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,nsw++);
                            }
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
                    c=OPT_COMPAR(pa,pc,options,context),
                        d=OPT_COMPAR(pb,pc,options,context);
                    if (0>=c) { /*c>=a*/
                        if (0<d) { /*b>c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                nsw++);
                        } else if (0>d) { /*b<c*/
                            if ((0==c)
                            ||(0<OPT_COMPAR(pa,pb,options,context))
                            ) { /*a==c||a>b*/
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                                    size_ratio,nsw++);
                            }
                        }
                    } else { /*a>c*/
                        if (0>d) { /*b<c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                nsw++);
                        } else if ((0<d)
                        &&(0>OPT_COMPAR(pa,pb,options,context))) { /*c<a<b*/
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                nsw++);
                        }
                        EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,nsw++);
                    }
                }
#endif /* FAVOR_SORTED */
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
    unsigned int options, char *conditions, size_t *indices, char *element,
    size_t *peq, size_t *pgt)
{
    char *pa, *pb, *pg, *ph, *pl, *pu;
    int c=0, d=0;

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
#if QUICKSELECT_INDIRECT
            if (0U==(options&(QUICKSELECT_INDIRECT))) {
#endif /* QUICKSELECT_INDIRECT */
                if (pa==pb) {
                    while ((pb<=pg)&&(0==(c=COMPAR(pivot,pb,context)))) { pb+=size; }
                    pa=pb;
                    if (0<c)
                        for (pb+=size; (pb<=pg)&&(0<=(c=COMPAR(pivot,pb,context))); pb+=size)
                            if (0==c) { A(pa!=pb);
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                            }
                } else {
                    for (; (pb<=pg)&&(0<=(c=COMPAR(pivot,pb,context))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
                }
                if (pg==ph-size) {
                    while ((pb<pg)&&(0==(d=COMPAR(pivot,pg,context)))) { pg-=size; }
                    ph=pg+size;
                    if (0>d)
                        for (pg-=size; (pb<pg)&&(0>=(d=COMPAR(pivot,pg,context))); pg-=size)
                            if (0==d) { A(pg!=ph-size);
                                ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                            }
                } else {
                    for (; (pb<pg)&&(0>=(d=COMPAR(pivot,pg,context))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
                }
                while (pb<pg) {
                    EXCHANGE_SWAP(swapf,pb,pg,size,alignsize,size_ratio,nsw++); pb+=size, pg-=size;
                    for (;(pb<=pg)&&(0<=(c=COMPAR(pivot,pb,context))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
                    for (;(pb<pg)&&(0>=(d=COMPAR(pivot,pg,context))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
                }
#if QUICKSELECT_INDIRECT
            } else { /* indirect */
                pivot=*((char **)pivot);
                if (pa==pb) {
                    while ((pb<=pg)&&(0==(c=COMPAR(pivot,*((char **)pb),context)))) { pb+=size; }
                    pa=pb;
                    if (0<c)
                        for (pb+=size; (pb<=pg)&&(0<=(c=COMPAR(pivot,*((char **)pb),context))); pb+=size)
                            if (0==c) { A(pa!=pb);
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                            }
                } else {
                    for (; (pb<=pg)&&(0<=(c=COMPAR(pivot,*((char **)pb),context))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
                }
                if (pg==ph-size) {
                    while ((pb<pg)&&(0==(d=COMPAR(pivot,*((char **)pg),context)))) { pg-=size; }
                    ph=pg+size;
                    if (0>d)
                        for (pg-=size; (pb<pg)&&(0>=(d=COMPAR(pivot,*((char **)pg),context))); pg-=size)
                            if (0==d) { A(pg!=ph-size);
                                ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                            }
                } else {
                    for (; (pb<pg)&&(0>=(d=COMPAR(pivot,*((char **)pg),context))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
                }
                while (pb<pg) {
                    EXCHANGE_SWAP(swapf,pb,pg,size,alignsize,size_ratio,nsw++); pb+=size, pg-=size;
                    for (;(pb<=pg)&&(0<=(c=COMPAR(pivot,*((char **)pb),context))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
                    for (;(pb<pg)&&(0>=(d=COMPAR(pivot,*((char **)pg),context))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
                }
            }
#endif /* QUICKSELECT_INDIRECT */
            if (pb>=pa) pd=blockmove(pl,pa,pb,swapf); else pd=pb;
            pe=blockmove(pb,ph,pu,swapf);
            A(pl<=pd); A(pd<pe); A(pe<=pu);
done: ;
            *peq=(pd-base)/size;
            *pgt=(pe-base)/size;
            A(pe>pd);
        break;
#if QUICKSELECT_STABLE
        case QUICKSELECT_STABLE :
#if QUICKSELECT_LINEAR_STABLE
            if ((NULL!=conditions)&&(NULL!=indices)) {
                /* linear-time stable partition using O(N) extra space */
                register size_t i, ieq, igt;

                pl=base+size*first, pu=base+size*beyond;
                /* 1st scan: left-to-right to get element partial order and set
                   pointer for < elements.
                */
                /* split into pre-pivot, pivot, post-pivot */
                /* further split into direct, indirect for pivot cache */
#if QUICKSELECT_INDIRECT
                if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
#endif /* QUICKSELECT_INDIRECT */
                    for (i=ieq=first,pb=pl,pg=pivot; pb<pg; pb+=size,i++) {
                        c=COMPAR(pb,pg,context);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
                    A(pb==pg);
                    conditions[i]=(char)0;
                    for (++i,pb+=size; pb<pu; pb+=size,i++) {
                        c=COMPAR(pb,pg,context);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
#if QUICKSELECT_INDIRECT
                } else { /* indirect; pivot data pointer cached */
                    pg=*((char **)pivot);
                    for (i=ieq=first,pb=pl; pb<pivot; pb+=size,i++) {
                        c=COMPAR(*((char **)pb),pg,context);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
                    A(pb==pivot);
                    conditions[i]=(char)0;
                    for (++i,pb+=size; pb<pu; pb+=size,i++) {
                        c=COMPAR(*((char **)pb),pg,context);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
                }
#endif /* QUICKSELECT_INDIRECT */
                *peq=ieq;
                /* 2nd scan: right-to-left to set pointers for > elements. */
                for (igt=beyond,i=igt-1UL,pb=pu-size; pb>=pl; pb-=size,i--)
                    if (0<conditions[i]) indices[--igt]=i;
                *pgt=igt;
                /* 3rd scan: left to right to set pointers for == elements. */
                for (i=first,pb=pl; ieq<igt; pb+=size,i++) {
                    if (0==conditions[i]) indices[ieq++]=i;
                    A(pb<pu);A(i<beyond);
                }
                i=rearrange_array(base,beyond-first,size,indices,
                    beyond-first,first,beyond,element);
                A(beyond-first>(i>>1));
                /* Update count of moves: for direct data sort/select,
                   size_ratio reflects data size compared to alignsize (which
                   is assumed to be the basic move size used by memcpy); for
                   indirect sort/select, size_ratio==1 because only pointers
                   are moved here.
                */
                if (0UL!=i)
                    nmoves+=(i+beyond-first)*size_ratio;
            } else {
#endif /* QUICKSELECT_LINEAR_STABLE */
                /* divide-and-conquer partition */
                pl=base+size*first, pu=base+size*beyond;
                /* Recursively partitions unpartitioned regions down to a partition
                   containing a single element, then merges partitions to fully
                   partition the array.
                */
                if (pl+size==pu) { /* 1 element */
                    if (pivot!=pl) { /* external pivot */
                        c=OPT_COMPAR(pl,pivot,options,context);
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
                                conditions,indices,element,peq,pgt);
                            A(*peq<=*pgt);A(ipf<=*peq);A(*pgt<=beyond);
                            merge_partitions(base,first,ipd,ipe,ipf,*peq,*pgt,
                                beyond,size,swapf,alignsize,size_ratio,options,peq,pgt);
                        } else if (pu==pf) { /* already-partitioned at right end */
                            /* partition unpartitioned left region and merge */
                            A(pl<pc);
                            partition_s(base,first,ipc,pc,pd,pivot,pe,pf,size,
                                compar,context,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                            A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipc);
                            merge_partitions(base,first,*peq,*pgt,ipc,ipd,ipe,
                                beyond,size,swapf,alignsize,size_ratio,options,peq,pgt);
                        } else { /* already-partitioned in middle */
                            /* partition unpartitioned regions and merge */
                            size_t eq2, gt2;
                            A(pl<pc);
                            partition_s(base,first,ipc,pc,pd,pivot,pe,pf,size,
                                compar,context,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                            A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipc);
                            merge_partitions(base,first,*peq,*pgt,ipc,ipd,ipe,ipf,
                                size,swapf,alignsize,size_ratio,options,peq,pgt);
                            A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipf);
                            A(pf<pu);
                            partition_s(base,ipf,beyond,pl,base+*peq*size,
                                base+*peq*size,base+*pgt*size,pf,size,compar,
                                context,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,&eq2,&gt2);
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
                                context,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                        A(first<mid);A(mid<beyond);
                        partition_s(base,mid,beyond,pc,pd,pivot,pe,pf,size,compar,
                                context,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,&eq2,&gt2);
                        merge_partitions(base,first,*peq,*pgt,mid,eq2,gt2,beyond,
                                size,swapf,alignsize,size_ratio,options,peq,pgt);
                    }
                }
#if QUICKSELECT_LINEAR_STABLE
            }
#endif /* QUICKSELECT_LINEAR_STABLE */
        break;
#endif /* QUICKSELECT_STABLE */
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
                if (0<OPT_COMPAR(mn,q,options,context)) mn=q;
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
                int c=OPT_COMPAR(mn,q,options,context);
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
                if (0>OPT_COMPAR(mx,p,options,context)) mx=p;
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
                int c=OPT_COMPAR(mx,q,options,context);
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
    int(*compar)(const void *,const void *,void *), void *context,
    unsigned int options, char **pmn, char **pmx)
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
                find_minmax_s(base,first,first+na,size,compar,context,options,&mna,&mxa);
                find_minmax_s(base,first+na,beyond,size,compar,context,options,&mnb,&mxb);
                /* overall min is smaller of *mna, *mnb; similarly for max */
                /* stability requires choosing mna if mna compares equal to mnb */
                if (0<OPT_COMPAR(mna,mnb,options,context)) mn=mnb; else mn=mna;
                /* stability requires choosing mxb if mxa compares equal to mxb */
                if (0<OPT_COMPAR(mxa,mxb,options,context)) mx=mxa; else mx=mxb;
            } else { /* nmemb==2UL */
                char *a, *z;
                /* first and last (i.e. second) elements */
                a=base+first*size,z=a+size;
                if (0<OPT_COMPAR(a,z,options,context)) mn=z,mx=a; else mn=a,mx=z; /* stable */
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
    ret=find_minmax_s(base,first,beyond,size,compar,context,options,&mn,&mx);
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
                if (a!=mn) {
                    EXCHANGE_SWAP(swapf,a,mn,size,alignsize,size_ratio,nsw++);
                }
                if (z!=mx) { /* beware case where a was mx before above swap! */
                    EXCHANGE_SWAP(swapf,z,a==mx?mn:mx,size,alignsize,size_ratio,
                        nsw++);
                }
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
                            &&(0==OPT_COMPAR(*ppeq-size,*ppeq,options,context)))
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
                            &&(0==OPT_COMPAR(*ppeq,*ppgt,options,context)))
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
    size_t cutoff, unsigned int options, char *conditions,
    size_t *indices, char *element, char **ppeq, char **ppgt)
{
    errno_t ret=0;
    size_t nmemb=beyond-first;
    auto int c=0; /* repivot factor2 count */
    unsigned int table_index=2U;

    A(2UL<=nmemb);
    A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
    /* both or neither NULL */
    for (;;) {
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
            conditions,indices,element,&(lt_region.beyond),
            &(gt_region.first));

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
                    swapf,alignsize,size_ratio,cutoff,options,conditions,
                    indices,element,ppeq,ppgt);
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
    /*NOTREACHED*/
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}
/* public interfaces for N1570 draft extensions */
errno_t quickselect_s_internal(void *base, rsize_t nmemb, /*const*/ rsize_t size,
    int (*compar)(const void *,const void *,void *), void *context,
#if 0
    void (*swapf)(char *,char *,size_t),
#endif
    size_t *pk, size_t nk, unsigned int options)
{
    errno_t ret=0;
    unsigned int sort;
    unsigned int table_index=2U; /* optimized for small nmemb */
    size_t onk=nk; /* original nk value */
    size_t s=0UL;  /* rotation amount */
    struct sampling_table_struct *pst;
    size_t alignsize, size_ratio;
    void (*swapf)(char *, char *, size_t)=NULL;
    void (*sswapf)(char *, char *, size_t);
    size_t cutoff
#if SILENCE_WHINEY_COMPILERS
        =0UL
#endif
        ;
#if QUICKSELECT_INDIRECT + QUICKSELECT_LINEAR_STABLE 
    char *conditions = NULL;
    size_t *indices = NULL;
    char *element = NULL;
#endif /* QUICKSELECT_INDIRECT + QUICKSELECT_LINEAR_STABLE */
#if QUICKSELECT_INDIRECT
    char **pointers = NULL;
    void (*pswapf)(char *, char *, size_t)
#if SILENCE_WHINEY_COMPILERS
        =NULL
#endif
        ;
#endif /* QUICKSELECT_INDIRECT */

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || ((0UL!=nk) && (NULL==pk))
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size) || (RSIZE_MAX<nk)
    || (0U!=(options&~(QUICKSELECT_USER_OPTIONS_MASK)))
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
            QSORT_FUNCTION_NAME((void *)pk,nk,sizeof(size_t),size_tcmp);
        /* verify, fix uniqueness */
        alignsize=alignment_size((char *)pk,sizeof(size_t));
        size_ratio=sizeof(size_t)/alignsize;
        sswapf=swapn(alignsize);
        for (p=0UL,q=1UL; q<=nk; q++) {
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    irotate((char *)pk,p,q,nk,sizeof(size_t),sswapf,alignsize,size_ratio);
                    s=q-p, nk-=s, q=p;
                }
            }
        }
    }

    /* base array alignment size and size_ratio */
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;

    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

#if QUICKSELECT_INDIRECT
    /* Direct or indirect sorting based on nmemb and size_ratio. */
    if ((1UL<size_ratio)&&(7UL<nmemb+size_ratio))
        options|=(QUICKSELECT_INDIRECT);

    /* Don't use indirection if size_ratio==1UL (it would be inefficient). */
    if (1UL==size_ratio) options&=~(QUICKSELECT_INDIRECT);
    /* If indirect sorting, alignment size and size_ratio are reset to
       appropriate values for pointers.
    */
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        char **p=set_array_pointers(pointers,nmemb,base,nmemb,size,0UL,nmemb);
        if (NULL==p) {
            if (NULL!=pointers) { free(pointers); pointers=NULL; }
            options&=~(QUICKSELECT_INDIRECT);
        } else {
            if (pointers!=p) pointers=p;
            if (size>=sizeof(char *))
                element=malloc(size);
            else
                element=malloc(sizeof(char *));
            if (NULL==element) {
                free(pointers);
                pointers=NULL;
                options&=~(QUICKSELECT_INDIRECT);
            } else {
                alignsize=alignment_size((char *)pointers,sizeof(char *));
                if (0U==instrumented) pswapf=swapn(alignsize);
                else pswapf=iswapn(alignsize);
                A(pointers[0]==base);
            }
        }
    }
#endif /* QUICKSELECT_INDIRECT */

#if QUICKSELECT_LINEAR_STABLE
    /* If sorting/selection partial-order stability is supported and requsted,
       try to allocate space for linear-time stable partitioning (fallback if
       allocation fails is O(N log N) in-place stable partitioning).
    */
    if (0U!=(options&(QUICKSELECT_STABLE))) {
        conditions=malloc(nmemb);
        if (NULL!=conditions) indices=malloc(sizeof(size_t)*nmemb);
        if ((NULL!=conditions)&&(NULL==element)) element=malloc(size);
    }
#endif /* QUICKSELECT_LINEAR_STABLE */

    A(table_index < (SAMPLING_TABLE_SIZE));
    if (cutoff<nmemb) {
        /* Initialize the sampling table index for the array size. Sub-array
           sizes will be smaller, and this step ensures that the main loop
           won't have to traverse much of the table during recursion and
           iteration.
        */
        /* sampling table isn't used for dedicated sort */
        pst=d_sampling_table(0UL,nmemb,pk,0UL,nk,NULL,&sort,&table_index,nmemb);
        while (nmemb>pst[table_index].max_nmemb) table_index++;
        if (0UL!=quickselect_small_array_cutoff)
            cutoff=quickselect_small_array_cutoff;
        else cutoff=cutoff_value(size_ratio,options);
    }

    /* For sorting (but not selection), use dedicated sort if nmemb<=cutoff. */
    if ((NULL==pk)&&(nmemb<=cutoff)) {
#if QUICKSELECT_INDIRECT
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            ret=dedicated_sort_s((char *)pointers,0UL,nmemb,sizeof(char *),compar,
                context,pswapf,alignsize,1UL,options);
            if (NULL==indices) indices=(size_t *)pointers;
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,element);
            free(element); element=NULL;
            free(pointers);
            if ((void *)indices==(void *)pointers) indices=NULL;
            if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort */
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                if (0U==instrumented) swapf=swapn(alignsize);
                else swapf=iswapn(alignsize);
                ret=dedicated_sort_s(base,0UL,nmemb,size,compar,context,
                    swapf,alignsize,size_ratio,options);
            } else if (0UL!=alignsize)
                nmoves+=alignsize*size_ratio;
        } else
#endif /* QUICKSELECT_INDIRECT */
            ret=dedicated_sort_s(base,0UL,nmemb,size,compar,context,swapf,
                alignsize,size_ratio,options);
    } else {
        /* Special-case selection and sorting are handled in quickselect_loop. */
#if QUICKSELECT_INDIRECT
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            ret=quickselect_loop_s((char *)pointers,0UL,nmemb,sizeof(char *),
                compar,context,pk,0UL,nk,pswapf,alignsize,1UL,cutoff,
                options,conditions,indices,element,NULL,NULL);
            if (NULL==indices) indices=(size_t *)pointers;
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,element);
            free(element); element=NULL;
            free(pointers);
            if ((void *)indices==(void *)pointers) indices=NULL;
            if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort/select */
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                if (0U==instrumented) swapf=swapn(alignsize);
                else swapf=iswapn(alignsize);
                cutoff=cutoff_value(1UL,options);
                ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,pk,0UL,
                    nk,swapf,alignsize,size_ratio,cutoff,options,conditions,
                    indices,element,NULL,NULL);
            } else if (0UL!=alignsize)
                nmoves+=alignsize*size_ratio;
        } else
#endif /* QUICKSELECT_INDIRECT */
#if QUICKSELECT_LINEAR_STABLE
            ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,pk,0UL,nk,
                swapf,alignsize,size_ratio,cutoff,options,conditions,
                    indices,element,NULL,NULL);
#else
            ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,pk,0UL,nk,
                swapf,alignsize,size_ratio,cutoff,options,NULL,
                    NULL,NULL,NULL,NULL);
#endif /* QUICKSELECT_LINEAR_STABLE */
    }

#if QUICKSELECT_LINEAR_STABLE
    if (0U!=(options&(QUICKSELECT_STABLE))) {
        if (NULL!=element) free(element);
        if (NULL!=conditions) free(conditions);
        if (NULL!=indices) free(indices);
    }
#endif /* QUICKSELECT_LINEAR_STABLE */

    /* Restore pk to full sorted (non-unique) order for caller convenience. */
    /* This may be expensive if the caller supplied a large number of duplicate
       order statistic ranks.  So be it.  Caller: if it hurts, don't do it.
    */
    if (0UL!=s) /* there were duplicates */
        QSORT_FUNCTION_NAME((void *)pk,onk,sizeof(size_t),size_tcmp);
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

errno_t qsort_s_internal (void *base, size_t nmemb, /*const*/ size_t size,
    int (*compar)(const void *,const void *, void *), void *context)
{
    errno_t ret=0;
    unsigned int options=QUICKSELECT_NETWORK_MASK;
    size_t alignsize, size_ratio;
    void (*swapf)(char *, char *, size_t);
    size_t cutoff;
#if QUICKSELECT_INDIRECT
    void (*pswapf)(char *, char *, size_t);
    char *element = NULL;
    char **pointers = NULL;
#endif /* QUICKSELECT_INDIRECT */

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

        /* base array alignment size and size_ratio */
        alignsize=alignment_size(base,size);
        size_ratio=size/alignsize;

        /* dedicated sort cutoff for direct sorting */
        if (0UL!=quickselect_small_array_cutoff)
            cutoff=quickselect_small_array_cutoff;
        else cutoff=cutoff_value(size_ratio,options);

#if QUICKSELECT_INDIRECT > QUICKSELECT_NETWORK_MASK
# error "some reasonable heuristic for deciding when to sort indirectly is needed"
        /* (N.B. Unreliable) example: Direct or indirect sorting based on nmemb and size_ratio. */
        if ((1UL<size_ratio)&&(7UL<nmemb+size_ratio))
            options|=(QUICKSELECT_INDIRECT);
#endif /* QUICKSELECT_INDIRECT */

#if QUICKSELECT_INDIRECT
        /* Don't use indirection if size_ratio==1UL (it would be inefficient). */
        if (1UL==size_ratio) options&=~(QUICKSELECT_INDIRECT);
        /* If indirect sorting, alignment size and size_ratio are reset to
           appropriate values for pointers.
        */
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            char **p=set_array_pointers(pointers,nmemb,base,nmemb,size,0UL,
                nmemb);
            if (NULL==pointers) {
                if (NULL!=pointers) { free(pointers); pointers=NULL; }
                options&=~(QUICKSELECT_INDIRECT);
            } else {
                if (pointers!=p) pointers=p;
                if (size>=sizeof(char *))
                    element=malloc(size);
                else
                    element=malloc(sizeof(char *));
                if (NULL==element) {
                    free(pointers);
                    pointers=NULL;
                    options&=~(QUICKSELECT_INDIRECT);
                } else {
                    alignsize=alignment_size((char *)pointers,sizeof(char *));
                    cutoff=cutoff_value(1UL,options);
                    if (0U==instrumented) pswapf=swapn(alignsize);
                    else pswapf=iswapn(alignsize);
                    A(pointers[0]==base);
                }
            }
        }
#endif /* QUICKSELECT_INDIRECT */

        /* No sorting/selection partial-order stability is supported */

        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

        /* Use dedicated sort if nmemb<=cutoff. */
        if (nmemb<=cutoff) {
#if QUICKSELECT_INDIRECT
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                size_t *indices=(size_t *)pointers;
                ret=dedicated_sort_s((char *)pointers,0UL,nmemb,sizeof(char *),compar,
                    context,pswapf,alignsize,1UL,options);
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,indices,0UL,nmemb);
                A(NULL!=indices);
                alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,element);
                free(element); element=NULL;
                free(pointers);
                if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort */
                    options&=~(QUICKSELECT_INDIRECT);
                    alignsize=alignment_size(base,size);
                    size_ratio=size/alignsize;
                    if (0U==instrumented) swapf=swapn(alignsize);
                    else swapf=iswapn(alignsize);
                    ret=dedicated_sort_s(base,0UL,nmemb,size,compar,context,
                        swapf,alignsize,size_ratio,options);
                } else if (0UL!=alignsize)
                    nmoves+=alignsize*size_ratio;
            } else
#endif /* QUICKSELECT_INDIRECT */
                ret=dedicated_sort_s(base,0UL,nmemb,size,compar,context,swapf,
                    alignsize,size_ratio,options);
        } else {
#if QUICKSELECT_INDIRECT
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                size_t *indices=(size_t *)pointers;
                ret=quickselect_loop_s((char *)pointers,0UL,nmemb,sizeof(char *),
                    compar,context,NULL,0UL,0UL,pswapf,alignsize,1UL,
                    cutoff,options,NULL,NULL,NULL,NULL,NULL);
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,indices,0UL,nmemb);
                A(NULL!=indices);
                alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,element);
                free(element); element=NULL;
                free(pointers);
                if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort/select */
                    options&=~(QUICKSELECT_INDIRECT);
                    alignsize=alignment_size(base,size);
                    size_ratio=size/alignsize;
                    if (0U==instrumented) swapf=swapn(alignsize);
                    else swapf=iswapn(alignsize);
                    cutoff=cutoff_value(size_ratio,options);
                    ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,
                        NULL,0UL,0UL,swapf,alignsize,size_ratio,cutoff,options,
                        NULL,NULL,NULL,NULL,NULL);
                } else if (0UL!=alignsize)
                    nmoves+=alignsize*size_ratio;
            } else
#endif /* QUICKSELECT_INDIRECT */
                ret=quickselect_loop_s(base,0UL,nmemb,size,compar,context,NULL,
                    0UL,0UL,swapf,alignsize,size_ratio,cutoff,options,
                    NULL,NULL,NULL,NULL,NULL);
        }
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}
#endif /* RSIZE_MAX */
