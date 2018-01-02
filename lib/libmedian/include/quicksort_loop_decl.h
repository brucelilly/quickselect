/* $Id: ~|^` @(#)   This is quicksort_loop_decl.h version 1.2 dated 2017-09-15T04:49:55Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quicksort_loop_decl.h */
#error "obsolete source"

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

/* quicksort_loop is externally visible so that it can be called from qsort
   and/or quickselect
*/
QSORT_RETURN_TYPE QUICKSORT_LOOP(char *base, size_t first, size_t beyond,
    const size_t size, COMPAR_DECL,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t cutoff,
    int pivot_selection_method)
