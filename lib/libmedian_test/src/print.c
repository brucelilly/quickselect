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
* $Id: ~|^` @(#)    print.c copyright 2016-2018 Bruce Lilly.   \ print.c $
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
/* $Id: ~|^` @(#)   This is print.c version 1.12 dated 2018-06-05T23:43:22Z. \ $ */
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
#define MODULE_VERSION "1.12"
#define MODULE_DATE "2018-06-05T23:43:22Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

static size_t element_size(char *pointer)
{
    if (pointer==(char*)global_refarray) return sizeof(long); /* XXX could be changed to uint_least64_t */
    if (pointer==(char*)global_sharray) return sizeof(short)*global_ratio;
    if (pointer==(char*)global_iarray) return sizeof(int)*global_ratio;
    if (pointer==(char*)global_larray) return sizeof(long)*global_ratio;
    if (pointer==(char*)global_darray) return sizeof(double)*global_ratio;
    if (pointer==(char*)global_data_array) return sizeof(struct data_struct)*global_ratio;
    if (pointer==(char*)global_parray) return sizeof(struct data_struct *)*global_ratio;
    if (pointer==(char*)global_uarray) return sizeof(float);
    if (pointer==(char*)global_sarray) return sizeof(float);
    if (pointer==(char*)global_warray) return sizeof(float);
    return sizeof(size_t); /* for sorting pk */
}

/* print a pointer (address) offset by its index in array at base, return pointer value */
static char *fprint_pointer(FILE *f, char **base, size_t i, const char *prefix)
{
    char *p=base[i];
    if (NULL!=prefix) (V)fprintf(f, "%s", prefix);
    (V)fprintf(f, "%p[%lu]",base+i,i);
    return p;
}

static void fprint_datum(FILE *f, char *base, size_t i, const char *prefix)
{
    size_t size=element_size(base);

    if (NULL!=prefix) (V)fprintf(f,"%s",prefix);
    if (base!=global_parray) {
        if ((base==global_sharray)||(base==global_iarray)||(base==global_larray)
        || (base==global_darray)||(base==global_data_array)
        )
            (V)fprintf(f,"%p[%lu]",base+global_ratio*size*i,i);
        else
            (V)fprintf(f,"%p[%lu]",base+size*i,i);
    }
    if (base==(char*)global_refarray) (V)fprintf(f,":%ld",global_refarray[i]); /* XXX could be changed to format for uint_least64_t */
    else if (base==(char*)global_sharray) (V)fprintf(f,":%hd",global_sharray[global_ratio*i]);
    else if (base==(char*)global_iarray) (V)fprintf(f,":%d",global_iarray[global_ratio*i]);
    else if (base==(char*)global_larray) (V)fprintf(f,":%ld",global_larray[global_ratio*i]);
    else if (base==(char*)global_darray) (V)fprintf(f,":%.9G",global_darray[global_ratio*i]);
    else if (base==(char*)global_data_array) (V)fprintf(f,":%s",global_data_array[global_ratio*i].string);
    else if (base==(char*)global_parray) { 
        struct data_struct *p=fprint_pointer(f,(char *)global_parray,i,prefix);
        size_t j=(p-global_data_array)/sizeof(struct data_struct);
        (V)fprintf(f,"%p[%lu]",p,i);
        fprint_datum(f,(char *)global_data_array,j,"->");
    }
    else if (base==(char*)global_uarray) (V)fprintf(f,":%.9G",global_uarray[i]);
    else if (base==(char*)global_sarray) (V)fprintf(f,":%.9G",global_sarray[i]);
    else if (base==(char*)global_warray) (V)fprintf(f,":%.9G",global_warray[i]);
    else (V)fprintf(f,":%lu",((size_t *)base)[i]);
}

static char *base_array(char *pointer)
{
    if ((NULL!=(char*)global_refarray)&&(pointer>=((char*)global_refarray))
    && (pointer<((char*)global_refarray)
       +element_size((char*)global_refarray)*global_sz)
    ) return (char *)global_refarray;
    if ((NULL!=(char*)global_sharray)&&(pointer>=((char*)global_sharray))
    && (pointer<((char*)global_sharray)
       +element_size((char*)global_sharray)*global_sz)
    ) return (char *)global_sharray;
    if ((NULL!=(char*)global_iarray)&&(pointer>=((char*)global_iarray))
    && (pointer<((char*)global_iarray)
       +element_size((char*)global_iarray)*global_sz)
    ) return (char *)global_iarray;
    if ((NULL!=(char*)global_larray)&&(pointer>=((char*)global_larray))
    && (pointer<((char*)global_larray)
       +element_size((char*)global_larray)*global_sz)
    ) return (char *)global_larray;
    if ((NULL!=(char*)global_darray)&&(pointer>=((char*)global_darray))
    && (pointer<((char*)global_darray)
       +element_size((char*)global_darray)*global_sz)
    ) return (char *)global_darray;
    if ((NULL!=(char*)global_data_array)&&(pointer>=((char*)global_data_array))
    && (pointer<((char*)global_data_array)
       +element_size((char*)global_data_array)*global_sz)
    ) return (char *)global_data_array;
    if ((NULL!=(char*)global_parray)&&(pointer>=((char*)global_parray))
    && (pointer<((char*)global_parray)
       +element_size((char*)global_parray)*global_sz)
    ) return (char *)global_parray;
    if ((NULL!=(char*)global_uarray)&&(pointer>=((char*)global_uarray))
    && (pointer<((char*)global_uarray)
       +element_size((char*)global_uarray)*global_count)
    ) return (char *)global_uarray;
    if ((NULL!=(char*)global_sarray)&&(pointer>=((char*)global_sarray))
    && (pointer<((char*)global_sarray)
       +element_size((char*)global_sarray)*global_count)
    ) return (char *)global_sarray;
    if ((NULL!=(char*)global_warray)&&(pointer>=((char*)global_warray))
    && (pointer<((char*)global_warray)
       +element_size((char*)global_warray)*global_count)
    ) return (char *)global_warray;
    return pointer; /* for sorting pk */
}

void fprint_some_array(FILE *f, char *target, size_t l, size_t u, const char *prefix, const char *suffix, unsigned int options)
{
    size_t i;
    char *base;

    if (NULL!=prefix) (V)fprintf(f,"%s", prefix);
    if (0U!=(options&(QUICKSELECT_INDIRECT))) {
        char *p=fprint_pointer(f,(char **)target,l,NULL);
        size_t j, size;
        base=base_array(p);
        size=element_size(base);
        if (NULL!=base) {
            j=(p-base)/size;
            fprint_datum(f,base,j,"->");
        } else
            (V)fprintf(f,"->?");
        for (i=l+1UL; i<=u; i++) {
            p=fprint_pointer(f,(char **)target,i," ");
            if (NULL!=base) {
                j=(p-base)/size;
                fprint_datum(f,base,j,"->");
            } else
                (V)fprintf(f,"->?");
        }
    } else {
        base=base_array(target);
        if (NULL!=base) {
            for (i=l; i<=u; i++) fprint_datum(f,base,i,i==l?NULL:" ");
        } else {
            (V)fprintf(f,"target %p -> base array ?",target);
        }
    }
    if (NULL!=suffix) (V)fprintf(f,"%s", suffix);
    (V)fprintf(f,"\n");
}

void print_some_array(char *target, size_t l, size_t u, const char *prefix, const char *suffix, unsigned int options)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);
    fprint_some_array(stderr,target,l,u,prefix,suffix,options);
}

#if 1
void print_size_t_array(size_t *target, size_t ratio, size_t l, size_t u, const char *prefix, const char *suffix)
{
    size_t i;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (NULL!=prefix) (V)fprintf(stderr, "%s", prefix);
    for (i=l; i<=u; i++) {
        if (i != l)
            (V)fprintf(stderr, ", ");
        (V)fprintf(stderr, "%lu:%lu", (unsigned long)i, target[ratio*i]);
    }
    (V)fprintf(stderr, "%s\n",suffix);
}
#endif

#if 0
static void write_int_array(char *buf, size_t sz, int *target, size_t ratio, size_t l, size_t u, const char *prefix, const char *suffix)
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
