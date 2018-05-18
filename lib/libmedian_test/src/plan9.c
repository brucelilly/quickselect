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
* $Id: ~|^` @(#)    plan9.c modifications copyright 2016-2017 Bruce Lilly.   \ plan9.c $
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
/* $Id: ~|^` @(#)   This is plan9.c version 1.5 dated 2018-05-06T21:07:51Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.plan9.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: plan9.c ~|^` @(#)"
#define SOURCE_MODULE "plan9.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2018-05-06T21:07:51Z"
#define COPYRIGHT_HOLDER "Lucent Technologies Inc"
#define COPYRIGHT_DATE "2002"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* ************************************************************** */
/* Plan 9 qsort; name changed */

/*
 * qsort -- simple quicksort
 */

#if 0
#include <u.h>
#else
typedef unsigned long long uintptr;
#endif

typedef
struct
{
/* BL: added const qualifiers to comparison function argument to conform to standard */
        int        (*cmp)(const void*, const void*);
        void        (*swap)(char*, char*, long);
        long        es;
} Sort;

/* BL: added instrumentation to swap functions */
/* BL: N.B. no provision for swapping by sizes other than char, long */
static        void
swapb(char *i, char *j, long es)
{
        char c;

        do {
                c = *i;
                *i++ = *j;
                *j++ = c;
                nsw++; /* BL */
                es--;
        } while(es != 0);

}

static        void
swapi(char *ii, char *ij, long es)
{
        long *i, *j, c;

        i = (long*)ii;
        j = (long*)ij;
        do {
                c = *i;
                *i++ = *j;
                *j++ = c;
                nsw++; /* BL */
                es -= sizeof(long);
        } while(es != 0);
}

static        char*
pivot(char *a, long n, Sort *p)
{
        long j;
        char *pi, *pj, *pk;

/* BL: N.B. nonuniformity due to integer arithmetic truncation */
        j = n/6 * p->es;
        pi = a + j;        /* 1/6 */
        j += j;
        pj = pi + j;        /* 1/2 */
        pk = pj + j;        /* 5/6 */
/* BL: N.B. no provision for early return on == comparison results */
        if(p->cmp(pi, pj) < 0) {
                if(p->cmp(pi, pk) < 0) {
                        if(p->cmp(pj, pk) < 0)
                                return pj;
                        return pk;
                }
                return pi;
        }
        if(p->cmp(pj, pk) < 0) {
                if(p->cmp(pi, pk) < 0)
                        return pi;
                return pk;
        }
        return pj;
}

static        void
qsorts(char *a, long n, Sort *p)
{
               long j, es;
        char *pi, *pj, *pn;

        es = p->es;
/* BL: no dedicated sort for small sub-arrays */
        while(n > 1) {
/* BL: 3 sample pivot selection at size >= 11 elements; never more samples, 1 sample at mid-point for <= 10 elements */
                if(n > 10) {
                        nfrozen=0UL, pivot_minrank=1UL; /* BL: for antiqsort */
                        pi = pivot(a, n, p);
                } else
                        pi = a + (n>>1)*es;

                p->swap(a, pi, es);
                pi = a;
                pn = a + n*es;
                pj = pn;
/* BL: no handling of equal elements; expect poor performance for all-equal input */
                for(;;) {
                        do
                                pi += es;
                        while(pi < pn && p->cmp(pi, a) < 0);
                        do
                                pj -= es;
                        while(pj > a && p->cmp(pj, a) > 0);
                        if(pj < pi)
                                break;
                        p->swap(pi, pj, es);
                }
                p->swap(a, pj, es);
                j = (pj - a) / es;

                npartitions++; /* BL: added partitions instrumentation */
                n = n-j-1;
                if(j >= n) {
                        nrecursions++; /* BL: added recursion instrumentation */
                        qsorts(a, j, p);
                        a += (j+1)*es;
                } else {
                        nrecursions++; /* BL: added recursion instrumentation */
                        qsorts(a + (j+1)*es, n, p);
                        n = j;
                }
        }
}

/* BL: long for #elements, element size incompatible with standard qsort */
/* BL: added qualifiers (const in cmp) */
void
p9qsort(void *va, long n, long es, int (*cmp)(const void*, const void*))
{
        Sort s;

    if ((char)0==file_initialized) initialize_file(__FILE__); /* BL */
        s.cmp = cmp;
        s.es = es;
        s.swap = swapi;
        if(((uintptr)va | es) % sizeof(long))
                s.swap = swapb;
        qsorts((char*)va, n, &s);
}
/* ************************************************************** */
