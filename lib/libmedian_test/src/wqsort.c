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
* $Id: ~|^` @(#)    wqsort.c copyright 2016-2019 Bruce Lilly.   \ wqsort.c $
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
/* $Id: ~|^` @(#)   This is wqsort.c version 1.26 dated 2019-03-16T15:37:11Z. \ $ */
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
#define MODULE_VERSION "1.26"
#define MODULE_DATE "2019-03-16T15:37:11Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define QUICKSELECT_BUILD_FOR_SPEED 0 /* d_dedicated_sort is extern */
#define __STDC_WANT_LIB_EXT1__ 0
#define LIBMEDIAN_TEST_CODE 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if QUICKSELECT_BUILD_FOR_SPEED
# include "pivot_src.h"
#endif

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

static size_t lsz=sizeof(long);
static void (*lswap)(char *, char *, size_t)=NULL;

/* minimum rank for pivot after partitioning, for pivot selection method */
static QUICKSELECT_INLINE size_t min_rank(size_t nmemb, size_t s, int method)
{
    size_t r;

    switch (method) {
        case QUICKSELECT_PIVOT_REMEDIAN_FULL :
        /*FALLTHROUGH*//* to remedian */
        case QUICKSELECT_PIVOT_REMEDIAN_SAMPLES :
            if (3UL>s) return 0UL;
            r=floor_log3(s);
            for (s=2UL,nmemb=1UL; nmemb<r; nmemb++,s<<=1) ;
            s--;
            return s;
        break;
        case QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS :
            r=nmemb/3UL;
            return (((r-1UL)>>1)<<1)+1UL;
        break;
        case QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES :
            return s/2UL;
        break;
    }
}

/* like remedian; return two sample indices which may potentially be used to
   find the largest sample index < x
*/
static void two_choices(size_t mid,size_t s,size_t x,size_t *ps1,size_t *ps2)
{
    /* 3 sample indices are mid-s, mid, mid+s */
    /* if mid < x, mid-s can be eliminated as a possibility */
    /* if mid >= x, mid+s can be eliminated as a possibility */
    if (mid<x) {
        *ps1=mid, *ps2=mid+s;
    } else {
        *ps1=mid-s, *ps2=mid;
    }
}

/* return index of rightmost sample with index < x
   array size nmemb starting at index first with n uniformly-spaced samples
      obtained by pivot sampling method pm
*/
static size_t sample_index_lt(size_t nmemb, size_t first, size_t n, size_t x, int pm)
{
    size_t i, mid, o, p, q, r, s, t, u, v;
    mid=first+(nmemb>>1); /* index of [upper-]middle element */
    /* single sample may be offset from mid, positions of larger number of
       samples depends on pivot selection method
    */
    switch (n) {
        case 1UL :
            switch (nmemb) {
                case 0UL : case 1UL : case 2UL : case 3UL : case 4UL :
                case 6UL : case 8UL :
                    /* leave pivot at [upper-]middle element */
                break;
                case 5UL : case 7UL : case 9UL :
                    mid++; /* away from middle for bitonic */
                break;
                default :
                    mid+=((nmemb-1UL)/8); /* 1/2+1/8=5/8 */
                break;
            }
            if (x<=mid) i=0UL; else i=mid;
        break;
        default :
            switch (pm) {
                case QUICKSELECT_PIVOT_REMEDIAN_FULL :
                /*FALLTHROUGH*//* to remedian */
                case QUICKSELECT_PIVOT_REMEDIAN_SAMPLES :
                   /* n is a power of 3
                      top-level samples are at mid +- nmemb/3
                      next-level samples are at current-level samples +-
                         current spacing/3
                   */
                   /* r is current-level spacing */
                   /* o is minimum spacing */
                   /* p<=q are samples at current level */
                   /* s<=t<=u<=v are samples at next level */
                   for (p=q=mid,r=nmemb/3UL,o=nmemb/n; o<=r; r/=3UL) {
                       two_choices(p,r,x,&s,&t);
                       if (p!=q) {
                           two_choices(q,r,x,&u,&v);
                           /* now have 4 choices: s, t, u, v */
                           if (s>=x) { /* t,u,v are not viable */
                               if (3*o<=r) { /* at least one more iteration */
                                   p=q=s;
                               } else {
                                   i=0UL; /* s>=x */
                                   break; /* loop would terminate anyway */
                               }
                           } else if (t>=x) { /* u,v are not viable */
                               if (3*o<=r) { /* at least one more iteration */
                                   p=s, q=t;
                               } else {
                                   i=s; /* t>=x */
                                   break; /* loop would terminate anyway */
                               }
                           } else if (u>=x) { /* v is not viable */
                               /* s<=t<=u; s is not viable */
                               if (3*o<=r) { /* at least one more iteration */
                                   p=t, q=u;
                               } else {
                                   i=t; /* u>=x */
                                   break; /* loop would terminate anyway */
                               }
                           } else if (v>=x) {
                               /* s<=t<=u<=v; s,t are not viable */
                               if (3*o<=r) { /* at least one more iteration */
                                   p=u, q=v;
                               } else {
                                   i=u; /* v>=x */
                                   break; /* loop would terminate anyway */
                               }
                           } else { /* s<=t<=u<=v<x; s,t,u are not viable */
                               if (3*o<=r) { /* at least one more iteration */
                                   p=q=v;
                               } else {
                                   i=v; /* v<x */
                                   break; /* loop would terminate anyway */
                               }
                           }
                       } else {
                           /* only 2 choices: s or t */
                           if (3*o<=r) { /* at least one more iteration */
                               p=s, q=t;
                           } else {
                               if (t<x) i=t;
                               else if (s>=x) i=0UL;
                               else i=s;
                               break; /* loop would terminate anyway */
                           }
                       }
                   }
                break;
                case QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS :
/* XXX */
/* XXX what is desired: index of median in mid-position, index of median in original position, index of some element in set of 3? */
                break;
                case QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES :
                    /* simple case: samples are uniformly spaced around mid */
                    for (o=nmemb/n,i=mid+(n>>1)*o; i>=x; i-=o)
                        if (i<=o) return 0UL;
                break;
            }
        break;
    }
    return i;
}

/* XXX work in progress:
   As large sub-arrays are processed, pivot selection might use
   median-of-medians or median of samples, and median finding entails
   pivot selection for quickselect to find the median of medians or samples.
   Reversing the data movement to produce worst-case input at every stage
   can quickly get quite complicated.
*/
void make_adverse(long *base, size_t first, size_t beyond, size_t *pk,
    size_t firstk, size_t beyondk, size_t size_ratio,
    unsigned int distribution, unsigned int options)
{
    size_t karray[1], mid, n, nmemb=beyond-first, o, p, q, r, s, t, u, prank;
    auto size_t lk, rk;
    auto unsigned int new_distribution;
    int method;
    register long *pb, *pc, *pivot, *pm;
    if (NULL==lswap) lswap=swapn(lsz);
    switch (nmemb) {
        /* small sub-arrays with no method (using dedicated_sort) */
        case 0UL : return;
        case 1UL : return;
        return;
        case 2UL :
                /* 0 1 */
                EXCHANGE_SWAP(lswap,base+first,base+first+1UL,lsz,lsz,1UL,/**/);
                /* 1 0 */
        return;
        case 3UL :
                /* 0 1 2 */
                EXCHANGE_SWAP(lswap,base+first,base+first+2UL,lsz,lsz,1UL,/**/);
                /* 2 1 0 */
                EXCHANGE_SWAP(lswap,base+first,base+first+1UL,lsz,lsz,1UL,/**/);
                /* 1 2 0 */
        return;
        case 4UL :
                /* 0 1 2 3 */
                EXCHANGE_SWAP(lswap,base+first,base+first+1UL,lsz,lsz,1UL,/**/);
                /* 1 0 2 3 */
                EXCHANGE_SWAP(lswap,base+first,base+first+2UL,lsz,lsz,1UL,/**/);
                /* 2 0 1 3 */
                EXCHANGE_SWAP(lswap,base+first+2UL,base+first+3UL,lsz,lsz,1UL,
                    /**/);
                /* 2 0 3 1 */
        return;
        case 5UL :
                /* 0 1 2 3 4 */
                EXCHANGE_SWAP(lswap,base+first,base+first+1UL,lsz,lsz,1UL,/**/);
                /* 1 0 2 3 4 */
                EXCHANGE_SWAP(lswap,base+first,base+first+3UL,lsz,lsz,1UL,/**/);
                /* 3 0 2 1 4 */
                EXCHANGE_SWAP(lswap,base+first+3UL,base+first+4UL,lsz,lsz,1UL,
                    /**/);
                /* 3 0 2 4 1 */
        return;
        default:
            new_distribution=
#if LIBMEDIAN_TEST_CODE
                d_sampling_table
#else
                sampling_table
#endif
                (first,beyond,pk,firstk,beyondk,NULL,&pk,nmemb,size_ratio,
                options);
            method=pivot_method(NULL,nmemb,0UL,0UL,new_distribution,size_ratio,
                options);
            n=samples(nmemb,method,new_distribution,options);
            if ((QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES==method)&&(1UL==n))
                method=QUICKSELECT_PIVOT_REMEDIAN_SAMPLES; /* pivot offset */
#if 0
            mid=first+(nmemb>>1); /* index of [upper-]middle element */
#else
            /* sample element near the middle */
            if (1UL<n)
                mid=sample_index_lt(nmemb,first,n,first+(nmemb>>1)+1UL,method);
            else
                mid=sample_index_lt(nmemb,first,n,beyond,method);
/* make_adverse line 310: first=3, nmemb=6, mid=0 */
#endif
            switch (n) {
#if 0
                case 1UL :
                    /* single sample: extreme value at sample (pivot) */
                    switch (nmemb) {
                        case 6UL : case 8UL :
                            /* leave pivot at [upper-]middle element */
                        break;
                        case 7UL : case 9UL :
                            mid++; /* away from middle for bitonic */
                        break;
                        default :
                            mid+=((nmemb-1UL)/8); /* 1/2+1/8=5/8 */
                        break;
                    }
                    /* make sequence for next smaller size */
                    make_adverse(base,first,beyond-1UL,pk,firstk,beyondk,
                        size_ratio,distribution,options);
                    /* large element is in last position */
                    /* swap extreme element into position */
fprintf(stderr,"/* %s line %d: first=%lu, beyond=%lu, nmemb=%lu, mid=%lu */\n",__func__,__LINE__,first,beyond,nmemb,mid);
                    EXCHANGE_SWAP(lswap,base+mid,base+(beyond-1UL),lsz,lsz,1UL,
                        /**/);
                        print_some_array(base,first,beyond-1UL,"/* "," */",options);
                return;
#endif
                default :
                    prank=min_rank(nmemb,n,method);
                    /* split will be at pivot (rank prank) */
                    /* left and right regions should be adverse sequences */
                    /* if ratio is large, large region will use median of
                       medians
                    */
                    r=(nmemb-1UL-prank)/(1UL+prank); /* ratio */
                    if (0UL<prank) {
                        if (NULL!=pk) d_klimits(first,first+prank,pk,firstk,
                            beyondk,&lk,&rk);
                        else rk=firstk, lk=beyondk;
                        new_distribution=
#if LIBMEDIAN_TEST_CODE
                            d_sampling_table
#else
                            sampling_table
#endif
                            (first,first+prank,pk,lk,rk,NULL,&pk,prank,
                            size_ratio,options);
                        make_adverse(base,first,first+prank,pk,lk,rk,
                            size_ratio,new_distribution,options);
                    }
                    if (prank<beyond-1UL) {
                        if (NULL!=pk) d_klimits(first+prank+1UL,beyond,pk,
                            firstk,beyondk,&lk,&rk);
                        else rk=firstk, lk=beyondk;
                        new_distribution=
#if LIBMEDIAN_TEST_CODE
                            d_sampling_table
#else
                            sampling_table
#endif
                            (first+prank+1UL,beyond,pk,lk,rk,NULL,&pk,
                            nmemb-prank-1UL,size_ratio,options);
                        make_adverse(base,first+prank+1UL,beyond,pk,lk,
                            rk,size_ratio,new_distribution,options);
                    }
                    o = nmemb / n; /* elements offset between samples */
                    /* Put prank+1 extreme values in positions where they would
                       be prior to partitioning, i.e. in sample positions for
                       remedian, and in the middle of the array for median of
                       medians and median of samples.  This step is the reverse
                       of partitioning.  For median of medians and median of
                       samples, the pivot should be in the middle, with smaller
                       elements to its left.
                    */
                    switch (method) {
                        case QUICKSELECT_PIVOT_REMEDIAN_FULL :
                        /*FALLTHROUGH*//* to remedian */
                        case QUICKSELECT_PIVOT_REMEDIAN_SAMPLES :
                            /* pivot goes to highest sample less than the lower-
                               middle element+1.  prank/2 samples go in the next
                               sample positions below that.  Remaining samples
                               go in sample locations below
                               lower-middle+1-(nmemb/3) (i.e. left side of first
                               row of samples).
                               This works up to nmemb=2047.
                            */
                            if (1UL<n) {
                                mid=first+((nmemb-1UL)>>1); /* lower-middle index */
                                o=sample_index_lt(nmemb,first,n,mid+2UL,method);
                            } else o=mid;
                            EXCHANGE_SWAP(lswap,base+first+prank,base+o,lsz,lsz,
                                1UL,/**/);
                            if (1UL<n) {
                                for (p=first,q=first+(prank>>1); p<q; p++) {
                                    o=sample_index_lt(nmemb,first,n,o,method);
                                    if (o==p) continue;
                                    EXCHANGE_SWAP(lswap,base+p,base+o,lsz,lsz,1UL,
                                        /**/);
                                }
                                for (o=mid+2UL-(nmemb/3UL),q=first+prank; p<q; p++) {
                                    o=sample_index_lt(nmemb,first,n,o,method);
                                    if (o==p) continue;
                                    EXCHANGE_SWAP(lswap,base+p,base+o,lsz,lsz,1UL,
                                        /**/);
                                }
                            }
                        break;
                        case QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS :
                        case QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES :
                            q=(n>>1);
                            for (p=first,o=mid-q; o<=mid; p++,o++) {
                                EXCHANGE_SWAP(lswap,base+p,base+o,lsz,lsz,1UL,
                                    /**/);
                            }
                            /* If the pivot for the middle section isn't the
                               middle element, swap the pivot to where it's
                               expected to be found.  If the samples (or
                               medians) in the middle section will be rearranged
                               for median finding, undo that.
                               This works up to nmemb=1917.
                            */
                            o=samples(n,pivot_method(NULL,nmemb,0UL,0UL,2U,size_ratio,options),
                                2U,options);
                            switch (o) {
                                case 1UL : /* median pivot might be offset */
                                    p=mid;
                                    switch (n) {
                                        case 0UL : case 1UL : case 2UL : case 3UL :
                                        case 4UL : case 6UL : case 8UL :
                                            /* leave pivot at [upper-]middle element */
                                        break;
                                        case 5UL : case 7UL : case 9UL :
                                            p++; /* away from middle for bitonic */
                                        break;
                                        default :
                                            p+=((n-1UL)/8); /* 1/2+1/8=5/8 */
                                        break;
                                    }
                                    if (p!=mid) {
                                        EXCHANGE_SWAP(lswap,base+p,base+mid,lsz,lsz,
                                            1UL,/**/);
                                    }
                                break;
                                default :
                                    p=(o>>1),q=n/o,s=mid+p,t=mid-p;
                                    for (u=mid+p*q; mid<s; s--,u-=q) {
                                        if (s!=u) {
                                            EXCHANGE_SWAP(lswap,base+s,base+u,
                                                lsz,lsz, 1UL,/**/);
                                        }
                                    }
                                    for (u=mid-p*q; mid>t; t++,u+=q) {
                                        if (t!=u) {
                                            EXCHANGE_SWAP(lswap,base+t,base+u,
                                                lsz,lsz, 1UL,/**/);
                                        }
                                    }
                                break;
                            }
                        break;
                    }
                    /* If the pivot selection method is median of medians or
                       median of samples,
                       make the median selection adverse, then
                       distribute the mid-array elements (medians of 3 or
                       samples) to appropriate positions in the array.  This
                       step is the reverse of pivot selection.
                       Note that after
                       the mid-array elements have been permuted to make median
                       finding adverse, it is no longer clear where the small-
                       valued elements are; for median of medians, elements will
                       have to be dispersed such that the dispersed elements are
                       medians of the sets of 3.
                       For median of samples, the dispersion is purely
                       mechanical.
                       Before reversing pivot selection, the middle samples or
                       medians should be partitioned around the median, as that
                       is the order they will have during forward processing.
                    */
                    if ((1UL<n)&&((QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS==method)
                    ||(QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES==method))) {
#if 0 /* XXX don't worry about making median selection adverse; it will make dispersion of samples for median of medians very complicated */
                        size_t karray[1];
                        karray[0]=mid;
                        make_adverse(base,mid-(n>>1),mid+(n>>1),karray,0UL,1UL,
                            size_ratio,2U,options);
#endif
                        switch (method) {
                            case QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS :
                                /* For median of medians, 
                                */
                            break;
                            case QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES :
#if 1
                                q=(n>>1),p=mid+q,r=mid-q;
                                for (o=beyond; r<=p; p--) {
                                    o=sample_index_lt(nmemb,first,n,o,method);
                                    if (p==o) continue;
                                    EXCHANGE_SWAP(lswap,base+p,base+o,lsz,lsz,
                                        1UL,/**/);
                                }
#else
                                p=(n>>1),q=nmemb/n,s=mid+p,t=mid-p;
                                for (u=mid+p*q; mid<s; s--,u-=q) {
                                    if (s!=u) {
                                        EXCHANGE_SWAP(lswap,base+s,base+u,
                                            lsz,lsz, 1UL,/**/);
                                    }
                                }
                                for (u=mid-p*q; mid>t; t++,u+=q) {
                                    if (t!=u) {
                                        EXCHANGE_SWAP(lswap,base+t,base+u,
                                            lsz,lsz, 1UL,/**/);
                                    }
                                }
#endif
                            break;
                        }
                    }

                break;
            }
        return;
    }
}

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
#if LIBMEDIAN_TEST_CODE
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
#if LIBMEDIAN_TEST_CODE
    if (DEBUGGING(WQSORT_DEBUG))
        (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
            "options=0x%x */\n",__func__,source_file,__LINE__,
            first,beyond,options);
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
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "options=0x%x, pivot_minrank=%lu */\n",
                    __func__,source_file,__LINE__,first,beyond,
                    options,pivot_minrank);
