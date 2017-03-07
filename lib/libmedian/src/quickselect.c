/*INDENT OFF*/

/* Description: C source code for quickselect */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    quickselect.c copyright 2016-2017 Bruce Lilly.   \ quickselect.c $
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
/* $Id: ~|^` @(#)   This is quickselect.c version 1.72 dated 2017-03-07T05:57:07Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.quickselect.c */

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
 initial array base is maintained, and a pair of indices brackets the subarray
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
 Partitioning can still result in an unfavorable split of the initial subarray;
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

/* compile-time configuration options */
#define ASSERT_CODE         0 /* Adds size and cost to aid debugging.
                                 0 for tested production code. */

/* Analysis at small N indicates an INSERTION_CUTOFF of 6 is optimal. */
#ifndef INSERTION_CUTOFF
# define INSERTION_CUTOFF   6UL /* insertion sort arrays this size or smaller */
#endif

/* Repivoting parameters LOPSIDED_LIMIT, REPIVOT_FACTOR, and REPIVOT_CUTOFF:
   Subarrays with at least REPIVOT_CUTOFF elements are checked for a large
   region which is at least (REPIVOT_FACTOR-1)/REPIVOT_FACTOR times the
   total number of elements in the subarray.
   To avoid integer overflow, the number of elements in the large region is
   divided by REPIVOT_FACTOR for comparison (bit shifts are used for
   REPIVOT_FACTOR at powers of 2).
   To avoid unnecessary repivoting on an occasional lopsided partition,
   repivoting only occurs if the number of lopsided partitions is at least
   LOPSIDED_LIMIT since the last repivot.
   Median-of-3 pivot selection guarantees at least 1 element in the small
   region.  That plus the pivot guarantees that the large region has at most
   nmemb-2 elements.
   Therefore, as long as REPIVOT_CUTOFF is at least median-of-3 cutoff
   REPIVOT_CUTOFF should be at least REPIVOT_FACTOR+2 to avoid unnecessary
   division and comparison.
   Small REPIVOT_FACTOR (e.g. 6UL) keeps adverse inputs from requiring
   excessive comparisons due to poor partition splits, but causes extra
   comparisons for random inputs due to an occasional lopsided partition.
   Large REPIVOT_FACTOR (e.g. 64UL) allows adverse inputs to require many
   comparisons to resolve marginally poor (not bad enough to repivot)
   partition splits, but has negligible effect on random sequences which
   rarely have extremely lopsided partitions.
*/
#ifndef LOPSIDED_LIMIT
# define LOPSIDED_LIMIT             3
#endif
#ifndef REPIVOT_FACTOR
# define REPIVOT_FACTOR             6UL
#endif
#ifndef REPIVOT_CUTOFF
# define REPIVOT_CUTOFF             12UL
#endif

/* When repivoting is necessary using median-of-medians, the middle third of
   the array is partitioned as a result of finding the median of the medians.
   This partial partitioning may be preserved to avoid some recomparisons
   when repartitioning. Careful coding does not affect regular partitioning,
   but object file size increases due to the additional code required to
   preserve and use the partial partitioning information to avoid some
   comparisons.
*/
#define SAVE_PARTIAL                1

/* Nothing to configure below this line. */

