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
* $Id: ~|^` @(#)    sequences.c copyright 2016-2017 Bruce Lilly.   \ sequences.c $
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
/* $Id: ~|^` @(#)   This is sequences.c version 1.4 dated 2017-11-04T00:48:50Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.sequences.c */

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
#define ID_STRING_PREFIX "$Id: sequences.c ~|^` @(#)"
#define SOURCE_MODULE "sequences.c"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2017-11-04T00:48:50Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* test pattern sequences */
/* reasonable modulus for sawtooth pattern for array of size n */
static size_t sawtooth_modulus(size_t n, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    size_t m, r;

    if (n < MAXIMUM_SAWTOOTH_MODULUS * 2UL) {
        if (n < 5UL) {
            m =  2UL;
        } else {
            m = n >> 1; /* n >> 1 is a fast version of n / 2 */
            if (0 != is_even(m))
                m++;
        }
    } else {
        m = MAXIMUM_SAWTOOTH_MODULUS;
    }
    for (r=n%m; 5UL<=m; m-=2UL,r=n%m) {
        if (0 != is_even(r))
            break;
    }
    if (NULL != f)
        f(LOG_INFO, log_arg, "%s: %s line %d: %s(%lu,...) returns %lu, r=%lu", __func__, source_file, __LINE__, __func__, n, m, r);
    return m;
}

/* replace elements of p between l and r inclusive with element value modulo modulus */
static void modulo(long *p, size_t l, size_t r, long modulus)
{
    for (;l<=r; l++)
        p[l] %= modulus;
}

/* reverse the order of elements in p between l and r inclusive */
static void reverse_long(long *p, size_t l, size_t r)
{
    long t;

    for (;l<r; l++,r--) {
        t=p[l]; p[l]=p[r]; p[r]=t;
    }
}

/* fill long array p between l and r inclusive with integers beginning with n */
static void increment(long *p, size_t l, size_t r, long n, long incr)
{
    for (;l<=r; l++)
        p[l]=n, n+=incr;
}

/* fill long array p between l and r inclusive with constant value n */
static void constant_fill(long *p, size_t l, size_t r, long n)
{
    for (;l<=r; l++)
        p[l] = n;
}

/* fill long array p with random values in [0,modulus) */
static void random_fill(long *p, size_t l, size_t r, uint64_t modulus)
{
    for (;l<=r; l++)
        p[l] = (long)random64n(modulus);
}

/* add random values in [0,modulus) to values in long array p */
static void random_add(long *p, size_t l, size_t r, uint64_t modulus)
{
    for (;l<=r; l++)
        p[l] += (long)random64n(modulus);
}

static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void jumble(char *base, size_t first, size_t last, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    size_t nmemb=last+1UL-first, oldneq, oldnlt, oldngt, oldnsw;
    char *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11;

    oldnlt=nlt, oldneq=neq, oldngt=ngt, oldnsw=nsw;

#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "// %s line %d: first=%lu, last=%lu, nmemb=%lu, size=%lu, nsw=%lu\n",__func__,__LINE__,first,last,nmemb,size,nsw);
#endif
    p0=base+first*size;
    p1=p0+size;
    switch (nmemb) {
        case 1UL :
        break;
        case 2UL : /* 1 comparison */
            REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
        break;
        case 3UL : /* 3 comparisons */
            p2=p1+size;
            REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio); /* 3 */
            REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio); /* 2 */
            REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio); /* 1 */
        break;
        case 4UL : /* 5 comparisons */
            p2=p1+size;
            p3=p2+size;
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
        break;
        case 5UL : /* 9 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio);
        break;
        case 6UL : /* 12 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p5,cx,size,swapf,alignsize,size_ratio);
        break;
        case 7UL : /* 16 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            p6=p5+size;
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p6,cx,size,swapf,alignsize,size_ratio);
        /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p6,cx,size,swapf,alignsize,size_ratio);
        break;
        case 8UL : /* 19 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            p6=p5+size;
            p7=p6+size;
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p6,p7,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p6,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p7,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p6,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p7,cx,size,swapf,alignsize,size_ratio);
        break;
        case 9UL : /* 25 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            p6=p5+size;
            p7=p6+size;
            p8=p7+size;
            /* parallel group 9 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 8 */
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 7 */
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p0,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p7,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p3,p6,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p7,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p0,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p8,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p6,p7,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p7,p8,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p6,p7,cx,size,swapf,alignsize,size_ratio);
        break;
        case 10UL : /* 29 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            p6=p5+size;
            p7=p6+size;
            p8=p7+size;
            p9=p8+size;
            /* parallel group 9 */
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 8 */
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 7 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p6,p7,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p7,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p6,p8,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p6,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p7,p8,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p7,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p8,p9,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p6,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p7,p9,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p3,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p5,p8,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p6,p9,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p5,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p1,p6,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p2,p7,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p3,p8,cx,size,swapf,alignsize,size_ratio);
                REVERSE_COMPARE_EXCHANGE(p4,p9,cx,size,swapf,alignsize,size_ratio);
        break;
        case 11UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size;
            /* parallel group 9 */
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio); /* 3,4 */
                REVERSE_COMPARE_EXCHANGE(p7,p8,cx,size,swapf,alignsize,size_ratio); /* 7,8 */
            /* parallel group 8 */
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio); /* 2,4 */
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
                REVERSE_COMPARE_EXCHANGE(p7,p9,cx,size,swapf,alignsize,size_ratio); /* 7,9 */
            /* parallel group 7 */
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio); /* 1,4 */
                REVERSE_COMPARE_EXCHANGE(p3,p5,cx,size,swapf,alignsize,size_ratio); /* 3,5 */
                REVERSE_COMPARE_EXCHANGE(p6,p8,cx,size,swapf,alignsize,size_ratio); /* 6,8 */
                REVERSE_COMPARE_EXCHANGE(p7,p10,cx,size,swapf,alignsize,size_ratio); /* 7,10 */
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p1,p5,cx,size,swapf,alignsize,size_ratio); /* 1,5 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                REVERSE_COMPARE_EXCHANGE(p6,p10,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
                REVERSE_COMPARE_EXCHANGE(p8,p9,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio); /* 0,4 */
                REVERSE_COMPARE_EXCHANGE(p2,p6,cx,size,swapf,alignsize,size_ratio); /* 2,6 */
                REVERSE_COMPARE_EXCHANGE(p3,p8,cx,size,swapf,alignsize,size_ratio); /* 3,8 */
                REVERSE_COMPARE_EXCHANGE(p5,p9,cx,size,swapf,alignsize,size_ratio); /* 5,9 */
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio); /* 1,4 */
                REVERSE_COMPARE_EXCHANGE(p3,p8,cx,size,swapf,alignsize,size_ratio); /* 3,8 */
                REVERSE_COMPARE_EXCHANGE(p6,p10,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio); /* 0,4 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio); /* 1,2 */
                REVERSE_COMPARE_EXCHANGE(p3,p7,cx,size,swapf,alignsize,size_ratio); /* 3,7 */
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
                REVERSE_COMPARE_EXCHANGE(p9,p10,cx,size,swapf,alignsize,size_ratio); /* 9,10 */
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio); /* 0,2 */
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio); /* 1,3 */
                REVERSE_COMPARE_EXCHANGE(p4,p6,cx,size,swapf,alignsize,size_ratio); /* 4,6 */
                REVERSE_COMPARE_EXCHANGE(p5,p7,cx,size,swapf,alignsize,size_ratio); /* 5,7 */
                REVERSE_COMPARE_EXCHANGE(p8,p10,cx,size,swapf,alignsize,size_ratio); /* 8,10 */
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio); /* 0,1 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio); /* 4,5 */
                REVERSE_COMPARE_EXCHANGE(p6,p7,cx,size,swapf,alignsize,size_ratio); /* 6,7 */
                REVERSE_COMPARE_EXCHANGE(p8,p9,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
        break;
        case 12UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size; p11=p10+size;
            /* parallel group 9 */
                REVERSE_COMPARE_EXCHANGE(p3,p4,cx,size,swapf,alignsize,size_ratio); /* 3,4 */
                REVERSE_COMPARE_EXCHANGE(p7,p8,cx,size,swapf,alignsize,size_ratio); /* 7,8 */
            /* parallel group 8 */
                REVERSE_COMPARE_EXCHANGE(p2,p4,cx,size,swapf,alignsize,size_ratio); /* 2,4 */
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
                REVERSE_COMPARE_EXCHANGE(p7,p9,cx,size,swapf,alignsize,size_ratio); /* 7,9 */
            /* parallel group 7 */
                REVERSE_COMPARE_EXCHANGE(p1,p4,cx,size,swapf,alignsize,size_ratio); /* 1,4 */
                REVERSE_COMPARE_EXCHANGE(p3,p5,cx,size,swapf,alignsize,size_ratio); /* 3,5 */
                REVERSE_COMPARE_EXCHANGE(p6,p8,cx,size,swapf,alignsize,size_ratio); /* 6,8 */
                REVERSE_COMPARE_EXCHANGE(p7,p10,cx,size,swapf,alignsize,size_ratio); /* 7,10 */
            /* parallel group 6 */
                REVERSE_COMPARE_EXCHANGE(p1,p5,cx,size,swapf,alignsize,size_ratio); /* 1,5 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                REVERSE_COMPARE_EXCHANGE(p6,p10,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
                REVERSE_COMPARE_EXCHANGE(p8,p9,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
            /* parallel group 5 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio); /* 0,4 */
                REVERSE_COMPARE_EXCHANGE(p2,p6,cx,size,swapf,alignsize,size_ratio); /* 2,6 */
                REVERSE_COMPARE_EXCHANGE(p3,p8,cx,size,swapf,alignsize,size_ratio); /* 3,8 */
                REVERSE_COMPARE_EXCHANGE(p5,p9,cx,size,swapf,alignsize,size_ratio); /* 5,9 */
                REVERSE_COMPARE_EXCHANGE(p7,p11,cx,size,swapf,alignsize,size_ratio); /* 7,11 */
            /* parallel group 4 */
                REVERSE_COMPARE_EXCHANGE(p1,p5,cx,size,swapf,alignsize,size_ratio); /* 1,5 */
                REVERSE_COMPARE_EXCHANGE(p3,p7,cx,size,swapf,alignsize,size_ratio); /* 3,7 */
                REVERSE_COMPARE_EXCHANGE(p4,p8,cx,size,swapf,alignsize,size_ratio); /* 4,8 */
                REVERSE_COMPARE_EXCHANGE(p6,p10,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
            /* parallel group 3 */
                REVERSE_COMPARE_EXCHANGE(p0,p4,cx,size,swapf,alignsize,size_ratio); /* 0,4 */
                REVERSE_COMPARE_EXCHANGE(p1,p2,cx,size,swapf,alignsize,size_ratio); /* 1,2 */
                REVERSE_COMPARE_EXCHANGE(p5,p6,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
                REVERSE_COMPARE_EXCHANGE(p7,p11,cx,size,swapf,alignsize,size_ratio); /* 7,11 */
                REVERSE_COMPARE_EXCHANGE(p9,p10,cx,size,swapf,alignsize,size_ratio); /* 9,10 */
            /* parallel group 2 */
                REVERSE_COMPARE_EXCHANGE(p0,p2,cx,size,swapf,alignsize,size_ratio); /* 0,2 */
                REVERSE_COMPARE_EXCHANGE(p1,p3,cx,size,swapf,alignsize,size_ratio); /* 1,3 */
                REVERSE_COMPARE_EXCHANGE(p4,p6,cx,size,swapf,alignsize,size_ratio); /* 4,6 */
                REVERSE_COMPARE_EXCHANGE(p5,p7,cx,size,swapf,alignsize,size_ratio); /* 5,7 */
                REVERSE_COMPARE_EXCHANGE(p8,p10,cx,size,swapf,alignsize,size_ratio); /* 8,10 */
                REVERSE_COMPARE_EXCHANGE(p9,p11,cx,size,swapf,alignsize,size_ratio); /* 9,11 */
            /* parallel group 1 */
                REVERSE_COMPARE_EXCHANGE(p0,p1,cx,size,swapf,alignsize,size_ratio); /* 0,1 */
                REVERSE_COMPARE_EXCHANGE(p2,p3,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                REVERSE_COMPARE_EXCHANGE(p4,p5,cx,size,swapf,alignsize,size_ratio); /* 4,5 */
                REVERSE_COMPARE_EXCHANGE(p6,p7,cx,size,swapf,alignsize,size_ratio); /* 6,7 */
                REVERSE_COMPARE_EXCHANGE(p8,p9,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
                REVERSE_COMPARE_EXCHANGE(p10,p11,cx,size,swapf,alignsize,size_ratio); /* 10,11 */
        break;
        default :
            /* Fallback in case size is out-of-bounds. */
            reverse_long((long *)base, first, last); /* worst-case for insertion sort */
        break;
    }
    nlt=oldnlt, neq=oldneq, ngt=oldngt, nsw=oldnsw;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "// %s line %d: first=%lu, last=%lu, nmemb=%lu, size=%lu, nsw=%lu\n",__func__,__LINE__,first,last,nmemb,size,nsw);
#endif
}

static unsigned long ulceil(unsigned long numerator, unsigned long denominator)
{
    unsigned long r=numerator/denominator;
    if (r*denominator<numerator) r++;
    return r;
}

void antiquickselect(long *base, size_t first, size_t beyond, size_t size,
    int (*compar)(const void *,const void *),
    void (*swapf)(void *, void *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, struct sampling_table_struct *psts, unsigned int table_index,
    int c, unsigned int options)
{
    size_t nmemb, rank, o, p1, p2, p3, s, samples;
    auto unsigned char f1, f2;
    long *pm, *pp;
        
    nmemb=beyond-first;
    if (nmemb<=cutoff) {
        increment(base,first,beyond-1UL,first,1UL);
        jumble(base,first,beyond-1UL,size,compar,swapf,alignsize,size_ratio);
        return;
    }
    A(table_index < (SAMPLING_TABLE_SIZE));
    table_index=d_sample_index(psts,table_index,nmemb);
    samples=psts[table_index].samples;
    repivot_factors(table_index,NULL,&f1,&f2);
    if (0==c) {
        /* s is small region size; [beyond-s,beyond) */
        if (nmemb>(size_t)f1) {
            s=(size_t)ulceil(nmemb-f1,f1+1UL);
        } else {
            s=samples/3UL;
        }
    } else {
        /* s is small region size; [beyond-s,beyond) */
        if (nmemb>(size_t)f2) {
            s=(size_t)ulceil(nmemb-f2,f2+1UL);
        } else {
            s=samples/3UL;
        }
    }
    rank=beyond-s-1UL;
    p2=first+(nmemb>>1);
    o=nmemb/3UL;
    p1=p2-o;
    p3=p2+o;
if (DEBUGGING(SORT_SELECT_DEBUG))
fprintf(stderr,"/* %s: %s line %d: first=%lu, beyond=%lu, nmemb=%lu, pivot rank %lu, small region %lu, large region %lu, primary pivot selection ranks %lu,%lu,%lu, ratio %lu, f1=%u, f2=%u, c=%d */\n",__func__,source_file,__LINE__,first,beyond,nmemb,rank,s,nmemb-s-1UL,p1,p2,p3,(nmemb-s-1UL)/(s+1UL),f1,f2,c);
    base[rank]=(long)rank;
    /* recurse on small region */
    antiquickselect(base,beyond-s,beyond,size,compar,swapf,alignsize,
        size_ratio,cutoff,psts,table_index,0,options);
    /* recurse on large region */
    if ((nmemb-s-1UL)/(s+1UL)>=(size_t)f2) c++;
    antiquickselect(base,first,rank,size,compar,swapf,alignsize,
        size_ratio,cutoff,psts,table_index,c,options);
    /* departition */
    find_minmax((char *)base,first,beyond,size,compar,(char *)(&pm),
        (char *)(&pp));
    if (pm!=&(base[first]))
        EXCHANGE_SWAP(swapf,&(base[first]),pm,size,alignsize,size_ratio,
            /**/);
    EXCHANGE_SWAP(swapf,&(base[first+1UL]),&(base[rank]),size,alignsize,size_ratio,
        /**/);
    if (pp!=&(base[first+2UL]))
        EXCHANGE_SWAP(swapf,&(base[first+2UL]),pp,size,alignsize,size_ratio,
            /**/);
#if 1
    reverse_long(base,first+3UL,beyond-1UL);
#endif
    EXCHANGE_SWAP(swapf,&(base[first]),&(base[p3]),size,alignsize,size_ratio,
        /**/);
    EXCHANGE_SWAP(swapf,&(base[first+1UL]),&(base[p2]),size,alignsize,
        size_ratio,/**/);
    EXCHANGE_SWAP(swapf,&(base[first+2UL]),&(base[p1]),size,alignsize,
        size_ratio,/**/);
if (DEBUGGING(SORT_SELECT_DEBUG))
print_long_array(base,first,beyond-1UL,"/* "," */");
}

/* permute array p by swapping one pair of elements in range l..r */
/* Based on Algorithm 4a (Alternate Ives) in Sedgewick "Permutation Generation Methods" */
/* caller provides pointers to array c and variable t, both type size_t */
void permute(long *p, size_t l, size_t r, size_t *c, size_t *pt)
{
    size_t t=*pt;
    A(r>=t);
    for (;;) {
        if (c[t] < r - t) {
            if (0 == is_even(t)) { /* t is odd */
                A(c[t]<r);
                exchange((char *)p, c[t], c[t]+1UL, 1UL, sizeof(long));
            } else { /* t is even */
                exchange((char *)p, c[t], r-t, 1UL, sizeof(long));
            }
            c[t]++;
            *pt = l;
            return;
        } else {
            c[t] = l;
            *pt = ++t;
        }
    }
}

/* Generate specified test sequence in long array at p, index 0UL through n-1UL
      with maximum value limited to max_val.
*/
int generate_long_test_array(long *p, size_t n, unsigned int testnum, long incr,
    uint64_t max_val, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret = -1;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (NULL != p) {
        size_t h, j, k, u, x, y, z;
        long i;
        double d;
        size_t alignsize=alignment_size((char *)p,sizeof(long));
        size_t size_ratio=sizeof(long)/alignsize;
        void (*swapf)(char *, char *, size_t);
        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);

        u = n - 1UL;
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :
                for(j=0UL; j<=u; j++)
                    p[j] = input_data[j];
            break;
            case TEST_SEQUENCE_SORTED :
                increment(p,0UL,u,0L,incr);
            break;
            case TEST_SEQUENCE_REVERSE :
                increment(p,0UL,u,0L,incr);
                reverse_long(p,0UL,u);
            break;
            case TEST_SEQUENCE_ROTATED :
                if (0UL<u) increment(p,1UL,u,0L,incr);
                p[0]=(long)u;
            break;
            case TEST_SEQUENCE_SHIFTED :
                if (0UL<u) increment(p,0UL,u-1UL,1L,incr);
                p[u]=0L;
            break;
            case TEST_SEQUENCE_ORGAN_PIPE :
                increment(p,0UL,n>>1,0L,incr);
                increment(p,(n+1UL)>>1,u,0L,incr);
                reverse_long(p,(n+1UL)>>1,u);
            break;
            case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
                increment(p,n>>1,u,0L,incr);
                increment(p,0UL,(n-1UL)>>1,0L,incr);
                reverse_long(p,0UL,(n-1UL)>>1);
            break;
            case TEST_SEQUENCE_SAWTOOTH :
                increment(p,0UL,u,0L,incr);
                k = sawtooth_modulus(j,NULL,log_arg);
                modulo(p,0UL,u,k);
            break;
            case TEST_SEQUENCE_TERNARY :
                random_fill(p,0UL,u,3);
            break;
            case TEST_SEQUENCE_BINARY :
                random_fill(p,0UL,u,2);
            break;
            case TEST_SEQUENCE_CONSTANT :
                constant_fill(p,0UL,u,CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MEDIAN3KILLER :
                /* start with increasing sorted sequence */
                increment(p,0UL,u,0L,incr);
                k = n;
                /* median-of-3-killer sequence */
                if (1UL < k) {
                    if (0 == is_even(k))
                        k--;
                    /* k is the greatest even number <= n */
                    for(h = n-2UL; 1; h -= 2UL) {
                        j = ((k - h) >> 1); /* >> 1 is a fast version of / 2 */
                        exchange((char *)p, h+1UL, j, 1UL, sizeof(long));
                        if (2UL > h)  /* don't allow h to iterate below 2 */
                            break;
                    }
                }
            break;
            case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                /* start with increasing sorted sequence */
                increment(p,0UL,u,0L,incr);
                /* swap extreme elements to 1/3 and 2/3 positions */
                for (x=((n-4UL)>>1); ; x--) { /*x=1st index @ yqsort*/
                    A(n>(x<<1));
                    h=n-(x<<1); /* elements when yqsort sees subarray */
                    A(2UL<=h); A(h<=n);
                    j = (h-2UL)/3UL; /* yqsort pivot offset from x */
                    if (0UL==j) j++;
                    k = x+j; /* swap x,k */
                    A(k<u); A(k>x);
                    A(u==n-1UL);
                    z = u-x; /* index of last element seen by yqsort */
                    A(z<=u);
                    y = z-j; /* swap y,z */
                    A(y<z); A(y>k);
                    exchange((char *)p, x, k, 1UL, sizeof(long));
                    exchange((char *)p, y, z, 1UL, sizeof(long));
                    if (0UL==x) break;
                }
            break;
            case TEST_SEQUENCE_JUMBLE :
                /* start with increasing sorted sequence */
                A(0UL<=u);
                increment(p,0UL,u,0L,incr);
                jumble((char *)p,0UL,u,sizeof(long),longcmp,swapf,alignsize,size_ratio);
            break;
            case TEST_SEQUENCE_RANDOM_DISTINCT :
                increment(p,0UL,u,0L,incr);
                (V)fisher_yates_shuffle(p, n, sizeof(long), random64n, f, log_arg);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
                random_fill(p,0UL,u,max_val);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                random_fill(p,0UL,u,n);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
                h=snlround(sqrt((double)n),NULL,NULL);
                random_fill(p,0UL,u,h);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
                constant_fill(p,0UL,u,0L);
                /* Ref: R.W.Hamming, "Numerical Methods for Scientists and Engineers",
                   2nd Ed., Sect. 8.6, ISBN 978-0-486-65241-2
                */
                for (h=0UL,k=max_val/12UL; h<12UL; h++)
                    random_add(p,0UL,u,k);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL :
                /* Ref: R.W.Hamming, "Numerical Methods for Scientists and Engineers",
                   2nd Ed., Sect. 8.7, ISBN 978-0-486-65241-2
                */
                for(j=0UL; j<=u; j++) {
                    /* non-zero random number */
                    do {
                        x = (size_t)random64n(max_val);
                    } while (0UL==x);
                    /* convert to floating point, shift floating point (decimal)
                       digits to mantissa and shift out leading zeros
                    */
                    for (d=(double)x; d>=1.0; d/=10.0) ;
                    for (; d<0.1; d*=10.0) ;
                    if (j>0UL) {
                        /* multiply by previously generated number and make
                           mantissa
                        */
                        d*=(double)(p[j-1UL]);
                        for (; d>=1.0; d/=10.0) ;
                        for (; d<0.1; d*=10.0) ;
                    }
                    /* d is now in [0.1,1.0) */
                    /* Scale to a fraction of max_val (scaling uniformly doesn't
                       affect ordering for sorting, nor does it affect the
                       reciprocal nature of the distribution).  Range of long
                       numbers is now [int(0.1*max_val),max_val).
                    */
                    p[j]=(long)((double)max_val*d);
                }
            break;
            case TEST_SEQUENCE_HISTOGRAM :
                constant_fill(p,0UL,u,0L);
                if (max_val / n < n) x=n*n; else x=max_val/n;
                if (10000000UL<x) x=10000000UL; /* time limit */
                z=12UL*u+1UL;
                for (j=0UL; j<x; j++) {
                    for (i=0L,h=0UL; h<12UL; h++) {
                        i += (long)random64n(n);
                    }
                    p[(n*i)/z]++;
                }
            break;
            case TEST_SEQUENCE_MANY_EQUAL_LEFT :
                random_fill(p,0UL,u,n);
                constant_fill(p,0UL,n>>2,CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
                random_fill(p,0UL,u,n);
                constant_fill(p,(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
                random_fill(p,0UL,u,n);
                constant_fill(p,(n>>1)+(n>>2),u,CONSTANT_VALUE);
            break;
            case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                random_fill(p,0UL,u,n);
                constant_fill(p,(n>>1)-(n>>3),(n>>1)+(n>>3),CONSTANT_VALUE);
                (V)fisher_yates_shuffle(p, n, sizeof(long), random64n, f, log_arg);
            break;
            case TEST_SEQUENCE_ANTIQUICKSELECT :
                h=cutoff_value(size_ratio,0x07F8U); /* XXX antiquickselect will only work for same size_ratio==1 */
                antiquickselect(p,0UL,n,sizeof(long),longcmp,swapf,alignsize,
                    size_ratio,h,sorting_sampling_table,2UL,0,0x07F8U);
            break;
            case TEST_SEQUENCE_PERMUTATIONS :
                /* initialized and updated in test functions */
            break;
            case TEST_SEQUENCE_COMBINATIONS :
                /* initialized and updated in test functions */
            break;
            case TEST_SEQUENCE_ADVERSARY :
                /* handled by initialize_antiqsort */
            break;
            default:
                (V)fprintf(stderr, "// %s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
                ret = 7;
            break;
        }
        if (0 > ret) ret = 0;
    }
    return ret;
}

const char *sequence_name(unsigned int testnum)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (testnum) {
        case TEST_SEQUENCE_STDIN :
        return "input sequence";
        case TEST_SEQUENCE_SORTED :
        return "increasing (sorted) sequence";
        case TEST_SEQUENCE_REVERSE :
        return "decreasing (reversed) sequence";
        case TEST_SEQUENCE_ROTATED :
        return "rotated sequence";
        case TEST_SEQUENCE_SHIFTED :
        return "shifted sequence";
        case TEST_SEQUENCE_ORGAN_PIPE :
        return "bitonic organ pipe sequence";
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
        return "inverted organ pipe sequence";
        case TEST_SEQUENCE_SAWTOOTH :
        return "sawtooth sequence";
        case TEST_SEQUENCE_TERNARY :
        return "random ternary values";
        case TEST_SEQUENCE_BINARY :
        return "random zeros and ones (binary)";
        case TEST_SEQUENCE_CONSTANT :
        return "equal (constant) values";
        case TEST_SEQUENCE_MEDIAN3KILLER :
        return "median-of-3 killer";
        case TEST_SEQUENCE_RANDOM_DISTINCT :
        return "randomly ordered distinct values";
        case TEST_SEQUENCE_RANDOM_VALUES :
        return "random values (full range)";
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
        return "random values (limited range)";
        case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
        return "random values (restricted range)";
        case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
        return "random values (approximately normal distribution)";
        case TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL :
        return "random values (reciprocal distribution)";
        case TEST_SEQUENCE_HISTOGRAM :
        return "histogram data";
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :
        return "many equal (left)";
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
        return "many equal (middle)";
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
        return "many equal (right)";
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
        return "many equal (shuffled)";
        case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
        return "dual-pivot killer";
        case TEST_SEQUENCE_JUMBLE :
        return "jumbled";
        case TEST_SEQUENCE_PERMUTATIONS :
        return "distinct permutations";
        case TEST_SEQUENCE_COMBINATIONS :
        return "0/1 combinations";
        case TEST_SEQUENCE_ADVERSARY :
        return "McIlroy adversary";
        case TEST_SEQUENCE_ANTIQUICKSELECT :
        return "antiquickselect";
        default:
            (V)fprintf(stderr, "// %s: %s line %d: unrecognized testnum %u\n", __func__, source_file, __LINE__, testnum);
        break;
    }
    return "invalid sequence";
}
