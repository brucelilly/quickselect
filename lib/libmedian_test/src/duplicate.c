/*INDENT OFF*/

/* Description: C source code for selection-related development */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    duplicate.c copyright 2016-2017 Bruce Lilly.   \ duplicate.c $
* This software is provided 'as-is', without any express or implied warranty.
* In no event will the authors be held liable for any damages arising from the
* use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it freely,
* subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not claim
*    that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
****************************** (end of license) ******************************/
/* $Id: ~|^` @(#)   This is duplicate.c version 1.1 dated 2017-09-29T14:34:10Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.duplicate.c */

/********************** Long description and rationale: ***********************
* starting point for select/median implementation
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: duplicate.c ~|^` @(#)"
#define SOURCE_MODULE "duplicate.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2017-09-29T14:34:10Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* duplicate (as far as possible) long data in larray to other arrays */
/* n items; source starts at index 0UL, copies at index o */
void duplicate_test_data(long *refarray, long *larray, int *array, double *darray, struct data_struct *data_array, struct data_struct **parray, size_t o, size_t n)
{
    long l;
    int i;
    size_t j;
    double d;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    for (j=0UL; j<n; j++) {
        l = refarray[j];
        i = l % INT_MAX;
        d = (double)l;
        if (NULL!=larray) {
            larray[j+o] = l;
        }
        if (NULL!=data_array) {
#if 0 /* string as textual number representation */
            (V)snl(data_array[j+o].string,STRING_SIZE,NULL,NULL,l,STRING_BASE,
                '0', STRING_SIZE-1, NULL, NULL);
            if (NULL!=parray) {
                parray[j+o] = &(data_array[j+o]);
            }
#else /* string as date-time */
            struct civil_time_struct cts;
            time_t t; /* time_t might have greater range than long on 32-bit *BSD */

#if ULONG_MAX > 0xffffffffUL
            l %= 253402300800L; /* max. 9999-12-31T23:59:59 */
#endif
            t = (time_t)l;
            gmtime_r(&t, &(cts.tm));
            cts.fraction=cts.offset=0.0;
            (V)sn_civil_time(data_array[j+o].string, STRING_SIZE, &cts, 0, 1, 0, NULL, NULL);
#if ASSERT_CODE
            t = utc_mktime(&(cts.tm),NULL,NULL);
            if (t != (time_t)l) (V)fprintf(stderr, "// %s line %d: %ld -> %s (%04d-%02d-%02dT%02d:%02d:%02dZ) -> %ld\n",__func__,__LINE__,l,data_array[j+o].string,cts.tm.tm_year+1900,cts.tm.tm_mon+1,cts.tm.tm_mday,cts.tm.tm_hour,cts.tm.tm_min,cts.tm.tm_sec,(long)t);
            A(t==(time_t)l);
#endif /* ASSERT_CODE */
            data_array[j+o].year=cts.tm.tm_year+1900,data_array[j+o].month=cts.tm.tm_mon+1;
//            data_array[j+o].yday=cts.tm.tm_yday+1,data_array[j+o].wday=cts.tm.tm_wday;
            data_array[j+o].mday=cts.tm.tm_mday,data_array[j+o].hour=cts.tm.tm_hour;
            data_array[j+o].minute=cts.tm.tm_min,data_array[j+o].second=cts.tm.tm_sec;
#endif
        }
        if (NULL!=darray) {
            darray[j+o] = d;
        }
        if (NULL!=array) {
            array[j+o] = i;
        }
    }
}

void restore_test_data(size_t o, size_t n, long *refarray, long *larray, int *array, double *darray, struct data_struct *data_array, struct data_struct **parray)
{
    duplicate_test_data(refarray, larray, array, darray, data_array, parray, o, n);
}
