/* Description: file to include externally-visible find_minmax function */
/* $Id: ~|^` @(#)   This is d_find_minmax.c version 1.2 dated 2019-03-15T14:05:56Z. \ d_find_minmax.c $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.d_find_minmax.c */

#undef QUICKSELECT_BUILD_FOR_SPEED
#define QUICKSELECT_BUILD_FOR_SPEED 0

/* external visibility */
#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /**/

/* C11 _s variant */
#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#define LIBMEDIAN_TEST_CODE 1

#include "median_test_config.h"

#include "find_minmax_src.h"
