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
* $Id: ~|^` @(#)    timing.c copyright 2016-2017 Bruce Lilly.   \ timing.c $
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
/* $Id: ~|^` @(#)   This is timing.c version 1.9 dated 2017-12-28T22:17:34Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.timing.c */

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
#define ID_STRING_PREFIX "$Id: timing.c ~|^` @(#)"
#define SOURCE_MODULE "timing.c"
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2017-12-28T22:17:34Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

static unsigned int timing_test(int type, size_t size, long *refarray, long *larray, int *array, double *darray, struct data_struct *data_array, struct data_struct **parray, const char *typename, int (*compar)(const void *, const void *), int (*compar_s)(const void *,const void *,void *), void (*swapf)(void *,void *, size_t), unsigned int options, const char *prog, unsigned int func, unsigned int testnum, size_t n, size_t count,  unsigned int *psequences, unsigned int *ptests, int col, double timeout, void (*f)(int, void *, const char *, ...), void *log_arg, unsigned char *flags, float **pwarray, float **puarray, float **psarray, size_t *marray, size_t *pdn)
{
    char buf[256], buf2[256], buf3[256], buf4[256], buf5[256], buf6[256],
        buf7[256], buf8[256], buf9[256], buf10[256], buf11[256], buf12[256],
        buf13[256], buf14[256], buf15[256], buf16[256], buf17[256], buf18[256],
        buf19[256], buf20[256], buf21[256], buf22[256], buf23[256], buf24[256],
        buf25[256], buf26[256], buf27[256], buf28[256], buf29[256], buf30[256],
        buf31[256], buf32[256];
    const char *pcc=NULL, *pfunc=NULL, *ptest="", *psize
#if SILENCE_WHINEY_COMPILERS
        = "gcc-fodder"
#endif
        ;
    const char *comment="";
    int c, i, odebug
#if SILENCE_WHINEY_COMPILERS
        =0
#endif
        ;
    unsigned int errs=0U, inst;
    long l;
    size_t b[10], cycle, k, *karray=NULL, m, nk=0UL, nops, ocount=count, t, u,
        v, w, x, y;
    size_t best_m
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , j
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        , worst_m
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        ;
    FILE *fp
#if SILENCE_WHINEY_COMPILERS
        = NULL
#endif
        ;
    size_t carray[MAX_PERMUTATION_SIZE], cmp, ct, nc
#if SILENCE_WHINEY_COMPILERS
        = 0UL
#endif
        ;
    size_t best_cmp=(SIZE_MAX), best_op=(SIZE_MAX), best_sw=(SIZE_MAX),
        best_moves=(SIZE_MAX), worst_cmp=0UL, worst_op=0UL, worst_sw=0UL,
        worst_moves=0UL;
    auto struct rusage rusage_start, rusage_end;
    auto struct timespec timespec_start, timespec_end, timespec_diff;
    float best_s=9.9e9, best_u=9.9e9, best_w=9.9e9, test_s, tot_s=0.0, test_u,
        tot_u=0.0, test_w, tot_w=0.0, worst_s=0.0, worst_u=0.0, worst_w=0.0;
    double d, factor
#if SILENCE_WHINEY_COMPILERS
        = 0.0
#endif
        , total_eq=0.0, total_gt=0.0, total_lt=0.0, total_sw=0.0,
        total_moves=0.0;
    void *pv;
    uint64_t max_val;

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
#if 0
(V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */ */\n",
__func__,source_file,__LINE__,errs);
#endif
    if (0U == errs) {
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
                (V)fprintf(stderr,
                    "/* %s: %s line %d: unrecognized data type %u */\n",
                    __func__, source_file, __LINE__, type);
            return ++errs;
        }
#if 0
(V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
#endif
        ptest = sequence_name(testnum);
        u = n - 1UL;
        k = n >>1; /* upper median */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
#endif
        if (0U==errs) {
            switch (testnum) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    nc=MAX_PERMUTATION_SIZE;
                    if (n>nc) return ++errs;
                    if (nc>n) nc=n;
                    cycle = factorial(nc);
                    count *= cycle;
                    ocount = count;
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
                    ocount = count;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
                default :
                    cycle = count;
                break;
            }
            /* preparation */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u, count=%lu */\n",
__func__,source_file,__LINE__,errs,count);
#endif
            if (0U != flags['i']) { ngt = nlt = neq = nsw = nmoves = 0UL; }
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: psarray=%p, *psarray=%p, count=%lu */\n",
__func__,source_file,__LINE__,(void *)psarray,NULL==psarray?NULL:(void *)(*psarray),count);
#endif
            *psarray=realloc(*psarray, sizeof(float)*count);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: psarray=%p, *psarray=%p */\n",
__func__,source_file,__LINE__,(void *)psarray,NULL==psarray?NULL:(void *)(*psarray));
#endif
            if (NULL==*psarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog,
__func__, source_file, __LINE__);
                return errs;
            }
            global_sarray = *psarray;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: puarray=%p, *puarray=%p, count=%lu */\n",
__func__,source_file,__LINE__,(void *)puarray,NULL==puarray?NULL:(void *)(*puarray),count);
#endif
            *puarray=realloc(*puarray, sizeof(float)*count);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: puarray=%p, *puarray=%p */\n",
__func__,source_file,__LINE__,(void *)puarray,NULL==puarray?NULL:(void *)(*puarray));
#endif
            if (NULL==*puarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog,
__func__, source_file, __LINE__);
                return errs;
            }
            global_uarray = *puarray;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: pwarray=%p, *pwarray=%p, count=%lu */\n",
