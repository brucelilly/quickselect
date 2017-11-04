/* $Id: ~|^` @(#)   This is should_repivot_decl.h version 1.3 dated 2017-11-03T20:51:21Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.should_repivot_decl.h */

#ifndef SORTING_DEDICATED_SORT_CUTOFF
# include "quickselect_config.h"
#endif

/* need size_t */
#ifndef NULL
# include <stddef.h>            /* size_t NULL (maybe rsize_t) */
#endif
/* need struct sampling_table_struct */
#ifndef	TABLES_H_INCLUDED
# include "tables.h"
#endif

#if QUICKSELECT_BUILD_FOR_SPEED
static
# if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
# endif /* C99 */
#else
extern
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
unsigned int should_repivot(const size_t nmemb, const size_t n, size_t cutoff,
    struct sampling_table_struct *pst, unsigned int table_index,
    const size_t *pk, int *pn2)
