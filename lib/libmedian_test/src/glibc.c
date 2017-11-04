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
* $Id: ~|^` @(#)    glibc.c copyright 2016-2017 Bruce Lilly.   \ glibc.c $
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
/* $Id: ~|^` @(#)   This is glibc.c version 1.2 dated 2017-11-03T19:35:12Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.glibc.c */

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
#define ID_STRING_PREFIX "$Id: glibc.c ~|^` @(#)"
#define SOURCE_MODULE "glibc.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2017-11-03T19:35:12Z"
#define COPYRIGHT_HOLDER "Free Software Foundation, Inc."
#define COPYRIGHT_DATE "1996-2016"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* ********************** glibc qsort code ********************************** */
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, see
<http://www.gnu.org/licenses/>. */

/* If you consider tuning this algorithm, you should consult first:
Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993. */


/* Byte-wise swap two items of size SIZE. */
/* BL: added swap count instrumentation */
#define SWAP(a, b, size) \
do { \
    size_t __size = (size); \
    char *__a = (a), *__b = (b); \
    nsw+=(__size); \
    do { \
        char __tmp = *__a; \
        *__a++ = *__b; \
        *__b++ = __tmp; \
    } while (--__size > 0); \
} while (0)

/* Discontinue quicksort algorithm when partition gets below this size.
This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct {
  char *lo;
  char *hi;
} stack_node;

/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
log(MAX_THRESH)). Since total_elements has type size_t, we get as
upper bound for log (total_elements):
bits per byte (CHAR_BIT) * sizeof(size_t). */
#define STACK_SIZE (CHAR_BIT * sizeof(size_t))
#define PUSH(low, high) ((V) ((top->lo = (low)), (top->hi = (high)), ++top))
#define POP(low, high) ((V) (--top, (low = top->lo), (high = top->hi)))
#define STACK_NOT_EMPTY (stack < top)


/* Order size using quicksort. This implementation incorporates
four optimizations discussed in Sedgewick:

1. Non-recursive, using an explicit stack of pointer that store the
next array partition to sort. To save time, this maximum amount
of space required to store an array of SIZE_MAX is allocated on the
stack. Assuming a 32-bit (64 bit) integer for size_t, this needs
only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
Pretty cheap, actually.

2. Chose the pivot element using a median-of-three decision tree.
This reduces the probability of selecting a bad pivot value and
eliminates certain extraneous comparisons.

3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
insertion sort to order the MAX_THRESH items within each partition.
This is a big win, since insertion sort is faster for small, mostly
sorted array segments.

4. The larger of the two sub-partitions is always pushed onto the
stack first, with the algorithm then concentrating on the
smaller partition. This *guarantees* no more than log (total_elems)
stack size is needed (actually O(1) in this case)! */

/* BL: name changed to avoid namespace conflicts; conform compar() to standard */
void
glibc_quicksort (void *const pbase, size_t total_elems, size_t size,
            int(*compar)(const void *, const void *))
{
  char *base_ptr = (char *) pbase;

  const size_t max_thresh = MAX_THRESH * size;

    if ((char)0==file_initialized) initialize_file(__FILE__); /* BL */
  if (total_elems == 0)
    /* Avoid lossage with unsigned arithmetic below. */
    return;

  if (total_elems > MAX_THRESH)
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;

      PUSH (NULL, NULL);

      while (STACK_NOT_EMPTY)
        {
          char *left_ptr;
          char *right_ptr;

/* Select median value from among LO, MID, and HI. Rearrange
LO and HI so the three values are sorted. This lowers the
probability of picking a pathological pivot value and
skips a comparison for both the LEFT_PTR and RIGHT_PTR in
the while loops. */

          char *mid = lo + size * ((hi - lo) / size >> 1);

          if ((compar) ((void *) mid, (void *) lo) < 0)
            SWAP (mid, lo, size);
          if ((compar) ((void *) hi, (void *) mid) < 0)
            SWAP (mid, hi, size);
          else
            goto jump_over;
          if ((compar) ((void *) mid, (void *) lo) < 0)
            SWAP (mid, lo, size);
        jump_over:;

          left_ptr = lo + size;
          right_ptr = hi - size;

/* Here's the famous ``collapse the walls'' section of quicksort.
Gotta like those tight inner loops! They are the main reason
that this algorithm runs much faster than others. */
          /* BL: glibc partition
             initially:
             +---------------------------+
             | |     ?    |m|    ?     | |
             +---------------------------+
                l          m          r
             partial:
             +---------------------------+
             |   < | ?    |m|    ? | >   |
             +---------------------------+
                    l      m      r
             final:
             +---------------------------+
             |   <      |  =  |      >   |
             +---------------------------+
                       r   m   l
          */
          do
            {
              while ((compar) ((void *) left_ptr, (void *) mid) < 0)
                left_ptr += size;

              while ((compar) ((void *) mid, (void *) right_ptr) < 0)
                right_ptr -= size;

              if (left_ptr < right_ptr)
                {
                  SWAP (left_ptr, right_ptr, size);
                  if (mid == left_ptr)
                    mid = right_ptr;
                  else if (mid == right_ptr)
                    mid = left_ptr;
                  left_ptr += size;
                  right_ptr -= size;
                }
              else if (left_ptr == right_ptr)
                {
                  left_ptr += size;
                  right_ptr -= size;
                  break;
                }
            }
          while (left_ptr <= right_ptr);
          npartitions++; /* BL: added partitions instrumentation */

/* Set up pointers for next iteration. First determine whether
left and right partitions are below the threshold size. If so,
ignore one or both. Otherwise, push the larger partition's
bounds on the stack and continue sorting the smaller one. */

          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
                /* Ignore both small partitions. */
                POP (lo, hi);
              else
                /* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
            /* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
              /* Push larger left partition indices. */
              PUSH (lo, right_ptr);
              lo = left_ptr;
            }
          else
            {
              /* Push larger right partition indices. */
              PUSH (left_ptr, hi);
              hi = right_ptr;
            }
        }
    }

