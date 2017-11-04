/* *INDENT-OFF* */
/*INDENT OFF*/
#ifndef	SIMPLE_CONFIG_H_INCLUDED
/* Description: header file for simple_config
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    simple_config.h copyright 2011-2017 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is simple_config.h version 1.13 2017-09-21T16:32:27Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "simple_config" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/include/s.simple_config.h */

/* version-controlled header file version information */
#define SIMPLE_CONFIG_H_VERSION "simple_config.h 1.13 2017-09-21T16:32:27Z"

/*INDENT ON*/
/* *INDENT-ON* */

/* system header file needed for declarations */
# include <sys/types.h>         /* size_t */

/* macros used by code */
#define CONFIG_FLAG_ENTRY_STATIC  's'
#define CONFIG_FLAG_ENTRY_NEW     'n'
#define CONFIG_FLAG_ENTRY_DELETED 'd'
#define CONFIG_FLAG_VALUE_CHANGED 'c'
#define CONFIG_FLAG_VALUE_WRONG   'x'

/* structure used by simple_config code */
struct config_entry {
    const char *name;           /* configuration file path in root node */
    const char *value;          /* pointer to pthread_rwlock_t in root node */
    char *flags;                /* timestamp in root node */
    void *usrptr;               /* application data (unused, unmaintained) */
    int child_indent;           /* indent level of child nodes */
    int line;                   /* line number in configuration file (1-based) */
    unsigned short name_column; /* name column number (1-based) on line of configuration file */
    unsigned short value_column;/* value column number (1-based) on line of configuration file */
    size_t nchildren;           /* number of child nodes in configuration hierarchy */
    struct config_entry **children;     /* array of pointers to child nodes */
    struct config_entry *parent;/* NULL indicates root node */
};

/* simple_config.c function declarations */
#if defined(__cplusplus)
# define SIMPLE_CONFIG_EXTERN extern "C"
#else
# define SIMPLE_CONFIG_EXTERN extern
#endif

SIMPLE_CONFIG_EXTERN int configcmp(const void *, const void *);
SIMPLE_CONFIG_EXTERN struct config_entry *find_config_entry(struct config_entry *, char *const[], int, int (*)(const void *, const void *), void (*)(int, void *, const char *, ...), void *);
SIMPLE_CONFIG_EXTERN int top_down_config_walk(struct config_entry *, int (*)(struct config_entry *, void (*)(int, void *, const char *, ...), void *), void (*)(int, void *, const char *, ...), void *);
SIMPLE_CONFIG_EXTERN int free_config(struct config_entry *, void (*)(int, void *, const char *, ...), void *);

#if RE_CONFIG_SPECIAL_FUNCTIONS
/* special functions called by re_config: not intended for general use */
SIMPLE_CONFIG_EXTERN struct config_entry *add_config_entry(struct config_entry *, const char *, const char *, int, int, unsigned int, unsigned int, void (*)(int, void *, const char *, ...), void *);
SIMPLE_CONFIG_EXTERN int expunge_config(struct config_entry *, void (*)(int, void *, const char *, ...), void *);
SIMPLE_CONFIG_EXTERN int reset_config_flags(struct config_entry *, void (*)(int, void *, const char *, ...), void *);
SIMPLE_CONFIG_EXTERN int mark_config_subtree_deleted(struct config_entry *, void (*)(int, void *, const char *, ...), void *);
#endif

#define	SIMPLE_CONFIG_H_INCLUDED
#endif
