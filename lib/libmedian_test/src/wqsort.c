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
* $Id: ~|^` @(#)    wqsort.c copyright 2016-2018 Bruce Lilly.   \ wqsort.c $
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
/* $Id: ~|^` @(#)   This is wqsort.c version 1.18 dated 2018-04-23T05:16:06Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.wqsort.c */

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
#define ID_STRING_PREFIX "$Id: wqsort.c ~|^` @(#)"
#define SOURCE_MODULE "wqsort.c"
#define MODULE_VERSION "1.18"
#define MODULE_DATE "2018-04-23T05:16:06Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define QUICKSELECT_LOOP d_quickselect_loop

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* quickselect_loop declaration */
#if ! defined(QUICKSELECT_LOOP_DECLARED)
QUICKSELECT_EXTERN
# include "quickselect_loop_decl.h"
;
# define QUICKSELECT_LOOP_DECLARED 1
#endif /* QUICKSELECT_LOOP_DECLARED */

#include "pivot_src.h"

extern void d_klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *pfk, size_t *pbk);
extern struct sampling_table_struct *d_sampling_table(size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk, char **ppeq,
    unsigned int *psort, unsigned int *pindex, size_t nmemb);

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

static
QUICKSELECT_INLINE
/* spacings are in elements */
/* remedian does not modify the value pointed to by middle, but it is not
   declared as const to avoid spurious compiler warnings about discarding the
   const qualifier when passing middle to fmed3 (which also doesn't make
   modifications, but see the rationale there)
*/
char *low_remedian(register char *middle, register size_t row_spacing,
    register size_t sample_spacing, register size_t size,
    register unsigned int idx, int(*compar)(const void*,const void*),
    char *base)
{
    register size_t o;
    if (aqcmp!=compar) return NULL;
#if DEBUG_CODE
    if (DEBUGGING(WQSORT_DEBUG)||DEBUGGING(REMEDIAN_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: middle=%p[%lu], row_spacing=%lu, "
            "sample_spacing=%lu, idx=%u */\n",__func__,source_file,__LINE__,
            (const void *)middle,(middle-base)/size,(unsigned long)row_spacing,
            (unsigned long)sample_spacing,idx);
    }
#endif
    A((SAMPLING_TABLE_SIZE)>idx);
    if ((0U<idx)&&(0U < --idx)) {
        char *pa, *pb;
        register size_t s=sample_spacing/3UL;

        o=s*size;
        pa=low_remedian(middle-o,row_spacing,s,size,idx,compar,base);
        pb=low_remedian(middle,row_spacing,s,size,idx,compar,base);
        if (nfrozen<pivot_minrank)
            (V)freeze(aqindex(pa,base,size));
        if (pb!=middle) {
            if (nfrozen<pivot_minrank)
                (V)freeze(aqindex(pb,base,size));
        }
        return pb;
    }
    o=row_spacing*size;
    if (nfrozen<pivot_minrank)
        (V)freeze(aqindex(middle-o,base,size));
    return middle;
}

