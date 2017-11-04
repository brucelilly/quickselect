/* $Id: ~|^` @(#)   This is klimits_decl.h version 1.1 dated 2017-09-08T00:48:50Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.klimits_decl.h */

#ifndef SORTING_DEDICATED_SORT_CUTOFF
# include "quickselect_config.h"
#endif

/* need size_t */
#ifndef NULL
#include <stddef.h>             /* size_t NULL (maybe rsize_t) */
#endif

#if QUICKSELECT_BUILD_FOR_SPEED
static
# if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
# endif /* C99 */
#else
extern
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
void klimits(size_t first, size_t beyond, const size_t *pk, size_t firstk,
    size_t beyondk, size_t *pfk, size_t *pbk)
