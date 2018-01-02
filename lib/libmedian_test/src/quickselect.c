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
/* $Id: ~|^` @(#)   This is quickselect.c version 1.10 dated 2017-12-19T02:32:12Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.quickselect.c */

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
#define ID_STRING_PREFIX "$Id: quickselect.c ~|^` @(#)"
#define SOURCE_MODULE "quickselect.c"
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2017-12-19T02:32:12Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

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

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void repivot_factors(unsigned int repivot_table_index, const size_t *pk,
    unsigned char *pf1, unsigned char *pf2)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
if (DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: repivot_table_index=%u */\n",
__func__,source_file,__LINE__,repivot_table_index);
#endif /* DEBUG_CODE */
    if ((repivot_table_index>repivot_table_size)||(0U==repivot_table_index)) {
        *pf1 = 3U, *pf2 = 3U;
    } else {
	A(0U<repivot_table_index);
        repivot_table_index--; /* repivot table has no entry for single sample */
        if (NULL!=pk) { /* selection */
            *pf1 = selection_repivot_table[repivot_table_index].factor1;
            *pf2 = selection_repivot_table[repivot_table_index].factor2;
        } else { /* sorting */
            *pf1 = sorting_repivot_table[repivot_table_index].factor1;
            *pf2 = sorting_repivot_table[repivot_table_index].factor2;
        }
    }
#if DEBUG_CODE
if (DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: factor1=%u, factor2=%u */\n",
__func__,source_file,__LINE__,*pf1,*pf2);
#endif /* DEBUG_CODE */
}

/* Special-case selection of minimum, maximum, or both. */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void select_min(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, char **ppeq, char **ppgt)
{
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)
    ||DEBUGGING(REPARTITION_DEBUG)
    ||DEBUGGING(REPIVOT_DEBUG)
    )
        (V) fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, ppeq=%p, ppgt=%p */\n",
            __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(void *)ppeq,(void *)ppgt);
#endif
    if (beyond>first+1UL) {
        char *mn, *p, *q, *r;
        mn=p=base+first*size,q=p+size,r=base+beyond*size;
        if ((NULL==ppeq)&&(NULL==ppgt)) { /* non-partitioning */
            /* separate indirect/direct versions of loop to avoid repeated
               option testing in loop, cache dereferenced mn
            */
#if QUICKSELECT_INDIRECT
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                char *x=*((char **)mn);
                for (; q<r; q+=size)
                    if (0<COMPAR(x,*((char **)q),/**/)) x=*((char **)(mn=q));
            } else
#endif /* QUICKSELECT_INDIRECT */
                for (; q<r; q+=size)
                    if (0<COMPAR(mn,q,/**/)) mn=q;
            if (mn!=p) {
#if QUICKSELECT_STABLE
                if (0U==((QUICKSELECT_STABLE)&options)) {
#endif /* QUICKSELECT_STABLE */
                    EXCHANGE_SWAP(swapf,p,mn,size,alignsize,size_ratio,nsw++);
#if QUICKSELECT_STABLE
                } else {
                    /* |      mn      | */
                    protate(p,mn-size,mn,size,swapf,alignsize,size_ratio);
                }
#endif /* QUICKSELECT_STABLE */
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            /* separate indirect/direct versions of loop to avoid repeated
               option testing in loop, cache dereferenced mx
            */
#if QUICKSELECT_INDIRECT
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                char *x=*((char **)mn);
                for (; q<r; q+=size) {
                    int c=COMPAR(x,*((char **)q),/**/);
                    if (0>c) continue;
                    if (0==c) {
                        mn+=size, x=*((char **)mn);
                        if (q==mn) continue;
                    } else /* 0<c */ x=*((char **)(mn=p)); /* new min */
                    A(mn!=q);
                    EXCHANGE_SWAP(swapf,mn,q,size,alignsize,size_ratio,nsw++);
                }
            } else
#endif /* QUICKSELECT_INDIRECT */
                for (; q<r; q+=size) {
                    int c=COMPAR(mn,q,/**/);
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
#if DEBUG_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG)
            ||DEBUGGING(REPARTITION_DEBUG)
            ||DEBUGGING(REPIVOT_DEBUG)
            ) {
                (V) fprintf(stderr,
                    "/* %s: %s line %d: first=%lu, beyond=%lu, ppeq=%p@%lu, ppgt=%p@%lu */\n",
                    __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(void *)ppeq,(p-base)/size,
                    (void *)ppgt,(mn+size-base)/size);
                print_some_array(base,first,beyond-1UL, "/* "," */",options);
            }
#endif
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
void select_max(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, char **ppeq, char **ppgt)
{
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)
    ||DEBUGGING(REPARTITION_DEBUG)
    ||DEBUGGING(REPIVOT_DEBUG)
    )
        (V) fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, ppeq=%p, ppgt=%p */\n",
            __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(void *)ppeq,(void *)ppgt);
