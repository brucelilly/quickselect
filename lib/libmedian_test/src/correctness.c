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
* $Id: ~|^` @(#)    correctness.c copyright 2016-2018 Bruce Lilly.   \ correctness.c $
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
/* $Id: ~|^` @(#)   This is correctness.c version 1.27 dated 2018-04-18T04:08:56Z. \ $ */
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
#define MODULE_VERSION "1.27"
#define MODULE_DATE "2018-04-18T04:08:56Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* header files needed */
#include "median_test_config.h" /* configuration */

#include "initialize_src.h"

#if DEBUG_CODE + ASSERT_CODE
#if GENERATOR_TEST
static
QUICKSELECT_INLINE
size_t test_array_distinctness(const char *pv, size_t l, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, j;
    unsigned int odebug=debug;
    int (*compar)(const void *,const void *)=uninstrumented_comparator(icompar);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (NULL == pv) {
        if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: NULL pv", __func__, source_file, __LINE__);
        return u+1UL;
    }
    debug=0U;
    for (i=l; i<u; i++) {
        for (j=i+1UL; j<=u; j++) {
            c = OPT_COMPAR(pv+i*size, pv+j*size,options);
            if (0 == c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed distinctness comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)j);
                debug=odebug;
                return i;
            }
        }
    }
    debug=odebug;
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

QUICKSELECT_INLINE
size_t test_array_partition(const char *pv, size_t l, size_t pl,
    size_t pu, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    void(*f)(int, void *, const char *, ...), void *log_arg)
{
    int c;
    size_t i, n, lim;
    const char *p, *pe;
    unsigned int odebug=debug;
    int (*compar)(const void *,const void *)=uninstrumented_comparator(icompar);

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
    debug=0U;
    pe=pv+pl*size;
    if (pl >= l+1UL)
        for (i=pl-1UL; ; i--) {
            p=pv+i*size;
            c = OPT_COMPAR(p,pe,options);
            if (0 < c) {
                if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pl);
                debug=odebug;
                return i;
            }
            if (i < l+1UL) break;
        }
    for (i=pl+1UL; i<=pu; i++) {
        p=pv+i*size;
        c = OPT_COMPAR(p,pe,options);
        if (0 != c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)pl, (unsigned long)i);
            debug=odebug;
            return i;
        }
    }
    n = (u + 1UL - l);
    lim = l + n - 1UL;
    for (i=pu+1UL; i<=lim; i++) {
        p=pv+i*size;
        c = OPT_COMPAR(p,pe,options);
        if (0 > c) {
            if (NULL != f) f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at indices %lu, %lu", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            else fprintf(stderr, "/* %s: %s line %d: failed comparison at indices %lu, %lu */\n", __func__, source_file, __LINE__, (unsigned long)i, (unsigned long)pu);
            debug=odebug;
            return i;
        }
    }
    debug=odebug;
    return pl;
}

/* Test array for sorting correctness */
size_t test_array_sort(const char *pv, size_t l, size_t u, size_t size,
    int(*icompar)(const void *, const void *), unsigned int options,
    unsigned int distinct, void(*f)(int, void *, const char *, ...),
    void *log_arg)
{
    int c;
    size_t i;
    unsigned int odebug=debug;
    int (*compar)(const void *,const void *)=uninstrumented_comparator(icompar);

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
#if 0
    debug=0U;
#endif
    for (i=l; i<u; i++) {
        c = OPT_COMPAR(pv+i*size, pv+(i+1UL)*size,options);
        if (0 <= c) {
            if ((0U==distinct)&&(0==c)) continue; /* uniqueness not required */
            if (NULL != f)
                f(LOG_ERR, log_arg, "%s: %s line %d: failed comparison at index %lu", __func__, source_file, __LINE__, (unsigned long)i);
            debug=odebug;
            return i;
        }
    }
    debug=odebug;
    return u;
}

