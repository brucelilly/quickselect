/*INDENT OFF*/

/* Description: C source code for set_array_pointers for indirect sorting */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    indirect.c copyright 2017-2018 Bruce Lilly.   \ indirect.c $
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
/* $Id: ~|^` @(#)   This is indirect.c version 1.7 dated 2018-03-06T23:28:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "indirect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.indirect.c */

/********************** Long description and rationale: ***********************
Sorting an array of items generally involves swapping of array elements.  If
elements are large compared to basic type sizes, swapping can become an
appreciable part of the total cost of sorting.  When sorting large elements, it
can be more efficient to set up an array of pointers to elements, indirectly
sorting by rearranging the pointers, which provide a means of accessing the
(unchanged) array elements in sorted order via the (rearranged) pointers.
Function set_array_pointers initializes an array of pointers to point to array
elements, allocating the array of pointers if necessary. It returns a pointer
to the initialized array of pointers.
Rearranging elements is more efficient using indices rather than pointers to
elements; function convert_pointers_to_indices performs that conversion.
******************************************************************************/

/* Nothing to configure below this line. */

/* Minimum _XOPEN_SOURCE version for C99 (else illumos compilation fails) */
#undef MAX_XOPEN_SOURCE_VERSION
#undef MIN_XOPEN_SOURCE_VERSION
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) \
&& defined(MIN_XOPEN_SOURCE_VERSION) \
&& ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) \
&& defined(MAX_XOPEN_SOURCE_VERSION) \
&& ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

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
#define ID_STRING_PREFIX "$Id: indirect.c ~|^` @(#)"
#define SOURCE_MODULE "indirect.c"
#define MODULE_VERSION "1.7"
#define MODULE_DATE "2018-03-06T23:28:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"

/* compile-time configuration options */
/* assertions for validation testing */
#ifndef ASSERT_CODE
# define ASSERT_CODE                     0 /* Adds size & cost to aid debugging.
                                              0 for tested production code. */
                                           /* If ASSERT_CODE > 1, assertions
                                              might also affect the number of
                                              comparisons used.
                                           */
#endif

/* local header files needed */
#include "indirect.h"
#include "zz_build_str.h"       /* build_id build_strings_registered */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201101L) /* C11 */
# include <stdalign.h>          /* alignas alignof etc. */
#endif /* C11 */
#include <stddef.h>             /* size_t NULL */
#if ASSERT_CODE
# include <stdio.h>
#endif
#include <stdlib.h>             /* aligned_alloc(C11) malloc posix_memalign(POSIX(maybe)) */
#include <string.h>             /* strrchr */

/* macros */
/* space-saving abbreviations */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* Half of the pointers allocated for indirect merge sort are for temporary
   use during merges, and are otherwise ignored.  These pointers point to
   random addresses or array elements, but they can be forced to be cleared to
   NULL pointers by defining INDIRECT_MERGE_ZERO_UNUSED_POINTERS as a non-zero
   value.
*/
#define INDIRECT_MERGE_ZERO_UNUSED_POINTERS 0

/* static data */
static char indirect_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* initialize at run-time */
static INDIRECT_INLINE void initialize_indirect(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s) s = filenamebuf; else s++;
    indirect_initialized = register_build_strings(NULL, &source_file, s);
}

