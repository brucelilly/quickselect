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
* $Id: ~|^` @(#)    debug.c copyright 2016-2018 Bruce Lilly.   \ debug.c $
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
/* $Id: ~|^` @(#)   This is debug.c version 1.10 dated 2018-07-12T20:58:16Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.debug.c */

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
#define ID_STRING_PREFIX "$Id: debug.c ~|^` @(#)"
#define SOURCE_MODULE "debug.c"
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2018-07-12T20:58:16Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

const char *debug_name(unsigned int value)
{
    static char buf[32];
    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (value) {
        case AQCMP_DEBUG :
        return "aqcmp";
        case COMPARE_DEBUG :
        return "compare";
        case MEDIAN_DEBUG :
        return "median";
        case PARTITION_ANALYSIS_DEBUG :
        return "analysis";
        case PARTITION_DEBUG :
        return "partitioning";
        case PIVOT_SELECTION_DEBUG :
        return "pivots";
        case PK_ADJUSTMENT_DEBUG :
        return "duplicates";
        case RATIO_GRAPH_DEBUG :
        return "ratio";
        case RECURSION_DEBUG :
        return "recursion";
        case REMEDIAN_DEBUG :
        return "remedian";
        case REPARTITION_DEBUG :
        return "repartition";
        case REPIVOT_DEBUG :
        return "repivoting";
        case SAMPLING_DEBUG :
        return "sampling";
        case SHOULD_REPIVOT_DEBUG :
        return "should_repivot";
        case SORT_SELECT_DEBUG :
        return "sort/select";
        case SUPPORT_DEBUG :
        return "support";
        case SWAP_DEBUG :
        return "swap";
        case WQSORT_DEBUG :
        return "wqsort";
        case MERGE_DEBUG :
        return "merge";
        case DPQSORT_DEBUG :
        return "dpqsort";
        case CACHE_DEBUG :
        return "cache";
        case MEMORY_DEBUG :
        return "memory";
        case METHOD_DEBUG :
        return "method";
        case CORRECTNESS_DEBUG :
        return "correctness";
    }
    (V)snul(buf, sizeof(buf), "unknown value 0x", NULL, value, 16, '0', 8, NULL, NULL);
    return buf;
}
