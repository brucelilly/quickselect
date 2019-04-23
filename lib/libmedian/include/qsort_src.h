/*INDENT OFF*/

/* Description: common C source code for qsort and qsort_s */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    qsort_src.h copyright 2017-2019 Bruce Lilly.   \ qsort_src.h $
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
/* $Id: ~|^` @(#)   This is qsort_src.h version 1.19 dated 2019-04-17T23:46:05Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.qsort_src.h */

/********************** Long description and rationale: ***********************
 This file contains source to implement the public functions qsort and qsort_s.
 It is based on an implementation of multiple quickselect similar to that
 described by Mahmoud & Lent in "Average-case analysis of multiple Quickselect:
 An algorithm for finding order statistics".  The array of items to be sorted by
 quickselect, the array element size, and the comparison function are declared
 as for qsort, with two additional parameters to specify an optional array of
 desired order statistic ranks.
 N.B. the values for the order-statistic array elements are zero-based ranks
 (i.e. 0UL for the minimum, N-1UL for the maximum of an array of N elements,
 (N-1UL)/2UL and N/2UL for the lower- and upper-medians).
 The implementation can also be used to implement qsort; a wrapper function
 (which could alternatively be implemented as a macro) is provided for that
 purpose (this file).
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
#define ID_STRING_PREFIX "$Id: qsort_src.h ~|^` @(#)"
#define SOURCE_MODULE "qsort_src.h"
#define MODULE_VERSION "1.19"
#define MODULE_DATE "2019-04-17T23:46:05Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2019"

/* local header files needed */
#include "quickselect_config.h"
#include "exchange.h"           /* alignment_size swapn */
#if 0
#include "indirect.h"           /* */
#endif
#include "quickselect.h"        /* quickselect QSORT_FUNCTION_NAME */
#include "initialize_src.h"     /* last local header file */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* (maybe errno_t [N1570 K3.2]) */
#include <limits.h>             /* *_MAX */
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t (maybe RSIZE_MAX [N1570 K3.4]) */
# if __STDC_VERSION__ >= 201001L
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
#  include <stdio.h>            /* (maybe errno_t rsize_t [N1570 K3.5]) */
# endif /* N1570 */
#endif /* C99 or later */
#  include <stdlib.h>           /* malloc free (maybe errno_t rsize_t
                                   constraint_handler_t [N1570 K3.6]) */

/* macros */
/* stringification (don't change!) */
#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

