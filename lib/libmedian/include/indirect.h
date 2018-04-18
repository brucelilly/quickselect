/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring functions set_array_pointers, indirect_mergesort
    useful for indirect sorting of arrays of large elements accessed via pointers
*/
#ifndef	INDIRECT_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    indirect.h copyright 2017-2018 Bruce Lilly. \ indirect.h $
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
/* $Id: ~|^` @(#)   This is indirect.h version 1.12 dated 2018-04-15T03:26:40Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "indirect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.indirect.h */

/********************** Long description and rationale: ***********************
 Sorting arrays of large elements (e.g. structured data) can be expensive due to
 the cost of swapping or otherwise moving the data elements.  Indirect sorting
 can be more efficient: pointers to elements are used to access elements for
 comparisons, but only the pointers are moved during sorting (or selection).
 Moving pointers is relatively cheap because a pointer is a basic type. Pointers
 can continue to be used to access data elements after sorting or selection, or
 the pointers can be used to rearrange the data elements, moving each element at
 most one time to place it in its appropriate position.  For rearrangement, use
 of array indices rather than pointers is more efficient.  This file declares
 functions which can be used to allocate and initialize an array of pointers to
 data elements, to convert pointers to indices, to rearrange data elements using
 indices, and to indirectly sort using mergesort (using the other functions in
 the implementation).
******************************************************************************/

/* version-controlled header file version information */
#define INDIRECT_H_VERSION "indirect.h 1.12 2018-04-15T03:26:40Z"

/* compile-time configuration options */
/* assertions for validation testing */
#define INDIRECT_ASSERT_CODE  0 /* Adds size & cost to aid debugging.
                                   0 for tested production code.
                                   Value > 1 will increase comparisons count.
                                   N.B. assert() likely won't work here.
                                */

#include <errno.h>              /* errno E* */
#include <stddef.h>             /* size_t */
#if INDIRECT_ASSERT_CODE
#include <stdio.h>              /* fprintf stderr */
#include <stdlib.h>             /* abort */
#endif
#if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )
# include <stdint.h>            /* (header not standardized pre-C99) SIZE_MAX
                                   (maybe RSIZE_MAX (C11))
                                */
#else
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 or later */
#include <limits.h>             /* *_MAX */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# define SIZE_MAX ULONG_MAX
#endif /* SIZE_MAX */

#if defined(__cplusplus)
# define INDIRECT_EXTERN extern "C"
#else
# define INDIRECT_EXTERN extern
#endif

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define INDIRECT_INLINE inline
#else
# define INDIRECT_INLINE /**/
#endif /* C99 */

/* indirect.c */
INDIRECT_EXTERN char **set_array_pointers(char **pointers, size_t nptrs,
    char *base, size_t size, size_t first, size_t beyond);
INDIRECT_EXTERN size_t *convert_pointers_to_indices(char *base, size_t nmemb, size_t size,
    char **pointers, size_t nptrs, size_t *indices, size_t first, size_t beyond);

/* indirect_mergesort.c */
INDIRECT_EXTERN int indirect_mergesort(char *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *));

/* inline code follows */

/* headers necessary for inline code */
#if !defined(COMPAR_DECL) || !defined(COMPAR)
# include "quickselect_config.h"
#endif
#if !defined(EXCHANGE_SWAP)
# include "exchange.h"
#endif

/* quickselect_loop declaration */
#if ! defined(QUICKSELECT_LOOP_DECLARED)
QUICKSELECT_EXTERN
# include "quickselect_loop_decl.h"
;
# define QUICKSELECT_LOOP_DECLARED 1
#endif /* QUICKSELECT_LOOP_DECLARED */

/* swap function for pointers */
extern void (*pointerswap)(char *,char *,size_t);

