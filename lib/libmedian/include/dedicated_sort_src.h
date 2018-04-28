#ifndef DEDICATED_SORT_SRC_H_INCLUDED
# define DEDICATED_SORT_SRC_H_INCLUDED 1
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
* $Id: ~|^` @(#)    dedicated_sort_src.h copyright 2017-2018 Bruce Lilly.   \ dedicated_sort_src.h $
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
/* $Id: ~|^` @(#)   This is dedicated_sort_src.h version 1.14 dated 2018-04-27T23:33:39Z. \ $ */
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
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
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
#define MODULE_VERSION "1.14"
#define MODULE_DATE "2018-04-27T23:33:39Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"


/* local header files needed */
#include "quickselect_config.h" /* QUICKSELECT_INLINE */
#include "exchange.h"           /* irotate protate EXCHANGE_SWAP */
#include "indirect.h"           /* inplace_merge pointer_mergesort
                                   rearrange_array
                                */
#if ! QUICKSELECT_BUILD_FOR_SPEED
#include "initialize_src.h"
#endif /* QUICKSELECT_BUILD_FOR_SPEED */

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* (maybe errno_t [N1570 K3.2]) */
#include <limits.h>             /* *_MAX */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201101L) /* C11 */
# include <stdalign.h>          /* alignas alignof etc. */
#endif /* C11 */
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */
#if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )
# include <stdint.h>            /* (header not standardized pre-C99) SIZE_MAX
                                   (maybe RSIZE_MAX (C11))
                                */
#endif /* C99 or later */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# include <limits.h>            /* ULONG_MAX */
# define SIZE_MAX ULONG_MAX
#endif /* SIZE_MAX */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t (maybe RSIZE_MAX [N1570 K3.4]) */
# if __STDC_VERSION__ >= 201001L
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
#  include <stdio.h>            /* (maybe errno_t rsize_t [N1570 K3.5]) */
#  include <stdlib.h>           /* (maybe errno_t rsize_t constraint_handler_t
                                   [N1570 K3.6]) */
# endif /* N1570 */
#else /* pre-C99 */
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 or later */
#if ASSERT_CODE + DEBUG_CODE
# include <stdio.h>
#endif

/* shared data */
#if __STDC_WANT_LIB_EXT1__
# ifndef QUICKSELECT_LOOP
#  define QUICKSELECT_LOOP quickselect_loop_s
# endif
#else
# ifndef QUICKSELECT_LOOP
#  define QUICKSELECT_LOOP quickselect_loop
# endif
#endif /* __STDC_WANT_LIB_EXT1__ */

#ifndef DEDICATED_SORT_SRC_FILE_HERE
extern char dedicated_sort_src_file[];
extern char dedicated_sort_src_file_initialized;
#else
/* not static; referenced by inline functions */
char dedicated_sort_src_file[PATH_MAX];
char dedicated_sort_src_file_initialized=0;
#endif

/* static data */
static const size_t pointer_and_a_half = sizeof(char *)+(sizeof(char *)>>1);

/* quickselect_loop declaration */
#if ! defined(QUICKSELECT_LOOP_DECLARED)
QUICKSELECT_EXTERN
# include "quickselect_loop_decl.h"
;
# define QUICKSELECT_LOOP_DECLARED 1
#endif /* QUICKSELECT_LOOP_DECLARED */

/* insertion sort is used for size_ratio 2, modest nmemb */
#include "insertion_sort_src.h" /* isort_bs */

/* static functions */
/* Indirect mergesort using a limited-size array of pointers/indices; the
   size limit arises because the function is called for limited size
   sub-arrays.
*/
/* Merge sort is stable, uses minimal comparisons (except for some input
   sequences, such as binary (ones and zeros) and constant-value).
*/
static QUICKSELECT_INLINE
int limited_indirect_mergesort(char *base, size_t first, size_t beyond,
    size_t nmemb, size_t size, COMPAR_DECL, size_t alignsize, size_t size_ratio,
    unsigned int table_index, size_t cache_sz, size_t pbeyond,
    unsigned int options)
{
    int r=0;
    if ((NULL==base)
    || (0UL==size)
    || (NULL==compar)
    ) {
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: ERROR: size=%lu, base=%p, compar=%s */\n",
                __func__,dedicated_sort_src_file,__LINE__,(unsigned long)size,
                (void *)base,comparator_name(compar));
        }
#endif
        r=errno=EINVAL;
    } else {
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, "
                "size=%lu, size_ratio=%lu, pbeyond=%lu, compar=%s, options=0x%x"
                " */\n",__func__,dedicated_sort_src_file,__LINE__,
                (unsigned long)first,(unsigned long)beyond,(unsigned long)nmemb,
                (unsigned long)size,(unsigned long)size_ratio,
                (unsigned long)pbeyond,comparator_name(compar),options);
        }
#endif
        if (1UL<nmemb) {
            if (NULL==pointerswap) pointerswap=swapn(sizeof(char *));
            A(NULL!=pointerswap);
            options&=~(QUICKSELECT_RESTRICT_RANK);
            if ((0UL!=pbeyond) /* initally zero, non-zero in callees */
#if 0 /* pbeyond != 0UL may be sufficient */
                /* size_ratio==1 and size==sizeof(char*) are logically necessary
                   conditions if pointers have indeed been allocated; but they
                   could also apply under other circumstances (i.e. they are not
                   conclusive).
                */
            &&(1UL==size_ratio)&&(size==sizeof(char *))
                /* indirection option is another non-conclusive condition */
            &&((QUICKSELECT_INDIRECT)==(options&(QUICKSELECT_INDIRECT)))
#endif
            ) { /* already have pointers for indirect mergesort */
                pointer_mergesort((char **)base,first,base,beyond-first,pbeyond,
                    COMPAR_ARGS,table_index,cache_sz,options);
            } else {
                size_t n, *indices, npointers=nmemb+((nmemb+1UL)>>1);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201101L) /* C11 */
                _Alignas((QUICKSELECT_DEFAULT_ALIGNMENT))
#endif /* C11 */
                char *pointers[npointers];
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MEMORY_DEBUG)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, "
                        "npointers=%lu, pointers@%p */\n",
                        __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
                        (unsigned long)beyond,(unsigned long)nmemb,
                        (unsigned long)npointers,(void *)pointers);
                    print_some_array(base,first,beyond-1UL,"/* base array: ",
                        " */",options);
                }
