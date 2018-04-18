#ifndef INSERTION_SORT_SRC_H_INCLUDED
# define INSERTION_SORT_SRC_H_INCLUDED 1
/*INDENT OFF*/

/* Description: common C source code for isort_bs */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    insertion_sort_src.h copyright 2017-2018 Bruce Lilly.   \ insertion_sort_src.h $
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
/* $Id: ~|^` @(#)   This is insertion_sort_src.h version 1.6 dated 2018-04-16T05:52:06Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.insertion_sort_src.h */

/********************** Long description and rationale: ***********************
 This file contains source to implement the internal function isort_bs.
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
#define ID_STRING_PREFIX "$Id: insertion_sort_src.h ~|^` @(#)"
#define SOURCE_MODULE "insertion_sort_src.h"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2018-04-16T05:52:06Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"

#ifndef ASSERT_CODE
# define ASSERT_CODE        0  /* for validity testing; 0 for production code */
#endif

/* local header files needed */
#include "quickselect_config.h" /* BS_MID_L BS_MID_H QUICKSELECT_INLINE */
#include "exchange.h"           /* irotate protate reverse */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* system header files */
#include <assert.h>             /* assert */
#include <stddef.h>             /* size_t NULL */
#if ASSERT_CODE + DEBUG_CODE
# include <stdio.h>
#endif

/* macros */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
# ifndef INSERTION_SORT_SRC_FILE_HERE
extern char insertion_sort_src_file[];
extern char insertion_sort_src_file_initialized;
# else
/* not static; referenced by inline functions */
char insertion_sort_src_file[PATH_MAX];
char insertion_sort_src_file_initialized=0;
# endif
#endif

#ifndef SWAP_COUNT_STATEMENT
# define SWAP_COUNT_STATEMENT /**/
#endif

/* inline code */
/* insertion sort split into easily-digestible bite-size pieces:
   find the largest existing in-order run in the sub-array
   search for the position in the in-order run in which to insert an
      element adjacent to that run, using binary search (in insertion.h)
   insertion by rotation of elements (or pointers to elements) (in insertion.h)
   combined search-and-insert (in insertion.h)
   overall insertion sort with optimizations
*/

#include "insertion.h" /* inline code: binary_search insert search_and_insert */

/* Find a maximum-length in-order run of elements in [first,beyond) in
   base array.  Place the index of the start of the run in *s.  Return the
   length of the run (in elements).  For equal-length runs, prefer a run
   mearest the middle of the sub-array.
*/
static QUICKSELECT_INLINE
size_t in_order_run(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL, size_t size_ratio, size_t *s, unsigned int options)
{
    register char *pe=base+first*size;

    size_t f, i, len=1UL; /* minimum possible (for reversed input) */

    if (NULL!=s) *s=first;
#if ASSERT_CODE
    else return 0UL; /* indicates error (shouldn't happen) */
#endif
    if (beyond>first+1UL) {
        /* no options tests in loops! */
        if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
            for (f=i=first; i<beyond; pe+=size) {
                if ((++i==beyond)||(0<COMPAR(pe,pe+size))) {
                    if (i-f>=len) {
                        if (i-f==len) { /* try for more balanced ends */
                            size_t oldl, oldr, newl, newr, olddiff, newdiff;
                            oldl=*s-first, oldr=beyond-(*s+len);
                            if (oldl>oldr) olddiff=oldl-oldr;
                            else olddiff=oldr-oldl;
                            newl=f-first, newr=beyond-i;
                            if (newl>newr) newdiff=newl-newr;
                            else newdiff=newr-newl;
                            if (newdiff<olddiff) *s=f;
                        } else
                            len=i-f, *s=f; /* new longest run */
                    }
                    f=i; /* next run starts at pe+size */
                }
            }
        } else { /* indirect */
            register char **ptrs=(char **)base;
            pe=ptrs[first];
            for (f=i=first; i<beyond; pe=ptrs[i]) {
                if ((++i==beyond)||(0<COMPAR(pe,ptrs[i]))) {
                    if (i-f>=len) {
                        if (i-f==len) { /* try for more balanced ends */
                            size_t oldl, oldr, newl, newr, olddiff, newdiff;
                            oldl=*s-first, oldr=beyond-(*s+len);
                            if (oldl>oldr) olddiff=oldl-oldr;
                            else olddiff=oldr-oldl;
                            newl=f-first, newr=beyond-i;
                            if (newl>newr) newdiff=newl-newr;
                            else newdiff=newr-newl;
                            if (newdiff<olddiff) *s=f;
                        } else
                            len=i-f, *s=f;
                    }
                    f=i;
                }
            }
        }
    }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: base=%p, first=%lu, beyond=%lu, "
            "longest in-order run starts at index %lu, has length %lu (end at "
            "%lu) */\n",__func__,source_file,__LINE__,(void *)base,first,beyond,
            *s,len,*s+len-1UL);
        print_some_array(base,first,beyond-1UL, "/* "," */",options);
    }