__func__,source_file,__LINE__,(void *)pwarray,NULL==pwarray?NULL:(void *)(*pwarray),count);
#endif
            *pwarray=realloc(*pwarray, sizeof(float)*count);
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: pwarray=%p, *pwarray=%p */\n",
__func__,source_file,__LINE__,(void *)pwarray,NULL==pwarray?NULL:(void *)(*pwarray));
#endif
            if (NULL==*pwarray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m", prog,
__func__, source_file, __LINE__);
                return errs;
            }
            global_warray = *pwarray;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
            if (DEBUGGING(SORT_SELECT_DEBUG)) if (NULL!=f) f(LOG_INFO, log_arg, "%s %s line %d: %s %s %s",
__func__, source_file, __LINE__, pcc, typename, pfunc);
#endif /* DEBUG_CODE */
            /* prepare karray before timing for selection */
            switch (func) {
                case FUNCTION_QSELECT :
                    if ((1UL==selection_nk)&&(flags['m']=='m')) { /* special-case: 1 or 2 median(s) */
                        nk = 2UL;
                        karray = malloc(nk*sizeof(size_t));
                        if (NULL==karray) return ++errs;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: karray at %p through %p */\n",
__func__,source_file,__LINE__,(void *)karray, ((char *)karray)+sizeof(double)*nk);
#endif
                        karray[0] = u>>1; /* lower-median */
                        karray[1] = k; /* upper-median */
                    } else {
                        nk = selection_nk;
                        if (nk>n) nk=n;
                        karray = malloc(nk*sizeof(size_t));
                        if (NULL==karray) return ++errs;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: karray at %p through %p */\n",
__func__,source_file,__LINE__,(void *)karray, ((char *)karray)+sizeof(double)*nk);
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
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
if (DEBUGGING(SORT_SELECT_DEBUG)) {
(V)fprintf(stderr,"/* %s: %s line %d: %lu order statistic rank%s: %lu",
__func__,source_file,__LINE__,nk,nk==1?"":"s",karray[0]);
for (t=1UL; t<nk; t++)
(V)fprintf(stderr, ", %lu", karray[t]);
(V)fprintf(stderr, " */\n");
}
#endif
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
#endif
                break;
            }
            /* run tests */
            nmerges=npartitions=nrecursions=nrepivot=0UL;
            for (x=0UL; x<MAXROTATION; x++)
                nrotations[x] = 0UL;
            for (m=*pdn=0UL; m<count; m++) {
                switch (testnum) {
                    case TEST_SEQUENCE_ADVERSARY :
                        if (0UL==m) {
                            /* generate new test sequence */
                            if ((NULL!=f)&&(0u==flags[':'])) {
                                if (0U!=flags['K']) fprintf(stderr, "# ");
                                f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: preparing adverse input",
                                    prog, pcc, pfunc, typename, ptest);
                            }
                            v=nmerges,w=nrecursions,x=npartitions, y=nrepivot;
                            if (!(DEBUGGING(AQCMP_DEBUG))) odebug=debug, debug=0;
                            /* odebug is guaranteed to have been assigned a
                               value by the above statement when
                               DEBUGGING(AQCMP_DEBUG) whether or not (:-/) gcc's
                               authors realize it...
                            */
                            initialize_antiqsort(n, pv, type, size, refarray);
                            switch (func) {
                                case FUNCTION_QSELECT :      /*FALLTHROUGH*/
                                case FUNCTION_QSELECT_S :    /*FALLTHROUGH*/
                                case FUNCTION_QSELECT_SORT : /*FALLTHROUGH*/
                                case FUNCTION_QSORT_WRAPPER :/*FALLTHROUGH*/
                                case FUNCTION_SQSORT :       /*FALLTHROUGH*/
                                case FUNCTION_WQSORT :
                                    WQSORT(pv,0UL,u,size,aqcmp,karray,0UL,nk,0U);
                                break;
                                case FUNCTION_BMQSORT : /*FALLTHROUGH*/
                                case FUNCTION_NBQSORT : /*FALLTHROUGH*/
                                case FUNCTION_IBMQSORT :
                                    WBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
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
                                case FUNCTION_NETWORKSORT :
                                    NETWORKSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                break;
                                case FUNCTION_P9QSORT :
                                    P9QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
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
                            nmerges=v,nrecursions=w,npartitions=x,nrepivot=y;
                            if (!(DEBUGGING(AQCMP_DEBUG))) debug=odebug;
                            if ((NULL!=f)&&(0u==flags[':'])) {
                                if (0U!=flags['K']) fprintf(stderr, "# ");
                                f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: adverse input prepared",
                                    prog, pcc, pfunc, typename, ptest);
                            }
                        } else
                            restore_test_data(0UL,n,refarray,pv,type);
                            /* refarray should now contain an appropriate adverse sequence */
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        /* test number */
                        if (NULL != fp) {
                            c = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                                (int)n, f, log_arg);
                            (V)fprintf(fp, "%s", buf);
                            fflush(fp);
                        }
                        /* combinations of 0, 1 elements for this test */
                        for (ct=0UL; ct<nc; ct++) {
                            refarray[ct] = (long)(((m%cycle) >> (nc-ct-1UL)) & 0x01UL);
                        }
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                    case TEST_SEQUENCE_PERMUTATIONS :
                        /* test number */
                        if (NULL != fp) {
                            (V)fprintf(fp,"%lu/%lu %lu%% %lu%%          ",
                                m%cycle,cycle,(m%cycle)*100UL/cycle,
                                m*100UL/count);
                            fflush(fp);
                        }
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
                    case TEST_SEQUENCE_STDIN :               /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SORTED :              /*FALLTHROUGH*/
                    case TEST_SEQUENCE_REVERSE :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ORGAN_PIPE :          /*FALLTHROUGH*/
                    case TEST_SEQUENCE_INVERTED_ORGAN_PIPE : /*FALLTHROUGH*/
                    case TEST_SEQUENCE_ROTATED :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SHIFTED :             /*FALLTHROUGH*/
                    case TEST_SEQUENCE_SAWTOOTH :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_CONSTANT :            /*FALLTHROUGH*/
                    case TEST_SEQUENCE_MEDIAN3KILLER :       /*FALLTHROUGH*/
                    case TEST_SEQUENCE_DUAL_PIVOT_KILLER :   /*FALLTHROUGH*/
                    case TEST_SEQUENCE_JUMBLE :              /*FALLTHROUGH*/
                        /* computed, non-randomized sequences never change */
                        if (0UL==m) {
                            c=generate_long_test_array(refarray,
                                n, testnum, 1L, max_val, f, log_arg);
                            if (0 > c) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_array returned %d\n",
                                    __func__, source_file, __LINE__, c);
                                return ++errs;
                            } else if (0 < c) {
                                return errs;
                            }
                        } else
                            restore_test_data(0UL,n,refarray,pv,type);
                        /* copy test sequence to alternates */
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                    default :
                        /* generate new test sequence */
                        c=generate_long_test_array(refarray,
                            n, testnum, 1L, max_val, f, log_arg);
                        if (0 > c) {
                            (V)fprintf(stderr,
                                "%s: %s line %d: generate_long_test_array returned %d\n",
                                __func__, source_file, __LINE__, c);
                            return ++errs;
                        } else if (0 < c) {
                            return errs;
                        }
                        /* copy test sequence to alternates */
                        duplicate_test_data(refarray,pv,type,0UL,n);
                    break;
                }
                /* run and time the test */
                nlt=neq=ngt=nsw=nmoves = 0UL;
                test_s = test_u = test_w = 0.0;
                (V)getrusage(RUSAGE_SELF,&rusage_start);
                (V)clock_gettime(CLOCK_REALTIME,&timespec_start);
                switch (func) {
                    case FUNCTION_QSELECT_S :
                        QSEL_S(pv,0UL,u,size,compar_s,NULL,0UL,0UL,rpt);
                    break;
                    case FUNCTION_QSELECT_SORT :
                        QSEL(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_QSELECT :
                        QSEL(pv,0UL,u,size,compar,karray,0UL,nk,0U);
                    break;
                    case FUNCTION_BMQSORT :
                        BMQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_DEDSORT :
                        DEDSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_DPQSORT :
                        DPQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_GLQSORT :
                        GLQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_HEAPSORT :
                        HEAPSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_IBMQSORT :
                        IBMQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_INDIRECT_MERGESORT :
                        IMERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_INTROSORT :
                        INTROSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_ISORT :
                        ISORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_LOGSORT :
                        LOGSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_MBMQSORT :
                        MBMQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_MERGESORT :
                        MERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_MINMAXSORT :
                        MMSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_NBQSORT :
                        NBQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_NETWORKSORT :
                        NETWORKSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_P9QSORT :
                        P9QSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_QSORT :
                        QSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_QSORT_WRAPPER :
A(1UL<n);
A(u+1UL==n);
                        quickselect((char *)pv,n,size,compar,NULL,0UL,options);
                    break;
                    case FUNCTION_SELSORT :
                        SELSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_SHELLSORT :
                        SHELLSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_SMOOTHSORT :
                        SMOOTHSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_SQRTSORT :
                        SQRTSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_SQSORT :
                        SQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_SYSMERGESORT :
                        SYSMERGESORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_WQSORT :
                        WQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                    case FUNCTION_YQSORT :
                        YQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                    break;
                }
                (V)getrusage(RUSAGE_SELF,&rusage_end);
                (V)clock_gettime(CLOCK_REALTIME,&timespec_end);
                switch (testnum) {
                    case TEST_SEQUENCE_PERMUTATIONS :
                        if (NULL != fp) { fputc('\r', fp); fflush(fp); }
                    break;
                    case TEST_SEQUENCE_COMBINATIONS :
                        if (NULL != fp) { for (t=0UL; t<n; t++) { fputc('\b', fp); fflush(fp); } }
                    break;
                }
                test_u += (float)(rusage_end.ru_utime.tv_sec - rusage_start.ru_utime.tv_sec) + 1.0e-6 * (float)(rusage_end.ru_utime.tv_usec - rusage_start.ru_utime.tv_usec);
                if (test_u < 0.0) test_u = 0.0;
                tot_u += test_u;
                if (test_u > worst_u) worst_u = test_u, worst_m = m;
                if (test_u < best_u) best_u = test_u, best_m = m;
                test_s += (float)(rusage_end.ru_stime.tv_sec - rusage_start.ru_stime.tv_sec) + 1.0e-6 * (float)(rusage_end.ru_stime.tv_usec - rusage_start.ru_stime.tv_usec);
                if (test_s < 0.0) test_s = 0.0;
                tot_s += test_s;
                if (test_s > worst_s) worst_s = test_s;
                if (test_s < best_s) best_s = test_s;
                timespec_subtract(&timespec_end, &timespec_start, &timespec_diff);
                test_w = timespec_to_double(&timespec_diff);
                if (test_w < 0.0) test_w = 0.0;
                tot_w += test_w;
                if (test_w > worst_w) worst_w = test_w;
                if (test_w < best_w) best_w = test_w;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: errs=%u */\n",
__func__,source_file,__LINE__,errs);
#endif
                if (0UL<count) {
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: test_u=%G, dn=%lu, m=%lu, count=%lu */\n",
__func__,source_file,__LINE__,test_u,*pdn,m,count);
#endif
                    A(*pdn<count);
                    (*puarray)[*pdn] = test_u;
                    (*psarray)[*pdn] = test_s;
                    (*pwarray)[*pdn] = test_w;
                    (*pdn)++;
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: test_u=%G, dn=%lu */\n",
__func__,source_file,__LINE__,test_u,*pdn);
#endif
                }
                cmp = neq+ngt+nlt; /* XXX could overflow for very large nmemb (N log N > SIZE_MAX) or for adverse inputs (e.g. with quadratic performance and nmemb > sqrt(SIZE_MAX)) */
                if (cmp>worst_cmp) worst_cmp=cmp;
                if (cmp<best_cmp) best_cmp=cmp;
                if (nsw>worst_sw) worst_sw=nsw;
                if (nsw<best_sw) best_sw=nsw;
                nops = cmp+nsw;
                if (nops>worst_op) worst_op=nops;
                if (nops<best_op) best_op=nops;
                if (nmoves>worst_moves) worst_moves=nmoves;
                if (nmoves<best_moves) best_moves=nmoves;
                total_eq += (double)neq;
                total_gt += (double)ngt;
                total_lt += (double)nlt;
                total_sw += (double)nsw;
                total_moves += (double)nmoves;
                if (0U < errs) break;
                if ((tot_w > timeout)&&(m+1UL<count)) {
                    count = m; /* horrible performance; break out of loop */
                    if (NULL!= f)
                        f(LOG_INFO, log_arg,
                            "%s: %s %s %s %s timing test terminated early: %lu/%lu",
                            prog,pcc,pfunc,typename,ptest,
                            m,ocount);
                }
            } /* execute test count times */
            /* cleanup */
            if (NULL != karray) {
                free(karray);
                karray = NULL;
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
                    ocount=count; /* avoid "terminated early" warning */
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    if (NULL != fp) {
                        c = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                            (int)n, f, log_arg);
                        (V)fprintf(fp, "%s", buf);
                        fflush(fp);
                    }
                    if (NULL != fp) { fputc('\n', fp); fflush(fp); fclose(fp); fp=NULL; }
                    ocount=count; /* avoid "terminated early" warning */
                break;
            }
            if ((NULL!=f)&&(0u==flags[':'])) {
                if (0U!=flags['K']) fprintf(stderr, "# ");
                f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: %lu %lu completed: summarizing results", prog,
                    pcc, pfunc, typename, ptest, n, count);
            }
            /* count partitions, etc. before computing statistics */
            if (0U==flags[':']) { /* not silent for optimization runs */
                if (0U != flags['K']) comment="#";
                if (count == ocount) {
                    factor = test_factor(~(TEST_TYPE_ADVERSARY) & *ptests, n);
                    psize = test_size(~(TEST_TYPE_ADVERSARY) & *ptests);
                } else {
                    factor = test_factor(*ptests, n);
                    psize = test_size(*ptests);
                }
                buf29[0] = '\0';
                if ((0UL<npartitions)&&(0UL!=count))
                    (V)sng(buf29, sizeof(buf29), NULL,
                        npartitions==count?" partition":" partitions",
                        (double)npartitions/(double)count, -6, 3, f, log_arg);
                buf30[0] = '\0';
                if ((0UL<nrepivot)&&(0UL!=count))
                    (V)sng(buf30, sizeof(buf30), NULL,
                        nrepivot==count?" repivot":" repivots",
                        (double)nrepivot/(double)count, -6, 3, f, log_arg);
                buf31[0] = '\0';
                if ((0UL<nmerges)&&(0UL!=count))
                    (V)sng(buf31, sizeof(buf31), NULL,
                        nmerges==count?" merge":" merges",
                        (double)nmerges/(double)count, -6, 3, f, log_arg);
                buf32[0] = '\0';
                if ((0UL<nrecursions)&&(0UL!=count))
                    (V)sng(buf32, sizeof(buf32), NULL,
                        nrecursions==count?" recursion":" recursions",
                        (double)nrecursions/(double)count, -6, 3, f, log_arg);
            }
            /* find order statistics for times */
            if (0UL<*pdn) {
                /* marray may be modified by quickselect (sorting, duplicate removal) */
                b[0] = marray[0] = 0UL;                  /* min */
                b[1] = marray[1] = *pdn/50UL;            /* 2%ile */
                b[2] = marray[2] = *pdn/11UL;            /* 9%ile */
                b[3] = marray[3] = (*pdn)>>2;            /* 1st quartile */
#if DEBUG_CODE
#if 0
(V)fprintf(stderr,
"/* %s: %s line %d: dn=%lu, n=%lu, u=%lu, marray[3]=%lu, count=%lu */\n",
__func__, source_file,__LINE__, *pdn, n, u, marray[3], count);
#endif
#endif
                b[4] = marray[4] = (*pdn-1UL)>>1;        /* lower median */
                b[5] = marray[5] = (*pdn>>1);            /* upper median */
                b[6] = marray[6] = *pdn-1UL-((*pdn)>>2); /* 3rd quartile */
                b[7] = marray[7] = *pdn-1UL-(*pdn/11UL); /* 91%ile */
                b[8] = marray[8] = *pdn-1UL-(*pdn/50UL); /* 98%ile */
                b[9] = marray[9] = *pdn-1UL;             /* max */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: dn=%lu, n=%lu, u=%lu, b[4]=marray[4]=%lu, b[5]=marray[5]=%lu */\n",
__func__, source_file,__LINE__, *pdn, n, u, b[4], b[5]);
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: dn=%lu, n=%lu, u=%lu, b[0]=marray[0]=%lu, b[9]=marray[9]=%lu */\n",
__func__, source_file,__LINE__, *pdn, n, u, b[0], b[9]);
#endif /* DEBUG_CODE */
#if 0
                /* uninstrumented quickselect w/ uninstrumented comparison function */
                quickselect((void *)(*puarray),*pdn,sizeof(float),floatcmp,
                    NULL,marray,10UL,QUICKSELECT_NETWORK_MASK);
                quickselect((void *)(*psarray),*pdn,sizeof(float),floatcmp,
                    NULL,marray,10UL,QUICKSELECT_NETWORK_MASK);
                quickselect((void *)(*pwarray),*pdn,sizeof(float),floatcmp,
                    NULL,marray,10UL,QUICKSELECT_NETWORK_MASK);
