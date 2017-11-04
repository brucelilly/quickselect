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
* $Id: ~|^` @(#)    mergesort.c copyright 2016-2017 Bruce Lilly.   \ mergesort.c $
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
/* $Id: ~|^` @(#)   This is mergesort.c version 1.4 dated 2017-11-03T19:52:29Z. \ $ */
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
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-11-03T19:52:29Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* in-place mergesort */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void mergesort_internal(char *base, size_t first, size_t beyond,
    const size_t size, int (*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int network_map)
{
    size_t nmemb=beyond-first;
    /* If nmemb > 1, split into two pieces, sort the two pieces (recursively),
       then merge the two sorted pieces.
    */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: base=%p, nmemb=%lu, size=%lu */\n",
__func__,source_file,__LINE__,(void *)base,nmemb,size);
    print_some_array(base,0UL,nmemb-1UL, "/* "," */");
}
#endif
    if (1UL<nmemb) {
        if (merge_cutoff>=nmemb) { /* special case: very small sub-array */
            /* network (2), optimized (3) */
            /* insertion (4-7), merge (8+) or merge (4+) */
            dedicated_sort(base,first,beyond,size,compar,swapf,alignsize,
                size_ratio,network_map);
        } else {
            register char *pa, *pb, *pu; size_t mid, na;
            /* split */
            na=(nmemb>>1); mid=first+na, pa=base+first*size, pb=pa+na*size;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,"/* %s: %s line %d: base=%p, na=%lu, pb=%p */\n",
__func__,source_file,__LINE__,(void *)base,na,(void *)pb);
#endif
            /* sort pieces */
            /* Sorting pieces could proceed in parallel.
               2017-09-28:Intel cilk parallelization has much too much overhead.
                  A lower-overhead means of parallelization is needed to make
                  parallel sorting of pieces practical.
            */
            mergesort_internal(base,first,mid,size,compar,swapf,alignsize,
                size_ratio,network_map);
            mergesort_internal(base,mid,beyond,size,compar,swapf,alignsize,
                size_ratio,network_map);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: concatenated sorted halves: base=%p, na=%lu, pb=%p[%lu] */\n",
__func__,source_file,__LINE__,(void *)base,na,(void *)pb,(pb-base)/size);
    print_some_array(base,0UL,nmemb-1UL, "/* "," */");
}
#endif
#if ASSERT_CODE > 1
 {
char *pc, *pe=base+beyond*size;
size_t oneq=neq, ongt=ngt, onlt=nlt;
for (pc=base+first*size+size; pc<pb; pc+=size)
    A(0<=compar(pc,pc-size));
for (pc=pb+size; pc<pe; pc+=size) {
    int c=compar(pc,pc-size);
    if (0>c) { fprintf(stderr,"/* pb@%lu, pc@%lu, pe@%lu */ */\n",(pb-base)/size,(pc-base)/size,(pe-base)/size);
        print_some_array(base,first,beyond-1UL, "/* "," */");
    }
    A(0<=c);
}
neq=oneq, ngt=ongt, nlt=onlt;
}
#endif
            /* merge pieces */
#if COMPARE1
            if (0<compar(pb-size,pb)) {
#endif /* COMPARE1 */
                switch (alignsize) {
                    case 8UL :
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        MERGE(int_least64_t,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
#else
# if ( LONG_MAX > 2147483647 ) && ( LONG_MAX == 9223372036854775807 )
                        MERGE(long,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
# elif DBL_MAX_10_EXP == 308
                        MERGE(double,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
# else
                        MERGE(char *,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
# endif
#endif /* C99 */
                    break;
                    case 4UL :
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        MERGE(int_least32_t,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
#else
# if INT_MAX == 2147483647
                        MERGE(int,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
# elif LONG_MAX == 2147483647
                        MERGE(long,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
# endif
#endif /* C99 */
                    break;
                    case 2UL :
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
                        MERGE(int_least16_t,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
#else
                        MERGE(short,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
#endif /* C99 */
                    break;
                    default :
                        MERGE(char,base,first,beyond,pa,pb,size,
                            pu,cx,swapf,alignsize,size_ratio)
                    break;
                }
                nmerges++;
#if COMPARE1
            }
#endif /* COMPARE1 */
#if ASSERT_CODE > 1
 {
char *pf=base+first*size, *pg=base+beyond*size;
size_t oneq=neq, ongt=ngt, onlt=nlt;
for (pf+=size; pf<pg; pf+=size)
    A(0<=compar(pf,pf-size));
neq=oneq, ngt=ongt, nlt=onlt;
}
#endif
        }
    }
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V) fprintf(stderr,"/* %s: %s line %d: base=%p, nmemb=%lu */\n",
__func__,source_file,__LINE__,(void *)base,nmemb);
    print_some_array(base,0UL,nmemb-1UL, "/* "," */");
}
#endif
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
/* avoid namespace clash... */
void xmergesort(char *base, size_t nmemb,
    const size_t size, int (*compar)(const void *,const void *))
{
    size_t alignsize=alignment_size((char *)base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    /* If nmemb > 1, split into two pieces, sort the two pieces (recursively),
       then merge the two sorted pieces.
    */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,"/* %s: %s line %d: nmemb=%lu, size=%lu */\n",
__func__,source_file,__LINE__,nmemb,size);
#endif
    if (1UL<nmemb) {
        mergesort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,
            size_ratio,0x0EU);
    }
}
