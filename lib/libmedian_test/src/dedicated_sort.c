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
* $Id: ~|^` @(#)    dedicated_sort.c copyright 2016-2017 Bruce Lilly.   \ dedicated_sort.c $
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
/* $Id: ~|^` @(#)   This is dedicated_sort.c version 1.8 dated 2017-12-15T21:54:50Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.dedicated_sort.c */

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
#define ID_STRING_PREFIX "$Id: dedicated_sort.c ~|^` @(#)"
#define SOURCE_MODULE "dedicated_sort.c"
#define MODULE_VERSION "1.8"
#define MODULE_DATE "2017-12-15T21:54:50Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* configuration */
/* Sorting networks for small sub-array sizes 3-12:
   Size 2 has dedicated optimized comparison sorts. Size 3 sorting network is
   optimized (but uses more comparisons on average than insterion sort).
   1 for sorting network, 0 for default merge sort (specialized sort for 3
   elements).
*/
#define NETWORK3  0

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if DEBUG_CODE
# include <stdio.h>
#endif

/* Insertion sort using binary search to locate insertion position for
   out-of-order element, followed by rotation to insert the element in position.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void isort_bs(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *,const void *),
    void (*swapf)(char *,char *,size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    register size_t l, m, h, n, u=beyond-1UL;
    char *pa, *pu=base+u*size;

/* separate direct, indirect versions to avoid options checks in loops,
   also cache dereferenced pa in inner loop
*/
#if QUICKSELECT_INDIRECT
    if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
#endif /* QUICKSELECT_INDIRECT */
        for (n=u,pa=pu-size; n>first; pa-=size) {
            --n;
            if (0<COMPAR(pa,pa+size,/**/)) { /* skip over in-order */
                l=n+2UL;
                if (l>u) l=beyond; /* simple swap */
                else { /* binary search for insertion position */
                    for (h=u,m=l+((h-l)>>1); l<=h; m=l+((h-l)>>1)) {
                        if (0>=COMPAR(pa,base+m*size,/**/)) h=m-1UL;
                        else l=m+1UL;
                    } A(n!=l);
                }
                /* Insert element now at n at position before l by rotating elements
                   [n,l) left by 1.
                */
                irotate(base,n,n+1UL,l,size,swapf,alignsize,size_ratio);
                if (0U!=instrumented) {
                    h=l-n;
                    if (MAXROTATION<=h) h=0UL;
                    nrotations[h]+=size_ratio;
                }
            }
        }
#if QUICKSELECT_INDIRECT
    } else { /* indirect */
        register char *p;
        for (n=u,pa=pu-size; n>first; pa-=size) {
            --n;
            p=*((char **)pa);
            if (0<COMPAR(p,*((char**)(pa+size)),/**/)) {/* skip over in-order */
                l=n+2UL;
                if (l>u) l=beyond; /* simple swap */
                else { /* binary search for insertion position */
                    for (h=u,m=l+((h-l)>>1); l<=h; m=l+((h-l)>>1)) {
                        if (0>=COMPAR(p,*((char**)(base+m*size)),/**/)) h=m-1UL;
                        else l=m+1UL;
                    } A(n!=l);
                }
                /* Insert element now at n at position before l by rotating elements
                   [n,l) left by 1.
                */
                irotate(base,n,n+1UL,l,size,swapf,alignsize,size_ratio);
                if (0U!=instrumented) {
                    h=l-n;
                    if (MAXROTATION<=h) h=0UL;
                    nrotations[h]+=size_ratio;
                }
            }
        }
    }
#endif /* QUICKSELECT_INDIRECT */
}

