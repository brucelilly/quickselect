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
* $Id: ~|^` @(#)    quickselect.h copyright 2016-2017 Bruce Lilly. \ quickselect.h $
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
/* $Id: ~|^` @(#)   This is quickselect.h version 1.20 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect.h */

/* version-controlled header file version information */
#define QUICKSELECT_H_VERSION "quickselect.h 1.20 2017-12-22T04:14:04Z"

#include <stddef.h>             /* size_t (maybe rsize_t [N1570 K3.3]) */
/* preliminary support for 9899:201x draft N1570 */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 201001L )
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
#  include <limits.h>           /* SIZE_MAX */
#  ifndef SIZE_MAX /* not standardized pre-C99 */
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

  /* bits 0x01FF8U are reserved for network sort size mask */
  /* 0x01FF8U supports network sorting for sizes 2 (unconditional) and 3-12 via
     bits 0x01U<<N for size N.  The range of supported sizes may be reduced at
     compile-time to reduce code size.
  */
#ifndef QUICKSELECT_NETWORK_MASK
# define QUICKSELECT_NETWORK_MASK        0x01FF8U /* 2-12 */
#endif
/* sanity check for override */
#if ((QUICKSELECT_NETWORK_MASK) & ~(0x01FF8U) != 0)
# error "unreasonable definition for QUICKSELECT_NETWORK_MASK"
#endif

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

  /* Internal use flags (possibly subject to compile-time definition): */

  /* restrict pivot rank by using all array elements for pivot selection */
    /* use only a sample of array elements for pivot selection if not set */
    /* This flag is used by the ``break-glass'' mechanism to avoid performance
       degradation from linearithmic to polynomial with adverse inputs.  The
       ``break-glass'' mechanism uses little additional code and is critial for
       preventing poor performance; therefore it is not subject to being
       disabled at compile-time.
    */
#define QUICKSELECT_RESTRICT_RANK        0x02000U
/* sanity checks */
#if QUICKSELECT_NETWORK_MASK & ( QUICKSELECT_STABLE \
| QUICKSELECT_OPTIMIZE_COMPARISONS | QUICKSELECT_RESTRICT_RANK \
| QUICKSELECT_INDIRECT )
# error "options bitmap conflict"
#endif

  /* Possibly user-visible (0x04U), possibly internal (0x04000U) */
  /* Making this an internal flag implies some reasonable heuristic to
     determine when to use indirection (see quickselect.c and qsort.c).
  */
  /* Indirect sorting via an array of pointers to elements. */
  /* If QUICKSELECT_INDIRECT is pre-defined as zero, sorting will be direct
     unless arranged externally by the caller and object code size will be
     reduced.  Internal indirection uses O(N) + O(1) additional space for
     an array of pointers and a memory block to hold a temporary base array
     element.  For non-trivial base array element size, it reduces data
     movement cost by rearranging (permuting) base array elements after
     indirectly sorting by low-cost pointer movement.  However, rearranging
     base array elements has poor locality of access, and therfore suffers
     from machine-dependent (cache-related) performance issues when the
     base array (product of element size and number of elements) becomes
     large relative to memory cache size.  The caller can of course arrange
     for explicit indirect sorting by allocating and initializing an array of
     pointers, providing a comparison function which indirectly accesses the
     base elements for comparisons, and optionally rearranging base data
     elements after indirect sorting of pointers (or accessing base elements
     indirectly via the sorted pointers) [see header file indirect.h].  Such a
     method is fully compatible with the internal indirect sorting (the internal
     method never uses additional indirection when sorting trivial types such as
     pointers), but the internal method can provide some performance tweaks by
     caching dereferenced pointers e.g. for pivot comparisons during
     partitioning.
  */
#ifndef QUICKSELECT_INDIRECT
# define QUICKSELECT_INDIRECT            0x04U /* default user-visible */
#endif
/* sanity check for override */
#if ((QUICKSELECT_INDIRECT) != 0) && ((QUICKSELECT_INDIRECT) != 0x04000U) \
&& ((QUICKSELECT_INDIRECT) != 0x04U)
# error "unreasonable definition for QUICKSELECT_INDIRECT"
#endif

/* mask for user-accessible option bits (don't allow caller-supplied options
   to interfere with internal flags); internal flags are more significant bits
   than network mask reserved bits
*/
#define QUICKSELECT_INTERNAL_FLAGS 0x0000E000U
#define QUICKSELECT_USER_OPTIONS_MASK (( QUICKSELECT_NETWORK_MASK \
        | QUICKSELECT_STABLE | QUICKSELECT_OPTIMIZE_COMPARISONS \
        | QUICKSELECT_INDIRECT ) & ~( QUICKSELECT_INTERNAL_FLAGS ))
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
QUICKSELECT_EXTERN void quickselect(/*const*/ void *, size_t, /*const*/ size_t,
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
