/*INDENT OFF*/

/* Description: C source code for quickselect_loop */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    quickselect_loop_src.h copyright 2017 Bruce Lilly.   \ quickselect_loop_src.h $
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
/* $Id: ~|^` @(#)   This is quickselect_loop_src.h version 1.11 dated 2017-11-03T20:35:38Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect_loop_src.h */

/********************** Long description and rationale: ***********************
 An implementation of multiple quickselect similar to that described by Mahmoud
 & Lent in "Average-case analysis of multiple Quickselect: An algorithm for
 finding order statistics".  The array of items to be sorted by quickselect,
 the array element size, and the comparison function are declared as for qsort,
 with two additional parameters to specify an optional array of desired order
 statistic ranks.
 N.B. the values for the order-statistic array elements are zero-based ranks
 (i.e. 0UL for the minimum, N-1UL for the maximum of an array of N elements,
 (N-1UL)/2UL and N/2UL for the lower- and upper-medians).
 The implementation can also be used to implement qsort; a wrapper function
 (which could alternatively be implemented as a macro) is provided for that
 purpose.
 In order to be able to compute the rank of array elements for selection, the
 initial array base is maintained, and a pair of indices brackets the sub-array
 being processed.  The indices correspond to the ranks of elements.  A similar
 scheme is used by Musser's introsort, described in "Introspective Sorting and
 Selection Algorithms".  The same method is used for the array of desired order
 statistic ranks, similar to that described by Lent & Mahmoud.
 Pivot selection uses Rousseeuw & Basset's remedian, described in "The Remedian:
 A Robust Averaging Method for Large Data Sets", on a sample of a size which is
 a power of 3 taken from the array elements, with sample size approximately the
 square root of the array size as described in "Optimal Sampling Strategies for
 Quicksort" by McGeoch and Tygar, and in "Optimal Sampling Strategies in
 Quicksort and Quickselect" by Martinez and Roura.
 Elements sampled from the array for use by the pivot selection methods are
 chosen to provide good overall performance, avoiding choices that would fare
 poorly for commonly-seen input sequences (e.g. organ-pipe, already-sorted).
 Partitioning uses a method described by Bentley & McIlroy, with a modification
 to reduce disorder induced in arrays.
 Comparisons are made between array elements; there is no special-case code nor
 initialization macro.
 Partitioning can still result in an unfavorable split of the initial sub-array;
 when this happens, an emergency "break-glass" pivot selection function is used
 to ensure a better split, avoiding quadratic behavior even when handling a
 worst-case input array, such as can be generated by an adversary function
 (M. D. McIlroy "A Killer Adversary for Quicksort").
 Break-glass pivot selection uses median-of-medians using sets of 3 elements
 (ignoring leftovers) and using a call to quickselect to find the median of
 medians, rather than recursion.  Aside from those details, it is similar to the
 method described by Blum, Floyd, Pratt, Rivest, & Tarjan in "Time Bounds for
 Selection".
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
#if defined(_XOPEN_SOURCE) && defined(MIN_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && defined(MAX_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
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
#define ID_STRING_PREFIX "$Id: quickselect_loop_src.h ~|^` @(#)"
#define SOURCE_MODULE "quickselect_loop_src.h"
#define MODULE_VERSION "1.11"
#define MODULE_DATE "2017-11-03T20:35:38Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"

/* Although the implementation is different, several concepts are adapted from:
   qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

/* local header files needed */
#include "quickselect_config.h"
#include "quickselect.h"        /* quickselect QSORT_FUNCTION_NAME */
#include "tables.h"     /* sampling_table_struct SAMPLING_TABLE_SIZE */
#if ! QUICKSELECT_BUILD_FOR_SPEED
#include "initialize_src.h"
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

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
#endif /* C99 or later */

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* dedicated_sort declaration */
#include "dedicated_sort_decl.h"
;
/* klimits declaration */
#include "klimits_decl.h"
;
/* select_pivot declaration */
#include "select_pivot_decl.h"
;
/* partition declaration */
#include "partition_decl.h"
;
/* quickselect_loop declaration */
#include "quickselect_loop_decl.h"
;
/* sampling_table declaration */
#include "sampling_table_decl.h"
;
/* should_repivot declaration */
#include "should_repivot_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* Special-case selection of minimum, maximum, or both. */
/* called from quickselect_loop */
/* Keeps track of (and relocates if necessary) elements comparing equal to the
   desired (minimum or maximum) element if ppeq,ppgt are not NULL.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void SELECT_MIN_FUNCTION_NAME(char *base,size_t first,size_t beyond,size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
{
    if (beyond>first+1UL) {
        char *mn, *p, *q, *r;
        mn=p=base+first*size,q=p+size,r=base+beyond*size;
        /* both ppeq and ppgt are NULL, or both are non-NULL */
        A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
        if (NULL==ppeq) { /* non-partitioning */
            for (; q<r; q+=size)
                if (0<COMPAR(mn,q,context)) mn=q;
            if (mn!=p) {
                /* stability requires rotation rather than swaps */
                if (0U==((QUICKSELECT_STABLE)&options)) {
                    EXCHANGE_SWAP(swapf,p,mn,size,alignsize,size_ratio,/**/);
                } else {
                    /* |      mn      | */
                    protate(p,mn-size,mn,size,swapf,alignsize,size_ratio);
                }
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            for (; q<r; q+=size) {
                int c=COMPAR(mn,q,context);
                if (0>c) continue;
                if (0==c) {
                    mn+=size;
                    if (q==mn) continue;
                } else /* 0<c */ mn=p; /* new min */
                A(mn!=q);
                EXCHANGE_SWAP(swapf,mn,q,size,alignsize,size_ratio,/**/);
            }
            if (NULL!=ppeq) *ppeq=p; /* leftmost min */
            if (NULL!=ppgt) *ppgt=mn+size; /* beyond rightmost min */
        }
    } else { /* beyond==first+1UL; 1 element */
        if (NULL!=ppeq) *ppeq=base+first*size;
        if (NULL!=ppgt) *ppgt=base+beyond*size;
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void SELECT_MAX_FUNCTION_NAME(char *base,size_t first,size_t beyond,size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options, char **ppeq, char **ppgt)
{
    if (beyond>first+1UL) {
        char *mx, *p, *r;
        p=base+first*size,mx=r=base+(beyond-1UL)*size;
        /* both ppeq and ppgt are NULL, or both are non-NULL */
        A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
        if ((NULL==ppeq)&&(NULL==ppgt)) { /* non-partitioning */
            for (; p<r; p+=size)
                if (0>COMPAR(mx,p,context)) mx=p;
            if (mx!=r) {
                /* stability requires rotation rather than swaps */
                if (0U==((QUICKSELECT_STABLE)&options)) {
                    EXCHANGE_SWAP(swapf,r,mx,size,alignsize,size_ratio,/**/);
                } else {
                    /* |      mx      | */
                    protate(mx,mx+size,r,size,swapf,alignsize,size_ratio);
                }
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            char *q;
            for (q=r-size; q>=p; q-=size) {
                int c=COMPAR(mx,q,context);
                if (0<c) continue;
                if (0==c) {
                    mx-=size;
                    if (q==mx) continue;
                } else /* 0>c */ mx=r; /* new max */
                A(mx!=q);
                EXCHANGE_SWAP(swapf,mx,q,size,alignsize,size_ratio,/**/);
            }
            if (NULL!=ppeq) *ppeq=mx; /* leftmost max */
            if (NULL!=ppgt) *ppgt=r+size; /* beyond rightmost max */
        }
    } else { /* beyond==first+1UL; 1 element */
        if (NULL!=ppeq) *ppeq=base+first*size;
        if (NULL!=ppgt) *ppgt=base+beyond*size;
    }
}

/* Recursive divide-and-conquer method to find both minimum and maximum.
   Split into two parts; (recursively) find minimum and maximum of each part.
   Overall minimum is the smaller of the two minima; overall maximum is the
   larger of the two maxima.
*/
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void FIND_MINMAX_FUNCTION_NAME(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, char **pmn, char **pmx)
{
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
                FIND_MINMAX_FUNCTION_NAME(base,first,first+na,size,COMPAR_ARGS,&mna,&mxa);
                FIND_MINMAX_FUNCTION_NAME(base,first+na,beyond,size,COMPAR_ARGS,&mnb,&mxb);
                /* overall min is smaller of *mna, *mnb; similarly for max */
                /* stability requires choosing mna if mna compares equal to mnb */
                if (0<COMPAR(mna,mnb,context)) mn=mnb; else mn=mna;
                /* stability requires choosing mxb if mxa compares equal to mxb */
                if (0<COMPAR(mxa,mxb,context)) mx=mxa; else mx=mxb;
            } else { /* nmemb==2UL */
                char *a, *z;
                /* first and last (i.e. second) elements */
                a=base+first*size,z=a+size;
                if (0<COMPAR(a,z,context)) mn=z,mx=a; else mn=a,mx=z; /* stable */
            }
            *pmn=mn, *pmx=mx;
        } else /* 1 element; min==max */
            *pmn=*pmx=base+first*size;
    }
}

