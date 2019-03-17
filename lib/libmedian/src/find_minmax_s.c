/* Description: file to include externally-visible find_minmax_s function */
/* $Id: ~|^` @(#)   This is find_minmax_s.c version 1.1 dated 2019-03-06T21:12:50Z. \ find_minmax_s.c $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.find_minmax_s.c */

#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /**/

/* external visibility */
#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

/* C11 _s variant */
#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1

#include "quickselect_config.h"

#include "find_minmax_src.h"
