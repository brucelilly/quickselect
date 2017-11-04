/* $Id: ~|^` @(#)   This is partition_decl.h version 1.3 dated 2017-11-03T20:22:16Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.partition_decl.h */

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
# endif
#else
extern
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
void PARTITION_FUNCTION_NAME(char *base, size_t first, size_t beyond, char *pc,
    char *pd, char *pivot, char *pe, char *pf, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options, size_t *peq, size_t *pgt)
