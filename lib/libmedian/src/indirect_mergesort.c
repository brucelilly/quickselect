/*INDENT OFF*/

/* Description: C source code for indirect_mergesort */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    indirect_mergesort.c copyright 2017-2018 Bruce Lilly.   \ indirect_mergesort.c $
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
/* $Id: ~|^` @(#)   This is indirect_mergesort.c version 1.18 dated 2018-06-09T23:03:50Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "indirect_mergesort" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.indirect_mergesort.c */

/********************** Long description and rationale: ***********************
Sorting an array of items generally involves swapping of array elements.  If
elements are large compared to basic type sizes, swapping can become an
appreciable part of the total cost of sorting.  When sorting large elements, it
can be more efficient to set up an array of pointers to elements, indirectly
sorting by rearranging the pointers, which provide a means of accessing the
(unchanged) array elements in sorted order via the (rearranged) pointers.
Function indirect_mergesort sets up pointers, performs an indirect merge sort
rearranging the pointers, then rearranges array elements and cleans up.
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
#define ID_STRING_PREFIX "$Id: indirect_mergesort.c ~|^` @(#)"
#define SOURCE_MODULE "indirect_mergesort.c"
#define MODULE_VERSION "1.18"
#define MODULE_DATE "2018-06-09T23:03:50Z"
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
#include "quickselect_config.h" /* includes exchange.h for alignment_size */
#include "indirect.h"           /* pointer_mergesort set_array_pointers
                                   rearrange_array convert_pointers_to_indices
                                */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#else
# include <stdio.h>
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* */
#include <stddef.h>             /* size_t NULL */
#include <stdlib.h>             /* free realloc */
#include <string.h>             /* strrchr */

/* macros */
/* space-saving abbreviations */
#undef V
#define V void
#undef A
#define A(me) assert(me)

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

extern size_t quickselect_cache_size;

int indirect_mergesort(char *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *))
{
    int r=0;
    if ((NULL==base)
    && (0UL==size)
    && (NULL==compar)
    ) {
        r=errno=EINVAL;
    } else {
        if (1UL<nmemb) {
            size_t nptrs=nmemb+((nmemb+1UL)>>1); /* (1.5+)*N pointers */

            /* Determine cache size once on first call. */
            if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();

            /* allocate and initialize pointers */
            char **pointers=set_array_pointers(NULL,nptrs,base,size,0UL,nmemb);
            if (NULL!=pointers) {
                char **p; size_t *indices, n;
                if ((char)0 == indirect_initialized) initialize_indirect();
                /* mergesort using indirection; pointers moved, not data */
                pointer_mergesort(pointers,0UL,base,nmemb,nmemb,compar,
                    quickselect_cache_size,QUICKSELECT_INDIRECT);
                /* discard extra pointers */
                /* C11 defines aligned_alloc, but no aligned versions of
                   calloc or realloc; it is assumed(!) that realloc doesn't
                   decrease the alignment when decreasing the size of the
                   allocated block...
                */
#if REALLOC_DOES_NOT_DECREASE_ALIGNMENT
                p=realloc(pointers,nmemb*sizeof(char *));
                if (NULL!=p) pointers=p;
#endif /* REALLOC_DOES_NOT_DECREASE_ALIGNMENT */
                /* rearrangement of data elements is more efficiently performed
                   using indices to the data elements, rather than pointers
                */
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,(size_t *)pointers,0UL,nmemb);
                A(NULL!=indices);
                /* Rearrange the data elements according to sorted order using
                   the indices derived from the sorted pointers. No data element
                   is moved more than once.  However, there is poor locality of
                   access for random original data order.
                */
                n=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                    alignment_size(base,size));
                if (n>nmemb) r=errno;
                /* pointers (now indices) are no longer needed */
                free(pointers);
                if (n>nmemb) errno=r; /* in case free() changed errno */
            } else {
                r=errno;
            }
        }
    }
    return r;
}
