/*INDENT OFF*/

/* Description: C source code for selection-related development */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    partition.c copyright 2016-2018 Bruce Lilly.   \ partition.c $
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
/* $Id: ~|^` @(#)   This is partition.c version 1.10 dated 2018-02-14T09:05:28Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.partition.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: partition.c ~|^` @(#)"
#define SOURCE_MODULE "partition.c"
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2018-02-14T09:05:28Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* assertions for validation testing */
#ifndef ASSERT_CODE
# define ASSERT_CODE                     0 /* Adds size & cost to aid debugging.
                                              0 for tested production code. */
                                           /* If ASSERT_CODE > 1, assertions
                                              might also affect the number of
                                              comparisons used.
                                           */
#endif

#define QUICKSELECT_BUILD_FOR_SPEED 0
#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /* */

#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /* */

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* one source */
#include "partition_src.h"
