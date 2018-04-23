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
* $Id: ~|^` @(#)    heap.c copyright 2016-2018 Bruce Lilly.   \ heap.c $
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
/* $Id: ~|^` @(#)   This is heap.c version 1.6 dated 2018-04-23T05:16:06Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.heap.c */

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
#define ID_STRING_PREFIX "$Id: heap.c ~|^` @(#)"
#define SOURCE_MODULE "heap.c"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2018-04-23T05:16:06Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* generalized to take lower(root) and upper indices of heap array */
/* for debugging: not performance-optimized */
#if 0
#define DEBUG_PRINT_INT 1
static void print_int_heap(int *heap, size_t l, size_t u, "/* "," */")
{
    int c, columns[15] = { 7, 3, 11, 1, 5, 9, 13, 0, 2, 4, 6, 8, 10, 12, 14 }, d, digits=2;
    size_t j, k, m, row;

    /* first pass: find maximum absolute value (for number of digits) */
    for (d=1,j=l; j<=u; j++) {
        c = heap[j];
        if (0 > c)
            c = 0 - c;
        if (c > d)
            d = c;
    }
    for (c=1,digits=1; c<d; digits++,c*=10) ;
#if DEBUG_PRINT_INT
(V)fprintf(stderr, "// %s: %s line %d: %s(..., l=%lu, u=%lu), %d digit%s\n", __func__, source_file, __LINE__, __func__, l, u, digits, digits==1?"":"s");
#endif
    for (row=0UL,m=2UL,k=l; row<4UL; m*=2UL,row++) {
        for (d=0,j=m/2UL; (k<=u)&&(j<m); d+=digits,j++,k++) {
            for(c=(digits+1)*columns[j-1UL]; d<c; d++)
                (V)fprintf(stderr, " ");
            (V)fprintf(stderr, "%*d", digits, heap[k]);
        }
        (V)fprintf(stderr, "\n");
        if (k > u) {
#if DEBUG_PRINT_INT
(V)fprintf(stderr, "// %s: %s line %d: %s(..., l=%lu, u=%lu), %d digit%s: k=%lu, j=%lu, m=%lu\n", __func__, source_file, __LINE__, __func__, l, u, digits, digits==1?"":"s", k, j, m);
#endif
            break; /* out of outer loop also */
        }
    }
}
#endif

/* heap parent/child node index calculations */
/* generalized for root at any position in array */
static
QUICKSELECT_INLINE
size_t left_child(size_t l, size_t p)
{
    return l + ((p-l)<<1) + 1UL;
}

static
QUICKSELECT_INLINE
size_t right_child(size_t l, size_t p)
{
    return l + (((p-l) + 1UL)<<1);
}

static
QUICKSELECT_INLINE
size_t parent(size_t l, size_t c)
{
    return l + ((c-l-1UL)>>1);
}

/* make a max-heap */
static
QUICKSELECT_INLINE
size_t heap_siftdown(char *base, size_t l, size_t p, size_t u, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    size_t c, rc;

    /* pre-condition: heap(p+1..u) */
    for (; (c = left_child(l,p)) <= u; p = c) {
        rc=right_child(l,p);
        if (rc <= u) {          /* compare parent (i.e. node being sifted down) to farthest child */
            if (0 > compar(base+size*c, base+size*rc)) /* left child is farther from parent than right child */
                c= rc; /* compare p with largest (in max-heap) child; swap will maintain heap order */
        }
        if (0 <= compar(base+size*p, base+size*c))        /* proper p vs. c relationship achieved */
            return p; /* nothing else to do */
        EXCHANGE_SWAP(swapf,base+size*p,base+size*c,size,alignsize,size_ratio,
            nsw++);
    }
    return p;
}

/* generalized to take root and upper indices of heap array */
void heapify_array(char *base, size_t r, size_t u, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    size_t p;

    for (p=parent(r,u); /*CONSTANTCONDITION*/ 1; p--) {
        (V)heap_siftdown(base,r,p,u,size,compar,swapf,alignsize,size_ratio);
        if (p == r)
            break;
    }

}

/* generalized to take root and upper indices of heap array */
/* Moves the element at the root (minimum in a min-heap, maximum in a max-heap)
      to the lowest index in the array beyond the end of the reduced heap.
   Return the (new) largest index in the heap.
*/
size_t heap_delete_root(char *base, size_t r, size_t n, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio)
{
    if (r != n) { /* swap root and last */
        EXCHANGE_SWAP(swapf,base+size*r,base+size*n,size,alignsize,size_ratio,
            nsw++);
    }
    --n;
    (V)heap_siftdown(base,r,r,n,size,compar,swapf,alignsize,size_ratio);
    if ((char)0==file_initialized) initialize_file(__FILE__);
    return n;
}
