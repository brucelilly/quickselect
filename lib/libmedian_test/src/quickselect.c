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
* $Id: ~|^` @(#)    quickselect.c copyright 2016-2018 Bruce Lilly.   \ quickselect.c $
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
/* $Id: ~|^` @(#)   This is quickselect.c version 1.22 dated 2018-05-06T03:47:19Z. \ $ */
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
#define MODULE_VERSION "1.22"
#define MODULE_DATE "2018-05-06T03:47:19Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

#define QUICKSELECT_BUILD_FOR_SPEED 1 /* d_dedicated_sort is static */
#define DEDICATED_SORT d_dedicated_sort /* This is not a F'ing declaration, gcc authors! */
#define FIND_MINMAX_FUNCTION_NAME d_find_minmax
#define KLIMITS_FUNCTION_NAME d_klimits
#define PARTITION_FUNCTION_NAME d_partition
#define SAMPLE_INDEX_FUNCTION_NAME d_sample_index
#define SAMPLING_TABLE_FUNCTION_NAME d_sampling_table
#define SELECT_MIN_FUNCTION_NAME d_select_min
#define SELECT_MAX_FUNCTION_NAME d_select_max
#define SELECT_MINMAX_FUNCTION_NAME d_select_minmax
#define SHOULD_REPIVOT_FUNCTION_NAME d_should_repivot
#define QUICKSELECT_VISIBILITY extern

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes most other local and system header files required */
#include "indirect.h"           /* rearrange_array convert_pointers_to_indices */

#include "initialize_src.h"

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

/* repivot_factors is used by median_test main */
extern
void repivot_factors(unsigned int repivot_table_index, const size_t *pk,
    unsigned char *pf1, unsigned char *pf2)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
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
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
if (DEBUGGING(SHOULD_REPIVOT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: factor1=%u, factor2=%u */\n",
__func__,source_file,__LINE__,*pf1,*pf2);
#endif /* DEBUG_CODE */
}

#undef QUICKSELECT_BUILD_FOR_SPEED
#define QUICKSELECT_BUILD_FOR_SPEED 1
extern /* make it visible */
#include "klimits_src.h"

#ifndef SAMPLING_TABLE_SIZE
# error "SAMPLING_TABLE_SIZE is not defined"
#elsif ! SAMPLING_TABLE_SIZE
# error "SAMPLING_TABLE_SIZE is 0"
#endif

/* should_repivot source */
# include "repivot_src.h"

# undef QUICKSELECT_BUILD_FOR_SPEED
# define QUICKSELECT_BUILD_FOR_SPEED 0
# include "quickselect_loop_src.h"

# undef QUICKSELECT_BUILD_FOR_SPEED
# define QUICKSELECT_BUILD_FOR_SPEED 0
# include "sampling_table_src.h"

/* quickselect definition (internal interface) */
int quickselect_internal(char *base, size_t nmemb,
    /*const*/ size_t size, int (*compar)(const void *,const void *),
    size_t *pk, size_t nk, unsigned int options, char **ppeq, char **ppgt)
{
    int ret=0;
    size_t onk=nk; /* original nk value */
    size_t s=0UL;  /* rotation amount */
    size_t alignsize, salignsize
#if SILENCE_WHINEY_COMPILERS
        =0UL
#endif
        , size_ratio, sratio
#if SILENCE_WHINEY_COMPILERS
        =0UL
#endif
        , sz;
    void (*swapf)(char *, char *, size_t);
    void (*sswapf)(char *, char *, size_t)
#if SILENCE_WHINEY_COMPILERS
        =NULL
#endif
        ;
    size_t *indices = NULL;
    char **pointers = NULL;
    void (*pswapf)(char *, char *, size_t);
    unsigned int table_index;

    /* Initialization of strings is performed here (rather than in
       quickselect_loop) so that quickselect_loop can be made inline.
    */
    if ((char)0==file_initialized) initialize_file(__FILE__);

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
        "/* %s: %s line %d: nmemb=%lu, size=%lu, pk=%p, nk=%lu, options=0X%X "
        "*/\n",__func__,source_file,__LINE__,nmemb,size,(const void *)pk,nk,
        options);
#endif
    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || ((0UL!=nk) && (NULL==pk))
    || (0U!=(options&~(QUICKSELECT_USER_OPTIONS_MASK|QUICKSELECT_STRICT_SELECTION)))
    ) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: nmemb=%lu, base=%p, size=%lu, compar=%s, pk=%p,"
            " nk=%lu, pk[%lu]=%lu, pk[%lu]=%lu, options=0x%x */\n",__func__,
            source_file,__LINE__,nmemb,base,(unsigned long)size,
            comparator_name(compar),(const void *)pk,nk,0UL,pk[0],nk-1UL,
            pk[nk-1UL],options);
        return errno=EINVAL;
    }

    if (2UL>nmemb) return ret ; /* Return early if there's nothing to do. */
    /* Simplify tests for selection vs. sorting by ensuring a NULL pointer when
       sorting. Ensure consistency between pk and nk. Ensure sorted pk array
       with no duplicated order statistics.
    */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        if (NULL!=pk)
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=%p, nk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,nmemb,(const void *)pk,nk,
                0UL,pk[0],nk-1UL,pk[nk-1UL]);
        else
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=NULL */\n",
                __func__,source_file,__LINE__,nmemb);
    }
