/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring function trim
*/
#ifndef	TRIM_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    trim.h copyright 2016-2017 Bruce Lilly. \ trim.h $
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
/* $Id: ~|^` @(#)   This is trim.h version 1.2 dated 2017-02-07T14:30:21Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "trim" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libgraphing/include/s.trim.h */

/* version-controlled header file version information */
#define TRIM_H_VERSION "trim.h 1.2 2017-02-07T14:30:21Z"

#include <sys/types.h>          /* *_t (size_t) */

#if defined(__cplusplus)
# define TRIM_EXTERN extern "C"
#else
# define TRIM_EXTERN extern
#endif

/* trim.c */
TRIM_EXTERN size_t trim(double *, double *, unsigned char *, size_t, size_t, double);

#define	TRIM_H_INCLUDED
#endif

