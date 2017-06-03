/* *INDENT-OFF* */
/*INDENT OFF*/
#ifndef	RE_CONFIG_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    re_config.h copyright 2011 - 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is re_config.h version 1.6 2015-10-05T20:27:37Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "re_config" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/include/s.re_config.h */

/* version-controlled header file version information */
#define RE_CONFIG_H_VERSION "re_config.h 1.6 2015-10-05T20:27:37Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files needed */
#include "simple_config.h"      /* struct config_entry */

/* re_config.c function declarations */
#if defined(__cplusplus)
# define RE_CONFIG_EXTERN extern "C"
#else
# define RE_CONFIG_EXTERN extern
#endif

RE_CONFIG_EXTERN struct config_entry *re_config(const char *, struct config_entry *, const char *, int, int, void (*)(int, void *, const char *, ...), void *);

#define	RE_CONFIG_H_INCLUDED
#endif