#endif
                /* initialize pointers */
                (V)set_array_pointers(pointers,npointers,base,size,first,beyond);
                /* mergesort using indirection; pointers moved, not data */
                pointer_mergesort(pointers,0UL,base,nmemb,nmemb,COMPAR_ARGS,
                    table_index,cache_sz,options|(QUICKSELECT_INDIRECT));
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MEMORY_DEBUG)) 
                    print_some_array(pointers,0UL,nmemb-1UL,"/* pointers: ",
                        " */",options|(QUICKSELECT_INDIRECT));
#endif
                /* rearrangement of data elements is more efficiently performed
                   using indices to the data elements, rather than pointers
                */
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,(size_t *)pointers,first,beyond);
                A(NULL!=indices);
#if defined(DEBUGGING)
                npiconversions+=nmemb;
#endif
                /* Rearrange the data elements according to sorted order using
                   the indices derived from the sorted pointers. No data element
                   is moved more than once.  However, there is poor locality of
                   access for random original data order.
                */
                n=rearrange_array(base,nmemb,size,indices,nmemb,first,beyond,
                    alignsize);
                if (n>npointers) r=errno;
#if defined(DEBUGGING)
                nmoves+=n;
                if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(MEMORY_DEBUG)) 
                    print_some_array(base,first,beyond-1UL,
                        "/* rearranged base array: "," */",options);
#endif
            }
        }
    }
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                "options=0x%x */\n",__func__,dedicated_sort_src_file,__LINE__,
                first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return r;
}

static QUICKSELECT_INLINE
int ded_sort3(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
{
    register int ret=0;
    register char *pa, *pb, *pc;

    A(first+3UL==beyond);
    pa=base+first*size, pb=pa+size, pc=pb+size;
    /* Unrolled merge sort and decision tree each use the same average number of
       comparisons for random input; unrolled merge sort favors already-sorted
       input (2 comparisons) at the expense of inverted organ-pipe inputs (3
       comparisons) whereas the decision tree favors inverted organ-pipe inputs
       (2 comparisons) at the expense of already-sorted inputs (3 comparisons).
       Both methods use 3 comparisons for reverse-sorted inputs and 2
       comparisons for (non-inverted) organ-pipe inputs.  The decision tree is
       slightly faster (but lacks partial order stability) for large element
       size (size_ratio).
       Divide-and-conquer is slower than decision tree and in-place merge sort;
       it is not used.
       Indirect merge sort is slowest; it is not used.
       Insertion sort is stable and slightly faster than unrolled merge sort for
       very high size_ratio (> ~ 200); at 3 elements it uses the same average
       number of comparisons as merge sort and the decision tree.
       If partial order stability is required, the stable unrolled merge sort is
       used except at very high size_ratio (where stable insertion sort is
       used); otherwise the slightly faster decision tree is used.
    */
    if (0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS|QUICKSELECT_STABLE))) {
        /* simplified merge for 3 elements */
        /* split */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                "options=0x%x */\n",__func__,
                dedicated_sort_src_file,__LINE__,3UL,options);
        }
#endif
        CX(pb,pc);
        if (0<OPT_COMPAR(pa,pb,options)) {
            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,SWAP_COUNT_STATEMENT);
            CX(pb,pc);
        }
#if defined(DEBUGGING)
        nmerges++; /* this was a merge */
#endif
    } else {
        /* decision tree (minimum comparisons (same as in-place merge sort)
           and slightly faster than stable divide-and-conquer for large
           size_ratio, but not stable)
        */
        /* Optimized comparison-based sort:
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
        /* Decision tree is always better than divide-and-conquer:
           lower overhead, same average comparisons, same average data movement.
        */
        register int c, d;
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: non-stable decision tree: first=%lu, beyond"
                "=%lu, size_ratio=%lu */\n",
                __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
                (unsigned long)beyond,(unsigned long)size_ratio);
        }
#endif
        c=OPT_COMPAR(pa,pc,options), d=OPT_COMPAR(pb,pc,options);
        if (0>=c) { /*c>=a*/
            if (0<d) { /*b>c*/
                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                    SWAP_COUNT_STATEMENT);
            } else if (0>d) { /*b<c*/
                if ((0==c)
                ||(0<OPT_COMPAR(pa,pb,options))
                ) { /*a==c||a>b*/
                    EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                        size_ratio,SWAP_COUNT_STATEMENT);
                }
            }
        } else { /*a>c*/
            if (0>d) { /*b<c*/
                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,
                    SWAP_COUNT_STATEMENT);
            } else if ((0<d)
            &&(0>OPT_COMPAR(pa,pb,options))
            ) { /*c<a<b*/
                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                    SWAP_COUNT_STATEMENT);
            }
            EXCHANGE_SWAP(swapf,pa,pc,size,alignsize,size_ratio,
                SWAP_COUNT_STATEMENT);
        }
    }
#if ASSERT_CODE > 1
    A(0>=OPT_COMPAR(pa,pb,options));
    A(0>=OPT_COMPAR(pb,pc,options));
#endif
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        size_t nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}

static QUICKSELECT_INLINE
int ded_sort4(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
{
    register int ret=0;

    A(first+4UL==beyond);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, size_ratio=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
            (unsigned long)beyond,(unsigned long)size_ratio);
    }
#endif
    if (0U==(options&(QUICKSELECT_OPTIMIZE_COMPARISONS|QUICKSELECT_STABLE))) {
        /* sorting network */
        /* sorting network: 5 comparisons in 3 parallel groups */
        /* Optimal sorting network is always better than divide-and-conquer:
           lower overhead, same average comparisons, same average data movement.
        */
        register char *pa, *pb, *pc, *pd;
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                "first=%lu, beyond=%lu, options=0x%x */\n",__func__,
                dedicated_sort_src_file,__LINE__,4UL,first,beyond,options);
        }
#endif
        pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size;
        /* parallel group 1 */
        CX(pa,pc); CX(pb,pd);
        /* parallel group 2 */
        CX(pa,pb); CX(pc,pd);
        /* parallel group 3 */
        CX(pb,pc);
#if ASSERT_CODE > 1
        A(0>=OPT_COMPAR(pa,pb,options));
        A(0>=OPT_COMPAR(pb,pc,options));
        A(0>=OPT_COMPAR(pc,pd,options));
#endif
    } else {
        /* unrolled in-place merge sort */
        /* simplified in-place merge sort for 4 elements */
        /* split */
        register char *pa, *pb, *pc, *pd;
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: unrolled in-place mergesort "
                "nmemb=%lu */\n",
                __func__,dedicated_sort_src_file,__LINE__,4UL);
        }