/* Selection of both minimum and maximum using recursive find_minmax. */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void SELECT_MINMAX_FUNCTION_NAME(char *base,size_t first,size_t beyond,size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options)
{
    char *mn, *mx, *a, *z;
    A(beyond>first);
    /* first and last elements */
    a=base+first*size,z=base+(beyond-1UL)*size;
    FIND_MINMAX_FUNCTION_NAME(base,first,beyond,size,COMPAR_ARGS,&mn,&mx);
    /* put min and max in place with at most 2 swaps */
    /* stability requires rotation rather than swaps */
    /* +----------------------------------+ */
    /* |                                  | */
    /* +----------------------------------+ */
    /* +a         x               n      z+ */
    /* nothing to do if mx and mn are in place, i.e. mn==a and mx==z */
    if ((mn!=a)||(mx!=z)) {
        /* special case: mx==a and mn==z */
        if ((mx==a)&&(mn==z)) {
            EXCHANGE_SWAP(swapf,a,z,size,alignsize,size_ratio,/**/);
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
                    EXCHANGE_SWAP(swapf,a,mn,size,alignsize,size_ratio,/**/);
                if (z!=mx) /* beware case where a was mx before above swap! */
                    EXCHANGE_SWAP(swapf,z,a==mx?mn:mx,size,alignsize,size_ratio,
                        /**/);
            }
        }
    }
}

