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
* $Id: ~|^` @(#)    compare.c copyright 2016-2017 Bruce Lilly.   \ compare.c $
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
/* $Id: ~|^` @(#)   This is compare.c version 1.3 dated 2017-12-28T22:17:34Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.compare.c */

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
#define ID_STRING_PREFIX "$Id: compare.c ~|^` @(#)"
#define SOURCE_MODULE "compare.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2017-12-28T22:17:34Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

int ilongcmp(const void *p1, const void *p2)
{
    int r = longcmp(p1,p2);
    /* longcmp (in compare.h) always returns one of -1, 0, 1; update counts
       without favoring execution time for any distribution of results
    */
    switch (r) {
        case -1 : nlt++;
        break;
        case 0 : neq++;
        break;
        case 1 : ngt++;
        break;
    }
    return r;
}

int longcmp_s(const void *p1, const void *p2, void *unused)
{
    return longcmp(p1,p2);
}

int ilongcmp_s(const void *p1, const void *p2, void *unused)
{
    return ilongcmp(p1,p2);
}

int iintcmp(const void *p1, const void *p2)
{
#if 0
fprintf(stderr, "// %s line %d: p1=%p(%d), p2=%p(%d)\n",__func__,__LINE__,(void *)p1,*((int *)p1),(void *)p2,*((int *)p2));
#endif
    int r = intcmp(p1,p2);
    /* intcmp (in compare.h) always returns one of -1, 0, 1; update counts
       without favoring execution time for any distribution of results
    */
    switch (r) {
        case -1 : nlt++;
        break;
        case 0 : neq++;
        break;
        case 1 : ngt++;
        break;
    }
    return r;
}

int intcmp_s(const void *p1, const void *p2, void *unused)
{
    return intcmp(p1,p2);
}

int iintcmp_s(const void *p1, const void *p2, void *unused)
{
    return iintcmp(p1,p2);
}

int idoublecmp(const void *p1, const void *p2)
{
    int r = doublecmp(p1,p2);
    /* doublecmp (in compare.h) always returns one of -1, 0, 1; update counts
       without favoring execution time for any distribution of results
    */
    switch (r) {
        case -1 : nlt++;
        break;
        case 0 : neq++;
        break;
        case 1 : ngt++;
        break;
    }
    return r;
}

int doublecmp_s(const void *p1, const void *p2, void *unused)
{
    return doublecmp(p1,p2);
}

int idoublecmp_s(const void *p1, const void *p2, void *unused)
{
    return idoublecmp(p1,p2);
}

int data_struct_strcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        int c;
        const struct data_struct *pa = (const struct data_struct *)p1,
            *pb = (const struct data_struct *)p2;

        c = strcmp(pa->string, pb->string);
if (DEBUGGING(COMPARE_DEBUG)) fprintf(stderr, "string %s vs. %s\n",pa->string,pb->string);
        if (0>c) return -1;
        if (0<c) return 1;
    }
    return 0;
}

int idata_struct_strcmp(const void *p1, const void *p2)
{
    int c = data_struct_strcmp(p1, p2);
    switch (c) {
        case -1 : nlt++;
        break;
        case 0 : neq++;
        break;
        case 1 : ngt++;
        break;
    }
    return c;
}

int data_struct_strcmp_s(const void *p1, const void *p2, void *unused)
{
    return data_struct_strcmp(p1,p2);
}

int idata_struct_strcmp_s(const void *p1, const void *p2, void *unused)
{
    return idata_struct_strcmp(p1,p2);
}

int timecmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct data_struct *pa = (const struct data_struct *)p1,
            *pb = (const struct data_struct *)p2;
        int i;

        if (DEBUGGING(COMPARE_DEBUG)) {
            (V)fprintf(stderr,
                "%s %04d-%02d-%02dT%02d:%02d:%02d.%02d%02d%02d%02d%02d%02d vs. "
                "%s %04d-%02d-%02dT%02d:%02d:%02d.%02d%02d%02d%02d%02d%02d\n",
                pa->string,pa->year,pa->month,pa->mday,pa->hour,pa->minute,
                pa->second,pa->fractional[0],pa->fractional[1],
                pa->fractional[2],pa->fractional[3],pa->fractional[4],
                pa->fractional[5],
                pb->string,pb->year,pb->month,pb->mday,pb->hour,pb->minute,
                pb->second,pb->fractional[0],pb->fractional[1],
                pb->fractional[2],pb->fractional[3],pb->fractional[4],
                pb->fractional[5]);
        }
        if (pa->year>pb->year) return 1; else if (pa->year<pb->year) return -1;
        if (pa->month>pb->month) return 1;
        else if (pa->month<pb->month) return -1;
        if (pa->mday>pb->mday) return 1; else if (pa->mday<pb->mday) return -1;
        if (pa->hour>pb->hour) return 1; else if (pa->hour<pb->hour) return -1;
        if (pa->minute>pb->minute) return 1;
        else if (pa->minute<pb->minute) return -1;
        if (pa->second>pb->second) return 1;
        else if (pa->second<pb->second) return -1;
        for (i=0; i<6; i++) {
            if (pa->fractional[i]>pb->fractional[i]) return 1;
            else if (pa->fractional[i]<pb->fractional[i]) return -1;
        }
    }
    return 0;
}

int itimecmp(const void *p1, const void *p2)
{
    int c = timecmp(p1, p2);
    switch (c) {
        case -1 : nlt++;
        break;
        case 0 : neq++;
        break;
        case 1 : ngt++;
        break;
    }
    return c;
}

int timecmp_s(const void *p1, const void *p2, void *unused)
{
    return timecmp(p1,p2);
}

int itimecmp_s(const void *p1, const void *p2, void *unused)
{
    return itimecmp(p1,p2);
}

int indcmp(const void *p1, const void *p2)
{
    if ((NULL != p1) && (NULL != p2) && (p1 != p2)) {
        const struct data_struct *pa
            = *((const struct data_struct * const *)p1);
        const struct data_struct *pb
            = *((const struct data_struct * const *)p2);

        return timecmp((const void *)pa, (const void *)pb);
    }
    return 0;
}

int iindcmp(const void *p1, const void *p2)
{
    int c = indcmp(p1, p2);
    switch (c) {
        case -1 : nlt++;
        break;
        case 0 : neq++;
        break;
        case 1 : ngt++;
        break;
    }
    return c;
}

int indcmp_s(const void *p1, const void *p2, void *unused)
{
    return indcmp(p1,p2);
}

int iindcmp_s(const void *p1, const void *p2, void *unused)
{
    return iindcmp(p1,p2);
}

int nocmp(void *unused1, void *unused2)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    return 0;
}