/* Merging (as used in merge sort variants) compares elements in one sorted
   sub-array to elements in another sorted sub-array.  For random inputs, it is
   quite likely that one of the first few elements in the first sub-array will
   be displaced (50% probability for the first element, 75% for one of the first
   two, 87.5% for on of the first three, etc.).  A linear search therefore works
   well when comparing elements from random input.  But for already-sorted
   input, that will sequentially examine every element in the first sub-array,
   only to determine that merging requires no data moves.  Sometimes, merge sort
   implementations compare the last element of the first sub-array to the first
   element of the second sub-array initially to detect that condition.  But
   that costs an additional comparison for all input sequences and only saves
   comparisons in unusual cases, such as already-sorted input or constant-value
   input.  For binary-valued input (e.g. random zeros and ones), the expectation
   is that about half of the first sub-array (i.e. about 1/4 of the total
   elements) will have to be examined to find the first element which needs to
   be displaced.  For that specific case, a binary search would be best, but
   that is also an unusual situation.  In order to accomodate unusual inputs
   such as already-sorted, constant-value, and M-ary values as well as random
   input sequences, the following strategy is used: first compare some small
   number of initial elements linearly (this should find an element to displace
   when input is random), then check the last element (to catch already-sorted
   and constant-value inputs quickly), finally using binary search over the
   remaining unexamined elements (to efficiently handle M-ary input).  The
   number of elements scanned linearly in the initial part is the floor of the
   base 2 logarithm of the left part size (# elements); that makes the scan
   at worst O(log(N)) in any case (linear scan or binary search).
*/
/* floor of base 2 logarithm of the argument */
static INDIRECT_INLINE
size_t floor_lg(size_t n)
{
    register size_t r=0UL;

    while (1UL<n) n>>=1, r++;
    return r;
}

