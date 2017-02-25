/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining comparison functions (suitable for inline expansion)
*/
#ifndef	COMPARE_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    compare.h copyright 2016 Bruce Lilly. \ compare.h $
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
/* $Id: ~|^` @(#)   This is compare.h version 1.4 dated 2017-02-25T04:11:40Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "compare" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/include/s.compare.h */

/* version-controlled header file version information */
#define COMPARE_H_VERSION "compare.h 1.4 2017-02-25T04:11:40Z"

#include <stddef.h>             /* size_t NULL */

/* Generic macro. Implementation avoids overflow. */
#undef typecmp
#define typecmp(type) {                                            \
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {              \
        const type a= *((const type *)p1), b= *((const type *)p2); \
                                                                   \
    if (a > b)                                                     \
        return 1;                                                  \
    if (a < b)                                                     \
        return -1;                                                 \
    }                                                              \
    return 0;                                                      \
}

static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int charcmp(const void *p1, const void *p2)
typecmp(char)

static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int doublecmp(const void *p1, const void *p2)
typecmp(double)

static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int intcmp(const void *p1, const void *p2)
typecmp(int)

static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int shortcmp(const void *p1, const void *p2)
typecmp(short)

static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int longcmp(const void *p1, const void *p2)
typecmp(long)

static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int ulcmp(const void *p1, const void *p2)
typecmp(unsigned long)

#define	COMPARE_H_INCLUDED
#endif
