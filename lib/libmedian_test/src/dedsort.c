/*INDENT OFF*/

/* Description: C source code for dedicated_sort */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    dedsort.c copyright 2019 Bruce Lilly.   \ dedsort.c $
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
/* $Id: ~|^` @(#)   This is dedsort.c version 1.1 dated 2019-03-15T20:44:15Z. \ dedsort.c $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.dedsort.c */

/********************** Long description and rationale: ***********************
 This file contains source to implement the internal function dedicated_sort.
 Swapping of elements is handled by efficient inline swap functions.  No attempt
 is made to optimize for specific array element types, but swapping can be
 performed in units of basic language types.  The swap functions avoid useless
 work (e.g. when given two pointers to the same element).  No specialized macros
 are required.
******************************************************************************/

/* Nothing to configure below this line. */

#undef QUICKSELECT_BUILD_FOR_SPEED
#define QUICKSELECT_BUILD_FOR_SPEED 0

/* external visibility */
#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

/* C11 _s variant */
#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#define LIBMEDIAN_TEST_CODE 1

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components
   are version control fields.
   ID_STRING_PREFIX is suitable for the what(1) and ident(1) utilities.
   MODULE_DATE uses modern SCCS extensions.
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: dedsort.c ~|^` @(#)"
#define SOURCE_MODULE "dedsort.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-03-15T20:44:15Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2019"

#include "median_test_config.h"

#include "indirect.h"           /* rearrange_array convert_pointers_to_indices
                                */

#include "initialize_src.h"

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

/* N.B. With dedicated_sort part of quickselect_loop, this function (which
   calls quickselect_loop) is essentially equivalent to qsort.
*/
/* wrapper */
void dedsort(char *base, size_t nmemb, size_t size, COMPAR_DECL,
    unsigned int options)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Determine cache size once on first call. */
    if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
    if (1UL<nmemb) {
        size_t alignsize=alignment_size(base,size);
        size_t size_ratio=size/alignsize;
        void (*swapf)(char *, char *, size_t);
        size_t *indices = NULL;
        char **pointers = NULL;
        void (*pswapf)(char *, char *, size_t);

        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            char **p=set_array_pointers(pointers,nmemb,base,size,0UL,nmemb);
            if (NULL==p) {
                if (NULL!=pointers) { free(pointers); pointers=NULL; }
                goto no_ind;
            } else {
                size_t n;
                if (p!=pointers) pointers=p;
                if (0U==instrumented) pswapf=swapn(sizeof(char *));
                else pswapf=iswapn(sizeof(char *));
                /* pswapf is guaranteed to have been assigned a value by the
                   above statement (and always if options contains
                   QUICKSELECT_INDIRECT after this block), whether or not (:-/)
                   gcc's authors realize it...
                */
                A(pointers[0]==base);
                (V)d_quickselect_loop((char *)pointers,0UL,nmemb,sizeof(char *),
                    COMPAR_ARGS,NULL,0UL,0UL,pswapf,sizeof(char *),1UL,
                    quickselect_cache_size,0UL,options,NULL,NULL);
                if (NULL==indices) indices=(size_t *)pointers;
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,indices,0UL,nmemb);
                A(NULL!=indices);
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V) fprintf(stderr,"/* %s: %s line %d: indices:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%lu",n,indices[n]);
                    (V) fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,"/* base array: "," */",
                        options);
                }
#endif
                n=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                    alignsize);
                free(pointers);
                if ((void *)indices==(void *)pointers) indices=NULL;
                pointers=NULL;
                if (nmemb<(n>>1)) { /* indirection failed; use direct sort/select */
                    /* shouldn't happen */
                    (V)fprintf(stderr,"/* %s: %s line %d: size_ratio=%lu, nmemb"
                        "=%lu, indirect sort failed */\n",__func__,source_file,
                        __LINE__,size_ratio,nmemb);
no_ind:             options&=~(QUICKSELECT_INDIRECT);
                    (V)d_quickselect_loop(base,0UL,nmemb,size,COMPAR_ARGS,NULL,
                        0UL,0UL,swapf,alignsize,size_ratio,
                        quickselect_cache_size,0UL,options,NULL,NULL);
                } else if (0UL!=n)
                    nmoves+=n*size_ratio;
            }
        } else
            (V)d_quickselect_loop(base,0UL,nmemb,size,COMPAR_ARGS,NULL,0UL,0UL,
                swapf,alignsize,size_ratio,quickselect_cache_size,
                0UL,options,NULL,NULL);
    }
}
