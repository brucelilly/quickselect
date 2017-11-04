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
/* $Id: ~|^` @(#)   This is dedicated_sort_src.h version 1.4 dated 2017-11-03T20:19:36Z. \ $ */
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
#define ID_STRING_PREFIX "$Id: dedicated_sort_src.h ~|^` @(#)"
#define SOURCE_MODULE "dedicated_sort_src.h"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-11-03T20:19:36Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"


/* local header files needed */
#include "quickselect_config.h"
#include "quickselect.h"        /* set up errno_t, rsize_t, etc. */
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
#else
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 or later */

/* conformance */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# define SIZE_MAX ULONG_MAX
#endif

/* macros */
/* Comparison-exchange primitive macro used for sorting networks */
#define COMPARE_EXCHANGE(ma,mb,mc,msize,mswapf,malignsize,msize_ratio) \
    if(0<COMPAR(ma,mb,mc))                                             \
        EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,/**/)
    /* caller provides terminating semicolon */

#if 0 /* comments only in this block */
/* generalized adaptive-search insertion sort:
      preset mtype, mbase, mn, mfirst, ma, mb, msize, msize_ratio, mu, mpu, mcf
      modifies mn, ma, mb
   Adaptive search would use linear search for a sorted region of 3 or fewer
      elements and binary search over 4 or more elements.  Linear search in
      3 sorted elements requires 1-3 comparisons (average 2) whereas binary
      search requires 2 comparisons always, but with slightly higher overhead.
      At 4 elements, linear search requires 1-4 comparisons (average 2.5) and
      binary search requires 2-3 comparisons (average log2(5) ~ 2.322) which
      is a bit better.  But eliminating the test for the sorted region size and
      always using binary search runs faster; eliminating the test compensates
      for the overhead of binary search.
            } else if (mu-l<2UL) { /* linear search <= 3 sorted elements */    \
                for (pd=mb+msize; pd<=mpu; pd+=size)                           \
                    if (0>=mcf(pa,pd)) break;                                  \
            } else { /* binary search in region of 4 or more sorted elements */\
   N.B. This works well for small arrays, but not for large nearly-sorted
      arrays, where linear search is better due to the expectation that the
      insertion point is closer than half the size of the sorted region.
*/
#endif
#define ISORT_AS(mtype,mbase,mn,mfirst,ma,mb,mc,msize,msize_ratio,mu,mpu)      \
    while (mn>mfirst) {                                                        \
        --mn;                                                                  \
        if (0<COMPAR(ma,mb,mc)) {                                              \
            register mtype *px, *py, *pz, t;                                   \
            register size_t l=mn+2UL;                                          \
            register char *pc, *pd;                                            \
            if (l>mu) {                                                        \
                pd=mb+msize; /* a simple swap */                               \
            } else { /* binary search in region of sorted elements */          \
                register size_t h=mu;                                          \
                /* Binary search for insertion position: l->index beyond       \
                   elements to rotate.  Fewer comparisons than linear search,  \
                   slightly higher overhead.                                   \
                */                                                             \
                while (l<=h) {                                                 \
                    register size_t m=l+((h-l)>>1);                            \
                    /* >= important for stable sorting, rotation distance */   \
                    if (0>=COMPAR(ma,mbase+m*msize,mc)) h=m-1UL;               \
                    else l=m+1UL;                                              \
                }                                                              \
                A(mn!=l);                                                      \
                pd=mbase+l*msize;                                              \
            }                                                                  \
            /* Insertion by rotating elements [n,l) left by 1.  Same number of \
               swaps as ripple/bubble swapping if rotation by 1 position is    \
               implemented by reversals as for general rotation, but the       \
               specific case of 1 position can be implemented more efficiently \
               using the temporary variable once per iteration of the basic    \
               type size rather than swapping, which uses the temporary        \
               variable multiple times (unless the element size is exactly the \
               same size as the basic type and the rotation distance is 1 (i.e.\
               a simple swap)).                                                \
            */                                                                 \
            pc=ma, px=(mtype *)pc, pc+=msize;                                  \
            while ((char *)px<pc) {                                            \
                py=px,t=*py,pz=py+msize_ratio;                                 \
                while ((char *)pz<pd)                                          \
                    *py=*pz,py=pz,pz+=msize_ratio;                             \
                *py=t,px++;                                                    \
            }                                                                  \
        }                                                                      \
        mb=ma,ma-=msize;                                                       \
    }

/* macro for in-place merge using rotation:
      set mtype, mbase, mfirst, mbeyond, mb, msize, mo, mcf before call
      modifies ma, mb, mpu
   merges two adjacent sorted sub-arrays
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
#define MERGE(mtype,mbase,mfirst,mbeyond,ma,mb,msize,mpu,mc,mswapf,         \
    malignsize,msize_ratio)                                                 \
    {                                                                       \
        register mtype *px, *py, *pz, t;                                    \
        char *pc, *pd;                                                      \
        ma=mbase+mfirst*msize, pc=mb, mpu=mbase+mbeyond*msize;              \
        while (mb<mpu) {                                                    \
            A(pc==mb);                                                      \
            while (ma<pc) { /* first loop while pc==mb */                   \
                if (0<COMPAR(ma,pc,mc)) {                                   \
                    EXCHANGE_SWAP(swapf,ma,pc,msize,malignsize,msize_ratio, \
                        /**/);                                              \
                    pc+=msize; ma+=msize; break;                            \
                }                                                           \
                ma+=msize;                                                  \
            }                                                               \
            if (pc==ma) break; /* done */                                   \
            while (ma<mb) { /* second loop; pc>mb */                        \
                if ((pc<mpu)&&(0<COMPAR(mb,pc,mc))) {                       \
                    EXCHANGE_SWAP(swapf,ma,pc,msize,malignsize,msize_ratio, \
                        /**/);                                              \
                    pc+=msize;                                              \
                } else {                                                    \
                    EXCHANGE_SWAP(swapf,ma,mb,msize,malignsize,msize_ratio, \
                        /**/);                                              \
                    if (pc>mb+msize) { /* rotation */                       \
                        px=(mtype *)mb, pd=mb+msize;                        \
                        while ((char *)px<pd) {                             \
                            py=px,t=*py,pz=py+msize_ratio;                  \
                            while ((char *)pz<pc)                           \
                                *py=*pz,py=pz,pz+=msize_ratio;              \
                            *py=t,px++;                                     \
                        }                                                   \
                    }                                                       \
                }                                                           \
                ma+=msize;                                                  \
            }                                                               \
            A(ma==mb);                                                      \
            mb=pc; /* done or new merge */                                  \
        }                                                                   \
    }

