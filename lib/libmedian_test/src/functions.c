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
* $Id: ~|^` @(#)    functions.c copyright 2016-2017 Bruce Lilly.   \ functions.c $
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
/* $Id: ~|^` @(#)   This is functions.c version 1.5 dated 2017-12-28T22:17:34Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.functions.c */

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
#define ID_STRING_PREFIX "$Id: functions.c ~|^` @(#)"
#define SOURCE_MODULE "functions.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2017-12-28T22:17:34Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* name of the type of test for a particular function
   side-effect: sets appropriate tests, unsets inappropriate tests
*/
const char *function_type(unsigned int func, unsigned int *ptests)
{
    const char *ret="unknown function_type";

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (func) {
        case FUNCTION_QSELECT :
            *ptests |= ( TEST_TYPE_PARTITION );
            ret = "selection";
        break;
        case FUNCTION_QSELECT_S :          /*FALLTHROUGH*/
        case FUNCTION_QSELECT_SORT :       /*FALLTHROUGH*/
        case FUNCTION_QSORT_WRAPPER :      /*FALLTHROUGH*/
        case FUNCTION_BMQSORT :            /*FALLTHROUGH*/
        case FUNCTION_DEDSORT :            /*FALLTHROUGH*/
        case FUNCTION_DPQSORT :            /*FALLTHROUGH*/
        case FUNCTION_GLQSORT :            /*FALLTHROUGH*/
        case FUNCTION_HEAPSORT :           /*FALLTHROUGH*/
        case FUNCTION_IBMQSORT :           /*FALLTHROUGH*/
        case FUNCTION_INDIRECT_MERGESORT : /*FALLTHROUGH*/
        case FUNCTION_INTROSORT :          /*FALLTHROUGH*/
        case FUNCTION_ISORT :              /*FALLTHROUGH*/
        case FUNCTION_LOGSORT :            /*FALLTHROUGH*/
        case FUNCTION_MBMQSORT :           /*FALLTHROUGH*/
        case FUNCTION_MERGESORT :          /*FALLTHROUGH*/
        case FUNCTION_MINMAXSORT :         /*FALLTHROUGH*/
        case FUNCTION_NBQSORT :            /*FALLTHROUGH*/
        case FUNCTION_NETWORKSORT :        /*FALLTHROUGH*/
        case FUNCTION_P9QSORT :            /*FALLTHROUGH*/
        case FUNCTION_QSORT :              /*FALLTHROUGH*/
        case FUNCTION_SELSORT :            /*FALLTHROUGH*/
        case FUNCTION_SHELLSORT :          /*FALLTHROUGH*/
        case FUNCTION_SMOOTHSORT :         /*FALLTHROUGH*/
        case FUNCTION_SQRTSORT :           /*FALLTHROUGH*/
        case FUNCTION_SQSORT :             /*FALLTHROUGH*/
        case FUNCTION_SYSMERGESORT :       /*FALLTHROUGH*/
        case FUNCTION_WQSORT :             /*FALLTHROUGH*/
        case FUNCTION_YQSORT :
            ret = "sort";
            *ptests |= ( TEST_TYPE_SORT );
        break;
        default :
            errno = EINVAL;
        break;
    }
    return ret;
}

/* name of the function to be tested */
const char *function_name(unsigned int func)
{
    const char *ret="unknown";

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (func) {
        case FUNCTION_IBMQSORT :           /*FALLTHROUGH*/
        case FUNCTION_BMQSORT :            ret = "Bentley&McIlroy qsort";
        break;
        case FUNCTION_DEDSORT :            ret = "dedicated sort";
        break;
        case FUNCTION_DPQSORT :            ret = "dual-pivot qsort";
        break;
        case FUNCTION_GLQSORT :            ret = "glibc qsort";
        break;
        case FUNCTION_HEAPSORT :           ret = "heapsort";
        break;
        case FUNCTION_INDIRECT_MERGESORT : ret = "indirect mergesort";
        break;
        case FUNCTION_INTROSORT :          ret = "introsort";
        break;
        case FUNCTION_ISORT :              ret = "insertion sort";
        break;
        case FUNCTION_LOGSORT :            ret = "log sort";
        break;
        case FUNCTION_MBMQSORT :           ret = "modified Bentley&McIlroy qsort";
        break;
        case FUNCTION_MERGESORT :          ret = "in-place mergesort";
        break;
        case FUNCTION_MINMAXSORT :         ret = "minmax sort";
        break;
        case FUNCTION_NBQSORT :            ret = "NetBSD qsort";
        break;
        case FUNCTION_NETWORKSORT :        ret = "network sort";
        break;
        case FUNCTION_P9QSORT :            ret = "plan9 qsort";
        break;
        case FUNCTION_QSELECT_SORT :       /*FALLTHROUGH*/
        case FUNCTION_QSELECT :            ret = "quickselect";
        break;
        case FUNCTION_QSORT :              ret = "library qsort";
        break;
        case FUNCTION_QSORT_WRAPPER :      ret = "qsort_wrapper";
        break;
        case FUNCTION_SELSORT :            ret = "selection sort";
        break;
        case FUNCTION_SHELLSORT :          ret = "Shell sort";
        break;
        case FUNCTION_SMOOTHSORT :         ret = "smoothsort";
        break;
        case FUNCTION_SQRTSORT :           ret = "sqrt sort";
        break;
        case FUNCTION_SQSORT :             ret = "simplified quickselect";
        break;
        case FUNCTION_SYSMERGESORT :       ret = "system mergesort";
        break;
        case FUNCTION_WQSORT :             ret = "lopsided quickselect";
        break;
        case FUNCTION_YQSORT :             ret = "Yaroslavskiy's dual-pivot sort";
        break;
        case FUNCTION_QSELECT_S :          ret = "quickselect_s";
        break;
        default :
            (V)fprintf(stderr, "// %s: %s line %d: unrecognized func %u\n", __func__, source_file, __LINE__, func);
            errno = EINVAL;
        break;
    }
    return ret;
}

unsigned int test_function(const char *prog, long *refarray, int *array,
    struct data_struct *data_array, double *darray, long *larray,
    struct data_struct **parray, unsigned int func, size_t n, size_t count,
    unsigned int *pcsequences, unsigned int *ptsequences, unsigned int *ptests,
    unsigned int options, int col, double timeout,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn)
{
    unsigned char c;
    unsigned int errs=0U;
    int type;
    size_t size;
    int (*compar)(const void *, const void *);
    int (*compar_s)(const void *, const void *, void *);
    void (*swapf)(void *, void *, size_t);
#if DEBUG_CODE
    const char *fname = function_name(func);
#endif
    const char *ftype = function_type(func, ptests), *typename
#if SILENCE_WHINEY_COMPILERS
        ="gcc-fodder"
#endif
        ;

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        if ((0U==flags['h'])&&(NULL!=f))
            f(LOG_INFO, log_arg,
                "%s: %s %s start: array=%p, data_array=%p, darray=%p",
                prog, fname, ftype,(void *)array,(void *)data_array,
                (void *)darray);
#endif
    /* check flags, modify tests as requested */
    /* first pass for test types */
    for (c=0U; ; c++) {
        /* supported flags */
        switch (c) {
            case 'a' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_ADVERSARY ;
            break;
            case 'C' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_CORRECTNESS ;
            break;
            case 't' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_THOROUGH ;
            break;
            case 'T' :
                if (0U != flags[c]) *ptests |= TEST_TYPE_TIMING ;
            break;
        }
        if (c == UCHAR_MAX) break; /* That's All Folks! */
    }
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        if ((0U==flags['h'])&&(NULL!=f))
            f(LOG_INFO, log_arg, "%s: %s %s flags 1st pass completed",
                prog, fname, ftype);
#endif
    /* second pass for tests with data types */
    for (c=0U; ; c++) {
        /* supported flags */
        switch (c) {
            case 'A' :
                if (0U != flags[c]) { /* test requested */
                    typename = "string";
                    type = DATA_TYPE_STRING;
                    size = sizeof(struct data_struct);
                    if (0U != flags['i']) /* comparison/swap counts */
                        compar = idata_struct_strcmp,
                        compar_s = idata_struct_strcmp_s,
                        swapf=NULL;
                    else
                        compar = data_struct_strcmp,
                        compar_s = data_struct_strcmp_s,
                        swapf=NULL;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'F' :
                if (0U != flags[c]) { /* test requested */
                    typename = "double";
                    type = DATA_TYPE_DOUBLE;
                    size = sizeof(double);
                    if (0U != flags['i']) /* comparison/swap counts */
                        compar = idoublecmp,
                        compar_s = idoublecmp_s,
                        swapf=NULL;
                    else
                        compar = doublecmp,
                        compar_s = doublecmp_s,
                        swapf=NULL;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'I' :
                if (0U != flags[c]) { /* test requested */
                    typename = "integer";
                    type = DATA_TYPE_INT;
                    size = sizeof(int);
                    if (0U != flags['i']) /* comparison/swap counts */
                        compar = iintcmp, compar_s = iintcmp_s,
                        swapf=NULL;
                    else
                        compar = intcmp, compar_s = intcmp_s,
                        swapf=NULL;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'L' :
                if (0U != flags[c]) { /* test requested */
                    typename = "long";
                    type = DATA_TYPE_LONG;
                    size = sizeof(long);
                    if (0U != flags['i']) /* comparison/swap counts */
                        compar = ilongcmp, compar_s = ilongcmp_s,
                        swapf=NULL;
                    else
                        compar = longcmp, compar_s = longcmp_s,
                        swapf=NULL;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'P' :
                if (0U != flags[c]) { /* test requested */
                    typename = "pointer";
                    type = DATA_TYPE_POINTER;
                    size = sizeof(struct data_struct *);
                    if (0U != flags['i']) /* comparison/swap counts */
                        compar = iindcmp, compar_s = iindcmp_s,
                        swapf=NULL;
                    else
                        compar = indcmp, compar_s = indcmp_s,
                        swapf=NULL;
                } else { /* test not requested */
                    continue;
                }
            break;
            case 'S' :
                if (0U != flags[c]) { /* test requested */
                    typename = "structure";
                    type = DATA_TYPE_STRUCT;
                    size = sizeof(struct data_struct);
                    if (0U != flags['i']) /* comparison/swap counts */
                        compar = itimecmp, compar_s = itimecmp_s,
                        swapf=NULL;
                    else
                        compar = timecmp, compar_s = timecmp_s,
                        swapf=NULL;
                } else { /* test not requested */
                    continue;
                }
            break;
            default : /* not a relevant flag */
                if (c != UCHAR_MAX) continue;
            break;
        }
        if (c == UCHAR_MAX) break; /* That's All Folks! */
        /* typename is guaranteed to have been assigned a value by the above
           switch statement whether or not (:-/) gcc's authors realize it...
        */
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            if (NULL!=f)
                f(LOG_INFO, log_arg, "%s: %s %s flag %c tests",
                    prog, fname, ftype, c);
#endif
        if (0U == errs) {
            if (TEST_TYPE_CORRECTNESS == (TEST_TYPE_CORRECTNESS & *ptests)) {
                errs +=  correctness_tests(type, size, refarray, larray, array,
                    darray, data_array, parray, typename, compar, compar_s,
                    swapf, options, prog, func, n, count,
                    pcsequences, ptests, col, timeout, f, log_arg, flags,
                    pwarray, puarray, psarray, marray, pdn);
            }
        }
        if (0U == errs) {
            if (TEST_TYPE_TIMING == (TEST_TYPE_TIMING & *ptests)) {
                errs +=  timing_tests(type, size, refarray, larray, array,
                    darray, data_array, parray, typename, compar, compar_s,
                    swapf, options, prog, func, n, count,
                    ptsequences, ptests, col, timeout, f, log_arg, flags,
                    pwarray, puarray, psarray, marray, pdn);
            }
        }
    }
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG)) if ((0U==flags['h'])&&(NULL!=f)) {
        if (0U!=flags['K']) fprintf(stderr, "/* ");
        f(LOG_INFO, log_arg, "%s: %s %s %s tests completed: errs %u", prog,
            fname, ftype, typename, errs);
        if (0U!=flags['K']) fprintf(stderr, " */");
    }
#endif
    return errs;
}
