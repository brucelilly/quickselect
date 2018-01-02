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
* $Id: ~|^` @(#)    quickselect_src.h copyright 2017 Bruce Lilly.   \ quickselect_src.h $
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
/* $Id: ~|^` @(#)   This is quickselect_src.h version 1.14 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect_src.h */

/********************** Long description and rationale: ***********************
 An implementation of multiple quickselect similar to that described by Mahmoud
 & Lent in "Average-case analysis of multiple Quickselect: An algorithm for
 finding order statistics".  The array of items to be sorted by quickselect,
 the array element size, and the comparison function are declared as for qsort,
 with two additional parameters to specify an optional array of desired order
 statistic ranks.
 N.B. the values for the order-statistic array elements are zero-based ranks
 (i.e. 0UL for the minimum, N-1UL for the maximum of an array of N elements,
 (N-1UL)/2UL and N/2UL for the lower- and upper-medians).
 The implementation can also be used to implement qsort; a wrapper function
 (which could alternatively be implemented as a macro) is provided for that
 purpose.
 In order to be able to compute the rank of array elements for selection, the
 initial array base is maintained, and a pair of indices brackets the sub-array
 being processed.  The indices correspond to the ranks of elements.  A similar
 scheme is used by Musser's introsort, described in "Introspective Sorting and
 Selection Algorithms".  The same method is used for the array of desired order
 statistic ranks, similar to that described by Lent & Mahmoud.
 Pivot selection uses Rousseeuw & Basset's remedian, described in "The Remedian:
 A Robust Averaging Method for Large Data Sets", on a sample of a size which is
 a power of 3 taken from the array elements, with sample size approximately the
 square root of the array size as described in "Optimal Sampling Strategies for
 Quicksort" by McGeoch and Tygar, and in "Optimal Sampling Strategies in
 Quicksort and Quickselect" by Martinez and Roura.
 Elements sampled from the array for use by the pivot selection methods are
 chosen to provide good overall performance, avoiding choices that would fare
 poorly for commonly-seen input sequences (e.g. organ-pipe, already-sorted).
 Partitioning uses a method described by Bentley & McIlroy, with a modification
 to reduce disorder induced in arrays.
 Comparisons are made between array elements; there is no special-case code nor
 initialization macro.
 Partitioning can still result in an unfavorable split of the initial sub-array;
 when this happens, an emergency "break-glass" pivot selection function is used
 to ensure a better split, avoiding quadratic behavior even when handling a
 worst-case input array, such as can be generated by an adversary function
 (M. D. McIlroy "A Killer Adversary for Quicksort").
 Break-glass pivot selection uses median-of-medians using sets of 3 elements
 (ignoring leftovers) and using a call to quickselect to find the median of
 medians, rather than recursion.  Aside from those details, it is similar to the
 method described by Blum, Floyd, Pratt, Rivest, & Tarjan in "Time Bounds for
 Selection".
 Swapping of elements is handled by efficient inline swap functions.  No attempt
 is made to optimize for specific array element types, but swapping can be
 performed in units of basic language types.  The swap functions avoid useless
 work (e.g. when given two pointers to the same element).  No specialized macros
 are required.
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
#define ID_STRING_PREFIX "$Id: quickselect_src.h ~|^` @(#)"
#define SOURCE_MODULE "quickselect_src.h"
#define MODULE_VERSION "1.14"
#define MODULE_DATE "2017-12-22T04:14:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017"

/* Although the implementation is different, several concepts are adapted from:
   qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

/* local header files needed */
#include "quickselect_config.h"
#include "compare.h"            /* size_tcmp */
#include "exchange.h"           /* alignment_size blockmove reverse rotate
                                   swapn */
#include "indirect.h"           /* set_array_pointers rearrange_array */
#include "quickselect.h"        /* quickselect QSORT_FUNCTION_NAME */
/* if building a _s variant, declare dedicated_sort_s and quickselect_loop_s */
#if __STDC_WANT_LIB_EXT1__
/* dedicated_sort_s declaration */
QUICKSELECT_EXTERN
# include "dedicated_sort_s_decl.h"
;
/* quickselect_loop_s declaration */
QUICKSELECT_EXTERN
# include "quickselect_loop_s_decl.h"
;
#endif /* __STDC_WANT_LIB_EXT1__ */
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
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t (maybe RSIZE_MAX [N1570 K3.4]) */
# if __STDC_VERSION__ >= 201001L
    /* [N1570 6.10.8.1] (minimum value: y=0,mm=01) */
#  include <stdio.h>            /* (maybe errno_t rsize_t [N1570 K3.5]) */
# endif /* N1570 */
#endif /* C99 or later */
#include <stdlib.h>             /* free (maybe errno_t rsize_t
                                   constraint_handler_t [N1570 K3.6]) */

