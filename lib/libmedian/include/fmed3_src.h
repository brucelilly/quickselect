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
* $Id: ~|^` @(#)    fmed3_src.h copyright 2016-2019 Bruce Lilly.   \ fmed3_src.h $
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
/* $Id: ~|^` @(#)   This is fmed3_src.h version 1.1 dated 2019-03-06T21:27:45Z. \ fmed3_src.h $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.fmed3_src.h */

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
#define ID_STRING_PREFIX "$Id: fmed3_src.h ~|^` @(#)"
#define SOURCE_MODULE "fmed3_src.h"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-03-06T21:27:45Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

/* local header files needed */
#include "initialize_src.h"

/* Ternary median-of-3, biased toward middle element if possible, else first element. */
/* Average 8/3 comparisons for 3 elements (distinct values) = 0.889 comparisons/element */
/* fmed3 does not change values referenced by pointers, but these are not
   declared as const to prevent spurious compiler warnings about discarding const
   qualifiers on return, when the return value is passed to a swap function to
   rearrange elements.  Traditionally, const would be used in the argument
   declarations to indicate to the programmer/maintainer that the function
   guarantees no modification (while the returned pointer is not declared const
   because what the caller does with the returned pointer is beyond the control
   of this function), but compiler writers seem to conclude that that's
   inappropriate. Here const appears commented out to hide it from the compiler
   (and its authors).  ISO C<some large number> _restrict isn't appropriate
   because:
      A) it has entirely different semantics
      B) it would imply something (in this case uniqueness of supplied pointers)
         which is the caller's responsibility, not this function's. Moreover,
         adding pointer uniqueness tests (i.e. avoiding comparisons if pointers
         are equal, setting comparison value to zero) would pointlessly add
         cost, as this function is ONLY called with distinct pointers in this
         application.
      C) it is not backwards-compatible with earlier C standards
*/
#if __STDC_WANT_LIB_EXT1__
QUICKSELECT_FMED3_S
#else
QUICKSELECT_FMED3
#endif
{
    register int c=COMPAR(pa,pb);
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(MEDIAN_DEBUG)) {
    if ((char)0==file_initialized) initialize_file(__FILE__);
(V)fprintf(stderr, "/* %s: %s line %d: pa=%p[%lu], pb=%p[%lu], pc=%p[%lu] */\n",__func__,source_file,__LINE__,(void *)pa,(pa-base)/size,(void *)pb,(pb-base)/size,(void *)pc,(pc-base)/size);
}
#endif
    A(0U==(options&(QUICKSELECT_INDIRECT)));
    if (0!=c) {
        register int d=COMPAR(pb,pc);
        if (0!=d) {
            if ((0<d)&&(0>c)) {
#if LIBMEDIAN_TEST_CODE
                if (0>COMPAR(pa,pc)) pb=pc;
                else pb=pa;
#else
                if (0>COMPAR(pa,pc)) return pc;
                else return pa;
#endif
            } else if ((0>d)&&(0<c)) {
#if LIBMEDIAN_TEST_CODE
                if (0<COMPAR(pa,pc)) pb=pc;
                else pb=pa;
#else
                if (0<COMPAR(pa,pc)) return pc;
                else return pa;
#endif
            }
        }
    }
#if LIBMEDIAN_TEST_CODE
if (DEBUGGING(REPIVOT_DEBUG)||DEBUGGING(MEDIAN_DEBUG)) {
(V)fprintf(stderr, "/* %s: %s line %d: pb=%p[%lu] */\n",__func__,source_file,__LINE__,(void *)pb,(pb-base)/size);
}
#endif
    return pb;
}
