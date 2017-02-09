/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for get_host_name
*/
#ifndef GET_HOST_NAME_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    get_host_name.h copyright 2010 - 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is get_host_name.h version 1.1 2015-10-21T02:20:39Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "get_host_name" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libid/include/s.get_host_name.h */

/* version-controlled header file version information */
#define GET_HOST_NAME_H_VERSION "get_host_name.h 1.1 2015-10-21T02:20:39Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* get_host_name.c function declarations */
#if defined(__cplusplus)
# define GET_HOST_NAME_EXTERN extern "C"
#else
# define GET_HOST_NAME_EXTERN extern
#endif

GET_HOST_NAME_EXTERN char *get_host_name(char *, int, void (*)(int, void *, const char *, ...), void *);

/* *INDENT-OFF* */
/*INDENT OFF*/

#define GET_HOST_NAME_H_INCLUDED
#endif
