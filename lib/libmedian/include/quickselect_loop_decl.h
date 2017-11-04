/* $Id: ~|^` @(#)   This is quickselect_loop_decl.h version 1.4 dated 2017-11-03T20:35:38Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect_loop_decl.h */

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

/* quickselect_loop is externally visible so that it can be called from qsort
   and quickselect
*/
QSORT_RETURN_TYPE QUICKSELECT_LOOP(char *base,size_t first,size_t beyond,
    const size_t size, COMPAR_DECL,
    const size_t *pk, size_t firstk, size_t beyondk,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    size_t cutoff, unsigned int options, char **ppeq, char **ppgt)
