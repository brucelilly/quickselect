/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining inline functions binary_search, insert, search_and_insert
    useful for insertion sort
*/
#ifndef	INSERTION_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    insertion.h copyright 2018 Bruce Lilly. \ insertion.h $
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
/* $Id: ~|^` @(#)   This is insertion.h version 1.4 dated 2018-03-20T19:32:33Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "insertion" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.insertion.h */

/********************** Long description and rationale: ***********************
 Inline support functions for insertion sort using binary search for insertion
 position and insertion by rotation.
******************************************************************************/

/* version-controlled header file version information */
#define INSERTION_H_VERSION "insertion.h 1.4 2018-03-20T19:32:33Z"

#ifndef ASSERT_CODE
# define ASSERT_CODE        0  /* for validity testing; 0 for production code */
#endif

/* inline code */
#if !defined(COMPAR_DECL) || !defined(COMPAR) || !defined(QUICKSELECT_INDIRECT)
# include "quickselect_config.h"
#endif
#include "exchange.h"           /* irotate */

#include <assert.h>             /* assert */
#include <stddef.h>             /* size_t */

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define INSERTION_INLINE inline
#else
# define INSERTION_INLINE /**/
#endif /* C99 */

/* Binary search for sorted rank of key k in range [l,h] of array at base.
   Inputs k, l, h are indices of base array elements (element size size).
   Return value is the index that key k should have if the elements in the
   search range plus the key (which is adjacent to one end of the search
   range) are placed in sorted order, i.e. the position at which the key
   should be inserted to achieve sorted order. The return value is in the
   range [l,h]. It is assumed that the key has already been compared with
   the nearest element of [l,h] and has been found to be out-of-order with
   that nearest element; the key is not recompared with that element.
*/
static INSERTION_INLINE
size_t binary_search(char *base, register size_t k, register size_t l,
    register size_t h, size_t size, COMPAR_DECL, size_t size_ratio,
    unsigned int options)
{
    register char *pk;
    register size_t m;

    A(l<=h);A((k==l-1UL)||(k==h+1UL));
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: base=%p, search for insertion "
            "position for k=%lu in [%lu,%lu] */\n"
            ,__func__,source_file,__LINE__,(void *)base,k,l,h);
        print_some_array(base,k<l?k:l,k<h?h:k, "/* "," */",options);
    }
#endif
    if (h>l) { /* else simply swap k,l (l==h) */
        /* binary search for insertion position */
        /* no options tests in loops! */
        if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
            pk=base+k*size;
            if (k<l) {
                /* at termination, l=h=index of the rightmost element less than
                   the element pointed to by pk (index k)
                */
                while (l<h) {
                    m=BS_MID_H(l,h); A(m!=l);
                    if (0>=COMPAR(pk,base+m*size)) h=m-1UL; else l=m;
                }
            } else { /* k>h */
                /* at termination, l=h=index of leftmost element greater than or
                   equal to the element pointed to by pk (index k)
                */
                while (l<h) {
                    m=BS_MID_L(l,h); A(m!=h);
                    if (0<=COMPAR(pk,base+m*size)) l=m+1UL; else h=m;
                }
            }
        } else { /* indirect */
            register char **ptrs=(char **)base, *pm;
            pk=ptrs[k];
            if (k<l) {
                while (l<h) {
                    m=BS_MID_H(l,h); A(m!=l); pm=ptrs[m];
                    if (0>=COMPAR(pk,pm)) h=m-1UL; else l=m;
                }
            } else { /* k>h */
                while (l<h) {
                    m=BS_MID_L(l,h); A(m!=h); pm=ptrs[m];
                    if (0<=COMPAR(pk,pm)) l=m+1UL; else h=m;
                }
            }
        }
    }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: base=%p, insertion position for "
            "k=%lu is l=%lu */\n",__func__,source_file,__LINE__,(void *)base,k,
            l);
    }
#endif
    return l;
}

/* Insert element of base array with index i at index j by rotating elements
   [i,j] or [j,i] (depending on whether i<j or i>j) by one position.
*/
static INSERTION_INLINE
void insert(char *base, size_t i, size_t j, size_t size,
    void (*swapf)(char *,char *,size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    A(i!=j);
    if (i<j) {
        irotate(base,i,i+1UL,j+1UL,size,swapf,alignsize,size_ratio);
#ifdef QUICKSELECT_COUNT_ROTATIONS
        QUICKSELECT_COUNT_ROTATIONS(j+1UL-i,size_ratio);
#endif
    } else {
        irotate(base,j,i,i+1UL,size,swapf,alignsize,size_ratio);
#ifdef QUICKSELECT_COUNT_ROTATIONS
        QUICKSELECT_COUNT_ROTATIONS(i+1UL-j,size_ratio);
#endif
    }
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(SORT_SELECT_DEBUG)) {
        (V)fprintf(stderr,"/* %s: %s line %d: inserted i=%lu at j=%lu */\n",
            __func__,source_file,__LINE__,i,j);
        print_some_array(base,i<j?i:j,i>j?i+1UL:j, "/* "," */",options);
    }
#endif
}

/* Combined binary search and insertion by rotation */
static INSERTION_INLINE
void search_and_insert(char *base, size_t k, size_t l, size_t h, size_t size,
    COMPAR_DECL,
    void (*swapf)(char *,char *,size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    insert(base,k,binary_search(base,k,l,h,size,COMPAR_ARGS,size_ratio,options),
        size,swapf,alignsize,size_ratio,options);
}

#define	INSERTION_H_INCLUDED
#endif
