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
* $Id: ~|^` @(#)    exchange.h copyright 2016-2018 Bruce Lilly. \ exchange.h $
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
/* $Id: ~|^` @(#)   This is exchange.h version 1.20 dated 2018-03-07T01:53:17Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "exchange" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/include/s.exchange.h */
/* Long description and rationale:
   Swapping data is a fundamental operation. Swapping depends on size and
   alignment of basic types. Swapping can be performed using types with sizes
   1, 2, 4, 8 times sizeof(char), and could be extended farther.
   Function alignment_size determines a suitable type size.
   Function swapn returns a pointer to a swap function which uses a specified
   type size. The functions take two pointer arguments and a count in chars to
   specify the data blocks to be swapped. Individual functions are swap1,
   swap2, swap4, and swap8.
   Function exchange swaps array elements by indices.
   Function blockmove swaps array elements in blocks to effectively exchange
   adjacent blocks of data, using a minimum number of swaps but not preserving
   order within the blocks.
   Function reverse reverses the order of elements in an array by swapping.
   Function rotate rotates elements in an array using reversals.
*/

/* version-controlled header file version information */
#define EXCHANGE_H_VERSION "exchange.h 1.20 2018-03-07T01:53:17Z"

#include <sys/types.h>          /* *_t (size_t) */
#include <limits.h>             /* *_MAX */
#include <stddef.h>             /* size_t NULL */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t */
#else
# include <float.h>             /* DBL_MAX_10_EXP */
#endif /* C99 */

#ifndef DEBUG_EXCHANGE
# define DEBUG_EXCHANGE 0
# define UNDEFINE_DEBUG_EXCHANGE 1
#else
# define UNDEFINE_DEBUG_EXCHANGE 0
#endif

#if DEBUG_EXCHANGE
# include <stdio.h>
#endif /* DEBUG_EXCHANGE */

#ifndef SWAP_COUNT_STATEMENT
# define SWAP_COUNT_STATEMENT /**/
#endif

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define EXCHANGE_INLINE inline
#else
# define EXCHANGE_INLINE /**/
#endif /* C99 */

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 198901L )
# define EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,mST)       \
    if (1UL==(msize_ratio)) {                                               \
        switch ((malignsize)) {                                             \
            case 1UL :                                                      \
                { char exch_t = *(ma);                                      \
                  *(ma)=*(mb), *(mb)=exch_t;                                \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            case 2UL :                                                      \
                { uint_least16_t exch_t                                     \
                  = (uint_least16_t)(*((uint_least16_t *)(ma)));            \
                  *((uint_least16_t *)(ma)) = *((uint_least16_t *)(mb)),    \
                  *((uint_least16_t *)(mb)) = exch_t;                       \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            case 4UL :                                                      \
                { uint_least32_t exch_t                                     \
                  = (uint_least32_t)(*((uint_least32_t *)(ma)));            \
                  *((uint_least32_t *)(ma)) = *((uint_least32_t *)(mb)),    \
                  *((uint_least32_t *)(mb)) = exch_t;                       \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            case 8UL :                                                      \
                { uint_least64_t exch_t                                     \
                  = (uint_least64_t)(*((uint_least64_t *)(ma)));            \
                  *((uint_least64_t *)(ma)) = *((uint_least64_t *)(mb)),    \
                  *((uint_least64_t *)(mb)) = exch_t;                       \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            default :                                                       \
                mswapf((ma),(mb),(msize));                                  \
            break;                                                          \
        }                                                                   \
    } else mswapf((ma),(mb),(msize)) /* caller provides terminating semicolon */
#else
# if INT_MAX == 2147483647
#  define SWAP_TYPE4 unsigned int
# elif LONG_MAX == 2147483647
#  define SWAP_TYPE4 unsigned long
# endif
# if DBL_MAX_10_EXP == 308
#  define SWAP_TYPE8 double
# else
#  define SWAP_TYPE8 char *
# endif
# define EXCHANGE_SWAP(mswapf,ma,mb,msize,malignsize,msize_ratio,mST)       \
    if (1UL==(msize_ratio)) {                                               \
        switch ((malignsize)) {                                             \
            case 1UL :                                                      \
                { char exch_t = *(ma);                                      \
                  *(ma)=*(mb), *(mb)=exch_t;                                \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            case 2UL :                                                      \
                { unsigned short exch_t                                     \
                  = (unsigned short)(*((unsigned short *)(ma)));            \
                  *((unsigned short *)(ma)) = *((unsigned short *)(mb)),    \
                  *((unsigned short *)(mb)) = exch_t;                       \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            case 4UL :                                                      \
                { SWAP_TYPE4 exch_t                                         \
                  = (SWAP_TYPE4)(*((SWAP_TYPE4 *)(ma)));                    \
                  *((SWAP_TYPE4 *)(ma)) = *((SWAP_TYPE4 *)(mb)),            \
                  *((SWAP_TYPE4 *)(mb)) = exch_t;                           \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            case 8UL :                                                      \
                { SWAP_TYPE8 exch_t                                         \
                  = (SWAP_TYPE8)(*((SWAP_TYPE8 *)(ma)));                    \
                  *((SWAP_TYPE8 *)(ma)) = *((SWAP_TYPE8 *)(mb)),            \
                  *((SWAP_TYPE8 *)(mb)) = exch_t;                           \
                }                                                           \
                mST;                                                        \
            break;                                                          \
            default :                                                       \
                mswapf((ma),(mb),(msize));                                  \
            break;                                                          \
        }                                                                   \
    } else mswapf((ma),(mb),(msize)) /* caller provides terminating semicolon */
#endif

/* Element swapping code depends on the size and alignment of elements. */
/* Basic types sizeof(foo) etc. are powers of 2. */
#define EXCHANGE_NTYPES 4 /* 1, 2, 4, 8 chars */

/* logarithms of two for index as sizeof */
/* table in lieu of calculation for speed */
/* valid up to sizeof(foo) = 32 */
static const
int log2s[]={0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5};

/* Type sizes in multiples of sizeof(char). */
/* for C89 (a.k.a. C90) and earlier, char, short, int or long, double or char * */
static const size_t typsz[EXCHANGE_NTYPES] = { 1UL,2UL,4UL,8UL };

/* Mask off low-order address bits, indexed by log of type size.  */
static const unsigned long bitmask[EXCHANGE_NTYPES] = {
    0x0UL, 0x01UL, 0x03UL, 0x07UL
};

/* *INDENT-ON* */

#define is_aligned(var,shift) (0U==(((unsigned long)(var))&bitmask[(shift)]))

/* array element moves in blocks */
/* Given two adjacent blocks of data delimited by pointers
   pa = start of block A, pb = start of block B, and pc pointing just past the
   end of block B, swap the minimum number of chars to place all of block B
   before block A (order within blocks is not preserved), returning a pointer
   to the start of block A (which is now after block B).
 Z |  A |   B  |  C
    pa   pb     pc
    vvvvvvvvvvv
 Z |  B  |  A  |  C
    pa    p     pc
 returned ^
   In addition to reversing the relative order of blocks A and B, this function
   is useful to consolidate like regions separated by a different type of
   region, e.g. when it is desirable to consolidate Z and B and/or A and C in
   the above diagram.
*/
static
EXCHANGE_INLINE
char *blockmove(char *pa, char *pb, char *pc,
    void (*swapf)(char *, char *, size_t))
{
    if (pb<pc) {
        if (pa<pb) {
            size_t p=pc-pb, n=pb-pa;
            if (p<n) n=p;
            swapf(pa,pc-n,n); return pa+p;
        } return pc;
    } return pa;
}

/* Reverse the order of elements [first,beyond) in array at base by swapping. */
static
EXCHANGE_INLINE
void reverse(char *base, size_t first, size_t beyond, size_t size,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    if (beyond>first+1UL) {
        char *pl=base+first*size, *pr=base+(beyond-1UL)*size;
        for (; pl<pr; pl+=size,pr-=size) {
            EXCHANGE_SWAP(swapf,pl,pr,size,alignsize,size_ratio,SWAP_COUNT_STATEMENT);
        }
    }
}

/* Rotate elements in [first,beyond) to the left by i positions,
   where i=cut-first, by interchanging blocks using pairwise swaps.
*/
/* first block [f,m) interchanged (while preserving relative order) with
   second block [m,b).
*/
/* rotation using pointers to elements */

/* enable or disable special-case code for rotation-by-1 */
#define USE_SPECIAL_CASE_ROTATION_BY_1 1

static
EXCHANGE_INLINE
void protate(register char *pf, register char *pm, register char *pb,
    size_t size, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio)
{
    register size_t i, leftsize=pm-pf, rightsize=pb-pm;
    /* +--------------------+ */
    /* |      y |   x       | */
    /* +--------------------+ */
    /*  f        m           b*/
    /* +--------------------+ */
    /* |  x      |      y |x| */
    /* +--------------------+ */
    /*            f        m b*/
    /* in-place   iterate     */
    do {
        if (leftsize==rightsize) { /* simple swaps */
#if DEBUG_EXCHANGE
            (V)fprintf(stderr,"protate: leftsize==rightsize=%lu\n",leftsize);
#endif /* DEBUG_EXCHANGE */
            for (i=0UL; i<leftsize; i+=size) {
#if DEBUG_EXCHANGE
                (V)fprintf(stderr,"protate: swap pf@%p,pm@%p\n",pf,pm);
#endif /* DEBUG_EXCHANGE */
                EXCHANGE_SWAP(swapf,pf,pm,size,alignsize,size_ratio,SWAP_COUNT_STATEMENT);
                pf+=size, pm+=size;
            }
            break;
        } else if (leftsize>rightsize) {
#if DEBUG_EXCHANGE
            (V)fprintf(stderr,"protate: leftsize=%lu>rightsize=%lu\n",leftsize,rightsize);
#endif /* DEBUG_EXCHANGE */
            /* more elements in left block */
#if USE_SPECIAL_CASE_ROTATION_BY_1
            if (size==rightsize) { /* special-case rotation by 1 */
                register char *px;
#if DEBUG_EXCHANGE
                (V)fprintf(stderr,"protate: rightsize==size=%lu: hold pm value in t, shift [pf,pm) right, put t in pf\n",rightsize);
#endif /* DEBUG_EXCHANGE */
                switch (alignsize) {
                    case 8UL :
                        for (; pm<pb; pm+=alignsize,pf+=alignsize) {
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
                            uint_least64_t t=*((uint_least64_t *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((uint_least64_t *)(px+size))
                               =*((uint_least64_t *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((uint_least64_t *)pf)=t;
#else
# if ( LONG_MAX > 2147483647 ) && ( LONG_MAX == 9223372036854775807 )
                            long t=*((long *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((long *)(px+size))
                               =*((long *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((long *)pf)=t;
# elif DBL_MAX_10_EXP == 308
                            double t=*((double *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((double *)(px+size))
                               =*((double *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((double *)pf)=t;
# else
                            char *t=*((char **)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((char **)(px+size))
                               =*((char **)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((char **)pf)=t;
# endif
#endif /* C99 */
                        }
                    break;
                    case 4UL : /* uint_least32_t */
                        for (; pm<pb; pm+=alignsize,pf+=alignsize) {
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
                            uint_least32_t t=*((uint_least32_t *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((uint_least32_t *)(px+size))
                               =*((uint_least32_t *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((uint_least32_t *)pf)=t;
#else
# if INT_MAX == 2147483647
                            int t=*((int *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((int *)(px+size))
                               =*((int *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((int *)pf)=t;
# elif LONG_MAX == 2147483647
                            long t=*((long *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((long *)(px+size))
                               =*((long *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((long *)pf)=t;
# endif
#endif /* C99 */
                        }
                    break;
                    case 2UL : /* uint_least16_t */
                        for (; pm<pb; pm+=alignsize,pf+=alignsize) {
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
                            uint_least16_t t=*((uint_least16_t *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((uint_least16_t *)(px+size))
                               =*((uint_least16_t *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((uint_least16_t *)pf)=t;
#else
                            short t=*((short *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((short *)(px+size))
                               =*((short *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((short *)pf)=t;
#endif /* C99 */
                        }
                    break;
                    default : /* uint_least8_t or char */
                        for (; pm<pb; pm+=alignsize,pf+=alignsize) {
                            char t=*((char *)pm);
                            for (px=pm-size; px>=pf; px-=size) {
                               *((char *)(px+size))
                               =*((char *)px);
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((char *)pf)=t;
                        }
                    break;
                }
                return;
            }
#endif
            /* +--------------------+ */
            /* |        y      |  x | */
            /* +--------------------+ */
            /*  f               m    b*/
            /* +--------------------+ */
            /* |     y    |  x |  y | */
            /* +--------------------+ */
            /*  f          m    b     */
            /* iterate          in-place */
            for (i=0UL; i<rightsize; i+=size) {
                pm-=size, pb-=size;
#if DEBUG_EXCHANGE
                (V)fprintf(stderr,"protate: swap pm@%p,pb@%p\n",pm,pb);
#endif /* DEBUG_EXCHANGE */
                EXCHANGE_SWAP(swapf,pm,pb,size,alignsize,size_ratio,SWAP_COUNT_STATEMENT);
            }
            leftsize-=rightsize;
        } else /* (leftsize<rightsize) */ {
#if DEBUG_EXCHANGE
            (V)fprintf(stderr,"protate: leftsize=%lu<rightsize=%lu\n",leftsize,rightsize);
#endif /* DEBUG_EXCHANGE */
            /* more elements in right block */
#if USE_SPECIAL_CASE_ROTATION_BY_1
            if (size==leftsize) { /* special-case rotation by 1 */
                register char *px;
#if DEBUG_EXCHANGE
                (V)fprintf(stderr,"protate: leftsize==size=%lu\n",leftsize);
#endif /* DEBUG_EXCHANGE */
                switch (alignsize) {
                    case 8UL :
                        for (pb-=size; pf<pm; pf+=alignsize,pb+=alignsize) {
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
                            uint_least64_t t=*((uint_least64_t *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((uint_least64_t *)px)
                               =*((uint_least64_t *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((uint_least64_t *)pb)=t;
#else
# if ( LONG_MAX > 2147483647 ) && ( LONG_MAX == 9223372036854775807 )
                            long t=*((long *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((long *)px)
                               =*((long *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((long *)pb)=t;
# elif DBL_MAX_10_EXP == 308
                            double t=*((double *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((double *)px)
                               =*((double *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((double *)pb)=t;
# else
                            char *t=*((char **)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((char **)px)
                               =*((char **)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((char **)pb)=t;
# endif
#endif /* C99 */
                        }
                    break;
                    case 4UL : /* uint_least32_t */
                        for (pb-=size; pf<pm; pf+=alignsize,pb+=alignsize) {
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
                            uint_least32_t t=*((uint_least32_t *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((uint_least32_t *)px)
                               =*((uint_least32_t *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((uint_least32_t *)pb)=t;
#else
# if INT_MAX == 2147483647
                            int t=*((int *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((int *)px)
                               =*((int *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((int *)pb)=t;
# elif LONG_MAX == 2147483647
                            long t=*((long *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((long *)px)
                               =*((long *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((long *)pb)=t;
# endif
#endif /* C99 */
                        }
                    break;
                    case 2UL : /* uint_least16_t */
                        for (pb-=size; pf<pm; pf+=alignsize,pb+=alignsize) {
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
                            uint_least16_t t=*((uint_least16_t *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((uint_least16_t *)px)
                               =*((uint_least16_t *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((uint_least16_t *)pb)=t;
#else
                            short t=*((short *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((short *)px)
                               =*((short *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((short *)pb)=t;
#endif /* C99 */
                        }
                    break;
                    default : /* uint_least8_t or char */
                        for (pb-=size; pf<pm; pf+=alignsize,pb+=alignsize) {
                            char t=*((char *)pf);
                            for (px=pf; px<pb; px+=size) {
                               *((char *)px)
                               =*((char *)(px+size));
                               SWAP_COUNT_STATEMENT ;
                            }
                            *((char *)pb)=t;
                        }
                    break;
                }
                return;
            }
#endif
            /* +--------------------+ */
            /* |  y  |     x        | */
            /* +--------------------+ */
            /*  f     m              b*/
            /* +--------------------+ */
            /* |  x  |  y  |   x    | */
            /* +--------------------+ */
            /*        f     m        b*/
            /* in-place iterate */
            for (i=0UL; i<leftsize; i+=size) {
#if DEBUG_EXCHANGE
                (V)fprintf(stderr,"protate: swap pf@%p,pm@%p\n",pf,pm);
#endif /* DEBUG_EXCHANGE */
                EXCHANGE_SWAP(swapf,pf,pm,size,alignsize,size_ratio,SWAP_COUNT_STATEMENT);
                pf+=size, pm+=size;
            }
            rightsize-=leftsize;
        }
    } while ((0UL<leftsize)&&(0UL<rightsize));
}

/* rotation using element indices */
static
EXCHANGE_INLINE
void irotate(register char *base, size_t f, size_t m, size_t b, size_t size,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    register char *pf=base+f*size, *pm=base+m*size, *pb=base+b*size;
#if DEBUG_EXCHANGE
    (V)fprintf(stderr,"irotate: f=%lu, m=%lu, b=%lu, pf@%p, pm@%p, pb@%p\n",f,m,b,pf,pm,pb);
#endif /* DEBUG_EXCHANGE */
    protate(pf,pm,pb,size,swapf,alignsize,size_ratio);
}

/* Determine and return a type size (in bytes) of a type with size and alignment
   suitable for swapping and rotation functions.
*/
static
EXCHANGE_INLINE
size_t alignment_size(char *base, size_t size)
{
    int i, t;  /* general integer variables */
    size_t s;  /* size of type under consideration */

    /* Determine size of data chunks to copy for element swapping.  Size is
       determined by element size and alignment. typsz is an array of type sizes
       (uint8_t, uint16_t, uint32_t, uint64_t).
    */
    /* use largest type which has suitable size and alignment */
    for (t=(EXCHANGE_NTYPES)-1; t>=0; t--) {
        s=typsz[t]; /* size in chars */
        if (s>size) continue; /* Goldilocks: "This size is too big!" */
        if (!(is_aligned(size,i=log2s[s]))||(!(is_aligned(base,i)))) continue;
        return s; /* size and alignment are "just right" */
    }
    return 1UL; /* safety net; also tell compiler to STFU */
}

/* Array element swaps: */
/* count is in chars */
/* versions to swap by char (8 bits), int_least16_t, int_least32_t, int_least64_t */
static
EXCHANGE_INLINE
void swap1(register char *pa, register char *pb, register size_t count)
{
    register char t;
    do { t=*pa, *pa++=*pb, *pb++=t, count--; } while (0UL<count);
}

static
EXCHANGE_INLINE
void swap2(char *pa, char *pb, register size_t count)
{
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    register int_least16_t *px=(int_least16_t *)pa, *py=(int_least16_t *)pb, t;
#else
    register short *px=(short *)pa, *py=(short *)pb, t;
#endif /* C99 */
    for (count>>=1; 0UL<count; t=*px,*px++=*py,*py++=t,count--);
}

static
EXCHANGE_INLINE
void swap4(char *pa, char *pb, register size_t count)
{
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    register int_least32_t *px=(int_least32_t *)pa, *py=(int_least32_t *)pb, t;
#else
# if INT_MAX == 2147483647
    register int *px=(int *)pa, *py=(int *)pb, t;
# elif LONG_MAX == 2147483647
    register long *px=(long *)pa, *py=(long *)pb, t;
# endif
#endif /* C99 */
    for (count>>=2; 0UL<count; t=*px,*px++=*py,*py++=t,count--);
}

static
EXCHANGE_INLINE
void swap8(char *pa, char *pb, register size_t count)
{
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    register int_least64_t *px=(int_least64_t *)pa, *py=(int_least64_t *)pb, t;
#else
# if DBL_MAX_10_EXP == 308
    register double *px=(double *)pa, *py=(double *)pb, t;
# else
    register char **px=(char **)pa, *py=(char **)pb, t;
# endif
#endif /* C99 */
    for (count>>=3; 0UL<count; t=*px,*px++=*py,*py++=t,count--);
}

static
EXCHANGE_INLINE
void (*swapn(size_t alignsize))(char *, char *, size_t)
{
    /* Determine size of data chunks to copy for element swapping.  Size is
       determined by element size and alignment. typsz is an array of type sizes
    */
    switch (alignsize) {
        case 8UL : return swap8;
        case 4UL : return swap4;
        case 2UL : return swap2;
        default  : return swap1;
    }
}

static /* prevent multiple definition conflicts */
EXCHANGE_INLINE
void exchange(char *array, register size_t i, register size_t j, size_t count, register size_t size)
{
    char *pa, *pb;
    void (*swapf)(char *, char *, size_t);

#if DEBUG_EXCHANGE
    printf("%s(%p,%lu,%lu,%lu,%lu)\n",__func__,(void *)array,i,j,count,size);
#endif /* DEBUG_EXCHANGE */
    swapf=swapn(alignment_size(array,size));
    pa=array + i * size;
    pb=array + j * size;
    swapf(pa,pb,count*size);
#if DEBUG_EXCHANGE
    printf("\n");
#endif /* DEBUG_EXCHANGE */
}

#undef EXCHANGE_INLINE

#if UNDEFINE_DEBUG_EXCHANGE
#undef DEBUG_EXCHANGE
#endif
#undef UNDEFINE_DEBUG_EXCHANGE
#undef EXCHANGE_NTYPES

#define	EXCHANGE_H_INCLUDED
#endif
