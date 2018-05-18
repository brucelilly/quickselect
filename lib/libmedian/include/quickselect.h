/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining function quickselect
*/
#ifndef	QUICKSELECT_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    quickselect.h copyright 2016-2018 Bruce Lilly. \ quickselect.h $
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
/* $Id: ~|^` @(#)   This is quickselect.h version 1.24 dated 2018-05-07T05:27:30Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect.h */

/* version-controlled header file version information */
#define QUICKSELECT_H_VERSION "quickselect.h 1.24 2018-05-07T05:27:30Z"

#include <stddef.h>             /* size_t (maybe rsize_t [N1570 K3.3]) */
/* preliminary support for 9899:201x draft N1570 */
#if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 201001L )
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
# include <errno.h>             /* (maybe errno_t [N1570 K3.2]) */
# include <stdint.h>            /* (maybe RSIZE_MAX [N1570 K3.4]) */
# include <stdio.h>             /* (maybe errno_t rsize_t [N1570 K3.5]) */
# include <stdlib.h>            /* (maybe errno_t rsize_t constraint_handler_t
                                   [N1570 K3.6]) */
#endif /* N1570 */

/* separated from __STDC_VERSION__ test to permit testing in pre-C11 context */
#if __STDC_WANT_LIB_EXT1__
  /* C11 RSIZE_MAX, errno_t, rsize_t, constraint_handler_t are presumed to be
     all or none
  */
# ifndef RSIZE_MAX /* provide definitions if lacking in compilation environment */
    typedef int errno_t;
    typedef size_t rsize_t;
    typedef void(*constraint_handler_t)(const char * restrict msg,
        void * restrict ptr, errno_t error);
#  if defined(__STDC__) && ( __STDC__ == 1 ) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )
#   include <stdint.h>           /* (header not standardized pre-C99) SIZE_MAX */
#  endif /* C99 or later */
#  ifndef SIZE_MAX /* not standardized pre-C99 */
#  include <limits.h>           /* ULONG_MAX */
#   define SIZE_MAX ULONG_MAX
#  endif /* SIZE_MAX */
#  define RSIZE_MAX ((SIZE_MAX)>>1)
#  define QUICKSELECT_PROVIDE_HANDLER 1 /* provide static implementation */
#  define QUICKSELECT_HANDLER_PROVIDED 0
# endif /* RSIZE_MAX */
#endif /* __STDC_WANT_LIB_EXT1__ */

/* qsort wrapper function name */
#ifndef QSORT_FUNCTION_NAME
# define QSORT_FUNCTION_NAME qsort_wrapper
#endif

/* quickselect option and processing flag bitmap values */

  /* User-accessible (if supported at compile-time) options: */
  /* non-stable vs. stable partial ordering */
    /* faster (but not stable) if bit is not set */
  /* Quickselect can sort or select preserving partial order stability. However,
     code size approximately doubles, and there is a significant run-time cost.
     Partial order stability is a non-issue for scalar data, and multivariate
     data is most efficiently sorted using multivariate comparisons.  It is
     therefore recommended to define QUICKSELECT_STABLE as 0 (zero) for general-
     purpose use.
  */
  /* if QUICKSELECT_STABLE is pre-defined as zero, the option will be
     unavailable and object code size will be reduced
  */
#ifndef QUICKSELECT_STABLE
# define QUICKSELECT_STABLE              0x01U
#endif
/* sanity check for override */
#if ((QUICKSELECT_STABLE) != 0) && ((QUICKSELECT_STABLE) != 0x01U)
# error "unreasonable definition for QUICKSELECT_STABLE"
#endif

  /* minimize comparisons vs. minimize run-time for simple comparisons */
    /* optimize for speed with simple comparisons if bit is not set */
    /* in-place mergesort for small sub-arrays uses fewer comparisons on
       average than insertion sort (even with binary search), but is recursive
       and moves more data on average
    */
#define QUICKSELECT_OPTIMIZE_COMPARISONS 0x02U

  /* Indirect sorting via an array of pointers to elements. */
  /* Indirection (i.e. initially moving pointers to the data instead of the data
     elements, followed by a rearrangement of the data elements after their
     final positions have been determined) is used internally for modest-sizes
     arrays of large elements (when moving pointers is less costly than moving
     data elements) and is available for all arrays as a user-visible option in
     quickselect.  There is no guarantee of partial order stability for qsort.
  */
#define QUICKSELECT_INDIRECT             0x04U /* visible, used internally */

  /* Internal use flags (possibly subject to compile-time definition): */

  /* restrict pivot rank by using all array elements for pivot selection */
    /* use only a sample of array elements for pivot selection if not set */
    /* This flag is used by the ``break-glass'' mechanism to avoid performance
       degradation from linearithmic to polynomial with adverse inputs.  The
       ``break-glass'' mechanism uses little additional code and is critial for
       preventing poor performance; therefore it is not subject to being
       disabled at compile-time.
    */
#define QUICKSELECT_RESTRICT_RANK        0x08U /* internal */

/* mask for user-accessible option bits (don't allow caller-supplied options
   to interfere with internal flags); internal flags are high order bits
*/
#define QUICKSELECT_INTERNAL_FLAGS 0xFFFFFFF8U
#define QUICKSELECT_USER_OPTIONS_MASK (( QUICKSELECT_STABLE \
        | QUICKSELECT_OPTIMIZE_COMPARISONS | QUICKSELECT_INDIRECT ) \
        & ~( QUICKSELECT_INTERNAL_FLAGS ))
/* end of option bits */

/* function declarations */
#if defined(__cplusplus)
# define QUICKSELECT_EXTERN extern "C"
#else
# define QUICKSELECT_EXTERN extern
#endif

/* options.c */
QUICKSELECT_EXTERN unsigned int quickselect_options(void);

/* quickselect.c */
QUICKSELECT_EXTERN int quickselect(/*const*/ void *, size_t, /*const*/ size_t,
    int (*)(const void *, const void *),
    /*const*/ size_t /*const*/ *, /*const*/ size_t, unsigned int);
QUICKSELECT_EXTERN void QSORT_FUNCTION_NAME(/*const*/ void *, size_t,
    /*const*/ size_t, int (*)(const void *, const void *));
#if __STDC_WANT_LIB_EXT1__
# ifndef QSORT_S_FUNCTION_NAME
#  define QSORT_S_FUNCTION_NAME qsort_s_wrapper
# endif
QUICKSELECT_EXTERN errno_t quickselect_s(/*const*/ void *, rsize_t,
    /*const*/ rsize_t, int (*)(const void *, const void *, void *),
    /*const*/ void *, /*const*/ size_t /*const*/ *, /*const*/ size_t,
    unsigned int);
QUICKSELECT_EXTERN errno_t QSORT_S_FUNCTION_NAME(/*const*/ void *, rsize_t,
    /*const*/ rsize_t, int (*)(const void *, const void *, void *),
    /*const*/ void *);
/* QSORT_S_FUNCTION_NAME remains defined for use in quickselect.c */
#endif

/* QSORT_FUNCTION_NAME and QSORT_S_FUNCTION_NAME remain defined for use in
   quickselect.c
*/

#define	QUICKSELECT_H_INCLUDED
#endif