#endif
    if (beyond>first+1UL) {
        char *mx, *p, *r;
        p=base+first*size,mx=r=base+(beyond-1UL)*size;
        if ((NULL==ppeq)&&(NULL==ppgt)) { /* non-partitioning */
            /* separate indirect/direct versions of loop to avoid repeated
               option testing in loop, cache dereferenced mx
            */
#if QUICKSELECT_INDIRECT
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                char *x=*((char **)mx);
                for (; p<r; p+=size)
                    if (0>COMPAR(x,*((char **)p),/**/)) x=*((char **)(mx=p));
            } else
#endif /* QUICKSELECT_INDIRECT */
                for (; p<r; p+=size)
                    if (0>COMPAR(mx,p,/**/)) mx=p;
            if (mx!=r) {
#if QUICKSELECT_STABLE
                if (0U==((QUICKSELECT_STABLE)&options)) {
#endif /* QUICKSELECT_STABLE */
                    EXCHANGE_SWAP(swapf,r,mx,size,alignsize,size_ratio,nsw++);
#if QUICKSELECT_STABLE
                } else {
                    /* |      mx      | */
                    protate(mx,mx+size,r,size,swapf,alignsize,size_ratio);
                }
#endif /* QUICKSELECT_STABLE */
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            char *q;
            /* separate indirect/direct versions of loop to avoid repeated
               option testing in loop, cache dereferenced mn
            */
#if QUICKSELECT_INDIRECT
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                char *x=*((char **)mx);
                for (q=r-size; q>=p; q-=size) {
                    int c=COMPAR(x,*((char **)q),/**/);
                    if (0<c) continue;
                    if (0==c) {
                        mx-=size, x=*((char **)mx);
                        if (q==mx) continue;
                    } else /* 0>c */ x=*((char **)(mx=r)); /* new max */
                    A(mx!=q);
                    EXCHANGE_SWAP(swapf,mx,q,size,alignsize,size_ratio,nsw++);
                }
            } else
#endif /* QUICKSELECT_INDIRECT */
                for (q=r-size; q>=p; q-=size) {
                    int c=COMPAR(mx,q,/**/);
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
#if DEBUG_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG)
            ||DEBUGGING(REPARTITION_DEBUG)
            ||DEBUGGING(REPIVOT_DEBUG)
            ) {
                (V) fprintf(stderr,
                    "/* %s: %s line %d: first=%lu, beyond=%lu, ppeq=%p@%lu, ppgt=%p@%lu */\n",
                    __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(void *)ppeq,(mx-base)/size,
                    (void *)ppgt,(r+size-base)/size);
                print_some_array(base,first,beyond-1UL, "/* "," */",options);
            }
#endif
        }
    } else { /* beyond==first+1UL; 1 element */
        if (NULL!=ppeq) *ppeq=base+first*size;
        if (NULL!=ppgt) *ppgt=base+beyond*size;
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void d_klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *pfk, size_t *pbk)
{
    size_t s, l, lk, rk;

    A(NULL!=pfk);A(NULL!=pbk);A(NULL!=pk);
    /* binary search through pk to find limits for each region */
    for (s=firstk,l=beyondk,lk=s+((l-s)>>1); s<l; lk=s+((l-s)>>1)) {
        A(lk>=firstk);A(lk<=beyondk);
        if (pk[lk]<first) s=lk+1UL; else l=lk;
    }
    A(lk>=firstk);A(lk<=beyondk);
    for (s=lk,l=beyondk,rk=s+((l-s)>>1); s<l; rk=s+((l-s)>>1)) {
        A(rk>=firstk);A(rk<=beyondk);
        if (pk[rk]<beyond) s=rk+1UL; else l=rk;
    }
#if DEBUG_CODE
    if (DEBUGGING(SAMPLING_DEBUG)
    ||DEBUGGING(REPARTITION_DEBUG)
    ||DEBUGGING(REPIVOT_DEBUG)
    ) {
        if (rk>lk) (V)fprintf(stderr,
"/* %s: %s line %d: firstk=%lu, lk=%lu, pk[%lu]=%lu, rk=%lu, pk[%lu]=%lu, beyondk=%lu, first=%lu, beyond=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)firstk,(unsigned long)lk,(unsigned long)lk,(unsigned long)(pk[lk]),(unsigned long)rk,(unsigned long)rk-1UL,(unsigned long)(pk[rk-1UL]),(unsigned long)beyondk,(unsigned long)first,(unsigned long)beyond);
        else (V)fprintf(stderr,
"/* %s: %s line %d: firstk=%lu, pk[%lu]=%lu, lk=%lu, rk=%lu, beyondk=%lu, first=%lu, beyond=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)firstk,(unsigned long)firstk,(unsigned long)pk[firstk],(unsigned long)lk,(unsigned long)rk,(unsigned long)beyondk,(unsigned long)first,(unsigned long)beyond);
    }
#endif
    *pfk=lk, *pbk=rk;
}

#ifndef SAMPLING_TABLE_SIZE
# error "SAMPLING_TABLE_SIZE is not defined"
#elsif ! SAMPLING_TABLE_SIZE
# error "SAMPLING_TABLE_SIZE is 0"
#endif

/* sample_index: return index into sampling table psts for nmemb */
/* called from sampling_table and quicksort */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned int d_sample_index(struct sampling_table_struct *psts,
    unsigned int idx, size_t nmemb)
{
#if DEBUG_CODE
if (DEBUGGING(SAMPLING_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, idx=%u */\n",
__func__,source_file,__LINE__,(unsigned long)nmemb,idx);
#endif
    if (3UL>nmemb) idx=0U;
    else {
        A((psts==sorting_sampling_table)
        ||(psts==ends_sampling_table)
        ||(psts==middle_sampling_table)
        );
        A(idx <= (SAMPLING_TABLE_SIZE)-1U);
        A(nmemb <= psts[SAMPLING_TABLE_SIZE-1U].max_nmemb);
        while (nmemb>psts[idx].max_nmemb) { idx++; }
        A(0U<idx);
        while (nmemb<=psts[idx-1U].max_nmemb) { idx--; A(0U<idx); }
    }
#if DEBUG_CODE
if (DEBUGGING(SAMPLING_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, index=%u, max_nmemb=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)nmemb,idx,(unsigned long)(psts[idx].max_nmemb));
if (idx>=SAMPLING_TABLE_SIZE) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, index=%u */\n",
__func__,source_file,__LINE__,(unsigned long)nmemb,idx);
#endif
    A(idx<(SAMPLING_TABLE_SIZE));
    return idx;
}

/* sampling_table: select a suitable sampling table */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
struct sampling_table_struct *d_sampling_table(size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk, char **ppeq,
    unsigned int *psort, unsigned int *pindex, size_t nmemb)
{
    unsigned char raw, distribution, sort=1U;
    struct sampling_table_struct *psts=sorting_sampling_table;

    A(NULL!=pindex);
    if (NULL!=pk) { /* selection, not sorting */
        size_t bk, fk, m1, m2, nk, nr, x;
#if CONSOLIDATED_TABLES > 1
        size_t qk;
#endif
        A(beyondk>=firstk);
        nk=beyondk-firstk;
        raw=0U; /* raw 000 */
        /* median(s): lower(m1) and upper (m2) */
        m1=first+((nmemb-1UL)>>1); m2=first+(nmemb>>1);
#if CONSOLIDATED_TABLES > 1
        x=(nmemb>>3)+1UL; /* nmemb/8 + 1 */
#else
        if (16UL<=nk) x=(nmemb>>5)+1UL; /* nmemb/32 + 1 */
        else if (1UL<nk) x=(nmemb>>1)/nk+1UL; /* nmemb/2/nk + 1 */
        else x=(nmemb>>2)+1UL; /* nmemb/4 + 1 */
#endif
#if DEBUG_CODE
        if (DEBUGGING(SAMPLING_DEBUG)
        ||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(REPIVOT_DEBUG)
        )
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, nk=%lu, x(%s)=%lu, first=%lu, m1=%lu, m2=%lu, beyond=%lu, firstk=%lu, beyondk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,(unsigned long)nmemb,(unsigned long)nk,
                16UL<=nk?"nmemb/32+1":1UL<nk?"nmemb/2/nk+1":"nmemb/4+1",
                (unsigned long)x,(unsigned long)first,(unsigned long)m1,(unsigned long)m2,(unsigned long)beyond,(unsigned long)firstk,(unsigned long)beyondk,
                (unsigned long)firstk,(unsigned long)pk[firstk],(unsigned long)beyondk-1UL,(unsigned long)pk[beyondk-1UL]);
#endif
#if CONSOLIDATED_TABLES <= 1
        d_klimits(first,first+x,pk,firstk,beyondk,&fk,&bk);
        if (bk>fk) raw|=4U;
#endif
        d_klimits(m1-x,m2+x,pk,firstk,beyondk,&fk,&bk);
        nr = ((bk>fk)?bk-fk:0UL); /* ranks in [3/8-,5/8+) */
        if (0UL!=nr) raw|=2U;
#if CONSOLIDATED_TABLES > 1
        qk=nk>>2; /* 1/4 */
        if ((1UL<nk)&&(qk<=nr+1UL)&&(nr<=qk+1UL)) {
            /* nr OK for distributed ranks; check for ends,separated */
            d_klimits(first,m2+x,pk,firstk,beyondk,&fk,&bk);
            qk = ((bk>fk)?bk-fk:0UL); /* ranks in [0,3/8-) */
            if (0UL!=qk) raw|=4U;
            nr += qk; /* now ranks in [0,5/8+) */
            qk=(nk>>1)+(nk>>3); /* 5/8 */
            if ((qk<=nr+1UL)&&(nr<=qk+1UL))
                raw|=1U;
        }
#else
        d_klimits(beyond-x,beyond,pk,firstk,beyondk,&fk,&bk);
        if (bk>fk) raw|=1U;
#endif
#if DEBUG_CODE
        if (DEBUGGING(SAMPLING_DEBUG)
        ||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(REPIVOT_DEBUG)
        )
            (V)fprintf(stderr,
                "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, raw distribution=%u, ppeq=%p, firstk=%lu, beyondk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(unsigned long)nmemb,raw,
                (void *)ppeq,(unsigned long)firstk,(unsigned long)beyondk,(unsigned long)firstk,(unsigned long)pk[firstk],(unsigned long)beyondk-1UL,
                (unsigned long)pk[beyondk-1UL]);
#endif
        /* convert raw distribution to enumerated value */
        distribution=sampling_distribution_remap[raw];
#if DEBUG_CODE
        if (DEBUGGING(SAMPLING_DEBUG)
        ||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(REPIVOT_DEBUG)
        )
            (V)fprintf(stderr,
                "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, remapped distribution=%u, ppeq=%p */\n",
                __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(unsigned long)nmemb,distribution,(void *)ppeq);
#endif
#if DEBUG_CODE
        if (DEBUGGING(SAMPLING_DEBUG)
        ||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(REPIVOT_DEBUG)
        )
            (V)fprintf(stderr,
                "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, distribution=%u, ppeq=%p */\n",
                __func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,(unsigned long)nmemb,distribution,(void *)ppeq);
#endif
        if (NULL==ppeq) { /* sorting permitted (extents not required) */
            /* Worst-case sort of 3 elements costs 3 comparisons and at most
               2 swaps.  Worst-case selection of minimum or maximum of 3
               elements costs 2 comparisons and at most 1 swap.
               Sorting of 3 elements (i.e. selection of both minimum and
               maximum) costs at most 3 comparisons and at most
               2 swaps -- the same as dedicated sorting.
               Therefore, sorting is only advantageous over selection for at
               least 4 elements (depending on the distribution of the
               desired ranks).
            */
            if (0!=strict_selection) sort=0U;
            else
            if (SELECTION_BREAKPOINT_OFFSET<=nmemb) { /* possibly select */
                if ((SELECTION_BREAKPOINT_TABLE_MAX_NMEMB)>=nmemb) {
                    /* table breakpoints determine sorting vs. selection */
                    if (selection_breakpoint
                        [nmemb-SELECTION_BREAKPOINT_OFFSET]
                        [selection_distribution_remap[raw]]
                    >=nk)
                        sort=0U;
                } else { /* large sub-arrays */
                    /* 1/8, 15/16 sorting vs. selection rules of thumb */
                    if (7U==raw) { /* distributed ranks */
                        if (nk<=(nmemb>>3)) sort=0U; /* <= nmemb/8 ranks */
                    } else { /* middle, extended, separated, ends */
                        if (nk<=nmemb-(nmemb>>4)) sort=0U; /* <=15/16 nmemb */
                    }
                }
            } /* sort/select equivalent @2 elements (extents irrelevant) */
        } else {
            /* no sort if = region extents are required (save_partial) */
            /* Sorting is not possible because the pivot for sorting is not
               necessarily the desired oder statistic rank element.
            */
            sort=0U;
        }
#if DEBUG_CODE
        if (DEBUGGING(SAMPLING_DEBUG)
        ||DEBUGGING(REPARTITION_DEBUG)
        ||DEBUGGING(REPIVOT_DEBUG)
        )
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, distribution=%u */\n",
                __func__,source_file,__LINE__,(unsigned long)nmemb,distribution);
#endif
    } /* else sorting */
#if DEBUG_CODE
    if (DEBUGGING(SAMPLING_DEBUG)
    ||DEBUGGING(REPARTITION_DEBUG)
    ||DEBUGGING(REPIVOT_DEBUG)
    )
        (V)fprintf(stderr,
            "/* %s: %s line %d: nmemb=%lu, sort=%u */\n",
            __func__,source_file,__LINE__,(unsigned long)nmemb,sort);
#endif
    if (NULL!=psort) *psort=(unsigned int)sort;
    /* Select sampling table based on whether sorting or selecting, and if the
       latter, on the distribution of the desired order statistic ranks.
       If sort!=1U (i.e. it has been reset to 0U), distribution has been set.
    */
    A((0U!=sort)||((SAMPLING_TABLES)>distribution));
    if (0U==sort) psts=sampling_tables[distribution];
#if ASSERT_CODE
    if ((psts!=sorting_sampling_table)
    &&(psts!=ends_sampling_table)
    &&(psts!=middle_sampling_table)
    ) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: nmemb=%lu, pk=%p, firstk=%lu, beyondk=%lu, sort=%u, distribution=%u */\n",
            __func__,source_file,__LINE__,(unsigned long)nmemb,(const void *)pk,(unsigned long)firstk,(unsigned long)beyondk,sort,
            distribution);
    }