unsigned int correctness_tests(unsigned int sequences, unsigned int functions,
    unsigned int types, unsigned int options, const char *prog, size_t n,
    size_t ratio, size_t count, int col, double timeout, uint_least64_t *s,
    unsigned int p, unsigned int *plast_adv,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags)
{
    char buf[256], buf2[256];
    const char *comment="", *pcc, *pfunc, *typename, *psize, *ptest;
    int c, odebug;
    unsigned int distinct=0U, errs=0U, ff, function, rpt=flags['d'], ss,
        sequence, t, tests, tt, type;
    long l;
    double factor;
    size_t alignsize, carray[MAX_PERMUTATION_SIZE], ct, cycle, eql, equ, erro,
        errt, j, k, *karray=NULL, m, nc, nk, size, size_ratio, u, x, y;
    FILE *fp=NULL;
    void *pv;
    uint64_t max_val;
    int (*compar)(const void *, const void *);
    int (*compar_s)(const void *,const void *,void *);

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stderr,"/* %s(sequences=0x%x,functions=0x%x,types=0x%x,"
            "options=0x%x,prog=%s,n=%lu,ratio=%lu,count=%lu,col=%d,timeout=%.3f"
            ",...) */\n",__func__,sequences,functions,types,options,prog,n,
            ratio,count,col,timeout);
#endif
#if SILENCE_WHINEY_COMPILERS
    factor=0.0, nc=nk=erro=errt=0UL, odebug=debug;
#endif
    for (sequence=ls_bit_number(ss=sequences); sequence<32U; sequence=ls_bit_number(ss)) {
        ss &= ~(0x01<<sequence);
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: sequence=%u, ss=0x%x */\n",__func__,
                sequence,ss);
#endif
        ptest = sequence_name(sequence);
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: ptest=%s */\n",__func__,ptest);
#endif
        if (NULL == ptest) return ++errs;
        u = n - 1UL;
        k = n >>1; /* upper median */
        switch (sequence_is_randomized(sequence)) {
            case 0U : /* not randomized; set count to 1 */
                count=1UL;
            break;
            case 1U : /* randomized; do nothing here */
            break;
            default : /* error */
            return ++errs;
        }
        if (0U==errs) {
            switch (sequence) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    nc=MAX_PERMUTATION_SIZE;
                    if (n>nc) return ++errs;
                    if (nc>n) nc=n;
                    cycle = factorial(nc);
                    count *= cycle;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    nc=MAX_COMBINATION_SIZE;
                    if (n>=nc) return ++errs;
                    if (nc>n) nc=n;
                    cycle = 0x01UL << nc;
                    count *= cycle;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
                default :
                    cycle = count;
                break;
            }
            /* preparation */
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
            l=sysconf(_SC_PHYS_PAGES);
            if (0L>l) {
                (V)fprintf(stderr,
                   "%s: %s line %d: sysconf(_SC_PHYS_PAGES) returned %ld, "
                   "errno=%d(%s)\n",__func__,source_file,__LINE__,l,errno,
                   strerror(errno));
                x=268435456UL; /* 256MiB */
            } else {
                x=(size_t)l;
                l=sysconf(_SC_PAGESIZE);
                if (0L>l) {
                    (V)fprintf(stderr,
                       "%s: %s line %d: sysconf(_SC_PAGESIZE) returned %ld, "
                       "errno=%d(%s)\n",__func__,source_file,__LINE__,l,errno,
                       strerror(errno));
                    x=268435456UL; /* 256MiB */
                } else {
                    x*=(size_t)l;
                }
            }
#else
            x=268435456UL; /* 256MiB */
