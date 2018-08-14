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
* $Id: ~|^` @(#)    quickselect_s.c copyright 2016-2018 Bruce Lilly.   \ quickselect_s.c $
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
/* $Id: ~|^` @(#)   This is quickselect_s.c version 1.15 dated 2018-04-16T05:48:23Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.quickselect_s.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* Nothing to configure below this line. */

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: quickselect_s.c ~|^` @(#)"
#define SOURCE_MODULE "quickselect_s.c"
#define MODULE_VERSION "1.15"
#define MODULE_DATE "2018-04-16T05:48:23Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* configuration for C11 _s variant w/ debugging */
# define QUICKSELECT_BUILD_FOR_SPEED 1 /* make d_dedicated_sort_s() static */
#define __STDC_WANT_LIB_EXT1__ 1

#define COMPAR_DECL int(*compar)(const void*,const void*,void*),void *context
#define COMPAR_ARGS compar,context
#define DEDICATED_SORT d_dedicated_sort_s
#define FMED3_FUNCTION_NAME fmed3_s
#define REMEDIAN_FUNCTION_NAME remedian_s
#define SELECT_PIVOT_FUNCTION_NAME select_pivot_s
#define QUICKSELECT_LOOP d_quickselect_loop_s

#define FIND_MINMAX_FUNCTION_NAME d_find_minmax
#define KLIMITS_FUNCTION_NAME d_klimits
#define PARTITION_FUNCTION_NAME partition_s
#define SAMPLING_TABLE_FUNCTION_NAME d_sampling_table
#define SELECT_MIN_FUNCTION_NAME d_select_min
#define SELECT_MAX_FUNCTION_NAME d_select_max
#define SELECT_MINMAX_FUNCTION_NAME d_select_minmax
#define SHOULD_REPIVOT_FUNCTION_NAME d_should_repivot
#define QUICKSELECT_VISIBILITY extern


/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes most other local and system header files required */
#include "indirect.h"           /* rearrange_array convert_pointers_to_indices */

#include "initialize_src.h"

#if DEBUG_CODE
# include <stdio.h>
#endif

/* Data cache size (bytes), initialized on first run */
extern size_t quickselect_cache_size;

#ifdef RSIZE_MAX
/* Preliminary support for 9899:201x draft N1570 qsort_s w/ "context".
   "context" is similar to glibc qsort_r; N1570 also has provision for
   runtime-constraint violation detection and handling.
   Consequences of "context" are wide-ranging: any operation that requires
   comparisons (pivot selection, partitioning, dedicated sorting and/or
   selection, and support functions for any of those) has to be rewritten.
   The only operations unaffected are sampling, basic swaps, blockmoves using
   swaps, rank comparisons, and partition size evaluation; sampling and
   repivoting tables may also be reused without change.
   Executable code size is roughly doubled, not including external
   constraint-handler function code.
   "context" is declared as void *, not const void *, which implies that
   either the qsort_s implementation or the called comparison function
   may modify the object pointed to by "context", which invites
   non-deterministic, non-repeatable, (i.e. unspecified and undefined)
   behavior.
*/
# if QUICKSELECT_PROVIDE_HANDLER
#  include <stdio.h>            /* fprintf stderr */
static void default_handler(const char * restrict msg, void * restrict ptr,
    errno_t error)
{
    (V)fprintf(stderr, "%s\n",msg);
    errno=error;
}
static constraint_handler_t the_handler = default_handler;
static constraint_handler_t set_constraint_handler_s(constraint_handler_t handler)
{
    constraint_handler_t old_handler=the_handler;
    if (NULL==handler) the_handler=default_handler; else the_handler=handler;
    return old_handler;
}
# endif /* PROVIDE_HANDLER */
/* static functions supporting comparison "context" and runtime-constraint
   violation detection/handling
*/

/* One source for dedicated_sort and partition variants. */
#include "partition_src.h"

#undef QUICKSELECT_BUILD_FOR_SPEED
#define QUICKSELECT_BUILD_FOR_SPEED 1

#ifndef SAMPLING_TABLE_SIZE
# error "SAMPLING_TABLE_SIZE is not defined"
#elsif ! SAMPLING_TABLE_SIZE
# error "SAMPLING_TABLE_SIZE is 0"
#endif

# undef QUICKSELECT_BUILD_FOR_SPEED
# define QUICKSELECT_BUILD_FOR_SPEED 0
# include "quickselect_loop_src.h"

/* public interfaces for N1570 draft extensions */
QUICKSELECT_RETURN_TYPE quickselect_s_internal(void *base, rsize_t nmemb, /*const*/ rsize_t size,
    COMPAR_DECL, size_t *pk, size_t nk, unsigned int options)
{
    errno_t ret=0;
    size_t onk=nk; /* original nk value */
    size_t s=0UL;  /* rotation amount */
    size_t alignsize, size_ratio;
    void (*swapf)(char *, char *, size_t)=NULL;
    void (*sswapf)(char *, char *, size_t);
    size_t *indices = NULL;
    char **pointers = NULL;
    void (*pswapf)(char *, char *, size_t)
#if SILENCE_WHINEY_COMPILERS
        =NULL
#endif
        ;

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || ((0UL!=nk) && (NULL==pk))
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size) || (RSIZE_MAX<nk)
    || (0U!=(options&~(QUICKSELECT_USER_OPTIONS_MASK)))
    ) {
        constraint_handler_t foo;
        /* N1570 K3.1.3 may set errno */
        errno=ret=EINVAL;
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
        /* Finally, now call what might (or might not) have been the handler at
           the time of the runtime-constraint violation.
        */
        foo("invalid parameters",NULL,errno=ret); /* reset errno in case it was
                                                     changed during calls to
                                                     set_constraint_handler_s
                                                  */
        /* if the handler returns, return error code */
        return ret;
    }

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (2UL>nmemb) return ret; /* Return early if there's nothing to do. */

    /* Simplify tests for selection vs. sorting by ensuring a NULL pointer when
       sorting. Ensure consistency between pk and nk. Ensure sorted pk array
       with no duplicated order statistics.
    */
    if (0UL==nk) pk=NULL;
    else if (NULL==pk) onk=nk=0UL;
    else if (1UL<nk) { /* binary search requires nondecreasing pk */
        size_t p, q;
        for (p=0UL,q=1UL; q<nk; p++,q++) /* verify (linear scan with direct comparison) */
            if (pk[p]>pk[q]) break; /* not nondecreasing */
        if (q<nk) /* fix (sort) iff broken (not nondecreasing) */
            QSORT_FUNCTION_NAME((void *)pk,nk,sizeof(size_t),size_tcmp);
        /* verify, fix uniqueness */
        alignsize=alignment_size((char *)pk,sizeof(size_t));
        size_ratio=sizeof(size_t)/alignsize;
        sswapf=swapn(alignsize);
        for (p=0UL,q=1UL; q<=nk; q++) {
            if ((q==nk)||(pk[p]!=pk[q])) {
                p++;
                if (p!=q) { /* indices [p,q) are duplicates */
                    irotate((char *)pk,p,q,nk,sizeof(size_t),sswapf,alignsize,size_ratio);
                    s=q-p, nk-=s, q=p;
                }
            }
        }
    }

    /* base array alignment size and size_ratio */
    alignsize=alignment_size(base,size);
    size_ratio=size/alignsize;

    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

    /* Direct or indirect sorting based on nmemb and size_ratio. */
    if ((1UL<size_ratio)&&(7UL<nmemb+size_ratio))
        options|=(QUICKSELECT_INDIRECT);

    /* Don't use indirection if size_ratio==1UL (it would be inefficient). */
    if (1UL==size_ratio) options&=~(QUICKSELECT_INDIRECT);
    /* If indirect sorting, alignment size and size_ratio are reset to
       appropriate values for pointers.
    */
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        char **p=set_array_pointers(pointers,nmemb,base,size,0UL,nmemb);
        if (NULL==p) {
            if (NULL!=pointers) { free(pointers); pointers=NULL; }
            options&=~(QUICKSELECT_INDIRECT);
        } else {
            if (pointers!=p) pointers=p;
            alignsize=alignment_size((char *)pointers,sizeof(char *));
            if (0U==instrumented) pswapf=swapn(alignsize);
            else pswapf=iswapn(alignsize);
            A(pointers[0]==base);
        }
    }

    /* Special-case selection and sorting are handled in quickselect_loop. */
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        ret=QUICKSELECT_LOOP((char *)pointers,0UL,nmemb,sizeof(char *),
            COMPAR_ARGS,pk,0UL,nk,pswapf,alignsize,1UL,quickselect_cache_size,0UL,
            options,NULL,NULL);
        if (NULL==indices) indices=(size_t *)pointers;
        indices=convert_pointers_to_indices(base,nmemb,size,pointers,
            nmemb,indices,0UL,nmemb);
        A(NULL!=indices);
        alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
            alignsize);
        free(pointers);
        if ((void *)indices==(void *)pointers) indices=NULL;
        if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort/select */
            options&=~(QUICKSELECT_INDIRECT);
            alignsize=alignment_size(base,size);
            size_ratio=size/alignsize;
            if (0U==instrumented) swapf=swapn(alignsize);
            else swapf=iswapn(alignsize);
            ret=QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,pk,0UL,
                nk,swapf,alignsize,size_ratio,quickselect_cache_size,0UL,options,NULL,
                NULL);
        } else if (0UL!=alignsize)
            nmoves+=alignsize*size_ratio;
    } else
        ret=QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,pk,0UL,nk,
            swapf,alignsize,size_ratio,quickselect_cache_size,0UL,
            options,NULL,NULL);

    /* Restore pk to full sorted (non-unique) order for caller convenience. */
    /* This may be expensive if the caller supplied a large number of duplicate
       order statistic ranks.  So be it.  Caller: if it hurts, don't do it.
    */
    if (0UL!=s) /* there were duplicates */
        QSORT_FUNCTION_NAME((void *)pk,onk,sizeof(size_t),size_tcmp);
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}

QSORT_RETURN_TYPE qsort_s_internal (void *base, size_t nmemb, /*const*/ size_t size,
    COMPAR_DECL)
{
    QSORT_RETURN_TYPE ret=0;
    unsigned int options=0U;
    size_t alignsize, size_ratio;
    void (*swapf)(char *, char *, size_t);
    void (*pswapf)(char *, char *, size_t);
    char **pointers = NULL;

    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    if (((0UL!=nmemb) && ((NULL==base) || (0UL==size) || (NULL==compar)))
    || (RSIZE_MAX<nmemb) || (RSIZE_MAX<size)
    ) {
        constraint_handler_t foo;
        /* N1570 K3.1.3 may set errno */
        errno=ret=EINVAL;
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
        /* Finally, now call what might (or might not) have been the handler at
           the time of the runtime-constraint violation.
        */
        foo("invalid parameters",NULL,errno=ret); /* reset errno in case it was
                                                     changed during calls to
                                                     set_constraint_handler_s
                                                  */
        /* if the handler returns, return error code */
        return ret;
    }

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (1UL<nmemb) { /* nothing to do for fewer than 2 elements */
        /* Determine cache size once on first call. */
        if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();

        /* base array alignment size and size_ratio */
        alignsize=alignment_size(base,size);
        size_ratio=size/alignsize;

        /* Don't use indirection if size_ratio==1UL (it would be inefficient). */
        if (1UL==size_ratio) options&=~(QUICKSELECT_INDIRECT);
        /* If indirect sorting, alignment size and size_ratio are reset to
           appropriate values for pointers.
        */
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            char **p=set_array_pointers(pointers,nmemb,base,size,0UL,nmemb);
            if (NULL==pointers) {
                if (NULL!=pointers) { free(pointers); pointers=NULL; }
                options&=~(QUICKSELECT_INDIRECT);
            } else {
                if (pointers!=p) pointers=p;
                alignsize=alignment_size((char *)pointers,sizeof(char *));
                if (0U==instrumented) pswapf=swapn(alignsize);
                else pswapf=iswapn(alignsize);
                A(pointers[0]==base);
            }
        }

        /* No sorting/selection partial-order stability is supported */

        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            size_t *indices=(size_t *)pointers;
            ret=QUICKSELECT_LOOP((char *)pointers,0UL,nmemb,sizeof(char *),
                COMPAR_ARGS,NULL,0UL,0UL,pswapf,alignsize,1UL,
                quickselect_cache_size,0UL,options,NULL,NULL);
            indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                nmemb,indices,0UL,nmemb);
            A(NULL!=indices);
            alignsize=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                alignsize);
            free(pointers);
            if (nmemb<(alignsize>>1)) { /* indirection failed; use direct sort/select */
                options&=~(QUICKSELECT_INDIRECT);
                alignsize=alignment_size(base,size);
                size_ratio=size/alignsize;
                if (0U==instrumented) swapf=swapn(alignsize);
                else swapf=iswapn(alignsize);
                ret=QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,NULL,0UL,
                    0UL,swapf,alignsize,size_ratio,
                    quickselect_cache_size,0UL,options,NULL,NULL);
                } else if (0UL!=alignsize)
                nmoves+=alignsize*size_ratio;
        } else
            ret=QUICKSELECT_LOOP(base,0UL,nmemb,size,COMPAR_ARGS,NULL,
                0UL,0UL,swapf,alignsize,size_ratio,
                quickselect_cache_size,0UL,options,NULL,NULL);
    }
    A(0==ret); /* shouldn't get an error for internal calls! */
    return ret;
}
#endif /* RSIZE_MAX */