/* pivot selection using remedian or median-of-medians */
char *freeze_some_samples(register char *base, register size_t first,
    size_t beyond, register size_t size,
    int (*compar)(const void *, const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    register unsigned int table_index, unsigned int options)
{
    size_t m, nmemb=beyond-first, o, p, q, s, t, u, w, x;
    register size_t r=nmemb/3UL;     /* 1/3 #elements */
    register char *pb, *pc, *pivot, *pl, *pu;

    if (aqcmp!=compar) return NULL;
    if ((char)0==file_initialized) initialize_file(__FILE__);
    m=(nmemb>>1)+first, pivot=base+size*m;      /* [upper-]middle element */
    pl=base+size*first, pu=base+size*beyond;
    A((SAMPLING_TABLE_SIZE)>table_index);
#if DEBUG_CODE
    if (DEBUGGING(WQSORT_DEBUG))
        (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
            "table_index=%u, options=0x%x */\n",__func__,source_file,__LINE__,
            first,beyond,table_index,options);
#endif
    switch (options&((QUICKSELECT_STABLE)|(QUICKSELECT_RESTRICT_RANK))) {
#if QUICKSELECT_STABLE
        case ((QUICKSELECT_RESTRICT_RANK)|(QUICKSELECT_STABLE)) :
            /* almost full remedian */
            while (table_index<(SAMPLING_TABLE_SIZE)-1U) {
                s=sorting_sampling_table[table_index].samples;
                if (s>r) break;
                table_index++;
            }
            while ((s=sorting_sampling_table[table_index].samples)>r)
                table_index--;
            if (table_index==(SAMPLING_TABLE_SIZE)-1U) {
                for (; s<r; table_index++)
                    s*=3UL;
                if (s>r) table_index--;
            }
        /*FALLTHROUGH*/
#endif /* QUICKSELECT_STABLE */
        default : /* remedian of samples */
            nfrozen=0UL, pivot_minrank=minimum_remedian_rank(table_index);
#if DEBUG_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "table_index=%u, options=0x%x, pivot_minrank=%lu */\n",
                    __func__,source_file,__LINE__,first,beyond,table_index,
                    options,pivot_minrank);
#endif
            /* pre-freeze before pivot selection */
            p=count_frozen(base,first,beyond,size);
            q=pivot_rank(base,first,beyond,size,pivot);
            if (0U<table_index) s=sorting_sampling_table[table_index-1U].samples;
            else s=1UL;
            w=r/s;
            o=(s>>1)*w,x=m-r-o,u=m+r+o+1UL;
            t=0UL;
            pivot_sample_rank(pivot,r,r,size,table_index,pivot,&t,base);
#if DEBUG_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "table_index=%u, options=0x%x, pivot_minrank=%lu: "
                    "pre-freeze %lu frozen elements, overall pivot rank %lu/%lu"
                    ", pivot rank within samples ([%lu,%lu) by %lu) %lu */\n",
                    __func__,source_file,__LINE__,first,beyond,table_index,
                    options,pivot_minrank,p,q,nmemb,x,u,w,t);
#endif
            /* freeze low-address samples which will be used for pivot selection */
            /* mandatory elements (corresponding to minimum possible pivot rank) */
            (V)low_remedian(pivot,r,r,size,table_index,compar,base);
            /* optional pivot selection samples if needed */
            /* first row */
            if (0UL<w) {
                for (o=(s>>1)*w,x=m-r-o,u=m-r+o; x<=u; x+=w) {
                    if (nfrozen<pivot_minrank) {
#if DEBUG_CODE
                        if (DEBUGGING(WQSORT_DEBUG))
                            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, "
                                "beyond=%lu, nfrozen=%lu, pivot_minrank=%lu, "
                                "x=%lu, x+r=%lu */\n",__func__,source_file,
                                __LINE__,first,beyond,nfrozen,pivot_minrank,x,
                                x+r);
#endif
                        pb=base+size*x;
                        A(pb>=pl);
                        (V)freeze(aqindex(pb,base,size));
                    }
                }
                /* middle row, left */
                for (x=m-o; x<m; x+=w) {
                    if (nfrozen<pivot_minrank) {
#if DEBUG_CODE
                        if (DEBUGGING(WQSORT_DEBUG))
                            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, "
                                "beyond=%lu, nfrozen=%lu, pivot_minrank=%lu, x="
                                "%lu, x+r=%lu */\n",__func__,source_file,
                                __LINE__,first,beyond,nfrozen,pivot_minrank,x,
                                x+r);
#endif
                        pb=base+size*x;
                        A(pb>=pl);
                        (V)freeze(aqindex(pb,base,size));
                    }
                }
            }
            /* if necessary to increase pivot rank, freeze more elements */
#if 1
            for (pc=pl; (nfrozen<pivot_minrank)&&(pc+size<pu); pc+=size) {
                if (pc==pivot) continue; /* don't freeze middle pivot sample */
                (V)freeze(aqindex(pc,base,size)); /* freeze element @ pc */
            }
#endif
            /* freeze middle pivot selection sample */
            (V)freeze(aqindex(pivot,base,size));
            p=count_frozen(base,first,beyond,size);
            q=pivot_rank(base,first,beyond,size,pivot);
            x=m-r-o,u=m+r+o+1UL;
            t=0UL;
            pivot_sample_rank(pivot,r,r,size,table_index,pivot,&t,base);