/* inplace_merge is used by dedicated_sort and mergesort
*/
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
   [b,c) contains (sorted) elements displaced from the first section
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
/* Separate direct and indirect loops; OPT_COMPAR not required. */
static INDIRECT_INLINE
void inplace_merge(char *base, const size_t l, size_t m, const size_t u,
    const size_t size, COMPAR_DECL,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    register char *pc;  /* search pointers */
    register char *p, *q;
    register size_t s, z; /* search indices */
    size_t r, o, t, x, y;

    A(l<=m);A(m<u);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
        (V)fprintf(stderr,"/* %s: l=%lu, m=%lu, u=%lu, n1=%lu, n2=%lu, options"
            "=0x%x */\n",__func__,l,m,u,m-l,u-m,options);
#endif
#if defined(DEBUGGING)
    nmerges++;
#endif
    /* Find and swap the first element of the left side which is larger than the
       first element of the right side.  For random input, that element is
       almost always within the first few elements (50% probability that it is
       the first element, 75% that it is one of the first two, 87.5% that is is
       one of the first three, etc.), so use a linear scan for a few elements
       (floor(log2(left part size))).  Constant-value input would entail
       testing every element in the first piece; testing the last element of
       the first part vs. the first element of the second part would terminate
       the search.  But binary-valued input (random zeros and ones) has an
       expected position for the first displacement about half-way into the
       first part (so binary search would be attractive).  Constant-value input
       is less likely than M-ary input, which is less likely than nearly-random
       input.  The strategy is therefore to use a linear search for the first
       few elements, and if that does not find an element to displace, test the
       last element of the first part vs. the first element of the second part
       (in case the input is already sorted), then test the first element of the
       first part against the last element of the second part to see if the
       parts should be swapped, and if that also fails, then switch to a binary
       search within the remainder of the first part.  That is expected to work
       well for most random inputs, and to be only slightly suboptimal for
       M-ary, reversed, and constant inputs.
       The implementation is split into two similar parts; one for direct access
       and one which accesses data elements indirectly via pointers (passed as
       the base array).  Loop control is based on element (and pointer) indices;
       data pointers are maintained in sync with the indices for the direct
       version, whereas the indirect access version uses the indices to select
       pointers to the data elements.  Splitting the implementation avoids
       testing options multiple times within loops.
    */
    if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
        s=l,t=m;
        p=base+s*size;
        q=base+t*size;
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
        print_some_array(base,l,u-1UL,"/* pre-merge: "," */",options);
#endif
        while ((s<t)&&(t<u)) { /* still some parts to merge */
            A(p==base+s*size);
            /* limited-range linear search */
            if ((r=s+floor_lg(t-s))>t) r=t; /* test limit */
            for (o=s; s<r; p+=size,++s) {
                A(p==base+s*size);
                if (0<COMPAR(p,q)) { /* *pc>*pm: displace it */
                    if ((s==o) /* first element of left part */
                      /* test 1st 1st part element vs. last 2nd part element */
                    && (0<COMPAR(p,base+(u-1UL)*size))
                    ) {
                        irotate(base,s,t,u,size,swapf,alignsize,size_ratio);
#ifdef QUICKSELECT_COUNT_ROTATIONS
                        QUICKSELECT_COUNT_ROTATIONS(u-s,size_ratio);
#endif
                        return;
                    }
                    EXCHANGE_SWAP(swapf,p,q,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                    break;
                }
            }
            A(p==base+s*size);
            if (s>=r) { /* no displacement in initial scan */
                /* test pb=pm-size vs. pm; if *(pm-size)<=*pm, return (done). */
                /* already compared through pc-size */
                A(s<=t);
                if (s>=t) /* already tested; sorted; */
                    return; /* done. */
                p=q-size;
                if (0>=COMPAR(p,q)) {
                    return; /* all on left <= smallest on right; done */
                }
                /* else binary search within [pc=pa,pb=pm-size) */
                /* already tested pa-size, result c<=0 (*(pa-size)<=*pm) */
                /* now pc==pa, index b (element not yet tested vs. pm) */
                /* already tested pb=pm-size, result c>0 (*pb>*pm) */
                /* at termination, a=index of the leftmost element in range
                   [a,m-1] with value greater than *pm; that element will be
                   swapped with pm
                */
                if (t>1UL)
                    for (z=t-2UL; r<=z;) {
                        p=base+(o=BS_MID_L(r,z))*size;
                        if (0<COMPAR(p,q)) z=o-1UL;
                        else r=o+1UL;
                    }
                A(q==base+t*size);
                p=base+r*size;
                EXCHANGE_SWAP(swapf,p,q,size,alignsize,size_ratio,
                    SWAP_COUNT_STATEMENT);
                s=r;
            }
            s++;
            pc=p+size,p=q,q=base+(y=t+1UL)*size;
            A(pc==base+s*size);
            for (x=t; (s<t)&&(y<u); pc+=size,++s) {
            /* displaced element @x, 2nd region start @y, next to merge @s */
                if (0<COMPAR(p,q)) { /* 2nd part smallest is smaller */
                    EXCHANGE_SWAP(swapf,pc,q,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                    if (++y<u) q+=size;
                } else {
                    EXCHANGE_SWAP(swapf,pc,p,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                    if (y>x+1UL) {
                        /* Rotation is always by 1 position; protate has an
                           efficient special case for that.
                        */
                        protate(p,p+size,q,size,swapf,alignsize,size_ratio);
#ifdef QUICKSELECT_COUNT_ROTATIONS
                        QUICKSELECT_COUNT_ROTATIONS((q-p)/size,size_ratio);
#endif
                    }
                }
            }
            A(pc==base+s*size);
            A((s==t)||(y==u));
            /* next iteration uses previously displaced elements as the first
               part, and the remainder of the second part
            */
            if (s==t) s=x,t=y,p=pc;
            else p=base+s*size,q=base+t*size;
        }
    } else { /* indirect */
        char **pointers=(char **)base;
        s=l,t=m;
        p=pointers[s]; /* pointer to first element of left part */
        q=pointers[t]; /* pointer to first element of right part */
        while ((s<t)&&(t<u)) { /* still some parts to merge */
            /* Initial linear scan to find a left element larger than the
               smallest element on the right (or if you prefer, skip over
               in-place elements on the left).  All comparisons in this
               initial loop are between some (variable) early element of the
               left part and the (constant) first element of the right part.
               The loop terminates when an element has been displaced or the
               test limit has been reached.
            */
            A(p==pointers[s]);
            /* limited-range linear search */
            if ((r=s+floor_lg(t-s))>t) r=t; /* test limit */
            for (o=s; s<r; p=pointers[++s]) {
                /* Partial order stability is maintained by only displacing
                   left side elements with strictly smaller right side elements.
                */
                A(p==pointers[s]);
                if (0<COMPAR(p,q)) { /* q displaces p */
                    if ((s==o) /* first element of left part */
                      /* test 1st 1st part element vs. last 2nd part element */
                    && (0<COMPAR(p,pointers[u-1UL]))
                    ) {
                        /* entire left larger than right; swap by rotation */
                        irotate(base,s,t,u,size,swapf,alignsize,size_ratio);
#ifdef QUICKSELECT_COUNT_ROTATIONS
                        QUICKSELECT_COUNT_ROTATIONS(u-s,size_ratio);
#endif
                        return; /* done. */
                    }
                    A(p==pointers[s]);A(q==pointers[t]);
                    pointers[s]=q,pointers[t]=p; /* displace (swap pointers) */
                    break;
                }
            }
            if (s==r) { /* no displacement in initial scan */
                /* test last 1st part element vs. first 2nd part element (q) */
                A(s<=t);
                if (s>=t) /* already tested; sorted; */
                    return; /* done. */
                p=pointers[t-1UL];
                if (0>=COMPAR(p,q)) /* sorted (equality is OK) */
                    return; /* done. */
                /* element to be displaced is in [r,t-1); use binary search */
                if (t>1UL)
                    for (z=t-2UL; r<=z;) {
                        p=pointers[o=BS_MID_L(r,z)];
                        if (0<COMPAR(p,q)) z=o-1UL;
                        else r=o+1UL;
                    }
                A(q==pointers[t]);
                p=pointers[r];
                pointers[t]=p,pointers[r]=q; /* swap pointers */
                s=r;
            }
            /* An element from the 1st part has been displaced; further
               displacements (s) will be from the smaller of the smallest
               already-displaced elements (x) or the smallest 2nd part element
               (y).  s increases with displacements to t-1 (t is unchanged in
               this loop).  Displaced elements in [x,y) are maintained in sorted
               order by rotation as necessary.
            */
            p=pointers[x=t],s++,q=pointers[y=t+1UL];
            for (; (s<t)&&(y<u); ++s) {
                A(p==pointers[x]);A(q==pointers[y]);
                if (0<COMPAR(p,q)) { /* 2nd part smallest is smaller */
                    /* displace s by y++, update q */
                    pointers[y]=pointers[s],pointers[s]=q;
                    if (++y<u) q=pointers[y];
                    /* x and p should remain unchanged */
                } else { /* displaced is <= 2nd part smallest */
                    /* displace s by x, rotate displaced elements */
                    pointers[x]=pointers[s],pointers[s]=p;
                    if (y>x+1UL) {
                        irotate(base,x,x+1UL,y,size,swapf,alignsize,size_ratio);
#ifdef QUICKSELECT_COUNT_ROTATIONS
                        QUICKSELECT_COUNT_ROTATIONS(y-x,size_ratio);
#endif
                    }
                    /* index x is the same, but the pointer has changed */
                    p=pointers[x];
                    /* y and q should remain unchanged */
                }
            }
            /* displaced all 1st part elements; start again with displaced
               elements as part1 and remaining part2 elements
            */
            A((s==t)||(y==u));
            /* next iteration uses previously displaced elements as the first
               part, and the remainder of the second part
               If this part is reached, there was at least one displaced element
            */
            if (s==t) p=pointers[s=x],q=pointers[t=y];
            else p=pointers[s],q=pointers[t];
        }
    }
}

/* merge_pointers is called by pointer_mergesort, which is called by itself,
   indirect_mergesort, and limited_indirect_mergesort (from dedicated_sort).
*/
/* Indirect merge sort uses pointers to data elements, with additional space for
   displaced pointers. Note that the additional space is beyond the pointers for
   all data elements (even those not being merged in this instance) and is used
   by all instances; this precludes (N.B.) parallel merge execution.
   Initially there are two adjacent sorted regions followed by additional
   pointers corresponding to other elements in the original array, followed by
   extra pointers which will be used to hold pointers displaced from the first
   sorted region.  Two pointers are maintained into that displaced pointer
   region to indicate the head and tail of displaced elements; as elements are
   displaced from the first sorted region, they are appended at the tail, and
   elements at the head are compared to the smallest of the remaining second set
   of sorted elements.  The smaller of the two displaces the first element
   remaining in the first sorted set, displacing that element. When the end of
   the second sorted set is reached, any remaining displaced elements are moved
   to complete the merge.
   +------------------------------------{ {----------------+
   |   sorted1   |   sorted2   | others       |   NULL     |
   +------------------------------------{ {----------------+
                               |
                               V
   +------------------------------------{ {----------------+
   | merged | s1 |  xx |  s2   | others       | displaced  |
   +------------------------------------{ {-----^------^---+
                               |                head   tail
                               V
   +------------------------------------{ {----------------+
   |           merged          | others       |   xxxxxx   |
   +------------------------------------{ {----------------+
*/
#define C_OPT 0U
static INDIRECT_INLINE
void merge_pointers(char **sorted1, char **sorted2, char **end2,
    char **displaced, COMPAR_DECL)
{
    register char **dhead, **dtail, *e1, *e2, **s1=sorted1, **s2=sorted2,
        **lim=sorted1+floor_lg(sorted2-sorted1);
    register size_t a, b, z; /* binary search indices */
    /* Compare first elements of each sorted region until an element from the
       first region is displaced.  Variables e1 and e1 point to the elements in
       sorted regions 1 and 2.  For random input, the first displaced element is
       almost always within the first few elements (50% probability that it is
       the first element, 75% that it is one of the first two, 87.5% that is is
       one of the first three, etc.), so use a linear scan for a few elements.
       Constant-value input would entail testing every element in the first
       piece; testing the last element of the first part vs. the first element
       of the second part would terminate the search.  But binary-valued input
       (random zeros and ones) has an expected position for the first
       displacement about half-way into the first part (so binary search would
       be attractive).  Constant-value input is less likely than M-ary input,
       which is less likely than nearly-random input.  The strategy is therefore
       to use a linear search for the first few elements, and if that does not
       find an element to displace, test the last element of the first part vs.
       the first element of the second part (in case the input is already
       sorted), then test the first element of the first part against the last
       element of the second part to see if the parts should be swapped, and if
       that also fails, then switch to a binary search within the remainder of
       the first part.  That is expected to work well for most random inputs,
       and to be only slightly suboptimal for M-ary, reversed, and constant
       inputs.
    */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG)) {
        (V)fprintf(stderr, "/* %s: nmemb=%lu, n1=%lu, n2=%lu, displaced=%lu "
            "*/\n",__func__,end2-sorted1,sorted2-sorted1,end2-sorted2,
            displaced-sorted1);
    }
#endif

#if defined(DEBUGGING)
    nmerges++;
#endif
    if (lim==sorted1) lim++;
    if (lim>sorted2) lim=sorted2;
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
        (V)fprintf(stderr, "/* %s: lim=%p[%lu], sorted2=%p[%lu] */\n",
            __func__,(void *)lim,lim-sorted1,(void *)sorted2,sorted2-sorted1);
#endif
    for (dhead=dtail=displaced,e2=*s2; s1<lim; s1++)
    {
        e1=*s1;
#if defined(DEBUGGING)
        npderefs++;
#endif
        if (0<OPT_COMPAR((const void *)e1,(const void *)e2,C_OPT)) {
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
            if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
                (V)fprintf(stderr, "/* %s e1=%p[%lu] > e2=%p[%lu], "
                    "lim-sorted1=%lu, sorted2-sorted1-1=%lu */\n",__func__,e1,
                    s1-sorted1,e2,s2-sorted1,lim-sorted1,
                    (sorted2-sorted1)-1UL);
#endif
            if (s1==sorted1) { /* 1st part 1st element */
                /* 2nd part last element vs. 1st part 1st element */
                char **l2=end2-1, *e3=*l2;
                if (0<OPT_COMPAR((const  void *)e1,(const void *)e3,C_OPT)) {
                    /* swap parts */
                    protate((char *)s1,(char *)s2,(char *)end2,sizeof(char *),
                        pointerswap,sizeof(char *),1UL);
                    return;
                }
            }
            *(dtail++)=*s1, *s1=*(s2++);
            break;
        }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        else {
            if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
                (V)fprintf(stderr, "/* %s e1=%p[%lu] <= e2=%p[%lu], "
                    "lim-sorted1=%lu, sorted2-sorted1-1=%lu */\n",__func__,e1,
                    s1-sorted1,e2,s2-sorted1,lim-sorted1,
                    (sorted2-sorted1)-1UL);
       }
#endif
    }
    if (s1==lim) { /* nothing displaced; try last element */
        if (s1==sorted2) {
            return; /* done */
        }
        e1=*(sorted2-1); /* last element of first region */
#if defined(DEBUGGING)
        npderefs++;
#endif
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
            (V)fprintf(stderr, "/* %s none displaced: e1=%p[%lu], e2=%p[%lu], "
                "lim-sorted1=%lu, sorted2-sorted1-1=%lu */\n",__func__,e1,
                (sorted2-sorted1)-1UL,e2,s2-sorted1,lim-sorted1,
                (sorted2-sorted1)-1UL);
#endif
        if (0>=OPT_COMPAR((const void *)e1,(const void *)e2,C_OPT)) {
            return; /* done (already sorted) */
        }
        /* nothing displaced, but not yet merged; try binary search */
        A(s1==lim);
        /* already tested 1st part last element e1@sorted2-1 (index b below) */
        a=lim-sorted1,b=(sorted2-sorted1)-1UL,z=(b>0UL?b-1UL:0UL);
        if (a>b) a=b; /* not past last element of first part */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
            (V)fprintf(stderr, "/* %s none displaced, pre-binary search: a=%lu,"
                " z=%lu */\n",__func__,a,z);
#endif
        for (; a<=z; ) {
            b=BS_MID_L(a,z);
            e1=sorted1[b]; /* b is mid-range index */
#if defined(DEBUGGING)
            npderefs++;
#endif
            if (0<OPT_COMPAR((const void *)e1,(const void *)e2,C_OPT))
                z=b-1UL; /* try farther left */
            else a=b+1UL; /* try farther right */
        }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
            (V)fprintf(stderr, "/* %s post-binary search: a=%lu, b=%lu, z=%lu, "
                "s1 %p, s2 %p */\n",__func__,a,b,z,*(sorted1+a),*s2);
#endif
        s1=sorted1+a, e1=*(dtail++)=*s1, *s1=*(s2++);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG)) {
            (V)fprintf(stderr, "/* %s displaced a=%lu -> dhead=%lu with s1=%lu "
                "*/\n",__func__,a,dhead-sorted1,s1-sorted1);
        }
