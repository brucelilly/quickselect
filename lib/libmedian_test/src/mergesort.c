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
* $Id: ~|^` @(#)    mergesort.c copyright 2016-2018 Bruce Lilly.   \ mergesort.c $
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
/* $Id: ~|^` @(#)   This is mergesort.c version 1.22 dated 2018-06-06T04:31:56Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.mergesort.c */

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
#define ID_STRING_PREFIX "$Id: mergesort.c ~|^` @(#)"
#define SOURCE_MODULE "mergesort.c"
#define MODULE_VERSION "1.22"
#define MODULE_DATE "2018-06-06T04:31:56Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* configuration */
/* no useless "context" crap */
#undef __STDC_WANT_LIB_EXT1__
/* dedicated_sort and quickselect_loop may be called by pointer_mergesort
   (in indirect.h)
*/
#define DEDICATED_SORT d_dedicated_sort
#define QUICKSELECT_LOOP d_quickselect_loop
#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define COMPAR_DECL int(*compar)(const void*,const void*)
#define COMPAR_ARGS compar

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes most other local and system header files required */

#include "initialize_src.h"

#include "exchange.h"           /* reverse */
#include "indirect.h"           /* inplace_merge pointer_mergesort
                                   rearrange_array convert_pointers_to_indices
                                */

#include <assert.h>             /* assert */
#include <stdio.h>

/* in-place mergesort */
static QUICKSELECT_INLINE
void mergesort_internal(char *base, size_t first, size_t beyond,
    const size_t size, int (*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cachesz, size_t pbeyond, unsigned int options)
{
    size_t nmemb=beyond-first;
    /* If nmemb > 1, split into two pieces, sort the two pieces (recursively),
       then merge the two sorted pieces.
    */
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: base=%p, nmemb=%lu, size=%lu, "
            "options=0x%x */\n",__func__,source_file,__LINE__,(void *)base,
            nmemb,size,options);
        print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
    }
#endif
    if (1UL<nmemb) {
        char *pa=base+first*size, *pb=pa+size;
        switch (nmemb) {
            case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
                CX(pa,pb); /* 0,1 */
#if ASSERT_CODE > 1
                A(0>=OPT_COMPAR(pa,pb,options));
#endif
            break;
            case 3UL :
                {
                    /* simplified merge for 3 elements */
                    /* split */
                    char *pc=pb+size;
                    CX(pb,pc); /* 1,2 */
                    if (0<OPT_COMPAR(pa,pb,options)) {
                        EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                            nsw++); /*0,1*/
                        CX(pb,pc); /* 1,2 */
                    }
                    nmerges++; /* this was a merge */
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
#endif
                }
            break;
            case 4UL :
                {
                    /* simplified in-place merge sort for 4 elements */
                    /* split */
                    char *pc=pb+size, *pd=pc+size;
                    /* These compare-exchanges can be performed in parallel. */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    /* merge pieces */
                    inplace_merge(base,first,first+2UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
                    A(0>=OPT_COMPAR(pc,pd,options));
#endif
                }
            break;
            case 5UL :
                {
                    /* Simplified in-place merge sort for 5 elements; average 7.117
                       comparisons.
                    */
                    char *pc=pb+size, *pd=pc+size, *pe=pd+size;
                    /* split 2-3 elements */
                    /* The compare-exchange and sort3 can proceed in parallel. */
                    CX(pa,pb); /* 0,1 */
                    CX(pd,pe); /* 3,4 */
                    if (0<OPT_COMPAR(pc,pd,options)) { /* 2,3 */
                        EXCHANGE_SWAP(swapf,pc,pd,size,alignsize,size_ratio,
                            nsw++); /*2,3*/
                        CX(pd,pe); /* 3,4 */
                    }
                    nmerges++; /* this was a merge */
                    /* merge pieces */
                    inplace_merge(base,first,first+2UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
                    A(0>=OPT_COMPAR(pc,pd,options));
                    A(0>=OPT_COMPAR(pd,pe,options));
#endif
                }
            break;
            case 6UL :
                {
                    /* Simplified in-place merge of two pieces of 3 elements. */
                    char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size;
                    CX(pb,pc); /* 1,2 */
                    if (0<OPT_COMPAR(pa,pb,options)) {
                        EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                            nsw++); /*0,1*/
                        CX(pb,pc); /* 1,2 */
                    }
                    nmerges++; /* this was a merge */
                    CX(pe,pf); /* 4,5 */
                    if (0<OPT_COMPAR(pd,pe,options)) {
                        EXCHANGE_SWAP(swapf,pd,pe,size,alignsize,size_ratio,
                            nsw++); /*3,4*/
                        CX(pe,pf); /* 4,5 */
                    }
                    nmerges++; /* this was a merge */
                    /* merge pieces */
                    inplace_merge(base,first,first+3UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
                    A(0>=OPT_COMPAR(pc,pd,options));
                    A(0>=OPT_COMPAR(pd,pe,options));
                    A(0>=OPT_COMPAR(pe,pf,options));
#endif
                }
            break;
            case 7UL :
                {
                    /* Simplified merge for 7 elements: 12.744 comparisons. */
                    char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                        *pg=pf+size;
                    /* split 3-4 using ded_sort3 */
                    CX(pb,pc); /* 1,2 */
                    if (0<OPT_COMPAR(pa,pb,options)) {
                        EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,
                            nsw++); /*0,1*/
                        CX(pb,pc); /* 1,2 */
                    }
                    nmerges++; /* this was a merge */
                    /* These compare-exchanges can be performed in parallel. */
                    CX(pd,pe); /* 3,4 */ CX(pf,pg); /* 5,6 */
                    inplace_merge(base,first+3UL,first+5UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
                    /* merge pieces */
                    inplace_merge(base,first,first+3UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
                    A(0>=OPT_COMPAR(pc,pd,options));
                    A(0>=OPT_COMPAR(pd,pe,options));
                    A(0>=OPT_COMPAR(pe,pf,options));
                    A(0>=OPT_COMPAR(pf,pg,options));
#endif
                }
            break;
            case 8UL :
                {
                    /* simplified in-place merge sort for 8 elements */
                    /* split */
                    char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                        *pg=pf+size, *ph=pg+size;
                    /* These compare-exchanges can be performed in parallel. */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    CX(pe,pf); /* 4,5 */ CX(pg,ph); /* 6,7 */
                    /* merge pieces */
                    inplace_merge(base,first,first+2UL,first+4UL,size,compar,
                        swapf,alignsize,size_ratio,options);
                    inplace_merge(base,first+4UL,first+6UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
                    inplace_merge(base,first,first+4UL,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
                    A(0>=OPT_COMPAR(pc,pd,options));
                    A(0>=OPT_COMPAR(pd,pe,options));
                    A(0>=OPT_COMPAR(pe,pf,options));
                    A(0>=OPT_COMPAR(pf,pg,options));
                    A(0>=OPT_COMPAR(pg,ph,options));
#endif
                }
            break ;
            case 9UL :
                {
                    /* simplified in-place merge sort for 9 elements */
                    /* split 4,5 (4=2,2;5=2,3)*/
                    char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                        *pg=pf+size, *ph=pg+size, *pj=ph+size;
                    /* These compare-exchanges can be performed in parallel. */
                    CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                    CX(pe,pf); /* 4,5 */ CX(ph,pj); /* 7,8 */
                    /* merge top 3 elements */
                    if (0<OPT_COMPAR(pg,ph,options)) {
                        EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,
                            nsw++); /*6,7*/
                        CX(ph,pj); /* 7,8 */
                    }
                    nmerges++; /* this was a merge */
                    /* merge pieces */
                    /* 2+3=5 */
                    inplace_merge(base,first+4UL,first+6UL,beyond,size,
                        COMPAR_ARGS,swapf,alignsize,size_ratio,options);
#if 1 /* 20.803 */
                    /* 2+2=4 */
                    inplace_merge(base,first,first+2UL,first+4UL,size,
                        COMPAR_ARGS,swapf,alignsize,size_ratio,options);
                    /* 4+5=9 */
                    inplace_merge(base,first,first+4UL,beyond,size,COMPAR_ARGS,
                        swapf,alignsize,size_ratio,options);
#else /* 23.528 */
                    /* 2+5=7 */
                    inplace_merge(base,first+2UL,first+4UL,beyond,size,
                        COMPAR_ARGS,swapf,alignsize,size_ratio,options);
                    /* 2+7=9 */
                    inplace_merge(base,first,first+2UL,beyond,size,
                        COMPAR_ARGS,swapf,alignsize,size_ratio,options);
#endif
#if ASSERT_CODE > 1
                    A(0>=OPT_COMPAR(pa,pb,options));
                    A(0>=OPT_COMPAR(pb,pc,options));
                    A(0>=OPT_COMPAR(pc,pd,options));
                    A(0>=OPT_COMPAR(pd,pe,options));
                    A(0>=OPT_COMPAR(pe,pf,options));
                    A(0>=OPT_COMPAR(pf,pg,options));
                    A(0>=OPT_COMPAR(pg,ph,options));
                    A(0>=OPT_COMPAR(ph,pj,options));
#endif
                }
            break;
            /* larger sizes are handled by recursive split/sort/merge */
            default :
                {
                    register size_t mid, na;
                    /* split */
                    na=(nmemb>>1); mid=first+na;
# if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: %s line %d: base=%p, na=%lu, "
                            "pb=%p */\n",__func__,source_file,__LINE__,
                            (void *)base,na,(void *)pb);
# endif
                    /* sort pieces */
                    /* Sorting pieces could proceed in parallel.
                       2017-09-28:Intel cilk parallelization has much too much
                          overhead.  A lower-overhead means of parallelization
                          is needed to make parallel sorting of pieces practical.
                    */
                    /* This is pure recursive mergesort, using in-place merge
                       (of elements or pointers) for small sub-arrays.  Because
                       this was (originally) called as mergesort, there is no
                       use of sorting networks or insertion sort, and any
                       indirection was specified by the caller.
                    */
                    mergesort_internal(base,first,mid,size,compar,swapf,alignsize,
                        size_ratio,cachesz,pbeyond,options);
                    mergesort_internal(base,mid,beyond,size,compar,swapf,alignsize,
                        size_ratio,cachesz,pbeyond,options);
# if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG)) {
                        (V)fprintf(stderr,"/* %s: %s line %d: concatenated "
                            "sorted halves: base=%p, na=%lu, pb=%p[%lu] */\n",
                            __func__,source_file,__LINE__,(void *)base,na,
                            (void *)pb,(pb-base)/size);
                        print_some_array(base,0UL,nmemb-1UL,"/* ","*/",options);
                    }
# endif
                    inplace_merge(base,first,mid,beyond,size,compar,
                        swapf,alignsize,size_ratio,options);
                }
            break;
        }
    }
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: base=%p, nmemb=%lu */\n",
            __func__,source_file,__LINE__,(void *)base,nmemb);
        print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
    }
