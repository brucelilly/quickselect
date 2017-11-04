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
* $Id: ~|^` @(#)    operations.c copyright 2016-2017 Bruce Lilly.   \ operations.c $
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
/* $Id: ~|^` @(#)   This is operations.c version 1.2 dated 2017-10-03T07:28:05Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.operations.c */

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
#define ID_STRING_PREFIX "$Id: operations.c ~|^` @(#)"
#define SOURCE_MODULE "operations.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2017-10-03T07:28:05Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* time basic operations and print summary table */
__attribute__((optimize(0)))
void op_tests(const char *prefix, const char *prog,
    void (*f)(int, void *, const char *,...), void *log_arg)
{
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define OPTYPES 34
#else
# define OPTYPES 11
#endif /* C99 */
#define NOPS   34
    char buf[1024], numbuf[64];
    char testmatrix[OPTYPES][NOPS] = {      /* applicability matrix */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* _Bool */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* char */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_least8_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_fast8_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* unsigned char */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_least8_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_fast8_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* short */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_least16_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_fast16_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* unsigned short */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_least16_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_fast16_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* int */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_least32_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_fast32_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* unsigned */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_least32_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_fast32_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* long */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* long long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_least64_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* int_fast64_t */
#endif /* C99 */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy", /* unsigned long */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* unsigned long long */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_least64_t */
        "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyynyyy", /* uint_fast64_t */
#endif /* C99 */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnyyyy", /* float */
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnyyyy", /* double */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
        "yyyyyyyyyyyyynyyyyyyynnnnnnnnnnyyy", /* long double */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnyyy", /* float _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnyyy", /* double _Complex */
        "yyynnnnnyyyynnnnyyyyynnnnnnnnnnyyy", /* long double _Complex */
#endif /* C99 */
        "ynnnnnnyyyyyyyyynnnnnnnnnnnnnnnyyy", /* pointer */
    };
    const char *pcc, *pcc2;
    const char *typename[OPTYPES][2] = { /* 2 rows of text for type name */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "", "_Bool" },
#endif /* C99 */
       { "", "char" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "int_", "least8_t" }, { "int_", "fast8_t" },
#endif /* C99 */
       { "unsigned", "char" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "uint_", "least8_t" }, { "uint_", "fast8_t" },
#endif /* C99 */
       { "short", "int" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "int_", "least16_t" }, { "int_", "fast16_t" },
#endif /* C99 */
       { "unsigned", "short" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "uint_", "least16_t" }, { "uint_", "fast16_t" },
#endif /* C99 */
       { "plain", "int"},
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "int_", "least32_t" }, { "int_", "fast32_t" },
#endif /* C99 */
       { "unsigned", "int" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "uint_", "least32_t" }, { "uint_", "fast32_t" },
#endif /* C99 */
       { "long", "int" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "long", "long" },
       { "int_", "least64_t" }, { "int_", "fast64_t" },
#endif /* C99 */
       { "unsigned", "long" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "unsigned", "long long" },
       { "uint_", "least64_t" }, { "uint_", "fast64_t" },
#endif /* C99 */
       { "", "float" }, { "", "double" },
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
       { "long", "double" }, { "float", "_Complex" },
       { "double", "_Complex" }, { "long double", "_Complex" },
#endif /* C99 */
       { "", "pointer" }
    };
    unsigned int testnum, typenum;
    size_t i, j, k, l;
    double timing[OPTYPES][NOPS];
    FILE *fp;
    auto struct rusage rusage_start, rusage_end;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* flush output streams before writing to /dev/tty */
    fflush(stdout); fflush(stderr);
    /* Progress indication to /dev/tty */
    fp = fopen("/dev/tty", "w");
    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
    pcc = "Running tests, please be patient.  ";
    k = 1UL;
    pcc2="\b-\b\\\b|\b/"; /* "spinner" */
    l = 1UL;

    /* run tests, collect timing data */

/* macro to run one operation timing test */
/* Run (if possible) long enough to get sufficient accuracy with resolution of
   one microsecond (from struct rusage).
*/
#define MIN_OPS_TIME     0.02 /* 2*0.000001*10000 */
#define DESIRED_OPS_TIME 2.0*(MIN_OPS_TIME)