#endif
        pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size;
        /* These compare-exchanges can be performed in parallel. */
        CX(pa,pb); CX(pc,pd);
        /* merge pieces */
        inplace_merge(base,first,first+2UL,beyond,size,COMPAR_ARGS,
            swapf,alignsize,size_ratio,options);
    }
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        size_t nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}

static QUICKSELECT_INLINE
int ded_sort5(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
{
    register int ret=0;
    register char *pa, *pb;
    A(first+5UL==beyond);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, size_ratio=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
            (unsigned long)beyond,(unsigned long)size_ratio);
    }
#endif
    if (0U==(options&(QUICKSELECT_OPTIMIZE_COMPARISONS|QUICKSELECT_STABLE))) {
        /* network sort for small subarrays */
        /* See J. Gamble http://pages.ripco.net/~jgamble/nw.html */
        /* non-stable, not optimized for expensive comparisons */
        /* 9 comparisons with 4 parallelizable groups */
        /* Optimal sorting network is always better than divide-and-conquer:
           lower overhead, same average comparisons, less average data movement.
        */
        register char *pc, *pd, *pe;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                "options=0x%x */\n",__func__,
                dedicated_sort_src_file,__LINE__,5UL,options);
        }
# endif
        pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
        pe=pd+size;
        /* not stable (Batcher merge-exchange network) */
        /* parallel group 1 */
        CX(pa,pe);
        /* parallel group 2 */
        CX(pa,pc); CX(pb,pd);
        /* parallel group 3 */
        CX(pa,pb); CX(pc,pe);
        /* parallel group 4 */
        CX(pc,pd); CX(pb,pe);
        /* parallel group 5 */
        CX(pb,pc); CX(pd,pe);
    } else {
        /* Simplified merge for 5 elements; average 7.5 comparisons. */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: unrolled in-place mergesort "
                "nmemb=%lu */\n",
                __func__,dedicated_sort_src_file,__LINE__,5UL);
        }
#endif
        pa=base+first*size, pb=pa+size;
        /* split 2-3 elements */
        CX(pa,pb);
        ret= ded_sort3(base,first+2UL,beyond,size,COMPAR_ARGS,swapf,
            alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
        /* merge pieces */
        inplace_merge(base,first,first+2UL,beyond,size,COMPAR_ARGS,
            swapf,alignsize,size_ratio,options);
    }
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        size_t nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}


static QUICKSELECT_INLINE
int ded_sort6(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
{
    register int ret=0;
    A(first+6UL==beyond);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, size_ratio=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
            (unsigned long)beyond,(unsigned long)size_ratio);
    }
#endif
    if (0U==(options&(QUICKSELECT_OPTIMIZE_COMPARISONS|QUICKSELECT_STABLE))) {
#if QUICKSELECT_MAX_NETWORK < 6
        if (2UL<size_ratio) return EAGAIN;
#else
        /* network sort for small subarrays */
        /* See J. Gamble http://pages.ripco.net/~jgamble/nw.html */
        /* non-stable, not optimized for expensive comparisons */
        /* 12 comparisons with 6 parallelizable groups */
        register char *pa, *pb, *pc, *pd, *pe, *pf;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                "options=0x%x */\n",__func__,
                dedicated_sort_src_file,__LINE__,6UL,options);
        }
# endif
        pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
        pe=pd+size, pf=pe+size;
        /* parallel group 1 */
        CX(pa,pe); CX(pb,pf);
        /* parallel group 2 */
        CX(pa,pc); CX(pb,pd);
        /* parallel group 3 */
        CX(pa,pb); CX(pc,pe); CX(pd,pf);
        /* parallel group 4 */
        CX(pc,pd); CX(pe,pf);
        /* parallel group 5 */
        CX(pb,pe);
        /* parallel group 6 */
        CX(pb,pc); CX(pd,pe);
        goto check6;
#endif
    }
    /* Simplified merge for 6 elements; average 10.576 comparisons. */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: unrolled in-place mergesort "
            "nmemb=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,6UL);
    }
#endif
    /* split 3-3 elements */
    ret= ded_sort3(base,first,first+3UL,size,COMPAR_ARGS,swapf,
        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
    ret= ded_sort3(base,first+3UL,beyond,size,COMPAR_ARGS,swapf,
        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
    /* merge pieces */
    inplace_merge(base,first,first+3UL,beyond,size,COMPAR_ARGS,
        swapf,alignsize,size_ratio,options);
check6:
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        size_t nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}

static QUICKSELECT_INLINE
int ded_sort7(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
{
    register int ret=0;
    A(first+7UL==beyond);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, size_ratio=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
            (unsigned long)beyond,(unsigned long)size_ratio);
    }
#endif
    /* In-place merge sort w/ binary search if stability or minimum comparisons
       is required; otherwise sorting network is faster. Divide-and-conquer by
       partitioning is slower; it is not used.
    */
    if (0U==(options&(QUICKSELECT_OPTIMIZE_COMPARISONS|QUICKSELECT_STABLE))) {
#if QUICKSELECT_MAX_NETWORK < 7
        if (1UL<size_ratio) return EAGAIN;
#else
        /* network sort for small subarrays */
        /* See J. Gamble http://pages.ripco.net/~jgamble/nw.html */
        /* Can't return EAGAIN because ded_sort8 is called to sort pieces
           for merging.  Can't call quickselect_loop because that will simply
           return here, causing an infinite loop until the stack overflows.
           Divide-and-conquer by partitoning would only have a small run-time
           advantage, and only for very high (> 1720) size_ratio.
        */
        /* non-stable, not optimized for expensive comparisons */
        /* 16 comparisons with 6 parallelizable groups; average 6.196 swaps */
        register char *pa, *pb, *pc, *pd, *pe, *pf, *pg;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                "options=0x%x */\n",__func__,
                dedicated_sort_src_file,__LINE__,7UL,options);
        }
# endif
        pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
        pe=pd+size, pf=pe+size, pg=pf+size;
        /* not stable (! Batcher merge-exchange network) */
        /* parallel group 1 */
        CX(pa,pe); CX(pb,pf); CX(pc,pg);
        /* parallel group 2 */
        CX(pa,pc); CX(pb,pd); CX(pe,pg);
        /* parallel group 3 */
        CX(pa,pb); CX(pc,pe); CX(pd,pf);
        /* parallel group 4 */
        CX(pc,pd); CX(pe,pf);
        /* parallel group 5 */
        CX(pb,pe); CX(pd,pg);
        /* parallel group 6 */
        CX(pb,pc); CX(pd,pe); CX(pf,pg);
        goto check7;