#if DEBUG_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "table_index=%u, options=0x%x, pivot_minrank=%lu: post-"
                    "freeze %lu frozen elements, overall pivot rank %lu/%lu, "
                    "pivot rank within %lu samples %lu */\n",__func__,
                    source_file,__LINE__,first,beyond,table_index,options,
                    pivot_minrank,p,q,nmemb,s*3UL,t);
#endif
        break;
        case (QUICKSELECT_RESTRICT_RANK) : /* median-of-medians */
            nfrozen=0UL, pivot_minrank=r; /* pre-freeze not required */
            if (0UL==(r&0x01UL)) pivot_minrank++;
            p=count_frozen(base,first,beyond,size);
            q=pivot_rank(base,first,beyond,size,pivot);
#if DEBUG_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "table_index=%u, options=0x%x, pivot_minrank=%lu: pre-"
                    "selection %lu frozen elements, pivot rank %lu */\n",
                    __func__,source_file,__LINE__,first,beyond,table_index,
                    options,pivot_minrank,p,q);
#endif
        break;
    }
    return pivot;
}

size_t minimum_remedian_rank(unsigned int table_index)
{
    /* samples=3^table_index, minimum 0-based rank is 2^table_index-1 */
    size_t r;
    for (r=1UL; 0U<table_index; table_index--,r<<=1) ;
    return --r;
}

/* modified quickselect sort; always get worst-case pivot for given
   repivot_factor that would just avoid repivoting
*/
static void wqsort_internal(void *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t *pk, size_t firstk, size_t beyondk,
    unsigned int table_index, size_t pbeyond, unsigned int options, int c)
{
    char *pc, *pd, *pe, *pf, *pivot;
    size_t nmemb, r, ratio=0, s, samples, t;
    auto size_t lk=firstk, p, q, rk=beyondk;
    auto unsigned int sort;
    struct sampling_table_struct *psts;
        
    for (;;) {
#if DEBUG_CODE
        if (DEBUGGING(WQSORT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, firstk"
                "=%lu, beyondk=%lu */\n",__func__,source_file,__LINE__,first,
                beyond,firstk,beyondk);
#endif
        nmemb=beyond-first;
        if (1UL<nmemb) {
            nfrozen=0UL, pivot_minrank=nmemb;
            if ((nmemb<=quickselect_small_array_cutoff)
            && (nmemb<5UL) /* must avoid indirection */
            ) {
                (V)QUICKSELECT_LOOP(base,first,beyond,size,COMPAR_ARGS,NULL,0UL,
                    0UL,swapf,alignsize,size_ratio,table_index,
                    quickselect_cache_size,0UL,options,NULL,NULL);
                return;
            }
        } else return;
        A(table_index < (SAMPLING_TABLE_SIZE));
        psts=d_sampling_table(first,beyond,pk,firstk,beyondk,NULL,&sort,
            &table_index,nmemb);
        samples=psts[table_index].samples;

        /* freeze low-address samples which will be used for pivot selection */
        if (aqcmp==compar)
            (V)freeze_some_samples(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,table_index,options);

        /* normal pivot selection (for comparison and swap counts) */
        pivot=d_select_pivot(base,first,beyond,size,compar,swapf,alignsize,
            size_ratio,table_index,NULL,quickselect_cache_size,pbeyond,options,
            &pc,&pd,&pe,&pf);
#if DEBUG_CODE
        t=pivot_minrank;
#endif
        pivot_minrank=nmemb;
        /* XXX no support for efficient stable sorting */
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,swapf,
            alignsize,size_ratio,quickselect_cache_size,options,&p,&q);
#if DEBUG_CODE
        if (p+1UL!=q) {
            (V)fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, first=%lu, p=%lu, "
                "q=%lu, beyond=%lu, samples=%lu, pivot_minrank=%lu(%lu), "
                "nfrozen=%lu */\n",__func__,source_file,__LINE__,nmemb,first,p,
                q,beyond,samples,pivot_minrank,t,nfrozen);
            print_some_array(base,first,beyond-1UL,"/* "," */",options);
        }
#endif
        if (p>first) s=p-first; else s=0UL; /* size of the < region */
        if (beyond>q) r=beyond-q; else r=0UL;  /* size of the > region */
        if (s<r) { /* > region is larger */
            ratio=r/(nmemb-r);
        } else { /* < region is larger, or regions are the same size */
            ratio=s/(nmemb-s);
        }
#if DEBUG_CODE
        if (DEBUGGING(WQSORT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: p=%lu, q=%lu, pivot rank=s="
                "%lu, r=%lu, ratio=%lu */\n",
                __func__,source_file,__LINE__,p,q,s,r,ratio);
#endif
        if (NULL!=pk) d_klimits(p,q,pk,firstk,beyondk,&lk,&rk);
        else rk=firstk, lk=beyondk;
        /* < region indices [first,p), order statistics [firstk,lk) */
        /* > region indices [q,beyond), order statistics [rk,beyondk) */
#if DEBUG_CODE
        if (DEBUGGING(WQSORT_DEBUG)) {
            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, p=%lu, q=%lu, "
                "beyond=%lu, samples=%lu, pivot_minrank=%lu(%lu), nfrozen=%lu, "
                "firstk=%lu, lk=%lu, rk=%lu, beyondk=%lu */\n",
                __func__,source_file,__LINE__,first,p,q,beyond,samples,
                pivot_minrank,t,nfrozen,firstk,lk,rk,beyondk);
        }
#endif
        if (s<r) { /* > region is larger */
            if (1UL<s) {
                nrecursions++;
                wqsort_internal(base,first,p,size,compar,swapf,alignsize,
                    size_ratio,pk,firstk,lk,table_index,pbeyond,options,0);
            }
            first=q, firstk=lk;
        } else { /* < region is larger, or regions are the same size */
            if (1UL<r) {
                nrecursions++;
                wqsort_internal(base,q,beyond,size,compar,swapf,alignsize,
                    size_ratio,pk,rk,beyondk,table_index,pbeyond,options,0);
            }
            beyond=p, beyondk=rk;
        }
    }
}

