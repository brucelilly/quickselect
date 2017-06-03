/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for indent.c
*/
#ifndef	INDENT_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    indent.h copyright 2011 - 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is indent.h version 1.4 2015-10-05T20:27:37Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "indent" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/include/s.indent.h */

/* version-controlled header file version information */
#define INDENT_H_VERSION "indent.h 1.4 2015-10-05T20:27:37Z"

/*INDENT ON*/
/* *INDENT-ON* */

#if defined(__cplusplus)
# define INDENT_EXTERN extern "C"
#else
# define INDENT_EXTERN extern
#endif

/* indent.c function declarations */
INDENT_EXTERN int indent_level(const char *, int);

#define	INDENT_H_INCLUDED
#endif