/* Once the BASE_PTR array is partially sorted by quicksort the rest
is completely sorted using insertion sort, since this is efficient
for partitions below MAX_THRESH size. BASE_PTR points to the beginning
of the array to sort, and END_PTR points at the very last element in
the array (*not* one beyond it!). */

/* BL: name change to avoid namespace conflicts */
#define glmin(x, y) ((x) < (y) ? (x) : (y))

  {
    char *const end_ptr = &base_ptr[size * (total_elems - 1)];
    char *tmp_ptr = base_ptr;
    char *thresh = glmin (end_ptr, base_ptr + max_thresh);
    char *run_ptr;

/* Find smallest element in first threshold and place it at the
array's beginning. This is the smallest array element,
and the operation speeds up insertion sort's inner loop. */

    for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
      if ((compar) ((void *) run_ptr, (void *) tmp_ptr) < 0)
        tmp_ptr = run_ptr;

    if (tmp_ptr != base_ptr)
      SWAP (tmp_ptr, base_ptr, size);

    /* Insertion sort, running from left-hand-side up to right-hand-side. */

    run_ptr = base_ptr + size;
    while ((run_ptr += size) <= end_ptr)
      {
        tmp_ptr = run_ptr - size;
        while ((compar) ((void *) run_ptr, (void *) tmp_ptr) < 0)
          tmp_ptr -= size;

        tmp_ptr += size;
        if (tmp_ptr != run_ptr)
          {
            char *trav;

            trav = run_ptr + size;
            while (--trav >= run_ptr)
              {
                char c = *trav;
                char *hi, *lo;

                for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                  *hi = *lo;
                *hi = c;
              }
          }
      }
  }
}
/* ************************************************************************** */
