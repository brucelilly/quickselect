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
* $Id: ~|^` @(#)    comparators.c copyright 2019 Bruce Lilly.   \ comparators.c $
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
/* $Id: ~|^` @(#)   This is comparators.c version 1.1 dated 2019-02-27T19:57:53Z. \ comparators.c $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.comparators.c */

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
#define ID_STRING_PREFIX "$Id: comparators.c ~|^` @(#)"
#define SOURCE_MODULE "comparators.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2019-02-27T19:57:53Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2019"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

int (*type_context_comparator(unsigned int type, unsigned char *flags))(const void *, const void *, void *)
{
    int (*ret)(const void *, const void *, void *)=NULL;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (type) {
        case DATA_TYPE_CHAR :           return NULL;
        break;
        case DATA_TYPE_SHORT :
            if (0U!=flags['i']) return ishortcmp_s; else return shortcmp_s;
        break;
        case DATA_TYPE_INT :
            if (0U!=flags['i']) return iintcmp_s; else return intcmp_s;
        break;
        case DATA_TYPE_LONG :
            if (0U!=flags['i']) return ilongcmp_s; else return longcmp_s;
        break;
        case DATA_TYPE_FLOAT :          return NULL;
        break;
        case DATA_TYPE_DOUBLE :
            if (0U!=flags['i']) return idoublecmp_s; else return doublecmp_s;
        break;
        case DATA_TYPE_STRUCT :
            if (0U!=flags['i']) return itimecmp_s; else return timecmp_s;
        break;
        case DATA_TYPE_STRING :
            if (0U!=flags['i']) return idata_struct_strcmp_s; else return data_struct_strcmp_s;
        break;
        case DATA_TYPE_POINTER :
            if (0U!=flags['i']) return iindcmp_s; else return indcmp_s;
        break;
        case DATA_TYPE_UINT_LEAST8_T :  return NULL;
        break;
        case DATA_TYPE_UINT_LEAST16_T : return NULL;
        break;
        case DATA_TYPE_UINT_LEAST32_T : return NULL;
        break;
        case DATA_TYPE_UINT_LEAST64_T : return NULL;
        break;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized type %u */\n", __func__, source_file, __LINE__, type);
            errno = EINVAL;
        break;
    }
    return ret;
}

int (*uninstrumented_comparator(int (*compar)(const void *,const void *,...)))(const void *, const void *,...)
{
    if (compar==(int (*)(const void *,const void *,...))ishortcmp) return shortcmp;
    if (compar==(int (*)(const void *,const void *,...))iintcmp) return intcmp;
    if (compar==(int (*)(const void *,const void *,...))ilongcmp) return longcmp;
    if (compar==(int (*)(const void *,const void *,...))idoublecmp) return doublecmp;
    if (compar==(int (*)(const void *,const void *,...))itimecmp) return timecmp;
    if (compar==(int (*)(const void *,const void *,...))idata_struct_strcmp) return data_struct_strcmp;
    if (compar==(int (*)(const void *,const void *,...))iindcmp) return indcmp;
    if (compar==(int (*)(const void *,const void *,...))ishortcmp_s) return shortcmp_s;
    if (compar==(int (*)(const void *,const void *,...))iintcmp_s) return intcmp_s;
    if (compar==(int (*)(const void *,const void *,...))ilongcmp_s) return longcmp_s;
    if (compar==(int (*)(const void *,const void *,...))idoublecmp_s) return doublecmp_s;
    if (compar==(int (*)(const void *,const void *,...))itimecmp_s) return timecmp_s;
    if (compar==(int (*)(const void *,const void *,...))idata_struct_strcmp_s) return data_struct_strcmp_s;
    if (compar==(int (*)(const void *,const void *,...))iindcmp_s) return indcmp_s;
    return compar;
}

const char *comparator_name(int (*compar)(const void *, const void *,...))
{
    if (compar==(int (*)(const void *,const void *,...))shortcmp) return "shortcmp";
    if (compar==(int (*)(const void *,const void *,...))ishortcmp) return "ishortcmp";
    if (compar==(int (*)(const void *,const void *,...))intcmp) return "intcmp";
    if (compar==(int (*)(const void *,const void *,...))iintcmp) return "iintcmp";
    if (compar==(int (*)(const void *,const void *,...))longcmp) return "longcmp";
    if (compar==(int (*)(const void *,const void *,...))ilongcmp) return "ilongcmp";
    if (compar==(int (*)(const void *,const void *,...))doublecmp) return "doublecmp";
    if (compar==(int (*)(const void *,const void *,...))idoublecmp) return "idoublecmp";
    if (compar==(int (*)(const void *,const void *,...))timecmp) return "timecmp";
    if (compar==(int (*)(const void *,const void *,...))itimecmp) return "itimecmp";
    if (compar==(int (*)(const void *,const void *,...))data_struct_strcmp) return "data_struct_strcmp";
    if (compar==(int (*)(const void *,const void *,...))idata_struct_strcmp) return "idata_struct_strcmp";
    if (compar==(int (*)(const void *,const void *,...))indcmp) return "indcmp";
    if (compar==(int (*)(const void *,const void *,...))iindcmp) return "iindcmp";
    if (compar==(int (*)(const void *,const void *,...))shortcmp_s) return "shortcmp_s";
    if (compar==(int (*)(const void *,const void *,...))ishortcmp_s) return "ishortcmp_s";
    if (compar==(int (*)(const void *,const void *,...))intcmp_s) return "intcmp_s";
    if (compar==(int (*)(const void *,const void *,...))iintcmp_s) return "iintcmp_s";
    if (compar==(int (*)(const void *,const void *,...))longcmp_s) return "longcmp_s";
    if (compar==(int (*)(const void *,const void *,...))ilongcmp_s) return "ilongcmp_s";
    if (compar==(int (*)(const void *,const void *,...))doublecmp_s) return "doublecmp_s";
    if (compar==(int (*)(const void *,const void *,...))idoublecmp_s) return "idoublecmp_s";
    if (compar==(int (*)(const void *,const void *,...))timecmp_s) return "timecmp_s";
    if (compar==(int (*)(const void *,const void *,...))itimecmp_s) return "itimecmp_s";
    if (compar==(int (*)(const void *,const void *,...))data_struct_strcmp_s) return "data_struct_strcmp_s";
    if (compar==(int (*)(const void *,const void *,...))idata_struct_strcmp_s) return "idata_struct_strcmp_s";
    if (compar==(int (*)(const void *,const void *,...))indcmp_s) return "indcmp_s";
    if (compar==(int (*)(const void *,const void *,...))iindcmp_s) return "iindcmp_s";
    if (compar==(int (*)(const void *,const void *,...))aqcmp) return "aqcmp";
    if (compar==(int (*)(const void *,const void *,...))floatcmp) return "floatcmp";
    if (compar==(int (*)(const void *,const void *,...))size_tcmp) return "size_tcmp";
    if (compar==(int (*)(const void *,const void *,...))db_compare_struct) return "db_compare_struct";
    if (compar==(int (*)(const void *,const void *,...))db_compare_string) return "db_compare_string";
    if (compar==(int (*)(const void *,const void *,...))db_compare_double) return "db_compare_double";
    if (compar==(int (*)(const void *,const void *,...))db_compare_long) return "db_compare_long";
    if (compar==(int (*)(const void *,const void *,...))db_compare_int) return "db_compare_int";
    if (compar==(int (*)(const void *,const void *,...))db_compare_short) return "db_compare_short";
    if (NULL==compar) return "NULL";
    return "unknown";
}