#endif
#define BELL_PAUSE_TIME 4
            y=0UL;
            if (NULL!=global_refarray) y+=sizeof(long)*n;
            if (NULL!=global_larray) y+=sizeof(long)*n*ratio;
            if (NULL!=global_iarray) y+=sizeof(int)*n*ratio;
            if (NULL!=global_sharray) y+=sizeof(short)*n*ratio;
            if (NULL!=global_darray) y+=sizeof(double)*n*ratio;
            if (NULL!=global_data_array) y+=sizeof(struct data_struct)*n*ratio;
            if (NULL!=global_parray) y+=sizeof(struct data_struct *)*n*ratio;
            if (y>=(x>>2)) {
                if (NULL==fp) {
                    /* warning to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                    else fp=stderr;
                }
                (V)sng(buf,sizeof(buf),NULL,NULL,(double)y,-4,3,f,log_arg);
                (V)fprintf(fp,
                    "%s: %s: \aWARNING: test data for ratio %lu requires %lu "
                    "(%s) bytes.\n"
                    "WARNING: Expect significant delays, or abort and specify "
                    "a smaller count or problem size.\n",
                    prog,__func__,(unsigned long)ratio,(unsigned long)y,buf);
                sleep(BELL_PAUSE_TIME); (V)fprintf(fp, "\a");
                sleep(BELL_PAUSE_TIME); (V)fprintf(fp, "\a");
                fflush(fp);
                if (fp==stderr) fp=NULL;
                sleep(BELL_PAUSE_TIME);
            }
#if DEBUG_CODE
            else if (DEBUGGING(MEMORY_DEBUG))
                (V)fprintf(stderr,
                    "/* %s: %s: test data for ratio %lu requires %lu bytes <= "
                    "physical memory %lu bytes. */\n",
                    prog,__func__,ratio,y,x);
#endif
            /* Generate non-random, count-independent, type-independent,
               function-independent sequence data here.
            */
            switch (sequence) {
                case TEST_SEQUENCE_STDIN :
                    duplicate_test_data(input_data,(char *)global_refarray,
                        DATA_TYPE_LONG,ratio,0UL,n);
                break;
                case TEST_SEQUENCE_ADVERSARY :
                    /* do nothing here; sequence is function-dependent */
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    /* do nothing here; sequence is count-dependent */
                break;
                case TEST_SEQUENCE_PERMUTATIONS :
                    /* do nothing here; sequence is count-dependent */
                break;
                default :
                    switch (sequence_is_randomized(sequence)) {
                        case 0U : /* not randomized; initialize */
                            c=generate_long_test_array(global_refarray,
                                n, sequence, 1L, LONG_MAX, f, log_arg);
                            if (0 > c) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_array returned %d\n",
                                    __func__, source_file, __LINE__, c);
                                return ++errs;
                            } else if (0 < c) {
                                return errs;
                            }
                        break;
                        case 1U : /* randomized; do nothing here */
                        break;
                        default : /* error */
                        return ++errs;
                    }
                break;
            }
            for (function=ls_bit_number(ff=functions); function<32U; function=ls_bit_number(ff)) {
                ff &= ~(0x01<<function);
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG))
                    (V)fprintf(stderr,"/* %s: function=%u, ff=0x%x */\n",
                        __func__,function,ff);
#endif
                pcc=function_name(function);
                if (NULL == pcc) return ++errs;
                else {
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: pcc=%s */\n",__func__,pcc);
#endif
                    tests=0U;
                    pfunc = function_type(function, &tests);
                    if (NULL == pfunc) return ++errs;
                    factor = test_factor(tests, n);
                    psize = test_size(tests);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: pcc=%s, pfunc=%s, factor=%.3f"
                            ", psize=%s, tests=0x%x */\n",__func__,pcc,pfunc,
                            factor,psize,tests);
#endif
                }
                /* prepare karray before timing for selection */
                switch (function) {
                    case FUNCTION_QSELECT_S : /*FALLTHROUGH*/
                    case FUNCTION_QSELECT :
                        if ((1UL==selection_nk)&&(flags['m']=='m')) { /* special-case: 1 or 2 median(s) */
                            nk = 2UL;
#if DEBUG_CODE > 1
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: allocating karray  %lu ranks */\n",
                                __func__,source_file,__LINE__,nk);
#endif
                            karray = malloc(nk*sizeof(size_t));
                            if (NULL==karray) return ++errs;
#if DEBUG_CODE > 1
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: karray at %p through %p */\n",
                                __func__,source_file,__LINE__,(void *)karray,
                                ((char *)karray)+sizeof(double)*nk);
#endif
                            karray[0] = u>>1; /* lower-median */
                            karray[1] = k; /* upper-median */
                        } else {
                            nk = selection_nk;
                            if (nk>n) nk=n;
#if DEBUG_CODE
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: allocating karray  %lu ranks */\n",
                                __func__,source_file,__LINE__,nk);
