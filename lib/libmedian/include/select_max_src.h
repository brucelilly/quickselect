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
* $Id: ~|^` @(#)    select_max_src.h copyright 2019 Bruce Lilly.   \ select_max_src.h $
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
/* $Id: ~|^` @(#)   This is select_max_src.h version 1.2 dated 2019-03-26T20:43:06Z. \ select_max_src.h $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.select_max_src.h */

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
#define ID_STRING_PREFIX "$Id: select_max_src.h ~|^` @(#)"
#define SOURCE_MODULE "select_max_src.h"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2019-03-26T20:43:06Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2019"

/* local header files needed */
#include "exchange.h"

#include "initialize_src.h"

/* Selection of maximum. */
#if __STDC_WANT_LIB_EXT1__
QUICKSELECT_SELECT_MAX_S
#else
QUICKSELECT_SELECT_MAX
#endif
{
#if LIBMEDIAN_TEST_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)
    ||DEBUGGING(REPARTITION_DEBUG)
    )
        (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, ppeq=%p, "
            "ppgt=%p, options=0x%x */\n",__func__,source_file,__LINE__,
            (unsigned long)first,(unsigned long)beyond,(void *)ppeq,
            (void *)ppgt,options);
#endif
    if (beyond>first+1UL) {
        char *mx, *p, *r;
        p=base+first*size,mx=r=base+(beyond-1UL)*size;
        /* both ppeq and ppgt are NULL, or both are non-NULL */
        A(((NULL==ppeq)&&(NULL==ppgt))||((NULL!=ppeq)&&(NULL!=ppgt)));
        if ((NULL==ppeq)/*&&(NULL==ppgt)*/) { /* non-partitioning */
            /* separate indirect/direct versions of loop to avoid repeated
               option testing in loop, cache dereferenced mx
            */
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                char *x=*((char **)mx);
                for (; p<r; p+=size)
                    if (0>COMPAR(x,*((char **)p))) x=*((char **)(mx=p));
            } else
                for (; p<r; p+=size)
                    if (0>COMPAR(mx,p)) mx=p;
            if (mx!=r) {
#if QUICKSELECT_STABLE
                /* stability requires rotation rather than swaps */
                if (0U==((QUICKSELECT_STABLE)&options)) {
#endif /* QUICKSELECT_STABLE */
                    EXCHANGE_SWAP(swapf,r,mx,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
#if QUICKSELECT_STABLE
                } else {
                    /* |      mx     r| */
                    protate(mx,mx+size,r+size,size,swapf,alignsize,size_ratio);
                }
#endif /* QUICKSELECT_STABLE */
            }
        } else { /* partitioning for median-of-medians */
            /* sorting stability is not an issue if median-of-medians is used */
            char *q;
            /* separate indirect/direct versions of loop to avoid repeated
               option testing in loop, cache dereferenced mx
            */
            if (0U!=(options&(QUICKSELECT_INDIRECT))) {
                char *x=*((char **)mx);
                for (q=r-size; q>=p; q-=size) {
                    int c=COMPAR(x,*((char **)q));
                    if (0<c) continue;
                    if (0==c) {
                        mx-=size;
                        if (q==mx) continue;
                    } else /* 0>c */ mx=r;
                    A(mx!=q);
                    EXCHANGE_SWAP(swapf,mx,q,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                    x=*((char **)mx); /* new max */
                }
            } else {
                for (q=r-size; q>=p; q-=size) {
                    int c=COMPAR(mx,q);
                    if (0<c) continue;
                    if (0==c) {
                        mx-=size;
                        if (q==mx) continue;
                    } else /* 0>c */ mx=r; /* new max */
                    A(mx!=q);
                    EXCHANGE_SWAP(swapf,mx,q,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                }
            }
            if (NULL!=ppeq) *ppeq=mx; /* leftmost max */
            if (NULL!=ppgt) *ppgt=r+size; /* beyond rightmost max */
#if LIBMEDIAN_TEST_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG)
            ||DEBUGGING(REPARTITION_DEBUG)
            ) {
                (V)fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, "
                    "ppeq=%p@%lu, ppgt=%p@%lu */\n",__func__,source_file,
                    __LINE__,(unsigned long)first,(unsigned long)beyond,
                    (void *)ppeq,(mx-base)/size,(void *)ppgt,
                    (r+size-base)/size);
                print_some_array(base,first,beyond-1UL,"/* "," */",options);
            }
#endif
        }
    } else { /* beyond==first+1UL; 1 element */
        if (NULL!=ppeq) *ppeq=base+first*size;
        if (NULL!=ppgt) *ppgt=base+beyond*size;
    }
}