#endif
    }
    /* Simplified merge for 7 elements; average 14.034 comparisons and 9.077 swap equivalents. */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: unrolled in-place mergesort "
            "nmemb=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,7UL);
    }
#endif
    /* split 3-4 elements */
    ret= ded_sort3(base,first,first+3UL,size,COMPAR_ARGS,swapf,
        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
    ret= ded_sort4(base,first+3UL,beyond,size,COMPAR_ARGS,swapf,
        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
    /* merge pieces */
    inplace_merge(base,first,first+3UL,beyond,size,COMPAR_ARGS,
        swapf,alignsize,size_ratio,options);
check7:
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        size_t nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}

static QUICKSELECT_INLINE
int ded_sort8(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
{
    register int ret=0;
    A(first+8UL==beyond);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, size_ratio=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
            (unsigned long)beyond,(unsigned long)size_ratio);
    }
#endif
    if (0U==(options&(QUICKSELECT_OPTIMIZE_COMPARISONS|QUICKSELECT_STABLE))) {
#if QUICKSELECT_MAX_NETWORK < 8
        if (1UL<size_ratio) return EAGAIN;
#else
        /* network sort for small subarrays */
        /* See J. Gamble http://pages.ripco.net/~jgamble/nw.html */
        /* Can't return EAGAIN because ded_sort8 is called to sort pieces
           for merging.  Can't call quickselect_loop because that will simply
           return here, causing an infinite loop until the stack overflows.
           Divide-and-conquer by partitoning would only have a small run-time
           advantage, and only for very high (> 8192) size_ratio.
        */
        /* non-stable, not optimized for expensive comparisons */
        /* 19 comparisons with 6 parallelizable groups; 8.295 average swaps */
        register char *pa, *pb, *pc, *pd, *pe, *pf, *pg, *ph;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                "options=0x%x */\n",__func__,
                dedicated_sort_src_file,__LINE__,8UL,options);
        }
# endif
        pa=base+first*size; pb=pa+size; pc=pb+size; pd=pc+size;
        pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size;
        /* parallel group 1 */
        CX(pa,pe); CX(pb,pf); CX(pc,pg); CX(pd,ph);
        /* parallel group 2 */
        CX(pa,pc); CX(pb,pd); CX(pe,pg); CX(pf,ph);
        /* parallel group 3 */
        CX(pa,pb); CX(pc,pe); CX(pd,pf); CX(pg,ph);
        /* parallel group 4 */
        CX(pc,pd); CX(pe,pf);
        /* parallel group 5 */
        CX(pb,pe); CX(pd,pg);
        /* parallel group 6 */
        CX(pb,pc); CX(pd,pe); CX(pf,pg);
        goto check8;
#endif
    }
    /* Simplified merge for 8 elements; average 17.28 comparisons and 12.081 swap equivalents. */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: unrolled in-place mergesort "
            "nmemb=%lu */\n",
            __func__,dedicated_sort_src_file,__LINE__,8UL);
    }
#endif
    /* split 4-4 elements */
    ret= ded_sort4(base,first,first+4UL,size,COMPAR_ARGS,swapf,
        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
    ret= ded_sort4(base,first+4UL,beyond,size,COMPAR_ARGS,swapf,
        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
    /* merge pieces */
    inplace_merge(base,first,first+4UL,beyond,size,COMPAR_ARGS,
        swapf,alignsize,size_ratio,options);
check8:
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        size_t nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}

/* In-place top-down merge sort.  The in-place merge uses rotations to
   maintain displaced elements in sorted order; the rotations can be
   expensive for large arrays.
*/
static QUICKSELECT_INLINE
int inplace_mergesort(char *base, size_t first, size_t beyond, size_t nmemb,
    size_t size, COMPAR_DECL, void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, unsigned int table_index,
    size_t cache_sz, size_t pbeyond, unsigned int options)
{
    register int ret=0;
    register size_t mid, na;
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, size_ratio=%lu, "
            "pbeyond=%lu, options=0x%x */\n",__func__,dedicated_sort_src_file,__LINE__,
            (unsigned long)nmemb,(unsigned long)size_ratio,
            (unsigned long)pbeyond,options);
    }
#endif
    /* split */
    na=(nmemb>>1); mid=first+na;
    /* No median-of-medians for sub-arrays (if divide-and-conquer is used) */
    options&=~(QUICKSELECT_RESTRICT_RANK);
    /* Sorting pieces could proceed in parallel.  */
    ret= QUICKSELECT_LOOP(base,first,mid,size,COMPAR_ARGS,NULL,0UL,0UL,
        swapf,alignsize,size_ratio,table_index,cache_sz,pbeyond,options,
        NULL,NULL);
    ret= QUICKSELECT_LOOP(base,mid,beyond,size,COMPAR_ARGS,NULL,0UL,0UL,
        swapf,alignsize,size_ratio,table_index,cache_sz,pbeyond,options,
        NULL,NULL);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        nmemb=test_array_sort(base,first,mid-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=mid-1UL) {
            fprintf(stderr,"/* %s: %s line %d: first=%lu, mid=%lu, options=0x%x */\n",
                __func__,dedicated_sort_src_file,__LINE__,first,mid,options);
            print_some_array(base,first,mid-1UL,"/* "," */",options);
            abort();
        }
        nmemb=test_array_sort(base,mid,beyond-1UL,size,compar,
            options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: %s line %d: mid=%lu, beyond=%lu, options=0x%x */\n",
                __func__,dedicated_sort_src_file,__LINE__,mid,beyond,options);
            print_some_array(base,mid,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    /* merge pieces */
    inplace_merge(base,first,mid,beyond,size,COMPAR_ARGS,swapf,alignsize,
        size_ratio,options);
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if (DEBUGGING(CORRECTNESS_DEBUG)) {
        nmemb=test_array_sort(base,first,beyond-1UL,size,
            compar,options,0U,NULL,NULL);
        if (nmemb!=beyond-1UL) {
            fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                __func__,dedicated_sort_src_file,__LINE__,first,beyond,options);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
            abort();
        }
    }
#endif
    return ret;
}

