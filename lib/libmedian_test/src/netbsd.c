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
* $Id: ~|^` @(#)    netbsd.c modifications copyright 2016-2018 Bruce Lilly.   \ netbsd.c $
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
/* $Id: ~|^` @(#)   This is netbsd.c version 1.3 dated 2018-05-06T21:07:51Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.netbsd.c */

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
#define ID_STRING_PREFIX "$Id: netbsd.c ~|^` @(#)"
#define SOURCE_MODULE "netbsd.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2018-05-06T21:07:51Z"
#define COPYRIGHT_HOLDER "The Regents of the University of California.  All rights reserved."
#define COPYRIGHT_DATE "1992, 1993"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* ************************************************************************** */
/*	$NetBSD: qsort.c,v 1.22 2012/05/26 21:47:05 christos Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0 /* BL non-standard header excluded */
#include <sys/cdefs.h>
#endif
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)qsort.c	8.1 (Berkeley) 6/4/93";
#else
__RCSID("$NetBSD: qsort.c,v 1.22 2012/05/26 21:47:05 christos Exp $");
#endif
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

static inline char	*med3(char *, char *, char *,
    int (*)(const void *, const void *));
/* BL name change for instrumented code */
static inline void	 iswapfunc(char *, char *, size_t, int);

#define min(a, b)	(a) < (b) ? a : b

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
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

static inline void iswapfunc(char *a, char *b, size_t n, int swaptype)
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

#define bmmin(x, y) ((x)<=(y) ? (x) : (y))

static inline char *med3(char *a, char *b, char *c, int (*compar)(const void *, const void *))
{
        return compar(a, b) < 0 ?
                  (compar(b, c) < 0 ? b : compar(a, c) < 0 ? c : a)
                : (compar(b, c) > 0 ? b : compar(a, c) > 0 ? c : a);
}

/* NetBSD qsort code (derived from Bentley&McIlroy code) */
/* BL: _DIAGASSERT -> assert, name change */
void nbqsort(void *a, size_t n, size_t es, int (*compar)(const void *, const void *))
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
        size_t d, r;
        int swaptype, cmp_result;

        assert(a != NULL || n == 0 || es == 0);
        assert(compar != NULL);

/* BL name change */
nbloop:        SWAPINIT((char *)a, es);
        if (n < 7) {
                for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
                        for (pl = pm; pl > (char *) a && compar(pl - es, pl) > 0;
                             pl -= es)
                                ibmswap(pl, pl - es);
                return;
        }
        pm = (char *) a + (n / 2) * es;
        if (n > 7) {
                pl = (char *)a;
                pn = (char *) a + (n - 1) * es;
                if (n > 40) {
                        d = (n / 8) * es;
                        pl = med3(pl, pl + d, pl + 2 * d, compar);
                        pm = med3(pm - d, pm, pm + d, compar);
                        pn = med3(pn - 2 * d, pn - d, pn, compar);
                }
                pm = med3(pl, pm, pn, compar);
        }
        ibmswap(a, pm);
        pa = pb = (char *) a + es;

        pc = pd = (char *) a + (n - 1) * es;
        for (;;) {
                while (pb <= pc && (cmp_result = compar(pb, a)) <= 0) {
                        if (cmp_result == 0) {
                                ibmswap(pa, pb);
                                pa += es;
                        }
                        pb += es;
                }
                while (pb <= pc && (cmp_result = compar(pc, a)) >= 0) {
                        if (cmp_result == 0) {
                                ibmswap(pc, pd);
                                pd -= es;
                        }
                        pc -= es;
                }
                if (pb > pc)
                        break;
                ibmswap(pb, pc);
                pb += es;
                pc -= es;
        }
        npartitions++;

        pn = (char *) a + n * es;
        /* compute the size of the block of chars that needs to be moved to put the < region before the lower = region */
        /* pb-pa is the number of chars in the < block */
        r = bmmin(pa - (char *) a, pb - pa);
        ivecswap(a, pb - r, r);
        /* after the swap (above), the start of the = section is at pb-r */
        /* compute the size of the block of chars that needs to be moved to put the > region after the upper = region */
        /* pn-pd-es (a.k.a. pu-pd) is the number of chars in the upper = block */
        /* pd-pc is the number of chars in the > block */
        r = bmmin((size_t)(pd - pc), pn - pd - es);
        ivecswap(pb, pn - r, r);

        if ((r = pb - pa) > es)
                nbqsort(a, r / es, es, compar);
        if ((r = pd - pc) > es) {
                /* Iterate rather than recurse to save stack space */
                a = pn - r;
                n = r / es;
                goto nbloop;
        }
/*                nbqsort(pn - r, r / es, es, compar);*/
        /* BL: added initialization for ID strings */
        if ((char)0==file_initialized) initialize_file(__FILE__);
}
/* ************************************************************************** */
