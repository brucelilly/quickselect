/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining function quickselect
*/
#ifndef	QUICKSELECT_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    quickselect.h copyright 2016-2017 Bruce Lilly. \ quickselect.h $
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
/* $Id: ~|^` @(#)   This is quickselect.h version 1.11 dated 2017-02-09T16:29:14Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect.h */

/* version-controlled header file version information */
#define QUICKSELECT_H_VERSION "quickselect.h 1.11 2017-02-09T16:29:14Z"

#include <stddef.h>          /* size_t */

#if defined(__cplusplus)
# define QUICKSELECT_EXTERN extern "C"
#else
# define QUICKSELECT_EXTERN extern
#endif

/* qsort wrapper function name */
#ifndef QSORT_FUNCTION_NAME
# define QSORT_FUNCTION_NAME qsort_wrapper
#endif

/* quickselect.c */
QUICKSELECT_EXTERN void quickselect(void *, size_t, const size_t, int (*)(const void *, const void *), size_t *, const size_t);
QUICKSELECT_EXTERN void QSORT_FUNCTION_NAME(void *, size_t, size_t, int (*)(const void *, const void *));

/* QSORT_FUNCTION_NAME remains defined for use in quickselect.c */

#define	QUICKSELECT_H_INCLUDED
#endif