#else
                /* internal quickselect w/ uninstrumented comparison function */
                inst = instrumented; instrumented=0U;
#if ASSERT_CODE
                for (w=0UL; w<10UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                x=npartitions,y=nrepivot;
                quickselect_internal((void *)(*puarray),*pdn,sizeof(float),
                    floatcmp,marray,10UL,QUICKSELECT_NETWORK_MASK,NULL,NULL);
#if ASSERT_CODE
                for (w=0UL; w<10UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                quickselect_internal((void *)(*psarray),*pdn,sizeof(float),
                    floatcmp,marray,10UL,QUICKSELECT_NETWORK_MASK,NULL,NULL);
#if ASSERT_CODE
                for (w=0UL; w<10UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                quickselect_internal((void *)(*pwarray),*pdn,sizeof(float),
                    floatcmp,marray,10UL,QUICKSELECT_NETWORK_MASK,NULL,NULL);
                npartitions=x,nrepivot=y;
#if ASSERT_CODE
                for (w=0UL; w<10UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                instrumented = inst;
#endif
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: quickselect returned */\n",
__func__, source_file,__LINE__);
#endif /* DEBUG_CODE */
                if (0U==flags[':']) { /* not silent for optimization runs */
                    if ((0U == errs)) {
                        /* summarize test results */
                        /* mean times */
                        test_u = tot_u / (double)m;
                        (V)sng(buf, sizeof(buf), NULL, NULL,
                            test_u / factor, -6, 3, f, log_arg); /* scaled mean user */
                        (V)sng(buf6, sizeof(buf6), NULL, NULL,
                            test_u, -6, 3, f, log_arg); /* mean user */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: tot_u=%G, factor=%G, count=%lu, mean=%G (%s) */\n",
__func__, source_file,__LINE__, tot_u, factor, count, test_u, buf);
#endif /* DEBUG_CODE */
                        test_s = tot_s / (double)m;
                        (V)sng(buf7, sizeof(buf7), NULL, NULL,
                            test_s / factor, -4, 3, f, log_arg); /* scaled mean system */
                        (V)sng(buf8, sizeof(buf8), NULL, NULL,
                            test_s, -4, 3, f, log_arg); /* mean system */
                        test_w = tot_w / (double)m;
                        (V)sng(buf9, sizeof(buf9), NULL, NULL,
                            test_w / factor, -4, 3, f, log_arg); /* scaled mean wall-clock */
                        (V)sng(buf10, sizeof(buf10), NULL, NULL,
                            test_w, -4, 3, f, log_arg); /* mean wall-clock */
                        c = 1 + snprintf(buf3, sizeof(buf3),
                            "%s %s of %lu %s%s: %s: (%lu * %lu)",
                            pcc, pfunc, n, typename, (n==1)?"":"s", ptest, n,
                            count);
                        (V)sng(buf4, sizeof(buf4), NULL, NULL,
                            tot_u, -4, 3, f, log_arg); /* total user */
                        (V)sng(buf15, sizeof(buf15), NULL, NULL,
                            tot_s, -4, 3, f, log_arg); /* total system */
                        (V)sng(buf16, sizeof(buf16), NULL, NULL,
                            tot_w, -4, 3, f, log_arg); /* total wall-clock */
                        test_u = ((*puarray)[b[4]] + (*puarray)[b[5]]) / 2.0;
                        (V)sng(buf2, sizeof(buf2), NULL, NULL,
                            test_u / factor, -4, 3, f, log_arg); /* scaled median user */
                        (V)sng(buf5, sizeof(buf5), NULL, NULL,
                            test_u, -4, 3, f, log_arg); /* median user */
                        (V)sng(buf17, sizeof(buf17), NULL, NULL,
                            (*puarray)[b[0]], -4, 3, f, log_arg); /* min user */
                        (V)sng(buf18, sizeof(buf18), NULL, NULL,
                            (*puarray)[b[9]], -4, 3, f, log_arg); /* max user */
                        test_s = ((*psarray)[b[4]] + (*psarray)[b[5]]) / 2.0;
                        (V)sng(buf11, sizeof(buf11), NULL, NULL,
                            test_s, -4, 3, f, log_arg); /* median system */
                        (V)sng(buf12, sizeof(buf12), NULL, NULL,
                            test_s / factor, -4, 3, f, log_arg); /* scaled median system */
                        (V)sng(buf19, sizeof(buf19), NULL, NULL,
                            (*psarray)[b[0]], -4, 3, f, log_arg); /* min sys */
                        (V)sng(buf20, sizeof(buf20), NULL, NULL,
                            (*psarray)[b[9]], -4, 3, f, log_arg); /* max sys */
                        test_w = ((*pwarray)[b[4]] + (*pwarray)[b[5]]) / 2.0;
                        (V)sng(buf13, sizeof(buf13), NULL, NULL,
                            test_w, -4, 3, f, log_arg); /* median wall-clock */
                        (V)sng(buf14, sizeof(buf14), NULL, NULL,
                            test_w / factor, -4, 3, f, log_arg); /* scaled median wall-clock */
                        (V)sng(buf21, sizeof(buf21), NULL, NULL,
                            (*pwarray)[b[0]], -4, 3, f, log_arg); /* min wall */
                        (V)sng(buf22, sizeof(buf22), NULL, NULL,
                            (*pwarray)[b[9]], -4, 3, f, log_arg); /* max wall */
#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
"/* %s: %s line %d: factor=%G, count=%lu, dn=%lu, median=%G (%s) */\n",
__func__, source_file,__LINE__, factor, count, *pdn, test_u, buf2);
#endif /* DEBUG_CODE */
                        if (0U!=flags['B']) {
                            /* wall-clock box plot data:
                               2%, 9%, 1/4, median, 3/4, 91%, 98%
                            */
                            (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                (*pwarray)[b[1]], -4, 3, f, log_arg); /* 2% wall */
                            (V)sng(buf24, sizeof(buf24), NULL, NULL,
                                (*pwarray)[b[2]], -4, 3, f, log_arg); /* 9% wall */
                            (V)sng(buf25, sizeof(buf25), NULL, NULL,
                                (*pwarray)[b[3]], -4, 3, f, log_arg); /* 1/4 wall */
                            (V)sng(buf26, sizeof(buf26), NULL, NULL,
                                (*pwarray)[b[6]], -4, 3, f, log_arg); /* 3/4 wall */
                            (V)sng(buf27, sizeof(buf27), NULL, NULL,
                                (*pwarray)[b[7]], -4, 3, f, log_arg); /* 91% wall */
                            (V)sng(buf28, sizeof(buf28), NULL, NULL,
                                (*pwarray)[b[8]], -4, 3, f, log_arg); /* 98% wall */
                            (V)printf(
                                "%s%s %s %s %s wall-clock box plot: %s %s %s %s %s %s %s\n",
                                comment,pcc,pfunc,ptest,typename,
                                buf23,buf24,buf25,buf13,buf26,buf27,buf28);
                        }
                        if (0U!=flags['R']) {
                            c = 1 + snprintf(buf3, sizeof(buf3), "%lu", n);
                            (V)printf("%s%s%*.*s%s %s user seconds per %s, %s total %s\n",
                                comment, buf3, col-c, col-c, " ",
                                0.0!=test_u?buf2:buf,
                                0.0!=test_u?"median":"mean",
                                psize,
                                0.0!=test_u?"median":"mean",
                                0.0!=test_u?buf5:buf6);
                            (V)printf("%s%s%*.*s%s %s system seconds per %s, %s total %s\n",
                                comment, buf3, col-c, col-c, " ",
                                0.0!=test_s?buf12:buf7,
                                0.0!=test_u?"median":"mean",
                                psize,
                                0.0!=test_u?"median":"mean",
                                0.0!=test_s?buf11:buf8);
                            (V)printf("%s%s%*.*s%s %s wall-clock seconds per %s, %s total %s\n",
                                comment, buf3, col-c, col-c, " ",
                                0.0!=test_w?buf14:buf9,
                                0.0!=test_u?"median":"mean",
                                psize,
                                0.0!=test_u?"median":"mean",
                                0.0!=test_w?buf13:buf10);
                        } else {
                            (V)printf(
                                "%s%s%*.*snormalized unit user times: %s (mean), %s (median), per %s, total user time %s%s, mean %s, median %s\n",
                                comment, buf3, col-c, col-c, " ", buf, buf2,
                                psize, buf4,
                                (count!=ocount)?" (terminated early)":"",
                                buf6, buf5);
                            if (0UL<*pdn) {
                                /* compare mean and median times, warn if there is a large discrepancy in user or wall-clock times */
                                d = tot_u / test_u / (double)m; /* ratio of mean to median */
                                if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                                if (d > DISCREPANCY_THRESHOLD)
                                    (V)fprintf(stderr,
                                        "%s%s %s User time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",
                                        comment,pcc,ptest,buf17,buf6,buf18,buf5,
                                        100.0*(d-1.0));
                            }
                            (V)printf(
                                "%s%s%*.*snormalized unit system times: %s (mean), %s (median), per %s, total system time %s%s, mean %s, median %s\n",
                                comment, buf3, col-c, col-c, " ", buf7, buf12,
                                psize, buf15,
                                (count!=ocount)?" (terminated early)":"",
                                buf8, buf11);
                            (V)printf(
                                "%s%s%*.*snormalized unit wall clock times: %s (mean), %s (median), per %s, total wall clock time %s%s, mean %s, median %s\n",
                                comment, buf3, col-c, col-c, " ", buf9, buf14,
                                psize, buf16,
                                (count!=ocount)?" (terminated early)":"",
                                buf10, buf13);
                            /* compare mean and median times, warn if there is
                               a large discrepancy in user or wall-clock times
                            */
                            d = tot_w / test_w / (double)m; /* ratio of mean to median */
                            if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                            if (d > DISCREPANCY_THRESHOLD)
                                (V)fprintf(stderr,
                                    "%s%s %s Wall-clock time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",
                                    comment,pcc,ptest,buf21,buf10,buf22,buf13,
                                    100.0*(d-1.0));
                            switch (testnum) {
                                case TEST_SEQUENCE_PERMUTATIONS : /*FALLTHROUGH*/
                                case TEST_SEQUENCE_COMBINATIONS :
                                    (V)snprintf(buf5, sizeof(buf5),
                                        " s/ %s %s\n", psize, typename);
                                    (V)sng(buf, sizeof(buf), NULL, buf5,
                                        best_u / factor, -4, 3, f, log_arg);
                                    c = 1 + snprintf(buf2, sizeof(buf2),
                                        "%s %s of %lu %s%s: %s: %s(%lu)",
                                        pcc, pfunc, n, typename, (n==1)?"":"s",
                                        ptest, "best", best_m);
                                    (V)printf("%s%s%*.*s%s", comment, buf2,
                                        col-c, col-c, " ", buf);
                                    (V)sng(buf, sizeof(buf), NULL, buf5,
                                        worst_u / factor, -4, 3, f, log_arg);
                                    c = 1 + snprintf(buf2, sizeof(buf2),
                                        "%s %s of %lu %s%s: %s: %s(%lu)", pcc,
                                        pfunc, n, typename, (n==1)?"":"s",
                                        ptest, "worst", worst_m);
                                    (V)printf("%s%s%*.*s%s", comment, buf2,
                                        col-c, col-c, " ", buf);
                                break;
                                case TEST_SEQUENCE_ADVERSARY :
                                    c = snul(buf, sizeof(buf), NULL,
                                        " total comparisons", antiqsort_ncmp, 10, '0',
                                        1, f, log_arg);
                                    c = snul(buf2, sizeof(buf2), NULL, " solid",
                                        (unsigned long)antiqsort_nsolid, 10,
                                        '0', 1, f, log_arg);
                                    c = 1 + snprintf(buf3, sizeof(buf3),
                                        "%s %s vs. %s:", pcc, pfunc, ptest);
                                    (V)printf("%s%s%*.*s%s, %s: %.6G %s\n",
                                        comment, buf3, col-c, col-c, " ", buf,
                                        buf2, (double)antiqsort_ncmp / factor,
                                        psize);
                                break;
                            }
                        }
                        if (0U != flags['i']) {
                            c = 1 + snprintf(buf, sizeof(buf), "%lu", n);
                            d = total_eq+total_gt+total_lt;
                            if (0U!=flags['R']) {
                                char bufb[256], bufw[256], buf0[256];
#if 0
fprintf(stderr,
 "/* %s%s: %s line %d: n=%lu, t=%lu, count=%lu, nsw=%lu */\n",comment,
__func__,source_file,__LINE__,n,t,count,nsw);
#endif
                                (V)snf(bufb, sizeof(bufb), NULL, NULL,
                                    best_cmp, '0', 1, -15, f, log_arg);
                                (V)snf(bufw, sizeof(bufw), NULL, NULL,
                                    worst_cmp, '0', 1, -15, f, log_arg);
                                (V)snf(buf0, sizeof(buf0), NULL, NULL,
                                    d / (double)count, '0', 1, -12, f, log_arg);
                                (V)printf("%s%s%*.*s%.6G %s mean comparisons [%s-%s] (%s)\n",
                                    comment, buf, col-c, col-c, " ",
                                    d / (double)count / factor,
                                    psize, bufb, bufw, buf0);
                            } else {
                                i = 1 + snprintf(buf3, sizeof(buf3),
                                    "%s %s vs. %lu %s %s total comparisons:",
                                    pcc, pfunc, n, typename, ptest);
                                (V)printf("%s%s%*.*s%.0f < (%.1f%%), %.0f == (%.1f%%), %.0f > (%.1f%%), mean %.0f: %.6G %s\n",
                                    comment, buf3, col-i, col-i, " ", total_lt,
                                    0.0==d? 100.0: 100.0*total_lt/d, total_eq,
                                    0.0==d? 100.0: 100.0*total_eq/d, total_gt,
                                    0.0==d? 100.0: 100.0*total_gt/d,
                                    d / (double)count, d / (double)count / factor, psize);
                            }
                            if (0.0<total_sw) {
                                char bufb[256], bufw[256], buf0[256];
                                d = total_sw / (double)count;
                                (V)snf(bufb, sizeof(bufb), NULL, NULL,
                                    (double)best_sw, '0', 1, -15, f, log_arg);
                                (V)snf(bufw, sizeof(bufw), NULL, NULL,
                                    (double)worst_sw, '0', 1, -15, f, log_arg);
                                (V)snf(buf0, sizeof(buf0), NULL, NULL,
                                    d, '0', 1, -15, f, log_arg);
                                (V)printf("%s%s%*.*s%.6G %s swaps [%s-%s] (%s)\n", comment,
                                    buf, col-c, col-c, " ",
                                    d / factor,
                                    psize, bufb, bufw, buf0);
                            }
                            if (0.0<total_sw) {
                                char bufb[256], bufw[256], buf0[256];
                                d = (total_eq+total_gt+total_lt+total_sw) / (double)count;
                                (V)snf(bufb, sizeof(bufb), NULL, NULL,
                                    best_op, '0', 1, -15, f, log_arg);
                                (V)snf(bufw, sizeof(bufw), NULL, NULL,
                                    worst_op, '0', 1, -15, f, log_arg);
                                (V)snf(buf0, sizeof(buf0), NULL, NULL,
                                    d, '0', 1, -15, f, log_arg);
                                (V)printf("%s%s%*.*s%.6G %s operations [%s-%s] (%s)\n", comment,
                                    buf, col-c, col-c, " ",
                                    d / factor,
                                    psize, bufb, bufw, buf0);
                            }
                            if (0.0<total_moves) {
                                char bufb[256], bufw[256], buf0[256];
                                d = total_moves / (double)count;
                                (V)snf(bufb, sizeof(bufb), NULL, NULL,
                                    (double)best_moves, '0', 1, -15, f, log_arg);
                                (V)snf(bufw, sizeof(bufw), NULL, NULL,
                                    (double)worst_moves, '0', 1, -15, f, log_arg);
                                (V)snf(buf0, sizeof(buf0), NULL, NULL,
                                    d, '0', 1, -15, f, log_arg);
                                (V)printf("%s%s%*.*s%.6G N = %.6G %s moves [%s-%s] (%s)\n", comment,
                                    buf, col-c, col-c, " ",
                                    d / (double)n, d / factor, psize,
                                    bufb, bufw, buf0);
                            }
                            for (x=1UL; x<MAXROTATION; x++) {
                                if (0UL!=nrotations[x]) {
                                    char buf0[256];
                                    d = (double)(nrotations[x])/(double)count;
                                    (V)snf(buf0, sizeof(buf0), NULL, NULL,
                                        d, '0', 1, -15, f, log_arg);
                                    (V)printf("%s%s%*.*s%.6G %s rotations of %lu elements (%s)\n", comment,
                                        buf, col-c, col-c, " ",
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
                                    buf, col-c, col-c, " ",
                                    d / factor,
                                    psize, MAXROTATION, buf0);
                            }
                            if ('\0'!=buf29[0])
                                (V)printf("%s%s%*.*s%s\n",comment,buf,col-c,col-c,
                                    " ",buf29);
                            if ('\0'!=buf30[0])
                                (V)printf("%s%s%*.*s%s (%.3f%%)\n",comment,buf,
                                    col-c,col-c," ",buf30,
                                    100.0*(double)nrepivot/(double)npartitions);
                            if ('\0'!=buf32[0])
                                (V)printf("%s%s%*.*s%s\n",comment,buf,col-c,col-c,
                                    " ",buf32);
                            if ('\0'!=buf31[0])
                                (V)printf("%s%s%*.*s%s\n",comment,buf,col-c,col-c,
                                    " ",buf31);
                        }
                        if (0U != flags['K']) {
                            switch (testnum) {
                                case TEST_SEQUENCE_ADVERSARY :
                                    (V)printf("#Adversary sequence:\n");
                                    for(j=0UL; j<n; j++) {
                                        (V)printf("%ld\n", refarray[j]);
                                    }
                                break;
                                case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                                    (V)printf("#Dual-pivot killer sequence:\n");
                                    for(j=0UL; j<n; j++) {
                                        /* take copied data from unsorted data set */
                                        switch (type) {
                                            case DATA_TYPE_LONG :
                                                l = strtol(data_array[j].string,NULL,STRING_BASE);
                                            break;
                                            default :
                                                l = larray[j];
                                            break;
                                        }
                                        (V)printf("%ld\n", l);
                                    }
                                break;
                                case TEST_SEQUENCE_MEDIAN3KILLER :
                                    (V)printf("#Median-of-3 killer sequence:\n");
                                    for(j=0UL; j<n; j++) {
                                        /* take copied data from unsorted data set */
                                        switch (type) {
                                            case DATA_TYPE_LONG :
                                                l = strtol(data_array[j].string,NULL,STRING_BASE);
                                            break;
                                            default :
                                                l = larray[j];
                                            break;
                                        }
                                        (V)printf("%ld\n", l);
                                    }
                                break;
                            }
                        }
                        fflush(stdout);
                    }
                }
            } /* test result summary if no errors */
        } /* test if no errors */
    } /* generate test sequence and test if no errors */
    return errs;
}