#endif
    }
#if SILENCE_WHINEY_COMPILERS /* e1 really has already been set... */
      else e1=*dhead;
#endif
#if defined(DEBUGGING)
    npderefs+=2UL, npcopies+=2UL;
#endif
#if 1
    A(dtail>dhead);
#endif
    /* Now comparisons are between the head of the displaced region and the
       smallest element in the second sorted region, until all elements from
       the first sorted set have been replaced.  Variable e1 now points to the
       smallest displaced element.
    */
#if 1
#if INDIRECT_ASSERT_CODE /* e1 is always *dhead here. */
if (e1!=*dhead) fprintf(stderr,"/* %s: e1=%p, *dhead=%p */\n",__func__,e1,*dhead);
    A(e1==*dhead);
#endif
#endif
    for (e2=*s2,s1++; s1<sorted2; s1++) {
        *(dtail++)=*s1; /* element from the first part is always displaced */
#if defined(DEBUGGING)
        npderefs+=2UL, npcopies++;
#endif
        A(s1!=s2);
        if (0<OPT_COMPAR((const void *)e1,(const void *)e2,C_OPT))
            *s1=*(s2++), e2=*s2; /* displaced by part2 element */
        else
            *s1=*(dhead++), e1=*dhead; /* by earlier displaced element */
    }
    A(dhead<=dtail);
    /* After the first set has been replaced, comparisons between the smallest
       elements from the second sorted set and the displaced region continue
       until one of those regions is exhausted.
    */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MERGE_DEBUG))
        (V)fprintf(stderr, "/* %s: displaced vs. set 2 */\n",__func__);