/* Dedicated (not divide-and-conquer) sorting.
   Return zero if sorting is completed.
   Return EAGAIN to direct caller to use divide-and-conquer.
   Return EINVAL for caller error (shouldn't happen for internal calls).
*/
/* definition */
#if QUICKSELECT_BUILD_FOR_SPEED
static
#endif
# include "dedicated_sort_decl.h"
{
    DEDICATED_SORT_RETURN_TYPE ret=0;
    size_t nmemb;

#if ! QUICKSELECT_BUILD_FOR_SPEED
    if ((char)0==file_initialized) initialize_file(__FILE__);
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if ((char)0==dedicated_sort_src_file_initialized) {
        (V)path_basename(__FILE__,dedicated_sort_src_file,PATH_MAX);
        dedicated_sort_src_file_initialized++;
    }
#endif
    A(256UL<=cache_sz); A(beyond>first);
    if (beyond<=first+1UL) { /* nmemb <= 1UL */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: ERROR: first=%lu >= beyond=%lu */\n",
                __func__,dedicated_sort_src_file,__LINE__,(unsigned long)first,
                (unsigned long)beyond);
        }
#endif
        A(beyond>first+1UL);
        return EINVAL; /* shouldn't happen */
    } else if (((nmemb=beyond-first)>5UL)
    && (((nmemb*pointer_and_a_half>cache_sz)) /* indirect mergesort */
    || (3UL*size>(cache_sz>>1))) /* others */
    ) {
    /* N.B. Never return EAGAIN for nmemb<8 (network sort 2-7 is faster than
       divide-and-conquer, even for huge element sizes, and in-place merge sort
       is faster than stable partition sort and uses fewer comparisons)
    */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: EAGAIN for divide-and-conquer nmemb=%lu, "
                " size=%lu, cache_sz=%lu */\n",
                __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb,
                (unsigned long)size,(unsigned long)cache_sz);
        }
#endif
        return EAGAIN; /* caller should use divide-and-conquer */
    } else { /* at least 2 elements */
        char *pa, *pb;

        switch (nmemb) {
#if ASSERT_CODE
            /* cases should be excluded by outer if test */
            case 0UL : /*FALLTHROUGH*/
            case 1UL : /* shouldn't happen... */
                A(2UL<=nmemb);
                ret= errno=EINVAL;
            break;
#endif /* ASSERT_CODE */
            case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
                pa=base+first*size;
                pb=pa+size;
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: compare-exchange nmemb=%lu, first="
                        "%lu */\n",__func__,dedicated_sort_src_file,__LINE__,
                        (unsigned long)nmemb,(unsigned long)first);
                }
#endif
                CX(pa,pb);
            break;
            case 3UL :
                ret = ded_sort3(base,first,beyond,size,COMPAR_ARGS,swapf,
                    alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
            break;
            case 4UL :
                ret = ded_sort4(base,first,beyond,size,COMPAR_ARGS,swapf,
                    alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
            break;
            case 5UL :
                ret = ded_sort5(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize,
                    size_ratio,table_index,cache_sz,pbeyond,options);
            break;
            case 6UL :
                ret = ded_sort6(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize,
                    size_ratio,table_index,cache_sz,pbeyond,options);
            break;
            case 7UL :
                ret = ded_sort7(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize,
                    size_ratio,table_index,cache_sz,pbeyond,options);
            break;
            case 8UL :
                ret = ded_sort8(base,first,beyond,size,COMPAR_ARGS,swapf,alignsize,
                    size_ratio,table_index,cache_sz,pbeyond,options);
            break;
            case 9UL :
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE)))
                ) {
#if QUICKSELECT_MAX_NETWORK > 8
                    /* 25 comparisons in 9 parallel groups; average 12.811 swaps. */
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size;
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size, pj=ph+size;
                    /* parallel group 1 */
                    CX(pa,pb); CX(pd,pe); CX(pg,ph);
                    /* parallel group 2 */
                    CX(pb,pc); CX(pe,pf); CX(ph,pj);
                    /* parallel group 3 */
                    CX(pa,pb); CX(pd,pe); CX(pg,ph); CX(pc,pf);
                    /* parallel group 4 */
                    CX(pa,pd); CX(pb,pe); CX(pf,pj);
                    /* parallel group 5 */
                    CX(pd,pg); CX(pe,ph); CX(pc,pf);
                    /* parallel group 6 */
                    CX(pa,pd); CX(pb,pe); CX(pf,ph); CX(pc,pg);
                    /* parallel group 7 */
                    CX(pb,pd); CX(pe,pg);
                    /* parallel group 8 */
                    CX(pc,pe); CX(pf,pg);
                    /* parallel group 9 */
                    CX(pc,pd);
#else
                    return EAGAIN;
#endif
                } else { /* simplified in-place merge sort for 9 elements */
                    /* split 4-5 using ded_sort4 and ded_sort5 */
                    /* average 20.326 comparisons and 14.58 swap equivalents */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    /* These sorts can be performed in parallel. */
                    (V)ded_sort4(base,first,first+4UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort5(base,first+4UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+4UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 10UL : /* 29 comparisons in 9 parallel groups */
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
#if QUICKSELECT_MAX_NETWORK > 9
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size;
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size, pj=ph+size;
                    pk=pj+size;
                    /* parallel group 1 */
                    CX(pa,pf); CX(pb,pg); CX(pc,ph); CX(pd,pj); CX(pe,pk);
                    /* parallel group 2 */
                    CX(pa,pd); CX(pb,pe); CX(pf,pj); CX(pg,pk);
                    /* parallel group 3 */
                    CX(pa,pc); CX(pd,pg); CX(ph,pk);
                    /* parallel group 4 */
                    CX(pa,pb); CX(pc,pe); CX(pf,ph); CX(pj,pk);
                    /* parallel group 5 */
                    CX(pb,pc); CX(pd,pf); CX(pe,pg); CX(ph,pj);
                    /* parallel group 6 */
                    CX(pb,pd); CX(pc,pf); CX(pe,ph); CX(pg,pj);
                    /* parallel group 7 */
                    CX(pc,pd); CX(pg,ph);
                    /* parallel group 8 */
                    CX(pd,pe); CX(pf,pg);
                    /* parallel group 9 */
                    CX(pe,pf);
#else
                    return EAGAIN;
#endif
                } else {
                    /* simplified in-place merge sort for 10 elements */
                    /* split 5-5 using ded_sort5 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort5(base,first,first+5UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort5(base,first+5UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+5UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 11UL :
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
#if QUICKSELECT_MAX_NETWORK > 10
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk, *pl;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size;
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size, pj=ph+size;
                    pk=pj+size, pl=pk+size;
                    /* parallel group 1 */
                    CX(pb,pl); CX(pc,pk); CX(pd,pj); CX(pe,ph); CX(pf,pg); 
                    /* parallel group 2 */
                    CX(pa,pg); CX(pb,pe); CX(pc,pd); CX(ph,pl); CX(pj,pk);
                    /* parallel group 3 */
                    CX(pa,pb); CX(pc,pf); CX(pd,pe); CX(ph,pj); CX(pk,pl);
                    /* parallel group 4 */
                    CX(pa,pc); CX(pb,pf); CX(pe,pg); CX(pj,pk);
                    /* parallel group 5 */
                    CX(pb,pj); CX(pc,pd); CX(pe,ph); CX(pf,pk); CX(pg,pl);
                    /* parallel group 6 */
                    CX(pb,pe); CX(pd,pf); CX(pg,pk); CX(ph,pj);
                    /* parallel group 7 */
                    CX(pb,pc); CX(pd,pe); CX(pf,ph); CX(pg,pj);
                    /* parallel group 8 */
                    CX(pc,pd); CX(pe,pf); CX(pg,ph);
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(CORRECTNESS_DEBUG)) {
                        nmemb=test_array_sort(base,first,beyond-1UL,size,
                            compar,options,0U,NULL,NULL);
                        if (nmemb!=beyond-1UL) {
                            fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                                __func__,__LINE__,first,beyond,options);
                            print_some_array(base,first,beyond-1UL,"/* "," */",options);
                            abort();
                        }
                    }
# endif
#else
                    return EAGAIN;
#endif
                } else {
                    /* simplified in-place merge sort for 11 elements */
                    /* split 5-6 using ded_sort5 and ded_sort6 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort5(base,first,first+5UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort6(base,first+5UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+5UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 12UL :
#if QUICKSELECT_MAX_NETWORK > 11
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk, *pl, *pm;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size,
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size,
                    pj=ph+size, pk=pj+size, pl=pk+size, pm=pl+size;
                    /* parallel group 1 */
                    CX(pa,pb);CX(pc,pd);CX(pe,pf);CX(pg,ph);CX(pj,pk);CX(pl,pm);
                    /* parallel group 2 */
                    CX(pa,pc);CX(pb,pd);CX(pe,pg);CX(pf,ph);CX(pj,pl);CX(pk,pm);
                    /* parallel group 3 */
                    CX(pa,pe); CX(pb,pc); CX(pf,pg); CX(ph,pm); CX(pk,pl);
                    /* parallel group 4 */
                    CX(pb,pf); CX(pd,ph); CX(pe,pj); CX(pg,pl);
                    /* parallel group 5 */
                    CX(pa,pe); CX(pc,pg); CX(pd,pj); CX(pf,pk); CX(ph,pm);
                    /* parallel group 6 */
                    CX(pb,pf); CX(pc,pd); CX(pg,pl); CX(pj,pk);
                    /* parallel group 7 */
                    CX(pb,pe); CX(pd,pf); CX(pg,pj); CX(ph,pl);
                    /* parallel group 8 */
                    CX(pc,pe); CX(pf,pg); CX(ph,pk);
                    /* parallel group 9 */
                    CX(pd,pe); CX(ph,pj);
                } else
