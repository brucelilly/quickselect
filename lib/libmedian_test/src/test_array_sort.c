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
* $Id: ~|^` @(#)    test_array_sort.c copyright 2019 Bruce Lilly.   \ test_array_sort.c $
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
/* $Id: ~|^` @(#)   This is test_array_sort.c version 1.1 dated 2019-02-27T21:08:51Z. \ test_array_sort.c $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.test_array_sort.c */

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
#define ID_STRING_PREFIX "$Id: test_array_sort.c ~|^` @(#)"
#define SOURCE_MODULE "test_array_sort.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-02-27T21:08:51Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2019"

/* header files needed */
#include "median_test_config.h" /* configuration */

#include "initialize_src.h"

/* Test array for sorting correctness */
size_t test_array_sort(const char *pv, size_t l, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    unsigned int distinct, void(*f)(int, void *, const char *, ...),
    void *log_arg)
{
    int c;
    size_t i;
    unsigned int odebug=debug;
    int (*compar)(const void *,const void *)=uninstrumented_comparator(icompar);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l > u) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > u (%lu)", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)u);
        return u+1UL;
    }
    if (NULL == pv) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+2UL;
    }
#if 0
    debug=0U;
#endif
    for (i=l; i<u; i++) {
        if (0U==(options&(QUICKSELECT_INDIRECT))) {
            c=compar(pv+i*size,pv+(i+1UL)*size);
        } else {
            c=compar(*((char *const *)(pv+i*size)),*((char *const *)(pv+(i+1UL)*size)));
        }
        if (0 <= c) {
            if ((0U==distinct)&&(0==c)) continue; /* uniqueness not required */
            if (NULL != f)
                f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at index %lu", __func__, source_file, __LINE__, (unsigned long)i);
            debug=odebug;
            return i;
        }
    }
    debug=odebug;
    return u;
}
