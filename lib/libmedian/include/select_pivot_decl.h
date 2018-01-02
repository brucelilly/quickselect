/* $Id: ~|^` @(#)   This is select_pivot_decl.h version 1.5 dated 2017-12-22T04:14:04Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.select_pivot_decl.h */

#if __STDC_WANT_LIB_EXT1__
char *select_pivot_s(char *base, size_t first, size_t beyond,
    size_t size, COMPAR_DECL,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int table_index, const size_t *pk, unsigned int options,
    char **ppc, char **ppd, char **ppe, char **ppf)
#else
char *select_pivot(char *base, size_t first, size_t beyond,
    size_t size, COMPAR_DECL,
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int table_index, const size_t *pk, unsigned int options,
    char **ppc, char **ppd, char **ppe, char **ppf)
#endif /* __STDC_WANT_LIB_EXT1__ */