/* function to consolidate logic and calls to special-case selection functions */
/* also sets sampling table and index, determines whether to sort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int special_cases(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, size_t cutoff, size_t nmemb, const size_t *pk,
    size_t firstk, size_t beyondk, unsigned int options, char **ppeq,
    char **ppgt, struct sampling_table_struct **ppst, unsigned int *psort,
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
                        SELECT_MIN_FUNCTION_NAME(base,first,beyond,size,
                            COMPAR_ARGS,swapf,alignsize,size_ratio,options,ppeq,ppgt);
                        if (fk==sp) {
                            SELECT_MIN_FUNCTION_NAME(base,sp,beyond,size,
                                COMPAR_ARGS,swapf,alignsize,size_ratio,options,ppeq,
                                ppgt);
                            if ((NULL!=ppeq/*)&&(NULL!=ppgt)*/)
                            &&(0==COMPAR(*ppeq-size,*ppeq,context)))
                                (*ppeq)-=size;
                        }
                        return 1 ;
                    } else if ((ek==lp)||(ek==pp)) {
                        SELECT_MAX_FUNCTION_NAME(base,first,beyond,size,
                            COMPAR_ARGS,swapf,alignsize,size_ratio,options,ppeq,ppgt);
                        if (ek==pp) {
                            SELECT_MAX_FUNCTION_NAME(base,first,lp,size,
                                COMPAR_ARGS,swapf,alignsize,size_ratio,options,ppeq,
                                ppgt);
                            if ((NULL!=ppeq/*)&&(NULL!=ppgt)*/)
                            &&(0==COMPAR(*ppeq,*ppgt,context)))
                                (*ppgt)+=size;
                        }
                        return 1 ;
                    }
                break;
                case 2UL : /* 2 smallest, 2 largest, or smallest & largest */
                    A(NULL==ppeq);
                    if (fk==first) {
                        if (ek==lp) { /* smallest & largest */
                            SELECT_MINMAX_FUNCTION_NAME(base,first,beyond,size,
                                COMPAR_ARGS,swapf,alignsize,size_ratio,options);
                            return 1 ;
                        } else if (ek==sp) { /* 2 smallest */
                            SELECT_MIN_FUNCTION_NAME(base,first,beyond,size,
                                COMPAR_ARGS,swapf,alignsize,size_ratio,options,NULL,
                                NULL);
                            SELECT_MIN_FUNCTION_NAME(base,sp,beyond,size,
                                COMPAR_ARGS,swapf,alignsize,size_ratio,options,NULL,
                                NULL);
                            return 1 ;
                        }
                    } else if ((fk==pp)&&(ek==lp)) { /* 2 largest */
                        SELECT_MAX_FUNCTION_NAME(base,first,beyond,size,
                            COMPAR_ARGS,swapf,alignsize,size_ratio,options,NULL,NULL);
                        SELECT_MAX_FUNCTION_NAME(base,first,lp,size,
                            COMPAR_ARGS,swapf,alignsize,size_ratio,options,NULL,NULL);
                        return 1 ;
                    }
                break;
            }
        }
    }
    A(*pindex < (SAMPLING_TABLE_SIZE));
    *ppst=sampling_table(first,beyond,pk,firstk,beyondk,ppeq,psort,
        pindex,nmemb);
    return ret;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void rank_tests(char *base, size_t first, size_t p, size_t q,
    size_t beyond, size_t size, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *plk, size_t *prk, char **ppeq, char **ppgt)
{
    /* revised range of order statistic ranks */
    /* < region indices [first,p), order statistics [firstk,lk) */
    /* > region indices [q,beyond), order statistics [rk,beyondk) */
    klimits(p,q,pk,firstk,beyondk,plk,prk);
    /* Only applicable for median-of-medians (one order statistic (the median)
       with non-NULL ppeq and ppgt pointers); extents of region containing
       median.
    */
    if ((0UL==firstk)&&(1UL==beyondk)&&(NULL!=ppeq)) {
        size_t r=pk[0]; /* desired rank */
        A(p<q);A(first<=p);A(q<=beyond);
        /*Update range pointers if median (of medians) is [in] one of 3 regions.
          This is the only place where the extents (ppeq and ppgt) are updated;
          when the desired (median of medians) rank is in the = region or is the
          only element in the < or > region.
        */
        if ((p<=r)&&(r<q)) /* in = region */
            *ppeq=base+p*size, *ppgt=base+q*size;
        else if ((first+1UL==p)&&(first<=r)&&(r<p)) /* < region only element */
            *ppeq=base+first*size, *ppgt=base+p*size;
        else if ((q+1UL==beyond)&&(q<=r)&&(r<beyond)) /* > region only element */
            *ppeq=base+q*size, *ppgt=base+beyond*size;
        /* If the extents are set, none of the regions from this partition need
           be (and will not be) further partitioned.  Otherwise, only the region
           containing the median's (of medians) rank will be processed.
        */
    }
    A(*plk>=firstk);A(*prk<=beyondk);A(*plk<=*prk);
}

