/*INDENT OFF*/

/* Description: C source code for more-or-less vanilla Bentley&McIlroy qsort */
/* $Id: ~|^` @(#)   This is bmqsort.c version 1.1 dated 2017-09-29T14:34:10Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.bmqsort.c */

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: bmqsort.c ~|^` @(#)"
#define SOURCE_MODULE "bmqsort.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2017-09-29T14:34:10Z"
#define COPYRIGHT_HOLDER "John Wiley"
#define COPYRIGHT_DATE "1993"

/* header files needed */
#include "median_test_config.h" /* configuration */

#include "initialize_src.h"

/* ******************** Bentley & McIlroy qsort ******************************** */
/* BL: namespace changed */
/* qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

    /*assume sizeof(long) is a power of 2 */
    /* BL: added parentheses to make gcc STFU */
#define SWAPINIT(a, es) swaptype =         \
    ((a-(char*)0) | es) % sizeof(long) ? 2 : es > sizeof(long);
#define swapcode(TYPE, parmi, parmj, n) {  \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while ((n -= sizeof(TYPE)) > 0);     \
}

static void swapfunc(char *a, char *b, size_t n, int swaptype)
{   if (swaptype <= 1) swapcode(long, a, b, n)
    else swapcode(char, a, b, n)
}

#define bmswap(a, b)                       \
    if (swaptype == 0) {                   \
        long t = *(long*)(a);              \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
    } else                                 \
        swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)

#define PVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; bmswap(pv, pm); }    \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }

#define bmmin(x, y) ((x)<=(y) ? (x) : (y))

static char *med3(char *a, char *b, char *c, int (*compar)(const void *, const void *))
{
        return compar(a, b) < 0 ?
                  (compar(b, c) < 0 ? b : compar(a, c) < 0 ? c : a)
                : (compar(b, c) > 0 ? b : compar(a, c) > 0 ? c : a);
}

void bmqsort(char *a, size_t n, size_t es, int (*compar)(const void*,const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
        int r, swaptype;
        long v;
        size_t s;

        SWAPINIT(a, es);
        if (n < (BM_INSERTION_CUTOFF)) {         /* Insertion sort on smallest arrays */
                for (pm = a + es; pm < a + n*es; pm += es)
                        for (pl = pm; pl > a && compar(pl-es, pl) > 0; pl -= es)
                                bmswap(pl, pl-es);
                return;
        }
        pm = a + (n/2)*es;    /* Small arrays, middle element */
        if (n > 7) {
                pl = a;
                pn = a + (n-1)*es; /* moved outside of pivot selection guard */
                if (n > 40) {    /* Big arrays, pseudomedian of 9 */
                        s = (n/8)*es;
                        pl = med3(pl, pl+s, pl+2*s, compar);
                        pm = med3(pm-s, pm, pm+s, compar);
                        pn = med3(pn-2*s, pn-s, pn, compar);
                }
                pm = med3(pl, pm, pn, compar); /* Mid-size, med of 3 */
        }
        PVINIT(pv, pm);       /* pv points to partition value */
        pa = pb = a; /* original code; redundant comparison pa vs. pa */
        pc = pd = a + (n-1)*es; /* original code; could have been set along with pn before pivot selection test */
        for (;;) {
                while (pb <= pc && (r = compar(pb, pv)) <= 0) {
                        if (r == 0) { bmswap(pa, pb); pa += es; }
                        pb += es;
                }
                while (pb <= pc && (r = compar(pc, pv)) >= 0) {
                        if (r == 0) { bmswap(pc, pd); pd -= es; }
                        pc -= es;
                }
                if (pb > pc) break;
                bmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        pn = a + n*es;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower = region */
        /* pa-a (a.k.a. pa-pl) is the number of chars in the lower = block */
        /* pb-pa is the number of chars in the < block */
        s = bmmin(pa-a,  pb-pa   ); vecswap(a,  pb-s, s);
        /* after the swap (above), the start of the = section is at pb-s */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper = region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper = block */
        /* pd-pc is the number of chars in the > block */
        s = bmmin(pd-pc, pn-pd-es); vecswap(pb, pn-s, s);

        if ((s = pb-pa) > es) bmqsort(a,    s/es, es, compar);
        if ((s = pd-pc) > es) bmqsort(pn-s, s/es, es, compar);
}
