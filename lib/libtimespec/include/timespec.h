/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring functions defined in timespec.c
*/
#ifndef	TIMESPEC_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    timespec.h copyright 2010 - 2015 Bruce Lilly.   \ timespec.h $
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
/* $Id: ~|^` @(#)   This is timespec.h version 1.7 2016-06-29T20:40:44Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "timespec" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libtimespec/include/s.timespec.h */

/* version-controlled header file version information */
#define TIMESPEC_H_VERSION "timespec.h 1.7 2016-06-29T20:40:44Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <time.h>               /* time_t struct timespec */

#if defined(__cplusplus)
# define TIMESPEC_EXTERN extern "C"
#else
# define TIMESPEC_EXTERN extern
#endif

/* add.c */
TIMESPEC_EXTERN void timespec_add(const struct timespec *, const struct timespec *, struct timespec *);

/* cmp.c */
TIMESPEC_EXTERN int timespec_cmp(const void *, const void *); /* void * to match qsort */

/* copy.c */
TIMESPEC_EXTERN void timespec_copy(const struct timespec *, struct timespec *);

/* double_to.c */
TIMESPEC_EXTERN void double_to_timespec(double, struct timespec *);

/* normalize_near.c */
TIMESPEC_EXTERN void normalize_timespec_nearest(struct timespec *);

/* normalize_nonneg.c */
TIMESPEC_EXTERN void normalize_timespec_nonnegative(struct timespec *);

/* round.c */
TIMESPEC_EXTERN void round_timespec(struct timespec *, long);

/* subtract.c */
TIMESPEC_EXTERN void timespec_subtract(const struct timespec *, const struct timespec *, struct timespec *);

/* to_double.c */
TIMESPEC_EXTERN double timespec_to_double(const struct timespec *);

/* tmd_to.c */
TIMESPEC_EXTERN void tmd_to_timespec(struct tm *, double *, struct timespec *);

/* to_tmd.c */
TIMESPEC_EXTERN void timespec_to_tmd(struct timespec *, struct tm *, double *);

#define	TIMESPEC_H_INCLUDED
#endif
