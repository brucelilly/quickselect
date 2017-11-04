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
* $Id: ~|^` @(#)    select_pivot.c copyright 2016-2017 Bruce Lilly.   \ select_pivot.c $
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
/* $Id: ~|^` @(#)   This is select_pivot.c version 1.4 dated 2017-11-03T21:29:35Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.select_pivot.c */

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
#define ID_STRING_PREFIX "$Id: select_pivot.c ~|^` @(#)"
#define SOURCE_MODULE "select_pivot.c"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-11-03T21:29:35Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
/* spacings are in elements */
/* remedian does not modify the value pointed to by middle, but it is not
   declared as const to avoid spurious compiler warnings about discarding the
   const qualifier when passing middle to fmed3 (which also doesn't make
   modifications, but see the rationale there)
*/
char *remedian(register char *middle, register size_t row_spacing,
    register size_t sample_spacing, register size_t size,
    register unsigned int idx, int(*compar)(const void*,const void*),
    char *base)
{
    register size_t o;
#if DEBUG_CODE
if (DEBUGGING(REMEDIAN_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: middle=%p[%lu], row_spacing=%lu, sample_spacing=%lu, idx=%u */\n",
__func__,source_file,__LINE__,(const void *)middle,(middle-base)/size,(unsigned long)row_spacing,(unsigned long)sample_spacing,idx);
}
#endif
    A((SAMPLING_TABLE_SIZE)>idx);
    if (0U < --idx) {
        char *pa, *pb, *pc;
        register size_t s=sample_spacing/3UL;

        o=s*size;
        pa=remedian(middle-o,row_spacing,s,size,idx,compar,base);
        pb=remedian(middle,row_spacing,s,size,idx,compar,base);
        pc=remedian(middle+o,row_spacing,s,size,idx,compar,base);
        return fmed3(pa,pb,pc,compar,base,size);
    }
    o=row_spacing*size;
    return fmed3(middle-o,middle,middle+o,compar,base,size);
}

/* pivot selection using remedian or median-of-medians */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
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
#if DEBUG_CODE
if (DEBUGGING(PIVOT_SELECTION_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) fprintf(stderr,
"/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, nmemb=%lu, table_index=%u, options=%x */\n",
__func__,source_file,__LINE__,(void *)base,first,beyond,nmemb,table_index,options);
#endif
    A((SAMPLING_TABLE_SIZE)>table_index);
    switch (options&((QUICKSELECT_RESTRICT_RANK)|(QUICKSELECT_STABLE))) {
#if 0
        case 0U : /*FALLTHROUGH*/
        case (QUICKSELECT_STABLE) : /*FALLTHROUGH*/
#endif
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
        default: /* remedian of samples */
            /*Fast pivot selection:1 sample, median-of-3, remedian of samples.*/
            pivot=base+size*(first+(nmemb>>1));     /* [upper-]middle element */
            if (0U<table_index) {       /* 3 or more samples */
#if DEBUG_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p, pu=%p */\n",
__func__,source_file,__LINE__,nmemb,(void *)pivot,(pivot-base)/size,*((int *)pivot),(void *)(base+first*size),(void *)(base+beyond*size));
    print_some_array(base,first,beyond-1UL, "/* "," */");
}
#endif
                A(base+first*size<=pivot);A(pivot<base+beyond*size);
                pivot=remedian(pivot,r,r,size,table_index,compar,base);
            }
            *ppc=*ppd=pivot, *ppe=*ppf=pivot+size;
#if DEBUG_CODE
if ((pivot<base+first*size)||(pivot>=base+beyond*size)) {
    (V)fprintf(stderr,
"/* %s: %s line %d: nmemb=%lu, pivot=%p[%lu](%d), pl=%p, pu=%p */\n",
__func__,source_file,__LINE__,nmemb,(void *)pivot,(pivot-base)/size,*((int *)pivot),(void *)(base+first*size),(void *)(base+beyond*size));
    print_some_array(base,first,beyond-1UL, "/* "," */");
}
#endif
#if DEBUG_CODE
if (DEBUGGING(PARTITION_DEBUG)||DEBUGGING(PIVOT_SELECTION_DEBUG)) fprintf(stderr,
"/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, nmemb=%lu, pivot@%lu, ppc@%lu, ppd@%lu, ppe@%lu, ppf@%lu */\n",
__func__,source_file,__LINE__,(void *)base,first,beyond,nmemb,(pivot-base)/size,(*ppc-base)/size,(*ppd-base)/size,(*ppe-base)/size,(*ppf-base)/size);
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
                if ((char)0==file_initialized) initialize_file(__FILE__);
                /* Finding a pivot with guaranteed intermediate rank. Ideally, median
                   (50%).  Blum, Floyd, Pratt, Rivest, Tarjan median-of-medians using
                   sets of 5 elements with recursion guarantees rank in (asymptotically)
                   30-70% range, often better; can guarantee linear median-finding,
                   N log(N) sorting. Simplification ignores "leftover" elements with a
                   slight increase in rank range.  Non-recursive method (using separate
                   linear median finding) can use sets of 3 elements to provide a
                   tighter 33.33% to 66.67% range (again, slightly wider if "leftover"
                   elements are ignored) at lower computational cost.
                */
                pc=base+first*size;
                A(9UL<=nmemb); /* never repivot for nmemb < 9 */
                /* Medians of sets of 3 elements. */
# if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, r=%lu */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r);
print_some_array(base,first,beyond-1UL, "/* "," */");
}
# endif
                A(1UL<r); /* never repivot for nmemb<9 */
                /* 3 element sets (columns); medians -> 1st row, ignore leftovers */
                for (o=0UL,n=r*size; o<n; o+=size) {
                    pa=pc+o;
                    pb=pa+n; /* middle element */
                    A(pb+n<base+beyond*size);
                    pm=fmed3(pb,pa,pb+n,compar,base,size);/* first element (pa) bias */
# if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, r=%lu, pa@%lu, pb@%lu, pc@%lu -> pm@%lu */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r,(pc+o-base)/size,(pb-base)/size,(pb+n-base)/size,(pm-base)/size);
print_some_array(base,first,beyond-1UL, "/* "," */");
}
# endif
                    if (pm!=pa) EXCHANGE_SWAP(swapf,pm,pa,size,alignsize,size_ratio,nsw++); /* medians at start of sub-array */