/* XXX could check for last displaced < first remaining of 2nd set, and last of
   2nd set vs. first displaced; if one of those conditions is met, pointers can
   be moved w/o further comparisons.  That would only make sense if there are
   more than two remaining displaced pointers and more than two remaining
   unmerged pointers to elements in the second part (otherwise there are
   probably fewer comparisons as-is).
*/
#endif
#if 0 /* e1=*dhead and e2=*s2 assignments are necessary here. */
if (e1!=*dhead) fprintf(stderr,"/* %s: e1=%p, *dhead=%p */\n",__func__,e1,*dhead);
if (e2!=*s2) fprintf(stderr,"/* %s: e2=%p, *s2=%p */\n",__func__,e2,*s2);
#endif
    A(s1==sorted2);
#if INDIRECT_ASSERT_CODE
    if (s1!=sorted2) { fprintf(stderr,"s1!=sorted2\n"); abort(); }
#endif
    for (e1=*dhead,e2=*s2; (s2<end2)&&(dhead<dtail); s1++) {
#if defined(DEBUGGING)
        npderefs+=2UL, npcopies++;
#endif
        if (0<OPT_COMPAR((const void *)e1,(const void *)e2,C_OPT)) {
            if (s1!=s2) *s1=*(s2++); else s2++;
            e2=*s2;
        } else
            *s1=*(dhead++), e1=*dhead;
    }
    A(dhead<=dtail);
    /* Finally, append remaining displaced pointers to the merged pointers. */
    while (dhead<dtail) {
        *(s1++)=*(dhead++);
#if defined(DEBUGGING)
        npcopies++;
#endif
    }
    A(dhead==dtail);
}

