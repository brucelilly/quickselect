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
* $Id: ~|^` @(#)    valid.c copyright 2016-2017 Bruce Lilly.   \ valid.c $
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
/* $Id: ~|^` @(#)   This is valid.c version 1.4 dated 2018-06-06T00:47:43Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.valid.c */

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
#define ID_STRING_PREFIX "$Id: valid.c ~|^` @(#)"
#define SOURCE_MODULE "valid.c"
#define MODULE_VERSION "1.4"
#define MODULE_DATE "2018-06-06T00:47:43Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* return non-zero if test testnum is valid for test_type at size n */
int valid_test(unsigned int test_type, unsigned int testnum, size_t n)
{
    if (1UL>n) return 0; /* nothing to test! */
    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* same conditions for correctness and timing */
    switch (testnum) {
        /* size-limited */
        case TEST_SEQUENCE_ORGAN_PIPE :               /*FALLTHROUGH*/
        case TEST_SEQUENCE_INVERTED_ORGAN_PIPE :      /*FALLTHROUGH*/
        case TEST_SEQUENCE_ADVERSARY :
            if (2UL > n) return 0;
        break;
        case TEST_SEQUENCE_HISTOGRAM :                /*FALLTHROUGH*/
            if (3UL > n) return 0;
        break;
        case TEST_SEQUENCE_DUAL_PIVOT_KILLER :        /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_LEFT :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_MIDDLE :        /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_RIGHT :         /*FALLTHROUGH*/
        case TEST_SEQUENCE_MANY_EQUAL_SHUFFLED :      /*FALLTHROUGH*/
        case TEST_SEQUENCE_SAWTOOTH :
            if (5UL > n) return 0;
        break;
        case TEST_SEQUENCE_MEDIAN3KILLER :
            if (7UL > n) return 0;
        break;
        /* not size-limited */
        case TEST_SEQUENCE_STDIN :                    /*FALLTHROUGH*/
        case TEST_SEQUENCE_SORTED :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_ROTATED :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_SHIFTED :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_REVERSE :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_BINARY :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_TERNARY :                  /*FALLTHROUGH*/
        case TEST_SEQUENCE_COMBINATIONS :             /*FALLTHROUGH*/
        case TEST_SEQUENCE_PERMUTATIONS :             /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_DISTINCT :          /*FALLTHROUGH*/
        case TEST_SEQUENCE_CONSTANT :                 /*FALLTHROUGH*/
        case TEST_SEQUENCE_JUMBLE :                   /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_LIMITED :    /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_RESTRICTED : /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_NORMAL :     /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES_RECIPROCAL : /*FALLTHROUGH*/
        case TEST_SEQUENCE_RANDOM_VALUES :            /*FALLTHROUGH*/
        case TEST_SEQUENCE_WORST :                    /*FALLTHROUGH*/
        break;
        default:
            (V)fprintf(stderr, "// %s: %s line %d: unrecognized testnum %u\n",
                __func__, source_file, __LINE__, testnum);
        return 0;
    }
#if 0
    /* conditions specific to correctness or timing */
    switch (test_type) {
        case TEST_TYPE_CORRECTNESS :
            switch (testnum) {
                /* always invalid for correctness */
                case TEST_SEQUENCE_ADVERSARY :
                return 0; /* no correctness test */
            }
        break;
        case TEST_TYPE_TIMING :
            switch (testnum) {
                /* size-limited */
                case TEST_SEQUENCE_ADVERSARY :
                    if (2UL > n) return 0;
                break;
            }
        break;
        default :
            (V)fprintf(stderr, "// %s: %s line %d: unrecognized test_type %u\n",
                __func__, source_file, __LINE__, test_type);
        return 0;
    }
#endif
    return 1;
}