#undef MAX_COUNT
#if (SIZE_MAX) > 4294967295
# define MAX_COUNT 4294967295
#else
# define MAX_COUNT SIZE_MAX
#endif

#define M(op) timing[typenum][testnum]=0.0;                       \
    if ('y' == testmatrix[typenum][testnum]) {                    \
        double t;                                                 \
        register size_t count, n;                                 \
        for (count=65536UL; ;) {  /* 2^^16 */                     \
            if (NULL != fp) {                                     \
                for (j=0UL; j<k; j++) {                           \
                    fputc(*pcc, fp); fflush(fp);                  \
                    if (pcc[1] != '\0') {                         \
                        pcc++;                                    \
                    } else {                                      \
                        pcc = pcc2;                               \
                        k = l;                                    \
                    }                                             \
                }                                                 \
            }                                                     \
            (V)getrusage(RUSAGE_SELF,&rusage_start);              \
            for (n=0UL; n<count; n++) {                           \
                op ;                                              \
            }                                                     \
            (V)getrusage(RUSAGE_SELF,&rusage_end);                \
            t = ((double)(rusage_end.ru_utime.tv_sec -            \
            rusage_start.ru_utime.tv_sec) + 1.0e-6 *              \
            (double)(rusage_end.ru_utime.tv_usec -                \
            rusage_start.ru_utime.tv_usec));                      \
            if (((MIN_OPS_TIME)<=t)||((MAX_COUNT)<=count)) {      \
                timing[typenum][testnum]=t/(double)count;         \
                break;                                            \
            } else {                                              \
                if (0.0>t) t=0.0;                                 \
                if (0.0==t) {                                     \
                    if (((MAX_COUNT)>>14)>count) count <<= 14;    \
                    else count=(MAX_COUNT);                       \
                } else {                                          \
                    n=snlround((DESIRED_OPS_TIME)/t,f,log_arg);   \
                    if (2UL>n) n=2UL; /* *1 is NG */              \
                    if ((MAX_COUNT)/n>count) count *= n;          \
                    else count=(MAX_COUNT);                       \
                }                                                 \
            }                                                     \
        }                                                         \
    }

    /* Integer types: */

#undef VOL
#define VOL /**/

/* macro to run tests applicable to integer types */
#define I(type,v1,v2,v3,fcmp) { register type a; VOL type x=v1;   \
        VOL type y=v2; VOL type z=v3;                             \
        size_t sz=sizeof(type);                                   \
        void (*swapf)(char *, char *, size_t)                     \
            =swapn(alignment_size((char *)(&z),sizeof(type)));    \
        if (0 == z) z=1; /* z must not be zero! */                \
        testnum=0U;  M(x=y)                                       \
        testnum=1U;  M(x=(y==z))                                  \
        testnum=2U;  M(x=(y!=z))                                  \
        testnum=3U;  M(x=(y<z))                                   \
        testnum=4U;  M(x=(y>=z))                                  \
        testnum=5U;  M(x=(y<=z))                                  \
        testnum=6U;  M(x=(y>z))                                   \
        testnum=7U;  M(x=(y==z)?y:z)                              \
        testnum=8U;  M(x=(y!=z)?y:z)                              \
        testnum=9U;  M(x=(y<z)?y:z)                               \
        testnum=10U; M(x=(y>=z)?y:z)                              \
        testnum=11U; M(x=(y<=z)?y:z)                              \
        testnum=12U; M(x=(y>z)?y:z)                               \
        testnum=13U; M(x++)                                       \
        testnum=14U; M(x+=1)                                      \
        testnum=15U; M(x=x+1)                                     \
        testnum=16U; M(x+=y)                                      \
        testnum=17U; M(x=y+z)                                     \
        testnum=18U; M(x=y-z)                                     \
        testnum=19U; M(x=y*z)                                     \
        testnum=20U; M(x=y/z)                                     \
        testnum=21U; M(x=y%z)                                     \
        testnum=22U; M(x=!y)                                      \
        testnum=23U; M(x=~y)                                      \
        testnum=24U; M(x=y&z)                                     \
        testnum=25U; M(x=y|z)                                     \
        testnum=26U; M(x=y^z)                                     \
        testnum=27U; M(x=y<<z)                                    \
        testnum=28U; M(x=y>>z)                                    \
        testnum=29U; M(x=&y-&z)                                   \
        testnum=30U; M((x=(type)fcmp((void *)(&y),(void *)(&z)))) \
        testnum=31U; M((swapf((char *)(&y),(char *)(&z),sz)))     \
        testnum=32U; M(a=x;x=y;y=a)                               \
        testnum=33U; M(a=x;x=y;y=z;z=a)                           \
    }

    typenum=0U;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    /* _Bool type can be treated as an integer, if careful */
    I(_Bool,                0,    1,    1,    nocmp)             typenum++;
