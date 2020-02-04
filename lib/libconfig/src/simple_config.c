/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: simple_config - read simple configuration file
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    simple_config.c copyright 2011-2020 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is simple_config.c version 2.8 2020-01-05T14:38:58Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "simple_config" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libconfig/src/s.simple_config.c */

/********************** Long description and rationale: ***********************
* General simple configuration file reading and configuration parameter lookup.
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
#define ID_STRING_PREFIX "$Id: simple_config.c ~|^` @(#)"
#define SOURCE_MODULE "simple_config.c"
#define MODULE_VERSION "2.8"
#define MODULE_DATE "2020-01-05T14:38:58Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011-2020"

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

/* compile-time option for debugging code */
#define SIMPLE_CONFIG_DEBUG_RWLOCK 1

/*INDENT ON*/
/* *INDENT-ON* */

/* include declarations for semi-private functions */
#define RE_CONFIG_SPECIAL_FUNCTIONS 1

/* local header files needed */
#include "simple_config.h"      /* public definitions and declarations */ /* includes sys/types.h */
#include "civil_time.h"         /* sn_civil_time */ /* includes time.h (clock_gettime CLOCK_REALTIME) */
#include "sorted_array.h"       /* arrayfind arrayinsert */ /* includes sys/types.h */
#include "utc_mktime.h"         /* local_utc_offset */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files */
#include <sys/stat.h>           /* open */
#include <ctype.h>              /* isalnum */
#include <errno.h>              /* errno EINVAL */
#include <fcntl.h>              /* open */
#include <limits.h>             /* *_MAX */
#include <pthread.h>            /* pthread_rwlock_init pthread_rwlock_rdlock pthread_rwlock_wrlock pthread_rwlock_unlock pthread_rwlock_destroy */
#include <stdlib.h>             /* NULL atoi strtol malloc calloc realloc free */
#include <string.h>             /* strcasecmp (some systems) strdup strchr strncpy */
#include <strings.h>            /* strcasecmp (some systems) */
#include <syslog.h>             /* LOG_* */
#include <time.h>               /* clock_gettime CLOCK_REALTIME struct timespec */
#include <sys/time.h>           /* CLOCK_REALTIME (e.g. on FreeBSD) */
#include <unistd.h>             /* access close unlink */

/* local macros and structure definitions */
#define FLAGS_BUFFER_SIZE     2
#define TIMESTAMP_BUFFER_SIZE 48

static unsigned char simple_config_initialized = (unsigned char)0U;
static char filenamebuf[] = __FILE__ ;
static const char *source_file = NULL;

/* private function definitions */
static void initialize_simple_config(void)
{
    simple_config_initialized
        = register_build_strings(NULL, &source_file, filenamebuf);
}

/* execute func for each entry in the hierarchy below pc, starting
     at the bottom and working back up to pc
*/
static int depth_first_config_walk(struct config_entry *pc,
    int (*func)(struct config_entry *, void (*)(int, void *, const char *, ...), void *),
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "depth_first_config_walk";
#endif
    if (NULL != pc) {
        int i;

        /* process children first */
        if (NULL != pc->children) {
            size_t c;

            /* start with highest index so delete works */
            for (c=pc->nchildren; 0U!=c; ) {
                i = depth_first_config_walk(pc->children[--c], func, f, log_arg);
                if (0 > i)
                    return i;
            }
        }
        /* process start entry */
        i = func(pc, f, log_arg);
        return i;
    }
    errno = EINVAL;
    return -1;
}

/* test configuration entry flag
   return 0 if not set
   return >0 if set
   return <0 on error
*/
static int config_entry_flag_is_set(struct config_entry *pc, int flag)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "config_entry_flag_is_set";
#endif
    if (NULL != pc) {
        if (NULL != pc->flags) {
           if (pc->flags[0] == flag)
               return 1;
        }
        return 0;
    }
    errno = EINVAL;
    return -1;
}

