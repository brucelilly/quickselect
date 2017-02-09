/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file declaring functions defined in paths.c
*/
#ifndef	PATHS_DECL_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    paths_decl.h copyright 2010 - 2016 Bruce Lilly.   \ paths_decl.h $
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
/* $Id: ~|^` @(#)   This is paths_decl.h version 1.6 dated 2016-04-07T00:29:44Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "paths" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libpaths/include/s.paths_decl.h */

/* version-controlled header file version information */
#define PATHS_DECL_H_VERSION "paths_decl.h 1.6 2016-04-07T00:29:44Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* paths.c function declarations */
#if defined(__cplusplus)
# define PATHS_EXTERN extern "C"
#else
# define PATHS_EXTERN extern
#endif

/* concatenate_path.c */
PATHS_EXTERN int concatenate_path(const char *, const char *, const char *, char *, int);

/* expand_path.c */
PATHS_EXTERN int expand_path(const char *, char *, int);

/* is_dir.c */
PATHS_EXTERN int is_dir(const char *);

/* normalize_path.c */
PATHS_EXTERN int normalize_path(const char *, char *, int);

/* path_basename.c */
PATHS_EXTERN int path_basename(const char *, char *, int);

/* path_dirname.c */
PATHS_EXTERN int path_dirname(const char *, char *, int);

/* path_separator.c */
PATHS_EXTERN const char *path_separator(const char *);

/* relative_path.c */
PATHS_EXTERN int relative_path(const char *, const char *, char *, int);

#define	PATHS_DECL_H_INCLUDED
#endif