/* In-place merge of two adjacent sorted sub-arrays. */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void inplace_merge(char *base, size_t l, size_t m, size_t u, size_t size,
    int (*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    char *pl, *pm, *pr, *pu;  /* left, middle, right pointers to elements */
    int c;

    pl=base+l*size;
    pr=pm=base+m*size;
    pu=base+u*size;
/* separate direct, indirect versions to avoid options checks in loops,
   also cache dereferenced pm in inner loops
*/
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
                    if (pr>pm+size) {
                        protate(pm,pm+size,pr,size,swapf,alignsize,size_ratio);
                        if (0U!=instrumented) {
                            size_t h=(pr-pm)/size;
                            if (MAXROTATION<=h) h=0UL;
                            nrotations[h]+=size_ratio;
                        }
                    }
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
                    if (pr>pm+size) {
                        protate(pm,pm+size,pr,size,swapf,alignsize,size_ratio);
                        if (0U!=instrumented) {
                            size_t h=(pr-pm)/size;
                            if (MAXROTATION<=h) h=0UL;
                            nrotations[h]+=size_ratio;
                        }
                    }
                }
                pl+=size;
            }
            pm=pr;
        }
    }
#endif /* QUICKSELECT_INDIRECT */
}


/* Dedicated (not divide-and-conquer) sorting. */
/* called by each loop function and recursively */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void d_dedicated_sort(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, options=0x%x */\n",__func__,source_file,__LINE__,(void *)base,(unsigned long)first,(unsigned long)beyond,options);
print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#endif
    if (beyond>first) {
        size_t n=beyond-first;
        char *pa, *pb;

        /* sorting networks are only implemented for 2<=n<=12
           n==1 is by definition sorted (don't attempt to split/merge)
           options determines whether to use a network or merge
           LSB of options requests a stable sort if set
        */
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
                /* split */
                size_t na=(n>>1), mid=first+na; /* split into 2 (nearly equal-size) pieces */
# if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG))
                    (V) fprintf(stderr,
                        "/* %s: %s line %d: split: base=%p, na=%lu */\n",
                        __func__,source_file,__LINE__,(void *)base,
                        (unsigned long)na);
# endif
                /* recursively sort pieces */
                    if (1UL<na) d_dedicated_sort(base,first,mid,size,compar,
                        swapf, alignsize,size_ratio,options);
                    A(first+na+1UL<beyond);
                    d_dedicated_sort(base,mid,beyond,size,compar,swapf,
                        alignsize,size_ratio,options);
                pb=base+(first+na)*size;
# if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: concatenated sorted halves: base=%p, na=%lu, pb=%p[%lu], neq+nlt+ngt=%lu, nsw=%lu */\n",__func__,source_file,__LINE__,(void *)base,(unsigned long)na,(void *)pb,(pb-base)/size,(unsigned long)neq+(unsigned long)nlt+(unsigned long)ngt,(unsigned long)nsw);
print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
# endif
                /* merge pieces */
                inplace_merge(base,first,mid,beyond,size,compar,
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
                isort_bs(base,first,beyond,size,compar,swapf,alignsize,
                    size_ratio,options);
            }
            return;
        }
        switch (n) {
            case 1UL :
                A(2UL<=n);
                errno=EINVAL;
            break;
            case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
                pa=base+first*size;
                pb=pa+size;
                COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: network sort: first=%lu, beyond=%lu, neq+nlt+ngt=%lu, nsw=%lu */\n",__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(unsigned long)neq+(unsigned long)nlt+(unsigned long)ngt,(unsigned long)nsw);
print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#endif
            break;
#if QUICKSELECT_NETWORK_MASK & 0x08U
            case 3UL : /* optimized sort of 3 */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    /* stable, but always 3 comparisons, cannot be parallelized */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
# if NETWORK3
                    /* The optimization is the "else", which avoids extraneous
                       comparisons. Average over all distinct-valued permutations:
                       8/3 comparisons and 7/6 swaps, which is 1/3 comparison less
                       than a sorting network w/o the "else". Insertion sort
                       averages 8/3 comparisons and 3/2 swaps, which is more swaps.
                       Already-sorted input requires 3 comparisons; reversed input
                       also uses 3 comparisons but requires only 1 swap to sort.
                       Bitonic input is sorted with 2 comparisons and 1 swap.
                       Considering duplicate and all-equal inputs, this uses an
                       average of 2+10/13 comparisons and 11/13 swaps.  No
                       parallelization is possible in this or any other 3-element
                       sorting network.
                    */
                    /* not stable */
                { char *pc;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,
                        size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,
                        size_ratio); /* 0,1 */
                    else
                        COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,
                            size_ratio); /* 1,2 */
                }