# if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, r=%lu, pa@%lu, pb@%lu, pc@%lu -> pm@%lu */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r,(pc+o-base)/size,(pb-base)/size,(pb+n-base)/size,(pm-base)/size);
print_some_array(base,first,beyond-1UL, "/* "," */");
}
# endif
                }
                /* medians in first row */
#if ASSERT_CODE
if ((pc<base+first*size)||(pc>base+beyond*size-r-r))
(V)fprintf(stderr,
"/* %s: %s line %d: pc=%p base=%p first=%lu beyond=%lu nmemb=%lu r=%lu size=%lu n=%lu o=%lu */\n",
__func__,source_file,__LINE__,(void *)pc,(void *)base,first,beyond,nmemb,r,size,n,o);
#endif
                A(pc<=base+beyond*size-n-n);
                *ppc=pc; /* first median */
                /* median of medians */
                karray[0]=first+(r>>1); /* upper-median for even size arrays */
                A(first<=karray[0]);A(karray[0]<first+r);
                *ppf=base+(first+r)*size; /* past last median */
# if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, r=%lu, karray[0]=%lu, *ppc=pc@%lu, *ppf=base+beyond*size@%lu */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r,karray[0],(pc-base)/size,(*ppf-base)/size);
print_some_array(base,first,beyond-1UL, "/* "," */");
}
# endif
# if ASSERT_CODE
                A((NULL!=ppd)&&(NULL!=ppe));
                *ppd=*ppe=NULL; /* clear before quickselect to avoid random values */
# endif
                /* select median of medians; partitions medians */
