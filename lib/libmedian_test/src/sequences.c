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
* $Id: ~|^` @(#)    sequences.c copyright 2016-2020 Bruce Lilly.   \ sequences.c $
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
/* $Id: ~|^` @(#)   This is sequences.c version 1.15 dated 2020-02-03T20:36:29Z. \ $ */
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
#define MODULE_VERSION "1.15"
#define MODULE_DATE "2020-02-03T20:36:29Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2020"

#define __STDC_WANT_LIB_EXT1__ 0
#define LIBMEDIAN_TEST_CODE 1

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
QUICKSELECT_INLINE
void jumble(char *base, size_t first, size_t last, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    size_t nmemb=last+1UL-first, oldneq, oldnlt, oldngt, oldnsw;
    char *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12,
        *p13, *p14, *p15;

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
#define RCX(ma,mb) REVERSE_COMPARE_EXCHANGE((ma),(mb),options,size,swapf,alignsize,size_ratio)
            RCX(p0,p1);
        break;
        case 3UL : /* 3 comparisons */
            p2=p1+size;
            RCX(p1,p2); /* 3 */
            RCX(p0,p1); /* 2 */
            RCX(p0,p2); /* 1 */
        break;
        case 4UL : /* 5 comparisons */
            p2=p1+size;
            p3=p2+size;
            /* parallel group 3 */
                RCX(p1,p2);
            /* parallel group 2 */
                RCX(p0,p1);
                RCX(p2,p3);
            /* parallel group 1 */
                RCX(p0,p2);
                RCX(p1,p3);
        break;
        case 5UL : /* 9 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            /* parallel group 5 */
                RCX(p1,p2);
                RCX(p3,p4);
            /* parallel group 4 */
                RCX(p2,p3);
                RCX(p1,p4);
            /* parallel group 3 */
                RCX(p0,p1);
                RCX(p2,p4);
            /* parallel group 2 */
                RCX(p0,p2);
                RCX(p1,p3);
            /* parallel group 1 */
                RCX(p0,p4);
        break;
        case 6UL : /* 12 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            /* parallel group 6 */
                RCX(p1,p2);
                RCX(p3,p4);
            /* parallel group 5 */
                RCX(p1,p4);
            /* parallel group 4 */
                RCX(p2,p3);
                RCX(p4,p5);
            /* parallel group 3 */
                RCX(p0,p1);
                RCX(p2,p4);
                RCX(p3,p5);
            /* parallel group 2 */
                RCX(p0,p2);
                RCX(p1,p3);
            /* parallel group 1 */
                RCX(p0,p4);
                RCX(p1,p5);
        break;
        case 7UL : /* 16 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            p6=p5+size;
            /* parallel group 6 */
                RCX(p1,p2);
                RCX(p3,p4);
                RCX(p5,p6);
            /* parallel group 5 */
                RCX(p1,p4);
                RCX(p3,p6);
            /* parallel group 4 */
                RCX(p2,p3);
                RCX(p4,p5);
            /* parallel group 3 */
                RCX(p0,p1);
                RCX(p2,p4);
                RCX(p3,p5);
            /* parallel group 2 */
                RCX(p0,p2);
                RCX(p1,p3);
                RCX(p4,p6);
        /* parallel group 1 */
                RCX(p0,p4);
                RCX(p1,p5);
                RCX(p2,p6);
        break;
        case 8UL : /* 19 comparisons */
            p2=p1+size;
            p3=p2+size;
            p4=p3+size;
            p5=p4+size;
            p6=p5+size;
            p7=p6+size;
            /* parallel group 6 */
                RCX(p1,p2);
                RCX(p3,p4);
                RCX(p5,p6);
            /* parallel group 5 */
                RCX(p1,p4);
                RCX(p3,p6);
            /* parallel group 4 */
                RCX(p2,p3);
                RCX(p4,p5);
            /* parallel group 3 */
                RCX(p0,p1);
                RCX(p2,p4);
                RCX(p3,p5);
                RCX(p6,p7);
            /* parallel group 2 */
                RCX(p0,p2);
                RCX(p1,p3);
                RCX(p4,p6);
                RCX(p5,p7);
            /* parallel group 1 */
                RCX(p0,p4);
                RCX(p1,p5);
                RCX(p2,p6);
                RCX(p3,p7);
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
                RCX(p2,p3);
            /* parallel group 8 */
                RCX(p2,p4);
                RCX(p5,p6);
            /* parallel group 7 */
                RCX(p1,p3);
                RCX(p4,p6);
            /* parallel group 6 */
                RCX(p0,p3);
                RCX(p1,p4);
                RCX(p5,p7);
                RCX(p2,p6);
            /* parallel group 5 */
                RCX(p3,p6);
                RCX(p4,p7);
                RCX(p2,p5);
            /* parallel group 4 */
                RCX(p0,p3);
                RCX(p1,p4);
                RCX(p5,p8);
            /* parallel group 3 */
                RCX(p0,p1);
                RCX(p3,p4);
                RCX(p6,p7);
                RCX(p2,p5);
            /* parallel group 2 */
                RCX(p1,p2);
                RCX(p4,p5);
                RCX(p7,p8);
            /* parallel group 1 */
                RCX(p0,p1);
                RCX(p3,p4);
                RCX(p6,p7);
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
                RCX(p4,p5);
            /* parallel group 8 */
                RCX(p3,p4);
                RCX(p5,p6);
            /* parallel group 7 */
                RCX(p2,p3);
                RCX(p6,p7);
            /* parallel group 6 */
                RCX(p1,p3);
                RCX(p2,p5);
                RCX(p4,p7);
                RCX(p6,p8);
            /* parallel group 5 */
                RCX(p1,p2);
                RCX(p3,p5);
                RCX(p4,p6);
                RCX(p7,p8);
            /* parallel group 4 */
                RCX(p0,p1);
                RCX(p2,p4);
                RCX(p5,p7);
                RCX(p8,p9);
            /* parallel group 3 */
                RCX(p0,p2);
                RCX(p3,p6);
                RCX(p7,p9);
            /* parallel group 2 */
                RCX(p0,p3);
                RCX(p1,p4);
                RCX(p5,p8);
                RCX(p6,p9);
            /* parallel group 1 */
                RCX(p0,p5);
                RCX(p1,p6);
                RCX(p2,p7);
                RCX(p3,p8);
                RCX(p4,p9);
        break;
        case 11UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size;
            /* parallel group 9 */
                RCX(p3,p4); /* 3,4 */
                RCX(p7,p8); /* 7,8 */
            /* parallel group 8 */
                RCX(p2,p4); /* 2,4 */
                RCX(p5,p6); /* 5,6 */
                RCX(p7,p9); /* 7,9 */
            /* parallel group 7 */
                RCX(p1,p4); /* 1,4 */
                RCX(p3,p5); /* 3,5 */
                RCX(p6,p8); /* 6,8 */
                RCX(p7,p10); /* 7,10 */
            /* parallel group 6 */
                RCX(p1,p5); /* 1,5 */
                RCX(p2,p3); /* 2,3 */
                RCX(p6,p10); /* 6,10 */
                RCX(p8,p9); /* 8,9 */
            /* parallel group 5 */
                RCX(p0,p4); /* 0,4 */
                RCX(p2,p6); /* 2,6 */
                RCX(p3,p8); /* 3,8 */
                RCX(p5,p9); /* 5,9 */
            /* parallel group 4 */
                RCX(p1,p4); /* 1,4 */
                RCX(p3,p8); /* 3,8 */
                RCX(p6,p10); /* 6,10 */
            /* parallel group 3 */
                RCX(p0,p4); /* 0,4 */
                RCX(p1,p2); /* 1,2 */
                RCX(p3,p7); /* 3,7 */
                RCX(p5,p6); /* 5,6 */
                RCX(p9,p10); /* 9,10 */
            /* parallel group 2 */
                RCX(p0,p2); /* 0,2 */
                RCX(p1,p3); /* 1,3 */
                RCX(p4,p6); /* 4,6 */
                RCX(p5,p7); /* 5,7 */
                RCX(p8,p10); /* 8,10 */
            /* parallel group 1 */
                RCX(p0,p1); /* 0,1 */
                RCX(p2,p3); /* 2,3 */
                RCX(p4,p5); /* 4,5 */
                RCX(p6,p7); /* 6,7 */
                RCX(p8,p9); /* 8,9 */
        break;
        case 12UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size; p11=p10+size;
            /* parallel group 9 */
                RCX(p3,p4); /* 3,4 */
                RCX(p7,p8); /* 7,8 */
            /* parallel group 8 */
                RCX(p2,p4); /* 2,4 */
                RCX(p5,p6); /* 5,6 */
                RCX(p7,p9); /* 7,9 */
            /* parallel group 7 */
                RCX(p1,p4); /* 1,4 */
                RCX(p3,p5); /* 3,5 */
                RCX(p6,p8); /* 6,8 */
                RCX(p7,p10); /* 7,10 */
            /* parallel group 6 */
                RCX(p1,p5); /* 1,5 */
                RCX(p2,p3); /* 2,3 */
                RCX(p6,p10); /* 6,10 */
                RCX(p8,p9); /* 8,9 */
            /* parallel group 5 */
                RCX(p0,p4); /* 0,4 */
                RCX(p2,p6); /* 2,6 */
                RCX(p3,p8); /* 3,8 */
                RCX(p5,p9); /* 5,9 */
                RCX(p7,p11); /* 7,11 */
            /* parallel group 4 */
                RCX(p1,p5); /* 1,5 */
                RCX(p3,p7); /* 3,7 */
                RCX(p4,p8); /* 4,8 */
                RCX(p6,p10); /* 6,10 */
            /* parallel group 3 */
                RCX(p0,p4); /* 0,4 */
                RCX(p1,p2); /* 1,2 */
                RCX(p5,p6); /* 5,6 */
                RCX(p7,p11); /* 7,11 */
                RCX(p9,p10); /* 9,10 */
            /* parallel group 2 */
                RCX(p0,p2); /* 0,2 */
                RCX(p1,p3); /* 1,3 */
                RCX(p4,p6); /* 4,6 */
                RCX(p5,p7); /* 5,7 */
                RCX(p8,p10); /* 8,10 */
                RCX(p9,p11); /* 9,11 */
            /* parallel group 1 */
                RCX(p0,p1); /* 0,1 */
                RCX(p2,p3); /* 2,3 */
                RCX(p4,p5); /* 4,5 */
                RCX(p6,p7); /* 6,7 */
                RCX(p8,p9); /* 8,9 */
                RCX(p10,p11); /* 10,11 */
        break;
        case 13UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size; p11=p10+size;
            p12=p11+size;
            /* parallel group 10 */
                RCX(p3,p4); RCX(p5,p6);
            /* parallel group 9 */
                RCX(p2,p3); RCX(p4,p5); RCX(p6,p7); RCX(p8,p9);
            /* parallel group 8 */
                RCX(p2,p4); RCX(p5,p6); RCX(p7,p8); RCX(p1,p3);
            /* parallel group 7 */
                RCX(p5,p8); RCX(p1,p2); RCX(p3,p4); RCX(p6,p7); RCX(p9,p10);
            /* parallel group 6 */
                RCX(p0,p1); RCX(p3,p7); RCX(p8,p9); RCX(p10,p11);
            /* parallel group 5 */
                RCX(p1,p3); RCX(p4,p6); RCX(p9,p11); RCX(p2,p5); RCX(p8,p10);
            /* parallel group 4 */
                RCX(p3,p10); RCX(p11,p12); RCX(p6,p8); RCX(p2,p9);
            /* parallel group 3 */
                RCX(p5,p8); RCX(p10,p12); RCX(p0,p7); RCX(p3,p11); RCX(p4,p9);
            /* parallel group 2 */
                RCX(p1,p2); RCX(p7,p8); RCX(p9,p12); RCX(p0,p4); RCX(p6,p11);
                RCX(p5,p10);
            /* parallel group 1 */
                RCX(p1,p6); RCX(p8,p9); RCX(p2,p11); RCX(p0,p5); RCX(p7,p12);
                RCX(p4,p10);
        break;
        case 14UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size; p11=p10+size;
            p12=p11+size; p13=p12+size;
            /* parallel group 10 */
                RCX(p6,p7); RCX(p8,p9);
            /* parallel group 9 */
                RCX(p3,p4); RCX(p5,p6); RCX(p7,p8); RCX(p9,p10); RCX(p11,p12);
            /* parallel group 8 */
                RCX(p3,p5); RCX(p6,p8); RCX(p7,p9); RCX(p10,p12);
            /* parallel group 7 */
                RCX(p2,p4); RCX(p5,p6); RCX(p9,p10); RCX(p11,p13); RCX(p3,p8);
                RCX(p7,p12);
            /* parallel group 6 */
                RCX(p1,p4); RCX(p7,p13); RCX(p2,p8);
            /* parallel group 5 */
                RCX(p5,p10); RCX(p6,p9); RCX(p3,p12); RCX(p7,p11); RCX(p1,p2);
                RCX(p4,p8);
            /* parallel group 4 */
                RCX(p0,p8); RCX(p1,p9); RCX(p2,p10); RCX(p3,p11); RCX(p4,p12);
                RCX(p5,p13);
            /* parallel group 3 */
                RCX(p0,p4); RCX(p8,p12); RCX(p1,p5); RCX(p9,p13); RCX(p2,p6);
                RCX(p3,p7);
            /* parallel group 2 */
                RCX(p0,p2); RCX(p4,p6); RCX(p8,p10); RCX(p1,p3); RCX(p5,p7);
                RCX(p9,p11);
            /* parallel group 1 */
                RCX(p0,p1); RCX(p2,p3); RCX(p4,p5); RCX(p6,p7); RCX(p8,p9);
                RCX(p10,p11); RCX(p12,p13);
        break;
        case 15UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size; p11=p10+size;
            p12=p11+size; p13=p12+size; p14=p13+size;
            /* parallel group 10 */
                RCX(p6,p7); RCX(p8,p9);
            /* parallel group 9 */
                RCX(p3,p4); RCX(p5,p6); RCX(p7,p8); RCX(p9,p10); RCX(p11,p12);
            /* parallel group 8 */
                RCX(p3,p5); RCX(p6,p8); RCX(p7,p9); RCX(p10,p12);
            /* parallel group 7 */
                RCX(p2,p4); RCX(p5,p6); RCX(p9,p10); RCX(p11,p13); RCX(p3,p8);
                RCX(p7,p12);
            /* parallel group 6 */
                RCX(p1,p4); RCX(p7,p13); RCX(p2,p8); RCX(p11,p14);
            /* parallel group 5 */
                RCX(p5,p10); RCX(p6,p9); RCX(p3,p12); RCX(p13,p14); RCX(p7,p11);
                RCX(p1,p2); RCX(p4,p8);
            /* parallel group 4 */
                RCX(p0,p8); RCX(p1,p9); RCX(p2,p10); RCX(p3,p11); RCX(p4,p12);
                RCX(p5,p13); RCX(p6,p14);
            /* parallel group 3 */
                RCX(p0,p4); RCX(p8,p12); RCX(p1,p5); RCX(p9,p13); RCX(p2,p6);
                RCX(p10,p14); RCX(p3,p7);
            /* parallel group 2 */
                RCX(p0,p2); RCX(p4,p6); RCX(p8,p10); RCX(p12,p14); RCX(p1,p3);
                RCX(p5,p7); RCX(p9,p11);
            /* parallel group 1 */
                RCX(p0,p1); RCX(p2,p3); RCX(p4,p5); RCX(p6,p7); RCX(p8,p9);
                RCX(p10,p11); RCX(p12,p13);
        break;
        case 16UL :
            p2=p1+size; p3=p2+size; p4=p3+size; p5=p4+size; p6=p5+size;
            p7=p6+size; p8=p7+size; p9=p8+size; p10=p9+size; p11=p10+size;
            p12=p11+size; p13=p12+size; p14=p13+size; p15=p14+size;
            /* parallel group 10 */
                RCX(p6,p7); RCX(p8,p9);
            /* parallel group 9 */
                RCX(p3,p4); RCX(p5,p6); RCX(p7,p8); RCX(p9,p10); RCX(p11,p12);
            /* parallel group 8 */
                RCX(p3,p5); RCX(p6,p8); RCX(p7,p9); RCX(p10,p12);
            /* parallel group 7 */
                RCX(p2,p4); RCX(p5,p6); RCX(p9,p10); RCX(p11,p13); RCX(p3,p8);
                RCX(p7,p12);
            /* parallel group 6 */
                RCX(p1,p4); RCX(p7,p13); RCX(p2,p8); RCX(p11,p14);
            /* parallel group 5 */
                RCX(p5,p10); RCX(p6,p9); RCX(p3,p12); RCX(p13,p14); RCX(p7,p11);
                RCX(p1,p2); RCX(p4,p8);
            /* parallel group 4 */
                RCX(p0,p8); RCX(p1,p9); RCX(p2,p10); RCX(p3,p11); RCX(p4,p12);
                RCX(p5,p13); RCX(p6,p14); RCX(p7,p15);
            /* parallel group 3 */
                RCX(p0,p4); RCX(p8,p12); RCX(p1,p5); RCX(p9,p13); RCX(p2,p6);
                RCX(p10,p14); RCX(p3,p7); RCX(p11,p15);
            /* parallel group 2 */
                RCX(p0,p2); RCX(p4,p6); RCX(p8,p10); RCX(p12,p14); RCX(p1,p3);
                RCX(p5,p7); RCX(p9,p11); RCX(p13,p15);
            /* parallel group 1 */
                RCX(p0,p1); RCX(p2,p3); RCX(p4,p5); RCX(p6,p7); RCX(p8,p9);
                RCX(p10,p11); RCX(p12,p13); RCX(p14,p15);
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

#if 0
static unsigned long ulceil(unsigned long numerator, unsigned long denominator)
{
    unsigned long r=numerator/denominator;
    if (r*denominator<numerator) r++;
    return r;
}
#endif

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

/* output a histogram of some random (long integer) data distribution
*/
static void print_histogram(long *p, size_t n, void(*f)(int, void *, const char *, ...),
    void *log_arg)
{
    auto long *pmin, *pmax;
    long min_val, max_val;
    size_t b, *bins, nbins, range, w, x;

    d_find_minmax(p, 0UL, n, sizeof(long), longcmp, 0U, &pmin, &pmax);
    /* for constant data range is 1, binary data range is 2, etc. */
    min_val=*pmin, max_val=*pmax, range=1UL+(unsigned long)max_val-(unsigned long)min_val;
    nbins=ulsqrt(n);
    if (nbins>range) nbins=range; /* can't have more bins than range */
    if (100UL<nbins) nbins=100UL; /* upper limit on output */
    /* allocate and initialize bin counters */
    bins=calloc(nbins,sizeof(size_t));
    if (NULL==bins) {
        if (NULL != f)
            f(LOG_INFO, log_arg, "%s: %s line %d: callocs(%lu,%lu) returned NULL", __func__, source_file, __LINE__, nbins, sizeof(size_t));
        else
            (V)fprintf(stderr, "%s: %s line %d: callocs(%lu,%lu) returned NULL\n", __func__, source_file, __LINE__, nbins, sizeof(size_t));
    } else {
            w=range/nbins;
	    (V)fprintf(stderr, "data range %lu in %lu bins, bin width %lu\n", range, nbins, w);
            /* update counts */
            for (x=0UL; x<n; x++) {
                b=(p[x]-min_val)/w;
                bins[b]++;
            }
            /* output counts */
            for (b=0UL; b<nbins; b++) {
                (V)fprintf(stderr, "bin[%lu]: %lu\n", b, bins[b]);
            }
            /* free bin counters */
            free(bins);
    }
}

unsigned int sequence_is_randomized(unsigned int sequence)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (sequence) {
        case TEST_SEQUENCE_STDIN :
        return 0U;
        case TEST_SEQUENCE_SORTED :
        return 0U;
        case TEST_SEQUENCE_REVERSE :
        return 0U;
        case TEST_SEQUENCE_ROTATED :
        return 0U;
        case TEST_SEQUENCE_SHIFTED :
        return 0U;
        case TEST_SEQUENCE_ORGAN_PIPE :
        return 0U;
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
        return 0U;
        case TEST_SEQUENCE_SAWTOOTH :
        return 0U;
        case TEST_SEQUENCE_TERNARY :
        return 1U;
        case TEST_SEQUENCE_BINARY :
        return 1U;
        case TEST_SEQUENCE_CONSTANT :
        return 0U;
        case TEST_SEQUENCE_MEDIAN3KILLER :
        return 0U;
        case TEST_SEQUENCE_RANDOM_DISTINCT :
        return 1U;
        case TEST_SEQUENCE_RANDOM_VALUES :
        return 1U;
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
        return 1U;
        case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
        return 1U;
        case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
        return 1U;
        case TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL :
        return 1U;
        case TEST_SEQUENCE_HISTOGRAM :
        return 1U;
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :
        return 1U;
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :
        return 1U;
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :
        return 1U;
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
        return 1U;
        case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
        return 0U;
        case TEST_SEQUENCE_JUMBLE :
        return 0U;
        case TEST_SEQUENCE_PERMUTATIONS :
        return 0U;
        case TEST_SEQUENCE_COMBINATIONS :
        return 0U;
        case TEST_SEQUENCE_ADVERSARY :
        return 0U;
        case TEST_SEQUENCE_WORST :
        return 0U;
        default:
            (V)fprintf(stderr, "// %s: %s line %d: unrecognized sequence %u\n", __func__, source_file, __LINE__, sequence);
        break;
    }
    return 2U;
}

/* Generate specified test sequence in long array at p, index 0UL through n-1UL
      with maximum value limited to max_val.
*/
int generate_long_test_array(long *p, size_t n, unsigned int testnum, long incr,
    uint64_t max_val, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int ret = -1;
    FILE *fp=NULL;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (NULL != p) {
        char buf[32], buf2[32];
        int c;
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
                k = sawtooth_modulus(n,NULL,log_arg);
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
                    for(h = n-2UL; /*CONSTANTCONDITION*/ 1; h -= 2UL) {
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
                jumble((char *)p,0UL,u,sizeof(long),longcmp,swapf,alignsize,size_ratio,0U);
            break;
            case TEST_SEQUENCE_RANDOM_DISTINCT :
                increment(p,0UL,u,0L,incr);
                (V)fisher_yates_shuffle(p, n, sizeof(long), random64n, f, log_arg);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
                random_fill(p,0UL,u,max_val); /* full range (to max_val) */
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
                random_fill(p,0UL,u,n); /* values limited to n */
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :
                h=ulsqrt(n); /* values restricted to sqrt(n) */
                random_fill(p,0UL,u,h);
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :
                constant_fill(p,0UL,u,0L);
                /* Ref: R.W.Hamming, "Numerical Methods for Scientists and Engineers",
                   2nd Ed., Sect. 8.6, ISBN 978-0-486-65241-2
                */
                /* Each element is the sum of 12 random values. Each random
                   value is limited to max_val/12 to avoid overflow.
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
                /* Generate enough random values (within reasonable limits) to
                   produce a reasonably Gaussian histogram.
                */
                if (max_val / n < n) x=n*n; else x=max_val/n;
                if (10000000UL<x) x=10000000UL; /* time limit */
                z=12UL*u+1UL; /* > max. sum of 12 random numbers in [0,u] */
                if (NULL==fp) { /* for progress indication */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                    else fp=stderr;
                }
                c = snul(buf,sizeof(buf),NULL,NULL,x,10,'0',1,f,log_arg);
                for (j=0UL; j<x; j++) {
                    /* Generate normally-distributed random values (see
                       TEST_SEQUENCE_RANDOM_VALUES_NORMAL for description).
                       Assume that n < LONG_MAX / 12L...
                    */
                    for (i=0L,h=0UL; h<12UL; h++) {
                        i += (long)random64n(n);
                    }
                    /* Assign value to a bin, incrementing the count of values
                       in the bin.  32-bit integer arithmetic would limit usable
                       n to 160529, therefore double-precision floating-point
                       arithmetic is used for intermediate calculations to
                       assign the value to a bin.
                    */
                    d=(double)n*(double)i/(double)z; /* up to 12*n*n/(12*n)=n */
                    p[snlround(d,f,log_arg)]++;
                    if (NULL!=fp) {
                        (void)snul(buf2,sizeof(buf2),NULL,NULL,j,10,' ',c,f,log_arg);
                        (V)fprintf(fp,"\r%s/%s %3lu%%",buf2,buf,(j+1UL)*100UL/x);
                        fflush(fp);
                    }
                }
                if (fp==stderr) fp=NULL;
                if (NULL!=fp) { fflush(fp); fclose(fp); fp=NULL; }
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
#if DEBUG_CODE
        if (DEBUGGING(HISTOGRAM_DEBUG) && sequence_is_randomized(testnum)) {
            (V)fprintf(stderr, "histogram data for %s:\n", sequence_name(testnum));
            print_histogram(p, n, f, log_arg);
        }
#endif
        if (0 > ret) ret = 0;
    }
    return ret;
}

const char *sequence_name(unsigned int sequence)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (sequence) {
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
        case TEST_SEQUENCE_WORST :
        return "quickselect adversary";
        default:
            (V)fprintf(stderr,"/* %s: %s line %d: unrecognized sequence %u */\n",
                __func__, source_file, __LINE__, sequence);
        break;
    }
    return "invalid sequence";
}
