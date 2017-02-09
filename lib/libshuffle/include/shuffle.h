/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring functions defined in shuffle.c
*/
#ifndef	SHUFFLE_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    shuffle.h copyright 2016 Bruce Lilly. \ $
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
/* $Id: ~|^` @(#)   This is shuffle.h version 1.0 dated 2016-03-11T06:09:00Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "shuffle" */
/*****************************************************************************/
/* maintenance note: master file /data/weather/lib/libshuffle/include/s.shuffle.h */

/* version-controlled header file version information */
#define SHUFFLE_H_VERSION "shuffle.h 1.0 2016-03-11T06:09:00Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <stdint.h>             /* uint64_t */
#include <sys/types.h>          /* *_t */

/* function prototypes and data symbol declarations */
#if defined(__cplusplus)
# define SHUFFLE_EXTERN extern "C"
#else
# define SHUFFLE_EXTERN extern
#endif

    /* shuffle.c */
SHUFFLE_EXTERN int fisher_yates_shuffle(void *array, size_t n, size_t size, uint64_t (*randf)(uint64_t), void (*logf)(int, void *, const char *, ...), void *log_arg);

#define	SHUFFLE_H_INCLUDED
#endif
