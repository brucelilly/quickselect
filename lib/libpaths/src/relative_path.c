/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: C source code for handling file and directory paths
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    relative_path.c copyright 2010 - 2016 Bruce Lilly.   \ relative_path.c $
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
/* $Id: ~|^` @(#)   This is relative_path.c version 1.6 dated 2016-04-07T00:32:35Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/src/s.relative_path.c */

/********************** Long description and rationale: ***********************
* int relative_path(const char *dir, const char *path, char *buf, int sz)
* put full path of given path relative to dir in supplied buffer
*   put result in buf (if not NULL) of size sz (if large enough)
*   return length of result if buf is not NULL and is large enough,
*     or size required for result if buf is NULL or is too small
*   return -1 on error
*   neither dir nor path are checked for existence or modified
******************************************************************************/

/* source module identification */
/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: relative_path.c ~|^` @(#)"
#define SOURCE_MODULE "relative_path.c"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2016-04-07T00:32:35Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2010 - 2016"

/* configuration (which might affect feature test macros) */

/* feature test macros must appear before any header file inclusion */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files needed */
#include "paths_decl.h"         /* concatenate_path expand_path normalize_path relative_path */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <errno.h>              /* errno */
#include <limits.h>             /* *_MAX */
#include <stdlib.h>             /* NULL */
#include <string.h>             /* strrchr */

/* static data and function definitions */
static char paths_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_paths(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    paths_initialized = register_build_strings(NULL, &source_file, s);
}

/* put full path of given path relative to dir in supplied buffer
   put result in buf (if not NULL) of size sz (if large enough)
   return length of result if buf is not NULL and is large enough,
     or size required for result if buf is NULL or is too small
   return -1 on error
   neither dir nor path are checked for existence or modified
*/
int relative_path(const char *dir, const char *path, char *buf, int sz)
{
    char *p, dirbuf[PATH_MAX], tempbuf[PATH_MAX];
    int r, ret;

    if (0U == paths_initialized)
        initialize_paths();
    if (((NULL == dir) || ('\0' == *dir)) && (NULL != path))
       return expand_path(path, buf, sz);
    if ((NULL != dir) && ((NULL == path) || ('\0' == *path)))
       return expand_path(dir, buf, sz);
    ret = expand_path(dir, tempbuf, sizeof(tempbuf));
    if ((0 > ret) || ((int)sizeof(tempbuf) < ret)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    r = normalize_path(tempbuf, p = dirbuf, sizeof(dirbuf));
    if ((int)sizeof(dirbuf) < r) {
        errno = ENAMETOOLONG;
        return -1;
    }
    ret = concatenate_path(p, path, NULL, tempbuf, sizeof(tempbuf));
    if ((0 > ret) || ((int)sizeof(tempbuf) < ret)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    r = normalize_path(tempbuf, p = dirbuf, sizeof(dirbuf));
    if ((int)sizeof(dirbuf) < r) {
        errno = ENAMETOOLONG;
        return -1;
    }
    return expand_path(p, buf, sz);
}
