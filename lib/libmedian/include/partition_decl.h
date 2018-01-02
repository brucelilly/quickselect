/* $Id: ~|^` @(#)   This is partition_decl.h version 1.5 dated 2017-12-22T04:14:04Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.partition_decl.h */

void PARTITION_FUNCTION_NAME(char *base, size_t first, size_t beyond, char *pc,
    char *pd, char *pivot, char *pe, char *pf, size_t size,
    COMPAR_DECL, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, unsigned int options, char *conditions,
    size_t *indices, char *element, size_t *peq, size_t *pgt)
