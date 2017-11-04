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
* $Id: ~|^` @(#)    swap.c copyright 2016-2017 Bruce Lilly.   \ swap.c $
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
/* $Id: ~|^` @(#)   This is swap.c version 1.2 dated 2017-11-03T19:32:18Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.swap.c */

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
#define ID_STRING_PREFIX "$Id: swap.c ~|^` @(#)"
#define SOURCE_MODULE "swap.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2017-11-03T19:32:18Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* Array element swaps: */
/* Called many times from several functions. */
/* count is in chars */
/* versions to swap by char (8 bits), int_least16_t, int_least32_t, int_least64_t */
/* instrumented versions support assert/debug (avoid adding cost to op tests) */
static
void iswap1(char *pa, char *pb, register size_t count)
{
    A(pa!=pb);A(0UL<count);
#if DEBUG_CODE
if (DEBUGGING(SWAP_DEBUG)) (V) fprintf(stderr,"// %s line %d: pa=%p, pb=%p, count=%lu\n",__func__,__LINE__,(void *)pa,(void *)pb,(unsigned long)count);
#endif
    nsw+=count;
    swap1(pa,pb,count);
}

static
void iswap2(char *pa, char *pb, register size_t count)
{
    A(pa!=pb);A(0UL<count);
    nsw+=(count>>1);
    swap2(pa,pb,count);
}

static
void iswap4(char *pa, char *pb, register size_t count)
{
    A(pa!=pb);A(0UL<count);
    nsw+=(count>>2);
    swap4(pa,pb,count);
}

static
void iswap8(char *pa, char *pb, register size_t count)
{
    A(pa!=pb);A(0UL<count);
    nsw+=(count>>3);
    swap8(pa,pb,count);
}

void (*iswapn(size_t alignsize))(char *, char *, size_t)
{
    /* Determine size of data chunks to copy for element swapping.  Size is
       determined by element size and alignment. typsz is an array of type sizes
    */
    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (alignsize) {
        case 8UL : return iswap8;
        case 4UL : return iswap4;
        case 2UL : return iswap2;
        default  : return iswap1;
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
size_t ausz = sizeof(union aligned_union);

void swapd(void * restrict p1, void * restrict p2, register size_t size)
{
    union aligned_union pv[(size+ausz-1UL)/ausz];
    A(p1!=p2);A(0UL<size);
    (V)memcpy((void *)pv,p1,size);
    (V)memcpy(p1,p2,size);
    (V)memcpy(p2,(void *)pv,size);
}

void iswapd(void * restrict p1, void * restrict p2, register size_t size)
{
    nsw+=2UL;
    swapd(p1,p2,size);
}
#endif /* C99 */

void swapstruct(register struct data_struct *p1, register struct data_struct *p2, register size_t count)
{
    struct data_struct t;
    count /= sizeof(struct data_struct);
    for (; 0UL<count; t=*p1,*p1++=*p2,*p2++=t,count--);
}

void iswapstruct(register struct data_struct *p1, register struct data_struct *p2, register size_t count)
{
    struct data_struct t;
    count /= sizeof(struct data_struct);
    for (nsw+=count; 0UL<count; t=*p1,*p1++=*p2,*p2++=t,count--);
}
