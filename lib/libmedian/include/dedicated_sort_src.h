/*INDENT OFF*/

/* Description: common C source code for dedicated_sort and dedicated_sort_s */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    dedicated_sort_src.h copyright 2017 Bruce Lilly.   \ dedicated_sort_src.h $
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
/* $Id: ~|^` @(#)   This is dedicated_sort_src.h version 1.9 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.dedicated_sort_src.h */

/********************** Long description and rationale: ***********************
 This file contains source to implement the internal functions dedicated_sort
 and dedicated_sort_s.
 Swapping of elements is handled by efficient inline swap functions.  No attempt
 is made to optimize for specific array element types, but swapping can be
 performed in units of basic language types.  The swap functions avoid useless
 work (e.g. when given two pointers to the same element).  No specialized macros
 are required.
******************************************************************************/

/* Nothing to configure below this line. */

/* Minimum _XOPEN_SOURCE version for C99 (else illumos compilation fails) */
#undef MAX_XOPEN_SOURCE_VERSION
#undef MIN_XOPEN_SOURCE_VERSION
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) \
&& defined(MIN_XOPEN_SOURCE_VERSION) \
&& ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) \
&& defined(MAX_XOPEN_SOURCE_VERSION) \
&& ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components
   are version control fields.
   ID_STRING_PREFIX is suitable for the what(1) and ident(1) utilities.
   MODULE_DATE uses modern SCCS extensions.
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: dedicated_sort_src.h ~|^` @(#)"
#define SOURCE_MODULE "dedicated_sort_src.h"
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2017-12-22T04:14:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"


/* local header files needed */
#include "quickselect_config.h" /* QUICKSELECT_INLINE */
#include "exchange.h"           /* irotate protate EXCHANGE_SWAP */
#if ! QUICKSELECT_BUILD_FOR_SPEED
#include "initialize_src.h"
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* (maybe errno_t [N1570 K3.2]) */
#include <limits.h>             /* *_MAX */
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t (maybe RSIZE_MAX [N1570 K3.4]) */
# if __STDC_VERSION__ >= 201001L
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
#  include <stdio.h>            /* (maybe errno_t rsize_t [N1570 K3.5]) */
#  include <stdlib.h>           /* (maybe errno_t rsize_t constraint_handler_t
                                   [N1570 K3.6]) */
# endif /* N1570 */
#else
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 or later */

/* conformance */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# define SIZE_MAX ULONG_MAX
#endif

/* macros */
/* Comparison-exchange primitive macro used for sorting networks */
#undef COMPARE_EXCHANGE
#if SILENCE_WHINEY_COMPILERS /* no ambiguity here... */
# define COMPARE_EXCHANGE(ma,mb,mopts,mc,msize,mswapf,malignsize,msize_ratio) \
   if(0<OPT_COMPAR(ma,mb,mopts,mc)) {                                         \
       EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,/**/); }
   /* Programmer's terminating semicolon is now an excess statement.
      And this precludes a following "else".
      Thanks :-/ gcc.
   */
#else
# define COMPARE_EXCHANGE(ma,mb,mopts,mc,msize,mswapf,malignsize,msize_ratio) \
   if(0<OPT_COMPAR(ma,mb,mopts,mc))                                           \
       EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,/**/)
    /* caller provides terminating semicolon */
#endif

/* Insertion sort using binary search to locate insertion position for
   out-of-order element, followed by rotation to insert the element in position.
   Adaptive search would use linear search for a sorted region of 3 or fewer
      elements and binary search over 4 or more elements.  Linear search in
      3 sorted elements requires 1-3 comparisons (average 2) whereas binary
      search requires 2 comparisons always, but with slightly higher overhead.
      At 4 elements, linear search requires 1-4 comparisons (average 2.5) and
      binary search requires 2-3 comparisons (average log2(5) ~ 2.322) which
      is a bit better.  But eliminating the test for the sorted region size and
      always using binary search runs faster; eliminating the test compensates
      for the overhead of binary search.
   N.B. This works well for small arrays, but not for large nearly-sorted
      arrays, where linear search is better due to the expectation that the
      insertion point is closer than half the size of the sorted region.
*/
static QUICKSELECT_INLINE
void isort_bs(char *base, size_t first, size_t beyond, size_t size, COMPAR_DECL,
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
                /* Insert element now at n at position before l by rotating
                   elements [n,l) left by 1.
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
            if (0<COMPAR(p,*((char**)(pa+size)),context)) {/* skip in-order */
                l=n+2UL;
                if (l>u) l=beyond; /* simple swap */
                else { /* binary search for insertion position */
                    for (h=u,m=l+((h-l)>>1); l<=h; m=l+((h-l)>>1)) {
                        if (0>=COMPAR(p,*((char**)(base+m*size)),context))
                            h=m-1UL;
                        else l=m+1UL;
                    } A(n!=l);
                }
                /* Insert element now at n at position before l by rotating
                   elements [n,l) left by 1.
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
static QUICKSELECT_INLINE
void inplace_merge(char *base, size_t l, size_t m, size_t u, size_t size,
    COMPAR_DECL,
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
            /* Find and swap the first element of the left side which is larger
               than the first element of the right side.  That element is almost
               always within the first few elements, so use a linear scan
               (binary search would be counterproductive).
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
            /* displaced element @pm, 2nd region start @pr, next to merge @pl */
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
            /* Find and swap the first element of the left side which is larger
               than the first element of the right side.  That element is almost
               always within the first few elements, so use a linear scan
               (binary search would be counterproductive).
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
            /* displaced element @pm, 2nd region start @pr, next to merge @pl */
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

/* clutter removal */
#undef PREFIX
#undef RETURN
#undef SUFFIX1
#undef SUFFIX2
#undef SUFFIX3
#undef TEST
#if __STDC_WANT_LIB_EXT1__
# define PREFIX A(0==ret);ret=
# define RETURN return ret
# define SUFFIX1 A(NULL!=pivot);if(NULL==pivot)return ret=errno;
# define SUFFIX2 A(0==ret);if(0!=ret)return ret;
# define SUFFIX3 A(0==ret);return ret;
# define TEST A(0==ret)
#else
# define PREFIX /**/
# define RETURN return
# define SUFFIX1 /**/
# define SUFFIX2 /**/
# define SUFFIX3 /**/
# define TEST /**/
#endif /* __STDC_WANT_LIB_EXT1__ */

/* definition */
#if __STDC_WANT_LIB_EXT1__
# include "dedicated_sort_s_decl.h"
#else /* __STDC_WANT_LIB_EXT1__ */
# include "dedicated_sort_decl.h"
#endif /* __STDC_WANT_LIB_EXT1__ */
{
#if __STDC_WANT_LIB_EXT1__
    errno_t ret=0;
#endif /* __STDC_WANT_LIB_EXT1__ */

#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file();
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
    if (beyond>first) {
        size_t n=beyond-first;
        char *pa, *pb;

        /* Sorting networks are only implemented for 2<=n<=12;
           n==1 is by definition sorted (don't attempt to split/merge) --
           options determine whether to use a network or merge or insertion
           sort.
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
            if ((INPLACE_MERGE_CUTOFF<=n)
               ||(0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS)))
            ) {
                /* in-place merge sort using rotation */
                size_t mid, na;
                /* split */
                na=(n>>1); mid=first+na;
                /* use recursion to sort pieces */
                if (1UL<na) {
                    PREFIX DEDICATED_SORT(base,first,mid,size,COMPAR_ARGS,swapf,
                        alignsize, size_ratio, options); SUFFIX2
                }
                A(first+na+1UL<beyond);
                PREFIX DEDICATED_SORT(base,mid,beyond,size,COMPAR_ARGS,swapf,
                    alignsize, size_ratio, options); SUFFIX2
                /* merge pieces */
                inplace_merge(base,first,mid,beyond,size,COMPAR_ARGS,
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
                isort_bs(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize,
                    size_ratio,options);
            }
            RETURN ;
        }
#define CX(ma,mb) COMPARE_EXCHANGE(ma,mb,options,context,size,swapf,alignsize,\
   size_ratio)
        switch (n) {
            case 1UL : /* shouldn't happen... */
                A(2UL<=n);
                PREFIX errno=EINVAL;
            RETURN ;
            case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
                pa=base+first*size;
                pb=pa+size;
                CX(pa,pb); /* 0,1 */
            RETURN ;
#if QUICKSELECT_NETWORK_MASK & 0x08U
            case 3UL : /* stable network or optimized decision tree sort of 3 */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    /* stable, but always 3 comparisons, cannot be parallelized */
                    CX(pa,pb); /* 0,1 */
                    CX(pb,pc); /* 1,2 */
                    CX(pa,pb); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
# if FAVOR_SORTED /* favor already-sorted, reverse-sorted input sequences */
                {   /* Optimized comparison-based sort:
                       average 32/13 comparisons and 11/13 swaps over all
                       possible inputs (including duplicate and all-equal
                       values), 8/3 comparisons and 7/6 swaps averaged
                       over distinct-valued inputs.  Already-sorted and
                       reversed inputs use 2 comparisons.  Bitonic and rotated
                       inputs require 3 comparisons.
                    */
                    /* not stable */
                    char *pc;
                    int c, d;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    c=OPT_COMPAR(pa,pb,options,context),
                        d=OPT_COMPAR(pb,pc,options,context);
                    if (0>=c) { /*b>=a*/
                        if (0<d) { /*b>c*/
                            if (0==c) { /*a==b*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,/**/); /* 0,2 */
                            } else { /*b>a,b>c*/
                                if (0<OPT_COMPAR(pa,pc,options,context)) /*a>c*/
                                    EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                        size_ratio,/**/); /* 0,2 */
                                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,
                                    size_ratio,/**/); /* 1,2 */
                            }
                        }
                    } else { /*a>b*/
                        if (0<=d) /*b>=c*/
                            EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,
                                /**/); /* 0,2 */
                        else { /*c>b*/
                            if (0<OPT_COMPAR(pa,pc,options,context)) /*a>c*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,/**/); /* 0,2 */
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                /**/); /* 0,1 */
                        }
                    }
                }
# else /* ! FAVOR_SORTED */
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
                    /* not stable */
                    char *pc;
                    int c, d;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    c=OPT_COMPAR(pa,pc,options,context),
                        d=OPT_COMPAR(pb,pc,options,context);
                    if (0>=c) { /*c>=a*/
                        if (0<d) { /*b>c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                /**/); /* 1,2 */
                        } else if (0>d) { /*b<c*/
                            if ((0==c)
                            ||(0<OPT_COMPAR(pa,pb,options,context))
                            ) { /*a==c||a>b*/
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                                    size_ratio,/**/); /* 0,1 */
                            }
                        }
                    } else { /*a>c*/
                        if (0>d) { /*b<c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                /**/); /* 1,2 */
                        } else if ((0<d)
                        &&(0>OPT_COMPAR(pa,pb,options,context))
                        ) { /*c<a<b*/
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                /**/); /* 0,1 */
                        }
                        EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,
                            /**/); /* 0,2 */
                    }
                }
# endif /* FAVOR_SORTED */
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x08U */
#if QUICKSELECT_NETWORK_MASK & 0x010U
            case 4UL : /* 6 or 5 comparisons with 1 or 2 parallelizable groups */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    /* 6 comparisons, 5 groups (only one of which contains more than one comparison) */
                    CX(pa,pb); /* 0,1 */ CX(pb,pc); /* 1,2 */
                    /* parallel group */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* end of group */
                    CX(pb,pc); /* 1,2 */
                    CX(pa,pb); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
                {
                    char *pc, *pd;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group 1 */
                    CX(pa,pc); /* 0,2 */ CX(pb,pd); /* 1,3 */
                    /* parallel group 2 */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* parallel group 3 */
                    CX(pb,pc); /* 1,2 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x010U */
#if QUICKSELECT_NETWORK_MASK & 0x020U
            case 5UL : /* 10 or 9 comparisons with 3 or 4 parallelizable groups */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd, *pe;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size;
                    /* 10 comparisons, 3 opportunities for parallelism */
                    CX(pa,pb); /* 0,1 */
                    CX(pb,pc); /* 1,2 */
                    /* parallel group */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* end of group */
                    /* parallel group */
                    CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* end of group */
                    CX(pb,pc); /* 1,2 */
                    CX(pa,pb); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
                {
                    char *pc, *pd, *pe;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group 1 */
                    CX(pa,pe); /* 0,4 */
                    /* parallel group 2 */
                    CX(pa,pc); /* 0,2 */ CX(pb,pd); /* 1,3 */
                    /* parallel group 3 */
                    CX(pa,pb); /* 0,1 */ CX(pc,pe); /* 2,4 */
                    /* parallel group 4 */
                    CX(pc,pd); /* 2,3 */ CX(pb,pe); /* 1,4 */
                    /* parallel group 5 */
                    CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x020U */
#if QUICKSELECT_NETWORK_MASK & 0x040U
            case 6UL : /* 15 or 12 comparisons w/ 5 or 6 parallelizable groups */
# if QUICKSELECT_STABLE
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd, *pe, *pf;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size; pf=pe+size;
                    /* 15 comparisons (more than average quicksort),
                       5 opportunities for parallelism
                    */
                    /* equivalent to unrolled insertion sort or bubble sort */
                    CX(pa,pb); /* 0,1 */
                    CX(pb,pc); /* 1,2 */
                    /* parallel group */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* end of group */
                    /* parallel group */
                    CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    CX(pa,pb);/* 0,1 */ CX(pc,pd);/* 2,3 */ CX(pe,pf);/* 4,5 */
                    /* end of group */
                    /* parallel group */
                    CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* end of group */
                    CX(pb,pc); /* 1,2 */
                    CX(pa,pb); /* 0,1 */
                } else
# endif /* QUICKSELECT_STABLE */
                {
                    char *pc, *pd, *pe, *pf;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size; pf=pe+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group 1 */
                    CX(pa,pe); /* 0,4 */ CX(pb,pf); /* 1,5 */
                    /* parallel group 2 */
                    CX(pa,pc); /* 0,2 */ CX(pb,pd); /* 1,3 */
                    /* parallel group 3 */
                    CX(pa,pb);/* 0,1 */ CX(pc,pe);/* 2,4 */ CX(pd,pf);/* 3,5 */
                    /* parallel group 4 */
                    CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                    /* parallel group 5 */
                    CX(pb,pe); /* 1,4 */
                    /* parallel group 6 */
                    CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x040U */
#if QUICKSELECT_NETWORK_MASK & 0x080U
            case 7UL : /* 16 comparisons in 6 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                /* parallel group 1 */
                    CX(pa,pe);/* 0,4 */ CX(pb,pf);/* 1,5 */ CX(pc,pg);/* 2,6 */
                /* parallel group 2 */
                    CX(pa,pc);/* 0,2 */ CX(pb,pd);/* 1,3 */ CX(pe,pg);/* 4,6 */
                /* parallel group 3 */
                    CX(pa,pb);/* 0,1 */ CX(pc,pe);/* 2,4 */ CX(pd,pf);/* 3,5 */
                /* parallel group 4 */
                    CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                /* parallel group 5 */
                    CX(pb,pe); /* 1,4 */ CX(pd,pg); /* 3,6 */
                /* parallel group 6 */
                    CX(pb,pc);/* 1,2 */ CX(pd,pe);/* 3,4 */ CX(pf,pg);/* 5,6 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x080U */
#if QUICKSELECT_NETWORK_MASK & 0x0100U
            case 8UL : /* 19 comparisons in 6 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg, *ph;
                pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                pe=pd+size; pf=pe+size; pg=pf+size; ph=pg+size;
                /* parallel group 1 */
                    CX(pa,pe); /* 0,4 */ CX(pb,pf); /* 1,5 */
                    CX(pc,pg); /* 2,6 */ CX(pd,ph); /* 3,7 */
                /* parallel group 2 */
                    CX(pa,pc); /* 0,2 */ CX(pb,pd); /* 1,3 */
                    CX(pe,pg); /* 4,6 */ CX(pf,ph); /* 5,7 */
                /* parallel group 3 */
                    CX(pa,pb); /* 0,1 */ CX(pc,pe); /* 2,4 */
                    CX(pd,pf); /* 3,5 */ CX(pg,ph); /* 6,7 */
                /* parallel group 4 */
                    CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                /* parallel group 5 */
                    CX(pb,pe); /* 1,4 */ CX(pd,pg); /* 3,6 */
                /* parallel group 6 */
                    CX(pb,pc);/* 1,2 */ CX(pd,pe);/* 3,4 */ CX(pf,pg);/* 5,6 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0100U */
#if QUICKSELECT_NETWORK_MASK & 0x0200U
            case 9UL : /* 25 comparisons in 9 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj;
                pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                pe=pd+size; pf=pe+size; pg=pf+size; ph=pg+size; pj=ph+size;
                /* parallel group 1 */
                    CX(pa,pb);/* 0,1 */ CX(pd,pe);/* 3,4 */ CX(pg,ph);/* 6,7 */
                /* parallel group 2 */
                    CX(pb,pc);/* 1,2 */ CX(pe,pf);/* 4,5 */ CX(ph,pj);/* 7,8 */
                /* parallel group 3 */
                    CX(pa,pb); /* 0,1 */ CX(pd,pe); /* 3,4 */
                    CX(pg,ph); /* 6,7 */ CX(pc,pf); /* 2,5 */
                /* parallel group 4 */
                    CX(pa,pd);/* 0,3 */ CX(pb,pe);/* 1,4 */ CX(pf,pj);/* 5,8 */
                /* parallel group 5 */
                    CX(pd,pg);/* 3,6 */ CX(pe,ph);/* 4,7 */ CX(pc,pf);/* 2,5 */
                /* parallel group 6 */
                    CX(pa,pd); /* 0,3 */ CX(pb,pe); /* 1,4 */
                    CX(pf,ph); /* 5,7 */ CX(pc,pg); /* 2,6 */
                /* parallel group 7 */
                    CX(pb,pd); /* 1,3 */ CX(pe,pg); /* 4,6 */
                /* parallel group 8 */
                    CX(pc,pe); /* 2,4 */ CX(pf,pg); /* 5,6 */
                /* parallel group 9 */
                    CX(pc,pd); /* 2,3 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0200U */
#if QUICKSELECT_NETWORK_MASK & 0x0400U
            case 10UL : /* 29 comparisons in 9 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj, *pk;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                ph=pg+size; pj=ph+size; pk=pj+size;
                /* parallel group 1 */
                    CX(pa,pf);/* 0,5 */ CX(pb,pg);/* 1,6 */ CX(pc,ph);/* 2,7 */
                    CX(pd,pj); /* 3,8 */ CX(pe,pk); /* 4,9 */
                /* parallel group 2 */
                    CX(pa,pd); /* 0,3 */ CX(pb,pe); /* 1,4 */
                    CX(pf,pj); /* 5,8 */ CX(pg,pk); /* 6,9 */
                /* parallel group 3 */
                    CX(pa,pc);/* 0,2 */ CX(pd,pg);/* 3,6 */ CX(ph,pk);/* 7,9 */
                /* parallel group 4 */
                    CX(pa,pb); /* 0,1 */ CX(pc,pe); /* 2,4 */
                    CX(pf,ph); /* 5,7 */ CX(pj,pk); /* 8,9 */
                /* parallel group 5 */
                    CX(pb,pc); /* 1,2 */ CX(pd,pf); /* 3,5 */
                    CX(pe,pg); /* 4,6 */ CX(ph,pj); /* 7,8 */
                /* parallel group 6 */
                    CX(pb,pd); /* 1,3 */ CX(pc,pf); /* 2,5 */
                    CX(pe,ph); /* 4,7 */ CX(pg,pj); /* 6,8 */
                /* parallel group 7 */
                    CX(pc,pd); /* 2,3 */ CX(pg,ph); /* 6,7 */
                /* parallel group 8 */
                    CX(pd,pe); /* 3,4 */ CX(pf,pg); /* 5,6 */
                /* parallel group 9 */
                    CX(pe,pf); /* 4,5 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0400U */
#if QUICKSELECT_NETWORK_MASK & 0x0800U
            case 11UL : /* 35 comparisons in 9 groups (optimal depth, maybe not
                           # comparisons, not stable)
                        */
                /* source: http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj, *pk, *pl;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                ph=pg+size; pj=ph+size; pk=pj+size; pl=pk+size;
                /* parallel group 1 */
                    CX(pb,pl);/* 1,10 */ CX(pc,pk);/* 2,9 */ CX(pd,pj);/* 3,8 */
                    CX(pe,ph); /* 4,7 */ CX(pf,pg); /* 5,6 */
                /* parallel group 2 */
                    CX(pa,pg);/* 0,6 */ CX(pb,pe);/* 1,4 */ CX(pc,pd);/* 2,3 */
                    CX(ph,pl); /* 7,10 */ CX(pj,pk); /* 8,9 */
                /* parallel group 3 */
                    CX(pa,pb);/* 0,1 */ CX(pc,pf);/* 2,5 */ CX(pd,pe);/* 3,4 */
                    CX(ph,pj); /* 7,8 */ CX(pk,pl); /* 9,10 */
                /* parallel group 4 */
                    CX(pa,pc); /* 0,2 */ CX(pb,pf); /* 1,5 */
                    CX(pe,pg); /* 4,6 */ CX(pj,pk); /* 8,9 */
                /* parallel group 5 */
                    CX(pb,pj);/* 1,8 */ CX(pc,pd);/* 2,3 */ CX(pe,ph);/* 4,7 */
                    CX(pf,pk); /* 5,9 */ CX(pg,pl); /* 6,10 */
                /* parallel group 6 */
                    CX(pb,pe); /* 1,4 */ CX(pd,pf); /* 3,5 */
                    CX(pg,pk); /* 6,9 */ CX(ph,pj); /* 7,8 */
                /* parallel group 7 */
                    CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                    CX(pf,ph); /* 5,7 */ CX(pg,pj); /* 6,8 */
                /* parallel group 8 */
                    CX(pc,pd);/* 2,3 */ CX(pe,pf);/* 4,5 */ CX(pg,ph);/* 6,7 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x0800U */
#if QUICKSELECT_NETWORK_MASK & 0x01000U
            case 12UL : /* 39 comparisons in 9 parallel groups (not optimal or stable) */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj, *pk, *pl, *pm;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                ph=pg+size; pj=ph+size; pk=pj+size; pl=pk+size; pm=pl+size;
                /* parallel group 1 */
                    CX(pa,pb);/* 0,1 */ CX(pc,pd);/* 2,3 */ CX(pe,pf);/* 4,5 */
                    CX(pg,ph);/* 6,7 */ CX(pj,pk);/* 8,9 */ CX(pl,pm);/* 10,11 */
                /* parallel group 2 */
                    CX(pa,pc);/* 0,2 */ CX(pb,pd);/* 1,3 */ CX(pe,pg);/* 4,6 */
                    CX(pf,ph);/* 5,7 */ CX(pj,pl);/* 8,10 */ CX(pk,pm);/* 9,11 */
                /* parallel group 3 */
                    CX(pa,pe);/* 0,4 */ CX(pb,pc);/* 1,2 */ CX(pf,pg);/* 5,6 */
                    CX(ph,pm); /* 7,11 */ CX(pk,pl); /* 9,10 */
                /* parallel group 4 */
                    CX(pb,pf); /* 1,5 */ CX(pd,ph); /* 3,7 */
                    CX(pe,pj); /* 4,8 */ CX(pg,pl); /* 6,10 */
                /* parallel group 5 */
                    CX(pa,pe);/* 0,4 */ CX(pc,pg);/* 2,6 */ CX(pd,pj);/* 3,8 */
                    CX(pf,pk); /* 5,9 */ CX(ph,pm); /* 7,11 */
                /* parallel group 6 */
                    CX(pb,pf); /* 1,5 */ CX(pc,pd); /* 2,3 */
                    CX(pg,pl); /* 6,10 */ CX(pj,pk); /* 8,9 */
                /* parallel group 7 */
                    CX(pb,pe); /* 1,4 */ CX(pd,pf); /* 3,5 */
                    CX(pg,pj); /* 6,8 */ CX(ph,pl); /* 7,10 */
                /* parallel group 8 */
                    CX(pc,pe);/* 2,4 */ CX(pf,pg);/* 5,6 */ CX(ph,pk);/* 7,9 */
                /* parallel group 9 */
                    CX(pd,pe); /* 3,4 */ CX(ph,pj); /* 7,8 */
                }
            RETURN ;
#endif /* QUICKSELECT_NETWORK_MASK & 0x01000U */
            default :
                A(0==1);
                PREFIX errno = EINVAL;
            RETURN ;
        }
    }
    RETURN ;
}