#endif
            /* pre-freeze before pivot selection */
            p=count_frozen(base,first,beyond,size);
            if (0U<table_index) s=sorting_sampling_table[table_index-1U].samples;
            else {
                s=1UL;
                switch (nmemb) {
                    case 0UL :
                    case 1UL :
                    case 2UL :
                    case 3UL :
                    case 4UL :
                    case 6UL :
                    case 8UL :
                        /* leave pivot at [upper-]middle element */
                    break;
                    case 5UL :
                    case 7UL :
                    case 9UL :
                        pivot+=size; /* away from middle for bitonic */
                    break;
                    default :
                        pivot+=size*((nmemb-1UL)/8); /* 1/2+1/8=5/8 */
                    break;
                }
            }
            w=r/s;
            o=(s>>1)*w,x=m-r-o,u=m+r+o+1UL;
            t=0UL;
            pivot_sample_rank(pivot,r,r,size,table_index,pivot,&t,base);
            q=pivot_rank(base,first,beyond,size,pivot);
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "options=0x%x, pivot_minrank=%lu: "
                    "pre-freeze %lu frozen elements, overall pivot rank %lu/%lu"
                    ", pivot rank within samples ([%lu,%lu) by %lu) %lu */\n",
                    __func__,source_file,__LINE__,first,beyond,
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
#if LIBMEDIAN_TEST_CODE
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
#if LIBMEDIAN_TEST_CODE
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
            for (pc=pl; (nfrozen<pivot_minrank)&&(pc+size<pu); pc+=size) {
                if (pc==pivot) continue; /* don't freeze middle pivot sample */
                (V)freeze(aqindex(pc,base,size)); /* freeze element @ pc */
            }
            /* freeze middle pivot selection sample */
            (V)freeze(aqindex(pivot,base,size));
            p=count_frozen(base,first,beyond,size);
            q=pivot_rank(base,first,beyond,size,pivot);
            x=m-r-o,u=m+r+o+1UL;
            t=0UL;
            pivot_sample_rank(pivot,r,r,size,table_index,pivot,&t,base);
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "options=0x%x, pivot_minrank=%lu: post-"
                    "freeze %lu frozen elements, overall pivot rank %lu/%lu, "
                    "pivot rank within %lu samples %lu */\n",__func__,
                    source_file,__LINE__,first,beyond,options,
                    pivot_minrank,p,q,nmemb,s*3UL,t);