#endif
    A((psts==sorting_sampling_table)
    ||(psts==ends_sampling_table)
    ||(psts==middle_sampling_table)
    );
    A(*pindex < (SAMPLING_TABLE_SIZE));
    *pindex=d_sample_index(psts, *pindex, nmemb);
#if DEBUG_CODE
    if (DEBUGGING(SAMPLING_DEBUG)
    ||DEBUGGING(REPARTITION_DEBUG)
    ||DEBUGGING(REPIVOT_DEBUG)
    )
        if (NULL!=pindex)
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, index=%u, max_nmemb=%lu */\n",
                __func__,source_file,__LINE__,(unsigned long)nmemb,*pindex,psts[*pindex].max_nmemb);
#endif
    return psts;
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
unsigned int d_should_repivot(size_t nmemb, size_t n, size_t cutoff,
    struct sampling_table_struct *pst, unsigned int table_index,
    const size_t *pk, int *pn2, char **ppeq)
{
    size_t q, ratio;
    unsigned char factor1, factor2;

    A((SAMPLING_TABLE_SIZE)>table_index);
    if (NULL==pk) { /* sorting */
        /* cutoff+3UL presumes 3 samples for pivot selection */
        if ((8UL>=n)||(cutoff+3UL>n)) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, cutoff=%lu, pk=%p, no repivot */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,cutoff,(const void *)pk);
#endif
            return 0U;
        }
    } else { /* order statistic selection */
        if (8UL>=n) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, pk=%p, no repivot */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,(const void *)pk);
