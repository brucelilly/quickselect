/*INDENT OFF*/

/* Description: C source code for (modified) McIlroy antiqsort */
/* $Id: ~|^` @(#)   This is aqsort.c version 1.2 dated 2017-11-03T19:25:19Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.aqsort.c */

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: aqsort.c ~|^` @(#)"
#define SOURCE_MODULE "aqsort.c"
#define MODULE_VERSION "1.2"
#define MODULE_DATE "2017-11-03T19:25:19Z"
#define COPYRIGHT_HOLDER "M. Douglas McIlroy"
#define COPYRIGHT_DATE "1998"

/* header files needed */
#include "median_test_config.h" /* configuration */

#include "initialize_src.h"

/* ***** code copied from M.D.McIlroy "A Killer Adversary for Quicksort" ******* */
/* Copyright 1998, M. Douglas McIlroy
   Permission is granted to use or copy with this notice attached.

   Aqsort is an antiquicksort.  It will drive any qsort implementation
   based on quicksort into quadratic behavior, provided the imlementation
   has these properties:

   1.  The implementation is single-threaded.

   2.  The pivot-choosing phase uses O(1) comparisons.

   3.  Partitioning is a contiguous phase of n-O(1) comparisons, all
   against the same pivot value.

   4.  No comparisons are made between items not found in the array.
   Comparisons may, however, involve copies of those items.

   Method

   Values being sorted are dichotomized into "solid" values that are
   known and fixed, and "gas" values that are unknown but distinct and
   larger than solid values.  Initially all values are gas.  Comparisons
   work as follows:

   Solid-solid.  Compare by value.  Solid-gas.  Solid compares low.
   Gas-gas.  Solidify one of the operands, with a value greater
      than any previously solidified value.  Compare afresh.

   During partitioning, the gas values that remain after pivot-choosing
   will compare high, provided the pivot is solid.  Then qsort will go
   quadratic.  To force the pivot to be solid, a heuristic test
   identifies pivot candidates to be solidified in gas-gas comparisons.
   A pivot candidate is the gas item that most recently survived
   a comparison.  This heuristic assures that the pivot gets
   solidified at or before the second gas-gas comparison during the
   partitioning phase, so that n-O(1) gas values remain.

   To allow for copying, we must be able to identify an operand even if
   it was copied from an item that has since been solidified.  Hence we
   keep the data in fixed locations and sort pointers to them.  Then
   qsort can move or copy the pointers at will without disturbing the
   underlying data.

   int aqsort(int n, int *a); returns the count of comparisons qsort used
   in sorting an array of n items and fills in array a with the
   permutation of 0..n-1 that achieved the count.
*/
/* variable names have namespace prefix added */
/* type of ncmp modified to size_t */
/* first two are global for statistics reporting */
size_t antiqsort_ncmp; /* number of comparisons */
size_t antiqsort_nsolid; /* number of solid items */
size_t antiqsort_lsolid; /* next low solid value */ /* BL */
size_t antiqsort_hsolid; /* next high solid value */ /* BL */
int antiqsort_dir=1; /* BL */

static size_t pivot_candidate; /* pivot candidate */
static size_t antiqsort_gas; /* gas value */
static size_t antiqsort_n;
/* qsort uses one array, antiqsort another */
static long *antiqsort_base;
static char *qsort_base;
static size_t qsort_typsz;
static size_t qsort_type;
static FILE *fp = NULL;

#define CHANGE_DIR_ON_INIT_ONLY 1

/* freeze implemented as a function */
static
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void set_freeze_direction(void) /* BL */
{
#if CHANGE_DIR_ON_INIT_ONLY
    if (0UL==nfrozen) { /* BL */
#endif
        if (
        (antiqsort_gas-antiqsort_lsolid<antiqsort_hsolid-antiqsort_gas) /* BL */
        )
           antiqsort_dir = -1; /* BL */
        else
           antiqsort_dir = 1; /* BL */
#if DEBUG_CODE
        if (DEBUGGING(AQCMP_DEBUG)) {
            if ((char)0==file_initialized) initialize_file(__FILE__);
            (V)fprintf(stderr,
                "/* %s: %s line %d: antiqsort_dir=%d: lsolid=%lu, gas=%lu, hsolid=%lu, pivot_minrank=%lu */\n",
                __func__,source_file,__LINE__,antiqsort_dir,antiqsort_lsolid,
                antiqsort_gas,antiqsort_hsolid,pivot_minrank);
        }
#endif
#if CHANGE_DIR_ON_INIT_ONLY
    }
#endif
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
long freeze(long z)
{
    long l=antiqsort_base[z]; /* BL */

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (l==antiqsort_gas) { /* not already frozen */
        set_freeze_direction(); /* BL; in case freeze is called externally */
        if (0<antiqsort_dir) /* BL */
            antiqsort_base[z] = l = antiqsort_lsolid++; /* BL */
        else /* BL */
            antiqsort_base[z] = l = antiqsort_hsolid--; /* BL */
        antiqsort_nsolid++;
#if DEBUG_CODE
        if (DEBUGGING(AQCMP_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: %s(%ld)->%ld */\n",
                __func__,source_file,__LINE__,__func__,z,l);
# if 0
        else
            (V)fprintf(stderr,
                "/* %s: %s line %d: %s(%ld)->%ld */\n",
                __func__,source_file,__LINE__,__func__,z,l);
# endif
#endif
        /* progress indication */
        if (antiqsort_nsolid+1UL==antiqsort_n) {
            if (NULL != fp) {
                (V)fprintf(fp,"\r                      \r");
                fflush(fp);
                fclose(fp);
                fp=NULL;
            }
        } else {
            if (NULL != fp) {
                (V)fprintf(fp,"\r%s: %s line %d: %lu/%lu %lu%%: %s(%ld)->%ld           ",
                    __func__,source_file,__LINE__,
                    antiqsort_nsolid,antiqsort_n,
                    antiqsort_nsolid*100UL/antiqsort_n,
                    __func__,z,l
                );
#if DEBUG_CODE
                if (DEBUGGING(AQCMP_DEBUG))
                    (V)fprintf(fp,"\n");
#endif
                fflush(fp);
            }
        }
    }
#if DEBUG_CODE
    else if (DEBUGGING(AQCMP_DEBUG))
        (V)fprintf(stderr,
            "/* %s: %s line %d: z=%ld, l=%ld */\n",
            __func__,source_file,__LINE__,z,l);
#endif
    nfrozen++; /* BL */
    return l;
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
long aqindex(const void *pv, void *base, size_t size)
{
    long v;
    const struct data_struct *pds;
    struct civil_time_struct cts; /* for specific string format used in structured data */
    if ((char)0==file_initialized) initialize_file(__FILE__);
    switch (qsort_type) {
        case DATA_TYPE_LONG :
            v = *((const long *)pv);
        break;
        case DATA_TYPE_INT :
            v = (long)(*((const int *)pv));
        break;
        case DATA_TYPE_DOUBLE :
            v = (long)(*((const double *)pv));
        break;
        case DATA_TYPE_POINTER :
            pds = *((struct data_struct * const *)pv);
            (void)parse_civil_time_text(pds->string,&cts,NULL,-1,NULL,NULL,NULL);
            v = (long)utc_mktime(&(cts.tm), NULL, NULL);
        break;
        case DATA_TYPE_STRUCT : 
            pds = (const struct data_struct *)pv;
            (void)parse_civil_time_text(pds->string,&cts,NULL,-1,NULL,NULL,NULL);
            v = (long)utc_mktime(&(cts.tm), NULL, NULL);
        break;
        case DATA_TYPE_STRING :
            pds = (const struct data_struct *)pv;
            (void)parse_civil_time_text(pds->string,&cts,NULL,-1,NULL,NULL,NULL);
            v = (long)utc_mktime(&(cts.tm), NULL, NULL);
        break;
        case DATA_TYPE_UINT_LEAST8_T :
            v = (long)(*((const uint_least8_t *)pv));
        break;
        case DATA_TYPE_UINT_LEAST16_T :
            v = (long)(*((const uint_least16_t *)pv));
        break;
        case DATA_TYPE_UINT_LEAST32_T :
            v = (long)(*((const uint_least32_t *)pv));
        break;
        case DATA_TYPE_UINT_LEAST64_T :
            v = (long)(*((const uint_least64_t *)pv));
        break;
    }
#if DEBUG_CODE
    if (DEBUGGING(AQCMP_DEBUG)&&(NULL!=base)&&(0UL<size))
        (V)fprintf(stderr,"/* %s: %s line %d: %s(%p[%lu])=%ld */\n",
            __func__,source_file,__LINE__,__func__,pv,(pv-base)/size,v);
#endif
    return v;
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t count_frozen(char *base, size_t first, size_t beyond, size_t size)
{
    long v;
    size_t n;
    char *pa;

    for (n=0UL,pa=base+size*first; first<beyond; pa+=size,first++) {
        v=aqindex(pa,base,size);
        if (antiqsort_base[v]!=antiqsort_gas) n++;
    }
    return n;
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
size_t pivot_rank(char *base, size_t first, size_t beyond, size_t size, char *pivot)
{
    long l, v;
    size_t n;
    char *pa;

    v=antiqsort_base[aqindex(pivot,base,size)];
    for (n=0UL,pa=base+size*first; first<beyond; pa+=size,first++) {
#if DEBUG_CODE > 1
if (DEBUGGING(AQCMP_DEBUG)) (V)fprintf(stderr,"/* %s: %s line %d: v=%ld, index=%lu */\n",__func__,source_file,__LINE__,v,first);
#endif
        if (pa==pivot) continue;
        l=antiqsort_base[aqindex(pa,base,size)];
        if (l<v) n++;
#if DEBUG_CODE > 1
if (DEBUGGING(AQCMP_DEBUG)) (V)fprintf(stderr,"/* %s: %s line %d: v=%ld, index=%lu, value=%ld, n=%lu */\n",__func__,source_file,__LINE__,v,first,l,n);
#endif
    }
    return n;
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
/* spacings are in elements */
/* remedian does not modify the value pointed to by middle, but it is not
   declared as const to avoid spurious compiler warnings about discarding the
   const qualifier when passing middle to fmed3 (which also doesn't make
   modifications, but see the rationale there)
*/
void pivot_sample_rank(register char *middle, register size_t row_spacing,
    register size_t sample_spacing, register size_t size,
    register unsigned int idx, char *pivot, size_t *prank, char *base)
{
    register size_t o;
    long l, v;
#if DEBUG_CODE
if (DEBUGGING(WQSORT_DEBUG)||DEBUGGING(REMEDIAN_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: middle=%p[%lu], row_spacing=%lu, sample_spacing=%lu, idx=%u */\n",
__func__,source_file,__LINE__,(const void *)middle,(middle-base)/size,(unsigned long)row_spacing,(unsigned long)sample_spacing,idx);
}
#endif
    A((SAMPLING_TABLE_SIZE)>idx);
    if ((0U<idx)&&(0U < --idx)) {
        char *pa, *pb, *pc;
        register size_t s=sample_spacing/3UL;

        o=s*size;
        pivot_sample_rank(middle-o,row_spacing,s,size,idx,pivot,prank,base);
        pivot_sample_rank(middle,row_spacing,s,size,idx,pivot,prank,base);
        pivot_sample_rank(middle+o,row_spacing,s,size,idx,pivot,prank,base);
        return;
    }
    o=row_spacing*size;
    v=antiqsort_base[aqindex(pivot,base,size)];
#if DEBUG_CODE
if (DEBUGGING(WQSORT_DEBUG)||DEBUGGING(REMEDIAN_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: pivot=%p[%lu], value=%ld */\n",
__func__,source_file,__LINE__,(const void *)pivot,(pivot-base)/size,v);
}
#endif
    l=antiqsort_base[aqindex(middle-o,base,size)];
#if DEBUG_CODE
if (DEBUGGING(WQSORT_DEBUG)||DEBUGGING(REMEDIAN_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: middle-o=%p[%lu], value=%ld */\n",
__func__,source_file,__LINE__,(const void *)(middle-o),((middle-o)-base)/size,l);
}
#endif
    if (l<v) (*prank)++;
    l=antiqsort_base[aqindex(middle,base,size)];
#if DEBUG_CODE
if (DEBUGGING(WQSORT_DEBUG)||DEBUGGING(REMEDIAN_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: middle=%p[%lu], value=%ld */\n",
__func__,source_file,__LINE__,(const void *)(middle),((middle)-base)/size,l);
}
#endif
    if (l<v) (*prank)++;
    l=antiqsort_base[aqindex(middle+o,base,size)];
#if DEBUG_CODE
if (DEBUGGING(WQSORT_DEBUG)||DEBUGGING(REMEDIAN_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: middle+o=%p[%lu], value=%ld */\n",
__func__,source_file,__LINE__,(const void *)(middle+o),((middle+o)-base)/size,l);
}
#endif
    if (l<v) (*prank)++;
}

/* comparison function called by qsort; values corresponding to pointers are
   used as indices into the antiqsort array
*/
int aqcmp(const void *px, const void *py) /* per C standard */
{
    long a, b, x, y;
    const struct data_struct *pds;
    struct civil_time_struct cts; /* for specific string format used in structured data */

    antiqsort_ncmp++;
    x = aqindex(px,NULL,0UL);
    y = aqindex(py,NULL,0UL);
if ((0L>x)||(x>=antiqsort_n)) (V)fprintf(stderr,"/* %s: %s line %d: x (%ld) is out of range 0<=x<%ld */\n",__func__,source_file,__LINE__,x,antiqsort_n);
if ((0L>y)||(y>=antiqsort_n)) (V)fprintf(stderr,"/* %s: %s line %d: y (%ld) is out of range 0<=y<%ld */\n",__func__,source_file,__LINE__,y,antiqsort_n);
    A(0L<=x);A(x<antiqsort_n);A(0L<=y);A(y<antiqsort_n);
    a=antiqsort_base[x];
    b=antiqsort_base[y];
#if DEBUG_CODE
    if (DEBUGGING(AQCMP_DEBUG))
        (V)fprintf(stderr,
            "/* %s: %s line %d: x=%ld, y=%ld, a=%ld, b=%ld */\n",
            __func__,source_file,__LINE__,x,y,a,b);
#endif
    /* brackets added BL */
    if ((a==antiqsort_gas) && (b==antiqsort_gas)) {
        if (0UL==nfrozen) set_freeze_direction(); /* BL; before candidate selection here */
        if (nfrozen<=pivot_minrank) { /* BL added condition */
            if (0<antiqsort_dir) {
                if (x == pivot_candidate) {
                    a=freeze(x);
                    if (x==y) b=a;
                } else {
                    b=freeze(y);
                    if (x==y) a=b;
                }
            } else {
                if (y == pivot_candidate) {
                    b=freeze(y);
                    if (x==y) a=b;
                } else {
                    a=freeze(x);
                    if (x==y) b=a;
                }
            }
        }
    }
    if (0<antiqsort_dir) {
        if (a==antiqsort_gas) {
            pivot_candidate = x;
        } else if (b==antiqsort_gas) {
            pivot_candidate = y;
        }
    } else {
        if (b==antiqsort_gas) {
            pivot_candidate = y;
        } else if (a==antiqsort_gas) {
            pivot_candidate = x;
        }
    }
#if 0 /* original code: applicable only to plain integers, return can overflow/underflow */
int
cmp(const void *px, const void *py)  /* per C standard */
{
        const int x = *(const int*)px;
        const int y = *(const int*)py;
        ncmp++;
        if(val[x]==gas && val[y]==gas)
                if(x == candidate)
                        freeze(x);
                else
                        freeze(y);
        if(val[x] == gas)
                candidate = x;
        else if(val[y] == gas)
                candidate = y;
        return val[x] - val[y];
}
#endif
    if (a > b) return 1; else if (a < b) return -1; return 0;
}
/* ********** modified into a separate function for intitalization ************ */
/* added args: pv points to array of type "type" elements each having size sz used by qsort, alt to array used by antiqsort */
void initialize_antiqsort(size_t n, char *pv, int type, const size_t sz, long *alt)
{
    size_t i;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    A(NULL!=pv);A(NULL!=alt);A(pv!=(char *)alt);
    qsort_base = pv;
    qsort_type = type;
    qsort_typsz = sz;
    antiqsort_n = n;
    antiqsort_base = alt;
    antiqsort_lsolid=0UL; /* BL */
    antiqsort_hsolid=n-1UL; /* BL */
    antiqsort_nsolid = 0UL;
    pivot_candidate = 0UL;
    antiqsort_ncmp = 0UL;
    /* Progress indication to /dev/tty */
    if (NULL==fp) fp = fopen("/dev/tty", "w");
    if (NULL != fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
    i=(size_t)d_sample_index(sorting_sampling_table,2U,n);
    nfrozen=0UL, pivot_minrank=minimum_remedian_rank(i);
    if (4UL>n) antiqsort_gas=n-1UL; else antiqsort_gas=(n>>1)-pivot_minrank;
#if DEBUG_CODE
    if (DEBUGGING(AQCMP_DEBUG))
        (V)fprintf(stderr,
            "/* %s: %s line %d: n=%lu, pv=%p, type=%d, sz=%lu, alt=%p, gas=%ld, lsolid=%lu, hsolid=%lu, sampling table index=%lu, pivot_minrank=%lu */\n",
            __func__,source_file,__LINE__,(unsigned long)n,(void *)pv,type,
            (const unsigned long)sz,(void *)alt,antiqsort_gas,antiqsort_lsolid,
            antiqsort_hsolid,i,pivot_minrank);
#endif
    for(i=0UL; i<n; i++) {
        /* initialize the array that qsort uses; values 0 to n-1 */
        switch (qsort_type) {
            case DATA_TYPE_UINT_LEAST8_T :
                {
                    uint_least8_t *p1;
                    p1 = (uint_least8_t *)(qsort_base + qsort_typsz*i);
                    *p1 = (uint_least8_t)(i&0x0FFUL);
                }
            break;
            case DATA_TYPE_UINT_LEAST16_T :
                {
                    uint_least16_t *p2;
                    p2 = (uint_least16_t *)(qsort_base + qsort_typsz*i);
                    *p2 = (uint_least16_t)(i&0x0FFFFUL);
                }
            break;
            case DATA_TYPE_UINT_LEAST32_T :
                {
                    uint_least32_t *p4;
                    p4 = (uint_least32_t *)(qsort_base + qsort_typsz*i);
                    *p4 = (uint_least32_t)(i&0x07FFFFFFFUL);
                }
            break;
            case DATA_TYPE_UINT_LEAST64_T :
                {
                    uint_least64_t *p8;
                    p8 = (uint_least64_t *)(qsort_base + qsort_typsz*i);
                    *p8 = (uint_least64_t)i;
                }
            break;
            case DATA_TYPE_LONG :
                {
                    long *plong;
                    plong = (long *)(qsort_base + qsort_typsz*i);
                    *plong = (long)i;
                }
            break;
            case DATA_TYPE_INT :
                {
                    int *pint;
                    pint = (int *)(qsort_base + qsort_typsz*i);
                    *pint = (int)i;
                }
            break;
            case DATA_TYPE_DOUBLE :
                {
                    double *pd;
                    pd = (double *)(qsort_base + qsort_typsz*i);
                    *pd = (double)i;
                }
            break;
            case DATA_TYPE_POINTER :
                {
                    struct data_struct *pds;
                    struct civil_time_struct cts;
                    unsigned long ul;
                    ul = (unsigned long)i; /* unsigned long may differ from size_t on 32-bit *BSD */
                    time_t t; /* time_t might have greater range than unsigned long on 32-bit *BSD */

#if ULONG_MAX > 0xffffffffUL
                    ul %= 253402300800UL; /* max. 9999-12-31T23:59:59 */
#endif
                    t = (time_t)ul;
                    gmtime_r(&t, &(cts.tm));
                    cts.fraction=cts.offset=0.0;
                    pds = *((struct data_struct **)(qsort_base + qsort_typsz*i));
                    (V)sn_civil_time(pds->string, STRING_SIZE, &cts, 0, 1, 0, NULL, NULL);
#if ASSERT_CODE
                    t = utc_mktime(&(cts.tm),NULL,NULL);
                    if (t != (time_t)ul) (V)fprintf(stderr, "/* %s: %s line %d: %ld -> %s (%04d-%02d-%02dT%02d:%02d:%02dZ) -> %ld */\n",__func__,source_file,__LINE__,ul,pds->string,cts.tm.tm_year+1900,cts.tm.tm_mon+1,cts.tm.tm_mday,cts.tm.tm_hour,cts.tm.tm_min,cts.tm.tm_sec,(long)t);
                    A(t==(time_t)ul);
#endif /* ASSERT_CODE */
                    pds->year=cts.tm.tm_year+1900,pds->month=cts.tm.tm_mon+1;
//                    pds->yday=cts.tm.tm_yday+1,pds->wday=cts.tm.tm_wday;
                    pds->mday=cts.tm.tm_mday,pds->hour=cts.tm.tm_hour;
                    pds->minute=cts.tm.tm_min,pds->second=cts.tm.tm_sec;
                }
            break;
            case DATA_TYPE_STRUCT :
                {
                    struct data_struct *pds;
                    struct civil_time_struct cts;
                    unsigned long ul;
                    ul = (unsigned long)i; /* unsigned long may differ from size_t on 32-bit *BSD */
                    time_t t; /* time_t might have greater range than unsigned long on 32-bit *BSD */

#if ULONG_MAX > 0xffffffffUL
                    ul %= 253402300800UL; /* max. 9999-12-31T23:59:59 */
#endif
                    t = (time_t)ul;
                    gmtime_r(&t, &(cts.tm));
                    cts.fraction=cts.offset=0.0;
                    pds = (struct data_struct *)(qsort_base + qsort_typsz*i);
                    (V)sn_civil_time(pds->string, STRING_SIZE, &cts, 0, 1, 0, NULL, NULL);
#if ASSERT_CODE
                    t = utc_mktime(&(cts.tm),NULL,NULL);
                    if (t != (time_t)ul) (V)fprintf(stderr, "/* %s: %s line %d: %ld -> %s (%04d-%02d-%02dT%02d:%02d:%02dZ) -> %ld */\n",__func__,source_file,__LINE__,ul,pds->string,cts.tm.tm_year+1900,cts.tm.tm_mon+1,cts.tm.tm_mday,cts.tm.tm_hour,cts.tm.tm_min,cts.tm.tm_sec,(long)t);
                    A(t==(time_t)ul);
#endif /* ASSERT_CODE */
                    pds->year=cts.tm.tm_year+1900,pds->month=cts.tm.tm_mon+1;
//                    pds->yday=cts.tm.tm_yday+1,pds->wday=cts.tm.tm_wday;
                    pds->mday=cts.tm.tm_mday,pds->hour=cts.tm.tm_hour;
                    pds->minute=cts.tm.tm_min,pds->second=cts.tm.tm_sec;
                }
            break;
            case DATA_TYPE_STRING :
                {
                    struct data_struct *pds;
                    struct civil_time_struct cts;
                    unsigned long ul;
                    ul = (unsigned long)i; /* unsigned long may differ from size_t on 32-bit *BSD */
                    time_t t; /* time_t might have greater range than unsigned long on 32-bit *BSD */

#if ULONG_MAX > 0xffffffffUL
                    ul %= 253402300800UL; /* max. 9999-12-31T23:59:59 */
#endif
                    t = (time_t)ul;
                    gmtime_r(&t, &(cts.tm));
                    cts.fraction=cts.offset=0.0;
                    pds = (struct data_struct *)(qsort_base + qsort_typsz*i);
                    (V)sn_civil_time(pds->string, STRING_SIZE, &cts, 0, 1, 0, NULL, NULL);
#if ASSERT_CODE
                    t = utc_mktime(&(cts.tm),NULL,NULL);
                    if (t != (time_t)ul) (V)fprintf(stderr, "/* %s: %s line %d: %ld -> %s (%04d-%02d-%02dT%02d:%02d:%02dZ) -> %ld */\n",__func__,source_file,__LINE__,ul,pds->string,cts.tm.tm_year+1900,cts.tm.tm_mon+1,cts.tm.tm_mday,cts.tm.tm_hour,cts.tm.tm_min,cts.tm.tm_sec,(long)t);
                    A(t==(time_t)ul);
#endif /* ASSERT_CODE */
                    pds->year=cts.tm.tm_year+1900,pds->month=cts.tm.tm_mon+1;
//                    pds->yday=cts.tm.tm_yday+1,pds->wday=cts.tm.tm_wday;
                    pds->mday=cts.tm.tm_mday,pds->hour=cts.tm.tm_hour;
                    pds->minute=cts.tm.tm_min,pds->second=cts.tm.tm_sec;
                }
            break;
        }
        /* initialize the array that antiqsort uses; all "gas" */
        antiqsort_base[i] = (long)antiqsort_gas;
    }
}
/* ***************************************************************************** */