#endif
        break;
        case (QUICKSELECT_RESTRICT_RANK) : /* median-of-medians */
            nfrozen=0UL, pivot_minrank=r; /* pre-freeze not required */
            if (0UL==(r&0x01UL)) pivot_minrank++;
            p=count_frozen(base,first,beyond,size);
            q=pivot_rank(base,first,beyond,size,pivot);
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(WQSORT_DEBUG))
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "options=0x%x, pivot_minrank=%lu: pre-"
                    "selection %lu frozen elements, pivot rank %lu */\n",
                    __func__,source_file,__LINE__,first,beyond,
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
    auto unsigned int distribution;
    struct sampling_table_struct *psts;
        
    for (;;) {
#if LIBMEDIAN_TEST_CODE
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
                (V)d_quickselect_loop(base,first,beyond,size,COMPAR_ARGS,NULL,0UL,
                    0UL,swapf,alignsize,size_ratio,quickselect_cache_size,0UL,
                    options,NULL,NULL);
                return;
            }
        } else return;
        distribution=
#if LIBMEDIAN_TEST_CODE
            d_sampling_table
#else
            sampling_table
#endif
            (first,beyond,pk,firstk,beyondk,NULL,
            &pk,nmemb,size_ratio,options);

        /* normal pivot selection (for comparison and swap counts) */
        pivot=