#endif
}

extern size_t quickselect_cache_size;

/* avoid namespace clash... */
void xmergesort(char *base, size_t nmemb,
    const size_t size, int (*compar)(const void *,const void *), unsigned int options)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);

    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();

    /* If nmemb > 1, split into two pieces, sort the two pieces (recursively),
       then merge the two sorted pieces.
    */
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
        "/* %s: %s line %d: nmemb=%lu, size=%lu */\n",
        __func__,source_file,__LINE__,nmemb,size);
#endif
    if (1UL<nmemb) {
        size_t alignsize=alignment_size(base,size);
        size_t size_ratio=size/alignsize;
        void (*swapf)(char *, char *, size_t);
        size_t *indices = NULL;
        char **pointers = NULL;
        void (*pswapf)(char *, char *, size_t);

        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            char **p=set_array_pointers(pointers,nmemb,base,size,0UL,nmemb);
            if (NULL==p) {
                if (NULL!=pointers) { free(pointers); pointers=NULL; }
                goto no_ind;
            } else {
                size_t n;
                if (p!=pointers) pointers=p;
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: pointer index:pointer"
                        "->base index:\n",__func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%p->%lu",n,pointers[n],
                            (pointers[n]-base)/size);
                    (V)fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,
                            "/* base array: "," */",options);
                }
