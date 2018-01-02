/*INDENT OFF*/
#ifndef	CUTOFF_TABLE_H_INCLUDED

/* Description: type declarations for quickselect tables */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    cutoff_table.h 1.2 copyright 2017 Bruce Lilly.   \ cutoff_table.h $
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
/* $Id: ~|^` @(#)   This is cutoff_table.h version 1.2 dated 2017-12-22T04:14:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.cutoff_table.h */

/********************** Long description and rationale: ***********************
 Dedicated sorting is used for small sub-arrays, where "small" depends on the
 ratio of element size to the basic type size used for data movementm and on
 options specified, such as optimization for complex comparisosns or
 preservation of partial order stability.  This header file defines a structure
 used for tables which map size ratio to cutoff size (number of elements) for
 use of dedicated sorting.
******************************************************************************/

/* Nothing to configure below this line. */

/* version-controlled header file version information */
#define CUTOFF_TABLE_H_VERSION "cutoff_table.h 1.2 2017-12-22T04:14:04Z"

/* system header files */
#include <stddef.h>             /* size_t */

/* data structures */
struct cutoff_table_struct {
    size_t min_ratio;           /* smallest ratio of size to alignment_size */
    size_t cutoff;              /* largest array sorted by dedicated_sort */
};

#define	CUTOFF_TABLE_H_INCLUDED
#endif