#endif

    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();

    /* rough estimate for table_size */
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

    if (0UL==nk) pk=NULL;
    else if (NULL==pk) onk=nk=0UL;
    else if (1UL<nk) { /* binary search requires nondecreasing pk */
        size_t p, q;
        for (p=0UL,q=1UL; q<nk; p++,q++) /* verify (linear scan & compare) */
            if (pk[p]>pk[q]) break; /* not nondecreasing */

        /* Alignment, swapping function for order statistics. */
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

        if (q<nk) { /* fix (sort) iff broken (not nondecreasing) */
            d_quickselect_loop((char *)pk,0UL,nk,sz,size_tcmp,NULL,0UL,0UL,sswapf,
                salignsize,sratio,2U,quickselect_cache_size,0UL,0U,NULL,NULL);
        }
        /* verify, fix uniqueness */
        for (p=0UL,q=1UL; q<=nk; q++) { /* CLEARLY executed iff nk>0UL */
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    /* CLEARLY not executed if nk==1UL */
                    size_t r=q-p;
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
                    if (DEBUGGING(PK_ADJUSTMENT_DEBUG)) { (V)fprintf(stderr,
                            "/* %s: %s line %d: %lu duplicate order statistic%s"
                            " [%lu,%lu): */\n",__func__,source_file,__LINE__,r,
                            r==1?"":"s",p,q);
                        print_size_t_array(pk,sratio,0UL, nk-1UL, "/* "," */");
                    }
#endif
                    irotate((char *)pk,p,q,nk,sz,sswapf,salignsize,sratio);
                    s=q-p, nk-=s, q=p;
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
                    if (DEBUGGING(PK_ADJUSTMENT_DEBUG)) { (V)fprintf(stderr,
                            "/* %s: %s line %d: duplicate order statistic%s "
                            "rotated to end: */\n",__func__,source_file,
                            __LINE__,r==1?"":"s");
                        print_size_t_array(pk, 1UL,0UL, nk-1UL, "/* "," */");
                    }
#endif
                }
            }
        }
    }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
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

    /* Swap function based on alignment size */
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

#if 0 /* allow indirection at size_ratio==1 for testing */
    /* Don't use indirection if size_ratio==1UL (it would be inefficient). */
    if (1UL==size_ratio) options&=~(QUICKSELECT_INDIRECT);