/* defaults */
#if ! LOPSIDED_LIMIT
# undef LOPSIDED_LIMIT
# define LOPSIDED_LIMIT             3
#endif
#if REPIVOT_FACTOR < 5UL
# undef REPIVOT_FACTOR
# define REPIVOT_FACTOR             6UL
#endif
#if ! REPIVOT_FACTOR
# ifdef REPIVOT_FACTOR
#  undef REPIVOT_FACTOR
#  define REPIVOT_FACTOR            6UL
# endif
#endif
#if ! REPIVOT_CUTOFF
# ifdef REPIVOT_CUTOFF
#  undef REPIVOT_CUTOFF
#  define REPIVOT_CUTOFF            12UL
# endif
#endif

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* Minimum _XOPEN_SOURCE version for C99 (else illumos compilation fails) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components
   are version control fields.
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: quickselect.c ~|^` @(#)"
#define SOURCE_MODULE "quickselect.c"
#define MODULE_VERSION "1.72"
#define MODULE_DATE "2017-03-07T05:57:07Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
/* Although the implementation is different, several concepts are adapted from:
   qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "quickselect.h"        /* quickselect QSORT_FUNCTION_NAME */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* */
#include <limits.h>             /* *_MAX */
#include <stddef.h>             /* size_t NULL */
#if ASSERT_CODE
# include <stdio.h>             /* for fprintf when debugging */
#endif
#include <string.h>             /* strrchr */

/* macros */

/* space-saving */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* stringification (don't change!) */
#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

/* static data (for version tracking for debugging/error reports) */
static char quickselect_initialized=(char)0;
static const char *filenamebuf= __FILE__ ;
static const char *source_file= NULL;
const char *quickselect_build_options="@(#)quickselect.c: "
    "built with configuration options: "
    "ASSERT_CODE=" xbuildstr(ASSERT_CODE)
#ifdef BUILD_NOTES
    ", BUILD_NOTES=" BUILD_NOTES
#endif
#ifdef INSERTION_CUTOFF
    ", INSERTION_CUTOFF=" xbuildstr(INSERTION_CUTOFF)
#endif
#if LOPSIDED_LIMIT
    ", LOPSIDED_LIMIT=" xbuildstr(LOPSIDED_LIMIT)
#endif
#if REPIVOT_FACTOR
    ", REPIVOT_FACTOR=" xbuildstr(REPIVOT_FACTOR)
#endif
#if REPIVOT_CUTOFF
    ", REPIVOT_CUTOFF=" xbuildstr(REPIVOT_CUTOFF)
#endif
#if SAVE_PARTIAL
    ", SAVE_PARTIAL=" xbuildstr(SAVE_PARTIAL)
#endif
    ;

#ifndef SIZE_T_MAX /* not defined on Solaris */
# define SIZE_T_MAX ULONG_MAX
#endif

/* data structures */

/* To avoid repeatedly calculating the number of samples required for pivot
   element selection vs. nmemb, which is expensive, a table is used; then
   determining the number of samples simply requires a search of the (small)
   table.  As the number of samples in each table entry is a power of 3, the
   table may also be used to avoid multiplication and/or division by 3.
   McGeoch & Tygar, in "Optimal Sampling Strategies for Quicksort" also
   suggest use of a table, and determine the optimal sample size as proportional
   to the square root of the array size. The table min_nmemb cutoff values
   mostly correspond to that relationship (cutoff is the square of the number of
   samples) except for a few entries where a different optimum has been found.
*/
struct sampling_table_struct {
    size_t min_nmemb;
    size_t samples;
};
#if (SIZE_T_MAX) > 4294967295
# define SAMPLING_TABLE_SIZE 22
#else
# define SAMPLING_TABLE_SIZE 12
#endif
static struct sampling_table_struct sampling_table[SAMPLING_TABLE_SIZE] = {
   {                    1UL,           1UL }, /* single sample, 1/4 position */
   {                   13UL,           3UL }, /* median-of-3, 1/4,1/2,3/4 */
   {                   81UL,           9UL }, /* remedian of samples */
   {                  729UL,          27UL },
   {                 6486UL,          81UL },
   {                55388UL,         243UL },
   {               531441UL,         729UL },
   {              4185683UL,        2187UL },
   {             43046721UL,        6561UL },
   {            387420489UL,       19683UL },
   {           3486784401UL,       59049UL },
#if (SIZE_T_MAX) > 4294967295
   {          31381059609UL,      177147UL }, /* silly to include large sizes?
                                              check again in a decade or two */
   {         282429536481UL,      531441UL },
   {        2541865828329UL,     1594323UL },
   {       22876792454961UL,     4782969UL },
   {      205891132094649UL,    14348907UL },
   {     1853020188851841UL,    43046721UL },
   {    16677181699666569UL,   129140163UL },
   {   150094635296999121UL,   387420489UL },
   {  1350851717672992089UL,  1162261467UL },
   { 12157665459056928801UL,  3486784401UL },
   {   (SIZE_T_MAX),         10460353203UL }
#else
   {   (SIZE_T_MAX),              177147UL }
#endif
};

/* Element swapping code depends on the size and alignment of elements. */
/* Assume basic types sizeof(foo) etc. are powers of 2. */
#define NTYPES 6 /* double, pointer, long, int, short, char */
/* Logarithms of size indexed by sizeof: table in lieu of calculation for speed,
   valid up to sizeof(foo) = 32.
*/
static const
int log2s[]={0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5};
/* Type sizes. It is assumed that sizeof(double)>=sizeof(char *)... */
static const size_t typsz[NTYPES] = {
    sizeof(double),sizeof(char *),sizeof(long),sizeof(int),sizeof(short),1UL
};
/* Mask off low-order bits, indexed by log of type size.  */
static const unsigned long bitmask[NTYPES] = {
    0x0UL, 0x01UL, 0x03UL, 0x07UL, 0x0fUL, 0x01fUL
};

/* alignment; no fiddly bits */
#define is_aligned(var,shift) (0U==(((unsigned long)(var))&bitmask[(shift)]))

/* static functions */

/* initialization of static strings at run-time */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void initialize_quickselect(V)
{
    const char *s;

    /* Initialize strings */
    s=strrchr(filenamebuf,'/');
    if (NULL==s) s=filenamebuf; else s++;
    quickselect_initialized=register_build_strings(quickselect_build_options,
        &source_file,s);
}

/* Array element swaps: */
/* Called many times from quickselect_internal, medians3. */
/* count is in chars */
/* versions to swap by char, short, int, double */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void charswap(char *pa, char *pb, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        char t;
        do {
            t=*pa, *pa=*pb, *pb=t;
            if (0UL==--count) break;
            pa++, pb++;
        } while (1);
    }
}

# define GENERIC_SWAP(type)                 \
    type *px=(type *)pa, *py=(type *)pb, t; \
    count /= sizeof(type);                  \
    do {                                    \
        t=*px, *px=*py, *py=t;              \
        if (0UL==--count) break;            \
        px++, py++;                         \
    } while (1)

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shortswap(char *pa, char *pb, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        GENERIC_SWAP(short);
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void intswap(char *pa, char *pb, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        GENERIC_SWAP(int);
    }
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void doubleswap(char *pa, char *pb, size_t count)
{
    if ((pa!=pb)&&(0UL<count)) { /* else nothing to do */
        GENERIC_SWAP(double);
    }
}

/* medians of sets of 3 elements */
/* Given base, size, and compar as for qsort, plus a row offset and sample
   offset, both in chars, find medians of sets of 3 elements where the three
   elements for each median are taken from three rows starting with base, and
   each subsequent set of three elements is offset from the previous set by
   sample_offset. As each median is determined, it is placed in the position of
   the middle element of the set. As a result, the set medians comprise the
   middle row, to which a pointer is returned.
   N.B. base is not necessarily the first element in the array passed to qsort.
*/
/* called from select_pivot() and repivot() */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
char *medians3(char *base, size_t size,
    int(*compar)(const void *, const void *), size_t row_offset,
    size_t sample_offset, size_t row_samples,
    void(*swapfunc)(char *,char *,size_t))
{
    int c;
    char *pa, *pb;
    size_t n, o;

    for (n=o=0UL; n<row_samples; n++,o+=sample_offset) {
        pa=base+o;
        pb=pa+row_offset; /* middle element */
        /* Optimized ternary median-of-3: 1-3 comparisons.
           Minimum number of comparisons (because in the qsort model (external
           function for comparison), comparisons are expensive).  If any two
           elements (of the 3) compare equal, either can be chosen for the
           median (the value is the median of the 3, regardless of the value of
           the third element).  Likewise, the median may be determined by the
           first two comparisons, e.g. a < b and b < c.  In the worst case, 3
           comparisons are required. The median element is swapped to the middle
           position.
        */
        c=compar(pa,pb);
        if (0!=c) {
            char *pc=pb+row_offset;
            int d=compar(pb,pc);
            if (0!=d) {
                if ((0<d)&&(0>c)) {
                    if (0>compar(pa,pc)) swapfunc(pb,pc,size);
                    else swapfunc(pa,pb,size);
                } else if ((0>d)&&(0<c)) {
                    if (0<compar(pa,pc)) swapfunc(pb,pc,size);
                    else swapfunc(pa,pb,size);
                }
            }
        }
    }
    return base+row_offset; /* middle row */
}

/* quickselect definition (internal interface) */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_loop(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *),
    const size_t *pk, size_t firstk, size_t beyondk,
    void(*swapfunc)(char *,char *,size_t)
#if SAVE_PARTIAL
    , char **ppeq, char **ppgt
#endif
   )
{
    size_t nmemb=beyond-first;
    int c, nlopsided=0;
    char *pd, *pe, *pivot, *pl, *pu;

    while ((NULL!=pk)||(nmemb>(INSERTION_CUTOFF))) {
        size_t lk, n, p, q, r, rk, s, t;
        char *pa, *pc, *pf, *pg;

        A((NULL==pk)||(firstk<beyondk));
        A(first<beyond);
        /* sample array elements */
        for (q=0UL; q<SAMPLING_TABLE_SIZE; q++)
            if (nmemb<sampling_table[q].min_nmemb)
                break; /* stop looking */
        if (0UL<q) q--; /* sampling table index */
        n=sampling_table[q].samples; /* total samples (a power of 3) */

        /* select pivot from samples using a fast method */
        switch (n) {
            case 1UL :
                /* best for organ-pipe, OK others */
                pivot=base+(first+(nmemb>>2))*size;
                s=r=0UL;
            break;
            case 3UL :
                s=r=(nmemb>>2)*size;                        /* 1/4 spacing */
                pivot=base+size*(first+(nmemb>>1))-r;      /* first sample */
            break;
            default : /* > 3, power of 3 */
                t = n/3UL;                              /* samples per row */
                if (0UL<t) s=(p=nmemb>>2)/t; else s=0UL; /* sample spacing */
                if (1UL>=t) r=((nmemb+2UL)>>2); else r=p+s; /* row spacing */
                s*=size, r*=size;                      /* spacing in chars */
                pivot=base+size*(first+(nmemb>>1))       /* middle element */
                    -r                              /* -> middle first row */
                    -(t>>1)*s;                       /* - half row samples */
            break;
        }
        if (1UL<n) { /* median or remedian of (at least 3) samples */
            n=sampling_table[--q].samples;
            pivot=medians3(pivot,size,compar,r,s,n,swapfunc);
            while (0UL<q) {
                n=sampling_table[--q].samples;
                pivot=medians3(pivot,size,compar,n*s,s,n,swapfunc);
            }
        }

        /* partition the array around the pivot element */
#if ! SAVE_PARTIAL
partition_array: ;
#endif
        pl=base+size*first;
        pa=pc=pl;
        pg=(pf=pu=base+beyond*size)-size;
        while (1) { /* loop - partitioning */
            while ((pc<=pg)&&(0<=(c=compar(pivot,pc)))) { /* scan -> */
                if (c==0) { swapfunc(pivot=pa,pc,size); pa+=size; }
                pc+=size;
            }
            while ((pc<=pg)&&(0>=(c=compar(pivot,pg)))) { /* scan <- */
                if (c==0) { pf-=size; swapfunc(pg,pivot=pf,size); }
                pg-=size;
            }
            if (pc>pg) break;
            swapfunc(pc,pg,size); /* <, > */
            pc+=size; pg-=size;
        }
        pg+=size; /* now start of > region */
        /* canonicalize */
        if (pf<pu) { /* pg unchanged if pf>=pu */
            if (pg<pf) { /* pg= original pu if pf>=pg */
                p=pu-pf, n=pf-pg; if (p<n) n=p;
                swapfunc(pg,pu-n,n); pg+=p;
            } else pg=pu;
        }
        if (pa<pc) { /* pa= original pl if pa>=pc */
            if (pl<pa) { /* pa= original pc if pa>=pl */
                p=pc-pa, n=pa-pl; if (p<n) n=p;
                swapfunc(pl,pc-n,n); pa=pl+p;
            } else pa=pc;
        } else pa=pl;
        /* |       <            |         =           |      >      | */
        /*  pl                   a                     g             u*/

check_sizes: ; /* compare partitioned region (effective) sizes */
#if SAVE_PARTIAL
        /* update = region pointers (only applicable for median-of-medians) */
        if ((0UL==firstk)&&(1UL==beyondk)&&(NULL!=ppeq)&&(NULL!=ppgt))
            *ppeq=pa, *ppgt=pg;
#endif
        /* ranks of first element and element beyond = region */
        p=(pa-base)/size, q=(pg-base)/size;
        /* sizes of < and > regions (elements) */
        s=p-first, t=beyond-q;
        /* revised range of order statistic ranks */
        /* < region indices [first,p), order statistics [firstk,lk) */
        /* > region indices [q,beyond), order statistics [rk,beyondk) */
        if (NULL==pk) { /* sorting */
            lk=beyondk, rk=firstk;
        } else {
            size_t ll, lr, rl, rr;

            /* binary search through pk to find limits for each region */
            ll=rl=firstk, lr=rr=beyondk;
            lk=rk=ll+((lr+1UL-ll)>>1);
            while (ll<lr) {
                if (pk[lk]>=p) { if (0UL<lk) lr=lk-1UL; else lr=lk; }
                else ll=lk;
                lk=ll+((lr+1UL-ll)>>1);
                if (lk==beyondk) break;
                A(lk>=firstk);
            }
            if ((lk<beyondk)&&(pk[lk]<p)) lk++;
            A(lk>=firstk);
            while (rl<rr) {
                if (pk[rk]<q) { if (rk<beyondk) rl=rk+1UL; else rl=rk; }
                else rr=rk;
                rk=rl+((rr-rl)>>1);
                if (rk==beyondk) break;
                A(rk<=beyondk);
            }
        }
        A(lk>=firstk);A(rk<=beyondk);A(lk<=rk);

        /* process smaller region (unless no processing required) first */
        if (s<t) { /* > region is larger */
            /* recurse on small region, if effective size > 1 */
            if ((1UL<s)&&((NULL==pk)||(lk>firstk)))
                quickselect_loop(base,first,p,size,compar,pk,firstk,lk,swapfunc
#if SAVE_PARTIAL
                ,ppeq,ppgt
#endif
            );
            /* large region [q,beyond) size t, order statistics [rk,beyondk) */
            first=q, r=t, firstk=rk;
        } else { /* < region is larger, or regions are same size */
            /* recurse on small region, if effective size > 1 */
            if ((1UL<t)&&((NULL==pk)||(beyondk>rk)))
               quickselect_loop(base,q,beyond,size,compar,pk,rk,beyondk,swapfunc
#if SAVE_PARTIAL
                ,ppeq,ppgt
#endif
            );
            /* large region [first,p) size s, order statistics [firstk,lk) */
            beyond=p, r=s, beyondk=lk;
        }
        /* iterate on large region (size r), if effective size > 1 */
        if ((2UL>r)||((NULL!=pk)&&(firstk>=beyondk))) return;
        /* should large region be repivoted? */
        /* Determine whether or not to repivot/repartition region of size r
           elements (large region) resulting from a partition of nmemb elements.
           Assumed that continued processing of the region w/o repivoting will
           yield a similar split.  Repivot if the cost of repivoting plus
           processing the resulting regions is expected to be less than the cost
           of processing the region w/o repivoting.  Tuning parameters used here
           are defined near the top of this file.
        */
        if ( ((REPIVOT_CUTOFF)<=nmemb)
          && (
#if REPIVOT_FACTOR == 8UL
              (r>>3)
#elif REPIVOT_FACTOR == 16UL
              (r>>4)
#else
              (r/(REPIVOT_FACTOR))
#endif
                              >=nmemb-r-1UL)
        ) /* repivot large region */
            nlopsided++; /* (tentatively) repivot */
        if ((LOPSIDED_LIMIT)<=nlopsided) {
#if SAVE_PARTIAL
            char *pb, *ph;
#endif
            /* yes: do it and check resulting sizes */
            nlopsided=0; /* clean slate */

            /* Find pivot by median-of-medians, sets of 3 elements. */
            p=first+(nmemb>>1); /* middle */
            pl=base+first*size;
            /* Finding a pivot with guaranteed intermediate rank. Ideally,
               median (50%).  Blum, Floyd, Pratt, Rivest, Tarjan
               median-of-medians using sets of 5 elements with recursion
               guarantees rank in (asymptotically) 30-70% range, often
               better; can guarantee linear median-finding, N log(N)
               sorting. Simplification ignores "leftover" elements with a
               slight increase in rank range.  Non-recursive method (using
               separate linear median finding) can use sets of 3 elements to
               provide a tighter 33.33% to 66.67% range (again, slightly
               wider if "leftover" elements are ignored) at lower
               computational cost.
            */
            n=nmemb, pb=pl;
            /* Medians of sets of 3 elements. */
            n/=3UL;    /* number of complete sets */
            if (1UL<=n)
                pb=medians3(pl,size,compar,n*size,size,n,swapfunc);
            pc=pb;
            if (1UL<n) { /* median of medians */
                size_t karray[1];
                karray[0]=p; /* upper-median for even size arrays */
                q=(pb-base)/size; /* first */
                s=first+n; /* beyond */
                pf=base+s*size;
                quickselect_loop(base,q,s,size,compar,karray,0UL,1UL,swapfunc
#if SAVE_PARTIAL
                    ,&pd,&pe
#endif
                );
#if SAVE_PARTIAL
                /* Middle third of array (medians) is partitioned. */
                pivot=pd;
#else
                pe=pd=pivot=base+karray[0]*size; /* median of medians */
#endif
            } else {
                pf=pb+n*size;
            }

#if SAVE_PARTIAL
            /* Partition, avoiding recomparison of already partitioned (by
               median-of-medians) elements.  Similar to, but a bit more
               involved than regular partitioning.
            */
            pa=pb=pl;
            ph=pu=base+beyond*size;
            pg=ph-size;
            /* |   ?   : < :=p=: > :   ?   | */
            /*          c   d   e   f       u*/
            /* reinitialize for repartitioning */
            /* |   ?   : < :=p=: > :   ?   | */
            /*  pl      c   d   e   f       u*/
            /*  a=b                       g h*/
            /* McGeoch & Tygar suggest that partial partition information
               from median-of-medians might be used to avoid recomparisons
               during repartitioning.
            */
            /* Skip over some already-partitioned sections. */
            /* Separate partitioning loop here to avoid overhead when not
               preserving partial partition. Subtleties involved: empty
               regions (pc-pd, pe-pf, pg-ph, pl-pa, pa-pb), whether pb or pg
               reaches the partially partitioned region first.
            */
            while (1) { /* partitioning loop */
                /* +-----------------------------------------------------+ */
                /* |   =   |   <   |  ?  : < :=: > :  ?  |   >   |   =   | */
                /* +-----------------------------------------------------+ */
                /*  pl      a       b     c   d e   f   G g       h       u*/
                A(pl<=pa);A(pa<=pb);A(pg<=ph);A(ph<=pu);
                A(pc<=pd);A(pd<pe);A(pe<=pf);
                while (pb<=pg) { /* scan -> */
                    /* 1. Like regular partitioning except when within
                          partial partition from selection.
                    */
                    while (((pb<pc)||(pf<=pb))&&(0<=(c=compar(pivot,pb)))) {
                        if (0==c) { swapfunc(pivot=pa,pb,size); pa+=size; }
                        pb+=size;
                    }
                    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
                    /* 2. Within partial partition; no recomparisons. */
                    if ((pc<=pb)&&(pb<pf)) { /* partitioned */
                        /* First skip over < elements, possibly past pg. */
                        if (pd<=pg) { /* If pg didn't, move = block. */
                            if (pb<pe) { /* move = elements as a block */
                                if (pa<pd) { /* pa=pe if pd<=pa */
                                    p=pe-pd, n=pd-pa; if (p<n) n=p;
                                    swapfunc(pa,pe-n,n); pa+=p;
                                } else pa=pe;
                                pivot=pa-size;
                                pb=pe;
                            }
                        } else pb=pg+size; /* < elements */
                        if ((pb>=pe)&&(pb<pf)) break; /* at > element */
                    } else break; /* not partitioned region; break loop */
                }
                A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
                while (pb<=pg) { /* scan <- */
                    /* 3. Like regular partitioning except when within partial
                          partition from selection.
                    */
                    while (((pf<=pg)||(pg<pc))&&(0>=(c=compar(pivot,pg)))) {
                        if (0==c) { ph-=size; swapfunc(pg,pivot=ph,size); }
                        pg-=size;
                    }
                    A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
                    /* 4. Within partial partition; no recomparisons. */
                    if ((pc<=pg)&&(pg<pf)) { /* partitioned */
                        /* First skip over > elements, possibly past pb. */
                        if (pb<pe) { /* If pb didn't, move = block. */
                            if (pd<=pg) { /* move = elements as a block */
                                if (pe<ph) { /* ph= original pd if pe>=ph */
                                    p=ph-pe, n=pe-pd; if (p<n) n=p;
                                    swapfunc(pd,ph-n,n); ph-=n;
                                } else ph=pd;
                                pivot=ph;
                                pg=pd-size;
                            }
                        } else pg=pb-size; /* > elements */
                        if ((pg<pd)&&(pc<=pg)) break; /* at < element */
                    } else break; /* not partitioned region; break loop */
                }
                A(pl<=pa);A(pa<=pb);A(pc<=pd);A(pd<pe);A(pe<=pf);A(pg<=ph);A(ph<=pu);
                if (pb>pg) break;
                A(pb!=pg); /* no self-swapping */
                swapfunc(pb,pg,size);
                pb+=size, pg-=size;
            }
            pg+=size; /* now start of > region */
            /* canonicalize */
            /* |    =  |      <             |             >    |   =    | */
            /*  pl      a                    g                  h        u*/
            /* swap > and upper = regions, set pg to start of > */
            pb=pg; /* = region will start here */
            A(pl<=pa); A(pb==pg); A(pg<=ph); A(ph<=pu);
            if (ph<pu) { /* pg unchanged if ph>=pu */
                if (pg<ph) { /* pg= pu if ph<=pg */
                    p=pu-ph, n=ph-pg; if (p<n) n=p;
                    swapfunc(pg,pu-n,n); pg+=p;
                } else pg=pu;
            }
            /* |    =  |      <           |        =      |      >      | */
            /*  pl      a                  b               g             u*/
            /* swap left = and < regions */
            A(pl<=pa); A(pa<=pb); A(pg<=pu);
            if (pa<pb) { /* pa= original pl if pa>=pb */
                if (pl<pa) { /* pa= original pb if pa<=pl */
                    p=pb-pa, n=pa-pl; if (p<n) n=p;
                    swapfunc(pl,pb-n,n); pa=pl+p;
                } else pa=pb;
            } else pa=pl;
            A(pl<=pa); A(pa<=pg); A(pg<=pu);
            /* |       <            |         =           |      >      | */
            /*  pl                   a                     g             u*/
            goto check_sizes;
#else
            /* regular partition, repeating some comparisons */
            goto partition_array;
#endif
        }
        /* no repivot: just process large region as usual */
        nmemb=r;
    }
    if (2UL>nmemb) return;
    /* insertion sort */
    for (pd=pl=base+first*size,pu=base+(beyond-1UL)*size; pd<pu; pd+=size)
        for (pe=pd; (pe>=pl)&&(0<compar(pe,pivot=pe+size)); pe-=size)
            swapfunc(pe,pivot,size);
}

/* comparison function for sorting order statistic ranks */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int size_t_cmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const size_t a = *((const size_t *)p1), b = *((const size_t *)p2);

        if (a > b) return 1;
        if (a < b) return -1;
    }
    return 0;
}

/* public quickselect definition */
void quickselect(void *base, size_t nmemb, const size_t size,
    int (*compar)(const void *,const void *), size_t *pk, size_t nk)
{
    int i, t;  /* general integer variables */
    size_t s;  /* general size_t variable */
    void (*swapfunc)(char *,char *,size_t);

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if ((NULL==base) || (0UL==size) || (NULL==compar)) {
        errno=EINVAL;
        return;
    }

    if (2UL>nmemb) return; /* Return early if there's nothing to do. */

    /* Initialization of strings is performed here (rather than in
       quickselect_internal) so that quickselect_internal can be made inline.
    */
    if ((char)0==quickselect_initialized) initialize_quickselect();

    /* Simplify tests for selection vs. sorting by ensuring a NULL pointer when
       sorting. Ensure consistency between pk and nk.
    */
    if (0UL==nk) pk=NULL;
    else if (NULL==pk) nk=0UL;
    else if (1UL<nk) /* binary search requires sorted pk */
        quickselect((void *)pk,nk,sizeof(size_t),size_t_cmp,NULL,0UL);

    /* Determine size of data chunks to copy for element swapping.  Size is
       determined by element size and alignment. typsz is an
       array of type sizes (double, pointer, long, int, short, char).
       Types char, short, int, double have sizes 1, 2, 4, 8 on most (all?)
       32-bit and 64-bit architectures.  Types long and pointer sizes vary
       by architecture.
    */
    s=typsz[t=0]; /* double */
    if ((size<s)||(!(is_aligned(size,i=log2s[s])))||(!(is_aligned(base,i)))) {
        s=typsz[t=3]; /* int */
        if ((size<s)||(!(is_aligned(size,i=log2s[s])))||(!(is_aligned(base,i)))) {
            s=typsz[t=4]; /* short */
            if ((size<s)||(!(is_aligned(size,i=log2s[s])))
            ||(!(is_aligned(base,i))))
                s=typsz[t=5]; /* char */
        }
    }
    switch (t) {
        case 0 :  swapfunc=doubleswap;
        break;
        case 3 :  swapfunc=intswap;
        break;
        case 4 :  swapfunc=shortswap;
        break;
        default : swapfunc=charswap;
        break;
    }

    quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,swapfunc
#if SAVE_PARTIAL
        ,NULL,NULL
#endif
    );
}

/* Using quickselect to replace qsort: */
void QSORT_FUNCTION_NAME (void *base, size_t nmemb, size_t size,
    int (*compar)(const void *,const void *))
{
    int i, t;  /* general integer variables */
    size_t s;  /* general size_t variable */
    void (*swapfunc)(char *,char *,size_t);

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if ((NULL==base) || (0UL==size) || (NULL==compar)) {
        errno=EINVAL;
        return;
    }

    if (2UL>nmemb) return; /* Return early if there's nothing to do. */

    /* Initialization of strings is performed here (rather than in
       quickselect_internal) so that quickselect_internal can be made inline.
    */
    if ((char)0==quickselect_initialized) initialize_quickselect();

    /* Determine size of data chunks to copy for element swapping.  Size is
       determined by element size and alignment. typsz is an
       array of type sizes (double, pointer, long, int, short, char).
       Types char, short, int, double have sizes 1, 2, 4, 8 on most (all?)
       32-bit and 64-bit architectures.  Types long and pointer sizes vary
       by architecture.
    */
    s=typsz[t=0]; /* double */
    if ((size<s)||(!(is_aligned(size,i=log2s[s])))||(!(is_aligned(base,i)))) {
        s=typsz[t=3]; /* int */
        if ((size<s)||(!(is_aligned(size,i=log2s[s])))||(!(is_aligned(base,i)))) {
            s=typsz[t=4]; /* short */
            if ((size<s)||(!(is_aligned(size,i=log2s[s])))
            ||(!(is_aligned(base,i))))
                s=typsz[t=5]; /* char */
        }
    }
    switch (t) {
        case 0 :  swapfunc=doubleswap;
        break;
        case 3 :  swapfunc=intswap;
        break;
        case 4 :  swapfunc=shortswap;
        break;
        default : swapfunc=charswap;
        break;
    }

    quickselect_loop(base,0UL,nmemb,size,compar,NULL,0UL,0UL,swapfunc
#if SAVE_PARTIAL
        ,NULL,NULL
#endif
    );
}
