/* $Id: ~|^` @(#)   This is dedicated_sort_decl.h version 1.4 dated 2017-11-03T20:19:36Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.dedicated_sort_decl.h */

#ifndef SORTING_DEDICATED_SORT_CUTOFF
# include "quickselect_config.h"
#endif

/* need size_t */
#ifndef NULL
# include <stddef.h>            /* size_t NULL (maybe rsize_t) */
#endif

#if QUICKSELECT_BUILD_FOR_SPEED
static
# if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
# endif /* C99 */
#else
extern
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
QSORT_RETURN_TYPE DEDICATED_SORT(char *base, size_t first, size_t beyond,
    /*const*/ size_t size, COMPAR_DECL, void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, unsigned int options)