# else /* decision-tree comparison-exchange sorting (not stable) */
#  if FAVOR_SORTED /* favor already-sorted, reverse-sorted for merge sort */
                {   /* Optimized comparison-based sort:
                       average 2+6/13 comparisons and 11/13 swaps over all
                       possible inputs (including duplicate and all-equal
                       values), 2+2/3 comparisons and 1+1/6 swaps averaged
                       over distinct-valued inputs.  Already-sorted and
                       reversed inputs use 2 comparisons.  Bitonic and rotated
                       inputs require 3 comparisons.
                    */
                    /* not stable */
                    char *pc;
                    int c, d;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    c=OPT_COMPAR(pa,pb,options,/**/), d=OPT_COMPAR(pb,pc,options,/**/);
                    if (0>=c) { /*b>=a*/
                        if (0<d) { /*b>c*/
                            if (0==c) { /*a==b*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,nsw++); /* 0,2 */
                            } else { /*b>a,b>c*/
                                if (0<OPT_COMPAR(pa,pc,options,/**/)) { /*a>c*/
                                    EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                        size_ratio,nsw++); /* 0,2 */
                                }
                                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,
                                    size_ratio,nsw++); /* 1,2 */
                            }
                        }
                    } else { /*a>b*/
                        if (0<=d) { /*b>=c*/
                            EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,
                                nsw++); /* 0,2 */
                        } else { /*c>b*/
                            if (0<OPT_COMPAR(pa,pc,options,/**/)) { /*a>c*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,nsw++); /* 0,2 */
                            }
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                nsw++); /* 0,1 */
                        }
                    }
                }
#  else
                {   /* Optimized comparison-based sort:
                       average 2+6/13 comparisons and 11/13 swaps over all
                       possible inputs (including duplicate and all-equal
                       values), 2+2/3 comparisons and 1+1/6 swaps averaged
                       over distinct-valued inputs.  Bitonic and right-rotated
                       inputs use 2 comparisons.  Already-sorted, left-rotated,
                       and reversed inputs require 3 comparisons.  Favoring
                       bitonic inputs here slightly offsets the disadvantage
                       for those inputs of uniform sampling, which favors
                       reversed inputs.
                    */
                    /* not stable */
                    char *pc;
                    int c, d;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    c=OPT_COMPAR(pa,pc,options,/**/), d=OPT_COMPAR(pb,pc,options,/**/);
                    if (0>=c) { /*c>=a*/
                        if (0<d) { /*b>c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                nsw++); /* 1,2 */
                        } else if (0>d) { /*b<c*/
                            if ((0==c)||(0<OPT_COMPAR(pa,pb,options,/**/))) { /*a==c or a>b*/
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                                    size_ratio,nsw++); /* 0,1 */
                            }
                        }
                    } else { /*a>c*/
                        if (0>d) { /*b<c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                nsw++); /* 1,2 */
                        } else if ((0<d)&&(0>OPT_COMPAR(pa,pb,options,/**/))) { /*c<a<b*/
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                nsw++); /* 0,1 */
                        }
                        EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,
                            nsw++); /* 0,2 */
                    }
                }
#   if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: dedicated 3-element sort: first=%lu, beyond=%lu, neq+nlt+ngt=%lu, nsw=%lu */\n",__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(unsigned long)neq+(unsigned long)nlt+(unsigned long)ngt,(unsigned long)nsw);
print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#   endif
#  endif
# endif
            break;
#endif
            /* optional network sort for small subarrays */
            /* See J. Gamble http://pages.ripco.net/~jgamble/nw.html */
            /* stable variants (through size 6) are insertion/bubble sort
               unrolled and parallelized
            */