#endif /* C99 */
    I(char,                '0',  '1',  '2',   charcmp)           typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(int_least8_t,        '0',  '1',  '2',   int_least8_tcmp)   typenum++;
    I(int_fast8_t,         '0',  '1',  '2',   int_fast8_tcmp)    typenum++;
#endif /* C99 */
    I(unsigned char,       '0',  '1',  '2',   nocmp)             typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(uint_least8_t,       '0',  '1',  '2',   uint_least8_tcmp)  typenum++;
    I(uint_fast8_t,        '0',  '1',  '2',   uint_fast8_tcmp)   typenum++;
#endif /* C99 */
    I(short,                0,    1,    2,    shortcmp)          typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(int_least16_t,       '0',  '1',  '2',   int_least16_tcmp)  typenum++;
    I(int_fast16_t,        '0',  '1',  '2',   int_fast16_tcmp)   typenum++;
#endif /* C99 */
    I(unsigned short,       0U,   1U,   2U,   nocmp)             typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(uint_least16_t,      '0',  '1',  '2',   uint_least16_tcmp) typenum++;
    I(uint_fast16_t,       '0',  '1',  '2',   uint_fast16_tcmp)  typenum++;
#endif /* C99 */
    I(int,                  0,    1,    2,    intcmp)            typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(int_least32_t,       '0',  '1',  '2',   int_least32_tcmp)  typenum++;
    I(int_fast32_t,        '0',  '1',  '2',   int_fast32_tcmp)   typenum++;
#endif /* C99 */
    I(unsigned int,         0U,   1U,   2U,   nocmp)             typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(uint_least32_t,      '0',  '1',  '2',   uint_least32_tcmp) typenum++;
    I(uint_fast32_t,       '0',  '1',  '2',   uint_fast32_tcmp)  typenum++;
#endif /* C99 */
    I(long,                 0L,   1L,   2L,   longcmp)           typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(long long,            0LL,  1LL,  2LL,  nocmp)             typenum++;
    I(int_least64_t,       '0',  '1',  '2',   int_least64_tcmp)  typenum++;
    I(int_fast64_t,        '0',  '1',  '2',   int_fast64_tcmp)   typenum++;
#endif /* C99 */
    I(unsigned long,        0UL,  1UL,  2UL,  ulcmp)             typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    I(unsigned long long,   0ULL, 1ULL, 2ULL, nocmp)             typenum++;
    I(uint_least64_t,      '0',  '1',  '2',   uint_least64_tcmp) typenum++;
    I(uint_fast64_t,       '0',  '1',  '2',   uint_fast64_tcmp)  typenum++;
#endif /* C99 */

