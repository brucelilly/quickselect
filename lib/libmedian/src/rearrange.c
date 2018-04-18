/*INDENT OFF*/
/* XXX obsolete file; see quickselect_config.h */

/* Description: C source code for rearrange_array */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    rearrange.c copyright 2017-2018 Bruce Lilly.   \ rearrange.c $
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
/* $Id: ~|^` @(#)   This is rearrange.c version 1.9 dated 2018-02-14T09:51:59Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "rearrange" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.rearrange.c */

/********************** Long description and rationale: ***********************
Sorting an array of items generally involves swapping of array elements.  If
elements are large compared to basic type sizes, swapping can become an
appreciable part of the total cost of sorting.  When sorting large elements, it
can be more efficient to set up an array of pointers to elements, indirectly
sorting by rearranging the pointers, which provide a means of accessing the
(unchanged) array elements in sorted order via the (rearranged) pointers.
In some cases, it may be necessary to rearrange the array in sorted order rather
than indirectly via the rearranged pointers.  Function rearrange_array performs
that rearrangement using indices rather than pointers, and updates the indices
to point to the rearranged elements.
******************************************************************************/

/* Nothing to configure below this line. */

/* Minimum _XOPEN_SOURCE version for C99 (else illumos compilation fails) */
#undef MAX_XOPEN_SOURCE_VERSION
#undef MIN_XOPEN_SOURCE_VERSION
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
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
#define ID_STRING_PREFIX "$Id: rearrange.c ~|^` @(#)"
#define SOURCE_MODULE "rearrange.c"
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2018-02-14T09:51:59Z"
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
#include "indirect.h"           /* rearrange_array */
#include "zz_build_str.h"       /* build_id build_strings_registered
                                   copyright_id register_build_strings */
/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* */
#include <limits.h>             /* *_MAX */
#ifndef SIZE_MAX /* not standardized pre-C99 */
# define SIZE_MAX ULONG_MAX
#endif
#include <stddef.h>             /* size_t NULL */
#if ASSERT_CODE
# include <stdio.h>
#endif
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t */
#else
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 */
#include <stdlib.h>             /* free malloc */
#include <string.h>             /* strrchr */

/* macros */
/* space-saving abbreviations */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* XXX obsolete file; see quickselect_config.h */
#if 0
/* type finagle */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define TYPE8 uint_least64_t
# define TYPE4 uint_least32_t
# define TYPE2 uint_least16_t
#else
# if ( LONG_MAX > 2147483647 ) && ( LONG_MAX == 9223372036854775807 )
#  define TYPE8 long
# elif DBL_MAX_10_EXP == 308
#  define TYPE8 double
# else
#  define TYPE8 char *
# endif /* 8 byte types */
# if INT_MAX == 2147483647
#  define TYPE4 int
# elif LONG_MAX == 2147483647
#  define TYPE4 long
# endif /* 4 byte types */
# define TYPE2 short
#endif /* C99 */

QUICKSELECT_INLINE
size_t rearrange_array(char *base, size_t nmemb, size_t size, size_t *indices,
    size_t nindices, size_t first, size_t beyond, size_t alignsize,
    size_t size_ratio)
{
    size_t ndata_moves=0UL;

    if ((nindices < nmemb)
    || (NULL==base)
    || (NULL==indices)
    || (0UL==size)
    || (first>beyond)
    || (first+nmemb<beyond)
    ) {
        errno=EINVAL;
        return SIZE_MAX;
    }
    if (0UL<nmemb) {
        /* Step through the indices: at the first index which points to an
           array element whose position cpos=((address-base)/size) differs from
           the index position fpos=((p-indices)/sizeof(char *)), find the cycle
           in the array which needs to be rotated to permute the array into the
           desired order.  Folow that cycle, rotating the data by alignsize
           slices until the elements have been rearranged, then update the
           indices.  Continue, looking for the next cycle until all cycles have
           been processed. Indices are unconditionally updated by the algorithm.
           The return value is the number of data moves.
           indices[0] corresponds to the data element at base+first*size
        */
        register size_t cpos, hpos, ppos, n, o, slice;
        register char *cp, *p, *hp;
#if ASSERT_CODE
        cp=base+first*size, hp=base+beyond*size;
        /* index sanity check; n counts errors */
        for (n=0UL,hpos=0UL; hpos<nmemb; hpos++) {
            cpos=indices[hpos];
            if ((first<=cpos)&&(cpos<beyond)) continue;
            (V)fprintf(stderr,
                "%s: indices[%lu]=%lu is out of bounds: first=%lu, "
                "beyond=%lu: %ld\n",
                __func__,hpos,cpos,first,beyond);
            n++;
        }
        A(0UL==n);
#endif
        /* Loop through indices [0,nmemb) and array pointers; pointer
           updated by inexpensive increment. Skip over in-place elements
           (final index matches position).  Follow a cycle starting at
           the next out-of-place element found, rearranging elements in
           the cycle and updating their indices to indicate that they
           have been placed in position. In-place detection is intended
           to be fast.
        */
        /* Four variants, depending on alignsize; identical other than type. */
#define REARRANGE_ARRAY(mtype)                                      \
for (n=0UL,o=first,p=base+o*size; n<nmemb; n++,o++,p+=size) {       \
    mtype t;                                                        \
    cpos=indices[n];/*current index element to be at first+n*/      \
    if (cpos==o) continue; /* already in final position */          \
    /* data elements' slice loop */                                 \
    for (ppos=cpos,slice=0UL; slice<size_ratio;                     \
    p+=alignsize,slice++)                                           \
    {   /* follow cycle */                                          \
        for (t=*((mtype *)p),cpos=ppos,hpos=o,hp=p; cpos!=o; ) {    \
            cp=base+cpos*size+slice*alignsize;                      \
            *((mtype *)hp) = *((mtype *)cp);                        \
            hpos=cpos, hp=cp; cpos=indices[hpos-first];             \
        } /* data movement cycle loop */                            \
        *((mtype *)cp) = t;                                         \
    } /* elements' slice loop */                                    \
    /* 2nd loop over cycle: update indices, count moves */          \
    for (p-=size,cpos=indices[n],hpos=o; cpos!=o; hpos=cpos) {      \
        cpos=indices[hpos-first]; indices[hpos-first]=hpos;         \
    } /* index update cycle loop */                                 \
    ndata_moves++;                                                  \
} /* array index loop */
        switch (alignsize) {
            case 8UL : REARRANGE_ARRAY(TYPE8)
            break;
            case 4UL : REARRANGE_ARRAY(TYPE4)
            break;
            case 2UL : REARRANGE_ARRAY(TYPE2)
            break;
            default : REARRANGE_ARRAY(char)
            break;
        }
    }
    return ndata_moves;
}
#endif
