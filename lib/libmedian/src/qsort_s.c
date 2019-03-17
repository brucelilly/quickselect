/*INDENT OFF*/

/* Description: C source code for qsort */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    qsort_s.c copyright 2017-2019 Bruce Lilly.   \ qsort_s.c $
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
/* $Id: ~|^` @(#)   This is qsort_s.c version 1.5 dated 2019-03-15T14:08:53Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.qsort_s.c */

/********************** Long description and rationale: ***********************
 This file contains source to implement the public function qsort_s.
 It is based on an implementation of multiple quickselect similar to that
 described by Mahmoud & Lent in "Average-case analysis of multiple Quickselect:
 An algorithm for finding order statistics".  The array of items to be sorted by
 quickselect, the array element size, and the comparison function are declared
 as for qsort, with two additional parameters to specify an optional array of
 desired order statistic ranks.
 N.B. the values for the order-statistic array elements are zero-based ranks
 (i.e. 0UL for the minimum, N-1UL for the maximum of an array of N elements,
 (N-1UL)/2UL and N/2UL for the lower- and upper-medians).
 The implementation can also be used to implement qsort; a wrapper function
 (which could alternatively be implemented as a macro) is provided for that
 purpose.
******************************************************************************/

/* Nothing to configure below this line. */

#define __STDC_WANT_LIB_EXT1__ 1

#include "quickselect_config.h"

#include "qsort_src.h"          /* contains the actual source code */