#if QUICKSELECT_NETWORK_MASK & 0x010U
            case 4UL : /* 6 or 5 comparisons with 1 or 2 parallelizable groups */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    /* 6 comparisons, 5 groups (only one of which contains more than one comparison) */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
                {
                    char *pc, *pd;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x010U */
#if QUICKSELECT_NETWORK_MASK & 0x020U
            case 5UL : /* 10 or 9 comparisons with 3 or 4 parallelizable groups */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd, *pe;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size;
                    /* 10 comparisons, 3 opportunities for parallelism */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
                {
                    char *pc, *pd, *pe;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size;
                /* not stable (Batcher merge-exchange network) */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pe,options,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x020U */
#if QUICKSELECT_NETWORK_MASK & 0x040U
            case 6UL : /* 15 or 12 comparisons with 5 or 6 parallelizable groups */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd, *pe, *pf;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size; pf=pe+size;
                    /* 15 comparisons (more than average quicksort), 5 opportunities for parallelism */
                    /* equivalent to unrolled insertion sort or bubble sort */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
                {
                    char *pc, *pd, *pe, *pf;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                /* not stable (Batcher merge-exchange network) */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pe,options,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pf,options,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pd,pf,options,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x040U */
#if QUICKSELECT_NETWORK_MASK & 0x080U
            case 7UL : /* 16 comparisons in 6 parallel groups */
                { char *pc, *pd, *pe, *pf, *pg;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                    pg=pf+size;
                /* not stable (! Batcher merge-exchange network) */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pe,options,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pf,options,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pg,options,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,options,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pd,pf,options,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pg,options,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x080U */
#if QUICKSELECT_NETWORK_MASK & 0x0100U
            case 8UL : /* 19 comparisons in 6 parallel groups */
                { char *pc, *pd, *pe, *pf, *pg, *ph;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                    pg=pf+size; ph=pg+size;
                /* not stable (! Batcher merge-exchange network) */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pe,options,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pf,options,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pg,options,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    COMPARE_EXCHANGE(pd,ph,options,context,size,swapf,alignsize,size_ratio); /* 3,7 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,options,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(pf,ph,options,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pd,pf,options,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pg,ph,options,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pg,options,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0100U */
#if QUICKSELECT_NETWORK_MASK & 0x0200U
            case 9UL : /* 25 comparisons in 9 parallel groups */
                { char *pc, *pd, *pe, *pf, *pg, *ph,
                      /* no English names for Greek characters... */
                       *pj;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                    pg=pf+size; ph=pg+size; pj=ph+size;
                /* not stable */ /* Batcher merge-exchange network uses 26 comparators (more than quicksort average) */
                    /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pg,ph,options,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    /* parallel group 2 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    COMPARE_EXCHANGE(ph,pj,options,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                    /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pg,ph,options,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    COMPARE_EXCHANGE(pc,pf,options,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                    /* parallel group 4 */
                    COMPARE_EXCHANGE(pa,pd,options,context,size,swapf,alignsize,size_ratio); /* 0,3 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pf,pj,options,context,size,swapf,alignsize,size_ratio); /* 5,8 */
                    /* parallel group 5 */
                    COMPARE_EXCHANGE(pd,pg,options,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                    COMPARE_EXCHANGE(pe,ph,options,context,size,swapf,alignsize,size_ratio); /* 4,7 */
                    COMPARE_EXCHANGE(pc,pf,options,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                    /* parallel group 6 */
                    COMPARE_EXCHANGE(pa,pd,options,context,size,swapf,alignsize,size_ratio); /* 0,3 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pf,ph,options,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pc,pg,options,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    /* parallel group 7 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,options,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    /* parallel group 8 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    /* parallel group 9 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0200U */
#if QUICKSELECT_NETWORK_MASK & 0x0400U
            case 10UL : /* 29 comparisons in 9 parallel groups */
                { char *pc, *pd, *pe, *pf, *pg, *ph,
                      /* no English names for Greek characters... */
                       *pj, *pk;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                    pg=pf+size; ph=pg+size; pj=ph+size; pk=pj+size;
                /* not stable */ /* Batcher merge-exchange network uses 31 comparators (more than quicksort average) */
                    /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pf,options,context,size,swapf,alignsize,size_ratio); /* 0,5 */
                    COMPARE_EXCHANGE(pb,pg,options,context,size,swapf,alignsize,size_ratio); /* 1,6 */
                    COMPARE_EXCHANGE(pc,ph,options,context,size,swapf,alignsize,size_ratio); /* 2,7 */
                    COMPARE_EXCHANGE(pd,pj,options,context,size,swapf,alignsize,size_ratio); /* 3,8 */
                    COMPARE_EXCHANGE(pe,pk,options,context,size,swapf,alignsize,size_ratio); /* 4,9 */
                    /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pd,options,context,size,swapf,alignsize,size_ratio); /* 0,3 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pf,pj,options,context,size,swapf,alignsize,size_ratio); /* 5,8 */
                    COMPARE_EXCHANGE(pg,pk,options,context,size,swapf,alignsize,size_ratio); /* 6,9 */
                    /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pd,pg,options,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                    COMPARE_EXCHANGE(ph,pk,options,context,size,swapf,alignsize,size_ratio); /* 7,9 */
                    /* parallel group 4 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,ph,options,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pj,pk,options,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                    /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pf,options,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pe,pg,options,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(ph,pj,options,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                    /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pc,pf,options,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                    COMPARE_EXCHANGE(pe,ph,options,context,size,swapf,alignsize,size_ratio); /* 4,7 */
                    COMPARE_EXCHANGE(pg,pj,options,context,size,swapf,alignsize,size_ratio); /* 6,8 */
                    /* parallel group 7 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pg,ph,options,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    /* parallel group 8 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    /* parallel group 9 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0400U */
#if QUICKSELECT_NETWORK_MASK & 0x0800U
            case 11UL : /* 35 comparisons in 8 groups (optimal depth, maybe not # comparators) */
                /* source: http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
                { char *pc, *pd, *pe, *pf, *pg, *ph,
                      /* no English names for Greek characters... */
                       *pj, *pk, *pl;
                    pa=base+first*size;
                    pb=pa+size; pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                    pg=pf+size; ph=pg+size; pj=ph+size; pk=pj+size; pl=pk+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pb,pl,options,context,size,swapf,alignsize,size_ratio); /* 1,10 */
                    COMPARE_EXCHANGE(pc,pk,options,context,size,swapf,alignsize,size_ratio); /* 2,9 */
                    COMPARE_EXCHANGE(pd,pj,options,context,size,swapf,alignsize,size_ratio); /* 3,8 */
                    COMPARE_EXCHANGE(pe,ph,options,context,size,swapf,alignsize,size_ratio); /* 4,7 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pg,options,context,size,swapf,alignsize,size_ratio); /* 0,6 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(ph,pl,options,context,size,swapf,alignsize,size_ratio); /* 7,10 */
                    COMPARE_EXCHANGE(pj,pk,options,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pf,options,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(ph,pj,options,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                    COMPARE_EXCHANGE(pk,pl,options,context,size,swapf,alignsize,size_ratio); /* 9,10 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pf,options,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pe,pg,options,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(pj,pk,options,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pj,options,context,size,swapf,alignsize,size_ratio); /* 1,8 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,ph,options,context,size,swapf,alignsize,size_ratio); /* 4,7 */
                    COMPARE_EXCHANGE(pf,pk,options,context,size,swapf,alignsize,size_ratio); /* 5,9 */
                    COMPARE_EXCHANGE(pg,pl,options,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pf,options,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pg,pk,options,context,size,swapf,alignsize,size_ratio); /* 6,9 */
                    COMPARE_EXCHANGE(ph,pj,options,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                /* parallel group 7 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,ph,options,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pg,pj,options,context,size,swapf,alignsize,size_ratio); /* 6,8 */
                /* parallel group 8 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    COMPARE_EXCHANGE(pg,ph,options,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0800U */
#if QUICKSELECT_NETWORK_MASK & 0x01000U
            case 12UL : /* 39 comparisons in 9 parallel groups (not optimal) */
                { char *pc, *pd, *pe, *pf, *pg, *ph,
                      /* no English names for Greek characters... */
                       *pj, *pk, *pl, *pm;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size; pf=pe+size; pg=pf+size; ph=pg+size; pj=ph+size;
                    pk=pj+size; pl=pk+size; pm=pl+size;
                /* not stable */ /* Batcher merge-exchange network uses 41 comparisons (more than quicksort average) */
                    /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pb,options,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,options,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    COMPARE_EXCHANGE(pg,ph,options,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    COMPARE_EXCHANGE(pj,pk,options,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                    COMPARE_EXCHANGE(pl,pm,options,context,size,swapf,alignsize,size_ratio); /* 10,11 */
                    /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,options,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,options,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,options,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(pf,ph,options,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pj,pl,options,context,size,swapf,alignsize,size_ratio); /* 8,10 */
                    COMPARE_EXCHANGE(pk,pm,options,context,size,swapf,alignsize,size_ratio); /* 9,11 */
                    /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pe,options,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pc,options,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    COMPARE_EXCHANGE(ph,pm,options,context,size,swapf,alignsize,size_ratio); /* 7,11 */
                    COMPARE_EXCHANGE(pk,pl,options,context,size,swapf,alignsize,size_ratio); /* 9,10 */
                    /* parallel group 4 */
                    COMPARE_EXCHANGE(pb,pf,options,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pd,ph,options,context,size,swapf,alignsize,size_ratio); /* 3,7 */
                    COMPARE_EXCHANGE(pe,pj,options,context,size,swapf,alignsize,size_ratio); /* 4,8 */
                    COMPARE_EXCHANGE(pg,pl,options,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                    /* parallel group 5 */
                    COMPARE_EXCHANGE(pa,pe,options,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pc,pg,options,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    COMPARE_EXCHANGE(pd,pj,options,context,size,swapf,alignsize,size_ratio); /* 3,8 */
                    COMPARE_EXCHANGE(pf,pk,options,context,size,swapf,alignsize,size_ratio); /* 5,9 */
                    COMPARE_EXCHANGE(ph,pm,options,context,size,swapf,alignsize,size_ratio); /* 7,11 */
                    /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pf,options,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pd,options,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pg,pl,options,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                    COMPARE_EXCHANGE(pj,pk,options,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                    /* parallel group 7 */
                    COMPARE_EXCHANGE(pb,pe,options,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pf,options,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pg,pj,options,context,size,swapf,alignsize,size_ratio); /* 6,8 */
                    COMPARE_EXCHANGE(ph,pl,options,context,size,swapf,alignsize,size_ratio); /* 7,10 */
                    /* parallel group 8 */
                    COMPARE_EXCHANGE(pc,pe,options,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,pg,options,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    COMPARE_EXCHANGE(ph,pk,options,context,size,swapf,alignsize,size_ratio); /* 7,9 */
                    /* parallel group 9 */
                    COMPARE_EXCHANGE(pd,pe,options,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(ph,pj,options,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                }
            break;
#endif /* QUICKSELECT_NETWORK_MASK & 0x01000U */
            default :
                A(0==1); /* logic error */
            break;
        }
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: sorted: first=%lu, beyond=%lu, n=%lu, neq+nlt+ngt=%lu, nsw=%lu */\n",__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(unsigned long)n,(unsigned long)neq+(unsigned long)nlt+(unsigned long)ngt,(unsigned long)nsw);
print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#endif /* DEBUG_CODE */
#if ASSERT_CODE > 1
for (pa=base+first*size,pb=pa+size,pu=base+beyond*size; pb<pu; pa=pb,pb+=size)
    A(0>=compar(pa,pb));
#endif /* ASSERT_CODE */
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void dedsort(char *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *), unsigned int options)
{
    size_t alignsize=alignment_size(base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);
    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    if (1UL<nmemb) d_dedicated_sort(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,options);
}