#endif
                {
                    /* simplified in-place merge sort for 12 elements */
                    /* split 6-6 using ded_sort6 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort6(base,first,first+6UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort6(base,first+6UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+6UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 13UL :
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
#if QUICKSELECT_MAX_NETWORK > 12
                    /* 45 comparisons in 10 parallel groups (maybe not optimal) */
                    /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
                    /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk, *pl, *pm, *pn;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size,
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size, pj=ph+size,
                    pk=pj+size, pl=pk+size, pm=pl+size, pn=pm+size;
                    /* parallel group 1 */
                    CX(pb,pg); CX(pj,pk); CX(pc,pm); CX(pa,pf); CX(ph,pn);
                    CX(pe,pl);
                    /* parallel group 2 */
                    CX(pb,pc); CX(ph,pj); CX(pk,pn); CX(pa,pe); CX(pg,pm);
                    CX(pf,pl);
                    /* parallel group 3 */
                    CX(pf,pj); CX(pl,pn); CX(pa,ph); CX(pd,pm); CX(pe,pk);
                    /* parallel group 4 */
                    CX(pd,pl); CX(pm,pn); CX(pg,pj); CX(pc,pk);
                    /* parallel group 5 */
                    CX(pb,pd); CX(pe,pg); CX(pk,pm); CX(pc,pf); CX(pj,pl);
                    /* parallel group 6 */
                    CX(pa,pb); CX(pd,ph); CX(pj,pk); CX(pl,pm);
                    /* parallel group 7 */
                    CX(pf,pj); CX(pb,pc); CX(pd,pe); CX(pg,ph); CX(pk,pl);
                    /* parallel group 8 */
                    CX(pc,pe); CX(pf,pg); CX(ph,pj); CX(pb,pd);
                    /* parallel group 9 */
                    CX(pc,pd); CX(pe,pf); CX(pg,ph); CX(pj,pk);
                    /* parallel group 10 */
                    CX(pd,pe); CX(pf,pg);
#else
                    return EAGAIN;
#endif
                } else {
                    /* simplified in-place merge sort for 13 elements */
                    /* split 6-7 using ded_sort6 and ded_sort7 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort6(base,first,first+6UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort7(base,first+6UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+6UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 14UL :
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
#if QUICKSELECT_MAX_NETWORK > 13
                    /* 51 comparisons in 10 parallel groups (maybe not optimal) */
                    /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
                    /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk, *pl, *pm, *pn,
                        *po;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size,
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size, pj=ph+size,
                    pk=pj+size, pl=pk+size, pm=pl+size, pn=pm+size, po=pn+size;
                    /* parallel group 1 */
                    CX(pa,pb); CX(pc,pd); CX(pe,pf); CX(pg,ph); CX(pj,pk);
                    CX(pl,pm); CX(pn,po);
                    /* parallel group 2 */
                    CX(pa,pc); CX(pe,pg); CX(pj,pl); CX(pb,pd); CX(pf,ph);
                    CX(pk,pm);
                    /* parallel group 3 */
                    CX(pa,pe); CX(pj,pn); CX(pb,pf); CX(pk,po); CX(pc,pg);
                    CX(pd,ph);
                    /* parallel group 4 */
                    CX(pa,pj); CX(pb,pk); CX(pc,pl); CX(pd,pm); CX(pe,pn);
                    CX(pf,po);
                    /* parallel group 5 */
                    CX(pf,pl); CX(pg,pk); CX(pd,pn); CX(ph,pm); CX(pb,pc);
                    CX(pe,pj);
                    /* parallel group 6 */
                    CX(pb,pe); CX(ph,po); CX(pc,pj);
                    /* parallel group 7 */
                    CX(pc,pe); CX(pf,pg); CX(pk,pl); CX(pm,po); CX(pd,pj);
                    CX(ph,pn);
                    /* parallel group 8 */
                    CX(pd,pf); CX(pg,pj); CX(ph,pk); CX(pl,pn);
                    /* parallel group 9 */
                    CX(pd,pe); CX(pf,pg); CX(ph,pj); CX(pk,pl); CX(pm,pn);
                    /* parallel group 10 */
                    CX(pg,ph); CX(pj,pk);
#else
                    return EAGAIN;
#endif
                } else if (128UL<size_ratio) {
                    ret=limited_indirect_mergesort(base,first,beyond,nmemb,
                        size,COMPAR_ARGS,alignsize,size_ratio,table_index,
                        cache_sz,pbeyond,options);
                } else {
                    /* simplified in-place merge sort for 13 elements */
                    /* split 7-7 using ded_sort7 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort7(base,first,first+7UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort7(base,first+7UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+7UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 15UL :
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
#if QUICKSELECT_MAX_NETWORK > 14
                    /* 56 comparisons in 10 parallel groups (maybe not optimal) */
                    /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
                    /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk, *pl, *pm,
                         *pn, *po, *pp;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size,
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size,
                    pj=ph+size, pk=pj+size, pl=pk+size, pm=pl+size,
                    pn=pm+size, po=pn+size, pp=po+size;
                    /* parallel group 1 */
                    CX(pa,pb); CX(pc,pd); CX(pe,pf); CX(pg,ph); CX(pj,pk);
                    CX(pl,pm); CX(pn,po);
                    /* parallel group 2 */
                    CX(pa,pc); CX(pe,pg); CX(pj,pl); CX(pn,pp); CX(pb,pd);
                    CX(pf,ph); CX(pk,pm);
                    /* parallel group 3 */
                    CX(pa,pe); CX(pj,pn); CX(pb,pf); CX(pk,po); CX(pc,pg);
                    CX(pl,pp); CX(pd,ph);
                    /* parallel group 4 */
                    CX(pa,pj); CX(pb,pk); CX(pc,pl); CX(pd,pm); CX(pe,pn);
                    CX(pf,po); CX(pg,pp);
                    /* parallel group 5 */
                    CX(pf,pl); CX(pg,pk); CX(pd,pn); CX(po,pp); CX(ph,pm);
                    CX(pb,pc); CX(pe,pj);
                    /* parallel group 6 */
                    CX(pb,pe); CX(ph,po); CX(pc,pj); CX(pm,pp);
                    /* parallel group 7 */
                    CX(pc,pe); CX(pf,pg); CX(pk,pl); CX(pm,po); CX(pd,pj);
                    CX(ph,pn);
                    /* parallel group 8 */
                    CX(pd,pf); CX(pg,pj); CX(ph,pk); CX(pl,pn);
                    /* parallel group 9 */
                    CX(pd,pe); CX(pf,pg); CX(ph,pj); CX(pk,pl); CX(pm,pn);
                    /* parallel group 10 */
                    CX(pg,ph); CX(pj,pk);
#else
                    return EAGAIN;
#endif
                } else if (128UL<size_ratio) {
                    ret=limited_indirect_mergesort(base,first,beyond,nmemb,
                        size,COMPAR_ARGS,alignsize,size_ratio,table_index,
                        cache_sz,pbeyond,options);
                } else {
                    /* simplified in-place merge sort for 13 elements */
                    /* split 7-8 using ded_sort7 and ded_sort8 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort7(base,first,first+7UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort8(base,first+7UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+7UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            case 16UL :
                if (0U==(options&(
                (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                )) {
#if QUICKSELECT_MAX_NETWORK > 15
                    /* 45 comparisons in 10 parallel groups (maybe not optimal) */
                    /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
                    /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
                    char *pc, *pd, *pe, *pf,  *pg, *ph, *pj, *pk, *pl, *pm,
                         *pn, *po, *pp, *pq;
# if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: SORTING NETWORK nmemb=%lu, "
                            "options=0x%x */\n",__func__,
                            dedicated_sort_src_file,__LINE__,
                            (unsigned long)nmemb,options);
                    }
