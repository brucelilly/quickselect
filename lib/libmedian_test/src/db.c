/*INDENT OFF*/

/* Description: C source code for simulated database access for sorting/selection tests */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    db.c copyright 2018 Bruce Lilly.   \ db.c $
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
/* $Id: ~|^` @(#)   This is db.c version 1.1 dated 2018-12-18T15:59:10Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.db.c */

/********************** Long description and rationale: ***********************
* simulated database access for sorting/selection tests
* The idea is that some database (too large to fit into memory for direct quicksort/quickselect)
*   is sorted or order statistics are selected using a set of keys or indices into the
*   database.  Each index or key (in memory) is used to access data content which is used
*   for the comparison function; the keys or indices are rearranged.  This is similar to
*   indirect sorting or selection except that in this case the actual data resides in files
*   rather than in memory.  In this case, comparisons are relatively expensive (requiring
*   file access and conversion) but sorting movement (keys or indices) is relatively cheap.
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: db.c ~|^` @(#)"
#define SOURCE_MODULE "db.c"
#define MODULE_VERSION "1.1"
#define MODULE_DATE "2018-12-18T15:59:10Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes most other local and system header files required */

#include "initialize_src.h"

void index_to_path(unsigned long indx, const char *prefix, char *path, unsigned int sz)
{
    char buf1[PATH_MAX], buf2[PATH_MAX], buf3[PATH_MAX], buf4[PATH_MAX], *p;
    unsigned long ul;

    buf1[0] = buf3[0] = buf4[0] = '\0';
    ul=indx%256UL; /* for final text component */
    snul(buf3,PATH_MAX,NULL,NULL,ul,16,'0',2,NULL,NULL);
    (void)concatenate_path(NULL,buf3,"txt",buf2,PATH_MAX);
    do {
        indx-=ul;
        indx/=256UL;
        ul=indx%256UL;
        if ((0UL!=ul)||('\0'==buf1[0])) {
            snul(buf4,PATH_MAX,NULL,".d",ul,16,'0',2,NULL,NULL);
            (void)concatenate_path(buf4,buf2,NULL,buf1,PATH_MAX);
            (void)strncpy(buf2,buf1,PATH_MAX);
        }
    } while (indx>0UL);
    (V)concatenate_path(prefix,buf1,NULL,path,sz);
    for (p=strchr(path,'/'); NULL!=p; p=strchr(++p,'/')) {
        *p='\0';
        if (0!=access(path,F_OK)) mkdir(path,0777);
        *p='/';
    }
}

void read_text_file(const char *path, char *buf, unsigned int sz)
{
    char *endptr;
    FILE *f;

    f=fopen(path,"r");
    if (NULL==f) return 0U;
    if (NULL==fgets(buf,sz,f)) { fclose(f); return; }
    /* elide trailing newline */
    endptr=strchr(buf,'\n');
    if (NULL!=endptr) *endptr='\0';
    fclose(f);
    return;
}

void text_to_datum(const char *buf, unsigned int data_type, void *datum)
{
    double d;
    long l;
    struct data_struct *pds=(struct data_struct *)datum;
    struct civil_time_struct cts; /* for specific string format used in structured data */

    switch (data_type) {
        case DATA_TYPE_SHORT :
        /*FALLTHROUGH*/
        case DATA_TYPE_INT :
        /*FALLTHROUGH*/
        case DATA_TYPE_LONG :
            l=strtol(buf,NULL,10);
        break;
        case DATA_TYPE_FLOAT :
        /*FALLTHROUGH*/
        case DATA_TYPE_DOUBLE :
            d=strtod(buf,NULL);
        break;
        case DATA_TYPE_STRUCT :
            (V)parse_civil_time_text(buf,&cts,NULL,-1,NULL,NULL,NULL);
            (void)utc_mktime(&(cts.tm), NULL, NULL);
            pds->year=cts.tm.tm_year+1900;
            pds->u_var.s_md.month=cts.tm.tm_mon+1;
            pds->u_var.s_md.mday=cts.tm.tm_mday;
            pds->hour=cts.tm.tm_hour;
            pds->minute=cts.tm.tm_min;
            pds->second=cts.tm.tm_sec;
            d=cts.fraction*100.0;
            pds->fractional[0]=(int)d;
            d-=pds->fractional[0];
            d*=100.0;
            pds->fractional[1]=(int)d;
            d-=pds->fractional[1];
            d*=100.0;
            pds->fractional[2]=(int)d;
            d-=pds->fractional[2];
            d*=100.0;
            pds->fractional[3]=(int)d;
            d-=pds->fractional[3];
            d*=100.0;
            pds->fractional[4]=(int)d;
            d-=pds->fractional[4];
            d*=100.0;
            pds->fractional[5]=(int)d;
            strncpy(pds->string,buf,STRING_SIZE);
        return;
        case DATA_TYPE_STRING :
            *((const char **)datum)=buf;
        return;
        default :
            (V)fprintf(stderr, "/* %s: %s line %d: unrecognized type %u */\n", __func__, source_file, __LINE__, data_type);
            errno = EINVAL;
        return;
    }
    switch (data_type) {
        case DATA_TYPE_SHORT :
            *((short *)datum)=(short)l;
        return;
        case DATA_TYPE_INT :
            *((int *)datum)=(int)l;
        return;
        case DATA_TYPE_LONG :
            *((long *)datum)=l;
        return;
        case DATA_TYPE_FLOAT :
            *((float *)datum)=(float)d;
        return;
        case DATA_TYPE_DOUBLE :
            *((double *)datum)=d;
        return;
    }
}

