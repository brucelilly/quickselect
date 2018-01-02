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
* $Id: ~|^` @(#)    correctness.c copyright 2016-2017 Bruce Lilly.   \ correctness.c $
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
/* $Id: ~|^` @(#)   This is correctness.c version 1.10 dated 2017-12-28T22:17:34Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.correctness.c */

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
#define ID_STRING_PREFIX "$Id: correctness.c ~|^` @(#)"
#define SOURCE_MODULE "correctness.c"
#define MODULE_VERSION "1.10"
#define MODULE_DATE "2017-12-28T22:17:34Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* header files needed */
#include "median_test_config.h" /* configuration */

#include "initialize_src.h"

#if DEBUG_CODE + ASSERT_CODE
#if GENERATOR_TEST
size_t test_array_distinctness(const char *pv, size_t l, size_t u, size_t size, int(*compar)(const void *, const void *), unsigned int options, void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, j;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+1UL;
    }
    for (i=l; i<u; i++) {
        for (j=i+1UL; j<=u; j++) {
            c = OPT_COMPAR(pv+i*size, pv+j*size,options,/**/);
            if (0 == c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed distinctness comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)j);
                return i;
            }
        }
    }
    return u;
}
#endif /* GENERATOR_TEST */
#endif /* DEBUG_CODE + ASSERT_CODE */

/* Array pv should be partitioned such that elements at indices < pl are
      not larger than element at pl, elements at indices > pu are not smaller
      than element at index pu, and elements from index pl through pu compare
      as equal.
   l <= pl <= pu <= u
   If tests hold for all indices between l and u inclusive, return pl.
   If arguments are not valid, return >= u+1UL.
   On the first failed test, return the index of the item failing the test.
*/
size_t test_array_partition(const char *pv, size_t l, size_t pl,
    size_t pu, size_t u, size_t size,
    int(*compar)(const void *, const void *), unsigned int options,
    void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, n, lim;
    const char *p, *pe;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l > pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > pl (%lu)", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)pl);
        else fprintf(stderr, "/* %s: %s line %d: l (%lu) > pl (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)pl);
        return u+1UL;
    }
    if (pu > u) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) > u (%lu)", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)u);
        else fprintf(stderr, "/* %s: %s line %d: pu (%lu) > u (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)u);
        return u+2UL;
    }
    if (pu < pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) < pl (%lu)", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)pl);
        else fprintf(stderr, "/* %s: %s line %d: pu (%lu) < pl (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)pl);
        return u+3UL;
    }
    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        else fprintf(stderr, "/* %s: %s line %d: NULL pv */\n", __func__, source_file, __LINE__);
        return u+4UL;
    }
    pe=pv+pl*size;
    if (pl >= l+1UL)
        for (i=pl-1UL; ; i--) {
            p=pv+i*size;
            c = OPT_COMPAR(p,pe,options,/**/);
            if (0 < c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                return i;
            }
            if (i < l+1UL) break;
        }
    for (i=pl+1UL; i<=pu; i++) {
        p=pv+i*size;
        c = OPT_COMPAR(p,pe,options,/**/);
        if (0 != c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            return i;
        }
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    for (i=pu+1UL; i<=lim; i++) {
        p=pv+i*size;
        c = OPT_COMPAR(p,pe,options,/**/);
        if (0 > c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            return i;
        }
    }
    return pl;
}

#if 0
size_t test_array_partition_s(const char *pv, size_t l, size_t pl,
    size_t pu, size_t u, size_t size,
    int(*compar)(const void *, const void *,void *), void *context, unsigned int options,
    void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, n, lim;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l > pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > pl (%lu)", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)pl);
        else fprintf(stderr, "/* %s: %s line %d: l (%lu) > pl (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)pl);
        return u+1UL;
    }
    if (pu > u) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) > u (%lu)", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)u);
        else fprintf(stderr, "/* %s: %s line %d: pu (%lu) > u (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)u);
        return u+2UL;
    }
    if (pu < pl) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: pu (%lu) < pl (%lu)", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)pl);
        else fprintf(stderr, "/* %s: %s line %d: pu (%lu) < pl (%lu) */\n", __func__, source_file, __LINE__, (unsigned long)pu, (unsigned long)pl);
        return u+3UL;
    }
    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        else fprintf(stderr, "/* %s: %s line %d: NULL pv */\n", __func__, source_file, __LINE__);
        return u+4UL;
    }
    if (pl >= l+1UL)
        for (i=pl-1UL; ; i--) {
            c = OPT_COMPAR(pv+i*size, pv+pl*size, options, context);
            if (0 < c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                return i;
            }
            if (i < l+1UL) break;
        }
    for (i=pl+1UL; i<=pu; i++) {
        c = OPT_COMPAR(pv+i*size, pv+pl*size, options, context);
        if (0 != c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            return i;
        }
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    for (i=pu+1UL; i<=lim; i++) {
        c = OPT_COMPAR(pv+i*size, pv+pu*size, options, context);
        if (0 > c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            return i;
        }
    }
    return pl;
}
#endif

