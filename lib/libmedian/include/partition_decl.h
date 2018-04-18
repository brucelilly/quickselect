/* $Id: ~|^` @(#)   This is partition_decl.h version 1.7 dated 2018-02-24T05:45:38Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.partition_decl.h */

void PARTITION_FUNCTION_NAME(char *base, size_t first, size_t beyond, char *pc,
    char *pd, char *pivot, char *pe, char *pf, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, size_t cache_sz, unsigned int options, size_t *peq,
    size_t *pgt)
