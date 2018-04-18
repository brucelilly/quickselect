/* $Id: ~|^` @(#)   This is quickselect_loop_decl.h version 1.10 dated 2018-03-07T04:37:05Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect_loop_decl.h */

/* quickselect_loop is externally visible so that it can be called from qsort
   and quickselect
*/
/* N.B. declaration and comments only (e.g. no preprocessor directives) */
QUICKSELECT_INLINE QUICKSELECT_RETURN_TYPE QUICKSELECT_LOOP(char *base, size_t first,
    size_t beyond, const size_t size, COMPAR_DECL, const size_t *pk,
    size_t firstk, size_t beyondk, void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, unsigned int table_index,
    size_t cachesz, size_t pbeyond, unsigned int options, char **ppeq, char **ppgt)
