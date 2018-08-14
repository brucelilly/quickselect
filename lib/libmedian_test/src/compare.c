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
* $Id: ~|^` @(#)    compare.c copyright 2016-2018 Bruce Lilly.   \ compare.c $
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
/* $Id: ~|^` @(#)   This is compare.c version 1.9 dated 2018-07-27T18:21:36Z. \ $ */
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
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2018-07-27T18:21:36Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

COMPARE_DEFINITION(CHARCMP,char)
/* doublecmp (used by repeated_median) is defined in libmedian repeated_median.c */
COMPARE_DEFINITION(FLOATCMP,float)
COMPARE_DEFINITION(INTCMP,int)
COMPARE_DEFINITION(LONGCMP,long)
COMPARE_DEFINITION(SHORTCMP,short)
COMPARE_DEFINITION(ULCMP,unsigned long)
/* size_tcmp (used by quickselect) is defined in libmedian quickselect_src.h */

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
COMPARE_DEFINITION(INT_LEAST8_TCMP,int_least8_t)
COMPARE_DEFINITION(INT_FAST8_TCMP,int_fast8_t)
COMPARE_DEFINITION(UINT_LEAST8_TCMP,uint_least8_t)
COMPARE_DEFINITION(UINT_FAST8_TCMP,uint_fast8_t)

COMPARE_DEFINITION(INT_LEAST16_TCMP,int_least16_t)
COMPARE_DEFINITION(INT_FAST16_TCMP,int_fast16_t)
COMPARE_DEFINITION(UINT_LEAST16_TCMP,uint_least16_t)
COMPARE_DEFINITION(UINT_FAST16_TCMP,uint_fast16_t)

COMPARE_DEFINITION(INT_LEAST32_TCMP,int_least32_t)
COMPARE_DEFINITION(INT_FAST32_TCMP,int_fast32_t)
COMPARE_DEFINITION(UINT_LEAST32_TCMP,uint_least32_t)
COMPARE_DEFINITION(UINT_FAST32_TCMP,uint_fast32_t)

COMPARE_DEFINITION(INT_LEAST64_TCMP,int_least64_t)
COMPARE_DEFINITION(INT_FAST64_TCMP,int_fast64_t)
COMPARE_DEFINITION(UINT_LEAST64_TCMP,uint_least64_t)
COMPARE_DEFINITION(UINT_FAST64_TCMP,uint_fast64_t)
#endif /* C99 */

int ilongcmp(const void *p1, const void *p2)
{
    int r = longcmp(p1,p2);
    /* longcmp (in compare.h) always returns one of -1, 0, 1; update counts
       without favoring execution time for any distribution of results
    */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(COMPARE_DEBUG))
        (V)fprintf(stderr,"/* %s: compared %p(%ld),%p(%ld) -> %d */\n",__func__,
            p1,*((long *)p1),p2,*((long *)p2),r);
#endif
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
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(COMPARE_DEBUG))
        (V)fprintf(stderr,"/* %s: compared %p(%d),%p(%d) -> %d */\n",__func__,
            p1,*((int *)p1),p2,*((int *)p2),r);
#endif
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

int ishortcmp(const void *p1, const void *p2)
{
    short r = shortcmp(p1,p2);
    /* shortcmp (in compare.h) always returns one of -1, 0, 1; update counts
       without favoring execution time for any distribution of results
    */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(COMPARE_DEBUG))
        (V)fprintf(stderr,"/* %s: compared %p(%hd),%p(%hd) -> %d */\n",__func__,
            p1,*((short *)p1),p2,*((short *)p2),r);
#endif
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

int shortcmp_s(const void *p1, const void *p2, void *unused)
{
    return shortcmp(p1,p2);
}

int ishortcmp_s(const void *p1, const void *p2, void *unused)
{
    return ishortcmp(p1,p2);
}