/* free allocated memory for a configuration entry
*/
static int free_config_entry(struct config_entry *pc,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "free_config_entry";
#endif
    if (NULL != pc) {
        /* free storage for children array */
        /* children must have been freed, else memory will leak */
        if (0U == pc->nchildren) {
            if (NULL != pc->children) {
                free((void *)(pc->children));
                pc->children = NULL;
            }
            pc->nchildren = 0U;
        }
        /* free storage for flags, if any */
        if (NULL != pc->flags) {
            free((void *)(pc->flags));
            pc->flags = NULL;
        }
        /* free storage for name */
        if (NULL != pc->name) {
            free((void *)(pc->name));
            pc->name = NULL;
        }
        /* free storage for value, if any */
        if (NULL != pc->value) {
            if (NULL == pc->parent) { /* special case for root entry */
                int c;
                pthread_rwlock_t *prwlock;

                prwlock = (pthread_rwlock_t *)(pc->value);
                errno = 0;
                c = pthread_rwlock_unlock(prwlock);
                if (0 != c) {
                    errno = c;
#if SIMPLE_CONFIG_DEBUG_RWLOCK
                } else {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: pthread_rwlock_unlock() succeeded",
                        __func__, source_file, __LINE__);
#endif
                }
                c = pthread_rwlock_destroy(prwlock);
                if (0 != c) {
                    errno = c;
                }
            }
            free((void *)(pc->value));
            pc->value = NULL;
        }
        if (NULL != pc->parent) { /* not for root entry */
            if (NULL != pc->usrptr) {
                f(LOG_WARNING, log_arg,
                    "%s: %s line %d: usrptr is not NULL",
                    __func__, source_file, __LINE__);
            }
        }
        /* free storage for structure */
        free((void *)pc);
        return 0;
    }
    errno = EINVAL;
    return -1;
}

/* if entry is marked for deletion, free its descendants and it
   and remove it from the parent's list of children
*/
static int expunge_config_entry(struct config_entry *pc,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "expunge_config_entry";
#endif
    if (NULL != pc) {
        struct config_entry **children = pc->children;
        size_t c;
        int deleted = config_entry_flag_is_set(pc, (int)CONFIG_FLAG_ENTRY_DELETED);
        int i;

        /* always check descendants first (root itself is never marked for deletion) */
        /* no orphans! process descendants first so free works correctly */
        if (NULL != children) {
            for (c=pc->nchildren; 0U!=c; ) {
                --c;
                if (0 < deleted) {
                    i = mark_config_subtree_deleted(children[c], f, log_arg); /* make sure... */
                    if (0 > i)
                        return i;
                }
                i = expunge_config_entry(children[c], f, log_arg);
                if (0 > i)
                    return i;
            }
        }
        if (0 < deleted) {
            struct config_entry *parent = pc->parent;

            /* remove from parent's list of children */
            if (NULL != parent) {
                size_t n;

                for (children=parent->children,c=n=parent->nchildren; 0U!=c; ) {
                    if (children[--c] == pc) {
                        for (--n; c<n; c++)
                            children[c] = children[c+1];
                        children[n] = NULL;
                        parent->nchildren = n;
                        if (0U == n) {
                            free((void *)children);
                            parent->children = NULL;
                        }
                        break;
                    }
                }
                (void) free_config_entry(pc, f, log_arg);
            }
        }
        return 0;
    }
    errno = EINVAL;
    return -1;
}

