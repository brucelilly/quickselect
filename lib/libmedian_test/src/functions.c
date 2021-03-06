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
* $Id: ~|^` @(#)    functions.c copyright 2016-2019 Bruce Lilly.   \ functions.c $
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
/* $Id: ~|^` @(#)   This is functions.c version 1.26 dated 2019-07-25T00:13:35Z. \ $ */
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
#define MODULE_VERSION "1.26"
#define MODULE_DATE "2019-07-25T00:13:35Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* return bit number of least-significant 1 bit in mask
   0x01U -> 0U, 0x02U -> 1U, 0x03U -> 0U, etc.
*/
extern unsigned int ls_bit_number(unsigned int mask)
{
    unsigned int ret;

    for (ret=0U; ret<32U; ret++)
        if (0U!=(mask&(0x01U<<ret))) break;
    return ret;
}

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
        case FUNCTION_ILLUMOSQSORT :       /*FALLTHROUGH*/
        case FUNCTION_QSORT :              /*FALLTHROUGH*/
        case FUNCTION_RUNSORT :            /*FALLTHROUGH*/
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
        case FUNCTION_IBMQSORT :           ret = "Bentley&McIlroy qsort";
        break;
        case FUNCTION_BMQSORT :            ret = "uninstrumented Bentley&McIlroy qsort";
        break;
        case FUNCTION_WBMQSORT :           ret = "worst-case Bentley&McIlroy qsort";
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
        case FUNCTION_MBMQSORT :           ret = "modified qsort";
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
        case FUNCTION_ILLUMOSQSORT :       ret = "illumos qsort";
        break;
        case FUNCTION_QSELECT_SORT :       ret = "quickselect-sort";
        break;
        case FUNCTION_QSELECT :            ret = "multiple quickselect";
        break;
        case FUNCTION_QSORT :              ret = "library qsort";
        break;
        case FUNCTION_QSORT_WRAPPER :      ret = "qsort_wrapper";
        break;
        case FUNCTION_RUNSORT :            ret = "merge runs";
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
        case FUNCTION_WQSORT :             ret = "lopsided";
        break;
        case FUNCTION_YQSORT :             ret = "Yaroslavskiy's sort";
        break;
        case FUNCTION_QSELECT_S :          ret = "quickselect_s";
        break;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized func %u */\n", __func__, source_file, __LINE__, func);
            errno = EINVAL;
        break;
    }
    return ret;
}

/* size of the data type to be tested */
size_t type_size(unsigned int type)
{
    size_t ret=0UL;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (type) {
        case DATA_TYPE_CHAR :           ret = sizeof(char);
        break;
        case DATA_TYPE_SHORT :          ret = sizeof(short);
        break;
        case DATA_TYPE_INT :            ret = sizeof(int);
        break;
        case DATA_TYPE_LONG :           ret = sizeof(long);
        break;
        case DATA_TYPE_FLOAT :          ret = sizeof(float);
        break;
        case DATA_TYPE_DOUBLE :         ret = sizeof(double);
        break;
        case DATA_TYPE_STRUCT :         ret = sizeof(struct data_struct);
        break;
        case DATA_TYPE_STRING :         ret = sizeof(struct data_struct);
        break;
        case DATA_TYPE_POINTER :        ret = sizeof(struct data_struct *);
        break;
        case DATA_TYPE_UINT_LEAST8_T :  ret = sizeof(uint_least8_t);
        break;
        case DATA_TYPE_UINT_LEAST16_T : ret = sizeof(uint_least16_t);
        break;
        case DATA_TYPE_UINT_LEAST32_T : ret = sizeof(uint_least32_t);
        break;
        case DATA_TYPE_UINT_LEAST64_T : ret = sizeof(uint_least64_t);
        break;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized type %u */\n", __func__, source_file, __LINE__, type);
            errno = EINVAL;
        break;
    }
    return ret;
}

