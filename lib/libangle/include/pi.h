/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring constant and function defined in pi.c
*/
#ifndef	PI_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    pi.h copyright 2009 - 2016 Bruce Lilly.   \ pi.h $
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
/* $Id: ~|^` @(#)   This is pi.h version 3.1420 2016-06-29T18:12:19Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "pi" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libangle/include/s.pi.h */

/* version-controlled header file version information */
#define PI_H_VERSION "pi.h 3.1420 2016-06-29T18:12:19Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* pi.c function declarations */
#if defined(__cplusplus)
# define PI_EXTERN extern "C"
#else
# define PI_EXTERN extern
#endif

PI_EXTERN volatile double pi;
PI_EXTERN void initialize_pi(void);

#define	PI_H_INCLUDED
#endif