void ref_to_text(long l, unsigned int data_type, char *buf, unsigned int sz)
{
    char *p;
    long f;
    double d;
    time_t t; /* time_t might have greater range than long on 32-bit *BSD */
    struct civil_time_struct cts;

    f=FRACTION_COUNT;
    switch (data_type) {
        case DATA_TYPE_UINT_LEAST8_T :
        /*FALLTHROUGH*/
        case DATA_TYPE_UINT_LEAST16_T :
        /*FALLTHROUGH*/
        case DATA_TYPE_UINT_LEAST32_T :
        /*FALLTHROUGH*/
        case DATA_TYPE_UINT_LEAST64_T :
        /*FALLTHROUGH*/
        case DATA_TYPE_LONG :
        /*FALLTHROUGH*/
        case DATA_TYPE_INT :
        /*FALLTHROUGH*/
        case DATA_TYPE_SHORT :
            (V)snul(buf,sz,NULL,NULL,(unsigned long)l,10,'0',1,NULL,NULL);
        break;
        case DATA_TYPE_FLOAT :
        /*FALLTHROUGH*/
        case DATA_TYPE_DOUBLE :
            (V)sng(buf,sz,NULL,NULL,(double)l,-4,3,NULL,NULL);
        break;
        case DATA_TYPE_STRUCT :
        /*FALLTHROUGH*/
        case DATA_TYPE_STRING :
        /*FALLTHROUGH*/
        case DATA_TYPE_POINTER :
            t = (time_t)l/(time_t)f; /* l = seconds */
            gmtime_r(&t, &(cts.tm));
            d=(double)(l%f)/(double)f; /* fraction of a second */
            cts.fraction=d;
            cts.offset=0.0;
            (V)sn_civil_time(buf,sz,&cts,-12,1,0,NULL,NULL);
            p=strchr(buf,'Z');
            if (NULL!=p) *p='\0'; /* avoid comparisons of 'Z' vs. digits */
#if ASSERT_CODE
            t = utc_mktime(&(cts.tm),NULL,NULL);
            if (t != (time_t)l/(time_t)f)
                (V)fprintf(stderr,
                    "// %s line %d: %ld -> %s (%04d-%02d-%02dT%02d:%02d:"
                    "%02dZ) -> %ld\n",__func__,__LINE__,l,buf,
                    cts.tm.tm_year+1900,cts.tm.tm_mon+1,cts.tm.tm_mday,
                    cts.tm.tm_hour,cts.tm.tm_min,cts.tm.tm_sec,(long)t);
            A(t==(time_t)l/(time_t)f);
#endif /* ASSERT_CODE */
        break;
        default :
            (V)fprintf(stderr,
               "%s: %s line %d: qsort type %d not handled in switch cases\n",
               __func__,source_file,__LINE__,data_type);
        return;
    }
    (V)strcat(buf,"\n");
}

void write_database_files(long *p, size_t n, unsigned int data_type)
{
    char path[PATH_MAX], buf[32];
    size_t i;
    FILE *f;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    for (i=0UL; i<n; i++) {
        (void)index_to_path(i, "data", path, PATH_MAX);
        f=fopen(path,"w");
        if (NULL==f) {
            (V)fprintf(stderr,"write_database_files: fopen(\"%s\",\"w\") failed\n",path);
        } else {
            ref_to_text(p[i],data_type,buf,32U);
            fputs(buf,f);
            fclose(f);
        }
    }
}

void initialize_indices(size_t n)
{
    size_t i;

    for (i=0UL; i<n; i++)
        global_larray[i]=(long)i;
}