unsigned int timing_tests(int type, size_t size, long *refarray,
    long *larray, int *array, double *darray, struct data_struct *data_array,
    struct data_struct **parray, const char *typename,
    int (*compar)(const void *,const void *),
    int (*compar_s)(const void *,const void *,void *),
    void (*swapf)(void *,void *,size_t), unsigned int options, const char *prog,
    unsigned int func, size_t n, size_t count, unsigned int *psequences,
    unsigned int *ptests, int col, double timeout,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn)
{
    unsigned int errs=0U, testnum;
    const char *fname = function_name(func),
        *ftype = function_type(func, ptests);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* generated test sequences */
    for (testnum=0U; TEST_SEQUENCE_COUNT>testnum; testnum++) {
        /* permutations, combinations only for thorough tests */
        if (0==valid_test(TEST_TYPE_TIMING, testnum, n)) continue;
        if (0U == ((0x01U << testnum) & *psequences)) continue;
        if ((NULL!=f)&&(0u==flags[':'])) {
            if (0U!=flags['K']) fprintf(stderr, "# ");
            f(LOG_INFO, log_arg, "%s: %s %s %s %s timing test: %lu %lu", prog,
                fname, ftype, typename, sequence_name(testnum), n, count);
        }
        errs += timing_test(type, size, refarray, larray, array, darray,
            data_array, parray, typename, compar, compar_s, swapf, options,
            prog, func, testnum, n, count, psequences, ptests, col, timeout, f,
            log_arg, flags, pwarray, puarray, psarray, marray, pdn);
        if (0U != errs) break; /* abort on error */
    } /* loop through generated test sequences */
    return errs;
}
