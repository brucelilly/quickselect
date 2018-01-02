/* $Id: ~|^` @(#)   This is sampling_table_decl.h version 1.3 dated 2017-12-22T04:14:04Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.sampling_table_decl.h */

struct sampling_table_struct *sampling_table(size_t first, size_t beyond,
    const size_t *pk, size_t firstk, size_t beyondk, char **ppeq,
    unsigned int *psort, unsigned int *pindex, size_t nmemb)