/* verify median:
      no more than n/2 elements < median
      no more than n/2 elements > median
      every element is <, ==, or > median
*/
size_t test_array_median(const char *pv, size_t l, size_t m, size_t u, size_t size, int(*compar)(const void *, const void *), unsigned int options, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, eq, gt, lt, lim, n;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l > m) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > m (%lu)", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)m);
        return u+1UL;
    }
    if (m > u) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: m (%lu) > u (%lu)", __func__, source_file, __LINE__, (unsigned long)m, (unsigned long)u);
        return u+2UL;
    }
    if (NULL == pv) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+3UL;
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    gt=lt=0UL,eq=1UL;
    if (m >= l+1UL)
        for (i=m-1UL; ; i--) {
            c = OPT_COMPAR(pv+i*size, pv+m*size,options,/**/);
            if (0 < c) gt++; else if (0 > c) lt++; else eq++;
            if (i < l+1UL) break;
        }
    for (i=m+1UL; i<=lim; i++) {
        c = OPT_COMPAR(pv+i*size, pv+m*size,options,/**/);
        if (0 < c) gt++; else if (0 > c) lt++; else eq++;
    }
    if (eq + gt + lt != n) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: eq %lu + gt %lu + lt %lu != n %lu", __func__, source_file, __LINE__, (unsigned long)eq, (unsigned long)gt, (unsigned long)lt, (unsigned long)n);
        return u+4UL;
    }
    if (gt > (n >> 1)) { /* n >> 1 is a fast version of n / 2 */
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: gt %lu > n %lu / 2", __func__, source_file, __LINE__, (unsigned long)gt, (unsigned long)n);
        return u+5UL;
    }
    if (lt > (n >> 1)) { /* n >> 1 is a fast version of n / 2 */
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: lt %lu > n %lu / 2", __func__, source_file, __LINE__, (unsigned long)lt, (unsigned long)n);
        return u+6UL;
    }
    return m;
}

/* Test array for sorting correctness */
size_t test_array_sort(const char *pv, size_t l, size_t u, size_t size, int(*compar)(const void *, const void *), unsigned int options, unsigned int distinct, void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l > u) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: l (%lu) > u (%lu)", __func__, source_file, __LINE__, (unsigned long)l, (unsigned long)u);
        return u+1UL;
    }
    if (NULL == pv) {
        if (NULL != f)
            f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+2UL;
    }
    for (i=l; i<u; i++) {
        c = OPT_COMPAR(pv+i*size, pv+(i+1UL)*size,options,/**/);
        if (0 <= c) {
            if ((0U==distinct)&&(0==c)) continue; /* uniqueness not required */
            if (NULL != f)
                f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at index %lu", __func__, source_file, __LINE__, (unsigned long)i);
            return i;
        }
    }
    return u;
}

