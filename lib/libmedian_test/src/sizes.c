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
* $Id: ~|^` @(#)    sizes.c copyright 2016-2018 Bruce Lilly.   \ sizes.c $
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
/* $Id: ~|^` @(#)   This is sizes.c version 1.6 dated 2018-03-06T20:50:44Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.sizes.c */

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
#define ID_STRING_PREFIX "$Id: sizes.c ~|^` @(#)"
#define SOURCE_MODULE "sizes.c"
#define MODULE_VERSION "1.6"
#define MODULE_DATE "2018-03-06T20:50:44Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* print some size statistics to stdout */
void print_sizes(const char *prefix, const char *prog)
{
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    _Bool xb;
    int_least8_t xl8;
    int_fast8_t xf8;
    uint_least8_t xul8;
    uint_fast8_t xuf8;
    int_least16_t xl16;
    int_fast16_t xf16;
    uint_least16_t xul16;
    uint_fast16_t xuf16;
    int_least32_t xl32;
    int_fast32_t xf32;
    uint_least32_t xul32;
    uint_fast32_t xuf32;
    int_least64_t xl64;
    int_fast64_t xf64;
    long long xll; /* she *doesn't* wear short shorts >:{ */
    unsigned long long xull;
    uint_least64_t xul64;
    uint_fast64_t xuf64;
    long double xld;
    float _Complex xfc;
    double _Complex xdc;
    long double _Complex xldc;
    /* What? no long long double double _Complex _Complex?! */
#endif /* C99 */
    char xc;
    struct data_struct xstruct;
    unsigned char xuc;
    short xs;
    unsigned short xus;
    int xi;
    unsigned int xu;
    long xl;
    unsigned long xul;
    float xf;
    double xd;
    void *xptr;
    size_t xsize_t;
    size_t sz, as, r;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* information about things that may affect types and/or sizes: */
    (V)fprintf(stdout, "%s%s: %s (%s %s %s) COMPILER_USED=\"%s\" __STDC_VERSION__=%ld\n", prefix,
        prog, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED, MEDIAN_STDC_VERSION);

    /* formats */
#define FMT1 "%s%20.20s %4.4s %9.9s %5.5s\n"
#define FMT2 "%s%20.20s %4lu %9lu %5lu\n"
    /* headings */
    (V)fprintf(stdout, FMT1, prefix, "type", "size", "alignment", "ratio");
    (V)fprintf(stdout, FMT1, prefix, "--------------------", "----", "---------", "-----");
/* macro for printing type size */
#define PT(mtype,maddr) sz=sizeof(mtype), as=alignment_size((char *)(maddr),sz), r=sz/as; (V)fprintf(stdout, FMT2 ,prefix, xbuildstr(mtype), (unsigned long)sz,(unsigned long)as,(unsigned long)r) /* caller supplies terminating semicolon */

    /* common standard (as of C99) types */
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(_Bool,&xb);
#endif /* C99 */
    PT(char,&xc);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(int_least8_t,&xl8);
    PT(int_fast8_t,&xf8);
#endif /* C99 */
    PT(unsigned char,&xuc);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(uint_least8_t,&xul8);
    PT(uint_fast8_t,&xuf8);
#endif /* C99 */
    PT(short,&xs);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(int_least16_t,&xl16);
    PT(int_fast16_t,&xf16);
#endif /* C99 */
    PT(unsigned short,&xus);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(uint_least16_t,&xul16);
    PT(uint_fast16_t,&xuf16);
#endif /* C99 */
    PT(int,&xi);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(int_least32_t,&xl32);
    PT(int_fast32_t,&xf32);
#endif /* C99 */
    PT(unsigned int,&xu);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(uint_least32_t,&xul32);
    PT(uint_fast32_t,&xuf32);
#endif /* C99 */
    PT(long,&xl);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(long long,&xll);
    PT(int_least64_t,&xl64);
    PT(int_fast64_t,&xf64);
#endif /* C99 */
    PT(unsigned long,&xul);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(unsigned long long,&xull);
    PT(uint_least64_t,&xul64);
    PT(uint_fast64_t,&xuf64);
#endif /* C99 */
    PT(float,&xf);
    PT(double,&xd);
#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    PT(long double,&xld);
    PT(float _Complex,&xfc);
    PT(double _Complex,&xdc);
    PT(long double _Complex,&xldc);
#endif /* C99 */
    PT(void *,&xptr);
    PT(size_t,&xsize_t);
    /* structure specific to this test program */
    PT(struct data_struct,&xstruct);

    /* print maximum date string */
    xl=LONG_MAX;
    duplicate_test_data(&xl,(char *)(&xstruct),DATA_TYPE_STRUCT,1UL,0UL,1UL);
    (V)fprintf(stdout,
        "latest date-time string for %ld is %s\n",
        xl, xstruct.string);
}