#if  __STDC_WANT_LIB_EXT1__
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

   Runtime-constraint violation detection is OK, but handling is poorly defined.
   It is unclear whether the handler is supposed to be global, or per-process,
   or per-thread.  The only interface is set_constraint_handler_s, which changes
   the handler; there is no way to determine what the handler is (e.g. in order
   to actually call it) without (at least potentially) changing it.  It may be
   possible to reset it (provided no other process or thread made an intervening
   change), but there is an inherent window of vulnerability.  No provision is
   made for mutexes or other synchronization mechanisms to avoid the
   vulnerability.
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
# if 0
static constraint_handler_t set_constraint_handler_s(
    constraint_handler_t handler)
{
    constraint_handler_t old_handler=the_handler;
    if (NULL==handler) the_handler=default_handler; else the_handler=handler;
    return old_handler;
}
# endif
#endif /* __STDC_WANT_LIB_EXT1__ */

/* This file also needs a declaration for quickselect_loop if using
   quickselect_loop_s
*/
#if __STDC_WANT_LIB_EXT1__
# undef QSORT_RETURN_TYPE
# define QSORT_RETURN_TYPE void
# undef QUICKSELECT_LOOP
# define QUICKSELECT_LOOP quickselect_loop
# undef COMPAR_DECL
# define COMPAR_DECL int(*compar)(const void *,const void *)
# undef NMEMB_SIZE_TYPE
# define NMEMB_SIZE_TYPE size_t
# undef QSORT_RETURN_TYPE
# define QSORT_RETURN_TYPE errno_t
# undef QUICKSELECT_LOOP
# define QUICKSELECT_LOOP quickselect_loop_s
# undef COMPAR_DECL
# define COMPAR_DECL int(*compar)(const void*,const void*,void*),void*context
# undef NMEMB_SIZE_TYPE
# define NMEMB_SIZE_TYPE rsize_t
#endif /* __STDC_WANT_LIB_EXT1__ */

/* clutter removal */
#undef PREFIX
#undef RETURN
#undef SUFFIX3
#undef TEST
#if __STDC_WANT_LIB_EXT1__
# define PREFIX A(0==ret);ret=
# define RETURN return ret
# define SUFFIX3 A(0==ret);return ret;
# define TEST A(0==ret)
#else
# define PREFIX /**/
# define RETURN return
# define SUFFIX3 /**/
# define TEST /**/
#endif /* __STDC_WANT_LIB_EXT1__ */

QSORT_RETURN_TYPE QUICKSELECT_FUNCTION_NAME(void *base, NMEMB_SIZE_TYPE nmemb,
    /*const*/ NMEMB_SIZE_TYPE size, COMPAR_DECL,
    size_t *pk, size_t nk, unsigned int options)
{
#if __STDC_WANT_LIB_EXT1__
    QSORT_RETURN_TYPE ret=0;
#endif /* __STDC_WANT_LIB_EXT1__ */
    size_t onk=nk; /* original nk value */
    size_t s=0UL;  /* rotation amount */
    size_t alignsize, salignsize, cutoff, scutoff, size_ratio, sratio, sz;
    void (*swapf)(char *, char *, size_t)=NULL;
    void (*sswapf)(char *, char *, size_t);
#if QUICKSELECT_LINEAR_STABLE
    char *conditions = NULL;
#endif /* QUICKSELECT_LINEAR_STABLE */
#if QUICKSELECT_INDIRECT + QUICKSELECT_LINEAR_STABLE
    char *element = NULL;
    size_t *indices = NULL;
#endif
#if QUICKSELECT_INDIRECT
    char **pointers = NULL;
    void (*pswapf)(char *, char *, size_t);
#endif /* QUICKSELECT_INDIRECT */

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar))) 
    || ((0UL!=nk) && (NULL==pk))
