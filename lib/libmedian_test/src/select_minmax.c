/* Description: file to include externally-visible select_minmax function */
/* $Id: ~|^` @(#)   This is select_minmax.c version 1.2 dated 2019-03-16T15:37:11Z. \ select_minmax.c $ */
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.select_minmax.c */

#define LIBMEDIAN_TEST_CODE 1

#undef QUICKSELECT_INLINE
#define QUICKSELECT_INLINE /**/

#undef QUICKSELECT_STATIC
#define QUICKSELECT_STATIC /**/

#undef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 0

#include "median_test_config.h"

#include "select_minmax_src.h"
