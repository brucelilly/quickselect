/*INDENT OFF*/

/* Description: file to include externally-visible fmed3 function */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    fmed3.c copyright 2019 Bruce Lilly.   \ fmed3.c $
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
/* $Id: ~|^` @(#)   This is fmed3.c version 1.2 dated 2019-03-15T14:08:52Z. \ fmed3.c $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.fmed3.c */

/* Nothing to configure below this line. */

/* gcc handling of "inline" is inconsistent with C11 usage (gcc elides code!) */
#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /**/

/* external visibility */
#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

/* normal code, not C11 _s variant */
#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#define QUICKSELECT_FMED3_VISIBLE_HERE 1

#include "quickselect_config.h"

#include "fmed3_src.h"