#endif
                if (0U==instrumented) pswapf=swapn(sizeof(char *));
                else pswapf=iswapn(sizeof(char *));
                /* pswapf is guaranteed to have been assigned a value by the
                   above statement (and always if options contains
                   QUICKSELECT_INDIRECT after this block), whether or not (:-/)
                   gcc's authors realize it...
                */
                A(pointers[0]==base);
                mergesort_internal((char *)pointers,0UL,nmemb,sizeof(char *),
                    compar,pswapf,sizeof(char *),1UL,quickselect_cache_size,0UL,
                    options);
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: pointer index:pointer"
                        "->base index:\n",__func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%p->%lu",n,pointers[n],
                            (pointers[n]-base)/size);
                    (V)fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,
                            "/* base array: "," */",options);
                }
#endif
                if (NULL==indices) indices=(size_t *)pointers;
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,indices,0UL,nmemb);
                A(NULL!=indices);
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: indices:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%lu",n,indices[n]);
                    (V)fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,
                            "/* base array: "," */",options);
                }
#endif
                n=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                    alignsize);
                free(pointers);
                if ((void *)indices==(void *)pointers) indices=NULL;
                pointers=NULL;
                if (nmemb<(n>>1)) { /* indirection failed; use direct sort/select */
                    /* shouldn't happen */
                    (V)fprintf(stderr,"/* %s: %s line %d: size_ratio=%lu, nmemb"
                        "=%lu, indirect sort failed */\n",__func__,source_file,
                        __LINE__,size_ratio,nmemb);
no_ind:             options&=~(QUICKSELECT_INDIRECT);
                    mergesort_internal(base,0UL,nmemb,size,compar,swapf,
                        alignsize,size_ratio,quickselect_cache_size,0UL,options);
                } else if (0UL!=n) {
                    nmoves+=n*size_ratio;
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG)) {
                        (V)fprintf(stderr,"/* %s: %s line %d: rearrange_array "
                            "moved %lu elements */\n",__func__,source_file,
                            __LINE__,n);
                        print_some_array(base,0UL,nmemb-1UL,
                                "/* base array: "," */",options);
                    }
