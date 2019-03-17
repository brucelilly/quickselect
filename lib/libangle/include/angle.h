/* Description: header file declaring functions defined in angle.c
*/
#ifndef	ANGLE_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    angle.h copyright 2010 - 2016 Bruce Lilly.   \ angle.h $
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
/* $Id: ~|^` @(#)   This is angle.h version 0.5 2016-06-29T18:11:29Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "angle" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libangle/include/s.angle.h */

/* version-controlled header file version information */
#define ANGLE_H_VERSION "angle.h 0.5 2016-06-29T18:11:29Z"

/* local and system header files */
#include "pi.h"                 /* pi initialize_pi */

/* local macros and structure definitions */
#ifdef PI_DEG
# undef PI_DEG
#endif
#define PI_DEG 180.0            /* pi radians as degrees */

/* angle.c function declarations */
#if defined(__cplusplus)
# define ANGLE_EXTERN extern "C"
#else
# define ANGLE_EXTERN extern
#endif

/* inline functions */

#if INCLUDE_EXTERN_DECLARATION
# if defined(__STDC__) && __STDC_VERSION__ > 199900UL
ANGLE_EXTERN double degrees_to_radians(double);
ANGLE_EXTERN double radians_to_degrees(double);
ANGLE_EXTERN double dms_to_degrees(double, double, double);
# endif
#endif

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
double degrees_to_radians(double deg)
{
    initialize_pi();
    return deg / PI_DEG * pi;
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
double radians_to_degrees(double rad)
{
    initialize_pi();
    return rad / pi * PI_DEG;
}

#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline /* only applicable for C99 et. seq. */
#else
static /* prevent multiple definition conflicts */
#endif
double dms_to_degrees(double d, double m, double s)
{
    return d + m / 60.0 + s / 3600.0;
}

#define	ANGLE_H_INCLUDED
#endif
