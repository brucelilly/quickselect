/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for main entry shuffle
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    shuffle.c copyright 2016-2017 Bruce Lilly.   \ shuffle.c $
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
/* $Id: ~|^` @(#)   This is shuffle.c version 1.7 dated 2017-02-12T15:20:35Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "shuffle" */
/*****************************************************************************/
/* maintenance note: master file  /data/weather/lib/libshuffle/src/s.shuffle.c */

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
#define ID_STRING_PREFIX "$Id: shuffle.c ~|^` @(#)"
#define SOURCE_MODULE "shuffle.c"
#define MODULE_VERSION "1.7"
#define MODULE_DATE "2017-02-12T15:20:35Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files needed */
#include "shuffle.h"            /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#ifndef EINVAL
# include <errno.h>             /* errno */
#endif
#ifndef EXIT_FAILURE /* need NULL, but that comes from stddef.h which may be included elsewhere */
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#ifndef LOG_DEBUG
# include <syslog.h>            /* LOG_* */
#endif

/* static data and function definitions */
static char shuffle_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* assume sizeof(foo) etc. are powers of 2 */
/* logarithms of two for index as sizeof */
/* table in lieu of calculation for speed */
/* valid up to sizeof(foo) = 32 */
static const
int log2s[]={0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5};
/* Type sizes. It is assumed that sizeof(double)>=sizeof(char *)... */
static const size_t typsz[6] = {
    sizeof(double),sizeof(char *),sizeof(long),sizeof(int),sizeof(short),1UL
};
/* mask off low-order bits */
static const unsigned long bitmask[6] = {
    0x0UL, 0x01UL, 0x03UL, 0x07UL, 0x0fUL, 0x01fUL
};

#define is_aligned(var,shift) (0U==(((unsigned long)(var))&bitmask[(shift)]))

static void initialize_shuffle(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    shuffle_initialized = register_build_strings(NULL, &source_file, s);
}

/* shuffle an initialized array of n items of size bytes
     using a modern variant of the Fisher-Yates algorithm.
   function randf(N) supplies uniformly-distributed random numbers
     in the range [0, N).
   function logf logs errors, warnings, etc. using log_arg.
   If erroneous args are supplied, errno is set to EINVAL and a
     negative value is returned, else
     array is permuted in-place; function return value is zero.
*/
int fisher_yates_shuffle(void *array, size_t n, size_t size,
    uint64_t (*randf)(uint64_t),
    void (*logf)(int, void *, const char *, ...), void *log_arg)
{
    int ret = 0;
#ifndef PP__FUNCTION__
    static const char __func__[] = "fisher_yates_shuffle";
#endif

    /* ensure initialization */
    if ((unsigned char)0U == shuffle_initialized)
        initialize_shuffle();
    /* argument sanity tests */
    if (NULL == array) {
        if (NULL != logf)
            logf(LOG_ERR, log_arg, "%s: %s line %d: NULL array",
                __func__, source_file, __LINE__);
        errno = EINVAL;
        ret = -1;
    }
    if (0U == size) {
        if (NULL != logf)
            logf(LOG_ERR, log_arg, "%s: %s line %d: zero array element size",
                __func__, source_file, __LINE__);
        errno = EINVAL;
        ret = -1;
    }
    if (2U > n) {
        if (NULL != logf)
            logf(LOG_ERR, log_arg, "%s: %s line %d: n (%u) < 2",
                __func__, source_file, __LINE__, n);
        errno = EINVAL;
        ret = -1;
    }
    if (NULL == randf) {
        if (NULL != logf)
            logf(LOG_ERR, log_arg, "%s: %s line %d: NULL randf",
                __func__, source_file, __LINE__);
        errno = EINVAL;
        ret = -1;
    }
    if (0 == ret) { /* arguments are sane */
        int swaptype; /* index into array of type sizes for swapping size */
        size_t s;  /* typsz[swaptype] */
        int i;     /* log2s[s] */

        /* determine optimum size for swapping */
        s=typsz[swaptype=0]; /* double */
        if ((size<s)||(!(is_aligned(size,i=log2s[s])))
        ||(!(is_aligned(array,i)))) {
            s=typsz[swaptype=3]; /* int */
            if ((size<s)||(!(is_aligned(size,i=log2s[s])))
            ||(!(is_aligned(array,i)))) {
                s=typsz[swaptype=4]; /* short */
                if ((size<s)||(!(is_aligned(size,i=log2s[s])))
                ||(!(is_aligned(array,i))))
                    swaptype=5; /* char */
            }
        }
        /* the following lines of code implement the actual shuffle */
        while (1UL<n) {
            size_t count, m;

            m=(size_t)randf(n--);
            /* swap elements at indices n and m */
            if (n!=m) { /* else nothing to do */
                /* size-aligned swap */
                char *pa=(char *)array+size*n, *pb=(char *)array+size*m;
                if (5==swaptype) { /* char */
                    char t;
                    for (count=size; 0UL<count; pa++,pb++,count--)
                        t=*pa, *pa=*pb, *pb=t;
                } else {
                    count = size>>i;
                    switch (swaptype) {
                        case 0 : /* double */
                            {   double *px=(double *)pa, *py=(double *)pb, t;
                                for (;0UL<count; px++,py++,count--)
                                    t=*px, *px=*py, *py=t;
                            }
                        break;
                        case 3 : /* int */
                            {   int *px=(int *)pa, *py=(int *)pb, t;
                                for (;0UL<count; px++,py++,count--)
                                    t=*px, *px=*py, *py=t;
                            }
                        break;
                        case 4 : /* short */
                            {   short *px=(short *)pa, *py=(short *)pb, t;
                                for (;0UL<count; px++,py++,count--)
                                    t=*px, *px=*py, *py=t;
                            }
                        break;
                    }
                }
            }
        }
    }
    return ret;
}