#endif
                            karray = malloc(nk*sizeof(size_t));
                            if (NULL==karray) return ++errs;
#if DEBUG_CODE
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: karray at %p through %p */\n",
                                __func__,source_file,__LINE__,(void *)karray,
                                ((char *)karray)+sizeof(double)*nk);
#endif
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
                        if (DEBUGGING(SORT_SELECT_DEBUG)) {
                            (V)fprintf(stderr,"/* %s: %s line %d: %lu order statistic rank%s: %lu",
                            __func__,source_file,__LINE__,nk,nk==1?"":"s",karray[0]);
                            for (t=1UL; t<nk; t++)
                            (V)fprintf(stderr, ", %lu", karray[t]);
                            (V)fprintf(stderr, " */\n");
                        }
#endif
                    break;
                }
                /* Generate non-random, type-independent, count-independent,
                   function-dependent sequence data here.
                   Adversary sequence is function-dependent.
                   Generate sequence for first large type.
                */
                switch (sequence) {
                    case TEST_SEQUENCE_ADVERSARY :
                        type=ls_bit_number(tt=types); /* types are in descending size order */
#if DEBUG_CODE
                        if (DEBUGGING(SORT_SELECT_DEBUG))
                            (V)fprintf(stderr,"/* %s: %s line %d: type=%u, tt=0x%x */\n",
                                __func__,source_file,__LINE__,type,tt);
#endif
                        typename=type_name(type);
#if DEBUG_CODE
                        if (DEBUGGING(SORT_SELECT_DEBUG))
                            (V)fprintf(stderr,"/* %s: typename=%s */\n",__func__,typename);
#endif
                        pv=type_array(type);
                        size=ratio*type_size(type);
#if GENERATOR_TEST
                        compar=type_comparator(type,flags); /* for generator tests */
#endif
                        /* generate new test sequence */
                        if ((NULL!=f)&&(0U==flags[':'])) {
                            if (0U!=flags['K']) fprintf(stderr, "# ");
                            if (1UL<ratio)
                                f(LOG_INFO, log_arg, "%s: %s %s %s*%lu %s "
                                    "timing test: preparing adverse input",
                                    prog, pcc, pfunc, typename, ratio,ptest);
                            else
                                f(LOG_INFO, log_arg, "%s: %s %s %s %s timing "
                                    "test: preparing adverse input",
                                    prog, pcc, pfunc, typename, ptest);
                            fflush(stderr);
                        }
                        if (!(DEBUGGING(AQCMP_DEBUG))) odebug=debug, debug=0;
                        /* odebug is guaranteed to have been assigned a
                           value by the above statement when
                           DEBUGGING(AQCMP_DEBUG) whether or not (:-/) gcc's
                           authors realize it...
                        */
                        switch (function) {
                            case FUNCTION_QSELECT :      /*FALLTHROUGH*/
                            case FUNCTION_QSELECT_S :    /*FALLTHROUGH*/
                            case FUNCTION_QSELECT_SORT : /*FALLTHROUGH*/
                            case FUNCTION_QSORT_WRAPPER :/*FALLTHROUGH*/
                            case FUNCTION_SQSORT :       /*FALLTHROUGH*/
                            case FUNCTION_WQSORT :
                                if (FUNCTION_WQSORT!=*plast_adv) {
                                    *plast_adv=FUNCTION_WQSORT;
                                    initialize_antiqsort(n,pv,type,ratio,size,
                                        global_refarray);
                                    WQSORT(pv,0UL,u,size,aqcmp,karray,0UL,nk,0U);
                                }
                            break;
                            case FUNCTION_BMQSORT : /*FALLTHROUGH*/
                            case FUNCTION_IBMQSORT :
                                if (FUNCTION_WBMQSORT!=*plast_adv) {
                                    *plast_adv=FUNCTION_WBMQSORT;
                                    initialize_antiqsort(n,pv,type,ratio,size,
                                        global_refarray);
                                    WBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                }
                            break;
                            case FUNCTION_NBQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                NBQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_DEDSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                DEDSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_DPQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                DPQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_GLQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                GLQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_HEAPSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                HEAPSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_INDIRECT_MERGESORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                IMERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_INTROSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                INTROSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_ISORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                ISORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_LOGSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                LOGSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_MBMQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                MBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_MERGESORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                MERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_MINMAXSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                MMSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_NETWORKSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                NETWORKSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_P9QSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                P9QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_QSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_RUNSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                RUNSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SELSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SELSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SHELLSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SHELLSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SMOOTHSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SMOOTHSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SQRTSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SQRTSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SYSMERGESORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SYSMERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_YQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                YQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                        }
                        if (!(DEBUGGING(AQCMP_DEBUG))) debug=odebug;
                        if ((NULL!=f)&&(0u==flags[':'])) {
                            if (0U!=flags['K']) fprintf(stderr, "# ");
                            if (1UL<ratio)
                                f(LOG_INFO, log_arg, "%s: %s %s %s*%lu %s "
                                    "timing test: adverse input prepared",
                                    prog,pcc,pfunc,typename,ratio,ptest);
                            else
                                f(LOG_INFO, log_arg, "%s: %s %s %s %s timing "
                                    "test: adverse input prepared",
                                    prog, pcc, pfunc, typename, ptest);
                            fflush(stderr);
                        }
                        /* refarray should now contain an appropriate adverse sequence */
                    break;
                    default :
                        /* do nothing here */
                    break;
                }
                for (type=ls_bit_number(tt=types); type<32U; type=ls_bit_number(tt)) {
                    tt &= ~(0x01<<type);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: %s line %d: type=%u, tt=0x%x */\n",
                            __func__,source_file,__LINE__,type,tt);