/* set timestamp string in root entry "flags" string
   return 0 on success
   return <0 on error, errno set
*/
static int timestamp_config(struct config_entry *pc,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "timestamp_config";
#endif
    if (NULL != pc) {
        int r, r2;
        char *p;

        /* find root */
        while (NULL != pc->parent)
            pc = pc->parent;
        /* make buffer if there isn't one */
        r2 = TIMESTAMP_BUFFER_SIZE;
        if (NULL == pc->flags) {
            pc->flags = (char *)calloc(r2, sizeof(char));
            if (NULL == pc->flags)
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: calloc(%d, %d) returned NULL: %m",
                    __func__, source_file, __LINE__,
                    r2, sizeof(char));
        } else { /* make sure existing buffer is large enough */
            p = (char *)realloc(pc->flags, r2 * sizeof(char));
            if (NULL == p) {
                free(pc->flags);
                pc->flags = (char *)calloc(r2, sizeof(char));
                if (NULL == pc->flags)
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: calloc(%d, %d) returned NULL: %m",
                        __func__, source_file, __LINE__,
                        r2, sizeof(char));
            } else {
                pc->flags = p;
            }
        }
        if (NULL == pc->flags) {
            errno = ENOMEM;
            return -1;
        } else {
            struct timespec ts;

            r = clock_gettime(CLOCK_REALTIME, &ts);
            if (0 == r) {
                struct civil_time_struct cts;

                /* convert to struct tm plus doubles for sn_civil_time */
                (void)localtime_r(&(ts.tv_sec), &(cts.tm));
                cts.fraction = 1.0e-9 * ts.tv_nsec;
                cts.offset = 1.0 * local_utc_offset(-1, NULL, NULL);
                /* write timestamp string */
                r = sn_civil_time(pc->flags, r2, &cts, -15, 1, 0, NULL, NULL);
                if (0 > r)
                    return r; /* errno set by sn_civil_time */
                while (r2 < r) { /* buffer is too small */
                    r2 = r;
                    p = (char *)realloc(pc->flags, r2 * sizeof(char));
                    if (NULL == p) {
                        free(pc->flags);
                        pc->flags = (char *)calloc(r2, sizeof(char));
                        if (NULL == pc->flags) {
                            errno = ENOMEM;
                            return -1;
                        }
                    }
                    r = sn_civil_time(pc->flags=p, r2, &cts, -15, 1, 0, NULL, NULL);
                    if (0 > r)
                        return r; /* errno set by sn_civil_time */
                }
                return 0; /* done */
            }
        }
    } else
        errno = EINVAL;
    return -1;
}

/* set configuration entry flag and return pointer to flags string */
static char *set_config_entry_flag(struct config_entry *pc, int flag,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "set_config_entry_flag";
#endif
    if (NULL != pc) {
        if ((unsigned char)0U == simple_config_initialized)
            initialize_simple_config();
        if (NULL != pc->parent) { /* root flags is actually timestamp */
            if (NULL == pc->flags) {
                pc->flags = (char *)calloc(FLAGS_BUFFER_SIZE, sizeof(char));
                if (NULL == pc->flags)
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: calloc(%d, %d) returned NULL: %m",
                        __func__, source_file, __LINE__,
                        FLAGS_BUFFER_SIZE, sizeof(char));
            }
            if (NULL != pc->flags) {
                switch (flag) {
                    case CONFIG_FLAG_ENTRY_STATIC :
                    /*FALLTHROUGH*/
                    case CONFIG_FLAG_ENTRY_NEW :
                    /*FALLTHROUGH*/
                    case CONFIG_FLAG_ENTRY_DELETED :
                    /*FALLTHROUGH*/
                    case CONFIG_FLAG_VALUE_CHANGED :
                    /*FALLTHROUGH*/
                    case CONFIG_FLAG_VALUE_WRONG :
                        pc->flags[0] = (char)flag;
                    break;
                    default:
                        errno = EINVAL;
                    return NULL;
                }
                return pc->flags;
            }
        } else {
            if (NULL == pc->flags) {
                pc->flags = (char *)calloc(TIMESTAMP_BUFFER_SIZE, sizeof(char));
                if (NULL == pc->flags)
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: calloc(%d, %d) returned NULL: %m",
                        __func__, source_file, __LINE__,
                        TIMESTAMP_BUFFER_SIZE, sizeof(char));
            }
            return pc->flags;
        }
    } else
        errno = EINVAL;
    return NULL;
}