int idoublecmp(const void *p1, const void *p2)
{
    int r = doublecmp(p1,p2);
    /* doublecmp (in compare.h) always returns one of -1, 0, 1; update counts
       without favoring execution time for any distribution of results
    */
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(COMPARE_DEBUG))
        (V)fprintf(stderr,"/* %s: compared %p(%E),%p(%E) -> %d */\n",__func__,
            p1,*((double *)p1),p2,*((double *)p2),r);
#endif
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
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(COMPARE_DEBUG))
            (V)fprintf(stderr,
                "/* %s: compared string \"%s\" vs. \"%s\" -> %d */\n",
                __func__,pa->string,pb->string,c);
#endif
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

#if (DEBUG_CODE > 0) && defined(DEBUGGING)
        if (DEBUGGING(COMPARE_DEBUG)) {
            (V)fprintf(stderr,"/* %s: comparing "
                "%s %04d-%02d-%02dT%02d:%02d:%02d.%d%d%d%d%d%d%d%d%d%d%d%d vs. "
                "%s %04d-%02d-%02dT%02d:%02d:%02d.%d%d%d%d%d%d%d%d%d%d%d%d */\n"
                ,__func__,
                pa->string,pa->year,pa->u_var.s_md.month,pa->u_var.s_md.mday,pa->hour,pa->minute,
                pa->second,
                ((pa->fractional[0])>>4)&0x0f,
                (pa->fractional[0])&0x0f,
                ((pa->fractional[1])>>4)&0x0f,
                (pa->fractional[1])&0x0f,
                ((pa->fractional[2])>>4)&0x0f,
                (pa->fractional[2])&0x0f,
                ((pa->fractional[3])>>4)&0x0f,
                (pa->fractional[3])&0x0f,
                ((pa->fractional[4])>>4)&0x0f,
                (pa->fractional[4])&0x0f,
                ((pa->fractional[5])>>4)&0x0f,
                (pa->fractional[5])&0x0f,
                pb->string,pb->year,pb->u_var.s_md.month,pb->u_var.s_md.mday,pb->hour,pb->minute,
                pb->second,
                ((pb->fractional[0])>>4)&0x0f,
                (pb->fractional[0])&0x0f,
                ((pb->fractional[1])>>4)&0x0f,
                (pb->fractional[1])&0x0f,
                ((pb->fractional[2])>>4)&0x0f,
                (pb->fractional[2])&0x0f,
                ((pb->fractional[3])>>4)&0x0f,
                (pb->fractional[3])&0x0f,
                ((pb->fractional[4])>>4)&0x0f,
                (pb->fractional[4])&0x0f,
                ((pb->fractional[5])>>4)&0x0f,
                (pb->fractional[5])&0x0f
            );
        }
#endif
        if (pa->year>pb->year) return 1; else if (pa->year<pb->year) return -1;
        if (pa->u_var.s_md.month>pb->u_var.s_md.month) return 1;
        else if (pa->u_var.s_md.month<pb->u_var.s_md.month) return -1;
        if (pa->u_var.s_md.mday>pb->u_var.s_md.mday) return 1;
        else if (pa->u_var.s_md.mday<pb->u_var.s_md.mday) return -1;
        if (pa->hour>pb->hour) return 1; else if (pa->hour<pb->hour) return -1;
        if (pa->minute>pb->minute) return 1;
        else if (pa->minute<pb->minute) return -1;
        if (pa->second>pb->second) return 1;
        else if (pa->second<pb->second) return -1;
        for (i=0; i<6; i++) {
            /* comparing packed pairs of BCD digits as unsigned integers is OK */
            if (pa->fractional[i]>pb->fractional[i]) return 1;
            else if (pa->fractional[i]<pb->fractional[i]) return -1;
        }
    }
    return 0;
}

int itimecmp(const void *p1, const void *p2)
{
    int c = timecmp(p1, p2);
#if (DEBUG_CODE > 0) && defined(DEBUGGING)
    if (DEBUGGING(COMPARE_DEBUG))
        (V)fprintf(stderr,"/* %s: compared %p,%p -> %d */\n",__func__,p1,p2,c);
#endif
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
