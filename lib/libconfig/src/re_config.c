/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: re_config - read a simple configuration file using a regular expression
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    re_config.c copyright 2011 - 2016 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is re_config.c version 1.13 2016-05-31T14:22:59Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "re_config" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/src/s.re_config.c */

/********************** Long description and rationale: ***********************
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
#define ID_STRING_PREFIX "$Id: re_config.c ~|^` @(#)"
#define SOURCE_MODULE "re_config.c"
#define MODULE_VERSION "1.13"
#define MODULE_DATE "2016-05-31T14:22:59Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2016"

/* configuration (which might affect feature test macros) */
/* to include a main entry point for testing, compile with -DTESTING=1 */
#ifndef TESTING
# define TESTING 0
#endif

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif
#define	LOG_TABLES

#define RE_CONFIG_SPECIAL_FUNCTIONS 1

/* local header files needed */
#include "re_config.h"          /* header file for public definitions and declarations */
#include "indent.h"             /* indent_level */
#include "paths_decl.h"         /* concatenate_path */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files */
#include <ctype.h>              /* isalnum */
#include <errno.h>              /* errno E* */
#include <pthread.h>            /* pthread_rwlock_init pthread_rwlock_rdlock pthread_rwlock_wrlock pthread_rwlock_unlock pthread_rwlock_destroy */
#include <regex.h>              /* regcomp regerror regexec regfree */
#include <stdio.h>              /* FILE fopen */
#include <stdlib.h>             /* NULL calloc */
#include <string.h>             /* strdup strncpy */
#include <syslog.h>             /* LOG_* */
#include <unistd.h>             /* access */

/* static data and function definitions */
static unsigned char re_config_initialized = (unsigned char)0U;
static char filenamebuf[] = __FILE__ ;
static const char *source_file = NULL;

static void initialize_re_config(void)
{
    re_config_initialized
        = register_build_strings(NULL, &source_file, filenamebuf);
}

