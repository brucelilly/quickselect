/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file defining function repeated_median_filter
*/
#ifndef	REPEATED_MEDIAN_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    repeated_median.h copyright 2016 Bruce Lilly. \ repeated_median.h $
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
/* $Id: ~|^` @(#)   This is repeated_median.h version 1.2 dated 2016-07-13T22:54:11Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "repeated_median" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.repeated_median.h */

/* version-controlled header file version information */
#define REPEATED_MEDIAN_H_VERSION "repeated_median.h 1.2 2016-07-13T22:54:11Z"

#include <sys/types.h>          /* *_t (size_t) */

#if defined(__cplusplus)
# define REPEATED_MEDIAN_EXTERN extern "C"
#else
# define REPEATED_MEDIAN_EXTERN extern
#endif

/* repeated_median.c */
REPEATED_MEDIAN_EXTERN int repeated_median_filter(double *, double *, double *, size_t, double *, double *, size_t, double *, unsigned int);

#define	REPEATED_MEDIAN_H_INCLUDED
#endif