#endif
            return 0U; /* no repivot unless >= 3 medians-of-3 */
        }
    }
    if (n>=nmemb) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||(DEBUGGING(SHOULD_REPIVOT_DEBUG))||(n>nmemb)) (V)fprintf(stderr,
"/* %s: %s line %d: ERROR: nmemb=%lu <= n=%lu, pk=%p, repivoting */\n",
__func__,source_file,__LINE__,nmemb,n,(const void *)pk);
#endif
        ++*pn2;
        return QUICKSELECT_RESTRICT_RANK;
    }
    q=nmemb-n;
    A(0UL<q);
    if (n<=q) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, pk=%p, ppeq=%p, no repivot */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,(const void *)pk,(void *)ppeq);
#endif
        return 0U; /* ratio 0; return w/o incurring division cost */
    }
    /* if sorting, check n vs. cutoff with actual sampling table samples */
    if ((NULL==pk)&&(0U<table_index)&&(n<=cutoff+pst[table_index-1U].samples+1UL)) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, cutoff=%lu, pk=%p, no ppeq=%p, no repivot */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,cutoff,(const void *)pk,(void *)ppeq);
#endif
        return 0U; /* next regions handled by dedicated sort w/o repivot; return w/o incurring ratio division cost */
    }
    ratio=n/q;
    if ((NULL==ppeq)&&(DEBUGGING(RATIO_GRAPH_DEBUG))) /* for graphing worst-case partition ratios */
        if (ratio>stats_table[table_index].max_ratio) stats_table[table_index].max_ratio = ratio;
    /* aux tests, if not disabled */
    /* should be disabled for time vs. factor,limit graph */
    if (0==no_aux_repivot) {
        repivot_factors(table_index,(const void *)pk,&factor1,&factor2);
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(PARTITION_ANALYSIS_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, ratio>=%lu, repivot_factor1=%u, repivot_factor2=%u, pk=%p, ppeq=%p */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,ratio,factor1,factor2,(const void *)pk,(void *)ppeq);
#endif
        /* size-dependent count 1 limit test */
        if (ratio>=factor1) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(PARTITION_ANALYSIS_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, ratio>=%lu>=repivot_factor1=%u, pk=%p, ppeq=%p, repivoting */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,ratio,factor1,(const void *)pk,(void *)ppeq);
#endif
            if ((NULL==ppeq)&&(DEBUGGING(RATIO_GRAPH_DEBUG))) { /* for graphing worst-case partition ratios */
                stats_table[table_index].repivots++;
                stats_table[table_index].repivot_ratio = ratio;
            }
            return QUICKSELECT_RESTRICT_RANK;
        }
        /* size-dependent count 2 limit test */
        if (ratio>=factor2) {
            ++*pn2;
            if (*pn2>=2) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(PARTITION_ANALYSIS_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, pk=%p, ppeq=%p, ratio>=%lu>=factor2=%u, *pn2=%d, limit=%d, repivoting */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,(const void *)pk,(void *)ppeq,ratio,factor2,*pn2,2);
#endif
                if ((NULL==ppeq)&&(DEBUGGING(RATIO_GRAPH_DEBUG))) { /* for graphing worst-case partition ratios */
                    stats_table[table_index].repivots++;
                    stats_table[table_index].repivot_ratio = ratio;
                }
                return QUICKSELECT_RESTRICT_RANK;
            }
        }
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, pk=%p, ppeq=%p, ratio>=%lu<factor2=%u or *pn2=%d<2, limit=%d, no repivot */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,(const void *)pk,(void *)ppeq,ratio,factor2,*pn2,2);
#endif
    } else {
        /* global limit test */
        if (ratio>=repivot_factor) {
            if (0!=no_aux_repivot) {
                /* need limit here for time vs factor,limit graph */
                ++*pn2; /* use count for factor2 */
                if (*pn2>=lopsided_partition_limit) {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, ratio>=%lu>=repivot_factor=%lu, pk=%p, *pn2=%d, limit=%d, repivoting */\n",
__func__,source_file,__LINE__,nmemb,n,ratio,repivot_factor,(const void *)pk,*pn2,lopsided_partition_limit);
#endif
                    if ((NULL==ppeq)&&(DEBUGGING(RATIO_GRAPH_DEBUG))) { /* for graphing worst-case partition ratios */
                        stats_table[table_index].repivots++;
                        stats_table[table_index].repivot_ratio = ratio;
                    }
                    return QUICKSELECT_RESTRICT_RANK;
                }
            } else {
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, ratio>=%lu>=repivot_factor=%lu, pk=%p, repivoting */\n",
__func__,source_file,__LINE__,nmemb,n,ratio,repivot_factor,(const void *)pk);
#endif
                if ((NULL==ppeq)&&(DEBUGGING(RATIO_GRAPH_DEBUG))) { /* for graphing worst-case partition ratios */
                    stats_table[table_index].repivots++;
                    stats_table[table_index].repivot_ratio = ratio;
                }
                return QUICKSELECT_RESTRICT_RANK;
            }
        }
    }
#if DEBUG_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, n=%lu, samples=%lu, table_index=%u, pk=%p, no repivot */\n",
__func__,source_file,__LINE__,nmemb,n,pst[table_index].samples,table_index,(const void *)pk);
#endif
    return 0U;
}