#endif

    /* If indirect sorting, alignment size and size_ratio are reset to
       appropriate values for pointers.
    */
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
/* XXX
   It might be worthwhile trying to allocate 1.5*nmemb pointers to permit use
   of indirect mergesort (set pbeyond to nmemb).  Maybe for sorting only, maybe
   based on size_ratio (e.g. indirect mergesort is faster than indirect d&c for
   size_ratio=25 and nmemb over at least 17-256).  Maybe based on options
   (indirect mergesort is faster than stable d&c at size_ratio=1, and faster
   than in-place mergesort at size_ratio=1 and nmemb>~50.  Note that indirect
   d&c is faster than indirect mergesort at size_ratio=1, nmemb 17-256
*/
        char **p=set_array_pointers(pointers,nmemb,base,size,0UL,nmemb);
        if (NULL==p) {
            if (NULL!=pointers) { free(pointers); pointers=NULL; }
            options&=~(QUICKSELECT_INDIRECT);
        } else {
            if (p!=pointers) pointers=p;
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
            if (DEBUGGING(MEMORY_DEBUG)) {
                (V)fprintf(stderr,"/* %s: %s line %d: base=%p pointers=%p */\n",
                    __func__,source_file,__LINE__,base,pointers);
                print_some_array(base,0UL,nmemb-1UL,"/* base_array: "," */",
                    options & ~(QUICKSELECT_INDIRECT));
                print_some_array(pointers,0UL,nmemb-1UL,"/* pointers: "," */",
                    options);
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
        }
    }

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
        "/* %s: %s line %d: size=%lu, alignsize=%lu, size_ratio=%lu */\n",
        __func__,source_file,__LINE__,(unsigned long)size,
        (unsigned long)alignsize,(unsigned long)size_ratio);
#endif
    if (DEBUGGING(RATIO_GRAPH_DEBUG)) { /* for graphing worst-case partition ratios */
        size_t q;
        for (q=0UL; q<(SAMPLING_TABLE_SIZE); q++)
            stats_table[q].max_ratio=stats_table[q].repivot_ratio=0UL,
                stats_table[q].repivots=0UL;
    }

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        if (NULL!=pk)
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=%p, nk=%lu, pk[%lu]=%lu, pk[%lu]=%lu */\n",
                __func__,source_file,__LINE__,nmemb,(const void *)pk,nk,
                0UL,pk[0],nk-1UL,pk[nk-1UL]);
        else
            (V)fprintf(stderr,
                "/* %s: %s line %d: nmemb=%lu, pk=NULL */\n",
                __func__,source_file,__LINE__,nmemb);
    }
#endif
#if ASSERT_CODE
    if (NULL!=pk) {
        A(nk>0UL);
        A(0UL<=pk[0]);
#if DEBUG_CODE
        if (pk[nk-1]>=nmemb) (V)fprintf(stderr,
            "/* %s: %s line %d: nmemb=%lu, pk=%p, nk=%lu, pk[%lu]=%lu */\n",
            __func__,source_file,__LINE__,nmemb,(const void *)pk,nk,nk-1UL,pk[nk-1]);
#endif
        A(pk[nk-1]<nmemb);
    }
#endif

    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        size_t n;
        d_quickselect_loop((char *)pointers,0UL,nmemb,sizeof(char *),compar,pk,
            0UL,nk,pswapf,sizeof(char *),1UL,table_index,quickselect_cache_size,
            0UL,options,NULL,NULL);
        if (NULL==indices) indices=(size_t *)pointers;
        indices=convert_pointers_to_indices(base,nmemb,size,pointers,
            nmemb,indices,0UL,nmemb);
        A(NULL!=indices);
        n=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,alignsize);
        free(pointers);
        if ((void *)indices==(void *)pointers) indices=NULL;
        if (nmemb<(n>>1)) { /* indirection failed; use direct sort/select */
/* XXX shouldn't happen */fprintf(stderr,"/* %s: %s line %d: size_ratio=%lu, nmemb=%lu, indirect sort failed */\n",
__func__,source_file,__LINE__,size_ratio,nmemb);
            options&=~(QUICKSELECT_INDIRECT);
            d_quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,swapf,
                alignsize,size_ratio,table_index,quickselect_cache_size,0UL,
                options,NULL,NULL);
        } else if (0UL!=n)
            nmoves+=n*size_ratio;
    } else
        d_quickselect_loop(base,0UL,nmemb,size,compar,pk,0UL,nk,swapf,
            alignsize,size_ratio,table_index,quickselect_cache_size,0UL,options,
            NULL,NULL);

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

    /* Restore pk to full sorted (non-unique) order for caller convenience. */
    /* This may be expensive if the caller supplied a large number of duplicate
       order statistic ranks.  So be it.  Caller: if it hurts, don't do it.
    */
    if (0UL!=s) { /* there were duplicates */
        d_quickselect_loop((char *)pk,0UL,onk,sz,size_tcmp,NULL,0UL,0UL,sswapf,
            salignsize,sratio,2U,quickselect_cache_size,0UL,0U,NULL,NULL);

    }
    return ret;
}
