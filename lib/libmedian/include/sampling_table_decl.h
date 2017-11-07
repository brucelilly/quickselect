/* $Id: ~|^` @(#)   This is sampling_table_decl.h version 1.2 dated 2017-09-15T04:56:28Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.sampling_table_decl.h */

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
struct sampling_table_struct *sampling_table(size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk, char **ppeq,
    unsigned int *psort, unsigned int *pindex, size_t nmemb)