/* function to consolidate logic and calls to special-case selection functions */
/* also sets sampling table and index, determines whether to sort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int special_cases(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void*,const void *),
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
#if DEBUG_CODE
if ((fk<first)||(ek>=beyond)) (V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, pk=%p, firstk=%lu, beyondk=%lu, fk=%lu, ek=%lu */\n",
__func__,source_file,__LINE__,first,beyond,(const void *)pk,firstk,beyondk,fk,ek);
#endif
            A(fk>=first);A(ek<beyond); /* arg sanity */
            A((beyondk==firstk+1UL)||(fk!=ek)); /* no duplicate rank requests */
            switch (nk) {
                case 1UL : /* any of smallest 2 or largest 2 */
                    if ((fk==first)||(fk==sp)) {
                        select_min(base,first,beyond,size,
                            compar,swapf,alignsize,size_ratio,options,ppeq,ppgt);
                        if (fk==sp) {
                            select_min(base,sp,beyond,size,
                                compar,swapf,alignsize,size_ratio,options,ppeq,ppgt);
                            if ((NULL!=ppeq/*)&&(NULL!=ppgt)*/)
                            &&(0==OPT_COMPAR(*ppeq-size,*ppeq,options,/**/)))
                                (*ppeq)-=size;
                        }
                        return 1 ;
                    } else if ((ek==lp)||(ek==pp)) {
                        select_max(base,first,beyond,size,
                            compar,swapf,alignsize,size_ratio,options,ppeq,ppgt);
                        if (ek==pp) {
                            select_max(base,first,lp,size,
                                compar,swapf,alignsize,size_ratio,options,ppeq,ppgt);
                            if ((NULL!=ppeq/*)&&(NULL!=ppgt)*/)
                            &&(0==OPT_COMPAR(*ppeq,*ppgt,options,/**/)))
                                (*ppgt)+=size;
                        }
                        return 1 ;
                    }
                break;
                case 2UL : /* 2 smallest, 2 largest, or smallest & largest */
                    A(NULL==ppeq);
                    if (fk==first) {
                        if (ek==lp) { /* smallest & largest */
                            select_minmax(base,first,beyond,size,
                                compar,swapf,alignsize,size_ratio,options);
                            return 1 ;
                        } else if (ek==sp) { /* 2 smallest */
                            select_min(base,first,beyond,size,
                                compar,swapf,alignsize,size_ratio,options,NULL,NULL);
                            select_min(base,sp,beyond,size,
                                compar,swapf,alignsize,size_ratio,options,NULL,NULL);
                            return 1 ;
                        }
                    } else if ((fk==pp)&&(ek==lp)) { /* 2 largest */
                        select_max(base,first,beyond,size,
                            compar,swapf,alignsize,size_ratio,options,NULL,NULL);
                        select_max(base,first,lp,size,
                            compar,swapf,alignsize,size_ratio,options,NULL,NULL);
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
    d_klimits(p,q,pk,firstk,beyondk,plk,prk);
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

/* basic quickselect loop (using system stack) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void d_quickselect_loop(char *base, size_t first, size_t beyond,
    const size_t size, int(*compar)(const void *,const void *),
    const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, unsigned int options, char *conditions,
    size_t *indices, char *element, char **ppeq, char **ppgt)
{
    auto int c=0;
    unsigned int table_index=2U;

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(REPIVOT_DEBUG))
        (V) fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, pk=%p, firstk=%lu, beyondk=%lu, ppeq=%p, ppgt=%p */\n",
            __func__,source_file,__LINE__,first,beyond,(const void *)pk,firstk,beyondk,
            (void *)ppeq,(void *)ppgt);
    A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
#endif
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
        if (NULL!=pk)
            (V) fprintf(stderr,
                "/* %s: %s line %d: first=%lu, beyond=%lu, pk=%p, firstk=%lu, beyondk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,first,beyond,(const void *)pk,firstk,beyondk,
                firstk,pk[firstk],beyondk-1UL,pk[beyondk-1UL]);
        else
            (V) fprintf(stderr,
                "/* %s: %s line %d: first=%lu, beyond=%lu, pk=NULL */\n",
                __func__,source_file,__LINE__,first,beyond);
    }
#endif
    if (0U!=((QUICKSELECT_RESTRICT_RANK)&options)) {
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: repivot initially requested: first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu, options=0x%x */\n",
__func__,source_file, __LINE__,first,beyond,firstk,beyondk,options);
#endif
    }
    for(;;) {
        unsigned int sort;
        char *pc, *pd, *pe, *pf, *pivot;
        size_t nmemb;
        struct region_struct lt_region, gt_region, *ps_region, *pl_region;
        struct sampling_table_struct *pst;

        A(first<beyond);
        nmemb=beyond-first;
        A(2UL<=nmemb);
if (NULL!=pk) {
A(beyondk>firstk);
A(first<=pk[firstk]);
A(pk[beyondk-1UL]<beyond);
}
        /* Check for special-case selection: 1-2 order statistic ranks */
        A(table_index < (SAMPLING_TABLE_SIZE));
        if (0!=special_cases(base,first,beyond,size,compar,swapf,alignsize,
           size_ratio,cutoff,nmemb,pk,firstk,beyondk,options,ppeq,ppgt,&pst,&sort,
           &table_index))
           return ;
        A((SAMPLING_TABLE_SIZE)>table_index);
        A((0U==table_index)||(nmemb>pst[table_index-1U].max_nmemb));

#if ASSERT_CODE
        if (nmemb>pst[table_index].max_nmemb)
            (V)fprintf(stderr,
                "%s: %s line %d: nmemb=%lu, table_index=%u, pst[%u].max_nmemb=%lu */\n",
                __func__,source_file,__LINE__,nmemb,table_index,table_index,
                pst[table_index].max_nmemb);
        A(pst[table_index].max_nmemb>=nmemb);
        if ((0U<table_index)&&(nmemb<=pst[table_index-1U].max_nmemb))
            (V)fprintf(stderr,
                "%s: %s line %d: nmemb=%lu, table_index=%u, pst[%u].max_nmemb=%lu */\n",
                __func__,source_file,__LINE__,nmemb,table_index,table_index-1U,
                pst[table_index-1U].max_nmemb);
        A((0U==table_index)||(pst[table_index-1U].max_nmemb<nmemb));
#endif

        if (0U!=sort) { /* switch to sorting */
            if (nmemb<=cutoff) {
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(PARTITION_DEBUG))
                    (V) fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, pk=%p, firstk=%lu, beyondk=%lu, ppeq=%p, ppgt=%p, sort=%u, size_ratio=%lu, cutoff=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,
                        (const void *)pk,firstk,beyondk,
                        (void *)ppeq,(void *)ppgt,sort,
                        (unsigned long)size_ratio, (unsigned long)cutoff);
#endif
                A(1UL<nmemb);
                d_dedicated_sort(base,first,beyond,size,compar,swapf,alignsize,
                    size_ratio,options);
                return ; /* done */
            }
            pk=NULL; /* sorting */ A(pst==sorting_sampling_table);
            /* proceed with pivot selection */
        }

        /* antiqsort handshake */
        /* freeze low-address samples which will be used for pivot selection */