/* quickselect_loop declaration */
#if ! defined(QUICKSELECT_LOOP_DECLARED)
QUICKSELECT_EXTERN
# include "quickselect_loop_decl.h"
;
# define QUICKSELECT_LOOP_DECLARED 1
#endif /* QUICKSELECT_LOOP_DECLARED */

/* Indirectly sort by mergesort an array of nmemb elements at base, pointed to
   by nmemb pointers starting at pointers (and with more NULL pointers after
   last element pointer (starting at beyond)), using element comparison function
   compar.  Sorting is per mergesort; first the original set of pointers is
   split into two approximately equal-sizes pieces.  Recursive
   divide-and-conquer is used to sort the split pieces, which are then merged.
*/
/* dereferenced pointers; no OPT_COMPAR required. */
static INDIRECT_INLINE
void pointer_mergesort(char **pointers, size_t pfirst, char *base, size_t nmemb,
    size_t pbeyond, COMPAR_DECL, unsigned int table_index, size_t cache_sz,
    unsigned int options)
{
    if (1UL<nmemb) { /* size 1 (or smaller) is by definition sorted */
        size_t n1=(nmemb>>1), n2=nmemb-n1;
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr, "/* %s: nmemb=%lu, n1=%lu, n2=%lu, pfirst=%lu, "
                "pbeyond=%lu */\n",__func__,nmemb,n1,n2,pfirst,pbeyond);
#endif
        /* N.B. because the displaced pointers region is shared, parallel
           execution of merges is not possible (without significant modifcation
           of the merge code, and without a reasonably low-overhead mechanism
           for coordination of parallel execution). [recursive calls could
           specify different offsets into the displaced pointers region, but
           in the absence of low-overhead parallel execution, there's no point.]
        */
        A(0UL<n1);A(0UL<n2); /* no empty regions! */
        A(n2>=n1);A(n1+1UL>=n2); /* balanced, part 2 at least as large as 1 */
        if (1UL==n2) { /* simple case: nmemb=2 (1UL<=n1<=n2==1UL) */
            /* indirect compare-exchange */
            register char *pa, *pb;
            pa=pointers[pfirst], pb=pointers[pfirst+1UL];
            if (0<COMPAR(pa,pb))
                pointers[pfirst]=pb, pointers[pfirst+1UL]=pa;
        } else { /* sort parts, merge */
            size_t nb=(n1>>1);
            if (1UL<n1) { /* nothing to do for a single element (pointer) */
                /* Use of dedicated_sort permits optimization for speed or
                   number of comparisons.
                */
                (V)QUICKSELECT_LOOP((char *)pointers,pfirst,pfirst+n1,
                        sizeof(char *),COMPAR_ARGS,NULL,0UL,0UL,
                        pointerswap,sizeof(char *),1UL,table_index,
                        cache_sz,pbeyond,options|QUICKSELECT_INDIRECT,NULL,NULL);
# if ASSERT_CODE > 1
                for (n2=1UL; n2<n1; n2++) {
                    char *pb=pointers[pfirst+n2], *pa=pointers[pfirst+n2-1UL];
                    if (0<COMPAR(pa,pb)) {
                        (V)fprintf(stderr,"/* %s line %d: error pa %p[%lu] >"
                            " pb %p[%lu] */\n",__func__,__LINE__,pa,
                            pfirst+n2-1UL,pb,pfirst+n2);
                        print_some_array(base,pfirst,pbeyond-1UL,"/* "," */",
                            options);
                        A(0==1);
                        abort();
                    }
                }
                n2=nmemb-n1; /* restore correct value */
# endif
            }
            A(1UL<n2);
            (V)QUICKSELECT_LOOP((char *)pointers,pfirst+n1,pfirst+nmemb,
                sizeof(char *),COMPAR_ARGS,NULL,0UL,0UL,
                pointerswap,sizeof(char *),1UL,table_index,
                cache_sz,pbeyond+nb,options|QUICKSELECT_INDIRECT,NULL,NULL);
            merge_pointers(pointers+pfirst,pointers+pfirst+n1,
                pointers+pfirst+nmemb,pointers+pbeyond,COMPAR_ARGS);
        }
    }
}

