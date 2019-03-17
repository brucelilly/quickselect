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
* $Id: ~|^` @(#)    isort.c copyright 2016-2019 Bruce Lilly.   \ isort.c $
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
/* $Id: ~|^` @(#)   This is isort.c version 1.13 dated 2019-03-15T14:05:57Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.isort.c */

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
#define ID_STRING_PREFIX "$Id: isort.c ~|^` @(#)"
#define SOURCE_MODULE "isort.c"
#define MODULE_VERSION "1.13"
#define MODULE_DATE "2019-03-15T14:05:57Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define __STDC_WANT_LIB_EXT1__ 0
#define LIBMEDIAN_TEST_CODE 1
#define PURE_INSERTION 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if ! QUICKSELECT_BUILD_FOR_SPEED
# define INSERTION_SORT_SRC_FILE_HERE 1
#endif

#include "insertion_sort_src.h" /* isort_bs */

void isort_internal(char *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    if (beyond>first) {
        char *pa, *pb;
        register size_t n=beyond-first;
#if 0 /* one implementation */
        if (1UL<nmemb) {
            char *pc, *pl, *pu;
            for (pa=pl=base+first*size,pu=base+(beyond-1UL)*size; pa<pu; pa+=size)
                for (pb=pa; (pb>=pl)&&(0<compar(pb,pc=pb+size)); pb-=size)
                    EXCHANGE_SWAP(swapf,pb,pc,size,alignsize,size_ratio,nsw++);
        }
#else
    /* implementation by insertion of first element into sorted remainder
         (except for trivial cases)
    */
        switch (n) {
            case 1UL :
            break;
            case 2UL : /* less overhead than default loop */
                pa=base+first*size;
                pb=pa+size;
                COMPARE_EXCHANGE(pa,pb,options,size,swapf,alignsize,size_ratio);
            break;
#if 0 /* 0 for pure insertion sort for comparison and swap counts */
            case 3UL : /* optimized sort of 3 */
                /* The optimization is the "else", which avoids extraneous
                   comparisons. Average over all permutations: 8/3 comparisons
                   and 7/6 swaps, which is 1/3 comparison less than a sorting
                   network w/o the "else". Insertion sort averages 8/3
                   comparisons and 3/2 swaps, which is more swaps.  No
                   parallelization is possible in this or any other 3-element
                   sorting network.
                */
                { char *pc;
                    pa=base+first*size;
                    pb=pa+size;
                    pc=pb+size;
                    COMPARE_EXCHANGE(pa,pc,options,size,swapf,alignsize,
                        size_ratio);
                    COMPARE_EXCHANGE(pa,pb,options,size,swapf,alignsize,
                        size_ratio);
                    else
                        COMPARE_EXCHANGE(pb,pc,options,size,swapf,alignsize,
                            size_ratio);
                }
            break;
#endif
            default :
                /* Insertion sort iterative loop. */
                /* Implementation by insertion of out-of-place elements into
                   sorted remainder (except for trivial case).  Find extent of
                   longest (preferably central) sorted run of elements. See
                   libmedian insertion_sort.c and insertion.h files.
                */
#if LIBMEDIAN_TEST_CODE
# if  __STDC_WANT_LIB_EXT1__
                d_isort_bs_s
# else
                d_isort_bs
# endif
#else
# if  __STDC_WANT_LIB_EXT1__
                isort_bs_s
# else
                isort_bs
# endif
#endif
                (base,first,beyond,size,compar,swapf,alignsize,
                    size_ratio,options);
            break;
        }
# if ASSERT_CODE > 1
        for (pa=base+first*size,pb=base+beyond*size; pa<pb; pa+=size)
            A(0>=OPT_COMPAR(pa,pa+size,options));
# endif
    }
#endif
}

void isort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),
    unsigned int options)
{
    size_t alignsize=alignment_size(base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        (V)fprintf(stderr, "%s: indirect sorting is not supported\n",__func__);
        return;
    }
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    isort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,options);
}
