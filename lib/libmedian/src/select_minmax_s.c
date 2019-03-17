/*INDENT OFF*/

/* Description: file to include externally-visible select_minmax_s function */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    select_minmax_s.c copyright 2019 Bruce Lilly.   \ select_minmax_s.c $
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
/* $Id: ~|^` @(#)   This is select_minmax_s.c version 1.1 dated 2019-03-06T21:12:50Z. \ select_minmax_s.c $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.select_minmax_s.c */

/* Nothing to configure below this line. */

#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /**/

#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1

#include "quickselect_config.h"

#include "select_minmax_src.h"