#if __STDC_WANT_LIB_EXT1__
/* Preliminary support for 9899:201x draft N1570 qsort_s w/ "context".
   "context" is similar to glibc qsort_r; N1570 also has provision for
   runtime-constraint violation detection and handling. "Preliminary"
   because of the apparent lack of a convincing use-case for "context" and
   due to the poorly defined semantics and interface for constraint
   violation handling.

   Consequences of "context" are wide-ranging: any operation that requires
   comparisons (pivot selection, partitioning, dedicated sorting and/or
   selection, and support functions for any of those) has to be rewritten.
   The only operations unaffected are sampling, basic swaps, blockmoves using
   swaps, rank comparisons, and partition size evaluation; sampling and
   repivoting tables may also be reused without change.
   Executable code size is roughly doubled, not including external
   constraint-handler function code.
   Use case of "context" is questionable; one could simply use an appropriate
   comparison function.  Published supposed examples (e.g.
   https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/qsort-s
   tend to be rather silly (in the specific example above:
      1. functions are defined for specific locales, but not specifically
         implementing locale-specific collation -- one could at least as easily
         define locale-specific string comparison functions using the standard
         strcoll [+] function and use these with qsort to perform the
         locale-specific sorts with no need for "context", and avoiding the
         other implementation issues detailed below.
      2. Aside from inconsistent "context" argument order...
      3. ... the implementation comparison function copies (!) (leading portions
         of) supplied strings to fixed size (!!)[*] local buffers, then makes a
         second set of passes (!!!) over the copies to perform a conversion
         (which is discarded (!!!!) when the comparison function returns), which
         is then subjected to a third (!!!!!) pass to perform the comparison.
         Note that qsort implementations typically call the comparison function
         N log N times for an array of N elements; the above implementation
         makes 2 N log N copies and 6 N log N passes over data representing
         the strings to be sorted.  In particular, at each stage of partitioning
         qsort implementations compare a pivot element to many other elements;
         in the above implementation the string corresponding to that pivot
         element is copied, the copy modified and examined, only to be discarded
         before repeating those steps anew with the same pivot element's string
         for the next comparison.
      4. Although the copies (see #3 above) ignore the actual length of the
         original strings, strlen is subsequently called for each string (copy),
         resulting in a fourth (!!!!!!) pass over the data. Had strlen been
         called for the initial strings, the copies could have been made
         appropriate sizes (assuming that one still naively wished to make, then
         discard such copies).
      5. "context" is declared as void *, not const void *, which implies that
         either the qsort_s implementation or the called comparison function
         may modify the object pointed to by "context", which invites
         non-deterministic, non-repeatable, (i.e. unspecified and undefined)
         behavior.
      * a possibly-valid use of "context" *might* be to provide the maximum
        string length, avoiding magic numbers and failure to fully compare
        strings which have long identical leading substrings... *if* one
        insisted on the naive implementation involving copies (which are
        subsequently discarded) and multiple passes over the data.  It would
        be much more efficient to make one-time initial transformations (e.g.
        w/ strxfrm [+]) of the original string data supplemented by an index
        indicating the original order (or a pointer to the original string
        data), sorting the transformed and supplemented data using standard
        strcmp on the transformed strings.  That would involve N combined
        transformations and copies (instead of N log N copies and N log N
        transformations) plus N log N standard strcmp string comparisons (rather
        than 6 N log N passes over data), followed by access of the orginal
        string data via the indices or pointers for output.
      + Note that strcoll and strxfrm have been standardized since C89 (a.k.a.
        C90).
   ). In summary, there is no convincing use-case for "context" to justify
   doubling the size of library code.

   Runtime-constraint violation detection is OK, but error handling is poorly
   defined.  It is unclear whether the handler is supposed to be global, or
   per-process, or per-thread.  The only interface is set_constraint_handler_s,
   which changes the handler; there is no way to determine what the handler is
   (e.g. in order to actually call it) without (at least potentially) changing
   it.  It may be possible to reset it (provided no other process or thread made
   an intervening change), but there is an inherent window of vulnerability.
   No provision is made for mutexes or other synchronization mechanisms to avoid
   the vulnerability.
*/

# if QUICKSELECT_PROVIDE_HANDLER && ! QUICKSELECT_HANDLER_PROVIDED
#  include <stdio.h>            /* fprintf stderr */
static void default_handler(const char * restrict msg, void * restrict ptr,
    errno_t error)
{
    (V)fprintf(stderr, "%s\n",msg);
    errno=error;
}
static constraint_handler_t the_handler = default_handler;
/* Partial workaround for the poorly-defined constraint handler issue... */
static constraint_handler_t get_constraint_handler_s(void)
{
    return the_handler;
}
# undef QUICKSELECT_HANDLER_PROVIDED
# define QUICKSELECT_HANDLER_PROVIDED 1
# endif /* QUICKSELECT_PROVIDE_HANDLER */
#endif /* __STDC_WANT_LIB_EXT1__ */

/* clutter removal */
#undef PREFIX
#undef SUFFIX3
#if __STDC_WANT_LIB_EXT1__
# define PREFIX A(0==ret);ret=
# define SUFFIX3 A(0==ret);return ret;
#else
# define PREFIX /**/
# define SUFFIX3 /**/
#endif /* __STDC_WANT_LIB_EXT1__ */