#if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: before quickselect_loop: first=%lu, beyond=%lu, nmemb=%lu, r=%lu, karray[0]=%lu, *ppd=%p, *ppe=%p, save_partial=%u */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r,karray[0],(void *)(*ppd),(void *)(*ppe),save_partial);
print_some_array(base,first,beyond-1UL, "/* "," */");
}
#endif
                /* cutoff (5UL used here) is unimportant for selection */
                if (0U!=save_partial)
                    d_quickselect_loop(base,first,first+r,size,compar,
                        karray,0UL,1UL,swapf,alignsize,size_ratio,5UL,
                        0x07F8U,ppd,ppe);
                else
                    d_quickselect_loop(base,first,first+r,size,compar,
                        karray,0UL,1UL,swapf,alignsize,size_ratio,5UL,
                        0x07F8U,NULL,NULL);
# if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(SORT_SELECT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: after quickselect_loop: first=%lu, beyond=%lu, nmemb=%lu, r=%lu, pivot@karray[0]=%lu, *ppd=%p, *ppe=%p, save_partial=%u */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r,karray[0],(void *)(*ppd),(void *)(*ppe),save_partial);
print_some_array(base,first,beyond-1UL, "/* "," */");
}
# endif
                pivot=base+karray[0]*size; /* pointer to median of medians */
                /* First third of array (medians) is partitioned. */
                if (0U==save_partial) *ppe=(*ppd=pivot)+size;
#  if DEBUG_CODE
if (DEBUGGING(MEDIAN_DEBUG)||DEBUGGING(REPIVOT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, karray[0]=%lu, pivot=%p[%lu], *ppd=%p[%lu], *ppe=%p[%lu] */\n",
__func__,source_file,__LINE__,first,beyond,karray[0],(void *)pivot,(pivot-base)/size,(void *)(*ppd),(*ppd-base)/size,(void *)(*ppe),(*ppe-base)/size);
}
#  endif
#if ASSERT_CODE
if ((*ppe<=pivot||(*ppd>pivot))) 
(V)fprintf(stderr,
"/* %s: %s line %d: save_partial=%u, pivot=%p, ppd=%p, *ppd=%p, ppe=%p, *ppe=%p */\n",
__func__,source_file,__LINE__,save_partial,(void *)pivot,(void *)ppd,
NULL!=ppd?(void *)(*ppd):NULL,(void *)ppe,NULL!=ppe?(void *)(*ppe):NULL);
/* verify proper partitioning */
size_t t, u, v, w, x, y, z;
x=nlt, y=neq, z=ngt;
u=(*ppd-base)/size;
v=(*ppe-base)/size-1UL;
t=(*ppf-base)/size-1UL;
if (u>karray[0]) (V)fprintf(stderr, "%s: %s line %d: ppd@%lu, karray[0]=%lu */\n",
__func__,source_file,__LINE__,u,karray[0]);
A(u<=karray[0]);
if (v<karray[0]) (V)fprintf(stderr, "%s: %s line %d: ppe@%lu, karray[0]=%lu */\n",
__func__,source_file,__LINE__,v+1UL,karray[0]);
A(karray[0]<=v);
A(u<=v);
w=test_array_partition(base,first,u,v,t,size,compar,NULL,NULL);
nlt=x, neq=y, ngt=z;
if (w!=u) {
(V)fprintf(stderr,"/* %s: %s line %d: after quickselect: first=%lu, beyond=%lu, nmemb=%lu, r=%lu, pivot@karray[0]=%lu, *ppd=%p@%lu, *ppe=%p@%lu, save_partial=%u: bad partition */\n",
__func__,source_file,__LINE__,first,beyond,nmemb,r,karray[0],(void *)(*ppd),u,
(void *)(*ppe),v+1UL,save_partial);
print_some_array(base,first,beyond-1UL, "/* "," */");
A(0==1);
}
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
w=test_array_partition(base,s,t,u,v,size,compar,NULL,NULL);
nlt=x, neq=y, ngt=z;
if (w!=t) {
(V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, pc@%lu, pd@%lu, pe@%lu, pf@%lu, size=%lu, options=%x: bad partition */\n",
__func__,source_file,__LINE__,first,beyond,s,t,u+1UL,v+1UL,(unsigned long)size,options);
    print_some_array(base,s,v, "/* "," */");
A(0==1);
}
}
#endif
    return pivot;
}
