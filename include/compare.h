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
* $Id: ~|^` @(#)    compare.h copyright 2016-2017 Bruce Lilly. \ compare.h $
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
/* $Id: ~|^` @(#)   This is compare.h version 1.6 dated 2017-09-17T02:29:07Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "compare" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/include/s.compare.h */

/* version-controlled header file version information */
#define COMPARE_H_VERSION "compare.h 1.6 2017-09-17T02:29:07Z"

#include <stddef.h>             /* size_t NULL */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t */
#endif /* C99 */

/* Generic macro. Implementation avoids overflow. */
#undef typecmp
#define typecmp(type) {                                            \
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {              \
        const type a= *((const type *)p1), b= *((const type *)p2); \
                                                                   \
        if (a > b) return 1;                                       \
        if (a < b) return -1;                                      \
    }                                                              \
    return 0;                                                      \
}

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int charcmp(const void *p1, const void *p2)
typecmp(char)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int doublecmp(const void *p1, const void *p2)
typecmp(double)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int floatcmp(const void *p1, const void *p2)
typecmp(float)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int intcmp(const void *p1, const void *p2)
typecmp(int)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int shortcmp(const void *p1, const void *p2)
typecmp(short)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int longcmp(const void *p1, const void *p2)
typecmp(long)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int ulcmp(const void *p1, const void *p2)
typecmp(unsigned long)

static
#if defined(__STDC__) && __STDC_VERSION__ >= 199901UL
inline
#endif
int size_tcmp(const void *p1, const void *p2)
typecmp(size_t)

/* C99 mandatory specific-size types */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
static inline int int_least8_tcmp(const void *p1, const void *p2)
typecmp(int_least8_t)

static inline int int_fast8_tcmp(const void *p1, const void *p2)
typecmp(int_fast8_t)

static inline int uint_least8_tcmp(const void *p1, const void *p2)
typecmp(uint_least8_t)

static inline int uint_fast8_tcmp(const void *p1, const void *p2)
typecmp(uint_fast8_t)

static inline int int_least16_tcmp(const void *p1, const void *p2)
typecmp(int_least16_t)

static inline int int_fast16_tcmp(const void *p1, const void *p2)
typecmp(int_fast16_t)

static inline int uint_least16_tcmp(const void *p1, const void *p2)
typecmp(uint_least16_t)

static inline int uint_fast16_tcmp(const void *p1, const void *p2)
typecmp(uint_fast16_t)

static inline int int_least32_tcmp(const void *p1, const void *p2)
typecmp(int_least32_t)

static inline int int_fast32_tcmp(const void *p1, const void *p2)
typecmp(int_fast32_t)

static inline int uint_least32_tcmp(const void *p1, const void *p2)
typecmp(uint_least32_t)

static inline int uint_fast32_tcmp(const void *p1, const void *p2)
typecmp(uint_fast32_t)

static inline int int_least64_tcmp(const void *p1, const void *p2)
typecmp(int_least64_t)

static inline int int_fast64_tcmp(const void *p1, const void *p2)
typecmp(int_fast64_t)

static inline int uint_least64_tcmp(const void *p1, const void *p2)
typecmp(uint_least64_t)

static inline int uint_fast64_tcmp(const void *p1, const void *p2)
typecmp(uint_fast64_t)
#endif /* C99 */

#undef typecmp
#define	COMPARE_H_INCLUDED
#endif
