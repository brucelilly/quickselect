/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for random number functions
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    random64n.c copyright 2016-2017 Bruce Lilly.   \ random64n.c $
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
/* $Id: ~|^` @(#)   This is random64n.c version 1.6 dated 2017-09-01T02:24:35Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "random64n" */
/*****************************************************************************/
/* maintenance note: master file  /data/weather/lib/librandom/src/s.random64n.c */

/********************** Long description and rationale: ***********************
******************************************************************************/

/* source module identification */
/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: random64n.c ~|^` @(#)"
#define SOURCE_MODULE "random64n.c"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2017-09-01T02:24:35Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* feature test macros defined before any header files are included */
/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) && defined(MIN_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && defined(MAX_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files needed */
#include "random64n.h"          /* xorshift1024star random64n */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <sys/types.h>          /* *_t */
#ifndef EINVAL
# include <errno.h>             /* errno */
#endif
#include <string.h>             /* strrchr */
#include <time.h>               /* clock_gettime CLOCK_* struct timespec */

/* static data and function definitions */
static char random64n_initialized = (char)0;
static char random64n_seeded = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
static uint64_t s[16];
static unsigned int p;

static void random_seed_random64n(void)
{
    unsigned int u;
    uint64_t x, y;
    struct timespec tsm, tsr;

    /* random number state initialization */
    (void)clock_gettime(CLOCK_REALTIME, &tsr);
    p = ((unsigned int)(tsr.tv_sec) ^ (unsigned int)(tsr.tv_nsec)) % 16U;
    for (u=0U; 16U > u;) {
        (void)clock_gettime(CLOCK_MONOTONIC, &tsm);
        /* nanoseconds can only account for ~30 bits */
        x = ((((uint64_t)(tsm.tv_sec))<<30) + (uint64_t)(tsm.tv_nsec));
        (void)clock_gettime(CLOCK_REALTIME, &tsr);
        y = ((((uint64_t)(tsr.tv_sec))<<30) + (uint64_t)(tsr.tv_nsec));
        /* x and y are expected >30 bits; muliplication yields >60 bits */
        s[u] = x * y;
        if (UINT64_C(0) != s[u])
            u++; /* else try again */
    }
    random64n_seeded = (char)1;
}

static void initialize_random64n(void)
{
    const char *q;

    if ((char)0 == random64n_seeded)
        random_seed_random64n();
    q = strrchr(filenamebuf, '/');
    if (NULL == q)
        q = filenamebuf;
    random64n_initialized = register_build_strings(NULL, &source_file, q);
}

int seed_random64n(uint64_t *sarray, unsigned int pinit)
{
    unsigned int u;

    if (
       (NULL == sarray) 
    || (16U <= pinit) 
    ) { errno = EINVAL; random64n_seeded = (char)0; return -1; }
    for (u=0U; 16U > u; u++) { s[u] = sarray[u]; }
    p = pinit;
    random64n_seeded = (char)1;
    return 0;
}

int save_random64n_state(uint64_t *sarray, unsigned int *pp)
{
    unsigned int u;

    if (
       (NULL == sarray) 
    || (NULL == pp) 
    ) { errno = EINVAL; return -1; }
    if ((char)0 == random64n_seeded)
        random_seed_random64n();
    for (u=0U; 16U > u; u++) { sarray[u] = s[u]; }
    *pp = p;
    return 0;
}

/* non-linear transformed xorshift per Vigna.
   as retrieved from https://en.wikipedia.org/wiki/Xorshift#xorshift.2A 2016-03-10
   modified (bug removal, initialization).
   1024 bits of state, maximal period 2^1024 - 1
*/
uint64_t xorshift1024star(void) {
    uint64_t s0, s1;

    if ((char)0 == random64n_initialized)
        initialize_random64n();
    s0 = s[p];
    s1 = s[p = (p + 1U) & 15U];
    s1 ^= s1 << 31;
    s[p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
    return s[p] * UINT64_C(1181783497276652981);
}


/* return uniformly-distributed random number in range [0, n) as uint64_t
*/
uint64_t random64n(uint64_t n) {
    uint64_t r, x;

    if (UINT64_C(0) == n)
        return n;
    x = 0xffffffffffffffff % n;
    do {
        r = xorshift1024star();
    } while (r < x); /* avoid modulo bias */
    return (r % n);
}