void *type_array(unsigned int type)
{
    void *ret=NULL;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (type) {
        case DATA_TYPE_CHAR :           ret = NULL;
        break;
        case DATA_TYPE_SHORT :          ret = (void *)global_sharray;
        break;
        case DATA_TYPE_INT :            ret = (void *)global_iarray;
        break;
        case DATA_TYPE_LONG :           ret = (void *)global_larray;
        break;
        case DATA_TYPE_FLOAT :          ret = NULL;
        break;
        case DATA_TYPE_DOUBLE :         ret = (void *)global_darray;
        break;
        case DATA_TYPE_STRUCT :         ret = (void *)global_data_array;
        break;
        case DATA_TYPE_STRING :         ret = (void *)global_data_array;
        break;
        case DATA_TYPE_POINTER :        ret = (void *)global_parray;
        break;
        case DATA_TYPE_UINT_LEAST8_T :  ret = NULL;
        break;
        case DATA_TYPE_UINT_LEAST16_T : ret = NULL;
        break;
        case DATA_TYPE_UINT_LEAST32_T : ret = NULL;
        break;
        case DATA_TYPE_UINT_LEAST64_T : ret = NULL;
        break;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized type %u */\n", __func__, source_file, __LINE__, type);
            errno = EINVAL;
        break;
    }
    return ret;
}

size_t type_alignment(unsigned int type)
{
    size_t size=type_size(type);
    char *array=(char *)type_array(type);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if ((NULL==array)||(size==0UL)) return 0UL;
    return alignment_size(array,size);
}

/* maximum value of the data type to be tested */
uint_least64_t type_max(unsigned int type)
{
    uint_least64_t ret=0;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (type) {
        case DATA_TYPE_CHAR :           ret = CHAR_MAX;
        break;
        case DATA_TYPE_SHORT :          ret = SHRT_MAX;
        break;
        case DATA_TYPE_INT :            ret = INT_MAX;
        break;
        case DATA_TYPE_LONG :           ret = LONG_MAX;
        break;
        case DATA_TYPE_FLOAT :          ret = LONG_MAX;
        break;
        case DATA_TYPE_DOUBLE :         ret = LONG_MAX;
        break;
        case DATA_TYPE_STRUCT :         ret = LONG_MAX;
        break;
        case DATA_TYPE_STRING :         ret = LONG_MAX;
        break;
        case DATA_TYPE_POINTER :        ret = LONG_MAX;
        break;
        case DATA_TYPE_UINT_LEAST8_T :  ret = UINT_LEAST8_MAX;
        break;
        case DATA_TYPE_UINT_LEAST16_T : ret = UINT_LEAST16_MAX;
        break;
        case DATA_TYPE_UINT_LEAST32_T : ret = UINT_LEAST32_MAX;
        break;
        case DATA_TYPE_UINT_LEAST64_T : ret = UINT_LEAST64_MAX;
        break;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized type %u */\n", __func__, source_file, __LINE__, type);
            errno = EINVAL;
        break;
    }
    return ret;
}

/* name of the data type to be tested */
const char *type_name(unsigned int type)
{
    const char *ret="unknown";

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (type) {
        case DATA_TYPE_CHAR :           ret = "char";
        break;
        case DATA_TYPE_SHORT :          ret = "short";
        break;
        case DATA_TYPE_INT :            ret = "plain int";
        break;
        case DATA_TYPE_LONG :           ret = "long";
        break;
        case DATA_TYPE_FLOAT :          ret = "float";
        break;
        case DATA_TYPE_DOUBLE :         ret = "double";
        break;
        case DATA_TYPE_STRUCT :         ret = "struct";
        break;
        case DATA_TYPE_STRING :         ret = "string";
        break;
        case DATA_TYPE_POINTER :        ret = "pointer";
        break;
        case DATA_TYPE_UINT_LEAST8_T :  ret = "uint_least8_t";
        break;
        case DATA_TYPE_UINT_LEAST16_T : ret = "uint_least16_t";
        break;
        case DATA_TYPE_UINT_LEAST32_T : ret = "uint_least32_t";
        break;
        case DATA_TYPE_UINT_LEAST64_T : ret = "uint_least64_t";
        break;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized type %u */\n", __func__, source_file, __LINE__, type);
            errno = EINVAL;
        break;
    }
    return ret;
}
