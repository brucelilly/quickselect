/* $Id: ~|^` @(#)   This is select_pivot_decl.h version 1.4 dated 2017-11-03T20:26:44Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.select_pivot_decl.h */

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
# endif
#else
extern
#endif /* QUICKSELECT_BUILD_FOR_SPEED */
char *SELECT_PIVOT_FUNCTION_NAME(char *base, size_t first, size_t beyond,
    size_t size, COMPAR_DECL,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int table_index, const size_t *pk, unsigned int options,
    char **ppc, char **ppd, char **ppe, char **ppf)