#if 0
        if (NULL!=ppeq) {
            pivot_minrank=0UL; /* no freeze while selecting median of medians */
        } else
#endif
            if (aqcmp==compar)
                (V)freeze_some_samples(base,first,beyond,size,compar,swapf,alignsize,
                    size_ratio,table_index,options);
        /* select a pivot element */
        pivot=d_select_pivot(base,first,beyond,size,compar,swapf,alignsize,
            size_ratio,table_index,pk,options,&pc,&pd,&pe,&pf);
#if ASSERT_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr, "/* %s: %s line %d: pivot=%p(%d), pl=%p, pu=%p */\n",
        __func__,source_file,__LINE__,(void *)pivot,*((int *)pivot),
        base+first*size,base+beyond*size);
    print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#endif
        A(NULL!=pivot);A(base+first*size<=pivot);A(pivot<base+beyond*size);
        A(pc<=pd);A(pd<pe);A(pe<=pf);A(pd<=pivot);A(pivot<pe);
        pivot_minrank=nmemb;

        /* Partition the array around the pivot element into less-than,
           equal-to, and greater-than (w.r.t. the pivot) regions.  The
           equal-to region requires no further processing.
        */
        lt_region.first=first, gt_region.beyond=beyond;
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,
            compar,swapf,alignsize,size_ratio,options,
            conditions,indices,element,
            &(lt_region.beyond),&(gt_region.first));
        /* regions: < [first,p)
                    > [q,beyond)
        */

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
#if DEBUG_CODE
        if (DEBUGGING(PARTITION_DEBUG)) {
            (V)fprintf(stderr,
                "/* %s: %s line %d: small region from partition of nmemb=%lu, s=%lu, e=%lu, t=%lu, pk=%p, firstk=%lu, beyondk=%lu, ppeq=%p%s */\n",
                __func__,source_file, __LINE__,
                nmemb, ps_region->beyond-ps_region->first,
                gt_region.first-lt_region.beyond,
                pl_region->beyond-pl_region->first,
                (const void *)pk, ps_region->firstk, ps_region->beyondk,
                (void *)ppeq, (0U==ps_region->process)?" (ignored)":"");
        }
        if (((NULL==ppeq)||(DEBUGGING(PARTITION_DEBUG)))&&(DEBUGGING(PARTITION_ANALYSIS_DEBUG))) { /* for graphing ratios during sorting */
            size_t r=pl_region->beyond-pl_region->first;
            (V)fprintf(stderr, "/* %s: %s line %d: regions from partition of nmemb=%lu, s=%lu, e=%lu, t=%lu, table_index=%u, samples=%lu, pk=%p, ppeq=%p, firstk=%lu, beyondk=%lu, count=%d, ratio=%lu%s */\n",
                __func__, source_file, __LINE__,
                nmemb, ps_region->beyond-ps_region->first,
                gt_region.first-lt_region.beyond, r,
                table_index, pst[table_index].samples,
                (const void *)pk, (void *)ppeq,
                pl_region->firstk, pl_region->beyondk,
                c, r/(nmemb-r),(0U==pl_region->process)?" (ignored)":"");
        }
#endif
        if (0U!=pl_region->process) { /* Process large region. */
            if (0U!=ps_region->process) { /* Recurse on small region. */
                nrecursions++;
                d_quickselect_loop(base,ps_region->first,ps_region->beyond,size,
                    compar,pk,ps_region->firstk,ps_region->beyondk,swapf,
                    alignsize,size_ratio,cutoff,options,conditions,
                    indices,element,ppeq,ppgt);
            }
            /* Dedicated sort for large region? */
            if ((NULL==pk)&&(pl_region->beyond<=pl_region->first+cutoff)) {
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(PARTITION_DEBUG))
                    (V) fprintf(stderr,
                        "/* %s: %s line %d: pl_region->first=%lu, pl_region->beyond=%lu, cutoff=%lu, pk=%p, pl_region->firstk=%lu, pl_region->beyondk=%lu, ppeq=%p, ppgt=%p */\n",
                        __func__,source_file,__LINE__,pl_region->first,pl_region->beyond,
                        cutoff,(const void *)pk,pl_region->firstk,pl_region->beyondk,
                        (void *)ppeq,(void *)ppgt);
#endif
                A(pl_region->first+1UL<pl_region->beyond);
                d_dedicated_sort(base,pl_region->first,pl_region->beyond,size,
                    compar,swapf,alignsize,size_ratio,options);
                return ; /* done */
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
#if DEBUG_CODE
            if (0U!=((QUICKSELECT_RESTRICT_RANK)&options)) {
if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: restricted rank flag not reset: first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu, options=0x%x */\n",
__func__,source_file, __LINE__,first,beyond,firstk,beyondk,options);
            }
#endif
            sort=
                d_should_repivot(nmemb,beyond-first,cutoff,pst,table_index,
                pk,&c,ppeq);
#if DEBUG_CODE
            if (0U!=sort) {
if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: d_should_repivot returned %u: nmemb=%lu, first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu, options=0x%x */\n",
__func__,source_file, __LINE__,sort,nmemb,first,beyond,firstk,beyondk,options);
            }
