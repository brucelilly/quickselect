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
* $Id: ~|^` @(#)    sort5.c copyright 2016-2017 Bruce Lilly.   \ sort5.c $
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
/* $Id: ~|^` @(#)   This is sort5.c version 1.3 dated 2017-12-06T23:17:13Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.sort5.c */

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
#define ID_STRING_PREFIX "$Id: sort5.c ~|^` @(#)"
#define SOURCE_MODULE "sort5.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2017-12-06T23:17:13Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void sort5(char *pa, char *pb, char *pc, char *pd, char *pe, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* Batcher sorting network: 9 comparisons in 5 groups, 0-9 exchanges */
    COMPARE_EXCHANGE(pa,pe,options,cx,size,swapf,alignsize,size_ratio);
    /* parallel group */
    COMPARE_EXCHANGE(pa,pc,options,cx,size,swapf,alignsize,size_ratio);
    COMPARE_EXCHANGE(pb,pd,options,cx,size,swapf,alignsize,size_ratio);
    /* parallel group */
    COMPARE_EXCHANGE(pa,pb,options,cx,size,swapf,alignsize,size_ratio);
    COMPARE_EXCHANGE(pc,pe,options,cx,size,swapf,alignsize,size_ratio);
    /* parallel group */
    COMPARE_EXCHANGE(pc,pd,options,cx,size,swapf,alignsize,size_ratio);
    COMPARE_EXCHANGE(pb,pe,options,cx,size,swapf,alignsize,size_ratio);
    /* parallel group */
    COMPARE_EXCHANGE(pb,pc,options,cx,size,swapf,alignsize,size_ratio);
    COMPARE_EXCHANGE(pd,pe,options,cx,size,swapf,alignsize,size_ratio);
}