/* execute func for each entry in the hierarchy below pc, starting
     at the top and working down
*/
static int process_down(struct config_entry *pc,
    int (*func)(struct config_entry *, void (*)(int, void *, const char *, ...), void *),
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "process_down";
#endif
    if (NULL != pc) {
        int i;

        /* process parent first */
        i = func(pc, f, log_arg);
        /* process children next */
        if (NULL != pc->children) {
            int line, did, x;
            size_t c, nc, next;

            /* start with lowest line number */
            did = INT_MIN;
            nc = pc->nchildren;
            do {
                /* each pass: find and process child with lowest line number not yet processed */
                next = nc;
                line = INT_MAX;
                for (c=0U; c<nc; c++) {
                    x = pc->children[c]->line;
                    if ((x > did) && (x < line)) {
                        line = x;
                        next = c;
                    }
                }
                if (next < nc) {
                    i = process_down(pc->children[next], func, f, log_arg);
                    if (0 > i)
                        return i;
                    did = line;
                }
            } while (next < nc);
        }
        return i;
    }
    errno = EINVAL;
    return -1;
}

/* reset entry's flags */
static int reset_config_entry_flags(struct config_entry *pc,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "reset_config_entry_flags";
#endif
    if (NULL != pc) {
        if (NULL == pc->parent) /* root flags is actually timestamp */
            return 0;
        if (NULL != set_config_entry_flag(pc, (int)CONFIG_FLAG_ENTRY_STATIC, f, log_arg))
            return 0;
    } else
        errno = EINVAL;
    return -1;
}

/* public function definitions */

/* case-insensitive configuration entry name comparison */
int configcmp(const void *p1, const void *p2)
{
    const struct config_entry *c1 = p1, *c2 = p2;
#ifndef PP__FUNCTION__
    static const char __func__[] = "configcmp";
#endif

    return strcasecmp(c1->name, c2->name);
}

/* search starting at root for configuration entry with
      successive names (nnames in array) using compar
      to compare configuration entries
   return a NULL pointer on error or if no entry is found
   on success, return a pointer to the matching entry
*/
struct config_entry *find_config_entry(struct config_entry *root,
    char * const names[] /* like getopt(), see XPG getopt() and exec() */,
    int nnames, int (*compar)(const void *, const void *),
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    struct config_entry *pc = NULL;
#ifndef PP__FUNCTION__
    static const char __func__[] = "find_config_entry";
#endif

    if ((NULL == root) || (NULL == names) || (0 >= nnames) || (NULL == compar)) {
        if (NULL != f)
            f(LOG_ERR, log_arg,
                "%s: %s line %d: %s(0x%x, 0x%x, %d, 0x%x, ...) argument error",
                __func__, source_file, __LINE__,
                __func__, root, names, nnames, compar);
        errno = EINVAL;
    } else {
        int c, n;
        size_t l;
        struct config_entry temp; /* structure required for comparison */
        pthread_rwlock_t *prwlock;

        if ((unsigned char)0U == simple_config_initialized)
            initialize_simple_config();
        /* acquire read lock on configuration */
        prwlock = (pthread_rwlock_t *)(root->value);
        if (NULL == prwlock) {
            if (NULL != f)
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: NULL rwlock pointer",
                    __func__, source_file, __LINE__);
            errno = EINVAL;
            return pc;
        }
        errno = 0;
        c = pthread_rwlock_rdlock(prwlock);
        if (0 != c) {
            if (NULL != f)
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: pthread_rwlock_rdlock returned %d: %s",
                    __func__, source_file, __LINE__,
                    c, strerror(c));
            switch (c) {
                case EBUSY:
                /*FALLTHROUGH*/
                case EDEADLK:
                    c = pthread_rwlock_unlock(prwlock);
                    if (0 != c) {
                        if (NULL != f)
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: pthread_rwlock_unlock returned %d: %s",
                                __func__, source_file, __LINE__,
                                c, strerror(c));
                        errno = c;
                        return pc;
                    } else {
                        if (NULL != f)
                            f(LOG_DEBUG, log_arg,
                                "%s: %s line %d: pthread_rwlock_unlock() succeeded",
                                __func__, source_file, __LINE__);
                    }
                    c = pthread_rwlock_rdlock(prwlock);
                    if (0 != c) {
                        if (NULL != f)
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: pthread_rwlock_rdlock returned %d: %s",
                                __func__, source_file, __LINE__,
                                c, strerror(c));
                        errno = c;
                        return pc;
                    } else {
                        if (NULL != f)
                            f(LOG_DEBUG, log_arg,
                                "%s: %s line %d: pthread_rwlock_rdlock() succeeded",
                                __func__, source_file, __LINE__);
                    }
                break;
                default:
                    errno = c;
                    return pc;
                break;
            }