#endif
            options |= sort;
            if (0U!=((QUICKSELECT_RESTRICT_RANK)&options)) {
                nrepivot++;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: repivoting region first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu, options=0x%x */\n",
__func__,source_file, __LINE__,first,beyond,firstk,beyondk,options);
#endif
            }

        } else { /* large region to be ignored; maybe iterate small region */
            if (0U!=ps_region->process) { /* Iterate on small region. */
                first=ps_region->first, beyond=ps_region->beyond,
                    firstk=ps_region->firstk, beyondk=ps_region->beyondk;
#if DEBUG_CODE
if (DEBUGGING(REPARTITION_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: small region: first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu: %s */\n",
__func__,source_file,__LINE__,first,beyond,firstk,beyondk,"continuing");
#endif
            } else {
#if DEBUG_CODE
if (DEBUGGING(REPARTITION_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: %s */\n",
__func__,source_file,__LINE__,"done");
#endif
                return ; /* Process neither; nothing left to do. */
            }
        }
#if DEBUG_CODE
if (DEBUGGING(REPARTITION_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, firstk=%lu, beyondk=%lu: %s */\n",
__func__,source_file,__LINE__,first,beyond,firstk,beyondk,"continuing");
#endif
    }
}

static int printed_swapper = 0;

/* quickselect definition (internal interface) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void quickselect_internal(char *base, size_t nmemb,
    /*const*/ size_t size, int (*compar)(const void *,const void *),
#if 0
    void (*swapf)(char *,char *,size_t),
#endif
    size_t *pk, size_t nk, unsigned int options, char **ppeq, char **ppgt)
{
    size_t onk=nk; /* original nk value */
    size_t s=0UL;  /* rotation amount */
    size_t alignsize, salignsize
#if SILENCE_WHINEY_COMPILERS
        =0UL
#endif
        , cutoff, scutoff
#if SILENCE_WHINEY_COMPILERS
        =0UL
#endif
        , size_ratio, sratio
#if SILENCE_WHINEY_COMPILERS
        =0UL
#endif
        , sz;
    void (*swapf)(char *, char *, size_t)=NULL;
    void (*sswapf)(char *, char *, size_t)
#if SILENCE_WHINEY_COMPILERS
        =NULL
#endif
        ;
    char *conditions = NULL;
    size_t *indices = NULL;
#if QUICKSELECT_INDIRECT + QUICKSELECT_LINEAR_STABLE
    char *element = NULL;
#endif /* QUICKSELECT_INDIRECT + QUICKSELECT_LINEAR_STABLE */
#if QUICKSELECT_INDIRECT
    char **pointers = NULL;
    void (*pswapf)(char *, char *, size_t);
#endif /* QUICKSELECT_INDIRECT */

    /* Initialization of strings is performed here (rather than in
       quickselect_loop) so that quickselect_loop can be made inline.
    */
    if ((char)0==file_initialized) initialize_file(__FILE__);

#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, size=%lu, pk=%p, nk=%lu, options=0X%X */\n",
__func__,source_file,__LINE__,nmemb,size,(const void *)pk,nk,
options);
#endif
    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || ((0UL!=nk) && (NULL==pk))
    || (0U!=(options&~(QUICKSELECT_USER_OPTIONS_MASK)))
    ) {
        errno=EINVAL;
        return ;
    }

    if (2UL>nmemb) return ; /* Return early if there's nothing to do. */
    /* Simplify tests for selection vs. sorting by ensuring a NULL pointer when
       sorting. Ensure consistency between pk and nk. Ensure sorted pk array
       with no duplicated order statistics.
    */
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        if (NULL!=pk)
            (V) fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=%p, nk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,nmemb,(const void *)pk,nk,
                0UL,pk[0],nk-1UL,pk[nk-1UL]);
        else
            (V) fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=NULL */\n",
                __func__,source_file,__LINE__,nmemb);
    }
#endif
    if (0UL==nk) pk=NULL;
    else if (NULL==pk) onk=nk=0UL;
    else if (1UL<nk) { /* binary search requires nondecreasing pk */
        size_t p, q;
        for (p=0UL,q=1UL; q<nk; p++,q++) /* verify (linear scan & compare) */
            if (pk[p]>pk[q]) break; /* not nondecreasing */

        /* Alignment, swapping function, cutoff for order statistics. */
        sz=sizeof(size_t);
        sswapf=swapn(salignsize=alignment_size((char *)pk,sz));
        /* sswapf is guaranteed to have been assigned a value by the above
           statement (and always if pk != NULL or nk > 1UL), whether or not
           (:-/) gcc's authors realize it...
        */
        sratio=sz/salignsize;
        /* sratio is guaranteed to have been assigned a value by the above
           statement (and always if pk != NULL or nk > 1UL), whether or not
           (:-/) gcc's authors realize it...
        */
        scutoff=cutoff_value(sratio,0x07F8U);

        if (q<nk) { /* fix (sort) iff broken (not nondecreasing) */
            d_quickselect_loop((char *)pk,0UL,nk,sz,size_tcmp,NULL,0UL,0UL,sswapf,
                salignsize,sratio,scutoff,0x07F8U,NULL,NULL,NULL,NULL,NULL);
        }
        /* verify, fix uniqueness */
        for (p=0UL,q=1UL; q<=nk; q++) { /* CLEARLY executed iff nk>0UL */
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    /* CLEARLY not executed if nk==1UL */
                    size_t r=q-p;
#if DEBUG_CODE
if (DEBUGGING(PK_ADJUSTMENT_DEBUG)) { (V)fprintf(stderr, "/* %s: %s line %d: %lu duplicate order statistic%s [%lu,%lu): */\n",
__func__,source_file,__LINE__,r,r==1?"":"s",p,q);
print_size_t_array(pk, 0UL, nk-1UL, "/* "," */");
}
#endif
                    irotate((char *)pk,p,q,nk,sz,sswapf,salignsize,sratio);
                    s=q-p, nk-=s, q=p;
#if DEBUG_CODE
if (DEBUGGING(PK_ADJUSTMENT_DEBUG)) { (V)fprintf(stderr, "/* %s: %s line %d: duplicate order statistic%s rotated to end: */\n",
__func__,source_file,__LINE__,r==1?"":"s");
print_size_t_array(pk, 0UL, nk-1UL, "/* "," */");
}
#endif
                }
            }
        }
    }
#if DEBUG_CODE
    if ((DEBUGGING(PK_ADJUSTMENT_DEBUG))&&(nk<onk)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: %lu (of %lu) unique order statistic rank%s, %lu duplicate%s */\n",
            __func__,source_file,__LINE__,
            nk,onk,(1UL==nk)?"":"s",onk-nk,(1UL==onk-nk)?"":"s");
    }
#endif /* DEBUG_CODE */

    /* base array alignment size and size_ratio */
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;

    /* Initialize the dedicated sorting cutoff based on array element size
       and alignment (swapping cost relative to comparisons).
    */
    if (0UL!=quickselect_small_array_cutoff)
        cutoff=quickselect_small_array_cutoff;
    else cutoff=cutoff_value(size_ratio,options);
    /* cutoff is guaranteed to have been assigned a value by the above statement
       whether or not (:-/) gcc's authors realize it...
    */
    if (2UL>cutoff) cutoff=2UL;

    /* Swap function based on alignment size */
    if (NULL==swapf) {
        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    }

