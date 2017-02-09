/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring functions defined in random64n.c
*/
#ifndef	RANDOM64N_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    random64n.h copyright 2016 Bruce Lilly. \ random64n.h $
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
/* $Id: ~|^` @(#)   This is random64n.h version 1.1 dated 2016-12-22T17:02:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "random64n" */
/*****************************************************************************/
/* maintenance note: master file /data/weather/lib/librandom/include/s.random64n.h */

/* version-controlled header file version information */
#define RANDOM64N_H_VERSION "random64n.h 1.1 2016-12-22T17:02:04Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* local and system header files needed for declarations */
#include <stdint.h>             /* uint64_t */

/* function prototypes and data symbol declarations */
#if defined(__cplusplus)
# define RANDOM64N_EXTERN extern "C"
#else
# define RANDOM64N_EXTERN extern
#endif

    /* random64n.c */
RANDOM64N_EXTERN int seed_random64n(uint64_t *sarray, unsigned int pinit);
RANDOM64N_EXTERN int save_random64n_state(uint64_t *sarray, unsigned int *pp);
RANDOM64N_EXTERN uint64_t xorshift1024star(void);
RANDOM64N_EXTERN uint64_t random64n(uint64_t n);

#define	RANDOM64N_H_INCLUDED
#endif