#if LIBMEDIAN_TEST_CODE
            d_select_pivot
#else
            select_pivot
#endif
            (base,first,beyond,size,compar,swapf,alignsize,
            size_ratio,distribution,NULL,0UL,0UL,quickselect_cache_size,pbeyond,options,
            &pc,&pd,&pe,&pf,NULL,&samples);
#if LIBMEDIAN_TEST_CODE
        t=pivot_minrank;
#endif
        /* XXX no support for efficient stable sorting */
        d_partition(base,first,beyond,pc,pd,pivot,pe,pf,size,compar,swapf,
            alignsize,size_ratio,quickselect_cache_size,options,&p,&q);
#if DEBUG_CODE && defined(DEBUGGING)
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
#if LIBMEDIAN_TEST_CODE
        if (DEBUGGING(WQSORT_DEBUG))
            (V)fprintf(stderr,"/* %s: %s line %d: p=%lu, q=%lu, pivot rank=s="
                "%lu, r=%lu, ratio=%lu */\n",
                __func__,source_file,__LINE__,p,q,s,r,ratio);
#endif
        if (NULL!=pk) d_klimits(p,q,pk,firstk,beyondk,&lk,&rk);
        else rk=firstk, lk=beyondk;
        /* < region indices [first,p), order statistics [firstk,lk) */
        /* > region indices [q,beyond), order statistics [rk,beyondk) */
#if LIBMEDIAN_TEST_CODE
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
    table_index=d_sample_index(sorting_sampling_table,table_index,nmemb);

    nfrozen=0UL, pivot_minrank=nmemb;
    wqsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,
        pk,0UL,nk,table_index,0UL,options,0);
}
