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
* $Id: ~|^` @(#)    ulsqrt.c copyright 2016-2018 Bruce Lilly.   \ ulsqrt.c $
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
/* $Id: ~|^` @(#)   This is ulsqrt.c version 1.2 dated 2018-03-06T20:16:50Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.ulsqrt.c */

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
#define ID_STRING_PREFIX "$Id: ulsqrt.c ~|^` @(#)"
#define SOURCE_MODULE "ulsqrt.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2018-03-06T20:16:50Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t ulsqrt(size_t n)
{
    if (1UL<n) {
        size_t q, r, s, t;

        q=(n>>2);
        r=ulsqrt(q);
        t=(r<<1);
        s=t++;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "// %s line %d: n=%lu, q=%lu, r=%lu, s=%lu, t=%lu t*t=%lu\n",__func__,__LINE__,n,q,r,s,t,t*t);
#endif
        if (t*t>n) return s;
        else return t;
    }
    if ((char)0==file_initialized) initialize_file(__FILE__);
    return n;
}