/* rearrange_array is called from limited_indirect_mergesort (dedicated_sort),
   linear_partition (partition), merge_partitions (partition), quickselect[_s],
   and indirect_mergesort (mergesort and indirect_mergesort).
*/
static INDIRECT_INLINE
size_t rearrange_array(char *base, size_t nmemb, size_t size, size_t *indices,
    size_t nindices, size_t first, size_t beyond, size_t alignsize)
{
    size_t ndata_moves=0UL;

    if ((nindices < nmemb)
    || (NULL==base)
    || (NULL==indices)
    || (0UL==size)
    || (first>beyond)
    || (first+nmemb<beyond)
    ) {
        errno=EINVAL;
        return SIZE_MAX;
    }
    if (0UL<nmemb) {
        /* Step through the indices: at the first index which points to an
           array element whose position cpos=((address-base)/size) differs from
           the index position fpos=((p-indices)/sizeof(char *)), find the cycle
           in the array which needs to be rotated to permute the array into the
           desired order.  Folow that cycle, rotating the data by alignsize
           slices until the elements have been rearranged, then update the
           indices.  Continue, looking for the next cycle until all cycles have
           been processed. Indices are unconditionally updated by the algorithm.
           The return value is the number of data moves.
           indices[0] corresponds to the data element at base+first*size
        */
        register size_t cpos, hpos, ppos, n, o, slice;
        register char *cp, *p, *hp, *q;
#if INDIRECT_ASSERT_CODE
        cp=base+first*size, hp=base+beyond*size;
        /* index sanity check; n counts errors */
        /* this is a check on caller-supplied data */
        for (n=0UL,hpos=0UL; hpos<nmemb; hpos++) {
            cpos=indices[hpos];
            if ((first<=cpos)&&(cpos<beyond)) continue;
            (V)fprintf(stderr,
                "%s: indices[%lu]=%lu is out of bounds: first=%lu, "
                "beyond=%lu\n",
                __func__,hpos,cpos,first,beyond);
            n++;
        }
        A(0UL==n);
#endif
        /* Loop through indices [0,nmemb) and array pointers; pointer
           updated by inexpensive increment. Skip over in-place elements
           (final index matches position).  Follow a cycle starting at
           the next out-of-place element found, rearranging elements in
           the cycle and updating their indices to indicate that they
           have been placed in position. In-place detection is intended
           to be fast.
        */
        /* Four variants, depending on alignsize; identical other than type. */
/* type finagle */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define TYPE8 uint_least64_t
# define TYPE4 uint_least32_t
# define TYPE2 uint_least16_t
#else
# if ( LONG_MAX > 2147483647 ) && ( LONG_MAX == 9223372036854775807 )
#  define TYPE8 long
# elif DBL_MAX_10_EXP == 308
#  define TYPE8 double
# else
#  define TYPE8 char *
# endif /* 8 byte types */
# if INT_MAX == 2147483647
#  define TYPE4 int
# elif LONG_MAX == 2147483647
#  define TYPE4 long
# endif /* 4 byte types */
# define TYPE2 short
#endif /* C99 */
#define REARRANGE_ARRAY(mtype)                                                \
for (n=0UL,o=first,p=base+o*size; n<nmemb; n++,o++,p+=size) {                 \
    cpos=indices[n];/*current index element to be at first+n*/                \
    if (cpos==o) continue; /* already in final position */                    \
    /* data elements' slice loop */                                           \
    for (ppos=cpos,slice=0UL; slice<size; p+=alignsize,slice+=alignsize) {    \
        mtype t;                                                              \
        /* follow cycle */                                                    \
        for (t=*((mtype *)p),cpos=ppos,hpos=o,hp=p,q=base+slice; cpos!=o; ) { \
            cp=q+cpos*size;                                                   \
            *((mtype *)hp) = *((mtype *)cp);                                  \
            hpos=cpos, hp=cp; cpos=indices[hpos-first];                       \
        } /* data movement cycle loop */                                      \
        *((mtype *)cp) = t;                                                   \
    } /* elements' slice loop */                                              \
    /* 2nd loop over cycle (elements): update indices, count moves */         \
    for (ndata_moves++,p-=size,cpos=indices[n],hpos=o; cpos!=o; hpos=cpos) {  \
        cpos=indices[hpos-first]; indices[hpos-first]=hpos; ndata_moves++;    \
    } /* index update cycle loop */                                           \
} /* array index loop */
        switch (alignsize) {
            case 8UL : REARRANGE_ARRAY(TYPE8)
            break;
            case 4UL : REARRANGE_ARRAY(TYPE4)
            break;
            case 2UL : REARRANGE_ARRAY(TYPE2)
            break;
            default : REARRANGE_ARRAY(char)
            break;
        }
    }
    return ndata_moves;
}

/* for assert.h */
#undef NDEBUG
#if ! ASSERT_CODE
# define NDEBUG 1
#else
# define NDEBUG 0
#endif

#define	INDIRECT_H_INCLUDED
#endif
