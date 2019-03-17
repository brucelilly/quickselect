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
* $Id: ~|^` @(#)    select_minmax_src.h copyright 2019 Bruce Lilly.   \ select_minmax_src.h $
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
/* $Id: ~|^` @(#)   This is select_minmax_src.h version 1.1 dated 2019-03-15T20:53:43Z. \ select_minmax_src.h $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.select_minmax_src.h */

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
#define ID_STRING_PREFIX "$Id: select_minmax_src.h ~|^` @(#)"
#define SOURCE_MODULE "select_minmax_src.h"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-03-15T20:53:43Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2019"

/* local header files needed */
#include "exchange.h"

#include "initialize_src.h"

/* Selection of both minimum and maximum using recursive find_minmax. */
#if __STDC_WANT_LIB_EXT1__
QUICKSELECT_SELECT_MINMAX_S
#else
QUICKSELECT_SELECT_MINMAX
#endif
{
    char *mn, *mx, *a, *z;
    A(beyond>first);
    /* first and last elements */
    a=base+first*size,z=base+(beyond-1UL)*size;
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
        (base,first,beyond,size,COMPAR_ARGS,options,&mn,&mx);
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
            EXCHANGE_SWAP(swapf,a,z,size,alignsize,size_ratio,
                SWAP_COUNT_STATEMENT);
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
                    EXCHANGE_SWAP(swapf,a,mn,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                }
                if (z!=mx) { /* beware case where a was mx before above swap! */
                    EXCHANGE_SWAP(swapf,z,a==mx?mn:mx,size,alignsize,size_ratio,
                        SWAP_COUNT_STATEMENT);
                }
#if QUICKSELECT_STABLE
            }
#endif /* QUICKSELECT_STABLE */
        }
    }
}