/* quickselect_loop definition (internal interface) */
/* Never called for nmemb<2UL. */

#if  __STDC_WANT_LIB_EXT1__
/* Preliminary support for 9899:201x draft N1570 qsort_s w/ "context".
   "context" is similar to glibc qsort_r; N1570 also has provision for
   runtime-constraint violation detection and handling. "Preliminary"
   because of the apparent lack of a convincing use-case for "context" and
   due to the poorly defined semantics and interface for constraint
   violation handling.

   Consequences of "context" are wide-ranging: any operation that requires
   comparisons (pivot selection, partitioning, dedicated sorting and/or
   selection, and support functions for any of those) has to be rewritten.
   The only operations unaffected are sampling, basic swaps, blockmoves using
   swaps, rank comparisons, and partition size evaluation; sampling and
   repivoting tables may also be reused without change.
   Executable code size is roughly doubled, not including external
   constraint-handler function code.
   Use case of "context" is questionable; one could simply use an appropriate
   comparison function.  Published supposed examples (e.g.
   https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/qsort-s
   tend to be rather silly (in the specific example above:
      1. functions are defined for specific locales, but not specifically
         implementing locale-specific collation -- one could at least as easily
         define locale-specific string comparison functions using the standard
         strcoll [+] function and use these with qsort to perform the
         locale-specific sorts with no need for "context", and avoiding the
         other implementation issues detailed below.
      2. Aside from inconsistent "context" argument order...
      3. ... the implementation comparison function copies (!) (leading portions
         of) supplied strings to fixed size (!!)[*] local buffers, then makes a
         second set of passes (!!!) over the copies to perform a conversion
         (which is discarded (!!!!) when the comparison function returns), which
         is then subjected to a third (!!!!!) pass to perform the comparison.
         Note that qsort implementations typically call the comparison function
         N log N times for an array of N elements; the above implementation
         makes 2 N log N copies and 6 N log N passes over data representing
         the strings to be sorted.  In particular, at each stage of partitioning
         qsort implementations compare a pivot element to many other elements;
         in the above implementation the string corresponding to that pivot
         element is copied, the copy modified and examined, only to be discarded
         before repeating those steps anew with the same pivot element's string
         for the next comparison.
      4. Although the copies (see #3 above) ignore the actual length of the
         original strings, strlen is subsequently called for each string (copy),
         resulting in a fourth (!!!!!!) pass over the data. Had strlen been
         called for the initial strings, the copies could have been made
         appropriate sizes (assuming that one still naively wished to make, then
         discard such copies).
      * a possibly-valid use of "context" *might* be to provide the maximum
        string length, avoiding magic numbers and failure to fully compare
        strings which have long identical leading substrings... *if* one
        insisted on the naive implementation involving copies (which are
        subsequently discarded) and multiple passes over the data.  It would
        be much more efficient to make one-time initial transformations (e.g.
        w/ strxfrm [+]) of the original string data supplemented by an index
        indicating the original order (or a pointer to the original string
        data), sorting the transformed and supplemented data using standard
        strcmp on the transformed strings.  That would involve N combined
        transformations and copies (instead of N log N copies and N log N
        transformations) plus N log N standard strcmp string comparisons (rather
        than 6 N log N passes over data), followed by access of the orginal
        string data via the indices or pointers for output.
      + Note that strcoll and strxfrm have been standardized since C89 (a.k.a.
        C90).
   ). In summary, there is no convincing use-case for "context" to justify
   doubling the size of library code.
*/
#endif /* __STDC_WANT_LIB_EXT1__ */

