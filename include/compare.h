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
* $Id: ~|^` @(#)    compare.h copyright 2016-2018 Bruce Lilly. \ compare.h $
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
/* $Id: ~|^` @(#)   This is compare.h version 1.9 dated 2018-07-27T18:20:28Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "compare" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/include/s.compare.h */

/* version-controlled header file version information */
#define COMPARE_H_VERSION "compare.h 1.9 2018-07-27T18:20:28Z"

#include <stddef.h>             /* size_t NULL */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# include <stdint.h>            /* *int*_t */
# define COMPARE_INLINE inline
#else
# define COMPARE_INLINE /**/
#endif /* C99 */

#define COMPARE_EXTERN extern

/* Generic macro. Implementation avoids overflow. */
#undef TYPECMP
#define TYPECMP(type) {                                            \
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {              \
        const type a= *((const type *)p1), b= *((const type *)p2); \
                                                                   \
        if (a > b) return 1;                                       \
        if (a < b) return -1;                                      \
    }                                                              \
    return 0;                                                      \
}

#define COMPARE_DECLARATION(x) x;
#define COMPARE_DEFINITION(x,type) x TYPECMP(type)

#define CHARCMP COMPARE_EXTERN COMPARE_INLINE int charcmp(const void *p1, const void *p2)

#define DOUBLECMP COMPARE_EXTERN COMPARE_INLINE int doublecmp(const void *p1, const void *p2)

#define FLOATCMP COMPARE_EXTERN COMPARE_INLINE int floatcmp(const void *p1, const void *p2)

#define INTCMP COMPARE_EXTERN COMPARE_INLINE int intcmp(const void *p1, const void *p2)

#define SHORTCMP COMPARE_EXTERN COMPARE_INLINE int shortcmp(const void *p1, const void *p2)

#define LONGCMP COMPARE_EXTERN COMPARE_INLINE int longcmp(const void *p1, const void *p2)

#define ULCMP COMPARE_EXTERN COMPARE_INLINE int ulcmp(const void *p1, const void *p2)

#define SIZE_TCMP COMPARE_EXTERN COMPARE_INLINE int size_tcmp(const void *p1, const void *p2)

COMPARE_DECLARATION(CHARCMP)
COMPARE_DECLARATION(DOUBLECMP)
COMPARE_DECLARATION(FLOATCMP)
COMPARE_DECLARATION(INTCMP)
COMPARE_DECLARATION(SHORTCMP)
COMPARE_DECLARATION(LONGCMP)
COMPARE_DECLARATION(ULCMP)
COMPARE_DECLARATION(SIZE_TCMP)

/* C99 mandatory specific-size types */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define INT_LEAST8_TCMP COMPARE_EXTERN COMPARE_INLINE int int_least8_tcmp(const void *p1, const void *p2)

#define INT_FAST8_TCMP COMPARE_EXTERN COMPARE_INLINE int int_fast8_tcmp(const void *p1, const void *p2)

#define UINT_LEAST8_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_least8_tcmp(const void *p1, const void *p2)

#define UINT_FAST8_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_fast8_tcmp(const void *p1, const void *p2)

#define INT_LEAST16_TCMP COMPARE_EXTERN COMPARE_INLINE int int_least16_tcmp(const void *p1, const void *p2)

#define INT_FAST16_TCMP COMPARE_EXTERN COMPARE_INLINE int int_fast16_tcmp(const void *p1, const void *p2)

#define UINT_LEAST16_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_least16_tcmp(const void *p1, const void *p2)

#define UINT_FAST16_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_fast16_tcmp(const void *p1, const void *p2)

#define INT_LEAST32_TCMP COMPARE_EXTERN COMPARE_INLINE int int_least32_tcmp(const void *p1, const void *p2)

#define INT_FAST32_TCMP COMPARE_EXTERN COMPARE_INLINE int int_fast32_tcmp(const void *p1, const void *p2)

#define UINT_LEAST32_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_least32_tcmp(const void *p1, const void *p2)

#define UINT_FAST32_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_fast32_tcmp(const void *p1, const void *p2)

#define INT_LEAST64_TCMP COMPARE_EXTERN COMPARE_INLINE int int_least64_tcmp(const void *p1, const void *p2)

#define INT_FAST64_TCMP COMPARE_EXTERN COMPARE_INLINE int int_fast64_tcmp(const void *p1, const void *p2)

#define UINT_LEAST64_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_least64_tcmp(const void *p1, const void *p2)

#define UINT_FAST64_TCMP COMPARE_EXTERN COMPARE_INLINE int uint_fast64_tcmp(const void *p1, const void *p2)