#endif
                }
            }
        } else
            mergesort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio,quickselect_cache_size,0UL,options);
    }
}

int d_indirect_mergesort(char *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *), unsigned int options)
{
    int r=0;
    if ((NULL==base)
    || (0UL==size)
    || (NULL==compar)
    ) {
        r=errno=EINVAL;
    } else {
        if (1UL<nmemb) {
            size_t nptrs=nmemb+((nmemb+1UL)>>1); /* (1.5+)*N pointers */

            if ((char)0==file_initialized) initialize_file(__FILE__);

            /* Determine cache size once on first call. */
            if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();

#if DEBUG_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
                (V)fprintf(stderr,
                    "/* %s: %s line %d: nmemb=%lu, size=%lu, compar=%s, "
                    "options=0x%x */\n",__func__,source_file,__LINE__,
                    (unsigned long)nmemb,(unsigned long)size,
                    comparator_name(compar),options);
            }
#endif

            /* allocate and initialize pointers */
            char **pointers=set_array_pointers(NULL,nptrs,base,size,0UL,nmemb);
            if (NULL!=pointers) {
                char **p; size_t *indices, n, alignsize, size_ratio;
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: base=%p, nmemb=%lu, "
                        "nptrs=%lu */\n",__func__,source_file,__LINE__,
                        (void*)base,nmemb,nptrs);
                    print_some_array(base,0UL,nmemb-1UL,
                            "/* base array:"," */",options);
                    (V)fprintf(stderr,"/* %s: %s line %d: pointers:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nptrs; n++)
                        (V)fprintf(stderr," %lu:%p",n,pointers[n]);
                    (V)fprintf(stderr," */\n");
                }
#endif
                if (NULL==pointerswap) pointerswap=swapn(sizeof(char *));
                /* mergesort using indirection; pointers moved, not data */
                pointer_mergesort(pointers,0UL,base,nmemb,nmemb,compar,
                    quickselect_cache_size,options|QUICKSELECT_INDIRECT);
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: pointers:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nptrs; n++)
                        (V)fprintf(stderr," %lu:%p",n,pointers[n]);
                    (V)fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,
                            "/* base array: "," */",options);
                }
#endif
                /* discard extra pointers */
                /* C11 defines aligned_alloc, but no aligned versions of
                   calloc or realloc; it is assumed(!) that realloc doesn't
                   decrease the alignment when decreasing the size of the
                   allocated block...
                */
#if REALLOC_DOES_NOT_DECREASE_ALIGNMENT
                p=realloc(pointers,nmemb*sizeof(char *));
                if (NULL!=p) pointers=p;
#endif /* REALLOC_DOES_NOT_DECREASE_ALIGNMENT */
                /* rearrangement of data elements is more efficiently performed
                   using indices to the data elements, rather than pointers
                */
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,(size_t *)pointers,0UL,nmemb);
                A(NULL!=indices);
                npiconversions+=nmemb;
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: indices:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%lu",n,indices[n]);
                    (V)fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,
                            "/* base array: "," */",options);
                }
#endif
                /* Rearrange the data elements according to sorted order using
                   the indices derived from the sorted pointers. No data element
                   is moved more than once.  However, there is poor locality of
                   access for random original data order.
                */
                n=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                    alignsize);
                if (n>nptrs) r=errno;
                nmoves+=n;
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: base=%p, nmemb=%lu "
                        "*/\n",__func__,source_file,__LINE__,(void*)base,nmemb);
                    print_some_array(base,0UL,nmemb-1UL, "/* "," */",options);
                    (V)fprintf(stderr,"/* %s: %s line %d: indices:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%lu",n,indices[n]);
                    (V)fprintf(stderr," */\n");
                }