#if __STDC_WANT_LIB_EXT1__
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size) || (RSIZE_MAX<nk)
#endif /* __STDC_WANT_LIB_EXT1__ */
    || (0U!=(options&~(QUICKSELECT_USER_OPTIONS_MASK)))
    ) {
#if __STDC_WANT_LIB_EXT1__
        constraint_handler_t foo;
#if ! QUICKSELECT_PROVIDE_HANDLER /* use C11 poorly-defined handler */
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
#else /* semi-sane workaround */
        foo=get_constraint_handler_s();
#endif /* QUICKSELECT_PROVIDE_HANDLER */
        /* N1570 K3.1.3 may set errno */
        ret=
#endif /* __STDC_WANT_LIB_EXT1__ */
        errno=EINVAL;
#if __STDC_WANT_LIB_EXT1__
        /* Finally, now call what might (or might not) have been the handler at
           the time of the runtime-constraint violation.
        */
        foo("invalid parameters",NULL,ret);
        /* if the handler returns, return error code */
#endif /* __STDC_WANT_LIB_EXT1__ */
        return
#if __STDC_WANT_LIB_EXT1__
            errno=ret /* reset errno in case handler changed it */
#endif /* __STDC_WANT_LIB_EXT1__ */
        ;
    }

    if (2UL>nmemb) RETURN ; /* Return early if there's nothing to do. */

    /* Initialization of strings is performed here (rather than in
       quickselect_loop) so that quickselect_loop can be made inline.
    */
    if ((char)0==file_initialized) initialize_file(__FILE__);

    /* Simplify tests for selection vs. sorting by ensuring a NULL pointer when
       sorting. Ensure consistency between pk and nk. Ensure sorted pk array
       with no duplicated order statistics.
    */
    if (0UL==nk) pk=NULL;
    else if (NULL==pk) onk=nk=0UL;
    else if (1UL<nk) { /* binary search requires nondecreasing pk */
        size_t p, q;
        for (p=0UL,q=1UL; q<nk; p++,q++) /* verify (linear scan & compare) */
            if (pk[p]>pk[q]) break; /* not nondecreasing */

        /* Alignment, swapping function, cutoff for order statistics. */
        sz=sizeof(size_t);
        sswapf=swapn(salignsize=alignment_size((char *)pk,sz));
        sratio=sz/salignsize;
        scutoff=cutoff_value(sratio,QUICKSELECT_NETWORK_MASK);

        if (q<nk) { /* fix (sort) iff broken (not nondecreasing) */
            quickselect_loop((char *)pk,0UL,nk,sz,size_tcmp,NULL,0UL,0UL,sswapf,
                salignsize,sratio,scutoff,QUICKSELECT_NETWORK_MASK,NULL,NULL,
                NULL,NULL,NULL);
        }
        /* verify, fix uniqueness */
        for (p=0UL,q=1UL; q<=nk; q++) {
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    irotate((char *)pk,p,q,nk,sz,sswapf,salignsize,sratio);
                    s=q-p, nk-=s, q=p;
                }
            }
        }
    }

    /* base array alignment size and size_ratio */
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;

    /* Initialize the dedicated sorting cutoff based on array element size
       and alignment (swapping cost relative to comparisons).
    */
    cutoff=cutoff_value(size_ratio,options);

    /* swap function for direct sorting/selection */
    if (NULL==swapf) swapf=swapn(alignsize);

#if QUICKSELECT_INDIRECT > QUICKSELECT_NETWORK_MASK
# error "some reasonable heuristic for deciding when to sort indirectly is needed"
    /* (N.B. Unreliable) example: Direct or indirect sorting based on nmemb and
       size_ratio.
    */
    if ((1UL<size_ratio)&&(7UL<nmemb+size_ratio))
        options|=(QUICKSELECT_INDIRECT);
#endif /* QUICKSELECT_INDIRECT */

#if QUICKSELECT_INDIRECT
    /* Don't use indirection if size_ratio==1UL (it would be inefficient). */
    if (1UL==size_ratio) options&=~(QUICKSELECT_INDIRECT);
    /* If indirect sorting, alignment size and size_ratio are reset to
       appropriate values for pointers.
    */
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        pointers=set_array_pointers(pointers,nmemb,base,nmemb,size,0UL,nmemb);
        if (NULL==pointers) {
            free(pointers);
            pointers=NULL;
            options&=~(QUICKSELECT_INDIRECT);
        } else {
            if (size>=sizeof(char *))
                element=malloc(size);
            else
                element=malloc(sizeof(char *));
            if (NULL==element) {
                free(pointers);
                pointers=NULL;
                options&=~(QUICKSELECT_INDIRECT);
            } else {
                alignsize=alignment_size((char *)pointers,sizeof(char *));
                pswapf=swapn(alignsize);
                A(pointers[0]==base);
                cutoff=cutoff_value(1UL,options);
            }
        }
    }
#endif /* QUICKSELECT_INDIRECT */