#endif
    return len;
}

/* Insertion sort using binary search to locate insertion position for
   out-of-order element, followed by rotation to insert the element in position.
   Insertion begins with the largest in-order run in the input, to avoid
   pathological performance e.g. with rotated (either direction!) input.
   Finding the longest in-order run has the side-effect of finding reversed
   input (the longest in-order run length is 1 element), and that fact is
   exploited by reversing reversed input w/o further comparisons. Obviously, if
   the in-order run includes all elements, the input is already sorted and no
   data need be moved.
*/
/* Included here for inline use in dedicated_sort, introsort (when not using
   overall linear insertion sort), mbmqsort (optional alternative to
   dedicated_sort and isort_ls), and isort.
*/
static QUICKSELECT_INLINE
void isort_bs(char *base, size_t first, size_t beyond, size_t size,
    COMPAR_DECL,
    void (*swapf)(char *,char *,size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    auto size_t len; /* auto because address is required */
    register size_t l, m, n, h;

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if ((char)0==insertion_sort_src_file_initialized) {
        (V)path_basename(__FILE__,insertion_sort_src_file,PATH_MAX);
        insertion_sort_src_file_initialized++;
    }
    if (DEBUGGING(SORT_SELECT_DEBUG)||DEBUGGING(METHOD_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, "
            "size=%lu, size_ratio=%lu, compar=%s, options=0x%x"
            " */\n",__func__,insertion_sort_src_file,__LINE__,
            (unsigned long)first,(unsigned long)beyond,
            (unsigned long)size,(unsigned long)size_ratio,
            comparator_name(compar),options);
    }
#endif
    m=in_order_run(base,first,beyond,size,COMPAR_ARGS,size_ratio,&len,options);
    A(0UL!=m);
    if (1UL==m) { /* reversed input */
        reverse(base,first,beyond,size,swapf,alignsize,size_ratio);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(SORT_SELECT_DEBUG))
            print_some_array(base,first,beyond-1UL,"/* reversed: "," */",
                options);
#endif
    } else if (m!=beyond-first) { /* not all in-order */
        l=len;
        /* no options tests in loops! */
        if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
            register char *pa;
            /* It is known (from the scan for maximum-length in-order runs)
               that the element to the immediate left of the start of that run
               (if such an element exists) is out-of-order (no recomparison
               required).
            */
            h=l+m-1UL;
            if (l>first) {
                search_and_insert(base,n=l-1UL,l,h,size,COMPAR_ARGS,swapf,
                    alignsize,size_ratio,options);
                for (l=n,pa=base+l*size; first<l; l--,pa-=size) {
                    n--;
                    if (0<COMPAR(pa-size,pa))
                        search_and_insert(base,n,l,h,size,COMPAR_ARGS,swapf,
                            alignsize,size_ratio,options);
                }
            } A(l==first);
            /* It is known (from the scan for maximum-length in-order runs)
               that the element to the immediate right of the end of that run
               (if such an element exists) is out-of-order (no recomparison
               required).
            */
            if (h+1UL<beyond) {
                search_and_insert(base,n=h+1UL,l,h,size,COMPAR_ARGS,swapf,
                    alignsize,size_ratio,options);
                for (h=n++,pa=base+h*size; n<beyond; n++,pa+=size,h++) {
                    if (0<COMPAR(pa,pa+size))
                        search_and_insert(base,n,l,h,size,COMPAR_ARGS,swapf,
                            alignsize,size_ratio,options);
                }
            } A(n==beyond);
        } else { /* indirect */
            register char **ptrs=(char **)base, *pa;
            h=l+m-1UL;
            if (l>first) {
                search_and_insert(base,n=l-1UL,l,h,size,COMPAR_ARGS,
                    swapf,alignsize,size_ratio,options);
                for (l=n,pa=ptrs[n]; first<l; pa=ptrs[n],l--) {
                    n--; /* beware side-effects of macros... */
                    if (0<COMPAR(ptrs[n],pa))
                        search_and_insert(base,n,l,h,size,COMPAR_ARGS,swapf,
                            alignsize,size_ratio,options);
                }
            }
            if (h+1UL<beyond) {
                search_and_insert(base,n=h+1UL,l,h,size,COMPAR_ARGS,swapf,
                    alignsize,size_ratio,options);
                for (h=n++,pa=ptrs[h]; n<beyond; pa=ptrs[n++],h++) {
                    if (0<COMPAR(pa,ptrs[n]))
                        search_and_insert(base,n,l,h,size,COMPAR_ARGS,swapf,
                            alignsize,size_ratio,options);
                }
            }
        }
    }
}
#endif /* INSERTION_SORT_SRC_H_INCLUDED */
