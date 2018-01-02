/* $Id: ~|^` @(#)   This is dedicated_sort_decl.h version 1.6 dated 2017-12-22T04:14:04Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.dedicated_sort_decl.h */

void dedicated_sort(char *base, size_t first, size_t beyond,
    /*const*/ size_t size, int(*compar)(const void *,const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