int db_compare_struct(const long *pindx1, const long *pindx2)
{
    char buf1[32], buf2[32], p1[PATH_MAX], p2[PATH_MAX];
    unsigned char flags[256];
    const struct data_struct ds1, ds2;
    int (*compar)(const void *, const void *);

    index_to_path(*pindx1,"data",p1,PATH_MAX);
    index_to_path(*pindx2,"data",p2,PATH_MAX);
    read_text_file(p1,buf1,32U);
    read_text_file(p2,buf2,32U);
    text_to_datum(buf1,DATA_TYPE_STRUCT,&ds1);
    text_to_datum(buf2,DATA_TYPE_STRUCT,&ds2);
    flags[';']=0U; flags['i']=1U;
    compar=type_comparator(DATA_TYPE_STRUCT,flags);   
    flags[';']=1U;
    return compar(&ds1,&ds2);
}

int db_compare_string(const long *pindx1, const long *pindx2)
{
    char buf1[32], buf2[32], p1[PATH_MAX], p2[PATH_MAX];
    unsigned char flags[256];
    int (*compar)(const void *, const void *);

    index_to_path(*pindx1,"data",p1,PATH_MAX);
    index_to_path(*pindx2,"data",p2,PATH_MAX);
    read_text_file(p1,buf1,32U);
    read_text_file(p2,buf2,32U);
    flags[';']=0U; flags['i']=1U;
    compar=type_comparator(DATA_TYPE_STRING,flags);   
    flags[';']=1U;
    return compar(buf1,buf2);
}

int db_compare_double(const long *pindx1, const long *pindx2)
{
    char buf1[32], buf2[32], p1[PATH_MAX], p2[PATH_MAX];
    unsigned char flags[256];
    double d1, d2;
    int (*compar)(const void *, const void *);

    index_to_path(*pindx1,"data",p1,PATH_MAX);
    index_to_path(*pindx2,"data",p2,PATH_MAX);
    read_text_file(p1,buf1,32U);
    read_text_file(p2,buf2,32U);
    text_to_datum(buf1,DATA_TYPE_DOUBLE,&d1);
    text_to_datum(buf2,DATA_TYPE_DOUBLE,&d2);
    flags[';']=0U; flags['i']=1U;
    compar=type_comparator(DATA_TYPE_DOUBLE,flags);   
    flags[';']=1U;
    return compar(&d1,&d2);
}

int db_compare_long(const long *pindx1, const long *pindx2)
{
    char buf1[32], buf2[32], p1[PATH_MAX], p2[PATH_MAX];
    unsigned char flags[256];
    long l1, l2;
    int (*compar)(const void *, const void *);

    index_to_path(*pindx1,"data",p1,PATH_MAX);
    index_to_path(*pindx2,"data",p2,PATH_MAX);
    read_text_file(p1,buf1,32U);
    read_text_file(p2,buf2,32U);
    text_to_datum(buf1,DATA_TYPE_LONG,&l1);
    text_to_datum(buf2,DATA_TYPE_LONG,&l2);
    flags[';']=0U; flags['i']=1U;
    compar=type_comparator(DATA_TYPE_LONG,flags);   
    flags[';']=1U;
    return compar(&l1,&l2);
}

int db_compare_int(const long *pindx1, const long *pindx2)
{
    char buf1[32], buf2[32], p1[PATH_MAX], p2[PATH_MAX];
    unsigned char flags[256];
    int i1, i2;
    int (*compar)(const void *, const void *);

    index_to_path(*pindx1,"data",p1,PATH_MAX);
    index_to_path(*pindx2,"data",p2,PATH_MAX);
    read_text_file(p1,buf1,32U);
    read_text_file(p2,buf2,32U);
    text_to_datum(buf1,DATA_TYPE_INT,&i1);
    text_to_datum(buf2,DATA_TYPE_INT,&i2);
    flags[';']=0U; flags['i']=1U;
    compar=type_comparator(DATA_TYPE_INT,flags);   
    flags[';']=1U;
    return compar(&i1,&i2);
}

int db_compare_short(const long *pindx1, const long *pindx2)
{
    char buf1[32], buf2[32], p1[PATH_MAX], p2[PATH_MAX];
    unsigned char flags[256];
    short s1, s2;
    int (*compar)(const void *, const void *);

    index_to_path(*pindx1,"data",p1,PATH_MAX);
    index_to_path(*pindx2,"data",p2,PATH_MAX);
    read_text_file(p1,buf1,32U);
    read_text_file(p2,buf2,32U);
    text_to_datum(buf1,DATA_TYPE_SHORT,&s1);
    text_to_datum(buf2,DATA_TYPE_SHORT,&s2);
    flags[';']=0U; flags['i']=1U;
    compar=type_comparator(DATA_TYPE_SHORT,flags);   
    flags[';']=1U;
    return compar(&s1,&s2);
}
