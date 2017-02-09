#ifndef	GRAPHING_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    graphing.h copyright 2014 - 2015 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is graphing.h version 2.1 2015-10-06T18:03:16Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "graphing" */
/*****************************************************************************/
/* maintenance note: master file  /data/projects/automation/940/lib/libgraphing/include/s.graphing.h */

/* version-controlled header file version information */
#define GRAPHING_H_VERSION "graphing.h 2.1 2015-10-06T18:03:16Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <stdio.h>              /* FILE printf fprintf vfprintf */

#define COMMENTSTART ".\\\""
#define COMMENTEND "\n"

#if defined(__cplusplus)
# define GRAPHING_EXTERN extern "C"
#else
# define GRAPHING_EXTERN extern
#endif

    /* graphing.c */
GRAPHING_EXTERN int formatprecision(FILE *, double, double, double);
GRAPHING_EXTERN int formatwidth(FILE *, double, double, int);
GRAPHING_EXTERN double intvl(FILE *, double, double, double, double, double, int, unsigned int);
GRAPHING_EXTERN double mintick(FILE *, double, double, double, double, double, double);
GRAPHING_EXTERN double maxtick(FILE *, double, double, double, double, double, double);
GRAPHING_EXTERN int prec(FILE *, double);
GRAPHING_EXTERN double rem(FILE *, double, double, double);

#define	GRAPHING_H_INCLUDED
#endif
