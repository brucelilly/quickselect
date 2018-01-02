/* $Id: ~|^` @(#)   This is quickselect_loop_decl.h version 1.6 dated 2017-12-22T04:14:04Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect_loop_decl.h */

/* quickselect_loop is externally visible so that it can be called from qsort
   and quickselect
*/
void quickselect_loop(char *base,size_t first,size_t beyond, const size_t size,
    int(*compar)(const void *,const void *), const size_t *pk, size_t firstk,
    size_t beyondk, void (*swapf)(char *, char *, size_t), size_t alignsize,
    size_t size_ratio, size_t cutoff, unsigned int options, char *conditions,
    size_t *indices, char *element, char **ppeq, char **ppgt)