/* public interface */
/* calls: alignment_size, swapn, quickselect_loop */
#if __STDC_WANT_LIB_EXT1__
errno_t QSORT_S_FUNCTION_NAME
#else
void
# if LIBMEDIAN_TEST_CODE
    d_qsort
# else
    QSORT_FUNCTION_NAME
# endif
#endif
        (void *base, NMEMB_SIZE_TYPE nmemb,
        /*const*/ NMEMB_SIZE_TYPE size,
        COMPAR_DECL)
{
    size_t alignsize;
    void (*swapf)(char *, char *, size_t);
#if __STDC_WANT_LIB_EXT1__
    errno_t ret=0;
#endif /* __STDC_WANT_LIB_EXT1__ */

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar))) 
#if __STDC_WANT_LIB_EXT1__
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size)
#endif /* __STDC_WANT_LIB_EXT1__ */
    ) {
#if __STDC_WANT_LIB_EXT1__
        constraint_handler_t foo;
# if ! QUICKSELECT_PROVIDE_HANDLER /* use C11 poorly-defined handler */
        /* Runtime-constraint violation handler to be called, but what is it?
           Only interface is set_constraint_handler_s, which CHANGES the
           handler.  N.B. determining the handler either before or after the
           violation might not represent the handler in effect AT THE TIME OF
           the violation, if the handler is not per-thread and/or in the absence
           of cooperating thread-based mutexes to constrain changes to the
           handler (or if even one thread fails to cooperate by failing to use a
           common mutex).
        */
        foo=set_constraint_handler_s(NULL); /* returns pointer to handler
                                               function, sets default handler
                                               (which might not be the current
                                               handler) */
        /* Quickly (N.B. there is a window of vulnerability here during which
           the handler has been changed.  It is undefined whether the handler is
           global, or per-process, or per-thread. Other threads or processes
           might therefore be affected.  Also, another process or thread might
           change the handler during this window; resetting the handler (which
           has been changed by the above call to set_contraint_handler_s) might
           undo any such change.  In combination, two (or more!) threads or
           processes calling set_constraint_handler_s to determine what the
           handler is might interfere with the other thread(s) or process(es),
           possibly resulting in incorrect inferences about the handler and/or
           unintended changes to the handler.) reset the handler to foo.
        */
        (void)set_constraint_handler_s(foo); /* reset to (presumed) previous
                                                handler */
# else /* semi-sane workaround */
        foo=get_constraint_handler_s();
# endif /* QUICKSELECT_PROVIDE_HANDLER */
        ret=EINVAL;
        /* N1570 K3.1.3 may set errno */
#endif /* __STDC_WANT_LIB_EXT1__ */
        errno=EINVAL;
#if __STDC_WANT_LIB_EXT1__
        /* Finally, now call what might (or might not) have been the handler at
           the time of the runtime-constraint violation.
        */
        foo("invalid parameters",NULL,ret);
        /* if the handler returns, return error code */
        return errno=ret; /* reset errno in case handler changed it */
#else
        return;
#endif /* __STDC_WANT_LIB_EXT1__ */
    }

#if LIBMEDIAN_TEST_CODE
# if defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V) fprintf(stderr,
            "/* %s: %s line %d: nmemb=%lu, size=%lu, compar=%s */\n",
            __func__,"qsort_src.h",__LINE__,nmemb,size,comparator_name(compar));
    }
# endif
#endif

    if (1UL<nmemb) { /* nothing to do for fewer than 2 elements */
        PREFIX 
# if LIBMEDIAN_TEST_CODE
#  if __STDC_WANT_LIB_EXT1__
            d_quickselect_s
#  else
            d_quickselect
#  endif
# else
#  if __STDC_WANT_LIB_EXT1__
            quickselect_s
#  else
            quickselect
#  endif
# endif
	        (base,nmemb,size,COMPAR_ARGS,NULL,0UL,0U);
    }
    SUFFIX3
}
