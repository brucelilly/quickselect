/* $Id: ~|^` @(#)   This is cutoff_value_decl.h version 1.3 dated 2017-11-03T20:17:11Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.cutoff_value_decl.h */

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

size_t cutoff_value(size_t size_ratio, unsigned int options)