#if QUICKSELECT_LINEAR_STABLE
    if (0U!=(options&(QUICKSELECT_STABLE))) {
        conditions=malloc(nmemb);
        if (NULL!=conditions) indices=malloc(sizeof(size_t)*nmemb);
        if ((NULL!=conditions)&&(NULL==element)) element=malloc(size);
    }
#endif /* QUICKSELECT_LINEAR_STABLE */

    /* For sorting (but not selection), use dedicated sort if nmemb<=cutoff. */
    if ((NULL==pk)&&(nmemb<=cutoff)) {
#if QUICKSELECT_INDIRECT
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            PREFIX DEDICATED_SORT((char *)pointers,0UL,nmemb,sizeof(char *),
                COMPAR_ARGS,pswapf,alignsize,size_ratio,options);
            if (NULL==indices) indices=(size_t *)pointers;
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            size_ratio=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                element);
            free(element); element=NULL;
            free(pointers);
            if ((void *)indices==(void *)pointers) indices=NULL;
            if (nmemb<(size_ratio>>1)) { /* indirection failed; use direct sort */
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                swapf=swapn(alignsize);
                PREFIX DEDICATED_SORT(base,0UL,nmemb,size,COMPAR_ARGS,swapf,
                    alignsize,size_ratio,options);
            }
        } else
#endif /* QUICKSELECT_INDIRECT */
            PREFIX DEDICATED_SORT(base,0UL,nmemb,size,COMPAR_ARGS,swapf,
                alignsize,size_ratio,options);
    } else {
        /* Special-case selection and sorting are handled in quickselect_loop. */
#if QUICKSELECT_INDIRECT
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
#if QUICKSELECT_LINEAR_STABLE
            PREFIX QUICKSELECT_LOOP((char *)pointers,0UL,nmemb,sizeof(char *),
                COMPAR_ARGS,pk,0UL,nk,pswapf,alignsize,size_ratio,cutoff,
                options,conditions,indices,element,NULL,NULL);
#else
            PREFIX QUICKSELECT_LOOP((char *)pointers,0UL,nmemb,sizeof(char *),
                COMPAR_ARGS,pk,0UL,nk,pswapf,alignsize,size_ratio,cutoff,
                options,NULL,NULL,NULL,NULL,NULL);
#endif /* QUICKSELECT_LINEAR_STABLE */
            if (NULL==indices) indices=(size_t *)pointers;
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            size_ratio=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                element);
            free(element); element=NULL;
            free(pointers);
            if ((void *)indices==(void *)pointers) indices=NULL;
            if (nmemb<(size_ratio>>1)) { /*indirection NG; direct sort/select*/
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                swapf=swapn(alignsize);
                cutoff=cutoff_value(size_ratio,options);
#if QUICKSELECT_LINEAR_STABLE
                PREFIX QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,pk,0UL,
                    nk,swapf,alignsize,size_ratio,cutoff,options,conditions,
                    indices,element,NULL,NULL);
#else
                PREFIX QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,pk,0UL,
                    nk,swapf,alignsize,size_ratio,cutoff,options,NULL,
                    NULL,NULL,NULL,NULL);
#endif /* QUICKSELECT_LINEAR_STABLE */
            }
        } else
#endif /* QUICKSELECT_INDIRECT */
#if QUICKSELECT_LINEAR_STABLE
            PREFIX QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,pk,0UL,nk,
                swapf,alignsize,size_ratio,cutoff,options,conditions,
                indices,element,NULL,NULL);
#else
            PREFIX QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,pk,0UL,nk,
                swapf,alignsize,size_ratio,cutoff,options,NULL,
                NULL,NULL,NULL,NULL);
#endif /* QUICKSELECT_LINEAR_STABLE */
    }

#if QUICKSELECT_LINEAR_STABLE
    if (0U!=(options&(QUICKSELECT_STABLE))) {
        if (NULL!=element) free(element);
        if (NULL!=conditions) free(conditions);
        if (NULL!=indices) free(indices);
    }
#endif /* QUICKSELECT_LINEAR_STABLE */

    /* Restore pk to full sorted (non-unique) order for caller convenience. */
    /* This may be expensive if the caller supplied a large number of duplicate
       order statistic ranks.  So be it.  Caller: if it hurts, don't do it.
    */
    if (0UL!=s) { /* there were duplicates */
        quickselect_loop((char *)pk,0UL,onk,sz,size_tcmp,NULL,0UL,0UL,sswapf,
            salignsize,sratio,scutoff,QUICKSELECT_NETWORK_MASK,NULL,NULL,NULL,
            NULL,NULL);
    }
    SUFFIX3
}
