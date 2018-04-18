/*INDENT OFF*/
/* XXX obsolete file */

/* Description: C source code for selection-related development */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    select_pivot.c copyright 2016-2018 Bruce Lilly.   \ select_pivot.c $
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
/* XXX obsolete file */
/* $Id: ~|^` @(#)   This is select_pivot.c version 1.10 dated 2018-02-15T02:49:42Z. \ $ */
/* XXX obsolete file */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/* XXX obsolete file */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.select_pivot.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/
/* XXX obsolete file */

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components
   are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: select_pivot.c ~|^` @(#)"
#define SOURCE_MODULE "select_pivot.c"
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2018-02-15T02:49:42Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* XXX obsolete file */
#error "obsolete source"

#if 0
/* local header files needed */
#include "median_test_config.h" /* configuration */
                                /* includes most other local and system header
                                   files required
                                */
#include "initialize_src.h"

/* pivot selection using remedian or median-of-medians */
QUICKSELECT_INLINE
char *d_select_pivot(register char *base, register size_t first, size_t beyond,
    register size_t size, int (*compar)(const void *, const void*),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    register unsigned int table_index, const size_t *pk, unsigned int options,
    char **ppc, char **ppd, char **ppe, char **ppf)
{
    size_t nmemb=beyond-first;
    register size_t n, r=nmemb/3UL;     /* 1/3 #elements */
    register char *pivot;

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(PIVOT_SELECTION_DEBUG)||DEBUGGING(REPIVOT_DEBUG))
        (V)fprintf(stderr,
        "/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, nmemb=%lu, "
        "table_index=%u, options=%x */\n",
        __func__,source_file,__LINE__,(void *)base,first,beyond,nmemb,
        table_index,options);
#endif
    A((SAMPLING_TABLE_SIZE)>table_index);
    switch (options&((QUICKSELECT_RESTRICT_RANK)|(QUICKSELECT_STABLE))) {
#if QUICKSELECT_STABLE
        case ((QUICKSELECT_RESTRICT_RANK)|(QUICKSELECT_STABLE)) :
            /* almost full remedian */
            while (table_index<(SAMPLING_TABLE_SIZE)-1U) {
                n=sorting_sampling_table[table_index].samples;
                if (n>r) break;
                table_index++;
            }
            while ((n=sorting_sampling_table[table_index].samples)>r)
                table_index--;
            if (table_index==(SAMPLING_TABLE_SIZE)-1U) {
                for (; n<r; table_index++)
                    n*=3UL;
                if (n>r) table_index--;
            }
        /*FALLTHROUGH*/
#endif /* QUICKSELECT_STABLE */
        default: /* remedian of samples */
            /*Fast pivot selection:1 sample, median-of-3, remedian of samples.*/
            pivot=base+size*(first+(nmemb>>1));     /* [upper-]middle element */
            if (0U<table_index) {       /* 3 or more samples */
#if DEBUG_CODE + ASSERT_CODE
                if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p"
                        ", pu=%p */\n",
                        __func__,source_file,__LINE__,nmemb,(void *)pivot,
                        (pivot-base)/size,*((int *)pivot),
                        (void *)(base+first*size),(void *)(base+beyond*size));
                    if (1UL==size_ratio)
                        print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                            options);
                }
#endif
                A(base+first*size<=pivot);A(pivot<base+beyond*size);
                pivot=remedian(pivot,r,r,size,table_index,compar,options);
            }
#if DEBUG_CODE + ASSERT_CODE
                else { /* should never use a single sample! */
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p"
                        ", pu=%p, table_index=%u */\n",
                        __func__,source_file,__LINE__,nmemb,(void *)pivot,
                        (pivot-base)/size,*((int *)pivot),
                        (void *)(base+first*size),(void *)(base+beyond*size),
                        table_index);
                    if (1UL==size_ratio)
                        print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                            options);
                    A(0U<table_index);
                }
#endif
            *ppc=*ppd=pivot, *ppe=*ppf=pivot+size;
#if DEBUG_CODE + ASSERT_CODE
            if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
                (V)fprintf(stderr,
                    "/* %s: %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p, "
                    "pu=%p */\n",
                    __func__,source_file,__LINE__,nmemb,(void *)pivot,
                    (pivot-base)/size,*((int *)pivot),(void *)(base+first*size),
                    (void *)(base+beyond*size));
                if (1UL==size_ratio)
                    print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                        options);
            }
#endif
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
            if (DEBUGGING(PARTITION_DEBUG)||DEBUGGING(PIVOT_SELECTION_DEBUG))
                (V)fprintf(stderr,
                    "/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, "
                    "nmemb=%lu, pivot@%lu, ppc@%lu, ppd@%lu, ppe@%lu, ppf@%lu "
                    "*/\n",
                    __func__,source_file,__LINE__,(void *)base,first,beyond,
                    nmemb,(pivot-base)/size,(*ppc-base)/size,(*ppd-base)/size,
                    (*ppe-base)/size,(*ppf-base)/size);
#endif
        break;
        case (QUICKSELECT_RESTRICT_RANK) : /* median-of-medians */
            {
                size_t karray[1];
                register size_t o;
                register char *pa, *pb, *pc, *pm;

                A(NULL!=base);A(NULL!=compar);
                A((SAMPLING_TABLE_SIZE)>table_index);
                A(NULL!=ppc);A(NULL!=ppd);A(NULL!=ppe);A(NULL!=ppf);
                /* rearranges elements; precludes stable sort/selection */
                A(0U==(options&(QUICKSELECT_STABLE)));
                if ((char)0==file_initialized) initialize_file(__FILE__);
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
                pc=base+first*size;
                A(9UL<=nmemb); /* never repivot for nmemb < 9 */
                /* Medians of sets of 3 elements. */
# if (DEBUG_CODE > 0) && defined(DEBUGGING)
                if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
                    (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu"
                        ", nmemb=%lu, r=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,nmemb,r);
                    if (1UL==size_ratio)
                        print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                            options);
                }
# endif
                A(1UL<r); /* never repivot for nmemb<9 */
                /* 3 element columns; medians -> 1st row, ignore leftovers */
                for (o=0UL,n=r*size; o<n; o+=size) {
                    pa=pc+o;
                    pb=pa+n; /* middle element */
                    A(pb+n<base+beyond*size);
                    /* first element (pa) bias */
#if QUICKSELECT_INDIRECT
                    if (0U==(options&(QUICKSELECT_INDIRECT))) {
#endif /* QUICKSELECT_INDIRECT */
                        pm=FMED3_FUNCTION_NAME(pb,pa,pb+n,compar,options);
                        if (pm!=pa) {
                            EXCHANGE_SWAP(swapf,pm,pa,size,alignsize,size_ratio,
                                nsw++); /*medians at start of sub-array*/
                        }
#if QUICKSELECT_INDIRECT
                    } else {
                        pm=FMED3_FUNCTION_NAME(*((char **)pb),*((char **)pa),
                            *((char **)(pb+n)),compar,
                            options&~(QUICKSELECT_INDIRECT));
                        /*medians at start of sub-array*/
                        /*compare returned data pointer,swap indirect pointers*/
                        if (pm!=*((char **)pa)) {
                            if (pm==*((char **)pb)) {
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,
                                    size_ratio,nsw++);
                            } else {
                                EXCHANGE_SWAP(swapf,pa,pb+n,size,alignsize,
                                    size_ratio,nsw++);
                            }
                        }
                    }
#endif /* QUICKSELECT_INDIRECT */
# if (DEBUG_CODE > 0) && defined(DEBUGGING)
                    if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
                        (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, "
                            "beyond=%lu, nmemb=%lu, r=%lu, pa@%lu, pb@%lu, "
                            "pc@%lu -> pm@%lu */\n",
                            __func__,source_file,__LINE__,first,beyond,nmemb,r,
                            (pc+o-base)/size,(pb-base)/size,(pb+n-base)/size,
                            (pm-base)/size);
                        if (1UL==size_ratio)
                            print_some_array(base,1UL,first,beyond-1UL, "/* ",
                                " */",options);
                    }
# endif
                }
                /* medians in first row */
#if ASSERT_CODE
                if ((pc<base+first*size)||(pc>base+beyond*size-r-r))
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: pc=%p base=%p first=%lu beyond=%lu "
                        "nmemb=%lu r=%lu size=%lu n=%lu o=%lu */\n",
                        __func__,source_file,__LINE__,(void *)pc,(void *)base,
                        first,beyond,nmemb,r,size,n,o);
#endif
                A(pc<=base+beyond*size-n-n);
                *ppc=pc; /* first median */
                /* median of medians */
                karray[0]=first+(r>>1); /* upper-median for even size arrays */
                A(first<=karray[0]);A(karray[0]<first+r);
                *ppf=base+(first+r)*size; /* past last median */
# if (DEBUG_CODE > 0) && defined(DEBUGGING)
                if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, "
                        "r=%lu, karray[0]=%lu, *ppc=pc@%lu, "
                        "*ppf=base+beyond*size@%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,nmemb,r,
                        karray[0],(pc-base)/size,(*ppf-base)/size);
                    if (1UL==size_ratio)
                        print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                            options);
                }
# endif
# if ASSERT_CODE
                A((NULL!=ppd)&&(NULL!=ppe));
                *ppd=*ppe=NULL; /* clear before quickselect; no random values */
# endif
                /* select median of medians; partitions medians */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
                if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: before quickselect_loop: first=%lu,"
                        " beyond=%lu, nmemb=%lu, r=%lu, karray[0]=%lu, *ppd=%p,"
                        " *ppe=%p, save_partial=%u */\n",
                        __func__,source_file,__LINE__,first,beyond,nmemb,r,
                        karray[0],(void *)(*ppd),(void *)(*ppe),save_partial);
                    if (1UL==size_ratio)
                        print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                            options);
                }
#endif
                /* options wrangling: QUICKSELECT_STABLE doesn't apply here
                   (can't use median-of-medians if stability is required),
                   reset QUICKSELECT_RESTRICT_RANK for initial partition;
                   remaining options are network bits, QUICKSELECT_INDIRECT,
                   and QUICKSELECT_OPTIMIZE_COMPARISONS
                */
                A(0U==(options&(QUICKSELECT_STABLE)));
                options &= ~(QUICKSELECT_STABLE | QUICKSELECT_RESTRICT_RANK);
                if (0U!=save_partial)
                    d_quickselect_loop(base,first,first+r,size,compar,
                        karray,0UL,1UL,swapf,alignsize,size_ratio,
                        options,ppd,ppe);
                else
                    d_quickselect_loop(base,first,first+r,size,compar,
                        karray,0UL,1UL,swapf,alignsize,size_ratio,
                        options,NULL,NULL);
# if (DEBUG_CODE > 0) && defined(DEBUGGING)
                if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)
                ||DEBUGGING(SORT_SELECT_DEBUG)
                ) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: after quickselect_loop: first=%lu, "
                        "beyond=%lu, nmemb=%lu, r=%lu, pivot@karray[0]=%lu, "
                        "*ppd=%p, *ppe=%p, save_partial=%u */\n",
                        __func__,source_file,__LINE__,first,beyond,nmemb,r,
                        karray[0],(void *)(*ppd),(void *)(*ppe),save_partial);
                    if (1UL==size_ratio)
                        print_some_array(base,1UL,first,beyond-1UL, "/* "," */",
                            options);
                }
# endif
                pivot=base+karray[0]*size; /* pointer to median of medians */
                /* First third of array (medians) is partitioned. */
                if (0U==save_partial) *ppe=(*ppd=pivot)+size;
#  if (DEBUG_CODE > 0) && defined(DEBUGGING)
                if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, "
                        "karray[0]=%lu, pivot=%p[%lu], *ppd=%p[%lu], "
                        "*ppe=%p[%lu] */\n",
                        __func__,source_file,__LINE__,first,beyond,karray[0],
                        (void *)pivot,(pivot-base)/size,(void *)(*ppd),
                        (*ppd-base)/size,(void *)(*ppe),(*ppe-base)/size);
                }
#  endif
#if ASSERT_CODE
                if ((*ppe<=pivot||(*ppd>pivot))) 
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: save_partial=%u, pivot=%p, ppd=%p, "
                        "*ppd=%p, ppe=%p, *ppe=%p */\n",
                        __func__,source_file,__LINE__,save_partial,
                        (void *)pivot,
                        (void *)ppd,NULL!=ppd?(void *)(*ppd):NULL,
                        (void *)ppe,NULL!=ppe?(void *)(*ppe):NULL);
                A(*ppd<=pivot);A(pivot<*ppe);
#endif
            }
        break;
    }
#if ASSERT_CODE
/* verify proper partitioning */
    {
        size_t s, t, u, v, w, x, y, z;
        A(*ppc>=base);
        A(*ppd>=base);
        A(*ppe>=base);
        A(*ppf>=base);
        s=(*ppc-base)/size;
        t=(*ppd-base)/size;
        u=(*ppe-base)/size-1UL;
        v=(*ppf-base)/size-1UL;
        x=nlt, y=neq, z=ngt;
        w=test_array_partition(base,s,t,u,v,size,compar,options,NULL,NULL);
        nlt=x, neq=y, ngt=z;
        if (w!=t) {
            (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, pc@%lu"
                ", pd@%lu, pe@%lu, pf@%lu, size=%lu, options=%x: bad partition "
                "*/\n",
                __func__,source_file,__LINE__,first,beyond,s,t,u+1UL,v+1UL,
                (unsigned long)size,options);
            if (1UL==size_ratio)
                print_some_array(base,1UL,s,v, "/* "," */",options);
            A(0==1);
        }
    }
#endif
    return pivot;
}
#endif
