/*INDENT OFF*/
#error "obsolete file"

/* Description: C source code for selection-related development */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    minmax.c copyright 2016-2019 Bruce Lilly.   \ minmax.c $
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
/* $Id: ~|^` @(#)   This is minmax.c version 1.11 dated 2019-03-16T15:37:11Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.minmax.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/
#error "obsolete file"

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: minmax.c ~|^` @(#)"
#define SOURCE_MODULE "minmax.c"
#define MODULE_VERSION "1.11"
#define MODULE_DATE "2019-03-16T15:37:11Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define LIBMEDIAN_TEST_CODE 1
#define __STDC_WANT_LIB_EXT1__ 0

#error "obsolete file"
/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#error "obsolete file"
void find_minmax(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *,const void *), unsigned int options,
    char **pmn, char **pmx)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) { (V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond);
}
#endif
    A(beyond>first);
    {
        size_t nmemb=beyond-first;
        if (1UL<nmemb) {
            char *mn, *mx;
            if (2UL<nmemb) {
                /* split; divide-and-conquer saves comparisons */
                size_t na=(nmemb>>1);
                char *mna, *mnb, *mxa, *mxb;
                /* find min and max of both parts */
                find_minmax(base,first,first+na,size,compar,options,&mna,&mxa);
                find_minmax(base,first+na,beyond,size,compar,options,&mnb,&mxb);
                /* overall min is smaller of *mna, *mnb; similarly for max */
                /* stability requires choosing mna if mna compares equal to mnb */
                if (0<OPT_COMPAR(mna,mnb,options)) mn=mnb; else mn=mna;
                /* stability requires choosing mxb if mxa compares equal to mxb */
                if (0<OPT_COMPAR(mxa,mxb,options)) mx=mxa; else mx=mxb;
            } else { /* nmemb==2UL */
                char *a, *z;
                /* first and last (i.e. second) elements */
                a=base+first*size,z=a+size;
                if (0<OPT_COMPAR(a,z,options)) mn=z,mx=a; else mn=a,mx=z; /* stable */
            }
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) { (V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, min=%lu, max=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,
(mn-base)/size,(mx-base)/size);
}
#endif
            *pmn=mn, *pmx=mx;
        } else /* 1 element; min==max */
            *pmn=*pmx=base+first*size;
    }
}

void select_minmax(char *base,size_t first,size_t beyond,size_t size,
    int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    char *mn, *mx, *a, *z;

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V) fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond);
#endif
    A(beyond>first);
    /* first and last elements */
    a=base+first*size,z=base+(beyond-1UL)*size;
    find_minmax(base,first,beyond,size,compar,options,&mn,&mx);
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) { (V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, min=%lu, max=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond,
(mn-base)/size,(mx-base)/size);
print_some_array(base,first,beyond-1UL,"/* "," */",options);
}
#endif
    /* put min and max in place with at most 2 swaps */
    /* stability requires rotation rather than swaps */
    /* +----------------------------------+ */
    /* |                                  | */
    /* +----------------------------------+ */
    /* +a         x               n      z+ */
    /* nothing to do if mx and mn are in place, i.e. mn==a and mx==z */
    if ((mn!=a)||(mx!=z)) {
        /* special case: mx==a and mn==z */
        if ((mx==a)&&(mn==z)) {
            EXCHANGE_SWAP(swapf,a,z,size,alignsize,size_ratio,nsw++);
        } else { /* rotations required to preserve stability */
#if QUICKSELECT_STABLE
            if (0U!=((QUICKSELECT_STABLE)&options)) {
                if (a!=mn) {
                    protate(a,mn,mn+size,size,swapf,alignsize,size_ratio);
                    if (mx<mn) mx+=size; /* mx was moved by rotation */
                }
                if (z!=mx)
                    protate(mx,mx+size,z+size,size,swapf,alignsize,size_ratio);
            } else {
#endif /* QUICKSELECT_STABLE */
                if (a!=mn) {
                    EXCHANGE_SWAP(swapf,a,mn,size,alignsize,size_ratio,nsw++);
                }
                if (z!=mx) { /* beware case where a was mx before above swap! */
                    EXCHANGE_SWAP(swapf,z,a==mx?mn:mx,size,alignsize,size_ratio,
                        nsw++);
                }
#if QUICKSELECT_STABLE
            }
#endif /* QUICKSELECT_STABLE */
        }
    }
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) { (V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu */\n",
__func__,source_file,__LINE__,(unsigned long)first,(unsigned long)beyond);
print_some_array(base,first,beyond-1UL,"/* "," */",options);
}
#endif
}
#error "obsolete file"