# endif
                    pa=base+first*size, pb=pa+size, pc=pb+size, pd=pc+size,
                    pe=pd+size, pf=pe+size, pg=pf+size, ph=pg+size,
                    pj=ph+size, pk=pj+size, pl=pk+size, pm=pl+size,
                    pn=pm+size, po=pn+size, pp=po+size, pq=pp+size;
                    /* parallel group 1 */
                    CX(pa,pb); CX(pc,pd); CX(pe,pf); CX(pg,ph); CX(pj,pk);
                    CX(pl,pm); CX(pn,po); CX(pp,pq);
                    /* parallel group 2 */
                    CX(pa,pc); CX(pe,pg); CX(pj,pl); CX(pn,pp); CX(pb,pd);
                    CX(pf,ph); CX(pk,pm); CX(po,pq);
                    /* parallel group 3 */
                    CX(pa,pe); CX(pj,pn); CX(pb,pf); CX(pk,po); CX(pc,pg);
                    CX(pl,pp); CX(pd,ph); CX(pm,pq);
                    /* parallel group 4 */
                    CX(pa,pj); CX(pb,pk); CX(pc,pl); CX(pd,pm); CX(pe,pn);
                    CX(pf,po); CX(pg,pp); CX(ph,pq);
                    /* parallel group 5 */
                    CX(pf,pl); CX(pg,pk); CX(pd,pn); CX(po,pp); CX(ph,pm);
                    CX(pb,pc); CX(pe,pj);
                    /* parallel group 6 */
                    CX(pb,pe); CX(ph,po); CX(pc,pj); CX(pm,pp);
                    /* parallel group 7 */
                    CX(pc,pe); CX(pf,pg); CX(pk,pl); CX(pm,po); CX(pd,pj);
                    CX(ph,pn);
                    /* parallel group 8 */
                    CX(pd,pf); CX(pg,pj); CX(ph,pk); CX(pl,pn);
                    /* parallel group 9 */
                    CX(pd,pe); CX(pf,pg); CX(ph,pj); CX(pk,pl); CX(pm,pn);
                    /* parallel group 10 */
                    CX(pg,ph); CX(pj,pk);
#else
                    return EAGAIN;
#endif
                } else if (25UL<size_ratio) {
                    ret=limited_indirect_mergesort(base,first,beyond,nmemb,
                        size,COMPAR_ARGS,alignsize,size_ratio,table_index,
                        cache_sz,pbeyond,options);
                } else {
                    /* simplified in-place merge sort for 13 elements */
                    /* split 8-8 using ded_sort8 */
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
                    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: unrolled in-place mergesort "
                            "nmemb=%lu */\n",
                            __func__,dedicated_sort_src_file,__LINE__,(unsigned long)nmemb);
                    }