#endif
                    typename=type_name(type);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: typename=%s */\n",__func__,typename);
#endif
                    pv=type_array(type);
                    size=ratio*type_size(type);
                    alignsize=type_alignment(type);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: pv=%p, size=%lu, "
                            "alignsize=%lu, ratio=%lu */\n",__func__,pv,size,
                            alignsize,ratio);
#endif
                    if ((NULL==pv)||(0UL==alignsize)) {
                        (V)fprintf(stderr,"/* %s: %s line %d: sequence=%s, "
                            "function=%s, type=%s, ratio=%lu, pv=%p, size=%lu, "
                            "alignsize=%lu */\n",__func__,source_file,__LINE__,
                            ptest,pcc,typename,ratio,pv,size,alignsize);
                        return ++errs;
                    }
                    size_ratio=size/alignsize;
                    max_val=type_max(type);
                    compar=type_comparator(type,flags);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: compar=%s */\n",__func__,
                            comparator_name(compar));
#endif
                    compar_s=type_context_comparator(type,flags);
                    if (0U==flags[':']) {
                        if (0U!=flags['K']) fprintf(stderr, "# ");
                        if (NULL!=f) {
                            if (1UL<ratio)
                                f(LOG_INFO,log_arg,
                                    "%s: %s %s %s*%lu %s correctness test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ratio,ptest,n,count);
                            else
                                f(LOG_INFO,log_arg,
                                    "%s: %s %s %s %s correctness test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ptest,n,count);
                        } else {
                            if (1UL<ratio)
                                (V)fprintf(stderr,
                                    "%s: %s %s %s*%lu %s correctness test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ratio,ptest,n,count);
                            else
                                (V)fprintf(stderr,
                                    "%s: %s %s %s %s correctness test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ptest,n,count);
                        }
                        fflush(stderr);
                    }
                    if (0U != flags['i']) reset_counters(1U);
                    for (m=0UL; m<count; m++) {
                        /* Generate randomized, type-independent, count-dependent,
                           function-independent sequence data here.
                           Permutations and combinations are
                           function-independent and type-independent but
                           count-dependent.
                           Invariant sequences are copied from refarray.
                        */
                        /* preparation */
                        switch (sequence) {
                            case TEST_SEQUENCE_COMBINATIONS :
                                /* test number */
                                if (NULL!=fp) {
                                    c = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                                        (int)n, f, log_arg);
                                    (V)fprintf(fp,"%s %2lu%% %3lu%%",buf,
                                        ((m+1UL)%cycle)*100UL/cycle,(m+1UL)*100UL/count);
                                    fflush(fp);
                                }
                                /* combinations of 0, 1 elements for this test */
                                for (ct=0UL; ct<nc; ct++) {
                                    global_refarray[ct] = (long)(((m%cycle) >> (nc-ct-1UL)) & 0x01UL);
                                }
                            break;
                            case TEST_SEQUENCE_PERMUTATIONS :
                                /* test number */
                                if (NULL!=fp) {
                                    c=snprintf(buf2,sizeof(buf2),"%lu",cycle);
                                    (V)snul(buf,sizeof(buf),NULL,NULL,m%cycle,10,' ',
                                        c+1,f,log_arg);
                                    (V)fprintf(fp,"%s/%s %2lu%% %2lu%%",buf,buf2,
                                        (m%cycle)*100UL/cycle,m*100UL/count);
                                    fflush(fp);
                                }
                                if (0UL==m%cycle) {
                                    /* initial array is sorted sequence */
                                    c = generate_long_test_array(global_refarray, n,
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
                                    permute(global_refarray,0UL,u,carray,&ct);
                                }
                            break;
                            case TEST_SEQUENCE_ADVERSARY :
                                /* nothing to do here; sequence is prepared and will be duplicated */
                            break;
                            default :
                                switch (sequence_is_randomized(sequence)) {
                                    case 0U : /* not randomized; restore test data */
                                        /* computed, non-randomized sequences never change */
                                        /* nothing to do here; sequence is prepared and will be duplicated */
                                    break;
                                    case 1U : /* randomized; generate new sequence */
                                        /* generate new test sequence */
                                        c=generate_long_test_array(global_refarray,
                                            n, sequence, 1L, max_val, f, log_arg);
                                        if (0 > c) {
                                            (V)fprintf(stderr,
                                                "%s: %s line %d: generate_long_test_array returned %d\n",
                                                __func__, source_file, __LINE__, c);
                                            return ++errs;
                                        } else if (0 < c) {
                                            return errs;
                                        }
                                    break;
                                    default : /* error */
                                    return ++errs;
                                }
                            break;
                        } /* sequence switch */
do_test:
                        /* copy test sequence to typed array */
                        duplicate_test_data(global_refarray,pv,type,ratio,0UL,n);
                        /* long test data is in long array larray */
                        /* test sequence has been copied to others */
                        /* verify correctness of test sequences */
                        switch (sequence) {
                            case TEST_SEQUENCE_SORTED :               /*FALLTHROUGH*/
                                distinct=1U;
#if GENERATOR_TEST
                                t = test_array_sort(pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), distinct, f, log_arg);
                                if (t != u) {
                                    (V)fprintf(stderr, "ERROR ^^^^^: generator failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", ptest);
                                    errs++;
                                    erro = 0UL;
                                    if (t <= u) errt = t;
                                }
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
                                (V)fprintf(stderr, "/* %s: %s line %d: unrecognized sequence %u */\n", __func__, source_file, __LINE__, sequence);
                            return ++errs;
                        }
                        if ((2UL > count) && (MAX_ARRAY_PRINT > n) && (DEBUGGING(SORT_SELECT_DEBUG))) {
                            (V)fprintf(stderr, "initial %s array:\n", ptest);
                            print_some_array(pv,0UL, u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                        }
                        eql = equ = j = k;
                        if ((0UL==m)&&(NULL==f)) {
                            if (1UL<ratio)
                                (V)fprintf(stdout,"%s vs. %s*%lu %s: ",pcc,
                                    typename,ratio,ptest);
                            else
                                (V)fprintf(stdout,"%s vs. %s %s: ",pcc,typename,
                                    ptest);
                            fflush(stdout);
                        }
                        /* run test */
                        if (0U != flags['i']) reset_counters(0U);
                        switch (function) {
                            case FUNCTION_QSELECT_S :
                                QSEL_S(pv,0UL,u,size,compar_s,NULL,0UL,0UL,rpt);
                            break;
                            case FUNCTION_QSELECT_SORT :
                                QSEL(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
                            break;
                            case FUNCTION_QSELECT :
                                QSEL(pv,0UL,u,size,compar,karray,0UL,nk,rpt);
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
                                IMERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
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
                                A(1UL<n); A(u+1UL==n);
                                quickselect((char *)pv,n,size,compar,NULL,0UL,options);
                            break;
                            case FUNCTION_RUNSORT :
                                RUNSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,rpt);
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
                        } /* function switch */
                        switch (sequence) {
                            case TEST_SEQUENCE_COMBINATIONS : /*FALLTHROUGH*/
                            case TEST_SEQUENCE_PERMUTATIONS :
                                if (NULL!=fp) { fputc('\r', fp); fflush(fp); }
                            break;
                        }
                        update_counters(size_ratio);
                        if (0U < errs) break;
                        if ((0U == errs) && (TEST_TYPE_PARTITION == (TEST_TYPE_PARTITION & tests))) {
                            if (NULL!=karray) {
#if DEBUG_CODE
                                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                                    (V)fprintf(stderr, "/* %s: %s line %d, nk=%lu */\n", __func__,source_file,__LINE__,nk);
                                    print_some_array(pv,0UL, u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                                }
#endif /* DEBUG_CODE */
                                for (k=0UL; k<nk; k++) {
                                    size_t errl, erru, ll, ul, le, ue;
                                    char *pa, *pb, *pc, *pd, *pe;
                                    if (0UL==k) ll=0UL; else ll=karray[k-1UL];
                                    if (k==nk-1UL) ul=u; else ul=karray[k+1UL];
                                    le=ue=karray[k],pc=(char *)pv+ue*size;
#if DEBUG_CODE
                                    if (DEBUGGING(SORT_SELECT_DEBUG))
                                        (V)fprintf(stderr, "/* %s: %s line %d: karray[%lu]=%lu, ll=%lu, ul=%lu */\n", __func__,source_file,__LINE__,k,le,ll,ul);
#endif /* DEBUG_CODE */
                                    for (pa=(char *)pv+ll*size,pb=pc-size;pb>=pa;pb-=size,le--)
                                        if (0!=OPT_COMPAR(pb,pc,options&~(QUICKSELECT_INDIRECT))) break;
                                    for (pd=pc+size,pe=(char *)pv+ul*size;pd<pe;pd+=size,ue++)
                                        if (0!=OPT_COMPAR(pc,pd,options&~(QUICKSELECT_INDIRECT))) break;
#if DEBUG_CODE
                                    if (DEBUGGING(SORT_SELECT_DEBUG))
                                        (V)fprintf(stderr, "/* %s: %s line %d: karray[%lu]=%lu, le=%lu, ue=%lu */\n", __func__,source_file,__LINE__,k,karray[k],le,ue);
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
                                        print_some_array(pv,errl, erru, "/* "," */",options&~(QUICKSELECT_INDIRECT));
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
                        if ((0U == errs) && (TEST_TYPE_SORT == (TEST_TYPE_SORT & tests))) {
                            x = test_array_sort(pv, 0UL, u, size, compar, options&~(QUICKSELECT_INDIRECT), distinct, f, log_arg);
                            if (x != u) {
                                (V)fprintf(stderr, "ERROR ^^^^^: %s failed (test_array_sort) for %s ^^^^^ ERROR!!!!\n", pcc, ptest);
                                errs++;
                                erro = 0UL;
                                if (x <= u) errt = x;
                            }
                        }
                        if (0U < errs) break;
                        /* cleanup */
                    } /* count loop */
                    /* cleanup */
                    switch (sequence) {
                        case TEST_SEQUENCE_PERMUTATIONS :
                            if (NULL!=fp) {
                                c=snprintf(buf2,sizeof(buf2),"%lu",cycle);
                                (V)snul(buf,sizeof(buf),NULL,NULL,m%cycle,10,' ',
                                    c+1,f,log_arg);
                                (V)fprintf(fp,"%s/%s %2lu%% %2lu%%",buf,buf2,
                                    (m%cycle)*100UL/cycle,m*100UL/count);
                                fflush(fp);
                            }
                            if (NULL!=fp) { fputc('\n', fp); fflush(fp); }
                        break;
                        case TEST_SEQUENCE_COMBINATIONS :
                            if (NULL!=fp) {
                                c = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                                    (int)n, f, log_arg);
                                (V)fprintf(fp, "%s", buf);
                                fflush(fp);
                            }
                            if (NULL!=fp) { fputc('\n', fp); fflush(fp); }
                        break;
                    }
                    if (0U < errs) { /* error diagnostics */
                        (V)fprintf(stderr, "/* %s returned index %lu, range %lu through %lu */\n", pcc, j, eql, equ);
                        if (equ > u)
                            (V)fprintf(stderr, "ERROR ^^^^^: %s failed (equ %lu < u %lu) for %s ^^^^^ ERROR!!!!\n", pcc, equ, u, ptest);
                        if (MAX_ARRAY_PRINT > n) {
                            if (0UL != errt) print_some_array(pv,errt-1UL, errt+1UL, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                            print_some_array(pv,erro, erro+u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                        } else {
                            if (0UL != errt) print_some_array(pv,errt-1UL, errt+1UL, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                            else print_some_array(pv,0UL, 2UL, "/* "," */",options);
                            print_some_array(pv,0UL<eql?eql-1UL:eql, equ+1UL, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                        }
                        if (0U == rpt) {
                            restore_test_data(ratio,0UL,n,global_refarray,pv,type);
                            if (MAX_ARRAY_PRINT > n) {
                                (V)fprintf(stderr, "initial %s array:\n", ptest);
                                print_some_array(pv,0UL, u, "/* "," */",options&~(QUICKSELECT_INDIRECT));
                            }
                            errs = 0U;
                            rpt++;
                            goto do_test;
                        }
                    } else {
                        if (NULL==f) (V)fprintf(stdout, "passed\n");
                        if (0U!=flags['i']) {
                            if (0U != flags['K']) comment="#";
                            print_counters(comment,col,n,count,factor,pcc,pfunc,psize,
                                typename,ptest,flags,size_ratio,f,log_arg);
                        }
                        if (NULL!=f) {
                            if (0U!=flags['K']) fprintf(stderr, "# ");
                            if (1UL<ratio)
                                f(LOG_INFO, log_arg,
                                    "%s: %s %s %s*%lu %s correctness test: %lu "
                                    "%lu: passed",prog,pcc,pfunc,typename,ratio,
                                    ptest,n,count);
                            else
                                f(LOG_INFO, log_arg,
                                    "%s: %s %s %s %s correctness test: %lu %lu:"
                                    " passed",prog,pcc,pfunc,typename,ptest,n,
                                    count);
                        }
                        fflush(stdout);
                    }
                } /* types loop */
                /* cleanup */
                if (NULL!=fp) { fclose(fp); fp=NULL; }
                if (NULL != karray) {
#if DEBUG_CODE
                    if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                        "/* %s: %s line %d: freeing karray @ %p */\n",
                        __func__,source_file,__LINE__,(void *)karray);
#endif
                    free(karray);
                    karray = NULL;
                }
                /* Output test sequence if requested. */
                if (0U != flags['K']) {
                    switch (sequence) {
                        case TEST_SEQUENCE_ADVERSARY :
                            (V)printf("#Adversary sequence:\n");
                            for(j=0UL; j<n; j++) {
                                (V)printf("%ld\n", global_refarray[j]);
                            }
                        break;
                    }
                } /* test sequence output */
            } /* functions loop */
            /* Output test sequence if requested. */
            if (0U != flags['K']) {
                switch (sequence) {
                    case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                        (V)printf("#Dual-pivot killer sequence:\n");
                        for(j=0UL; j<n; j++) {
                            (V)printf("%ld\n", global_refarray[j]);
                        }
                    break;
                    case TEST_SEQUENCE_MEDIAN3KILLER :
                        (V)printf("#Median-of-3 killer sequence:\n");
                        for(j=0UL; j<n; j++) {
                            (V)printf("%ld\n", global_refarray[j]);
                        }
                    break;
                }
            } /* test sequence output */
        } /* no errors */
    } /* sequences loop */
    fflush(stdout);
    return errs;
}
