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
* $Id: ~|^` @(#)    ibmqsort.c copyright 2016-2017 Bruce Lilly.   \ ibmqsort.c $
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
/* $Id: ~|^` @(#)   This is ibmqsort.c version 1.2 dated 2017-11-03T19:58:17Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.ibmqsort.c */

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
#define ID_STRING_PREFIX "$Id: ibmqsort.c ~|^` @(#)"
#define SOURCE_MODULE "ibmqsort.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2017-11-03T19:58:17Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* ******************** Bentley & McIlroy qsort ******************************** */
/* namespace changed */
/* qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

    /*assume sizeof(long) is a power of 2 */
    /* BL: added parentheses to make gcc STFU */
#define SWAPINIT(a, es) swaptype =         \
    ((a-(char*)0) | es) % sizeof(long) ? 2 : es > sizeof(long);
#define iswapcode(TYPE, parmi, parmj, n) { \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    nsw += (n)/sizeof(TYPE);               \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while ((n -= sizeof(TYPE)) > 0);     \
}

static void iswapfunc(char *a, char *b, size_t n, int swaptype)
{   if (swaptype <= 1) iswapcode(long, a, b, n)
    else iswapcode(char, a, b, n)
}
#define ibmswap(a, b)                      \
    if (swaptype == 0) {                   \
        long t = *(long*)(a);              \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
        nsw++;                             \
    } else                                 \
        iswapfunc(a, b, es, swaptype)

#define ivecswap(a, b, n) if (n > 0) iswapfunc(a, b, n, swaptype)

#define IPVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; ibmswap(pv, pm); }    \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }

#define bmmin(x, y) ((x)<=(y) ? (x) : (y))

static char *med3(char *a, char *b, char *c, int (*compar)(const void *, const void *))
{
        return compar(a, b) < 0 ?
                  (compar(b, c) < 0 ? b : compar(a, c) < 0 ? c : a)
                : (compar(b, c) > 0 ? b : compar(a, c) > 0 ? c : a);
}

/* instrumented version */
void ibmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s;

        if ((char)0==file_initialized) initialize_file(__FILE__);
        SWAPINIT(a, es);
        if (n < (BM_INSERTION_CUTOFF)) {         /* Insertion sort on smallest arrays */
                for (pm = a + es; pm < a + n*es; pm += es)
                        for (pl = pm; pl > a && compar(pl-es, pl) > 0; pl -= es)
                                ibmswap(pl, pl-es);
                return;
        }
        pm = a + (n/2)*es;    /* Small arrays, middle element */
        nfrozen=0UL;
        if (n > 7) {
                pl = a;
                pn = a + (n-1)*es; /* moved outside of pivot selection guard */
                if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                        pivot_minrank=3UL;
                        s = (n/8)*es;
                        pl = med3(pl, pl+s, pl+2*s, compar);
                        pm = med3(pm-s, pm, pm+s, compar);
                        pn = med3(pn-2*s, pn-s, pn, compar);
                }
                else pivot_minrank=1UL;
                pm = med3(pl, pm, pn, compar); /* Mid-size, med of 3 */
        } else pivot_minrank=0UL;
        IPVINIT(pv, pm);       /* pv points to partition value */
        pivot_minrank=n;
        pa = pb = a; /* original code; redundant comparison pa vs. pa */
        pc = pd = a + (n-1)*es; /* original code; could have been set along with pn before pivot selection test */
        for (;;) {
                while (pb <= pc && (r = compar(pb, pv)) <= 0) {
                        if (r == 0) { ibmswap(pa, pb); pa += es; }
                        pb += es;
                }
                while (pb <= pc && (r = compar(pc, pv)) >= 0) {
                        if (r == 0) { ibmswap(pc, pd); pd -= es; }
                        pc -= es;
                }
                if (pb > pc) break;
                ibmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        pn = a + n*es;
        npartitions++;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower = region */
        /* pa-a (a.k.a. pa-pl) is the number of chars in the lower = block */
        /* pb-pa is the number of chars in the < block */
        s = bmmin(pa-a,  pb-pa   ); ivecswap(a,  pb-s, s);
        /* after the swap (above), the start of the = section is at pb-s */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper = region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper = block */
        /* pd-pc is the number of chars in the > block */
        s = bmmin(pd-pc, pn-pd-es); ivecswap(pb, pn-s, s);

        nrecursions++;
        if ((s = pb-pa) > es) ibmqsort(a,    s/es, es, compar);
        nrecursions++;
        if ((s = pd-pc) > es) ibmqsort(pn-s, s/es, es, compar);
}
/* ************************************************************************** */

/* worst-case version for adversary priming */
void wbmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s;

        if ((char)0==file_initialized) initialize_file(__FILE__);
        SWAPINIT(a, es);
        if (n < (BM_INSERTION_CUTOFF)) {         /* Insertion sort on smallest arrays */
                for (pm = a + es; pm < a + n*es; pm += es)
                        for (pl = pm; pl > a && compar(pl-es, pl) > 0; pl -= es)
                                ibmswap(pl, pl-es);
                return;
        }
        pm = a + (n/2)*es;    /* Small arrays, middle element */
        if (aqcmp==compar) {
            nfrozen=0UL;
            if (n > 7) {
                    if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                            pivot_minrank=3UL;
                    } else pivot_minrank=1UL;
            } else pivot_minrank=0UL;
            if (n > 7) {
                    pl = a;
                    (V)freeze(aqindex(pl,a,es));
                    if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                            s = (n/8)*es;
                            (V)freeze(aqindex(pl+s,a,es));
                            (V)freeze(aqindex(pm-s,a,es));
                    }
            }
            (V)freeze(aqindex(pm,a,es));
        }
        if (n > 7) {
                pl = a;
                pn = a + (n-1)*es; /* moved outside of pivot selection guard */
                if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                        s = (n/8)*es;
                        pl = med3(pl, pl+s, pl+s+s, compar);
                        pm = med3(pm-s, pm, pm+s, compar);
                        pn = med3(pn-s-s, pn-s, pn, compar);
                }
                pm = med3(pl, pm, pn, compar); /* Mid-size, med of 3 */
        }
        IPVINIT(pv, pm);       /* pv points to partition value */
        pivot_minrank=n;
        pa = pb = a; /* original code; redundant comparison pa vs. pa */
        pc = pd = a + (n-1)*es; /* original code; could have been set along with pn before pivot selection test */
        for (;;) {
                while (pb <= pc && (r = compar(pb, pv)) <= 0) {
                        if (r == 0) { ibmswap(pa, pb); pa += es; }
                        pb += es;
                }
                while (pb <= pc && (r = compar(pc, pv)) >= 0) {
                        if (r == 0) { ibmswap(pc, pd); pd -= es; }
                        pc -= es;
                }
                if (pb > pc) break;
                ibmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        pn = a + n*es;
        npartitions++;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower = region */
        /* pa-a (a.k.a. pa-pl) is the number of chars in the lower = block */
        /* pb-pa is the number of chars in the < block */
        s = bmmin(pa-a,  pb-pa   ); ivecswap(a,  pb-s, s);
        /* after the swap (above), the start of the = section is at pb-s */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper = region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper = block */
        /* pd-pc is the number of chars in the > block */
        s = bmmin(pd-pc, pn-pd-es); ivecswap(pb, pn-s, s);

        nrecursions++;
        if ((s = pb-pa) > es) ibmqsort(a,    s/es, es, compar);
        nrecursions++;
        if ((s = pd-pc) > es) ibmqsort(pn-s, s/es, es, compar);
}
/* ************************************************************************** */