#if ! QUICKSELECT_BUILD_FOR_SPEED
/* declaration */
#include "dedicated_sort_decl.h"
;
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

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
#include "dedicated_sort_decl.h"
{
#if __STDC_WANT_LIB_EXT1__
    errno_t ret=0;
#endif /* __STDC_WANT_LIB_EXT1__ */

#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file();
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
    if (beyond>first) {
        size_t n=beyond-first, na;
        char *pa, *pb;

        /* Sorting networks are only implemented for 2<=n<=12;
           n==1 is by definition sorted (don't attempt to split/merge) --
           options determine whether to use a network or merge or insertion
           sort.
        */
        if ((12UL<n)
        ||((2UL<n)
          &&((0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS)))
             ||(0U==(options&(0x01U<<n)))
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
                size_t na=(n>>1); /* split into 2 (nearly equal-size) pieces */
                /* in-place merge sort using rotation */
                char *pu;
                /* use recursion to sort pieces */
                if (1UL<na) {
                    PREFIX DEDICATED_SORT(base,first,first+na,size,COMPAR_ARGS,swapf,
                        alignsize, size_ratio, options); SUFFIX2
                }
                A(first+na+1UL<beyond);
                PREFIX DEDICATED_SORT(base,first+na,beyond,size,COMPAR_ARGS,swapf,
                    alignsize, size_ratio, options); SUFFIX2
                /* merge pieces */
                pb=base+(first+na)*size;
                switch (alignsize) {
                    case 8UL :
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        MERGE(int_least64_t,base,first,beyond,pa,pb,size,
                            pu,context,swapf,alignsize,size_ratio)
#else
# if DBL_MAX_10_EXP == 308
                        MERGE(double,base,first,beyond,pa,pb,size,pu,
                            context,swapf,alignsize,size_ratio)
# else
                        MERGE(char *,base,first,beyond,pa,pb,size,pu,
                            context,swapf,alignsize,size_ratio)
# endif
#endif /* C99 */
                    break;
                    case 4UL :
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        MERGE(int_least32_t,base,first,beyond,pa,pb,size,
                            pu,context,swapf,alignsize,size_ratio)
#else
# if INT_MAX == 2147483647
                        MERGE(int,base,first,beyond,pa,pb,size,pu,context,
                            swapf,alignsize,size_ratio)
# elif LONG_MAX == 2147483647
                        MERGE(long,base,first,beyond,pa,pb,size,pu,
                            context,swapf,alignsize,size_ratio)
# endif
#endif /* C99 */
                    break;
                    case 2UL :
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        MERGE(int_least16_t,base,first,beyond,pa,pb,size,
                            pu,context,swapf,alignsize,size_ratio)
#else
                        MERGE(short,base,first,beyond,pa,pb,size,pu,
                            context,swapf,alignsize,size_ratio)
#endif /* C99 */
                    break;
                    default :
                        MERGE(char,base,first,beyond,pa,pb,size,pu,
                            context,swapf,alignsize,size_ratio)
                    break;
                }
            } else {
                /* Insertion sort iterative loop. */
                /* Implementation by insertion of first out-of-place element
                   into sorted remainder (except for trivial case).
                */
                /* Find extent of rightmost sorted run of elements. */
                /* n is the index of the out-of-place element immediately to
                   the left of the rightmost sorted run of elements
                */
                register size_t u;
                char *pu;
                u=beyond-1UL, pu=pb=base+u*size, pa=pb-size;
                n=u; /* now n becomes the index of the last element */
                switch (alignsize) {
                    case 8UL : /* uint_least64_t */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        ISORT_AS(uint_least64_t,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
#else
# if ( LONG_MAX > 2147483647 ) && ( LONG_MAX == 9223372036854775807 )
                        ISORT_AS(long,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
# elif DBL_MAX_10_EXP == 308
                        ISORT_AS(double,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
# else
                        ISORT_AS(char *,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
# endif
#endif /* C99 */
                    break;
                    case 4UL : /* uint_least32_t */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        ISORT_AS(uint_least32_t,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
#else
# if INT_MAX == 2147483647
                        ISORT_AS(int,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
# elif LONG_MAX == 2147483647
                        ISORT_AS(long,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
# endif
#endif /* C99 */
                    break;
                    case 2UL : /* uint_least16_t */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        ISORT_AS(uint_least16_t,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
#else
                        ISORT_AS(short,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
#endif /* C99 */
                    break;
                    default : /* uint_least8_t or char */
                        ISORT_AS(char,
                            base,n,first,pa,pb,context,size,size_ratio,u,pu)
                    break;
                }
            }
            RETURN ;
        }
        switch (n) {
            case 1UL : /* shouldn't happen... */
                A(2UL<=n);
                PREFIX errno=EINVAL;
            RETURN ;
            case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
                pa=base+first*size;
                pb=pa+size;
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
            RETURN ;
            case 3UL : /* stable network or optimized decision tree sort of 3 */
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    /* stable, but always 3 comparisons, cannot be parallelized */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else
#if FAVOR_SORTED /* favor already-sorted, reverse-sorted input sequences */
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
                    c=COMPAR(pa,pb,context), d=COMPAR(pb,pc,context);
                    if (0>=c) { /*b>=a*/
                        if (0<d) { /*b>c*/
                            if (0==c) { /*a==b*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,/**/); /* 0,2 */
                            } else { /*b>a,b>c*/
                                if (0<COMPAR(pa,pc,context)) /*a>c*/
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
                            if (0<COMPAR(pa,pc,context)) /*a>c*/
                                EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,
                                    size_ratio,/**/); /* 0,2 */
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                /**/); /* 0,1 */
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
                    /* not stable */
                    char *pc;
                    int c, d;
                    pa=base+first*size; pb=pa+size; pc=pb+size;
                    c=COMPAR(pa,pc,context), d=COMPAR(pb,pc,context);
                    if (0>=c) { /*c>=a*/
                        if (0<d) { /*b>c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                /**/); /* 1,2 */
                        } else if (0>d) { /*b<c*/
                            if ((0==c)||(0<COMPAR(pa,pb,context)))/*a==c||a>b*/
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                                    size_ratio,/**/); /* 0,1 */
                        }
                    } else { /*a>c*/
                        if (0>d) /*b<c*/
                            EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                                /**/); /* 1,2 */
                        else if ((0<d)&&(0>COMPAR(pa,pb,context))) /*c<a<b*/
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                                /**/); /* 0,1 */
                        EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,
                            /**/); /* 0,2 */
                    }
                }
#endif /* FAVOR_SORTED */
            RETURN ;
            case 4UL : /* 6 or 5 comparisons with 1 or 2 parallelizable groups */
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    /* 6 comparisons, 5 groups (only one of which contains more than one comparison) */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else {
                    char *pc, *pd;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* parallel group 3 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                }
            RETURN ;
            case 5UL : /* 10 or 9 comparisons with 3 or 4 parallelizable groups */
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd, *pe;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size;
                    /* 10 comparisons, 3 opportunities for parallelism */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else {
                    char *pc, *pd, *pe;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    /* parallel group 4 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                }
            RETURN ;
            case 6UL : /* 15 or 12 comparisons with 5 or 6 parallelizable groups */
                if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                    char *pc, *pd, *pe, *pf;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size; pf=pe+size;
                    /* 15 comparisons (more than average quicksort), 5 opportunities for parallelism */
                    /* equivalent to unrolled insertion sort or bubble sort */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    /* end of group */
                    /* parallel group */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    /* end of group */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                } else {
                    char *pc, *pd, *pe, *pf;
                    pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                    pe=pd+size; pf=pe+size;
                    /* not stable (Batcher merge-exchange network) */
                    /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pd,pf,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    /* parallel group 4 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                }
            RETURN ;
            case 7UL : /* 16 comparisons in 6 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pg,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pd,pf,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pg,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                }
            RETURN ;
            case 8UL : /* 19 comparisons in 6 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg, *ph;
                pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                pe=pd+size; pf=pe+size; pg=pf+size; ph=pg+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pg,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    COMPARE_EXCHANGE(pd,ph,context,size,swapf,alignsize,size_ratio); /* 3,7 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(pf,ph,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pd,pf,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pg,ph,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pg,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                }
            RETURN ;
            case 9UL : /* 25 comparisons in 9 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj;
                pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
                pe=pd+size; pf=pe+size; pg=pf+size; ph=pg+size; pj=ph+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pg,ph,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    COMPARE_EXCHANGE(ph,pj,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pg,ph,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    COMPARE_EXCHANGE(pc,pf,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pa,pd,context,size,swapf,alignsize,size_ratio); /* 0,3 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pf,pj,context,size,swapf,alignsize,size_ratio); /* 5,8 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pd,pg,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                    COMPARE_EXCHANGE(pe,ph,context,size,swapf,alignsize,size_ratio); /* 4,7 */
                    COMPARE_EXCHANGE(pc,pf,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pa,pd,context,size,swapf,alignsize,size_ratio); /* 0,3 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pf,ph,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pc,pg,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                /* parallel group 7 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                /* parallel group 8 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                /* parallel group 9 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                }
            RETURN ;
            case 10UL : /* 29 comparisons in 9 parallel groups */ /* not stable */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj, *pk;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                ph=pg+size; pj=ph+size; pk=pj+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pf,context,size,swapf,alignsize,size_ratio); /* 0,5 */
                    COMPARE_EXCHANGE(pb,pg,context,size,swapf,alignsize,size_ratio); /* 1,6 */
                    COMPARE_EXCHANGE(pc,ph,context,size,swapf,alignsize,size_ratio); /* 2,7 */
                    COMPARE_EXCHANGE(pd,pj,context,size,swapf,alignsize,size_ratio); /* 3,8 */
                    COMPARE_EXCHANGE(pe,pk,context,size,swapf,alignsize,size_ratio); /* 4,9 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pd,context,size,swapf,alignsize,size_ratio); /* 0,3 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pf,pj,context,size,swapf,alignsize,size_ratio); /* 5,8 */
                    COMPARE_EXCHANGE(pg,pk,context,size,swapf,alignsize,size_ratio); /* 6,9 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pd,pg,context,size,swapf,alignsize,size_ratio); /* 3,6 */
                    COMPARE_EXCHANGE(ph,pk,context,size,swapf,alignsize,size_ratio); /* 7,9 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,ph,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pj,pk,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,pf,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pe,pg,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(ph,pj,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pc,pf,context,size,swapf,alignsize,size_ratio); /* 2,5 */
                    COMPARE_EXCHANGE(pe,ph,context,size,swapf,alignsize,size_ratio); /* 4,7 */
                    COMPARE_EXCHANGE(pg,pj,context,size,swapf,alignsize,size_ratio); /* 6,8 */
                /* parallel group 7 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pg,ph,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                /* parallel group 8 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                /* parallel group 9 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                }
            RETURN ;
            case 11UL : /* 35 comparisons in 9 groups (not optimal or stable) */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj, *pk, *pl;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                ph=pg+size; pj=ph+size; pk=pj+size; pl=pk+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    COMPARE_EXCHANGE(pg,ph,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    COMPARE_EXCHANGE(pj,pk,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(pf,ph,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pj,pl,context,size,swapf,alignsize,size_ratio); /* 8,10 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pd,ph,context,size,swapf,alignsize,size_ratio); /* 3,7 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    COMPARE_EXCHANGE(pk,pl,context,size,swapf,alignsize,size_ratio); /* 9,10 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pe,pj,context,size,swapf,alignsize,size_ratio); /* 4,8 */
                    COMPARE_EXCHANGE(pg,pl,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pc,pg,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    COMPARE_EXCHANGE(pd,pj,context,size,swapf,alignsize,size_ratio); /* 3,8 */
                    COMPARE_EXCHANGE(pf,pk,context,size,swapf,alignsize,size_ratio); /* 5,9 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pg,pl,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                    COMPARE_EXCHANGE(pj,pk,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                /* parallel group 7 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pf,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pg,pj,context,size,swapf,alignsize,size_ratio); /* 6,8 */
                    COMPARE_EXCHANGE(ph,pl,context,size,swapf,alignsize,size_ratio); /* 7,10 */
                /* parallel group 8 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    COMPARE_EXCHANGE(ph,pk,context,size,swapf,alignsize,size_ratio); /* 7,9 */
                /* parallel group 9 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(ph,pj,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                }
            RETURN ;
            case 12UL : /* 39 comparisons in 9 parallel groups (not optimal or stable) */
                { char *pc, *pd, *pe, *pf, *pg, *ph, *pj, *pk, *pl, *pm;
                pa=base+first*size; pb=pa+size;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size; pg=pf+size;
                ph=pg+size; pj=ph+size; pk=pj+size; pl=pk+size; pm=pl+size;
                /* parallel group 1 */
                    COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                    COMPARE_EXCHANGE(pg,ph,context,size,swapf,alignsize,size_ratio); /* 6,7 */
                    COMPARE_EXCHANGE(pj,pk,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                    COMPARE_EXCHANGE(pl,pm,context,size,swapf,alignsize,size_ratio); /* 10,11 */
                /* parallel group 2 */
                    COMPARE_EXCHANGE(pa,pc,context,size,swapf,alignsize,size_ratio); /* 0,2 */
                    COMPARE_EXCHANGE(pb,pd,context,size,swapf,alignsize,size_ratio); /* 1,3 */
                    COMPARE_EXCHANGE(pe,pg,context,size,swapf,alignsize,size_ratio); /* 4,6 */
                    COMPARE_EXCHANGE(pf,ph,context,size,swapf,alignsize,size_ratio); /* 5,7 */
                    COMPARE_EXCHANGE(pj,pl,context,size,swapf,alignsize,size_ratio); /* 8,10 */
                    COMPARE_EXCHANGE(pk,pm,context,size,swapf,alignsize,size_ratio); /* 9,11 */
                /* parallel group 3 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    COMPARE_EXCHANGE(ph,pm,context,size,swapf,alignsize,size_ratio); /* 7,11 */
                    COMPARE_EXCHANGE(pk,pl,context,size,swapf,alignsize,size_ratio); /* 9,10 */
                /* parallel group 4 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pd,ph,context,size,swapf,alignsize,size_ratio); /* 3,7 */
                    COMPARE_EXCHANGE(pe,pj,context,size,swapf,alignsize,size_ratio); /* 4,8 */
                    COMPARE_EXCHANGE(pg,pl,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                /* parallel group 5 */
                    COMPARE_EXCHANGE(pa,pe,context,size,swapf,alignsize,size_ratio); /* 0,4 */
                    COMPARE_EXCHANGE(pc,pg,context,size,swapf,alignsize,size_ratio); /* 2,6 */
                    COMPARE_EXCHANGE(pd,pj,context,size,swapf,alignsize,size_ratio); /* 3,8 */
                    COMPARE_EXCHANGE(pf,pk,context,size,swapf,alignsize,size_ratio); /* 5,9 */
                    COMPARE_EXCHANGE(ph,pm,context,size,swapf,alignsize,size_ratio); /* 7,11 */
                /* parallel group 6 */
                    COMPARE_EXCHANGE(pb,pf,context,size,swapf,alignsize,size_ratio); /* 1,5 */
                    COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                    COMPARE_EXCHANGE(pg,pl,context,size,swapf,alignsize,size_ratio); /* 6,10 */
                    COMPARE_EXCHANGE(pj,pk,context,size,swapf,alignsize,size_ratio); /* 8,9 */
                /* parallel group 7 */
                    COMPARE_EXCHANGE(pb,pe,context,size,swapf,alignsize,size_ratio); /* 1,4 */
                    COMPARE_EXCHANGE(pd,pf,context,size,swapf,alignsize,size_ratio); /* 3,5 */
                    COMPARE_EXCHANGE(pg,pj,context,size,swapf,alignsize,size_ratio); /* 6,8 */
                    COMPARE_EXCHANGE(ph,pl,context,size,swapf,alignsize,size_ratio); /* 7,10 */
                /* parallel group 8 */
                    COMPARE_EXCHANGE(pc,pe,context,size,swapf,alignsize,size_ratio); /* 2,4 */
                    COMPARE_EXCHANGE(pf,pg,context,size,swapf,alignsize,size_ratio); /* 5,6 */
                    COMPARE_EXCHANGE(ph,pk,context,size,swapf,alignsize,size_ratio); /* 7,9 */
                /* parallel group 9 */
                    COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                    COMPARE_EXCHANGE(ph,pj,context,size,swapf,alignsize,size_ratio); /* 7,8 */
                }
            RETURN ;
            default :
                A(0==1);
                PREFIX errno = EINVAL;
            RETURN ;
        }
    }
    RETURN ;
}
