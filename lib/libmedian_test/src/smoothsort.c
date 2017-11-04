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
* $Id: ~|^` @(#)    smoothsort.c copyright 2016-2017 Bruce Lilly.   \ smoothsort.c $
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
/* $Id: ~|^` @(#)   This is smoothsort.c version 1.1 dated 2017-09-29T14:34:10Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.smoothsort.c */

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
#define ID_STRING_PREFIX "$Id: smoothsort.c ~|^` @(#)"
#define SOURCE_MODULE "smoothsort.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2017-09-29T14:34:10Z"
#define COPYRIGHT_HOLDER "Valentin Ochs"
#define COPYRIGHT_DATE "2011"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* ************************************************************** */
/* BL: Smoothsort implementation from musl
    http://git.musl-libc.org/cgit/musl/tree/src/stdlib/qsort.c
   redundant header inclusion ifdef'ed, qsort->smoothsort, add swap counts
*/
/* Copyright (C) 2011 by Valentin Ochs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Minor changes by Rich Felker for integration in musl, 2011-04-27. */
/* BL: added preprocessor macros to support C89/C90 compilation */

#if 0
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "atomic.h"
#else
#define a_ctz_l a_ctz_l
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int a_ctz_l(unsigned long x)
{
        __asm__( "bsf %1,%0" : "=r"(x) : "r"(x) );
        return x;
}
#endif
#define ntz(x) a_ctz_l((x))

typedef int (*cmpfun)(const void *, const void *);

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
int pntz(size_t p[2]) {
        int r = ntz(p[0] - 1);
        if(r != 0 || (r = 8*sizeof(size_t) + ntz(p[1])) != 8*sizeof(size_t)) {
                return r;
        }
        return 0;
}

static void cycle(size_t width, unsigned char* ar[], int n)
{
        unsigned char tmp[256];
        size_t l;
        int i;

        if(n < 2) {
                return;
        }

        ar[n] = tmp;
        while(width) {
                l = sizeof(tmp) < width ? sizeof(tmp) : width;
                memcpy(ar[n], ar[0], l);
                for(i = 0; i < n; i++) {
                        memcpy(ar[i], ar[i + 1], l);
                        ar[i] += l;
                }
                width -= l;
        }
}

/* shl() and shr() need n > 0 */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shl(size_t p[2], int n)
{
        if(n >= 8 * sizeof(size_t)) {
                n -= 8 * sizeof(size_t);
                p[1] = p[0];
                p[0] = 0;
        }
        p[1] <<= n;
        p[1] |= p[0] >> (sizeof(size_t) * 8 - n);
        p[0] <<= n;
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void shr(size_t p[2], int n)
{
        if(n >= 8 * sizeof(size_t)) {
                n -= 8 * sizeof(size_t);
                p[0] = p[1];
                p[1] = 0;
        }
        p[0] >>= n;
        p[0] |= p[1] << (sizeof(size_t) * 8 - n);
        p[1] >>= n;
}

static void sift(unsigned char *head, size_t width, cmpfun cmp, int pshift, size_t lp[])
{
        unsigned char *rt, *lf;
        unsigned char *ar[14 * sizeof(size_t) + 1];
        int i = 1;

        ar[0] = head;
        while(pshift > 1) {
                rt = head - width;
                lf = head - width - lp[pshift - 2];

                if((*cmp)(ar[0], lf) >= 0 && (*cmp)(ar[0], rt) >= 0) {
                        break;
                }
                if((*cmp)(lf, rt) >= 0) {
                        ar[i++] = lf;
                        head = lf;
                        pshift -= 1;
                } else {
                        ar[i++] = rt;
                        head = rt;
                        pshift -= 2;
                }
        }
        cycle(width, ar, i);
}

static void trinkle(unsigned char *head, size_t width, cmpfun cmp, size_t pp[2], int pshift, int trusty, size_t lp[])
{
        unsigned char *stepson,
                      *rt, *lf;
        size_t p[2];
        unsigned char *ar[14 * sizeof(size_t) + 1];
        int i = 1;
        int trail;

        p[0] = pp[0];
        p[1] = pp[1];

        ar[0] = head;
        while(p[0] != 1 || p[1] != 0) {
                stepson = head - lp[pshift];
                if((*cmp)(stepson, ar[0]) <= 0) {
                        break;
                }
                if(!trusty && pshift > 1) {
                        rt = head - width;
                        lf = head - width - lp[pshift - 2];
                        if((*cmp)(rt, stepson) >= 0 || (*cmp)(lf, stepson) >= 0) {
                                break;
                        }
                }

                ar[i++] = stepson;
                head = stepson;
                trail = pntz(p);
                shr(p, trail);
                pshift += trail;
                trusty = 0;
        }
        if(!trusty) {
                cycle(width, ar, i);
                sift(head, width, cmp, pshift, lp);
        }
}

void smoothsort(void *base, size_t nel, size_t width, cmpfun cmp)
{
        size_t lp[12*sizeof(size_t)];
        size_t i, size = width * nel;
        unsigned char *head, *high;
        size_t p[2] = {1, 0};
        int pshift = 1;
        int trail;

        if ((char)0==file_initialized) initialize_file(__FILE__); /* BL */
        if (!size) return;

        head = base;
        high = head + size - width;

        /* Precompute Leonardo numbers, scaled by element width */
        for(lp[0]=lp[1]=width, i=2; (lp[i]=lp[i-2]+lp[i-1]+width) < size; i++);

        while(head < high) {
                if((p[0] & 3) == 3) {
                        sift(head, width, cmp, pshift, lp);
                        shr(p, 2);
                        pshift += 2;
                } else {
                        if(lp[pshift - 1] >= high - head) {
                                trinkle(head, width, cmp, p, pshift, 0, lp);
                        } else {
                                sift(head, width, cmp, pshift, lp);
                        }

                        if(pshift == 1) {
                                shl(p, 1);
                                pshift = 0;
                        } else {
                                shl(p, pshift - 1);
                                pshift = 1;
                        }
                }

                p[0] |= 1;
                head += width;
        }

        trinkle(head, width, cmp, p, pshift, 0, lp);

        while(pshift != 1 || p[0] != 1 || p[1] != 0) {
                if(pshift <= 1) {
                        trail = pntz(p);
                        shr(p, trail);
                        pshift += trail;
                } else {
                        shl(p, 2);
                        pshift -= 2;
                        p[0] ^= 7;
                        shr(p, 1);
                        trinkle(head - lp[pshift] - width, width, cmp, p, pshift + 1, 1, lp);
                        shl(p, 1);
                        p[0] |= 1;
                        trinkle(head - width, width, cmp, p, pshift, 1, lp);
                }
                head -= width;
        }
}
/* ************************************************************** */
