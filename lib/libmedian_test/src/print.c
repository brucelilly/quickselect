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
* $Id: ~|^` @(#)    print.c copyright 2016-2017 Bruce Lilly.   \ print.c $
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
/* $Id: ~|^` @(#)   This is print.c version 1.3 dated 2017-11-03T19:46:10Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.print.c */

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
#define ID_STRING_PREFIX "$Id: print.c ~|^` @(#)"
#define SOURCE_MODULE "print.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2017-11-03T19:46:10Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* Print elements of integer array with indices l through u inclusive. */
static void fprint_int_array(FILE *f, int *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if (NULL!=prefix) (V)fprintf(f, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%d", (unsigned long)i, target[i]);
    }
    (V)fprintf(f, "%s\n",suffix);
}

static void fprint_long_array(FILE *f, long *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if (NULL!=prefix) (V)fprintf(f, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%ld", (unsigned long)i, target[i]);
    }
    (V)fprintf(f, "%s\n",suffix);
}

static void fprint_data_array(FILE *f, struct data_struct *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if (NULL!=prefix) (V)fprintf(f, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%s", (unsigned long)i, target[i].string);
    }
    (V)fprintf(f, "%s\n",suffix);
}

static void fprint_double_array(FILE *f, double *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if (NULL!=prefix) (V)fprintf(f, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%.9G", (unsigned long)i, target[i]);
    }
    (V)fprintf(f, "%s\n",suffix);
}

static void fprint_float_array(FILE *f, float *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if (NULL!=prefix) (V)fprintf(f, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(f, ", ");
        (V)fprintf(f, "%lu:%.9G", (unsigned long)i, target[i]);
    }
    (V)fprintf(f, "%s\n",suffix);
}

void fprint_some_array(FILE *f, void *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    if (target == (void *)global_iarray) fprint_int_array(f,global_iarray,l,u,prefix,suffix);
    else if (target == (void *)global_darray) fprint_double_array(f,global_darray,l,u,prefix,suffix);
    else if (target == (void *)global_parray) fprint_data_array(f,*global_parray,l,u,prefix,suffix);
    else if (target == (void *)global_data_array) fprint_data_array(f,global_data_array,l,u,prefix,suffix);
    else if ((target == (void *)global_larray)||(target == (void *)global_refarray))
        fprint_long_array(f,target,l,u,prefix,suffix);
    else if ((target == (void *)global_uarray)||(target == (void *)global_sarray)||(target == (void *)global_warray))
        fprint_float_array(f,target,l,u,prefix,suffix);
    else {
        (V)fprintf(f,
            "%s %s: %s line %d: target=%p is not global_refarray=%p nor global_parray=%p nor global_darray=%p nor global_larray=%p nor global_iarray=%p nor global_data_array=%p nor global_uarray=%p nor global_sarray=%p nor global_warray=%p%s\n",
            prefix,__func__,source_file,__LINE__,target,(void *)global_refarray,
            (void *)global_parray,(void *)global_darray,(void *)global_larray,
            (void *)global_iarray,(void *)global_data_array,
            (void *)global_uarray,(void *)global_sarray,(void *)global_warray,
            suffix);
    }
}

void print_int_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    fprint_int_array(stderr,(int *)target,l,u,prefix,suffix);
}

void print_long_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    fprint_long_array(stderr,(long *)target,l,u,prefix,suffix);
}

void print_data_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    fprint_data_array(stderr,(struct data_struct *)target,l,u,prefix,suffix);
}

void print_indirect_keys(struct data_struct **target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    fprint_data_array(stderr,*target,l,u,prefix,suffix);
}

void print_double_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    fprint_double_array(stderr,(double *)target,l,u,prefix,suffix);
}

void print_some_array(void *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    fprint_some_array(stderr,target,l,u,prefix,suffix);
}

#if 1
void print_size_t_array(size_t *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (NULL!=prefix) (V)fprintf(stderr, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%lu", (unsigned long)i, target[i]);
    }
    (V)fprintf(stderr, "%s\n",suffix);
}
#endif

#if 0
static void write_int_array(char *buf, size_t sz, int *target, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;
    int n;

    if (NULL!=prefix) (V)fprintf(stderr, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l) {
            n=snprintf(buf,sz, ", ");
            buf+=n;
            if (n+2>=sz) break;
            sz-=n;
        }
        n=snprintf(buf,sz, "%lu:%d", (unsigned long)i, target[i]);
        buf+=n;
        if (n+2>=sz) break;
        sz-=n;
    }
    (V)snprintf(buf,sz, "%s\n",suffix);
}
#endif

# if ASSERT_CODE + DEBUG_CODE
FILE *start_log(const char *path) {
    return fopen(path,"w+");
}

void close_log(FILE *f) {
    if (NULL!=f) {
        fflush(f);
        fclose(f);
    }
}
#endif /* ASSERT_CODE */