/* read a simple configuration file into a linked list of configuration entries
   use existing list at root if not NULL (clearing flags and then indicating
      changed values via flags), or allocate a root if needed
   return pointer to root of structure, NULL on error (errno set)
   read name and value using supplied RE pattern in pat, with compilation flags
      cflags and execution flags eflags (see regcomp/regexec documentation)
   report errors and debugging information via f and log_arg
*/
struct config_entry *re_config(const char *path, struct config_entry *root,
    const char *pat, int cflags, int eflags,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    struct config_entry *pc = NULL;
#if ! defined(PP__FUNCTION__)
    static const char *__func__ = "re_config";
#endif

    if ((NULL == path) || ('\0' == *path)
    || (NULL == pat) || ('\0' == *pat)
    || (NULL == f)
    ) {
        errno = EINVAL;
    } else {
        FILE *pf;

        if ((unsigned char)0U == re_config_initialized)
            initialize_re_config();
        /* check arguments: */
        /* file exists at path? */
        if (0 != access(path, F_OK)) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: access: %s does not exist",
                __func__, source_file, __LINE__,
                path);
            return pc;
        }
        /* file can be read? */
        if (0 != access(path, R_OK)) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: access: %s cannot be read",
                __func__, source_file, __LINE__,
                path);
            return pc;
        }
        /* open a stream for the configuration file */
        pf = fopen(path, "r");
        if (NULL == pf) {
            f(LOG_ERR, log_arg,
                "%s: %s line %d: fopen(%s, \"r\"): %m",
                __func__, source_file, __LINE__,
                path);
            return pc;
        } else {
            int c, indent = 0, line = 0, m, max, n;
            unsigned short name_column = 0U, value_column = 0U;
            char *nl, line_buf[BUFSIZ], name[BUFSIZ], value[BUFSIZ];
            regex_t re;
            regmatch_t match[3]; /* line_buf, name, value */
            struct config_entry *parent, *temp;
            pthread_rwlock_t *prwlock;

            /* RE pattern can be compiled? */
            c = regcomp(&re, pat, cflags);
            if (0 != c) {
                (void)regerror(c, &re, line_buf, sizeof(line_buf));
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: regcomp: %s",
                    __func__, source_file, __LINE__,
                    line_buf);
                regfree(&re);
                (void)fclose(pf);
                return pc;
            }
            /* if there's an existing configuration hierarchy, reset flags
               then mark all as deleted (deleted flags are reset when re-read)
            */
            if (NULL != root) {
                /* lock config for writing */
                prwlock = (pthread_rwlock_t *)(root->value);
                n = pthread_rwlock_wrlock(prwlock);
                if (0 != n) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: pthread_rwlock_wrlock() failed: %s",
                        __func__, source_file, __LINE__,
                        strerror(n));
                    switch (n) {
                        case EBUSY:
                        /*FALLTHROUGH*/
                        case EDEADLK:
                            c = pthread_rwlock_unlock(prwlock);
                            if (0 != c) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: pthread_rwlock_unlock() failed: %s",
                                    __func__, source_file, __LINE__,
                                    strerror(c));
                            }
                        break;
                        default:
                        break;
                    }
                    c = pthread_rwlock_destroy(prwlock);
                    if (0 != c) {
                        f(LOG_ERR, log_arg,
                            "%s: %s line %d: pthread_rwlock_destroy() failed: %s",
                            __func__, source_file, __LINE__,
                            strerror(c));
                    }
                    regfree(&re);
                    (void)fclose(pf);
                    errno = n;
                    return pc;
                } else {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: pthread_rwlock_wrlock() succeeded",
                        __func__, source_file, __LINE__);
                }
                (void)reset_config_flags(root, f, log_arg);
                (void)mark_config_subtree_deleted(root, f, log_arg);
            } else {
                /* if there's no existing hierarchy, allocate a root entry */
                root = (struct config_entry *)calloc(1,
                    sizeof(struct config_entry));
                if (NULL == root) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: calloc: %m",
                        __func__, source_file, __LINE__);
                    regfree(&re);
                    (void)fclose(pf);
                    return pc;
                }
                /* put configuration file path in root "name" */
                n = concatenate_path(NULL, path, NULL, name, (int)sizeof(name));
                if (n > (int)sizeof(name)) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: concatenate_path(NULL, \"%s\", NULL) has %d bytes, needs %d",
                        __func__, source_file, __LINE__,
                        path, sizeof(name), n);
                    free(root);
                    regfree(&re);
                    (void)fclose(pf);
                    return pc;
                }
                root->name = strdup(name);
                if (NULL == root->name) {
                    n = errno;
                    free(root);
                    regfree(&re);
                    (void)fclose(pf);
                    errno = n;
                    return pc;
                }
                /* put a pointer to a pthread_rwlock_t in root "value" */
                prwlock = calloc(1, sizeof(pthread_rwlock_t));
                if (NULL == prwlock) {
                    n = errno;
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: calloc(1, %u) for pthread_rwlock_t failed: %m",
                        __func__, source_file, __LINE__,
                        sizeof(pthread_rwlock_t));
                    if (NULL != root->name)
                        free((void *)(root->name));
                    free(root);
                    regfree(&re);
                    (void)fclose(pf);
                    errno = n;
                    return pc;
                }
                root->value = (const char *)prwlock;
                n = pthread_rwlock_init(prwlock, NULL);
                if (0 != n) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: pthread_rwlock_init() failed: %s",
                        __func__, source_file, __LINE__,
                        strerror(n));
                    if (NULL != root->value)
                        free((void *)(root->value));
                    if (NULL != root->name)
                        free((void *)(root->name));
                    free(root);
                    regfree(&re);
                    (void)fclose(pf);
                    errno = n;
                    return pc;
                }
                /* lock config for writing */
                n = pthread_rwlock_wrlock(prwlock);
                if (0 != n) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: pthread_rwlock_wrlock() failed: %s",
                        __func__, source_file, __LINE__,
                        strerror(n));
                    c = pthread_rwlock_destroy(prwlock);
                    if (0 != c) {
                        f(LOG_ERR, log_arg,
                            "%s: %s line %d: pthread_rwlock_destroy() failed: %s",
                            __func__, source_file, __LINE__,
                            strerror(c));
                    }
                    if (NULL != root->value)
                        free((void *)(root->value));
                    if (NULL != root->name)
                        free((void *)(root->name));
                    free(root);
                    regfree(&re);
                    (void)fclose(pf);
                    errno = n;
                    return pc;
                } else {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: pthread_rwlock_wrlock() succeeded",
                        __func__, source_file, __LINE__);
                }
            }
            pc = parent = root;
            /* file is open, RE is compiled, is a root entry, write-locked */
            m = (int)sizeof(match) / (int)sizeof(regmatch_t);
            while (NULL != fgets(line_buf, (int)sizeof(line_buf), pf)) {
                nl = strchr(line_buf, '\n');
                if (NULL != nl) {
                    *nl = '\0';
                    ++line;
                }
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: input line %d: %s",
                    __func__, source_file, __LINE__,
                    line, line_buf);
                if (NULL != nl)
                    *nl = '\n';
                c = regexec(&re, line_buf, (size_t)m, match, eflags);
                if (REG_NOMATCH == c) {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: no match",
                        __func__, source_file, __LINE__);
                } else if (0 == c) {
                    for (max=0,c=1; c<m; c++) {
                        if (match[c].rm_so != -1) {
                            if ('\n' == line_buf[match[c].rm_eo - 1])
                                match[c].rm_eo--;
                            n = (int)(match[c].rm_eo - match[c].rm_so);
                            if ((0 < n) && ('\n' != line_buf[match[c].rm_so])) {
                                f(LOG_DEBUG, log_arg,
                                    "%s: %s line %d: match %d: offset %d through %d: \"%*.*s\"",
                                    __func__, source_file, __LINE__, c, match[c].rm_so,
                                    match[c].rm_eo - 1, n, n, line_buf + match[c].rm_so);
                                switch ((max = c)) {
                                    case 1: /* name */
                                        name_column = (unsigned short)(match[c].rm_so);
                                        indent = indent_level(line_buf, (int)name_column);
                                        f(LOG_DEBUG, log_arg,
                                            "%s: %s line %d: indent level %d",
                                            __func__, source_file, __LINE__, indent);
                                        (void)strncpy(name, line_buf+name_column, (size_t)n);
                                        name[n] = '\0';
                                    break;
                                    case 2: /* value */
                                        value_column = (unsigned short)(match[c].rm_so);
                                        (void)strncpy(value, line_buf+value_column, (size_t)n);
                                        value[n] = '\0';
                                    break;
                                    default: /* ? */
                                    break;
                                }
                            }
                        }
                    }
                    if (0 < max) {
                        while ((parent != root) && (indent < parent->child_indent))
                            parent = parent->parent;
                        temp = add_config_entry(parent, name, max>1? value: NULL, indent, line, name_column, value_column, f, log_arg);
                        if (NULL != temp)
                            parent = temp;
                    }
                } else {
                    (void)regerror(c, &re, line_buf, sizeof(line_buf));
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: regexec: %s",
                        __func__, source_file, __LINE__, line_buf);
                }
            }
            regfree(&re);
            /* close configuration file stream */
            (void)fclose(pf);
            /* remove any items no longer present in configuration file */
            (void)expunge_config(root, f, log_arg);
            /* unlock configuration */
            c = pthread_rwlock_unlock(prwlock);
            if (0 != c) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: pthread_rwlock_unlock() failed: %s",
                    __func__, source_file, __LINE__,
                    strerror(c));
                errno = c;
            } else {
                f(LOG_DEBUG, log_arg,
                    "%s: %s line %d: pthread_rwlock_unlock() succeeded",
                    __func__, source_file, __LINE__);
            }
        }
    }
    return pc;
}
