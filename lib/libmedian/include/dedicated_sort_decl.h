/* $Id: ~|^` @(#)   This is dedicated_sort_decl.h version 1.14 dated 2018-06-10T01:25:16Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.dedicated_sort_decl.h */

/* can qualify (e.g. extern or static) at inclusion */
DEDICATED_SORT_RETURN_TYPE DEDICATED_SORT(/*This is a declaration, not a macro*/
    char *base, size_t first, size_t beyond, /*const*/ size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, size_t cache_sz, size_t pbeyond, unsigned int options)
