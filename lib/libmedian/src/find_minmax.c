/* Description: file to include externally-visible find_minmax function */
/* $Id: ~|^` @(#)   This is find_minmax.c version 1.1 dated 2019-03-06T21:12:50Z. \ find_minmax.c $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.find_minmax.c */

#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /**/

/* external visibility */
#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

/* normal code, not C11 _s variant */
#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#include "quickselect_config.h"

#include "find_minmax_src.h"