static unsigned int correctness_test(int type, size_t size, long *refarray, long *larray, int *array, double *darray, struct data_struct *data_array, struct data_struct **parray, const char *typename, int (*compar)(const void *, const void *), int (*compar_s)(const void *,const void *,void *), void(*swapf)(void *,void *,size_t), unsigned int options, const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count, unsigned int *psequences, unsigned int *ptests, int col, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags)
{
    char buf[256], buf2[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="";
    const char *comment="";
    const char *psize = test_size(*ptests);
    int c, c1, odebug
#if SILENCE_WHINEY_COMPILERS
        = 0
#endif
        ;
    unsigned int distinct=0U, errs=0U, rpt=flags['d'], inst;
    size_t carray[MAX_PERMUTATION_SIZE], ct, cycle
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , eql, equ, k, *karray=NULL,
        m, nc
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , nk, t
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , u, v, w, x, y;
    size_t erro
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , errt = 0UL
        , j
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        ;
    FILE *fp
#if SILENCE_WHINEY_COMPILERS
        = NULL
#endif
        ;
    double d, d2, d3, factor;
    void *pv;
    uint64_t max_val;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U == ((0x01U << testnum) & *psequences)) return ++errs;
    pcc = function_name(func);
    if (NULL == pcc) {
        errs++;
    } else {
        pfunc = function_type(func, ptests);
        if (NULL == pfunc) {
            errs++;
        }
    }
    if (0U == errs) {
        inst = instrumented; instrumented = 0U;
        for (x=0UL; x<MAXROTATION; x++)
            nrotations[x] = 0UL;
        switch (type) {
            case DATA_TYPE_LONG :
                pv=larray;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_INT :
                pv=array;
                max_val = INT_MAX;
            break;
            case DATA_TYPE_DOUBLE :
                pv=darray;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_POINTER :
                pv=parray;
                max_val = LONG_MAX;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
                pv=data_array;
                max_val = LONG_MAX;
            break;
            default:
                (V)fprintf(stderr, "/* %s: %s line %d: unrecognized data type %u */\n", __func__, source_file, __LINE__, type);
            return ++errs;
        }
        ptest = sequence_name(testnum);
        u = n - 1UL;
        k = n>>1; /* upper median */
        /* generated test sequences */
        switch (testnum) {
            case TEST_SEQUENCE_STDIN :                /*FALLTHROUGH*/
            case TEST_SEQUENCE_SORTED :               /*FALLTHROUGH*/
            case TEST_SEQUENCE_ROTATED :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_SHIFTED :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_REVERSE :              /*FALLTHROUGH*/
                count = 1UL;
            break;
            case TEST_SEQUENCE_ORGAN_PIPE :           /*FALLTHROUGH*/
            case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :
                count = 1UL;
            break;
            case TEST_SEQUENCE_SAWTOOTH :
                count = 1UL;
            break;
            case TEST_SEQUENCE_BINARY :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_TERNARY :             /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH*/
            case TEST_SEQUENCE_JUMBLE :              /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_DISTINCT :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_ADVERSARY :
            break;
            case TEST_SEQUENCE_MANY_EQUAL_LEFT :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_RIGHT :    /*FALLTHROUGH*/
            case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :
                count = 1UL;                         /*FALLTHROUGH*/
            break;
            case TEST_SEQUENCE_CONSTANT :
            break;
            case TEST_SEQUENCE_DUAL_PIVOT_KILLER :   /*FALLTHROUGH*/
            case TEST_SEQUENCE_MEDIAN3KILLER :
                count = 1UL;
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL : /*FALLTHROUGH*/
            case TEST_SEQUENCE_HISTOGRAM :                /*FALLTHROUGH*/
            case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :
            break;
            case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED : /*FALLTHROUGH*/
            break;
            case TEST_SEQUENCE_RANDOM_VALUES :
            break;
            default:
                (V)fprintf(stderr, "/* %s: %s line %d: unrecognized testnum %u */\n", __func__, source_file, __LINE__, testnum);
            return ++errs;
        }
        if (0U == errs) {
            /* special count numbers, etc. */
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    nc=MAX_PERMUTATION_SIZE;
                    if (n>nc) return ++errs;
                    if (nc>n) nc=n;
                    cycle = factorial(nc);
                    count *= cycle;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    nc=MAX_COMBINATION_SIZE;
                    if (n>=nc) return ++errs;
                    if (nc>n) nc=n;
                    cycle = 0x01UL << nc;
                    count *= cycle;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
            }
            /* run tests */
            d=d2=d3=0.0;
            nmerges=npartitions=nrecursions=nrepivot=0UL;
            factor = test_factor(*ptests, n);
            for (m=0UL; m<count; m++) {
                instrumented = 0U;
                switch (testnum) {
                    case TEST_SEQUENCE_ADVERSARY :
                        if (0UL==m) {
                            /* generate test sequence */
                            if (!(DEBUGGING(AQCMP_DEBUG))) odebug=debug, debug=0;
                            /* odebug is guaranteed to have been assigned a
                               value by the above statement when
                               DEBUGGING(AQCMP_DEBUG) whether or not (:-/) gcc's
                               authors realize it...
                            */
                            v=nmerges,w=nrecursions,x=npartitions, y=nrepivot;
                            /* Initialization of antiqsort: place increasing
                               sequence in pv, initialize refarray to all "gas".
                            */
                            initialize_antiqsort(n, pv, type, size, refarray);
#if GENERATOR_TEST
                            if (1000UL > u) {
                                print_some_array(pv,0UL,u,"/* data: */\n/* "," */",options&~(QUICKSELECT_INDIRECT));
                                print_some_array(refarray,0UL,u,"/* antiqsort (all gas): */\n/* "," */",0U);
                            }
                            /* data in increasing order should pass sorted and distinctness tests */
                            t = test_array_sort(pv, 0UL, u, size, compar, 0U, 1U, logger, log_arg);
                            if (t != u) {
                                (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", ptest);
                                errs++;
                                erro = 0UL;
                                if (t <= u) errt = t;
                                if (1000UL > u) {
                                    print_some_array(pv,0UL<errt?errt-1UL:errt,errt<u-1UL?errt+1UL:errt,"/* "," */",options&~(QUICKSELECT_INDIRECT));
                                }
                            }
                            if (1000UL > u) {
                                t = test_array_distinctness((const char *)pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), f, log_arg);
                                if (t != u) {
                                    (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_distinctness) for %s ^^^^^ ERROR!!!! line %d\n", ptest, __LINE__);
                                    errs++;
                                    erro = 0UL;
                                    if (t <= u) errt = t;
                                    if (1000UL > u) {
                                        print_some_array(pv,0UL<errt?errt-1UL:errt,errt<u-1UL?errt+1UL:errt,"/* "," */",options&~(QUICKSELECT_INDIRECT));
                                    }
                                }
                            }
                            /* all-gas in refarray should pass sorted test */
                            t = test_array_sort((const char *)refarray, 0UL, u, sizeof(long), longcmp, 0U, 0U, logger, log_arg);
                            if (t != u) {
                                (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", ptest);
                                errs++;
                                erro = 0UL;
                                if (t <= u) errt = t;
                                if (1000UL > u) {
                                    print_some_array(refarray,0UL<errt?errt-1UL:errt,errt<u-1UL?errt+1UL:errt,"/* "," */",options&~(QUICKSELECT_INDIRECT));
                                }
                            }
#endif
                            /* run sorting function (even if selection is specified)
                               against adversary to generate adverse sequence in
                               refarray, without instrumented comparisons
                            */
                            switch (func) {
                                case FUNCTION_DEDSORT :
                                    DEDSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_DPQSORT :
                                    DPQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_GLQSORT :
                                    GLQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_HEAPSORT :
                                    HEAPSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_INDIRECT_MERGESORT :
                                    IMERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_INTROSORT :
                                    INTROSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_ISORT :
                                    ISORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_LOGSORT :
                                    LOGSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_MBMQSORT :
                                    MBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_MERGESORT :
                                    MERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_MINMAXSORT :
                                    MMSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_NBQSORT : /*FALLTHROUGH*/
                                case FUNCTION_BMQSORT : /*FALLTHROUGH*/
                                case FUNCTION_IBMQSORT :
                                    WBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_NETWORKSORT :
                                    NETWORKSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_P9QSORT :
                                    P9QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_QSELECT_SORT :  /*FALLTHROUGH*/
                                case FUNCTION_QSELECT_S :     /*FALLTHROUGH*/
                                case FUNCTION_QSELECT :       /*FALLTHROUGH*/
                                case FUNCTION_QSORT_WRAPPER : /*FALLTHROUGH*/
                                case FUNCTION_SQSORT :        /*FALLTHROUGH*/
                                case FUNCTION_WQSORT :
                                    WQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_QSORT :
                                    QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SELSORT :
                                    SELSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SHELLSORT :
                                    SHELLSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SMOOTHSORT :
                                    SMOOTHSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SQRTSORT :
                                    SQRTSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_SYSMERGESORT :
                                    SYSMERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_YQSORT :
                                    YQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                            }
                            /* Now pv is unsorted and refarray contains an
                               adverse sequence.  Both arrays should contain
                               unique values.
                            */
#if GENERATOR_TEST
                            (V)fprintf(stderr,"%s: %s line %d: n=%lu, antiqsort_nsolid=%lu\n",
                                __func__,source_file,__LINE__,n,antiqsort_nsolid);
                            if (1000UL > u) {
                                print_some_array(pv,0UL,u,"/* data: */\n/* "," */",options&~(QUICKSELECT_INDIRECT));
                                print_some_array(refarray,0UL,u,"/* adverse sequence: */\n/* "," */",0U);
                            }
                            if (1000UL > u) {
                                t = test_array_distinctness((const char *)pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), f, log_arg);
                                if (t != u) {
                                    (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_distinctness) for %s ^^^^^ ERROR!!!! %s line %d\n", ptest, source_file, __LINE__);
                                    errs++;
                                    erro = 0UL;
                                    if (t <= u) errt = t;
                                    if (1000UL > u) {
                                        print_some_array(pv,0UL<errt?errt-1UL:errt,errt<u-1UL?errt+1UL:errt,"/* "," */",options&~(QUICKSELECT_INDIRECT));
                                    }
                                }
                            } else
                                (V)fprintf(stderr, "generator passed (test_array_distinctness) for %s\n", ptest);
                            if (1000UL > u) {
                                t = test_array_distinctness((const char *)refarray, 0UL, u, sizeof(long), longcmp, 0U, f, log_arg);
                                if (t != u) {
                                    (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_distinctness) for %s ^^^^^ ERROR!!!! %s line %d\n", ptest, source_file, __LINE__);
                                    errs++;
                                    erro = 0UL;
                                    if (t <= u) errt = t;
                                    if (1000UL > u) {
                                        print_some_array(refarray,0UL<errt?errt-1UL:errt,errt<u-1UL?errt+1UL:errt,"/* "," */",0U);
                                    }
                                }
                            } else
                                (V)fprintf(stderr, "generator passed (test_array_distinctness) for %s refarray\n", ptest);
#endif /* GENERATOR_TEST */
                            if (0U != flags['i']) ngt=nlt=neq=nsw=nmoves=0UL;
                            if (!(DEBUGGING(AQCMP_DEBUG))) debug=odebug;
                            nmerges=v,nrecursions=w,npartitions=x,nrepivot=y;
                        } else
                            restore_test_data(0UL,n,refarray,pv,type);
                        /* refarray should now contain an appropriate adverse sequence */
                        duplicate_test_data(refarray,pv,type,0UL,n);
#if 1
                        if (1000UL > u) {
                            print_some_array(refarray,0UL,u,"/* adverse sequence: */\n/* "," */",0U);
                            print_some_array(pv,0UL,u,"/* data: */\n/* "," */",options&~(QUICKSELECT_INDIRECT));
                        }
#endif
                    break;
                    case TEST_SEQUENCE_PERMUTATIONS :
                        if (0UL==m%cycle) {
                            /* initial array is sorted sequence */
                            c = generate_long_test_array(refarray, n,
                                TEST_SEQUENCE_SORTED, 1L, max_val, f, log_arg);
                            if (0 > c) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_array returned %d\n",
                                    __func__, source_file, __LINE__, c);
                                return ++errs;
                            } else if (0 < c) {
                                return ++errs;
                            }
                            /* initialize for permutations */
                            for (ct=nc; 0UL<ct; carray[--ct]=0UL) ;
                        } else {
                            permute(refarray,0UL,u,carray,&ct);
                        }
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* combinations of 0, 1 elements for this test */
                        for (ct=0UL; ct<nc; ct++) {
                            refarray[ct] = (long)((m >> (nc-ct-1UL)) & 0x01UL);
                        }
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                    default :
                        /* generate new  test sequence */
                        c=generate_long_test_array(refarray,
                            n, testnum, 1L, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_array returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return ++errs;
                        }
                        /* copy test sequence to alternates */
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                }
                /* long test data is in long array larray */
                /* test sequence has been copied to others */
                /* verify correctness of test sequences */
                switch (testnum) {
                    case TEST_SEQUENCE_SORTED :               /*FALLTHROUGH*/
                        distinct=1U;
#if GENERATOR_TEST
                        t = test_array_sort(pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), distinct, logger, log_arg);
                        if (t != u) {
                            (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", ptest);
                            errs++;
                            erro = 0UL;
                            if (t <= u) errt = t;
                        }
                        if (0U != flags['i']) ngt = nlt = neq = nsw = nmoves = 0UL;
                    /*FALLTHROUGH*/
#endif /* GENERATOR_TEST */
                    case TEST_SEQUENCE_JUMBLE :                  /*FALLTHROUGH*/
                    case TEST_SEQUENCE_REVERSE :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ROTATED :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SHIFTED :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_DISTINCT :         /*FALLTHROUGH*/
                    case TEST_SEQUENCE_PERMUTATIONS :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_DUAL_PIVOT_KILLER :       /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MEDIAN3KILLER :           /*FALLTHROUGH*/
                        distinct=1U;
#if GENERATOR_TEST
                        if (1000UL > u) {
                            t = test_array_distinctness((const char *)pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), f, log_arg);
                            if (t != u) {
                                (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_distinctness) for %s ^^^^^ ERROR!!!! line %d\n", ptest, __LINE__);
                                errs++;
                                erro = 0UL;
                                if (t <= u) errt = t;
                            }
                        }
                        if (0U != flags['i']) ngt = nlt = neq = nsw = nmoves = 0UL;
#endif /* GENERATOR_TEST */
                    break;
                    case TEST_SEQUENCE_STDIN :                   /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ORGAN_PIPE :              /*FALLTHROUGH*/
                    case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :     /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SAWTOOTH :                /*FALLTHROUGH*/
                    case TEST_SEQUENCE_BINARY :                  /*FALLTHROUGH*/
                    case TEST_SEQUENCE_TERNARY :                 /*FALLTHROUGH*/
                    case TEST_SEQUENCE_COMBINATIONS :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_CONSTANT :                /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES :           /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :   /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :    /*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL :/*FALLTHROUGH*/
                    case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED :/*FALLTHROUGH*/
                    case TEST_SEQUENCE_HISTOGRAM :               /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_LEFT :         /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :       /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_RIGHT :        /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :     /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ADVERSARY :               /*FALLTHROUGH*/
                        distinct=0U;
                    break;
                    default:
                        (V)fprintf(stderr, "/* %s: %s line %d: unrecognized testnum %u */\n", __func__, source_file, __LINE__, testnum);
                    return ++errs;
                }
                if ((2UL > count) && (MAX_ARRAY_PRINT > n) && (DEBUGGING(SORT_SELECT_DEBUG))) {
                    (V)fprintf(stderr, "initial %s array:\n", ptest);
                    print_some_array(pv, 0UL, u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                }
                eql = equ = j = k;
                if ((0UL==m)&&(NULL==f)) {
                        (V)fprintf(stdout, "%s vs. %s %s: ", pcc, typename, ptest);
                        fflush(stdout);
                }
                switch (testnum) {
                    case TEST_SEQUENCE_PERMUTATIONS :
                        /* test number */
                        if (NULL != fp) {
                            (V)fprintf(fp,"%lu/%lu %lu%% %lu%%          ",
                                m%cycle,cycle,(m%cycle)*100UL/cycle,
                                m*100UL/count);
                            fflush(fp);
                        }
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* test number */
                        if (NULL != fp) {
                            c = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                                (int)n, f, log_arg);
                            (V)fprintf(fp, "%s", buf);
                            fflush(fp);
                        }
                    break;
                }