/* declaration */
#include "quickselect_loop_decl.h"
;

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
#include "quickselect_loop_decl.h"
{
#if __STDC_WANT_LIB_EXT1__
    QSORT_RETURN_TYPE ret=0;
#endif /* __STDC_WANT_LIB_EXT1__ */
    auto int c=0; /* repivot factor2 count */
    unsigned int table_index=2U;

    /* ppeq,ppgt consistency: both or neither NULL */
    A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file(__FILE__);
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
    for (;;) {
        unsigned int sort;
        char *pc, *pd, *pe, *pf, *pivot;
        size_t nmemb;
        struct region_struct lt_region, gt_region, *ps_region, *pl_region;
        struct sampling_table_struct *pst;

        A(first<beyond);
        nmemb=beyond-first;
#if 0
        A(2UL<=nmemb);
#else
        A(1UL<=nmemb);
#endif`

        /* Check for special-case selection: 1-2 order statistic ranks */
        A(table_index < (SAMPLING_TABLE_SIZE));
        if (0!=special_cases(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize,
            size_ratio,cutoff,nmemb,pk,firstk,beyondk,options,ppeq,ppgt,&pst,
            &sort,&table_index))
            RETURN ;
        A((SAMPLING_TABLE_SIZE)>table_index);
        A((0U==table_index)||(nmemb>pst[table_index-1U].max_nmemb));

        if (0U!=sort) { /* switch to sorting */
            if (nmemb<=cutoff) {
                if (1UL<nmemb) {
                    PREFIX DEDICATED_SORT(base,first,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options); TEST ;
                }
                RETURN ; /* done */
            }
            pk=NULL; /* sorting */ A(pst==sorting_sampling_table);
            /* proceed with pivot selection */
        }

        /* select a pivot element */
        pivot=SELECT_PIVOT_FUNCTION_NAME(base,first,beyond,size,COMPAR_ARGS,
            swapf,alignsize,size_ratio,table_index,pk,options,
            &pc,&pd,&pe,&pf); SUFFIX1
        A(NULL!=pivot);A(base+first*size<=pivot);A(pivot<base+beyond*size);
        A(pc<=pd);A(pd<pe);A(pe<=pf);A(pd<=pivot);A(pivot<pe);

        /* Partition the array around the pivot element into less-than,
           equal-to, and greater-than (w.r.t. the pivot) regions.  The
           equal-to region requires no further processing.
        */
        lt_region.first=first, gt_region.beyond=beyond;
        PARTITION_FUNCTION_NAME(base,first,beyond,pc,pd,pivot,pe,pf,size,
            COMPAR_ARGS,swapf,alignsize,size_ratio,options,
            &(lt_region.beyond),&(gt_region.first));
        /* regions: < [first,p)
                    > [q,beyond)
        */
        A(lt_region.beyond<gt_region.first);A(lt_region.beyond<=beyond);
        A(lt_region.first<=lt_region.beyond);
        A(gt_region.first<=gt_region.beyond);A(first<=gt_region.first);

        if (NULL!=pk) { /* selection only */
            rank_tests(base,first,lt_region.beyond,gt_region.first,beyond,size,
                pk,firstk,beyondk,&(lt_region.beyondk),&(gt_region.firstk),ppeq,
                ppgt);
            lt_region.firstk=firstk, gt_region.beyondk=beyondk;
        }

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
        /* default pivot selection uses array samples */
        options &= ~(QUICKSELECT_RESTRICT_RANK);

        /* Process less-than and/or greater-than regions by relative size. */
        if (0U!=pl_region->process) { /* Process large region. */
            if (0U!=ps_region->process) { /* Recurse on small region. */
                TEST ; PREFIX
                QUICKSELECT_LOOP(base,ps_region->first,ps_region->beyond,size,
                    COMPAR_ARGS,pk,ps_region->firstk,ps_region->beyondk,swapf,
                    alignsize,size_ratio,cutoff,options,ppeq,ppgt);
            } SUFFIX2
            /* Dedicated sort for large region? */
            if ((NULL==pk)&&(pl_region->beyond<=pl_region->first+cutoff)) {
                TEST ; PREFIX
                DEDICATED_SORT(base,pl_region->first,pl_region->beyond,size,
                    COMPAR_ARGS,swapf,alignsize,size_ratio,options); TEST ;
                RETURN ; /* done */
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
            A(first<beyond);
            options|=should_repivot(nmemb,beyond-first,cutoff,pst,table_index,
                pk,&c);
        } else { /* large region to be ignored; maybe iterate small region */
            if (0U!=ps_region->process) { /* Iterate on small region. */
                first=ps_region->first, beyond=ps_region->beyond,
                    firstk=ps_region->firstk, beyondk=ps_region->beyondk;
            } else RETURN ; /* Process neither; nothing left to do. */
        }
    } SUFFIX3
}
