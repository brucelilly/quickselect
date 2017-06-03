/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: sorted_array - insert and search in a sorted array of items
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    sorted_array.c copyright 2011 - 2016 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is sorted_array.c version 1.6 2016-05-31T14:22:59Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "sorted_array" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/src/s.sorted_array.c */

/********************** Long description and rationale: ***********************
* An array (of pointers to data) kept in sorted order can be efficiently
* searched to find an item using simple code with low overhead (both in terms
* of storage and code).  Maintaining sorted order during insertions and
* deletions, however, is O(N).  A sorted array, therefore, is useful as a
* dictionary where insertions, deletions, and reordering are infrequent
* compared to search/find operations.
******************************************************************************/

/* source module identification */
/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: sorted_array.c ~|^` @(#)"
#define SOURCE_MODULE "sorted_array.c"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2016-05-31T14:22:59Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2016"

/* configuration (which might affect feature test macros) */
/* to include a main entry point for testing, compile with -DTESTING=1 */
#ifndef TESTING
# define TESTING 0
#endif

/* feature test macros must appear before any header file inclusion */
/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

#include "sorted_array.h"      /* public definitions and declarations */ /* includes sys/types.h */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <errno.h>              /* errno E* */
#include <stdlib.h>             /* NULL malloc calloc realloc free */

/* static data and function definitions */
static char sorted_array_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_sorted_array(void)
{
    sorted_array_initialized
        = register_build_strings(NULL, &source_file, filenamebuf);
}

/* try to find item it in array (of pointers) arr with nelem elements using
      compar to compare elements
   put array index where it should be in *pindex
   return 0 if the item is already in arr, non-zero if not
   implemented as binary search
*/
int arrayfind(void **arr, size_t nelem, size_t *pindex, void *it,
    int (*compar)(const void *, const void *))
{
    int c = -1, l = 0, m, u;

    if ((unsigned char)0U == sorted_array_initialized)
        initialize_sorted_array();
    if ((NULL != arr) && (1 <= nelem)) {
        /* >> 1 is a fast version of / 2 */
        for (u=nelem-1,m=l+((u-l+1)>>1); l<=u; ) {
            c = compar(it, arr[m]);
            if (c < 0) {
                u = m - 1;
                m = l + (m - l)>>1;
            } else if (c > 0) {
                l = m + 1;
                m += 1 + (u-m)>>1;
            } else {
                l = m;
                break;
            }
        }
    } else
        errno = EINVAL;
    if (NULL != pindex)
        *pindex = l;
    return c;
}

/* insert an item pointed to by it into an expanded array of items initially
      containing nelem pointers, using compar as a comparison function
   return a pointer to the expanded array
   put index of new (or existing) item in *pwhere
   return NULL with errno set on error
*/
void **arrayinsert(void **arr, size_t nelem, size_t *pwhere, void *it,
    int (*compar)(const void *, const void *))
{
    void **p;
    int c;
    size_t l, u, new_sz;

    if ((unsigned char)0U == sorted_array_initialized)
        initialize_sorted_array();
    new_sz = (nelem + 1) * sizeof(void *);
    if (NULL == arr) {
        if (0 != nelem) {
            errno = EINVAL;
            return NULL;
        }
        p = (void **)malloc(new_sz);
        if (NULL != p) {
            p[0] = it;
            if (NULL != pwhere)
                *pwhere = 0;
        }
    } else {
        if (1 > nelem) {
            errno = EINVAL;
            return NULL;
        }
        /* where should new item it be inserted? */
        c = arrayfind(arr, nelem, &l, it, compar);
        if (NULL != pwhere)
            *pwhere = l;
        if (0 == c) {
            errno = EEXIST;
            return NULL;
        }
        p = (void **)realloc(arr, new_sz);
        if (NULL != p) {
            for (u=nelem; u>l; u--)
                p[u] = p[u-1];
            p[l] = it;
        }
    }
    return p;
}
