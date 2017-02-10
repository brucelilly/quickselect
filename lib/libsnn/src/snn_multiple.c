/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: snn_multiple - snmultiple
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    snn_multiple.c copyright 2011 - 2016 Bruce Lilly.   \ snn_multiple.c $
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
/* $Id: ~|^` @(#)   This is snn_multiple.c version 2.6 dated 2016-04-04T05:29:37Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file /src/relaymail/lib/libsnn/src/s.snn_multiple.c */
#error OBSOLETE: code is now in snn.h

/********************** Long description and rationale: ***********************
* snmultiple returns the nearest multiple of a specified increment to a
*  specified floating-point number.  If the increment is 1.0, this is equivalent
*  to snround.  However, it can also be used to provide the nearest multiple
*  of 10.0, of 0.1, of 25.4, etc.:
*
*  double snmultiple(double d, double incr)
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: snn_multiple.c ~|^` @(#)"
#define SOURCE_MODULE "snn_multiple.c"
#define MODULE_VERSION "2.6"
#define MODULE_DATE "2016-04-04T05:29:37Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2016"

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files */
#include "snn.h"      /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#ifndef NULL
# include <stdlib.h>            /* NULL */
#endif
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char snn_multiple_initialized= (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

/* initialize snn_almost_one, etc. at run-time */
static void initialize_snn_multiple(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    snn_multiple_initialized = register_build_strings(NULL, &source_file, s);
}

/* return d expressed as the nearest integral multiple of incr */
/* calls: initialize_snn_multiple, snround */
/* called by: no other snn functions */
double snmultiple(double d, double incr,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "snmultiple";
#endif

    if ((unsigned char)0U == snn_multiple_initialized)
        initialize_snn_multiple();
    if (NULL != f) {
        f(LOG_DEBUG, log_arg,
            "%s: %s line %d: %s(%.24g, %.24g)",
            __func__, source_file, __LINE__,
            __func__, d, incr
            );
    }
    if (0.0 > incr) incr = 0.0 - incr;
    if (0.0 == incr) return d;
    return incr * snround(d / incr, f, log_arg);
}
