/*INDENT OFF*/

/* Description: C source code for quickselect */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    cutoff_value.c copyright 2017 Bruce Lilly.   \ cutoff_value.c $
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
/* $Id: ~|^` @(#)   This is cutoff_value.c version 1.6 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.cutoff_value.c */

/********************** Long description and rationale: ***********************
 Dedicated sorting is used for small sub-arrays to avoid quicksort overhead. The
 largest sub-array size which uses dedicated sorting is given by a table (one of
 several based on options such as partial order stability).
******************************************************************************/

/* Nothing to configure below this line. */

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
#define ID_STRING_PREFIX "$Id: cutoff_value.c ~|^` @(#)"
#define SOURCE_MODULE "cutoff_value.c"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2017-12-22T04:14:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"

/* local header files needed */
#include "quickselect_config.h" /* QUICKSELECT_STABLE */
#include "cutoff_table.h"     /* cutoff_table_struct */
#include "initialize_src.h"

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <stddef.h>             /* size_t */

extern struct cutoff_table_struct cutoff_table[], cutoff_table_c[]
#if QUICKSELECT_STABLE
    , cutoff_table_s[], cutoff_table_sc[]
#endif /* QUICKSELECT_STABLE */
    ;

/* cutoff_value: return sorting cutoff value for dedicated_sort
*/
/* called from qsort and quickselect */
#include "cutoff_value_decl.h"
{
    size_t i=0UL;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    A(0UL!=size_ratio);
    switch (options&((QUICKSELECT_STABLE)|(QUICKSELECT_OPTIMIZE_COMPARISONS))) {
#if QUICKSELECT_STABLE
        case ((QUICKSELECT_STABLE)|(QUICKSELECT_OPTIMIZE_COMPARISONS)) :
            while (size_ratio>cutoff_table_sc[i].min_ratio) i++;
        return cutoff_table_sc[i].cutoff;
        case (QUICKSELECT_STABLE) :
            while (size_ratio>cutoff_table_s[i].min_ratio) i++;
        return cutoff_table_s[i].cutoff;
#endif /* QUICKSELECT_STABLE */
        case (QUICKSELECT_OPTIMIZE_COMPARISONS) :
            while (size_ratio>cutoff_table_c[i].min_ratio) i++;
        return cutoff_table_c[i].cutoff;
    }
    /* default */
    while (size_ratio>cutoff_table[i].min_ratio) i++;
    return cutoff_table[i].cutoff;
}