void wqsort(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *), size_t *pk, size_t nk,
    unsigned int options)
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);
    unsigned int table_index;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

#if DEBUG_CODE
    if (DEBUGGING(WQSORT_DEBUG)&&DEBUGGING(RATIO_GRAPH_DEBUG)) {
        /* for graphing worst-case partition ratios */
        size_t q;
        for (q=0UL; q<(SAMPLING_TABLE_SIZE); q++)
            stats_table[q].max_ratio=stats_table[q].repivot_ratio=0UL,
                stats_table[q].repivots=0UL;
    }
#endif

    table_index=nmemb<=
#if ( SIZE_MAX < 65535 )
# error "SIZE_MAX < 65535 [C11 draft N1570 7.20.3]"
#elif ( SIZE_MAX == 65535 ) /* 16 bits */
        sorting_sampling_table[2].max_nmemb?1UL:3UL
#elif ( SIZE_MAX == 4294967295 ) /* 32 bits */
        sorting_sampling_table[5].max_nmemb?2UL:7UL
#elif ( SIZE_MAX == 18446744073709551615UL ) /* 64 bits */
        sorting_sampling_table[10].max_nmemb?5UL:15UL
#else
# error "strange SIZE_MAX " SIZE_MAX
#endif /* word size */
    ; /* starting point; refined by sample_index() */

    nfrozen=0UL, pivot_minrank=nmemb;
    wqsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        pk,0UL,nk,table_index,0UL,options,0);

#if DEBUG_CODE
    if (DEBUGGING(WQSORT_DEBUG)&&DEBUGGING(RATIO_GRAPH_DEBUG)) {
        /* for graphing worst-case partition ratios */
        size_t q;
        for (q=0UL; q<(SAMPLING_TABLE_SIZE); q++) {
            if (0UL<stats_table[q].max_ratio)
                (V)fprintf(stderr,
                    "/* %s: table_index=%lu, max_ratio=%lu, repivots=%lu@%lu */\n",
                    __func__,q,stats_table[q].max_ratio,
                    stats_table[q].repivots,stats_table[q].repivot_ratio);
        }
    }
#endif
}
