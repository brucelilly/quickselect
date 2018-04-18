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
* $Id: ~|^` @(#)    shellsort.c copyright 2016-2018 Bruce Lilly.   \ shellsort.c $
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
/* $Id: ~|^` @(#)   This is shellsort.c version 1.4 dated 2018-03-06T21:46:43Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.shellsort.c */

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
#define ID_STRING_PREFIX "$Id: shellsort.c ~|^` @(#)"
#define SOURCE_MODULE "shellsort.c"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2018-03-06T21:46:43Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

size_t gaps[] = { 1, 4, 10, 23, (SIZE_MAX) }; /* for small nmemb */
size_t ngaps = sizeof(gaps)/sizeof(size_t);

QUICKSELECT_INLINE
void shellsort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    size_t h, n, nmemb, o;
    char *pa, *pb, *pc, *pl, *pu;

    pl=base+first*size;
    pu=base+beyond*size;
    /* loop, decreasing gap */
    /* obtain largest applicable gap h */
    for (nmemb=beyond-first,n=0UL,h=gaps[n]; h<=nmemb; h=gaps[++n]) { ; }
    for (h=gaps[--n]; ; h=gaps[--n]) {
        for (o=h*size,pa=pc=pl+o; pa<pu; pa+=size) {
/* Might be able to improve this:
   2nd gap is <5: use linear-search insertion sort with rotation for insertion on last pass
   might reduce swaps compared to ``waltzing'' insertion sort
   but Shellsort will always use more comparisons than insertion sort.
*/
            for (pb=pa; (pb>=pc)&&(0<compar(pb-o,pb)); pb-=o) {
                EXCHANGE_SWAP(swapf,pb-o,pb,size,alignsize,size_ratio,nsw++);
            }
        }
        if (1UL==h) return;
    }
}

QUICKSELECT_INLINE
void shellsort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    size_t alignsize=alignment_size(base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    shellsort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio);
}