/* macro to run tests applicable to floating-point types */
#define F(type,v1,v2,v3,fcmp) { register type a; VOL type x=v1;           \
        VOL type y=v2; VOL type z=v3;                                     \
        size_t sz=sizeof(type);                                           \
        void (*swapf)(char *, char *, size_t)                             \
            =swapn(alignment_size((char *)(&z),sz));                      \
        if (0.0 == z) z = 1.0; /* z non-zero! */                          \
        testnum=0U;  M(x=y)                                               \
        testnum=1U;  M(x=(y==z))                                          \
        testnum=2U;  M(x=(y!=z))                                          \
        testnum=3U;  M(x=(y<z))                                           \
        testnum=4U;  M(x=(y>=z))                                          \
        testnum=5U;  M(x=(y<=z))                                          \
        testnum=6U;  M(x=(y>z))                                           \
        testnum=7U;  M(x=(y==z)?y:z)                                      \
        testnum=8U;  M(x=(y!=z)?y:z)                                      \
        testnum=9U;  M(x=(y<z)?y:z)                                       \
        testnum=10U; M(x=(y>=z)?y:z)                                      \
        testnum=11U; M(x=(y<=z)?y:z)                                      \
        testnum=12U; M(x=(y>z)?y:z)                                       \
        testnum=14U; M(x+=1)                                              \
        testnum=15U; M(x=x+1)                                             \
        testnum=16U; M(x+=y)                                              \
        testnum=17U; M(x=y+z)                                             \
        testnum=18U; M(x=y-z)                                             \
        testnum=19U; M(x=y*z)                                             \
        testnum=20U; M(x=y/z)                                             \
        testnum=30U; M((x=(type)fcmp((VOL void *)(&y),(VOL void *)(&z)))) \
        testnum=31U; M((swapf((VOL char *)(&y),(VOL char *)(&z),sz)))     \
        testnum=32U; M(a=x;x=y;y=a)                                       \
        testnum=33U; M(a=x;x=y;y=z;z=a)                                   \
    }

    /* Floating-point types: */
    F(float,                0.0, 1.0, 2.0, floatcmp)  typenum++;
    F(double,               0.0, 1.0, 2.0, doublecmp) typenum++;
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
    F(long double,          0.0, 1.0, 2.0, nocmp)     typenum++;
#endif /* C99 */

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
/* macro to run tests applicable to complex types */
#define C(type,v1,v2,v3) { register type a; VOL type x=v1;            \
        VOL type y=v2; VOL type z=v3;                                 \
        size_t sz=sizeof(type);                                       \
        void (*swapf)(char *, char *, size_t)                         \
            =swapn(alignment_size((char *)(&z),sizeof(type)));        \
        if (0.0 == z) z = 1.0; /* z non-zero! */                      \
        testnum=0U;  M(x=y)                                           \
        testnum=1U;  M(x=(y==z)?y:z)                                  \
        testnum=2U;  M(x=(y!=z)?y:z)                                  \
        testnum=8U;  M(x+=1)                                          \
        testnum=9U;  M(x=x+1)                                         \
        testnum=10U; M(x=(y==z))                                      \
        testnum=11U; M(x=(y!=z))                                      \
        testnum=16U; M(x=y+z)                                         \
        testnum=17U; M(x=y-z)                                         \
        testnum=18U; M(x=y*z)                                         \
        testnum=19U; M(x+=y)                                          \
        testnum=20U; M(x=y/z)                                         \
        testnum=31U; M((swapf((VOL char *)(&y),(VOL char *)(&z),sz))) \
        testnum=32U; M(a=x;x=y;y=a)                                   \
        testnum=33U; M(a=x;x=y;y=z;z=a)                               \
    }

    /* Complex types: */
    C(float _Complex,       0.0, 1.0, 2.0) typenum++;
    C(double _Complex,      0.0, 1.0, 2.0) typenum++;
    C(long double _Complex, 0.0, 1.0, 2.0) typenum++;
#endif /* C99 */

    /* Last, but not least: pointer */
    {
        VOL char array[3];
        VOL char VOL *x=array;
        VOL char VOL *y=array+1;
        VOL char VOL *z=array+2;
        register char *a;
        size_t sz=sizeof(char *);
        void (*swapf)(char *, char *, size_t)
            =swapn(alignment_size((char *)(&z),sz));

        /* Last test, clean up. */
        pcc = " \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b";
        k = 12UL;
        pcc2 = pcc;
        l = k;
        testnum=0U;  M(x=y)
        testnum=7U;  M(x=(y==z)?y:z)
        testnum=8U;  M(x=(y!=z)?y:z)
        testnum=9U;  M(x=(y<z)?y:z)
        testnum=10U; M(x=(y>=z)?y:z)
        testnum=11U; M(x=(y<=z)?y:z)
        testnum=12U; M(x=(y>z)?y:z)
        testnum=13U; M(x++)
        testnum=14U; M(x+=1)
        testnum=15U; M(x=x+1)
        testnum=31U; M((swapf((VOL char *)(&y),(VOL char *)(&z),sz)))
        testnum=32U; M(a=x;x=y;y=a)
        testnum=33U; M(a=x;x=y;y=z;z=a)
    }

    /* Finished with /dev/tty */
    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); }

    /* print headings for results table */
    (V)fprintf(stdout, "%s%s: %s (%s %s %s) COMPILER_USED=\"%s\" __STDC_VERSION__=%ld\n",
        prefix, prog, HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED, MEDIAN_STDC_VERSION);
    (V)fprintf(stdout, "%sOperations: mean time per operation\n", prefix);