#if SIMPLE_CONFIG_DEBUG_RWLOCK
        } else {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: pthread_rwlock_rdlock() succeeded",
                __func__, source_file, __LINE__);
#endif
        }
        errno = 0;
        for (pc=root,n=0; n<nnames; n++) {
            temp.name = names[n];
            c = arrayfind((void **)(pc->children), pc->nchildren, &l,
                (void *)(&temp), compar);
            if (0 != c) { /* not found */
                pc = NULL;
                break;
            }
            pc = pc->children[l]; /* found (so far) */
        }
        c = pthread_rwlock_unlock(prwlock);
        if (0 != c) {
            if (NULL != f)
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: pthread_rwlock_unlock returned %d: %s",
                    __func__, source_file, __LINE__,
                    c, strerror(c));
            errno = c;
#if SIMPLE_CONFIG_DEBUG_RWLOCK
        } else {
            f(LOG_DEBUG, log_arg,
                "%s: %s line %d: pthread_rwlock_unlock() succeeded",
                __func__, source_file, __LINE__);
#endif
        }
    }
    return pc;
}

int top_down_config_walk(struct config_entry *pc,
    int (*func)(struct config_entry *, void (*)(int, void *, const char *, ...), void *),
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "top_down_config_walk";
#endif
    if (NULL != pc) {
        /* find root */
        while (NULL != pc->parent)
            pc = pc->parent;
        /* check configuration lock status for root */
        if (NULL == pc->parent) {
            int c, i;
            pthread_rwlock_t *prwlock;

            /* lock config root for writing */
            prwlock = (pthread_rwlock_t *)(pc->value);
            if (NULL == prwlock) {
                errno = EINVAL;
                return -1;
            }
            c = pthread_rwlock_wrlock(prwlock);
            if (0 != c) {
                switch (c) {
                    case EBUSY:
                    /*FALLTHROUGH*/
                    case EDEADLK:
                        c = pthread_rwlock_unlock(prwlock);
                        if (0 != c) {
                            errno = c;
                            return -1;
                        }
                        c = pthread_rwlock_wrlock(prwlock);
                        if (0 != c) {
                            errno = c;
                            return -1;
                        }
                    break;
                    default:
                        errno = c;
                        return -1;
                    break;
                }
            }
            errno = 0;
            i = process_down(pc, func, f, log_arg);
            c = pthread_rwlock_unlock(prwlock);
            if (0 != c) {
                errno = c;
            }
            return i;
        }
    }
    errno = EINVAL;
    return -1;
}

/* free entire configuration memory starting from root */
int free_config(struct config_entry *root,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int c, i;
    pthread_rwlock_t *prwlock;
#ifndef PP__FUNCTION__
    static const char __func__[] = "free_config";
#endif

    if ((unsigned char)0U == simple_config_initialized)
        initialize_simple_config();
    /* lock config root for writing */
    if (NULL == root) {
        errno = EINVAL;
        return -1;
    }
    while (NULL != root->parent)
        root = root->parent;
    prwlock = (pthread_rwlock_t *)(root->value);
    if (NULL == prwlock) {
        errno = EINVAL;
        return -1;
    }
    c = pthread_rwlock_wrlock(prwlock);
    if (0 != c) {
        switch (c) {
            case EBUSY:
            /*FALLTHROUGH*/
            case EDEADLK:
                c = pthread_rwlock_unlock(prwlock);
                if (0 != c) {
                    errno = c;
                    return -1;
                }
                c = pthread_rwlock_wrlock(prwlock);
                if (0 != c) {
                    errno = c;
                    return -1;
                }
            break;
            default:
                errno = c;
                return -1;
            break;
        }
    }
    errno = 0;
    i = depth_first_config_walk(root, free_config_entry, f, log_arg);
    /* rwlock is already gone, thanks to free_config_entry acting on root */
    return i;
}

