/* Description: header file declaring function defined in strntol.c
*/
#ifndef	STRNTOL_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    strntol.h copyright 2013 - 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is strntol.h version 2.5 2015-10-09T02:58:26Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "strntol" */
/*****************************************************************************/
/* maintenance note: master file  /src/relaymail/lib/libstrntol/include/s.strntol.h */

/* version-controlled header file version information */
#define STRNTOL_H_VERSION "strntol.h 2.5 2015-10-09T02:58:26Z"

#include <sys/types.h>          /* POSIX standard types */

/* function prototypes and data symbol declarations */
#if defined(__cplusplus)
# define STRNTOL_EXTERN extern "C"
#else
# define STRNTOL_EXTERN extern
#endif

/* *INDENT-OFF* */
/*INDENT OFF*/

/* the function should really be called strntol, but that's reserved... */
STRNTOL_EXTERN long nstrtol(const char *, size_t, char **, int, void (*)(int, void *, const char *, ...), void *);

#define	STRNTOL_H_INCLUDED
#endif
