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
* $Id: ~|^` @(#)    test_array_partition.c copyright 2016-2019 Bruce Lilly.   \ test_array_partition.c $
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
/* $Id: ~|^` @(#)   This is test_array_partition.c version 1.1 dated 2019-02-27T21:08:40Z. \ test_array_partition.c $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.test_array_partition.c */

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
#define ID_STRING_PREFIX "$Id: test_array_partition.c ~|^` @(#)"
#define SOURCE_MODULE "test_array_partition.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-02-27T21:08:40Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

/* header files needed */
#include "median_test_config.h" /* configuration */

#include "initialize_src.h"

/* Array pv should be partitioned such that elements at indices < pl are
      not larger than element at pl, elements at indices > pu are not smaller
      than element at index pu, and elements from index pl through pu compare
      as equal.
   l <= pl <= pu <= u
   If tests hold for all indices between l and u inclusive, return pl.
   If arguments are not valid, return >= u+1UL.
   On the first failed test, return the index of the item failing the test.
*/

size_t test_array_partition(const char *pv, size_t l, size_t pl,
    size_t pu, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, n, lim;
    const char *p, *pe;
    unsigned int odebug=debug;
    int (*compar)(const void *,const void *)=uninstrumented_comparator(icompar);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l > pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > pl (%lu)", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)pl);
        else fprintf(stderr, "/* %s: %s line %d: l (%lu) > pl (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)pl);
        return u+1UL;
    }
    if (pu > u) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) > u (%lu)", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)u);
        else fprintf(stderr, "/* %s: %s line %d: pu (%lu) > u (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)u);
        return u+2UL;
    }
    if (pu < pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) < pl (%lu)", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)pl);
        else fprintf(stderr, "/* %s: %s line %d: pu (%lu) < pl (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)pl);
        return u+3UL;
    }
    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        else fprintf(stderr, "/* %s: %s line %d: NULL pv */\n", __func__, source_file, __LINE__);
        return u+4UL;
    }
    debug=0U;
    pe=pv+pl*size;
    if (pl >= l+1UL)
        for (i=pl-1UL; ; i--) {
            p=pv+i*size;
            if (0U==(options&(QUICKSELECT_INDIRECT))) {
                c=compar(p,pe);
            } else {
                c=compar(*((char *const *)(p)),*((char *const *)(pe)));
            }
            if (0 < c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                debug=odebug;
                return i;
            }
            if (i < l+1UL) break;
        }
    for (i=pl+1UL; i<=pu; i++) {
        p=pv+i*size;
        if (0U==(options&(QUICKSELECT_INDIRECT))) {
            c=compar(p,pe);
        } else {
            c=compar(*((char *const *)(p)),*((char *const *)(pe)));
        }
        if (0 != c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            debug=odebug;
            return i;
        }
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    for (i=pu+1UL; i<=lim; i++) {
        p=pv+i*size;
        if (0U==(options&(QUICKSELECT_INDIRECT))) {
            c=compar(p,pe);
        } else {
            c=compar(*((char *const *)(p)),*((char *const *)(pe)));
        }
        if (0 > c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            debug=odebug;
            return i;
        }
    }
    debug=odebug;
    return pl;
}
