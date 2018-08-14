/* $Id: ~|^` @(#)   This is quickselect_loop_decl.h version 1.12 dated 2018-06-12T01:33:02Z. \ $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/include/s.quickselect_loop_decl.h */

/* quickselect_loop is externally visible so that it can be called from qsort
   and quickselect
*/
/* N.B. declaration and comments only (e.g. no preprocessor directives) */
QUICKSELECT_RETURN_TYPE QUICKSELECT_LOOP(char *base, size_t first,
    size_t beyond, const size_t size, COMPAR_DECL, const size_t *pk,
    size_t firstk, size_t beyondk, void (*swapf)(char *, char *, size_t),
    size_t alignsize, size_t size_ratio, size_t cachesz, size_t pbeyond,
    unsigned int options, char **ppeq, char **ppgt)