/* special functions called by re_config: not intended for general use */

/* given a suspected parent (confirmed by checking indentation level),
      add an entry for the name/value item at the specified indentation
      level if it does not already exist
   if there is already an entry with the given name, check if the value
      has been changed, and if so update it
   on error, return a NULL pointer with errno set appropriately
   on success, return a pointer to the entry which has been
      inserted in the appropriate place if new or updated if changed
*/
struct config_entry *add_config_entry(struct config_entry *parent,
    const char *name, const char *value, int indent, int line,
    unsigned int name_column, unsigned int value_column,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    struct config_entry *pc = NULL;     /* will hold return value */
    struct config_entry *proot;
    pthread_rwlock_t *prwlock;
#ifndef PP__FUNCTION__
    static const char __func__[] = "add_config_entry";
#endif

    if ((NULL == name) || (0 > indent)) {
        errno = EINVAL;
    } else {
        int i;

        if ((unsigned char)0U == simple_config_initialized)
            initialize_simple_config();
        while ((NULL != parent) && (indent < parent->child_indent)) {
            parent = parent->parent;
        }
        if (NULL == parent) {
            errno = EINVAL;
        } else {
            /* structure needed for comparison will hold new entry if needed */
            struct config_entry *temp = calloc(1, sizeof(struct config_entry));

            /* lock config root for writing */
            proot = parent;
            while (NULL != proot->parent) {
                proot = proot->parent;
            }
            if (NULL == proot) {
                errno = EINVAL;
                return pc;
            }
            prwlock = (pthread_rwlock_t *)(proot->value);
            if (NULL == prwlock) {
                errno = EINVAL;
                return pc;
            }
            /* caller is responsible for locking */
            if (NULL != temp) {
                /* comparison for insertion needs parent and name */
                temp->parent = parent;
                if (NULL == (temp->name = strdup(name))) { /* no memory */
                    /* clean up */
                    free((void *)temp);
                } else {
                    struct config_entry **arr;
                    size_t where;

                    errno = 0;
                    arr = (struct config_entry **)arrayinsert(
                        (void **)(parent->children), parent->nchildren, &where,
                        temp, configcmp);
                    if (NULL == arr) {
                        if (EEXIST == errno) { /* name exists; value changed? */
                            const char *p, *q;

                            /* free memory no longer needed; avoid ENOMEM */
                            free((void *)(temp->name));
                            free((void *)temp);
                            pc = parent->children[where];
                            p = pc->value;
                            (void)reset_config_entry_flags(pc, f, log_arg); /* no deleted flag */
                            /* possible cases regarding value change:
                                 1. old != NULL, new != NULL, same content
                                       no change
                                 2. old != NULL, new != NULL, different content,
                                    strdup OK
                                       value changed; replace (freeing old
                                       value) and set changed flag
                                 3. old != NULL, new != NULL, different,
                                    no memory to strdup
                                       errno becomes ENOMEM
                                 4. old != NULL, new == NULL
                                       value was deleted, replace pointer
                                       (freeing old value) and set changed flag
                                 5. old == NULL, new != NULL, no memory
                                       errno becomes ENOMEM
                                 6. old == NULL, new != NULL, strdup OK
                                       set new value and changed flag
                                 7. old == NULL, new == NULL
                                       no change
                            */
                            /* check for change; i.e. all cases except 1 & 7 */
                            if (
                              ((NULL != p) && (NULL != value)
                               && (0 != strcmp(p, value))) /* cases 2 & 3 */
                            ||
                              ((NULL != p) && (NULL == value)) /* case 4 */
                            ||
                              ((NULL == p) && (NULL != value)) /* cases 5 & 6 */
                            ) {
                                (void)set_config_entry_flag(pc, (int)CONFIG_FLAG_VALUE_CHANGED, f, log_arg);
                                /* simplest remaining case (4); deleted value */
                                if (NULL == value) { /* p != NULL */
                                    pc->value = NULL;
                                    free((void *)p);
                                } else { /* value != NULL */
                                    /* handle cases (5 & 6) where p was NULL */
                                    if (NULL == p) { 
                                        /* try to strdup value replacement */
                                        q = strdup(value);
                                        if (NULL != q) { /* case 6 */
                                            pc->value = q;
                                        } else { /* case 5 */
                                            /* indicate error in value */
                                            (void)set_config_entry_flag(pc,
                                                (int)CONFIG_FLAG_VALUE_WRONG, f, log_arg);
                                        }
                                    } else { /* p != NULL; last cases 2 & 3 */
                                        /* try to strdup value replacement */
                                        q = strdup(value);
                                        if (NULL != q) { /* case 2 */
                                            pc->value = q;
                                            free((void *)p);
                                        } else { /* case 3 */
                                            /* as it now stands, the value will
                                               be wrong; free old value, try
                                               strdup again
                                            */
                                            free((void *)p);
                                            q = strdup(value);
                                            if (NULL == (pc->value = q)) {
                                                /* freeing value didn't help */
                                                /* indicate error in value */
                                                (void)set_config_entry_flag(pc,
                                                    (int)CONFIG_FLAG_VALUE_WRONG, f, log_arg);
                                            } /* else that worked! */
                                        }
                                    }
                                }
                                /* update configuration timestamp to indicate change */
                                i = timestamp_config(pc, f, log_arg);
                                if (0 > i)
                                    pc = NULL;
                            }
                        } else {
                            free((void *)(temp->name));
                            free((void *)temp);
                        }
                    } else { /* arr != NULL */
                        if (0U == parent->nchildren)
                            parent->child_indent = indent;
                        parent->children = arr;
                        parent->nchildren++;
                        (void)set_config_entry_flag(temp,
                            (int)CONFIG_FLAG_ENTRY_NEW, f, log_arg);
                        temp->child_indent = indent + 1;
                        if (NULL == value)
                            temp->value = NULL;
                        else {
                            if (NULL == (temp->value = strdup(value))) {
                                /* indicate error in value */
                                (void)set_config_entry_flag(temp,
                                    (int)CONFIG_FLAG_VALUE_WRONG, f, log_arg);
                            }
                        }
                        pc = temp;
                        /* update configuration timestamp to indicate change */
                        i = timestamp_config(pc, f, log_arg);
                        if (0 > i)
                            pc = NULL;
                    }
                    if (NULL != pc) {
                        pc->line = line;
                        pc->name_column = name_column;
                        pc->value_column = value_column;
                    }
                }
            }
        }
    }
    return pc;
}

