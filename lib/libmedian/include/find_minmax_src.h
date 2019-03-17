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
* $Id: ~|^` @(#)    find_minmax_src.h copyright 2019 Bruce Lilly.   \ find_minmax_src.h $
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
/* $Id: ~|^` @(#)   This is find_minmax_src.h version 1.1 dated 2019-03-06T21:31:44Z. \ find_minmax_src.h $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.find_minmax_src.h */

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
#define ID_STRING_PREFIX "$Id: find_minmax_src.h ~|^` @(#)"
#define SOURCE_MODULE "find_minmax_src.h"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-03-06T21:31:44Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2019"

/* local header files needed */
#include "initialize_src.h"

#if __STDC_WANT_LIB_EXT1__
QUICKSELECT_FIND_MINMAX_S
#else
QUICKSELECT_FIND_MINMAX
#endif
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
#if LIBMEDIAN_TEST_CODE
# if __STDC_WANT_LIB_EXT1__
                d_find_minmax_s
# else
                d_find_minmax
# endif
#else
# if __STDC_WANT_LIB_EXT1__
                find_minmax_s
# else
                find_minmax
# endif
#endif
                    (base,first,first+na,size,COMPAR_ARGS,options,&mna,&mxa);
#if LIBMEDIAN_TEST_CODE
# if __STDC_WANT_LIB_EXT1__
                d_find_minmax_s
# else
                d_find_minmax
# endif
#else
# if __STDC_WANT_LIB_EXT1__
                find_minmax_s
# else
                find_minmax
# endif
#endif
                    (base,first+na,beyond,size,COMPAR_ARGS,options,&mnb,&mxb);
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
