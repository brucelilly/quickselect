/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining functions exchange and swap (suitable for inline expansion)
*/
#ifndef	EXCHANGE_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    exchange.h copyright 2016-2017 Bruce Lilly. \ exchange.h $
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
/* $Id: ~|^` @(#)   This is exchange.h version 1.12 dated 2017-02-08T09:01:03Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "exchange" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/include/s.exchange.h */
/* Long description and rationale:
   Swapping data is a fundamental operation. There are three entry points
   provided: exchange(array, index, index, size) swaps array elements of size at
   the two indices.  swap(a, b, count, size) swaps count elements of size at
   pointers a and b.  swap2(a, b, n) swaps b bytes at pointers a and b. exchange
   and swap are wrapper functions that call swap2 to do the work.
*/

/* version-controlled header file version information */
#define EXCHANGE_H_VERSION "exchange.h 1.12 2017-02-08T09:01:03Z"

#include <sys/types.h>          /* *_t (size_t) */
#include <stddef.h>

#ifndef DEBUG_EXCHANGE
# define DEBUG_EXCHANGE 0
# define UNDEFINE_DEBUG_EXCHANGE 1
#else
# define UNDEFINE_DEBUG_EXCHANGE 0
#endif

#if DEBUG_EXCHANGE
# include <stdio.h>
#endif /* DEBUG_EXCHANGE */

/* assume sizeof(foo) etc. are powers of 2 */
/* logarithms of two for index as sizeof */
/* table in lieu of calculation for speed */
/* valid up to sizeof(foo) = 32 */
static const
int log2s[]={0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5};
/* type sizes. It is assumed that sizeof(double)>=sizeof(char *)... */
static const size_t typsz[6] = {
    sizeof(double),sizeof(char *),sizeof(long),sizeof(int),sizeof(short),1UL
};
/* mask off low-order bits */
static const unsigned long bitmask[5] = {
    0x0UL, 0x01UL, 0x03UL, 0x07UL, 0x0fUL
};

#define is_aligned(var,shift) (0U==(((unsigned long)(var))&bitmask[(shift)]))

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
void swap2(char *pa, char *pb, size_t count)
{
#if DEBUG_EXCHANGE
    printf("%s(%p,%p,%lu)\n",__func__,(void *)pa,(void *)pb,count);
#endif /* DEBUG_EXCHANGE */
    if (pa==pb) return; /* nothing to do */
    else {
        int e, f, g;
        size_t m, s, x, y;
        unsigned long d; /* constant difference between pa and pb */

        if (pa<pb) d=(unsigned long)pb-(unsigned long)pa;
        else d=(unsigned long)pa-(unsigned long)pb;
        while (0UL<count) {
#if DEBUG_EXCHANGE > 1
            printf("%s: count=%lu, pa=%p, pb=%p\n",
                __func__,count,(void *)pa,(void *)pb);
#endif /* DEBUG_EXCHANGE */
            for (x=0UL; x<6UL; x++) {
                s=typsz[x];
                if (count>=s) {
                    e=log2s[s];
                    if (is_aligned(d,e)&&is_aligned(pa,e)) {
#if DEBUG_EXCHANGE > 2
                        printf("%s line %d: aligned to size %lu\n",
                            __func__,__LINE__,s);
#endif /* DEBUG_EXCHANGE */
                        /* Possible partial alignment to larger type? */
                        if (0UL==x) y=0UL;
                        else for (y=x-1UL; 0UL<y; y--) {
                            f=typsz[y];
                            if ((count>=f)&&(is_aligned(d,g=log2s[f]))) {
                                m=((f)-(((unsigned long)pa)&bitmask[g]))>>e;
#if DEBUG_EXCHANGE > 2
                                printf(
                                    "%s line %d: partial aligned to size %lu\n",
                                    __func__,__LINE__,f);
#endif /* DEBUG_EXCHANGE */
                                break;
                            }
                        }
                        if (0UL==y) m=count>>e; /* no partial aalignment */
                        /* swap aligned size in suitable quantity */
                        switch (x) {
                            case 0 : /* double */
                                for (;0UL<m; pa+=s,pb+=s,m--,count-=s) {
                                    double t = *((double *)pa);
                                    *((double *)pa)=*((double *)pb),
                                    *((double *)pb)=t;
                                }
                            break;
                            case 1 : /* pointer */
                                for (;0UL<m; pa+=s,pb+=s,m--,count-=s) {
                                    char *t = *((char **)pa);
                                    *((char **)pa)=*((char **)pb),
                                    *((char **)pb)=t;
                                }
                            break;
                            case 2 : /* long */
                                for (;0UL<m; pa+=s,pb+=s,m--,count-=s) {
                                    long t = *((long *)pa);
                                    *((long *)pa)=*((long *)pb),
                                    *((long *)pb)=t;
                                }
                            break;
                            case 3 : /* int */
                                for (;0UL<m; pa+=s,pb+=s,m--,count-=s) {
                                    int t = *((int *)pa);
                                    *((int *)pa)=*((int *)pb),
                                    *((int *)pb)=t;
                                }
                            break;
                            case 4 : /* short */
                                for (;0UL<m; pa+=s,pb+=s,m--,count-=s) {
                                    short t = *((short *)pa);
                                    *((short *)pa)=*((short *)pb),
                                    *((short *)pb)=t;
                                }
                            break;
                            case 5 : /* char */
                                for (;0UL<m; pa++,pb++,m--,count--) {
                                    char t = *pa;
                                    *pa=*pb, *pb=t;
                                }
                            break;
                        }
                        if (0UL==count) return;
                        break; /* try again with largest size */
                    }
                }
            }
        }
    }
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
void swap(char *pa, char *pb, size_t count, size_t size)
{
#if DEBUG_EXCHANGE
    printf("%s(%p,%p,%lu,%lu)\n",__func__,(void *)pa,(void *)pb,count,size);
#endif /* DEBUG_EXCHANGE */
    swap2(pa,pb,count*size);
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
static
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
void exchange(char *array, size_t i, size_t j, size_t count, size_t size)
{
    char *pa, *pb;

#if DEBUG_EXCHANGE
    printf("%s(%p,%lu,%lu,%lu,%lu)\n",__func__,(void *)array,i,j,count,size);
#endif /* DEBUG_EXCHANGE */
    pa=array + i * size;
    pb=array + j * size;
    swap2(pa,pb,count);
#if DEBUG_EXCHANGE
    printf("\n");
#endif /* DEBUG_EXCHANGE */
}

#if UNDEFINE_DEBUG_EXCHANGE
#undef DEBUG_EXCHANGE
#endif
#undef UNDEFINE_DEBUG_EXCHANGE
/* leave EXCHANGE_DO_* so that configuration can be recorded */

#define	EXCHANGE_H_INCLUDED
#endif
