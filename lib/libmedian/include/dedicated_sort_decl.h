/* $Id: ~|^` @(#)   This is dedicated_sort_decl.h version 1.12 dated 2018-04-15T03:40:59Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.dedicated_sort_decl.h */

/* can qualify (e.g. extern or static) at inclusion */
QUICKSELECT_INLINE
DEDICATED_SORT_RETURN_TYPE DEDICATED_SORT(/*This is a declaration, not a macro*/
    char *base, size_t first, size_t beyond, /*const*/ size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int table_index, size_t cache_sz,
    size_t pbeyond, unsigned int options)