do_test:
                /* reset counters every time, as they are altered by correctness test comparisons */
                if (0U != flags['i']) ngt = nlt = neq = nsw = nmoves = 0UL;
                instrumented = inst;
                switch (func) {
                    case FUNCTION_QSELECT_S :
                        QSEL_S(pv,0UL,u,size,compar_s,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSELECT_SORT :  /*FALLTHROUGH*/
                        QSEL(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSELECT :
                        if ((1UL==selection_nk)&&(flags['m']=='m')) { /* special-case: 1 or 2 median(s) */
                            nk = 2UL;
                            karray = malloc(nk*sizeof(size_t));
                            if (NULL==karray) return ++errs;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s line %d: karray at %p through %p */\n",__func__,__LINE__,(void *)karray, (void *)(((char *)karray)+sizeof(double)*nk));
#endif /* DEBUG_CODE */
                            karray[0] = u>>1; /* lower-median */
                            karray[1] = n>>1; /* upper-median */
                            if (karray[0]==karray[1]) nk--;
                        } else {
                            nk = selection_nk;
                            if (nk>n) nk=n;
                            karray = malloc(nk*sizeof(size_t));
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s line %d: karray at %p through %p */\n",__func__,__LINE__,(void *)karray, (void *)(((char *)karray)+sizeof(double)*nk));
#endif /* DEBUG_CODE */
                            if (NULL==karray) return ++errs;
                            switch (flags['m']) {
                                case 'b' : /* beginning */ /*FALLTHROUGH*/
                                case 'l' : /* left */
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = (t<n)?t:u;
                                break;
                                case 'm' : /* middle */
                                    x = (n-nk)>>1;
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = t+x;
                                break;
                                case 'r' : /* right */ /*FALLTHROUGH*/
                                case 'e' : /* end */
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                break;
                                case 's' : /* separated */
                                    x = (nk>>1);
                                    for (t=0UL; t<x; t++)
                                        karray[t] = (t<n)?t:u;
                                    for (; t<nk; t++)
                                        karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                break;
                                case 'd' : /*FALLTHROUGH*/
                                default : /* distributed */
                                    y=n/nk;
                                    x=(n-(nk-1UL)*y)>>1;
                                    for (t=0UL; t<nk; t++,x+=y)
                                        karray[t] = x;
                                break;
                            }
                        }
#if DEBUG_CODE
if ((0UL==m)&&(DEBUGGING(SORT_SELECT_DEBUG))) {
(V)fprintf(stderr,"/* %s line %d: %lu order statistic rank%s: %lu",__func__,__LINE__,nk,nk==1?"":"s",karray[0]);
for (t=1UL; t<nk; t++)
(V)fprintf(stderr, ", %lu", karray[t]);
(V)fprintf(stderr, " */\n");
}
#endif
                        QSEL(pv,0UL,u,size,compar,karray,0UL,nk,rpt);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s line %d: quickselect_internal returned */\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_DEDSORT :
                        DEDSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_DPQSORT :
                        DPQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_HEAPSORT :
                        HEAPSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_IBMQSORT :
                        IBMQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_INDIRECT_MERGESORT :
                        IMERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_INTROSORT :
                        INTROSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_ISORT :
                        ISORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_LOGSORT :
                        LOGSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MBMQSORT :
                        MBMQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MERGESORT :
                        MERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_MINMAXSORT :
                        MMSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_NETWORKSORT :
                        NETWORKSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_P9QSORT :
                        P9QSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s line %d: calling quickselect */\n", __func__, __LINE__);
#endif /* DEBUG_CODE */
A(NULL!=pv);A(1UL<n);A(0UL<size);A(NULL!=compar);
A(u+1UL==n);
errno=0;
                        quickselect((char *)pv,n,size,compar,NULL,0UL,options);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s line %d: quickselect returned: errno=%d */\n", __func__, __LINE__,errno);
#endif /* DEBUG_CODE */
                    break;
                    case FUNCTION_SELSORT :
                        SELSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SHELLSORT :
                        SHELLSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SMOOTHSORT :
                        SMOOTHSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQRTSORT :
                        SQRTSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SQSORT :
                        SQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_SYSMERGESORT :
                        SYSMERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_WQSORT :
                        WQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_YQSORT :
                        YQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                    break;
                }
                if (0U!=flags['i']) {
                    t=nlt+neq+ngt;
#if 0
fprintf(stderr, "/* %s line %d: *ptests=0x%x */\n",__func__,__LINE__,*ptests);
#endif
                    d += (double)t / (double)count / factor;
                    d3 += (double)t / (double)count / factor;
                    if (0UL<nsw) {
                        d2 += (double)nsw / (double)count / factor;
                        d3 += (double)nsw / (double)count / factor;
                    }
#if 0
fprintf(stderr, "/* %s line %d: n=%lu, t=%lu, count=%lu, factor=%f, d=%f, nsw=%lu, d2=%f */\n",__func__,__LINE__,n,t,count,factor,d,nsw,d2);
#endif
                }
                if ((0U == errs) && (TEST_TYPE_PARTITION == (TEST_TYPE_PARTITION & *ptests))) {
                    if (NULL!=karray) {
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) { (V)fprintf(stderr, "/* %s: %s line %d, nk=%lu */\n", __func__,source_file,__LINE__,nk);
print_some_array(pv, 0UL, u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
}
#endif /* DEBUG_CODE */
                        for (k=0UL; k<nk; k++) {
                            size_t errl, erru, ll, ul, le, ue;
                            char *pa, *pb, *pc, *pd, *pe;
                            if (0UL==k) ll=0UL; else ll=karray[k-1UL];
                            if (k==nk-1UL) ul=u; else ul=karray[k+1UL];
                            le=ue=karray[k],pc=(char *)pv+ue*size;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s: %s line %d: karray[%lu]=%lu, ll=%lu, ul=%lu */\n", __func__,source_file,__LINE__,k,le,ll,ul);
#endif /* DEBUG_CODE */
                            for (pa=(char *)pv+ll*size,pb=pc-size;pb>=pa;pb-=size,le--)
                                if (0!=OPT_COMPAR(pb,pc,options&~(QUICKSELECT_INDIRECT),/**/)) break;
                            for (pd=pc+size,pe=(char *)pv+ul*size;pd<pe;pd+=size,ue++)
                                if (0!=OPT_COMPAR(pc,pd,options&~(QUICKSELECT_INDIRECT),/**/)) break;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s: %s line %d: karray[%lu]=%lu, le=%lu, ue=%lu */\n", __func__,source_file,__LINE__,k,karray[k],le,ue);
#endif /* DEBUG_CODE */
                            x = test_array_partition(pv,ll,le,ue,ul,size,
                                compar,options&~(QUICKSELECT_INDIRECT),f,log_arg);
                            if (x!=le) {
                                (V)fprintf(stderr,
                                    "ERROR ^^^^^: %s failed (test_array_partition) for %s ^^^^^ ERROR!!!! line %d\n",
                                    pcc, ptest, __LINE__);
                                if (x<le) errl=ll,erru=le; else errl=ue,erru=ul;
                                (V)fprintf(stderr,
                                    "ERROR ^^^^^: %s failure at %lu between %lu and %lu for order statistic %lu of %lu at rank %lu ^^^^^ ERROR!!!! line %d\n",
                                    pcc,x,errl,erru,k,nk,karray[k],__LINE__);
                                errs++;
                                erro = 0UL;
                                errt = x;
                                print_some_array(pv, errl, erru, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                                break;
                            }
                        }
                    } else {
                        x = test_array_partition(pv, 0UL, eql, equ, u, size, compar, options&~(QUICKSELECT_INDIRECT),f, log_arg);
                        if (x != eql) {
                            (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_partition) for %s ^^^^^ ERROR!!!! line %d\n", pcc, ptest, __LINE__);
                            errs++;
                            erro = 0UL;
                            if (x <= u) errt = x;
                        }
                    }
                }
                if ((0U == errs) && (TEST_TYPE_MEDIAN == (TEST_TYPE_MEDIAN & *ptests))) {
                    x = test_array_median(pv, 0UL, j, u, size, compar, options&~(QUICKSELECT_INDIRECT), f, log_arg);
                    if (x != j) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_median) for %s ^^^^^ ERROR!!!! line %d\n", pcc, ptest, __LINE__);
                        errs++;
                        erro = 0UL;
                        if (x <= u) errt = x;
                    } else if (j != k) k = j; /* some median functions do not move data */
                }
                if ((0U == errs) && (TEST_TYPE_SORT == (TEST_TYPE_SORT & *ptests))) {
                    x = test_array_sort(pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), distinct, logger, log_arg);
                    if (x != u) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", pcc, ptest);
                        errs++;
                        erro = 0UL;
                        if (x <= u) errt = x;
                    }
                }
                if (NULL!=karray) { free(karray); karray = NULL; }
                if (0U < errs) break;
                switch (testnum) {
                    case TEST_SEQUENCE_PERMUTATIONS :
                        if (NULL != fp) { fputc('\r', fp); fflush(fp); }
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        if (NULL != fp) { for (t=0UL; t<n; t++) { fputc('\b', fp); fflush(fp); } }
                    break;
                }
            }
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    if (NULL != fp) {
                        (V)fprintf(fp,"%lu/%lu %lu%% %lu%%          ",
                            m%cycle,cycle,(m%cycle)*100UL/cycle,
                            m*100UL/count);
                        fflush(fp);
                    }
                    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); fp=NULL; }
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    if (NULL != fp) {
                        c = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                            (int)n, f, log_arg);
                        (V)fprintf(fp, "%s", buf);
                        fflush(fp);
                    }
                    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); fp=NULL; }
                break;
            }
            if (0U < errs) { /* error diagnostics */
                (V)fprintf(stderr, "/* %s returned index %lu, range %lu through %lu */\n", pcc, j, eql, equ);
#if 0
                if (eql < 0UL)
                    (V)fprintf(stderr, "ERROR ^^^^^: %s failed (eql %lu < o %lu) for %s ^^^^^ ERROR!!!!\n", pcc, eql, 0UL, ptest);
#endif
                if (equ > u)
                    (V)fprintf(stderr, "ERROR ^^^^^: %s failed (equ %lu < u %lu) for %s ^^^^^ ERROR!!!!\n", pcc, equ, u, ptest);
                if (MAX_ARRAY_PRINT > n) {
                    if (0UL != errt) print_some_array(pv, errt-1UL, errt+1UL, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                    print_some_array(pv, erro, erro+u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                } else {
                    if (0UL != errt) print_some_array(pv, errt-1UL, errt+1UL, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                    else print_some_array(pv, 0UL, 2UL, "/* "," */",options);
                    print_some_array(pv, 0UL<eql?eql-1UL:eql, equ+1UL, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                }
                if (0U == rpt) {
                    restore_test_data(0UL,n,refarray,pv,type);
                    if (MAX_ARRAY_PRINT > n) {
                        (V)fprintf(stderr, "initial %s array:\n", ptest);
                        print_some_array(pv, 0UL, u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                    }
                    errs = 0U;
                    rpt++;
                    goto do_test;
                }
            } else {
                if (NULL==f) (V)fprintf(stdout, "passed\n");
                if (0U!=flags['i']) {
                    if (0U != flags['K']) comment="#";
#if 0
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr, "/* %s: %s line %d */\n", __func__,source_file,__LINE__);
#endif /* DEBUG_CODE */
fprintf(stderr, "/* %s%s line %d: n=%lu, t=%lu, count=%lu, d=%f, nsw=%lu, d2=%f */\n",comment,__func__,__LINE__,n,t,count,d,nsw,d2);
#endif
                    c1 = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                    if (0U!=flags['R']) {
                        (V)snf(buf2, sizeof(buf2), NULL, NULL, d*factor, '0', 1, -10, f, log_arg);
                        (V)printf("%s%s%*.*s%.6G %s comparisons (%s)\n", comment, buf, col-c1, col-c1, " ", d, psize, buf2);
                    } else {
                        c1 = 1 + snprintf(buf, sizeof(buf), "%s %s vs. %lu %s %s", pcc, pfunc, n, typename, ptest);
                        (V)printf("%s%s%*.*stotal %lu: %.6G %s\n", comment, buf, col-c1, col-c1, " ", t, d, psize);
                    }
                    if (0.0<d2) {
                        (V)snf(buf2, sizeof(buf2), NULL, NULL, d2*factor, '0',
                            1, -10, f, log_arg);
                        (V)printf("%s%s%*.*s%.6G %s swaps (%s)\n", comment, buf,
                            col-c1, col-c1, " ", d2, psize, buf2);
                        (V)snf(buf2, sizeof(buf2), NULL, NULL, d3*factor, '0',
                            1, -10, f, log_arg);
                        (V)printf("%s%s%*.*s%.6G %s operations (%s)\n", comment,
                            buf, col-c1, col-c1, " ", d3, psize, buf2);
                    }
                    for (x=1UL; x<MAXROTATION; x++) {
                        if (0UL!=nrotations[x]) {
                            char buf0[256];
                            d = (double)(nrotations[x])/(double)count;
                            (V)snf(buf0, sizeof(buf0), NULL, NULL,
                                d, '0', 1, -10, f, log_arg);
                            (V)printf("%s%s%*.*s%.6G %s rotations of %lu elements (%s)\n", comment,
                                buf, col-c1, col-c1, " ",
                                d / factor,
                                psize, x, buf0);
                        }
                    }
                    if (0UL!=nrotations[0]) {
                        char buf0[256];
                        d = (double)(nrotations[0])/(double)count;
                        (V)snf(buf0, sizeof(buf0), NULL, NULL,
                            d, '0', 1, -15, f, log_arg);
                        (V)printf("%s%s%*.*s%.6G %s rotations of >= %d elements (%s)\n", comment,
                            buf, col-c1, col-c1, " ",
                            d / factor,
                            psize, MAXROTATION, buf0);
                    }
                    if (0UL<npartitions) {
                        (V)printf("%s%s%*.*s%.6G partition%s\n", comment, buf, col-c1, col-c1, " ", (double)npartitions/(double)count, 1UL==npartitions?"":"s");
                    }
                    if (0UL<nrepivot) {
                        (V)printf("%s%s%*.*s%lu repivot%s (%.3f%%)\n", comment, buf, col-c1, col-c1, " ", nrepivot, 1UL==nrepivot?"":"s", 100.0*(double)nrepivot/(double)npartitions);
                    }
                    if (0UL<nrecursions) {
                        (V)printf("%s%s%*.*s%.6G recursion%s\n", comment, buf, col-c1, col-c1, " ", (double)nrecursions/(double)count, 1UL==nrecursions?"":"s");
                    }
                    if (0UL<nmerges) {
                        (V)printf("%s%s%*.*s%.6G merge%s\n", comment, buf, col-c1, col-c1, " ", (double)nmerges/(double)count, count==nmerges?"":"s");
                    }
                }
                fflush(stdout);
            }
        } /* run test if no errors */
    } /* generate test sequence and run test if no errors */
    return errs;
}

unsigned int correctness_tests(int type, size_t size, long *refarray,
    long *larray, int *array, double *darray, struct data_struct *data_array,
    struct data_struct **parray, const char *typename,
    int (*compar)(const void *,const void *),
    int (*compar_s)(const void *,const void *,void *),
    void (*swapf)(void *,void *,size_t), unsigned int options, const char *prog,
    unsigned int func, size_t n, size_t count, unsigned int *psequences,
    unsigned int *ptests, int col, double timeout, void (*f)(int, void *,
    const char *, ...), void *log_arg, unsigned char *flags, float **pwarray,
    float **puarray, float **psarray, size_t *marray, size_t *pdn)
{
    unsigned int errs=0U, testnum;
    const char *fname = function_name(func), *ftype = function_type(func, ptests);

    /* generated test sequences */
    for (testnum=0U; TEST_SEQUENCE_COUNT>testnum; testnum++) {
        /* MEDIAN3KILLER only for n >= 7 */
        /* SAWTOOTH, MANY_EQUAL only for n >= 5 */
        /* permutations, combinations only for thorough tests */
        /* no adversary test */
        if (0==valid_test(TEST_TYPE_CORRECTNESS, testnum, n)) continue;
        switch (testnum) {
            case TEST_SEQUENCE_PERMUTATIONS :        /*FALLTHROUGH*/
            case TEST_SEQUENCE_COMBINATIONS :
            break;
        }
        if (0U == ((0x01U << testnum) & *psequences)) continue;
        if (NULL!=f) {
            if (0U!=flags['K']) fprintf(stderr, "# ");
            f(LOG_INFO, log_arg, "%s: %s %s %s %s correctness test: %lu %lu",
                prog, fname, ftype, typename, sequence_name(testnum), n, count);
        }
        errs += correctness_test(type, size, refarray, larray, array, darray,
            data_array, parray, typename, compar, compar_s, swapf, options,
            prog, func, testnum, n, count, psequences, ptests, col, f, log_arg,
            flags);
        if (0U != errs) break; /* abort on error */
        if (NULL!=f) {
            if (0U!=flags['K']) fprintf(stderr, "# ");
            f(LOG_INFO, log_arg,
                "%s: %s %s %s %s correctness test: %lu %lu: passed",
                prog, fname, ftype, typename, sequence_name(testnum), n, count);
        }
    } /* loop through generated test sequences */
    return errs;
}