#endif
                    (V)ded_sort8(base,first,first+8UL,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    (V)ded_sort8(base,first+8UL,beyond,size,COMPAR_ARGS,swapf,
                        alignsize,size_ratio,table_index,cache_sz,pbeyond,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+8UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
                }
            break;
            default :
                switch (size_ratio) {
                    case 1UL :
                        if (0U!=(options&(
                        (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                        )) {
                            if (70UL<nmemb)
                                ret=limited_indirect_mergesort(base,first,
                                    beyond,nmemb,size,COMPAR_ARGS,alignsize,
                                    size_ratio,table_index,cache_sz,pbeyond,
                                    options);
                            else
                                ret=inplace_mergesort(base,first,beyond,nmemb,
                                    size,COMPAR_ARGS,swapf,alignsize,size_ratio,
                                    table_index,cache_sz,pbeyond,options);
                        } else return EAGAIN;
                    break;
                    case 2UL :
                        /* insertion sort (stable, but not minimum comparisons)
                           is faster than merge sorts for a range of array sizes
                           at size_ratio 2 (but only an insignificant range and
                           insignificant time difference for size_ratio 1, and
                           never at size_ratio > 2).
                        */
                        switch (options&(
                        (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE)
                        )) {
                            case 0U :
                            return EAGAIN;
                            case QUICKSELECT_STABLE :
                                if (60UL<nmemb)
                                    ret=limited_indirect_mergesort(base,first,
                                        beyond,nmemb,size,COMPAR_ARGS,alignsize,
                                        size_ratio,table_index,cache_sz,pbeyond,
                                        options);
                                else if (25UL<nmemb)
                                    isort_bs(base,first,beyond,size,COMPAR_ARGS,
                                        swapf,alignsize,size_ratio,options);
                                else
                                    ret=inplace_mergesort(base,first,beyond,
                                        nmemb,size,COMPAR_ARGS,swapf,alignsize,
                                        size_ratio,table_index,cache_sz,pbeyond,
                                        options);
                            break;
                            default : /* OPTIMIZE_COMPARISONS w/ or w/o STABLE */
                                if (60UL<nmemb)
                                    ret=limited_indirect_mergesort(base,first,
                                        beyond,nmemb,size,COMPAR_ARGS,alignsize,
                                        size_ratio,table_index,cache_sz,pbeyond,
                                        options);
                                else
                                    ret=inplace_mergesort(base,first,beyond,
                                        nmemb,size,COMPAR_ARGS,swapf,alignsize,
                                        size_ratio,table_index,cache_sz,pbeyond,
                                        options);
                            break;
                        }
                    break;
                    case 3UL : /*FALLTHROUGH*/
                    case 4UL :
                        if (0U!=(options&(
                        (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                        )) {
                            if (40UL<nmemb)
                                ret=limited_indirect_mergesort(base,first,
                                    beyond,nmemb,size,COMPAR_ARGS,alignsize,
                                    size_ratio,table_index,cache_sz,pbeyond,
                                    options);
                            else
                                ret=inplace_mergesort(base,first,beyond,nmemb,
                                    size,COMPAR_ARGS,swapf,alignsize,size_ratio,
                                    table_index,cache_sz,pbeyond,options);
                        } else return EAGAIN;
                    break;
                    case 5UL : /*FALLTHROUGH*/
                    case 6UL :
                        if (0U!=(options&(
                        (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                        )) {
                            if (32UL<nmemb)
                                ret=limited_indirect_mergesort(base,first,
                                    beyond,nmemb,size,COMPAR_ARGS,alignsize,
                                    size_ratio,table_index,cache_sz,pbeyond,
                                    options);
                            else
                                ret=inplace_mergesort(base,first,beyond,nmemb,
                                    size,COMPAR_ARGS,swapf,alignsize,size_ratio,
                                    table_index,cache_sz,pbeyond,options);
                        } else return EAGAIN;
                    break;
                    default :
                        if (0U!=(options&(
                        (QUICKSELECT_OPTIMIZE_COMPARISONS)|(QUICKSELECT_STABLE))
                        )) {
                            /* For random input, ideally use one of the merge
                               sorts for any nmemb where pointers or data fit in
                               cache.  But that results in increased comparisons
                               for data with many repeated values, e.g. binary
                               values.  For binary or constant values, returning
                               EAGAIN on the first pass allows partitioning to
                               result in completion of the sort.  For ternary
                               values, a second pass might be required 2/3 of
                               the time.  Definitive detection of the first pass
                               can use npartitions only for the median_test
                               version of code.  Adding a constant size_t
                               variable nel initially equal to nmemb could be
                               used to detect the first pass (nmemb==nel). A
                               simpler but not definitive approach is to compare
                               nmemb vs. beyond; they are equal on the first
                               pass, but also on some subsequent passes.  For
                               random input, returning EAGAIN on the first (and
                               a few others) pass has only a small effect on
                               comparisons and run time.
                            */
                            if (beyond==nmemb) return EAGAIN;
                            else 
                            if (23UL<nmemb)
                                ret=limited_indirect_mergesort(base,first,
                                    beyond,nmemb,size,COMPAR_ARGS,alignsize,
                                    size_ratio,table_index,cache_sz,pbeyond,
                                    options);
                            else
                                ret=inplace_mergesort(base,first,beyond,nmemb,
                                    size,COMPAR_ARGS,swapf,alignsize,size_ratio,
                                    table_index,cache_sz,pbeyond,options);
                        } else return EAGAIN;
                    break;
                }
            break;
        }
#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
# if ASSERT_CODE < 2
        if ((ret!=EAGAIN)&&(DEBUGGING(CORRECTNESS_DEBUG))) {
# endif
            nmemb=test_array_sort(base,first,beyond-1UL,size,
                compar,options,0U,NULL,NULL);
            if (nmemb!=beyond-1UL) {
                fprintf(stderr,"/* %s: line %d: first=%lu, beyond=%lu, options=0x%x */\n",
                    __func__,__LINE__,first,beyond,options);
                print_some_array(base,first,beyond-1UL,"/* "," */",options);
                A(0==1);
                abort();
            }
# if ASSERT_CODE < 2
        }
# endif
#endif
    }
    return ret ;
}
#endif /* DEDICATED_SORT_SRC_H_INCLUDED */