#endif
                /* pointers (now indices) are no longer needed */
                free(pointers);
                if (n>nptrs) errno=r; /* in case free changed errno */
            } else {
                r=errno;
            }
        }
    }
    return r;
}

#include "insertion.h" /* inline code: binary_search insert search_and_insert */

/* convert comparison (first,second) return value to one of two values:
    1: non-decreasing
   -1: strictly decreasing (necessary to preserve partial-order stability)
   N.B. the sign is reversed from the raw comparison result
*/
static QUICKSELECT_INLINE int binary_value(int c)
{
    if (0 < c) return -1;
    return 1;
}

static QUICKSELECT_INLINE
size_t extend_run(char *base, size_t first, size_t current, const size_t size,
    COMPAR_DECL, size_t size_ratio, int runtype, unsigned int options)
{
    register char *pa, *pb, *pl;
    register int c;
    register size_t f;

#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
        "/* %s: %s line %d: first=%lu, current=%lu, size=%lu, runtype=%d */\n",
        __func__,source_file,__LINE__,first,current,size,runtype);
#endif
/* XXX this is for direct sorting; indirect version is not implemented */
    pl=base+first*size, pb=base+current*size, pa=pb-size, f=current;
    A(first<=current);A(current<((SIZE_MAX)>>1));
    A((f>0UL)||(pa<pl));
    for (; pa>=pl; f--,pb=pa,pa-=size) {
        c=binary_value(COMPAR(pa,pb));
# if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG)) {
            (V) fprintf(stderr,
                "/* %s: %s line %d: compared pa=%p[%lu] to pb=%p[%lu] -> c=%d, "
                "f=%lu */\n",
                __func__,source_file,__LINE__,(void *)pa,(pa-base)/size,
                (void *)pb,(pb-base)/size,c,f);
        }
# endif
        if (c!=runtype) break;
    }
# if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V) fprintf(stderr,
            "/* %s: %s line %d: run: first=%lu, f=%lu, current=%lu */\n",
            __func__,source_file,__LINE__,first,f,current);
    }
# endif
    A(f<=current);A(f>=first);A(f<((SIZE_MAX)>>1));
    return f;
}

/* XXX If this weren't so slow, it would be placed in indirect.h for use by
   dedicated_sort.  However, while the algorithm (merge existing runs in the
   input (after reversing decreasing sequences)) is interesting, the
   performance is poor: more comparisons on average than insertion sort with
   binary search (which itself uses more comparisons on average than in-place
   mergesort), and longer run time than other (insertion, in-place merge,
   network) sorting methods. However, it does use slightly less data movement
   than in-place mergesort (but more than sorting networks).
   This could possibly be improved by merging nearly-equal-size runs (i.e.
   deferring a merge while merging smaller regions to the left), but that gets
   complicated (especially given the constraint of the left region being no
   larger than the right for any given merge).
*/
static
QUICKSELECT_INLINE
void merge_runs(char *base, size_t first, size_t beyond, const size_t size,
    COMPAR_DECL, void(*swapf)(char*, char*, size_t),
    size_t alignsize, size_t size_ratio, unsigned int options)
{
    char *pa, *pb, *pu;
    int runtype;
    size_t b1, b2, f1, f2, len1, len2, b3, f3, len3;

#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
        "/* %s: %s line %d: first=%lu, beyond=%lu, size=%lu */\n",
        __func__,source_file,__LINE__,first,beyond,size);
#endif
    if (first+1UL>=beyond) return; /* nothing to do */
/* XXX this is for direct sorting; indirect version is not implemented */
    pu=base+beyond*size;
    /* find upper run; length is at least 2 elements */
    len1=0UL,len2=2UL, pb=pu-size,pa=pb-size,b2=beyond,f2=b2-2UL;
    runtype=binary_value(COMPAR(pa,pb));
# if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V) fprintf(stderr,
            "/* %s: %s line %d: compared pa=%p[%lu] to pb=%p[%lu] -> "
            "runtype=%d, f2=%lu, b2=%lu, len2=%lu */\n",
            __func__,source_file,__LINE__,(void *)pa,(pa-base)/size,
            (void *)pb,(pb-base)/size,runtype,f2,b2,len2);
        print_some_array(base,first,beyond-1UL, "/* "," */",options);
    }