/* reset all configuration flags */
/* this doesn't actually *require* depth-first */
int reset_config_flags(struct config_entry *root,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int i;
#ifndef PP__FUNCTION__
    static const char __func__[] = "reset_config_flags";
#endif

    /* caller is responsible for locking... */
    errno = 0;
    i = depth_first_config_walk(root, reset_config_entry_flags, f, log_arg);
    return i;
}

/* mark entry and its descendants as deleted */
int mark_config_subtree_deleted(struct config_entry *pc,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "mark_config_subtree_deleted";
#endif
    if (NULL != pc) {
        struct config_entry **children = pc->children;
        int i;

        if (NULL != pc->parent) /* never mark root as deleted! */
            (void)set_config_entry_flag(pc, (int)CONFIG_FLAG_ENTRY_DELETED, f, log_arg);
        if (NULL != children) {
            size_t c;

            for (c=pc->nchildren; 0U!=c; ) {
                i = mark_config_subtree_deleted(children[--c], f, log_arg);
                if (0 > i)
                    return i;
            }
        }
        return 0;
    }
    errno = EINVAL;
    return -1;
}

/* expunge entire configuration memory starting from root */
int expunge_config(struct config_entry *root,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int i;
#ifndef PP__FUNCTION__
    static const char __func__[] = "expunge_config";
#endif

    /* caller is responsible for locking */
    errno = 0;
    i = depth_first_config_walk(root, expunge_config_entry, f, log_arg);
    return i;
}