COMPARE_DECLARATION(INT_LEAST8_TCMP)
COMPARE_DECLARATION(INT_FAST8_TCMP)
COMPARE_DECLARATION(UINT_LEAST8_TCMP)
COMPARE_DECLARATION(UINT_FAST8_TCMP)

COMPARE_DECLARATION(INT_LEAST16_TCMP)
COMPARE_DECLARATION(INT_FAST16_TCMP)
COMPARE_DECLARATION(UINT_LEAST16_TCMP)
COMPARE_DECLARATION(UINT_FAST16_TCMP)

COMPARE_DECLARATION(INT_LEAST32_TCMP)
COMPARE_DECLARATION(INT_FAST32_TCMP)
COMPARE_DECLARATION(UINT_LEAST32_TCMP)
COMPARE_DECLARATION(UINT_FAST32_TCMP)

COMPARE_DECLARATION(INT_LEAST64_TCMP)
COMPARE_DECLARATION(INT_FAST64_TCMP)
COMPARE_DECLARATION(UINT_LEAST64_TCMP)
COMPARE_DECLARATION(UINT_FAST64_TCMP)
#endif /* C99 */

#if 0
COMPARE_EXTERN COMPARE_INLINE int charcmp(const void *p1, const void *p2);
TYPECMP(char)

COMPARE_EXTERN COMPARE_INLINE int doublecmp(const void *p1, const void *p2);
TYPECMP(double)

COMPARE_EXTERN COMPARE_INLINE int floatcmp(const void *p1, const void *p2);
TYPECMP(float)

COMPARE_EXTERN COMPARE_INLINE int intcmp(const void *p1, const void *p2);
TYPECMP(int)

COMPARE_EXTERN COMPARE_INLINE int shortcmp(const void *p1, const void *p2);
TYPECMP(short)

COMPARE_EXTERN COMPARE_INLINE int longcmp(const void *p1, const void *p2);
TYPECMP(long)

COMPARE_EXTERN COMPARE_INLINE int ulcmp(const void *p1, const void *p2);
TYPECMP(unsigned long)

COMPARE_EXTERN COMPARE_INLINE int size_tcmp(const void *p1, const void *p2);
TYPECMP(size_t)

/* C99 mandatory specific-size types */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
COMPARE_EXTERN COMPARE_INLINE int int_least8_tcmp(const void *p1, const void *p2);
TYPECMP(int_least8_t)

COMPARE_EXTERN COMPARE_INLINE int int_fast8_tcmp(const void *p1, const void *p2);
TYPECMP(int_fast8_t)

COMPARE_EXTERN COMPARE_INLINE int uint_least8_tcmp(const void *p1, const void *p2);
TYPECMP(uint_least8_t)

COMPARE_EXTERN COMPARE_INLINE int uint_fast8_tcmp(const void *p1, const void *p2);
TYPECMP(uint_fast8_t)

COMPARE_EXTERN COMPARE_INLINE int int_least16_tcmp(const void *p1, const void *p2);
TYPECMP(int_least16_t)

COMPARE_EXTERN COMPARE_INLINE int int_fast16_tcmp(const void *p1, const void *p2);
TYPECMP(int_fast16_t)

COMPARE_EXTERN COMPARE_INLINE int uint_least16_tcmp(const void *p1, const void *p2);
TYPECMP(uint_least16_t)

COMPARE_EXTERN COMPARE_INLINE int uint_fast16_tcmp(const void *p1, const void *p2);
TYPECMP(uint_fast16_t)

COMPARE_EXTERN COMPARE_INLINE int int_least32_tcmp(const void *p1, const void *p2);
TYPECMP(int_least32_t)

COMPARE_EXTERN COMPARE_INLINE int int_fast32_tcmp(const void *p1, const void *p2);
TYPECMP(int_fast32_t)

COMPARE_EXTERN COMPARE_INLINE int uint_least32_tcmp(const void *p1, const void *p2);
TYPECMP(uint_least32_t)

COMPARE_EXTERN COMPARE_INLINE int uint_fast32_tcmp(const void *p1, const void *p2);
TYPECMP(uint_fast32_t)

COMPARE_EXTERN COMPARE_INLINE int int_least64_tcmp(const void *p1, const void *p2);
TYPECMP(int_least64_t)

COMPARE_EXTERN COMPARE_INLINE int int_fast64_tcmp(const void *p1, const void *p2);
TYPECMP(int_fast64_t)

COMPARE_EXTERN COMPARE_INLINE int uint_least64_tcmp(const void *p1, const void *p2);
TYPECMP(uint_least64_t)

COMPARE_EXTERN COMPARE_INLINE int uint_fast64_tcmp(const void *p1, const void *p2);
TYPECMP(uint_fast64_t)
#endif /* C99 */
#endif

#define	COMPARE_H_INCLUDED
#endif