#define OPWIDTH  16 /* long enough for longest operation string */
#define NUMWIDTH 11 /* long enough for longest type name, longest result string */
    for (i=0UL; 2UL>i; i++) {
        memset(buf, ' ', sizeof(buf));
        if (1UL == i) {
            pcc = "operation";
            k = strlen(pcc);
            j = (OPWIDTH - k) / 2UL;
            while ('\0' != *pcc) { buf[j++] = *pcc++; }
        }
        for (typenum=0U; typenum<OPTYPES; typenum++) {
            pcc = typename[typenum][i];
            k = strlen(pcc);
            j = OPWIDTH + NUMWIDTH * typenum + (NUMWIDTH - k) / 2UL;
            while ('\0' != *pcc) { buf[j++] = *pcc++; }
        }
        j = OPWIDTH + NUMWIDTH * OPTYPES;
        while (' ' == buf[--j]) { buf[j] = '\0'; }
        (V)fprintf(stdout, "%s%s\n", prefix, buf);
    }
    /* print operation name and timing data */

/* macro to print one row of data, corresponding to one test operation */
#define P(op) pcc = buildstr(op);                                  \
    memset(buf, ' ', sizeof(buf));                                 \
    k = strlen(pcc);                                               \
    j = (OPWIDTH - k) / 2UL;                                       \
    while ('\0' != *pcc) { buf[j++] = *pcc++; }                    \
    for (typenum=0U; typenum<OPTYPES; typenum++) {                 \
        if ('y' != testmatrix[typenum][testnum]) {                 \
            pcc = "N/A";                                           \
        } else {                                                   \
            (V)sng(numbuf, sizeof(numbuf), NULL, NULL,             \
                timing[typenum][testnum], -4, 3, logger, log_arg); \
            pcc = numbuf;                                          \
        }                                                          \
        k = strlen(pcc);                                           \
        j = OPWIDTH + NUMWIDTH * typenum + (NUMWIDTH - k) / 2UL;   \
        while ('\0' != *pcc) { buf[j++] = *pcc++; }                \
    }                                                              \
    j = OPWIDTH + NUMWIDTH * OPTYPES;                              \
    while (' ' == buf[--j]) { buf[j] = '\0'; }                     \
    (V)fprintf(stdout, "%s%s\n", prefix, buf);

    testnum=0U;  P(x=y)
    testnum=1U;  P(x=(y==z))
    testnum=2U;  P(x=(y!=z))
    testnum=3U;  P(x=(y<z))
    testnum=4U;  P(x=(y>=z))
    testnum=5U;  P(x=(y<=z))
    testnum=6U;  P(x=(y>z))
    testnum=7U;  P(x=(y==z)?y:z)
    testnum=8U;  P(x=(y!=z)?y:z)
    testnum=9U;  P(x=(y<z)?y:z)
    testnum=10U; P(x=(y>=z)?y:z)
    testnum=11U; P(x=(y<=z)?y:z)
    testnum=12U; P(x=(y>z)?y:z)
    testnum=13U; P(x++)
    testnum=14U; P(x+=1)
    testnum=15U; P(x=x+1)
    testnum=16U; P(x+=y)
    testnum=17U; P(x=y+z)
    testnum=18U; P(x=y-z)
    testnum=29U; P(x=y*z)
    testnum=10U; P(x=y/z)
    testnum=21U; P(x=y%z)
    testnum=22U; P(x=!y)
    testnum=23U; P(x=~y)
    testnum=24U; P(x=y&z)
    testnum=25U; P(x=y|z)
    testnum=26U; P(x=y^z)
    testnum=27U; P(x=y<<z)
    testnum=28U; P(x=y>>z)
    testnum=39U; P(x=&y-&z)
    testnum=20U; P(compar)
    testnum=31U; P(swapf)
    testnum=32U; P(a=x;x=y;y=a)
    testnum=33U; P(a=x;x=y;y=z;z=a)
}