# endif
    A(f2>=first);A(f2<b2);A(b2==beyond);
    /* reverse decreasing runs (including node element) */
    if (-1==runtype) {
        f2=extend_run(base,first,f2,size,COMPAR_ARGS,size_ratio,runtype,
            options);
        A(f2>=first);A(f2<b2);A(b2==beyond);
        reverse(base,f2,b2,size,swapf,alignsize,size_ratio);
# if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG)) {
            print_some_array(base,first,beyond-1UL,
                    "/* upper run reversed: "," */",options);
        }
# endif
    }
    /* try to extend run */
    f2=extend_run(base,first,f2,size,COMPAR_ARGS,size_ratio,1,options);
    /* now have maximum length upper run, in non-decreasing order */
    /* if that's everything, return */
    while (f2>first) {
        A(f2<b2);A(b2==beyond);
        A(f2<((SIZE_MAX)>>1));A(b2<((SIZE_MAX)>>1));
        /* find lower run */
        b1=f2;
        if (b1>first) {
            /* establish run type */
            f1=b1-1UL,pb=base+f1*size,pa=pb-size;
            runtype=binary_value(COMPAR(pa,pb));
            if (f1>first)
                f1=extend_run(base,first,f1-1UL,size,COMPAR_ARGS,size_ratio,
                    runtype,options);
            A(f1<b1);A(b1==f2);A(b2==beyond);
            A(f1<((SIZE_MAX)>>1));A(b1<((SIZE_MAX)>>1));
            len1=b1-f1;
            A((f1==first)||(1UL<len1)); /* a run must have at least two elements (unless there aren't that many left) */
            if (len1>1UL) { /* more than 1 element in run */
                /* if reversed, reverse it */
                if (-1==runtype) {
                    reverse(base,f1,b1,size,swapf,alignsize,size_ratio);
                    /* if reversed, try to extend */
# if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG)) {
                        print_some_array(base,first,beyond-1UL,
                            "/* lower run reversed: "," */",options);
                    }
# endif
                    f1=extend_run(base,first,f1,size,COMPAR_ARGS,size_ratio,1,options);
                    len1=b1-f1;
                }
            }
            len2=b2-f2;
        } else continue; /* len1=0 */
        /* Merge up to len2 elements from lower run with upper run, decreasing
           lower run length and increasing upper run length; repeat until
           lower run is completely merged.
        */
        if (1UL==len1) {
            A(f1==first);
            binary_search_and_insert(base,f1,f2,b2-1UL,size,COMPAR_ARGS,swapf,
                alignsize,size_ratio,options);
            break; /* must have been the first element */
        }
        while (0UL<len1) {
            A(f1<b1);A(b1==f2);A(b2==beyond);
            if (len1<=len2) len3=len1; else len3=len2;
            b3=b1, f3=b3-len3;
            A(f3<((SIZE_MAX)>>1));A(b3<((SIZE_MAX)>>1));
            A(b1==f2);A(b2==beyond); /* XXX could possibly simplify variables to l,m,beyond */
/* XXX might choose to use a variant of indirect mergesort here depending on size_ratio */
            inplace_merge(base,f3,f2,b2,size,COMPAR_ARGS,swapf,alignsize,
                size_ratio,options);
            b1=f2=f3, len2=b2-f2, len1=b1-f1;
# if DEBUG_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG)) {
                (V) fprintf(stderr,
                    "/* %s: %s line %d: merged f3=%lu through b2=%lu, len1=%lu "
                    "remaining, f2=%lu, first=%lu */\n",
                    __func__,source_file,__LINE__,f3,b2,len1,f2,first);
                print_some_array(base,first,beyond-1UL,
                    "/* merged: "," */",options);
            }
# endif
        }
    }
# if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        print_some_array(base,first,beyond-1UL,"/* sorted: "," */",options);
    }
# endif
}

void runsort(char *base, size_t nmemb, const size_t size,
    int (*compar)(const void *,const void *), unsigned int options)
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        (V)fprintf(stderr, "%s: indirect sorting is not supported\n",__func__);
        return;
    }
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    /* If nmemb > 1, split into two pieces, sort the two pieces (recursively),
       then merge the two sorted pieces.
    */
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
        "/* %s: %s line %d: nmemb=%lu, size=%lu */\n",
        __func__,source_file,__LINE__,nmemb,size);
#endif
    if (1UL<nmemb) {
        merge_runs(base,0UL,nmemb,size,COMPAR_ARGS,swapf,alignsize,
            size_ratio,options);
    }
}
