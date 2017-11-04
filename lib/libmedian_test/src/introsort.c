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
* $Id: ~|^` @(#)    introsort.c copyright 2016-2017 Bruce Lilly.   \ introsort.c $
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
/* $Id: ~|^` @(#)   This is introsort.c version 1.4 dated 2017-11-03T19:51:34Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.introsort.c */

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
#define ID_STRING_PREFIX "$Id: introsort.c ~|^` @(#)"
#define SOURCE_MODULE "introsort.c"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-11-03T19:51:34Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* Insertion sort using linear search for insertion point followed by
   insertion via rotation.
   Binary search doesn't work well for large, nearly-sorted arrays.
   Linear-search-only insertion sort:
      preset mtype, mbase, mn, mfirst, ma, mb, msize, msize_ratio, mu, mpu, mcf
      modifies mn, ma, mb
*/
#define ISORT_LS(mtype,mbase,mn,mfirst,ma,mb,msize,msize_ratio,mu,mpu,mcf)     \
    while (mn>mfirst) {                                                        \
        --mn;                                                                  \
        if (0<mcf(ma,mb)) {                                                    \
            register mtype *px, *py, *pz, t;                                   \
            register size_t l=mn+2UL;                                          \
            register char *pc, *pd;                                            \
            if (l>mu) {                                                        \
                pd=mb+msize; /* a simple swap */                               \
            } else { /* linear search */                                       \
                for (pd=mb+msize; pd<=mpu; pd+=size)                           \
                    if (0>=mcf(pa,pd)) break;                                  \
            }                                                                  \
            /* Insertion by rotating elements [n,l) left by 1.  Same number of \
               swaps as ripple/bubble swapping.  Rotation by 1 position could  \
               be implemented by reversals as for general rotation, but the    \
               specific case of 1 position can be implemented more efficiently \
               using the temporary variable once per iteration of the basic    \
               type size rather than swapping, which uses the temporary        \
               variable multiple times (unless the element size is exactly the \
               same size as the basic type and the rotation distance is 1 (i.e.\
               a simple swap)).                                                \
            */                                                                 \
            pc=ma, px=(mtype *)pc, pc+=msize;                                  \
            if (0U!=instrumented) { size_t r=(pd-pc)/msize+1UL;                \
                if (MAXROTATION<=r) r=0UL;                                     \
                nrotations[r]+=msize_ratio;                                    \
            }                                                                  \
            while ((char *)px<pc) {                                            \
                py=px,t=*py,pz=py+msize_ratio;                                 \
                while ((char *)pz<pd)                                          \
                    *py=*pz,py=pz,pz+=msize_ratio;                             \
                *py=t,px++;                                                    \
            }                                                                  \
        }                                                                      \
        mb=ma,ma-=msize;                                                       \
    }

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void introsort_loop(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int table_index, size_t depth_limit, unsigned int options)
{
    size_t eq, gt, s;
    char *pc, *pd, *pe, *pf, *pivot;

    A(beyond>first);
    while (beyond-first>introsort_small_array_cutoff) {
        if (0UL==depth_limit) {
#if 1
            (V)fprintf(stderr,
                "/* %s: %s line %d: recursion depth limit reached */\n",
                __func__,source_file,__LINE__);
#endif
            nfrozen=0UL, pivot_minrank=beyond-first;
            heap_sort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
            return;
        }
        depth_limit--;
#if 0
        /* Introsort can be improved by improving the quality and
           quality of samples and pivot selection methods.  The following is
           a simple median-of-3 of the first, middle, and last elements, which
           causes issues with some input sequences (median-of-3 killer,
           rotated, organ-pipe, etc.). It does however use ternary median-of-3.
        */
        pivot=fmed3(base+size*first,base+size*(first+(beyond-first)>>1),
            base+size*(beyond-1UL),compar);
#else
        /* adaptive sampling for pivot selection as in quickselect */
        /* freeze low-address samples which will be used for pivot selection */
        if (aqcmp==compar)
            (V)freeze_some_samples(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,table_index,options);

        pivot=d_select_pivot(base,first,beyond,size,compar,swapf,alignsize,
            size_ratio,table_index,NULL,options,&pc,&pd,&pe,&pf);
        pivot_minrank=beyond-first;
#endif
        /* partition is modified Bentley&McIlroy split-end partition returning
           indices of equal-to and greater-than region starts.  This efficient
           partition function also avoids further processing of elements
           comparing equal to the pivot.
        */
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,swapf,
            alignsize,size_ratio,options,&eq,&gt);
        /* This processes the > region recursively, < region iteratively as
           described by Musser.  Could process smaller region recursively,
           larger iteratively, but Musser says it's not necessary due to
           recursion depth limit.
        */
        s=beyond-gt;
        if (1UL<s) {
            unsigned int idx=table_index;
            while ((0U<idx)&&(s<=sorting_sampling_table[idx-1U].max_nmemb))
                idx--;
            nrecursions++;
            introsort_loop(base,gt,beyond,size,compar,swapf,alignsize,
                size_ratio,idx,depth_limit,options);
        }
        beyond=eq;
        if (first+1UL<beyond) {
            while ((0U<table_index)
            &&(beyond-first<=sorting_sampling_table[table_index-1U].max_nmemb)
            )
                table_index--;
        }
    }
    if (0U==introsort_final_insertion_sort) {
        nfrozen=0UL, pivot_minrank=beyond-first;
        if (0!=use_networks)
            networksort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,options);
        else if (0!=use_shell)
            shellsort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
        else
#if 0
            isort_internal(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio);
#else
            if (first+1UL<beyond) dedicated_sort(base,first,beyond,size,compar,
                swapf,alignsize,size_ratio,options);
#endif
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void introsort(char *base, size_t nmemb, size_t size,
    int(*compar)(const void *,const void *), unsigned int options)
{
    size_t depth_limit;
    void (*swapf)(char *, char *, size_t);
    size_t alignsize, size_ratio;
    unsigned int table_index=2U; /* optimized for small nmemb */

    if ((char)0==file_initialized) initialize_file(__FILE__);
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    depth_limit=introsort_recursion_factor*floor_lg(nmemb);

    /* Initialize the sampling table index for the array size. Sub-array
       sizes will be smalller, and this step ensures that the main loop won't
       have to traverse much of the table during recursion and iteration.
    */
    while ((0U<table_index)
    &&(nmemb<=sorting_sampling_table[table_index-1U].max_nmemb)
    )
        table_index--;
    while (nmemb>sorting_sampling_table[table_index].max_nmemb)
        table_index++;
    A(table_index<SAMPLING_TABLE_SIZE);

    nfrozen=0UL, pivot_minrank=nmemb;
    introsort_loop(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        table_index,depth_limit,options);
    if (0U!=introsort_final_insertion_sort) {
        char *pa, *pb, *pu;
#if 1
        /* insertion sort with linear search (only) for insertion position */
        register size_t n, u;
        nfrozen=0UL, pivot_minrank=nmemb;
        n=u=nmemb-1UL, pu=pb=base+u*size, pa=pb-size;
        switch (alignsize) {
            case 8UL : /* uint64_t */
                ISORT_LS(uint64_t,base,n,0UL,pa,pb,size,size_ratio,u,pu,compar)
            break;
            case 4UL : /* uint32_t */
                ISORT_LS(uint32_t,base,n,0UL,pa,pb,size,size_ratio,u,pu,compar)
            break;
            case 2UL : /* uint16_t */
                ISORT_LS(uint16_t,base,n,0UL,pa,pb,size,size_ratio,u,pu,compar)
            break;
            default : /* uint8_t */
                ISORT_LS(uint8_t,base,n,0UL,pa,pb,size,size_ratio,u,pu,compar)
            break;
        }
#else
        /* Loop-based alternate compare/swap waltzing insertion sort */
        char *pc, *pl;
        for (pa=pl=base,pu=base+(nmemb-1UL)*size; pa<pu; pa+=size)
            for (pb=pa; (pb>=pl)&&(0<compar(pb,pc=pb+size)); pb-=size)
                EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,nsw++);
#endif
    }
}