char **set_array_pointers(char **pointers, size_t nptrs, char *base,
    size_t size, size_t first, size_t beyond)
{
    register size_t nmemb;

    /* Argument validity check. */
    if ((NULL==base)
    || (0UL==size)
    || (first>beyond)
    || (nptrs < (nmemb=beyond-first)) /* N.B. AFTER ensuring beyond >= first */
    || (0UL==nmemb) /* N.B. AFTER assignment to nmemb! */
    ) {
        errno=EINVAL;
        return NULL;
    }
    if ((char)0 == indirect_initialized) initialize_indirect();
    /* Allocate pointers array if not supplied. */
    if (NULL==pointers) {
#if QUICKSELECT_USE_ALIGNED_ALLOC && defined(__STDC__) \
&& ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201101L) /* C11 */
        pointers=(char **)aligned_alloc((size_t)(QUICKSELECT_DEFAULT_ALIGNMENT),
            sizeof(char *)*nptrs);
#elif QUICKSELECT_USE_POSIX_MEMALIGN
        int r=posix_memalign((void **)(&pointers),
            (size_t)(QUICKSELECT_DEFAULT_ALIGNMENT),sizeof(char *)*nptrs);
        if (r!=0) {
            pointers=NULL; /* maybe changed by posix_memalign */
            errno=r;
        }
#else
        pointers=(char **)malloc(sizeof(char *)*nptrs);
#endif
    }
    /* Initialize pointers to elements (unless allocation failed). */
    /* pointer[0] -> base+first*size, etc. */
    if (NULL!=pointers) {
        register size_t n;
        register char *p;

        for (n=0UL,p=base+first*size; n<nmemb; n++,p+=size) pointers[n]=p;
#if INDIRECT_MERGE_ZERO_UNUSED_POINTERS
        for (; n<nptrs; n++) pointers[n]=NULL;
#endif
    }
    return pointers;
}

size_t *convert_pointers_to_indices(char *base, size_t nmemb, size_t size,
    char **pointers, size_t nptrs, size_t *indices, size_t first, size_t beyond)
{
    /* Check argument validity. */
    if ((nptrs < nmemb)
    || (((char *)pointers==(char *)indices)&&(sizeof(size_t)>sizeof(char *)))
    || (NULL==base)
    || (NULL==indices)
    || (0UL==size)
    || (first>beyond)
    || (first+nmemb<beyond)
    ) {
        errno=EINVAL;
        return NULL;
    }
    if (0UL<nmemb) {
        register size_t n;
#if ASSERT_CODE
        register size_t i;
        char *pl=base+first*size, *p, *pu=base+beyond*size;
#endif /* ASSERT_CODE */
        if ((char)0 == indirect_initialized) initialize_indirect();
#if ASSERT_CODE
        /* pointer sanity check */
        for (n=0UL,i=0UL; i<nptrs; i++) {
            p=pointers[i];
            if ((pl<=p)&&(p<pu)) continue;
            (V)fprintf(stderr,
                "%s: %s line %d: pointers[%lu]=%p is out of bounds: base=%p, "
                "beyond@%p: %ld\n",__func__,source_file,__LINE__,i,(void *)p,
                (void *)base,(void *)pu,(long)(p-base)/(long)size);
            n++;
        }
        A(0UL==n);
#endif /* ASSERT_CODE */
        /* Allocate array of indices if not provided. */
        if (NULL==indices) {
#if QUICKSELECT_USE_ALIGNED_ALLOC && defined(__STDC__) \
&& ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201101L) /* C11 */
            indices=(size_t *)aligned_alloc(
                (size_t)(QUICKSELECT_DEFAULT_ALIGNMENT),sizeof(size_t)*nptrs);
#elif QUICKSELECT_USE_POSIX_MEMALIGN
            int r=posix_memalign((void **)(&indices),(
                size_t)(QUICKSELECT_DEFAULT_ALIGNMENT),sizeof(size_t)*nptrs);
            if (r!=0) {
                indices=NULL; /* maybe changed by posix_memalign */
                errno=r;
            }
#else
            indices=(size_t *)malloc(sizeof(size_t)*nptrs);
#endif
            if (NULL==indices) return indices;
        }
        /* Convert. */
        for (n=0UL; n<nptrs; n++) {
            if (NULL!=pointers[n])
                indices[n]=(pointers[n]-base)/size;
            else indices[n]=first+n;
            A(indices[n]<beyond);A(indices[n]>=first);
        }
    }
    return indices;
}