#if QUICKSELECT_INDIRECT > QUICKSELECT_NETWORK_MASK
# error "some reasonable heuristic for deciding when to sort indirectly is needed"
    /* (N.B. Unreliable) example: Direct or indirect sorting based on nmemb and size_ratio. */
    if ((1UL<size_ratio)&&(7UL<nmemb+size_ratio)) {
        options|=(QUICKSELECT_INDIRECT);
    }
#endif /* QUICKSELECT_INDIRECT */

#if QUICKSELECT_INDIRECT
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
            if (p!=pointers) pointers=p;
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
                /* pswapf is guaranteed to have been assigned a value by the
                   above statement (and always if options contains
                   QUICKSELECT_INDIRECT after this block), whether or not (:-/)
                   gcc's authors realize it...
                */
                cutoff=cutoff_value(1UL,options);
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

    /* For sorting (but not selection), use dedicated sort if nmemb<=cutoff. */
    if ((NULL==pk)&&(nmemb<=cutoff)) {
#if QUICKSELECT_INDIRECT
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            d_dedicated_sort((char *)pointers,0UL,nmemb,sizeof(char *),compar,pswapf,
                alignsize,1UL,options);
            if (NULL==indices) indices=(size_t *)pointers;
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,element);
            free(element); element=NULL;
            free(pointers);
            if ((void *)indices==(void *)pointers) indices=NULL;
            if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort */
/* shouldn't happen */fprintf(stderr,"/* %s: %s line %d: size_ratio=%lu, nmemb=%lu, indirect sort failed */\n",
__func__,source_file,__LINE__,size_ratio,nmemb);
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                if (0U==instrumented) swapf=swapn(alignsize);
                else swapf=iswapn(alignsize);
                d_dedicated_sort(base,0UL,nmemb,size,compar,swapf,alignsize,
                    size_ratio,options);
            } else if (0UL!=alignsize)
                nmoves+=(alignsize+nmemb)*size_ratio;
        } else
#endif /* QUICKSELECT_INDIRECT */
            d_dedicated_sort(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio,options);
    } else {
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
            "/* %s: %s line %d: size=%lu, alignsize=%lu, size_ratio=%lu, cutoff=%lu */\n",
            __func__,source_file,__LINE__,(unsigned long)size,
            (unsigned long)alignsize,(unsigned long)size_ratio,
            (unsigned long)cutoff);
#endif
        if (DEBUGGING(RATIO_GRAPH_DEBUG)) { /* for graphing worst-case partition ratios */
            size_t q;
            for (q=0UL; q<(SAMPLING_TABLE_SIZE); q++)
                stats_table[q].max_ratio=stats_table[q].repivot_ratio=0UL,
                    stats_table[q].repivots=0UL;
        }

#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG)) {
            if (NULL!=pk)
                (V) fprintf(stderr,
                    "/* %s: %s line %d: nmemb=%lu, pk=%p, nk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                    __func__,source_file,__LINE__,nmemb,(const void *)pk,nk,
                    0UL,pk[0],nk-1UL,pk[nk-1UL]);
            else
                (V) fprintf(stderr,
                    "/* %s: %s line %d: nmemb=%lu, pk=NULL */\n",
                    __func__,source_file,__LINE__,nmemb);
        }
#endif
#if ASSERT_CODE
        if (NULL!=pk) {
            A(nk>0UL);
            A(0UL<=pk[0]);
#if DEBUG_CODE
            if (pk[nk-1]>=nmemb) (V) fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=%p, nk=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,nmemb,(const void *)pk,nk,nk-1UL,pk[nk-1]);
#endif
            A(pk[nk-1]<nmemb);
        }
#endif
#if QUICKSELECT_INDIRECT
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            d_quickselect_loop((char *)pointers,0UL,nmemb,sizeof(char *),compar,pk,0UL,
                nk,swapf,alignsize,1UL,cutoff,options,conditions,
                indices,element,NULL,NULL);
            if (NULL==indices) indices=(size_t *)pointers;
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,element);
            free(element); element=NULL;
            free(pointers);
            if ((void *)indices==(void *)pointers) indices=NULL;
            if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort/select */
/* shouldn't happen */fprintf(stderr,"/* %s: %s line %d: size_ratio=%lu, nmemb=%lu, indirect sort failed */\n",
__func__,source_file,__LINE__,size_ratio,nmemb);
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                if (0U==instrumented) swapf=swapn(alignsize);
                else swapf=iswapn(alignsize);
                cutoff=cutoff_value(size_ratio,options);
                d_quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,swapf,
                    alignsize,size_ratio,cutoff,options,conditions,
                    indices,element,NULL,NULL);
            } else if (0UL!=alignsize)
                nmoves+=(alignsize+nmemb)*size_ratio;
        } else
#endif /* QUICKSELECT_INDIRECT */
#if QUICKSELECT_LINEAR_STABLE
            d_quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,swapf,
                alignsize,size_ratio,cutoff,options,conditions,
                indices,element,NULL,NULL);
#else
            d_quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,swapf,
                alignsize,size_ratio,cutoff,options,NULL,
                NULL,NULL,NULL,NULL);
#endif /* QUICKSELECT_LINEAR_STABLE */

#if QUICKSELECT_LINEAR_STABLE
        if (0U!=(options&(QUICKSELECT_STABLE))) {
            if (NULL!=element) free(element);
            if (NULL!=conditions) free(conditions);
            if (NULL!=indices) free(indices);
        }
#endif /* QUICKSELECT_LINEAR_STABLE */

        if (DEBUGGING(RATIO_GRAPH_DEBUG)) { /* for graphing worst-case partition ratios */
            size_t q;
            for (q=0UL; q<(SAMPLING_TABLE_SIZE); q++) {
                if (0UL<stats_table[q].max_ratio)
                    (V)fprintf(stderr,
                        "%s: table_index=%lu, max_ratio=%lu, repivots=%lu@%lu */\n",
                        __func__,q,stats_table[q].max_ratio,
                        stats_table[q].repivots,stats_table[q].repivot_ratio);
            }
        }
    }

    /* Restore pk to full sorted (non-unique) order for caller convenience. */
    /* This may be expensive if the caller supplied a large number of duplicate
       order statistic ranks.  So be it.  Caller: if it hurts, don't do it.
    */
    if (0UL!=s) { /* there were duplicates */
        d_quickselect_loop((char *)pk,0UL,onk,sz,size_tcmp,NULL,0UL,0UL,sswapf,
            salignsize,sratio,scutoff,0x07F8U,NULL,NULL,NULL,NULL,NULL);
    }
}
