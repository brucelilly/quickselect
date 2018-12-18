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
* $Id: ~|^` @(#)    median_test.c copyright 2016-2018 Bruce Lilly.   \ median_test.c $
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
/* $Id: ~|^` @(#)   This is median_test.c version 1.39 dated 2018-12-18T13:19:52Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/src/s.median_test.c */

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
#define ID_STRING_PREFIX "$Id: median_test.c ~|^` @(#)"
#define SOURCE_MODULE "median_test.c"
#define MODULE_VERSION "1.39"
#define MODULE_DATE "2018-12-18T13:19:52Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

#define  __STDC_WANT_LIB_EXT1__ 1 /* for qsort_s_wrapper */

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes most other local and system header files required */

#include "indirect.h"           /* set_array_pointers */

#include "initialize_src.h"

extern size_t quickselect_cache_size;
extern double mos_middle_power;
extern double mos_ends_power;

/* shell specials: ;&()|<> \"'#$`~{}*?[ */
/* getopt treats ? and : specially (: may be used as a magic silence flag) */
/* available characters: 012678(){}\'"`  plus control characters and whitespace */
#define OPTSTRING "a:Ab:BcC:d:D:eEfFgGhHiIjJ:k:KlLm:M:nNoO:p:Pq:Q:r:RsSt:T:uU:vVwW:xXY:y:zZ3:4:59!@#:%+,.|:/:~_<*&$[=>];"
#define USAGE_STRING     "[-a [opts]] [-A] [-b [0]] [-B] [-c [c1[,c2[,c3[...]]]]] [-C sequences] [-d debug_values] [-D [n]] [-e] [-E] [-f] [-F] [-g] [-G] [-h] [-H] [-i] [-I] [-j] [-J [gap[,gap[,...]]]] [-k col] [-l] [-L] [-m [t[,n[,f[,c[,l]]]]] [-M [opts]] [-n] [-N] [-o] [-O n] [-P] [-q [n[,f[,c[,l]]]]] [-Q timeout] [-r [i[,n[,f]]]] [-R] [-s] [-S] [-t [c1[,c2[,c3[...]]]]] [-T sequences] [-u] [-U n] [-v] [-V] [-w] [-W [f,c[,l]]] [-x] [-X] [-Y n,n] [-y [n]] [-z] [-Z] [-3 [c1[,c2[,c3[...]]]]] [-4 c] [-5] [-9] [-!] [-# n] [-+] [-,] [-.] [-| n] [-/ n] [-;] [-~] [-<] [-*] [-$] [-= functions] [->] ['-] cachesz'] -- [[start incr]] [size [count]]\n\
-a [opts]\ttest illumos qsort, possibly with modifications\n\
-A\talphabetic (string) data type tests\n\
-b [0]\ttest Bentley&McIlroy qsort optionally w/o instrumentation\n\
-B\toutput data for drawing box plots\n\
-c\ttest simplified sort-only quickselect\n\
-C sequences\tinclude correctness tests for specified sequences\n\
-d debug_values\tturn on debugging output for specified values\n\
-D [n]\ttest dual-pivot qsort code with optional small-array sort cutoff\n\
-e\ttest heapsort code\n\
-E\ttest selection sort\n\
-f\ttest sqrt sort\n\
-F\tfloating-point (double) tests\n\
-g\ttest glibc qsort code\n\
-G\tuse Shell sort instead of insertion sort for small arrays\n\
-h\tomit program headings (args, parameters, machine)\n\
-H\toutput host information\n\
-i\tuse instrumented comparison functions and report results\n\
-I\tinteger data type tests\n\
-j\ttest insertion sort\n\
-J [gap[,gap[,...]]]\ttest Shell sort with specified gaps\n\
-k col\talign timing numbers at column col\n\
-K\twrite integer sequence generated by adversary (if given with -a)\n\
-l\ttest library qsort\n\
-L\tlong integer data type tests\n\
-m [t[,n[,f[,c[,l]]]]]\ttest quickselect selection with optional order statistics distribution type, number of order statistics, repivot_factor, repivot_cutoff, lopsided_partition_limit\n\
-M [opts]\ttest modified Bentley&McIlroy qsort with specified modification options\n\
-n\tdo nothing except as specified by option flags\n\
-N\ttest NetBSD qsort code\n\
-o\tprint execution costs of operations on basic types\n\
-O n\tset sampling nmemb breakpoints according to Beta value n\n\
-p n\tpivot_method (only effective for first partition)\n\
-P\tpointer to structure data type tests\n\
-q [n[,f[,c]]]\ttest quickselect sort with optional small-array sort cutoff, repivot_factor, and repivot_cutoff and lopsided_partition_limit\n\
-Q timeout\ttime out tests after timeout seconds\n\
-r [i[,n[,f]]]\ttest introsort sort with optional final insertion sort flag, small-array sort cutoff and recursion depth factor\n\
-R\traw timing output (median time (and comparison counts if requested) only)\n\
-s\tprint sizes of basic types\n\
-S\tstructured data type tests\n\
-t[c c1[,c2[,c3[...]]]]\tset breakpoints c1, etc. in median of samples sampling table c\n\
-T sequences\tinclude timing tests for specified sequences\n\
-u\ttest preliminary 9899:201x draft N1570 qsort_s implementation based on quickselect\n\
-U n\tset sampling nmemb breakpoints according to factor n\n\
-v\ttest sorting networks for small arrays\n\
-V\tuse sorting networks instead of insertion sort for small arrays\n\
-w\ttest qsort_wrapper\n\
-W [f,c]\ttest lopsided partitions in quickselect with optional repivot_factor and repivot_cutoff and lopsided_partition_limit\n\
-x\ttest log sort\n\
-X\ttest smoothsort\n\
-Y nm,ne\tset power function exponent for median of samples middle and ends sampling tables\n\
-y [n]\tYaroslavskiy's dual-pivot sort with optional insertion sort cutoff\n\
-z\tset repeatable random nZumber generator state\n\
-Z\tdisable repivoting\n\
-3 c1[,c2[,...]]\tset remedian sampling table breakpoint sizes\n\
-4 c\tset small-array sort cutoff to c (disable use of small-array dedicated sort above size c)\n\
-5\tsave partial partitioning comparisons\n\
-9\ttest plan9 qsort\n\
-!\tcollect and report statistics for small array insertion sort\n\
-@\tignore sampling breakpoint table\n\
-# n\tsort if nk/nmemb>k\n\
-%\tdo not reset factor counter when selecting order statistics\n\
-+\ttest minmaxsort\n\
-,\ttest in-place merge sort\n\
-.\ttest dedicated sort\n\
-| n\tpartition_method\n\
-/ n\tnetwork_map (obsolete)\n\
-~\tperform sorting stability test on structured data\n\
-_\ttest system mergesort\n\
-<\toptimize quickselect for minimum comparisons\n\
-&\tinternal indirection\n\
-*\ttest indirect_mergesort\n\
-$\tshort integer data type tests\n\
-[r\tsimulate type size increase by a ratio r\n\
-= names\ttest named functions (rexexp)\n\
->\ttest sorting small arrays by merging runs\n\
-] n\tset cache size to n, overriding determination by system\n\
-;\tsimulate database lookup of data elements\n\
start\tbegin testing with array size\n\
incr\tincrement array size (expression)\n\
size\tnumber of items in each test (maximum size if start is given) (default 10000)\n\
count\tnumber of times to run each test (default 10)"

/* static functions */
static const char *sampling_table_name(struct sampling_table_struct *psts)
{
    if (psts==sorting_sampling_table) return "ros_sorting";
    if (psts==middle_sampling_table) return "ros_middle";
    if (psts==ends_sampling_table) return "ros_ends";
    if (psts==mos_sorting_sampling_table) return "mos_sorting";
    if (psts==mos_middle_sampling_table) return "mos_middle";
    if (psts==mos_ends_sampling_table) return "mos_ends";
    return "unknown";
}

static const char *pivot_name(int method)
{
    static char buf[256];

    switch (method) {
        case QUICKSELECT_PIVOT_REMEDIAN_SAMPLES :
        return "QUICKSELECT_PIVOT_REMEDIAN_SAMPLES" ;
        case QUICKSELECT_PIVOT_REMEDIAN_FULL :
        return "QUICKSELECT_PIVOT_REMEDIAN_FULL" ;
        case QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS :
        return "QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS" ;
        case QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES :
        return "QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES" ;
        default:
            (V)snl(buf,sizeof(buf),"unknown method ",NULL,(long)method,10,' ',1,NULL,NULL);
        break;
    }
    return (const char *)buf;
}

static const char *partition_name(int method)
{
    static char buf[256];

    switch (method) {
        case QUICKSELECT_PARTITION_FAST :
        return "QUICKSELECT_PARTITION_FAST" ;
#if QUICKSELECT_STABLE
        case QUICKSELECT_PARTITION_STABLE :
        return "QUICKSELECT_PARTITION_STABLE" ;
#endif
        default:
            (V)snl(buf,sizeof(buf),"unknown method ",NULL,(long)method,10,' ',1,NULL,NULL);
        break;
    }
    return (const char *)buf;
}

static const char *modification_name(unsigned int code)
{
    static char buf[256];

    switch (code) {
        case MOD_SAMPLE_QUALITY :
        return "MOD_SAMPLE_QUALITY";
        case MOD_SAMPLE_QUANTITY :
        return "MOD_SAMPLE_QUANTITY";
        case MOD_ISORT_LS :
        return "MOD_ISORT_LS";
        case MOD_ISORT_BS :
        return "MOD_ISORT_BS";
        case MOD_DEDICATED_SORT :
        return "MOD_DEDICATED_SORT";
        case MOD_TERNARY :
        return "MOD_TERNARY";
        default:
            (V)snul(buf,sizeof(buf),"unknown code ",NULL,(unsigned long)code,
                10,' ',1,NULL,NULL);
        break;
    }
    return (const char *)buf;
}

static void decode_options(FILE *fp, unsigned int options, const char *prefix,
    const char *suffix)
{
    char buf[256];
    int partition_method, pivot_method;
    unsigned int u;

    (V)snul(buf,sizeof(buf),NULL,NULL,options,2,'0',1,NULL,NULL);
    (V)fprintf(stdout, "%soptions = (decimal)%u = 0X%XU = (binary)%s%s\n",
        prefix,options,options,buf,suffix);
    /* defaults */
    partition_method = QUICKSELECT_PARTITION_FAST;
    pivot_method = QUICKSELECT_PIVOT_REMEDIAN_SAMPLES;
    for (u=0x01U; 0U!=options; options&=~u,u<<=1) {
        if (0U!=(u&(MOD_OPTIONS))) continue;
        if (0U!=(options&u)) {
            switch (u) {
#if QUICKSELECT_STABLE
                case QUICKSELECT_STABLE :
                    partition_method = QUICKSELECT_PARTITION_STABLE;
                break;
#endif
                case QUICKSELECT_OPTIMIZE_COMPARISONS :
                    (V)fprintf(stdout,
                        "%soptimized for complex comparisons%s\n",
                        prefix,suffix);
                    if (QUICKSELECT_PARTITION_FAST==partition_method)
                        pivot_method = QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES;
                break;
                case QUICKSELECT_RESTRICT_RANK :
                    if (QUICKSELECT_PARTITION_FAST==partition_method)
                        pivot_method = QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS;
                    else
                        pivot_method = QUICKSELECT_PIVOT_REMEDIAN_FULL;
                break;
                case QUICKSELECT_INDIRECT :
                    (V)fprintf(stdout,
                        "%sinternal indirection%s\n",
                        prefix,suffix);
                    /* indirect means size_ratio==1 */
                    if (QUICKSELECT_PARTITION_FAST==partition_method)
                        pivot_method = QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES;
                break;
                case QUICKSELECT_NO_REPIVOT :
                    (V)fprintf(stdout,
                        "%sno repivot%s\n",
                        prefix,suffix);
                break;
                case QUICKSELECT_STRICT_SELECTION :
                    (V)fprintf(stdout,
                        "%sstrict selection%s\n",
                        prefix,suffix);
                break;
                default :
                    (V)snul(buf,sizeof(buf),NULL,NULL,u,2,'0',1,NULL,NULL);
                    (V)fprintf(stderr,
                        "%sunrecognized option (decimal)%u = 0X%XU ="
                        " (binary)%s%s\n",
                        prefix,u,u,buf,suffix);
                break;
            }
        }
    }
    (V)fprintf(stdout,
        "%spartition_method = %s%s\n",
        prefix,partition_name(partition_method),suffix);
    (V)fprintf(stdout,
        "%spivot_method = %s%s\n",
        prefix,pivot_name(pivot_method),suffix);
}

/* support functions */
/* comparison functions for sorting stability tests */
static int fractioncmp(const struct data_struct *p1, const struct data_struct *p2)
{
    int n;

    for (n=0; n<6; n++) {
        /* comparing packed pairs of BCD digits as unsigned integers is OK */
        if (p1->fractional[n]>p2->fractional[n]) return 1;
        if (p1->fractional[n]<p2->fractional[n]) return -1;
    }
    return 0;
}

static int secondscmp(const struct data_struct *p1, const struct data_struct *p2)
{
    if (p1->second>p2->second) return 1;
    if (p1->second<p2->second) return -1;
    return 0;
}

static int minutescmp(const struct data_struct *p1, const struct data_struct *p2)
{
    if (p1->minute>p2->minute) return 1;
    if (p1->minute<p2->minute) return -1;
    return 0;
}

static int hourscmp(const struct data_struct *p1, const struct data_struct *p2)
{
    if (p1->hour>p2->hour) return 1;
    if (p1->hour<p2->hour) return -1;
    return 0;
}

static int daycmp(const struct data_struct *p1, const struct data_struct *p2)
{
    if (p1->u_var.s_md.mday>p2->u_var.s_md.mday) return 1;
    if (p1->u_var.s_md.mday<p2->u_var.s_md.mday) return -1;
    return 0;
}

static int monthcmp(const struct data_struct *p1, const struct data_struct *p2)
{
    if (p1->u_var.s_md.month>p2->u_var.s_md.month) return 1;
    if (p1->u_var.s_md.month<p2->u_var.s_md.month) return -1;
    return 0;
}

static int yearcmp(const struct data_struct *p1, const struct data_struct *p2)
{
    if (p1->year>p2->year) return 1;
    if (p1->year<p2->year) return -1;
    return 0;
}

static const char *repivot_comment[7] = {
    " /* single sample */", " /* median-of-3 */", " /* remedian of samples */", "", "", "", ""
};

/* count the number of occurrences of character c in string s */
static unsigned int strccount(const char *s, int c)
{
    unsigned int ret=0U;
    if (NULL!=s) for (; '\0'!=*s; s++) if (*s==c) ret++;
    return ret;
}

/* return the number of 1 bits in x */
static unsigned int bitcount(unsigned int x)
{
    unsigned int mask, ret=0U;
    for (mask=0x01U; 0u!=mask; mask<<=1) if (0U!=(x&mask)) ret++;
    return ret;
}

/* test sorting partial order stability */
static int stability_test(long *refarray, struct data_struct *data_array,
    unsigned int functions, unsigned long n, uint_least64_t *s, unsigned int p,
    size_t ratio, unsigned int options, void (*f)(int, void *, const char *, ...),
    void *log_arg)
{
    int c, i, ret=0;
    size_t sz, u, w, x;
    unsigned int errs=0U, v;
    int(*compar[])(const void *,const void *) = {
        (int(*)(const void *,const void *))fractioncmp,
        (int(*)(const void *,const void *))secondscmp,
        (int(*)(const void *,const void *))minutescmp,
        (int(*)(const void *,const void *))hourscmp,
        (int(*)(const void *,const void *))daycmp,
        (int(*)(const void *,const void *))monthcmp,
        (int(*)(const void *,const void *))yearcmp
    };
    const char *fname;

    u=n-1UL;
    A(NULL!=data_array);
    sz=ratio*sizeof(struct data_struct);
    if (0U!=functions) {
        for (v=0U; v<FUNCTION_COUNT; v++) {
            if (0U!=(functions & (0x01U << v))) {
                i = seed_random64n(s, p);
                if (0 != i) {
                    f(LOG_ERR, log_arg,
                        "%s: %s line %d: seed_random64n(0x%lx, %u) returned %d:"
                        " %m",__func__,source_file,__LINE__,
                        (unsigned long)s, p, i
                    );
                    errs++;
                } else {
                    switch (v) {
                        case FUNCTION_QSELECT :
                        /*FALLTHROUGH*/
                        case FUNCTION_QSELECT_S :
                            continue; /* test stability for sorting */
                        break;
                    }
                    fname = function_name(v);
                    /* initialize shuffled distinct data structures */
                    /* generate new test sequence */
                    /* times differ by 8:12:15+
                       2 values (0,0.5) for fraction on 32-bit machines
                         100 values (0.01s increments) on 64-bits
                       4 values (00,15,30,45) for seconds (60=2*2*3*5)
                       5 values (00,12,24,26,48) for minutes (60=2*2*3*5)
                       3 values (00,08,16) for hour (24=2*2*2*3)
                       31 values for mday
                       12 values for month
                       year [1970,7815] (64-bits, [1970,2002] for 32-bits)
                    */
                    c=generate_long_test_array(refarray,n,
                        TEST_SEQUENCE_RANDOM_DISTINCT,
#if LONG_MAX > 0x7fffffffL
                        500000L+50000000L*(15L+12L*60L+8L*3600L)
                             /* 8:12:15.01 */,
#else
                        1L+2L*(15L+12L*60L+8L*3600L) /* 8:12:15.5 */,
#endif
                        LONG_MAX,f,log_arg);
                    if (0 > c) {
                        (V)fprintf(stderr,
                            "%s: %s line %d: generate_long_test_array returned %d\n",
                            __func__, source_file, __LINE__, c);
                        return -1;
                    } else if (0 < c) {
                        return -1;
                    }
                    /* copy test sequence to alternates */
                    duplicate_test_data(refarray,(char *)data_array,
                        DATA_TYPE_STRUCT,ratio,0UL,n);
#if 0
                    (V)fprintf(stderr,
                        "%s: %s line %d: ul=%lu, n=%lu, u=%lu\n",
                        __func__,source_file,__LINE__,
                        (unsigned long)ul,
                        (unsigned long)n,(unsigned long)u);
                    print_some_array(data_array,0UL,u,"/* "," */",options);
#endif
                    /* sort stably by fraction */
                    /* sort stably by seconds */
                    /* sort stably by minutes */
                    /* sort stably by hours */
                    /* sort stably by days */
                    /* sort stably by months */
                    /* sort stably by years */
                    for (w=0UL; w<7UL; w++) {
                        switch (v) {
                            case FUNCTION_QSELECT_SORT :  /*FALLTHROUGH*/
                                /* quickselect_internal */
                                QSEL(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_BMQSORT :
                                BMQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_DEDSORT :
                                DEDSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_DPQSORT :
                                DPQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_GLQSORT :
                                GLQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_HEAPSORT :
                                HEAPSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_IBMQSORT :
                                IBMQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_INDIRECT_MERGESORT :
                                IMERGESORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_INTROSORT :
                                INTROSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_ISORT :
                                ISORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_LOGSORT :
                                LOGSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_MBMQSORT :
                                MBMQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,options,0);
                            break;
                            case FUNCTION_MERGESORT :
                                MERGESORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_MINMAXSORT :
                                MMSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_NBQSORT :
                                NBQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_NETWORKSORT :
                                NETWORKSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_P9QSORT :
                                P9QSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_ILLUMOSQSORT :
                                ILLUMOSQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,options,0);
                            break;
                            case FUNCTION_QSORT :
                                QSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_QSORT_WRAPPER :
                                /* use quickselect to support options */
                                quickselect(data_array,n,sz,compar[w],NULL,0UL,options&(QUICKSELECT_USER_OPTIONS_MASK));
                            break;
                            case FUNCTION_SELSORT :
                                SELSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_SHELLSORT :
                                SHELLSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_SMOOTHSORT :
                                SMOOTHSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_SQRTSORT :
                                SQRTSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_SQSORT :
                                SQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_SYSMERGESORT :
                                SYSMERGESORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_WQSORT :
                                WQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                            case FUNCTION_YQSORT :
                                YQSORT(data_array,0UL,u,sz,compar[w],NULL,0UL,0U,0);
                            break;
                        }
#if 0
                        print_some_array(data_array,0UL,u,"/* "," */",options);
#endif
                    }
                    /* verify sorted by date-time */
                    x=test_array_sort((char *)data_array,0UL,u,sz,
                        timecmp,options,1U,f,log_arg);
                    if (x != u) {
                        (V)fprintf(stderr, "ERROR ^^^^^: %s (test_array_sort) ^^^^^ ERROR!!!!\n",fname);
                        errs++;
                        print_some_array(data_array,0UL,u,"/* "," */",options);
                    } else
                        (V)fprintf(stdout,
                           "%s stability test size %lu passed\n",
                           fname,(unsigned long)n);
                    if (0U!=errs) return -1;
                }
            }
        }
    }
    return ret;
}

int main(int argc, char *argv[]) /* XPG (see exec()) */
{
    char buf[4096], buf2[256], buf3[256];  /* RATS: ignore (big enough) */
    char prog[PATH_MAX];
    char host[MAXHOSTNAMELEN];  /* RATS: ignore (big enough) */
    char iplist[1024];          /* RATS: ignore (size is checked) */
    char logbuf[65536];         /* RATS: ignore (it's big enough) */
    char loghost[MAXHOSTNAMELEN] = "localhost";  /* RATS: ignore (big enough) */
    char procid[32];            /* RATS: ignore (used with sizeof()) */
    char *endptr=NULL;
    const char *comment="", *incr="+1", *pcc, *pcc2;
    unsigned char flags[256], var_count='n';
    short int *sharray=NULL;
    int bufsz, c, col=2, i, optind;
    int maxlen = 1024;
    int sockfd = -1;
    int cflags = REG_EXTENDED | REG_ICASE ;
    int eflags = 0;
    int *array=NULL;
    unsigned int seqid = 1U;
    unsigned int tzknown = 1U;
    unsigned int csequences=0U, errs=0U, func, functions=0U, last_adv,
        options=0U, p, tsequences=0U, sequence, tests, types=0U, u, v;
    long *larray=NULL, *refarray=NULL;
    VOL unsigned long count, count_limit=ULONG_MAX , n, startn=0UL, ul, z;
    size_t ratio=1UL, sz, q, w, x, y;
    size_t marray[12], dn;
    double d, timeout = TEST_TIMEOUT;
    float *sarray=NULL, *uarray=NULL, *warray=NULL;
    double *darray=NULL;
    uint_least64_t s[16];
    struct data_struct *data_array=NULL, **parray=NULL;
    struct logger_struct ls;
    struct sampling_table_struct *pst;
    struct repivot_table_struct *prt;
    void (*f)(int, void *, const char *, ...) = logger;
    void *log_arg;
    pid_t pid;
    regex_t re;
    regmatch_t match[3]; /* line_buf, name, value */
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "main";
#endif

    /* I/O initialization */
    (V) setvbuf(stdout, NULL, (int)_IONBF, BUFSIZ);
    (V) setvbuf(stderr, NULL, (int)_IONBF, BUFSIZ);
    if ((char)0==file_initialized) initialize_file(__FILE__);
    /* variable initialization */
    host[0] = '\0';
    iplist[0] = '\0';
    procid[0] = '\0';
    (V)path_basename(argv[0], prog, sizeof(prog));
    pid = getpid();
    /* initial logging configuration */
    memset(&ls, 0, sizeof(struct logger_struct));
    ls.options = LOG_PID /* | LOG_PERROR | LOG_CONS */ ;
    ls.logfac = LOG_DAEMON ;
    ls.logmask = LOG_UPTO(LOG_INFO) ;
    ls.location = LOGGER_LOCATION_STREAM | LOGGER_LOCATION_SYSLOG_HOST ;
#if 0
    ls.format = LOGGER_FORMAT_RFC5424_UTC ;
#else
    ls.format = LOGGER_FORMAT_PLAIN ;
#endif
    ls.ext_msg_start = LOGGER_MESSAGE_ALL ;
    ls.psysUpTime = NULL;
    ls.psyncAccuracy = NULL;
    ls.ptzKnown = &tzknown;
    ls.pisSynced = NULL;
    ls.stream = stderr;
    ls.sysloghost = loghost;
    ls.fqdn = host;      /* filled in below (uses logger for error reporting) */
    ls.ident = prog;
    ls.msgid = "median_test";
    ls.procid = procid;  /* filled in below (uses logger for error reporting) */
    ls.enterpriseId = NULL;
    ls.software = prog;
    ls.swVersion = "2.10";      /* maintained by version control system */
    ls.language = NULL;
    ls.pip = iplist;     /* filled in below (uses logger for error reporting) */
    ls.func = NULL;
    ls.pmax_msg_len = &maxlen;
    bufsz = sizeof(logbuf);
    ls.pbufsz = &bufsz;
    ls.buf = logbuf;
    ls.psockfd = &sockfd;
    ls.psequenceId = &seqid;
    log_arg = &ls;
    (V)get_host_name(host, sizeof(host), logger, log_arg);
    /* host address(es) */
    (V)get_ips(iplist,sizeof(iplist),host,AF_UNSPEC,AI_ADDRCONFIG,
        logger,log_arg);
    /* logging of snn OK from logger caller after initialization */
    (V)snul(procid,sizeof(procid),NULL,NULL,(unsigned long)pid,10,'0',1,
        logger,log_arg);
    /* process command-line arguments */
    /* initialize flags */
    for (q=0UL; q<sizeof(flags); q++) flags[q] = (unsigned char)0U;
    /* emulate getopt (too many implementation differences to rely on it) */
    for (optind = 1; (optind < argc) && (argv[optind][0] == '-'); optind++) {
        if (0 == strcmp(argv[optind], "--")) {
            optind++;
            break;
        }
        for (pcc = argv[optind] + 1; (int)'\0' != (c = (int)(*pcc)); pcc++) {
            if ((':' == (char)c) || (NULL == strchr(OPTSTRING, (char)c)))
                c = (int)'?';
            switch (c) {
#if SILENCE_WHINEY_COMPILERS
                /* this program has no options (that's what a compiler
                   diagnostic for the loop is about)
                */
                case ':': /* clang! STFU!! */
                    /* cannot happen here, but clang is unable to figure that
                       out
                    */
                break;
#endif
                case 'a' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_ILLUMOSQSORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* option(s) */
                    }
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            p=1U+strccount(pcc,'|'); /* # alternatives in arg */
                            for (v=0U; v<32UL; v++) {
                                tests=0x01U<<v;
                                if (0U==(tests&(MOD_OPTIONS))) continue;
                                pcc2 = modification_name(tests);
                                if (0!=strncmp(pcc2,"MOD_",4)) break;
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            options |= tests;
                                        }
                                    }
                                }
                            }
                            if (0x0U==(options&(MOD_OPTIONS))) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: no modified qsort option matches for \"%s\"",
                                    __func__, source_file, __LINE__, pcc);
                                fprintf(stderr, "modifcation options:\n");
                                for (v=0U; v<32U; v++) {
                                    tests=0x01U<<v;
                                    if (0U==(tests&(MOD_OPTIONS))) continue;
                                    pcc2 = modification_name(tests);
                                    if (0==strncmp(pcc2,"unknown ",8)) break;
                                    (V)snul(buf, sizeof(buf), "0x", NULL, tests, 16,
                                        '0', 3, logger, log_arg);
                                    (V)fprintf(stderr,"%s %s\n",buf,pcc2);
                                }
                                errs++;
                            } else {
                                q=bitcount(options);
                                if (q<p) {
                                    f(LOG_ERR, log_arg,
                                        "%s: %s line %d: %u alternatives in "
                                        "\"%s\", %u modified qsort option%s matched: 0x%x",
                                        __func__,source_file,__LINE__,p,pcc,q,
                                        q==1U?"":"s",options);
                                    fprintf(stderr, "modifcation options:\n");
                                    for (v=0U; v<32U; v++) {
                                        tests=0x01U<<v;
                                        if (0U==(tests&(MOD_OPTIONS))) continue;
                                        pcc2 = modification_name(tests);
                                        if (0==strncmp(pcc2,"unknown ",8)) break;
                                        (V)snul(buf, sizeof(buf), "0x", NULL, tests, 16,
                                            '0', 3, logger, log_arg);
                                        (V)fprintf(stderr,"%s %s\n",buf,pcc2);
                                    }
                                    errs++;
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        options |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'b' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            if (0==instrumented_bmqsort)
                                functions |= (0x01U<<FUNCTION_BMQSORT);
                            else
                                functions |= (0x01U<<FUNCTION_IBMQSORT);
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    instrumented_bmqsort = atoi(pcc);
                    if (0==instrumented_bmqsort)
                        functions |= (0x01U<<FUNCTION_BMQSORT);
                    else
                        functions |= (0x01U<<FUNCTION_IBMQSORT);
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'c' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_SQSORT);
                break;
                case 'C' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        tests=0x0U;
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            p=1U+strccount(pcc,'|'); /* # alternatives in arg */
                            for (v=0U; v<TEST_SEQUENCE_COUNT; v++) {
                                pcc2 = sequence_name(v);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__,source_file,__LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            /* assume adequate size (15UL) */
                                            if (0!=valid_test(
                                            (TEST_TYPE_CORRECTNESS),v,15UL)
                                            )
                                                tests|=0x01U<<v;
                                            else {
                                                (V)fprintf(stderr,"%s: %s correctness test is not valid\n",
                                                    prog,pcc2);
                                                errs++;
                                            }
                                        }
                                    }
                                }
                            }
                            csequences |= tests;
                            if (0x0U==csequences) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: no correctness test sequence matches for \"%s\"",
                                    __func__, source_file, __LINE__, pcc);
                                fprintf(stderr, "test sequences:\n");
                                for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                                    (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                                        '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                                    (V)fprintf(stderr,"%s %s\n",buf,sequence_name(tests));
                                }
                                errs++;
                            } else {
                                q=bitcount(csequences);
                                if (q<p) {
                                    f(LOG_ERR, log_arg,
                                        "%s: %s line %d: %u alternatives in "
                                        "\"%s\", %u sequence%s matched: 0x%x",
                                        __func__,source_file,__LINE__,p,pcc,q,
                                        q==1U?"":"s",csequences);
                                    fprintf(stderr, "test sequences:\n");
                                    for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                                            '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                                        (V)fprintf(stderr,"%s %s\n",buf,sequence_name(tests));
                                    }
                                    errs++;
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        csequences |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'd' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        tests=0x0U;
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            p=1U+strccount(pcc,'|'); /* # alternatives in arg */
                            for (v=0U; v<DEBUG_VALUE_COUNT; v++) {
                                tests=0x01U<<v;
                                pcc2 = debug_name(tests);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__,source_file,__LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            debug |= tests;
                                        }
                                    }
                                }
                            }
                            if (0x0U==debug) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: no debug matches for \"%s\"",
                                    __func__, source_file, __LINE__, pcc);
                                for (tests=0U; tests<DEBUG_VALUE_COUNT; tests++) {
                                    v=0x01U<<tests;
                                    (V)snul(buf, sizeof(buf), "0x", NULL, v, 16,
                                        '0', (DEBUG_VALUE_COUNT+3)/4+1, logger, log_arg);
                                    (V)fprintf(stderr,"%s %s\n",buf,debug_name(v));
                                }
                                errs++;
                            } else {
                                q=bitcount(debug);
                                if (q<p) {
                                    f(LOG_ERR, log_arg,
                                        "%s: %s line %d: %u alternatives in "
                                        "\"%s\", %u debug item%s matched: 0x%x",
                                        __func__,source_file,__LINE__,p,pcc,q,
                                        q==1U?"":"s",debug);
                                    (V)fprintf(stderr, "debug values:\n");
                                    for (tests=0U; tests<DEBUG_VALUE_COUNT; tests++) {
                                        v=0x01U<<tests;
                                        (V)snul(buf, sizeof(buf), "0x", NULL, v, 16,
                                            '0', (DEBUG_VALUE_COUNT+3)/4+1, logger, log_arg);
                                        (V)fprintf(stderr,"%s %s\n",buf,debug_name(v));
                                    }
                                    errs++;
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        debug |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                    if (DEBUGGING(SUPPORT_DEBUG))
                        ls.logmask = LOG_UPTO(LOG_DEBUG) ;
                break;
                case 'D' :
                    flags[c] = 1U;
                    functions |= (0x01U << FUNCTION_DPQSORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    dp_cutoff = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'e' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_HEAPSORT);
                break;
                case 'E' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_SELSORT);
                break;
                case 'f' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_SQRTSORT);
                break;
                case 'g' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_GLQSORT);
                break;
                case 'G' :
                    flags[c] = 1U;
                    use_shell++;
                break;
                case 'i' :
                    flags[c] = instrumented = 1U;
                break;
                case 'j' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_ISORT);
                break;
                case 'J' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_SHELLSORT);
                    endptr = pcc;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no gaps) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* gap value(s) */
                    }
                    for (x=1UL; x<ngaps; x++) {
                        y = strtoul(pcc, &endptr, 10);
                        if (2UL>y) x=0UL; /* in case user gives gap of 1 */
                        gaps[x]=y;
                        if ('\0' != *endptr) pcc=endptr+1;
                        else break;
                    }
                    pcc=endptr;
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'k' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    col = atoi(pcc);
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'K' : /*FALLTHROUGH*/
                    flags[c] = 1U;
                    comment="#";
                break;
                case 'l' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_QSORT);
                break;
                case 'm' :
                    flags[c] = 'm'; /* middle for default median(s) */
                    functions |= (0x01U<<FUNCTION_QSELECT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no options) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* option value(s) */
                    }
                    if (isalpha(*pcc)) { /* placement of order statistics */
                        flags[c] = tolower((unsigned char)(*pcc));
                    }
                    while (('\0'!=*pcc)&&(!(isdigit(*pcc)))) ++pcc;
                    selection_nk = (size_t)snlround(parse_expr(1.0,pcc,&endptr,10),
                        f,log_arg); /* number of order statistics */
                    if (1UL>selection_nk) selection_nk = 1UL;
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        no_aux_repivot=1;
                        repivot_factor = strtoul(pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        lopsided_partition_limit=(int)strtol(++pcc,&endptr,10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'M' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_MBMQSORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* option(s) */
                    }
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            p=1U+strccount(pcc,'|'); /* # alternatives in arg */
                            for (v=0U; v<32UL; v++) {
                                tests=0x01U<<v;
                                if (0U==(tests&(MOD_OPTIONS))) continue;
                                pcc2 = modification_name(tests);
                                if (0!=strncmp(pcc2,"MOD_",4)) break;
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            options |= tests;
                                        }
                                    }
                                }
                            }
                            if (0x0U==(options&(MOD_OPTIONS))) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: no modified qsort option matches for \"%s\"",
                                    __func__, source_file, __LINE__, pcc);
                                fprintf(stderr, "modifcation options:\n");
                                for (v=0U; v<32U; v++) {
                                    tests=0x01U<<v;
                                    if (0U==(tests&(MOD_OPTIONS))) continue;
                                    pcc2 = modification_name(tests);
                                    if (0==strncmp(pcc2,"unknown ",8)) break;
                                    (V)snul(buf, sizeof(buf), "0x", NULL, tests, 16,
                                        '0', 3, logger, log_arg);
                                    (V)fprintf(stderr,"%s %s\n",buf,pcc2);
                                }
                                errs++;
                            } else {
                                q=bitcount(options);
                                if (q<p) {
                                    f(LOG_ERR, log_arg,
                                        "%s: %s line %d: %u alternatives in "
                                        "\"%s\", %u modified qsort option%s matched: 0x%x",
                                        __func__,source_file,__LINE__,p,pcc,q,
                                        q==1U?"":"s",options);
                                    fprintf(stderr, "modifcation options:\n");
                                    for (v=0U; v<32U; v++) {
                                        tests=0x01U<<v;
                                        pcc2 = modification_name(tests);
                                        if (0==strncmp(pcc2,"unknown ",8)) break;
                                        (V)snul(buf, sizeof(buf), "0x", NULL, tests, 16,
                                            '0', 3, logger, log_arg);
                                        (V)fprintf(stderr,"%s %s\n",buf,pcc2);
                                    }
                                    errs++;
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        options |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'N' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_NBQSORT);
                break;
                case 'O' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = strtod(pcc, NULL);
                    if (0.0<d) {
                        double e, g;
                        for (x=1UL; x<(SAMPLING_TABLE_SIZE); x++) {
                            e=(double)(sorting_sampling_table[x].samples);
                            g=d*e*e-1.0;
                            if (g<(double)(SIZE_MAX))
                                sorting_sampling_table[x].max_nmemb=
                                    (unsigned long)snlround(g,f,log_arg);
                            else
                                sorting_sampling_table[x].max_nmemb
                                    =(SIZE_MAX);
                        }
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'p' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            for (i=0; i<3; i++) {
                                pcc2 = pivot_name(i);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            break;
                                        }
                                    }
                                }
                            }
                            method_pivot=i;
                        }
                        regfree(&re);
                    } else {
                        method_pivot = (int)strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    if ((QUICKSELECT_PIVOT_MEDIAN_OF_MEDIANS)==method_pivot) {
                        options &= ~(QUICKSELECT_STABLE);
                        options |= (QUICKSELECT_RESTRICT_RANK);
                    } else if ((QUICKSELECT_PIVOT_REMEDIAN_FULL)==method_pivot) {
                        options |= (QUICKSELECT_RESTRICT_RANK);
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'q' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_QSELECT_SORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot=1;
                        repivot_factor = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        repivot_cutoff = strtoul(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        lopsided_partition_limit=(int)strtol(++pcc,&endptr,10);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot = (int)strtol(++pcc, &endptr, 10);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'Q' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    timeout = parse_expr(1.0,pcc, &endptr, 10);
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'r' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_INTROSORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    introsort_final_insertion_sort = strtoul(pcc, &endptr, 10);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        introsort_small_array_cutoff=strtoul(++pcc,&endptr,10);
                        pcc=endptr;
                    }
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        introsort_recursion_factor=strtoul(++pcc,&endptr,10);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 't' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    if (isalpha(*pcc)) { /* sampling table specifier */
                        switch (tolower((unsigned char)(*pcc))) {
                            default : /*FALLTHROUGH*/
                            case 'q' : /* sorting 000 */
                                pst=mos_sorting_sampling_table;
                            break;
                            case 'm' : /* middle 010 */
                                pst=mos_middle_sampling_table;
                            break;
                            case 'e' : /* ends 100 001 */
                                pst=mos_ends_sampling_table;
                            break;
                        }
                        while (('\0' != *pcc)&&(!isdigit(*pcc))) pcc++;
                    } else {
                        pst=mos_sorting_sampling_table;
                    }
                    if ('\0' != *pcc) {
                        for (x=0UL; x<(SAMPLING_TABLE_SIZE); x++) {
                            y = (size_t)snlround(parse_expr(1.0,pcc, &endptr, 10),
                                f,log_arg);
                            if (NULL!=pst) pst[x].max_nmemb=y;
                            if ('\0' != *endptr) pcc=endptr+1;
                            else break;
                        }
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'T' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        tests=0x0U;
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            p=1U+strccount(pcc,'|'); /* # alternatives in arg */
                            for (v=0U; v<TEST_SEQUENCE_COUNT; v++) {
                                pcc2 = sequence_name(v);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            /* assume adequate size (15UL) */
                                            if (0!=valid_test(TEST_TYPE_TIMING,
                                            v, 15UL)
                                            )
                                                tests|=0x01U<<v;
                                            else {
                                                (V)fprintf(stderr,"%s: %s timing test is not valid\n",
                                                    prog,pcc2);
                                                errs++;
                                            }
                                        }
                                    }
                                }
                            }
                            tsequences |= tests;
                            if (0x0U==tsequences) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: no test sequence matches for \"%s\"",
                                    __func__, source_file, __LINE__, pcc);
                                fprintf(stderr, "test sequences:\n");
                                for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                                    (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                                        '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                                    (V)fprintf(stderr,"%s %s\n",buf,sequence_name(tests));
                                }
                                errs++;
                            } else {
                                q=bitcount(tsequences);
                                if (q<p) {
                                    f(LOG_ERR, log_arg,
                                        "%s: %s line %d: %u alternatives in "
                                        "\"%s\", %u sequence%s matched: 0x%x",
                                        __func__,source_file,__LINE__,p,pcc,q,
                                        q==1U?"":"s",tsequences);
                                    fprintf(stderr, "test sequences:\n");
                                    for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                                            '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                                        (V)fprintf(stderr,"%s %s\n",buf,sequence_name(tests));
                                    }
                                    errs++;
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        tsequences |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'u' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_QSELECT_S);
                break;
                case 'U' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = strtod(pcc, NULL);
                    if (0.0<d) {
                        double e, g;
                        for (x=1UL; x<(SAMPLING_TABLE_SIZE); x++) {
                            e=(double)(sorting_sampling_table[x].samples);
                            g=d*e*e-1.0;
                            if (g<(double)(SIZE_MAX))
                                sorting_sampling_table[x].max_nmemb=
                                    (unsigned long)snlround(g,f,log_arg);
                            else
                                sorting_sampling_table[x].max_nmemb
                                    =(SIZE_MAX);
                        }
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'v' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_NETWORKSORT);
                break;
                case 'V' :
                    flags[c] = 1U;
                    use_networks++;
                break;
                case 'w' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_QSORT_WRAPPER);
                break;
                case 'W' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_WQSORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    no_aux_repivot=1;
                    repivot_factor = (size_t)snlround(parse_expr(1.0,pcc, &endptr,
                        10),f,log_arg);
                    pcc=endptr;
                    if ('\0' != *pcc) {
                        repivot_cutoff = (size_t)snlround(parse_expr(1.0,++pcc,
                            &endptr, 10),f,log_arg);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        lopsided_partition_limit=(int)snlround(parse_expr(1.0,++pcc,
                            &endptr, 10),f,log_arg);
                        pcc=endptr;
                    }
                    if ('\0' != *pcc) {
                        no_aux_repivot = (int)snlround(parse_expr(1.0,++pcc,&endptr,
                            10),f,log_arg);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'x' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_LOGSORT);
                break;
                case 'X' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_SMOOTHSORT);
                break;
                case 'Y' :
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = parse_expr(1.0,pcc, &endptr, 10);
                    if (0.0<d) {
                        mos_middle_power=d;
                    }
                    pcc=endptr;
                    if (('\0'!=*pcc)&&('\0'!=*(++pcc))) {
                        d = parse_expr(1.0,pcc, &endptr, 10);
                        if (0.0<d) {
                            mos_ends_power=d;
                        }
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'y' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_YQSORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoff) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value */
                    }
                    y_cutoff = (size_t)snlround(parse_expr(1.0,pcc, &endptr, 10),
                        f,log_arg);
                    pcc=endptr;
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case 'Z' :
                    flags[c] = 1U;
                    options|=QUICKSELECT_NO_REPIVOT;
                break;
                case '3' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoffs) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    if (isalpha(*pcc)) { /* sampling table specifier */
                        switch (tolower((unsigned char)(*pcc))) {
                            case 'e' : /* end=right 001 */
                                pst=ends_sampling_table;
                            break;
                            case 'r' : /* extended 011 *//*FALLTHROUGH*/
                            case 'd' : /* distributed 111 *//*FALLTHROUGH*/
                            case 'l' : /* midleft 110 *//*FALLTHROUGH*/
                            case 'm' : /* middle 010 */
                                pst=middle_sampling_table;
                            break;
                            case 's' : /* separated 101 *//*FALLTHROUGH*/
                            case 'b' : /* beginning=left 100 */
                                pst=ends_sampling_table;
                            break;
                            default : /* sorting 000 */
                                pst=sorting_sampling_table;
                            break;
                        }
                        while (('\0' != *pcc)&&(!isdigit(*pcc))) pcc++;
                    } else {
                        pst=sorting_sampling_table;
                    }
                    if ('\0' != *pcc) {
                        for (x=0UL; x<(SAMPLING_TABLE_SIZE); x++) {
                            y = (size_t)snlround(parse_expr(1.0,pcc, &endptr, 10),
                                f,log_arg);
                            if (NULL!=pst) pst[x].max_nmemb=y;
                            if ('\0' != *endptr) pcc=endptr+1;
                            else break;
                        }
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '4' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no cutoff) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* cutoff value(s) */
                    }
                    quickselect_small_array_cutoff
                        = (size_t)snlround(parse_expr(1.0,pcc, &endptr, 10),
                        f,log_arg);
                    if (0UL==quickselect_small_array_cutoff)
                        (V)fprintf(stderr,"%s: %lu for quickselect_small_array_cutoff is ignored!\n",
                            prog,(unsigned long)quickselect_small_array_cutoff);
                    pcc=endptr;
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '5' :
                    flags[c] = save_partial = 1U;
                break;
                case '9' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_P9QSORT);
                break;
                case '@' :
                    flags[c] = 1U;
                    options|=QUICKSELECT_STRICT_SELECTION;
                break;
                case '#' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no threshold) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    /* number of order statistics */
                    sort_threshold = strtoul(pcc, &endptr, 10);
                    /* pass over arg to satisfy loop conditions */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '%' :
                    flags[c] = 1U;
                    reset_selection_factor2_count=0;
                break;
                case '+' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_MINMAXSORT);
                break;
                case ',' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_MERGESORT);
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no threshold) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    merge_cutoff = strtoul(pcc, &endptr, 10);
                    /* pass over arg to satisfy loop conditions */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '.' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_DEDSORT);
                break;
                case '|' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            for (i=0; i<3; i++) {
                                pcc2 = partition_name(i);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            break;
                                        }
                                    }
                                }
                            }
                            if (3<=i) {
                                (V)fprintf(stderr,"%s: %s line %d: no option match for \"%s\"\n",__func__,source_file,__LINE__,pcc);
                                errs++;
                            }
                            method_partition=i;
                        }
                        regfree(&re);
                    } else {
                        method_partition = (int)strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    if ((QUICKSELECT_PARTITION_STABLE)==method_partition) {
                        options |= (QUICKSELECT_STABLE);
                        network_mask &= 0x078U;
                        options &= (network_mask|0x07U);
                    } else {
                        options &= ~(QUICKSELECT_STABLE);
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '/' : /* for obsolete network mask */
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    network_mask = (unsigned int)strtoul(pcc, &endptr, 0);
                    options &= 0x07U;
                    if (0U!=(0x01F8U&network_mask)) {
                        /* cannot have stable sort with networks >= size 7 */
                        if (0U!=(options&(QUICKSELECT_STABLE))) {
                            (V)fprintf(stderr,
                               "%s: %s line %d: WARNING: network mask 0x%x overrides QUICKSELECT_STABLE\n",
                               __func__,source_file,__LINE__,
                               network_mask);
                            options &= ~(QUICKSELECT_STABLE);
                        }
                    }
                    if (0U!=(0x01FF0U&network_mask)) {
                        /* network sort size > 3 incompatible with optimized comparisons (in-place merge sort) */
                        if (0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS))) {
                            (V)fprintf(stderr,
                               "%s: %s line %d: WARNING: network mask 0x%x overrides QUICKSELECT_OPTIMIZE_COMPARISONS\n",
                               __func__,source_file,__LINE__,
                               network_mask);
                            options &= ~(QUICKSELECT_OPTIMIZE_COMPARISONS);
                        }
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '_' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_SYSMERGESORT);
                break;
                case '<' :
                    flags[c] = 1U;
                    options |= (QUICKSELECT_OPTIMIZE_COMPARISONS);
                break;
                case '>' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_RUNSORT);
                break;
                case '&' :
                    flags[c] = 1U;
                    options |= (QUICKSELECT_INDIRECT);
                    network_mask &= 0x01FF8U;
                break;
                case '*' :
                    flags[c] = 1U;
                    functions |= (0x01U<<FUNCTION_INDIRECT_MERGESORT);
                break;
                case '[' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no threshold) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    ratio = (size_t)snlround(parse_expr(1.0,pcc,&endptr,10),
                        f,log_arg); /* number of order statistics */
                    /* pass over arg to satisfy loop conditions */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '=' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    if ((isalpha(*pcc))||(ispunct(*pcc))||(isspace(*pcc))) {
                        tests=0x0U;
                        c = regcomp(&re, pcc, cflags);
                        if (0 != c) {
                            (V)regerror(c, &re, buf, sizeof(buf));
                            f(LOG_ERR, log_arg,
                                "%s: %s line %d: regcomp: %s",
                                __func__, source_file, __LINE__,
                                buf);
                        } else {
                            p=1U+strccount(pcc,'|'); /* # alternatives in arg */
                            for (v=0U; v<FUNCTION_COUNT; v++) {
                                pcc2 = function_name(v);
                                c = regexec(&re, pcc2, 1UL, match, eflags);
                                if (REG_NOMATCH == c) {
                                    f(LOG_DEBUG, log_arg,
                                        "%s: %s line %d: no match %s in %s",
                                        __func__,source_file,__LINE__,pcc,pcc2);
                                } else if (0 == c) {
                                    if (match[0].rm_so != -1) {
                                        n = (match[0].rm_eo - match[0].rm_so);
                                        if (0UL < n) {
                                            f(LOG_DEBUG, log_arg,
                                                "%s: %s line %d: match: \"%s\": offset %d through %d: \"%*.*s\"",
                                                __func__, source_file, __LINE__,
                                                pcc2,
                                                match[0].rm_so,match[0].rm_eo-1,
                                                n,n,pcc2+match[0].rm_so);
                                            tests|=0x01U<<v;
                                        }
                                        /* specific function special requirements */
                                        /* introsort requires 'r' flag to set default insertion sort cutoff */
                                        if (v==FUNCTION_INTROSORT) flags['r']=1U;
                                    }
                                }
                            }
                            functions |= tests;
                            if (0x0U==tests) {
                                f(LOG_ERR, log_arg,
                                    "%s: %s line %d: no test function matches for \"%s\"",
                                    __func__, source_file, __LINE__, pcc);
                                (V)fprintf(stderr, "test functions:\n");
                                for (tests=0U; tests<FUNCTION_COUNT; tests++) {
                                    (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                                        '0', (FUNCTION_COUNT+3)/4+1, logger, log_arg);
                                    (V)fprintf(stderr,"%s %s\n",buf,function_name(tests));
                                }
                                errs++;
                            } else {
                                q=bitcount(tests);
                                if (q<p) {
                                    f(LOG_ERR, log_arg,
                                        "%s: %s line %d: %u alternatives in "
                                        "\"%s\", %u function%s matched: 0x%x",
                                        __func__,source_file,__LINE__,p,pcc,q,
                                        q==1U?"":"s",tests);
                                    (V)fprintf(stderr, "test functions:\n");
                                    for (tests=0U; tests<FUNCTION_COUNT; tests++) {
                                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                                            '0', (FUNCTION_COUNT+3)/4+1, logger, log_arg);
                                        (V)fprintf(stderr,"%s %s\n",buf,function_name(tests));
                                    }
                                    errs++;
                                }
                            }
                        }
                        regfree(&re);
                    } else {
                        functions |= strtoul(pcc, &endptr, 0);
                        pcc=endptr;
                    }
                    /* pass over arg to satisfy loop conditions */
                    for (; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case ']' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ((argc<=optind+1)||('-'==argv[optind+1][0])) {
                            /* next arg (no threshold) */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* factors value(s) */
                    }
                    quickselect_cache_size = (size_t)snlround(
                        parse_expr(1.0,pcc,&endptr,10),f,log_arg);
                    /* pass over arg to satisfy loop conditions */
                    for (pcc=endptr; '\0' != *pcc; pcc++) ;
                    pcc--;
                break;
                case '?' :
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: unrecognized option %s",
                        prog, __func__, source_file, __LINE__, argv[optind]);
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: OPTSTRING is %s",
                        prog, __func__, source_file, __LINE__, OPTSTRING);
usage:
                    logger(LOG_ERR, log_arg,
                        "%s: usage: %s %s",
                        prog, prog, USAGE_STRING);
                    (V)fprintf(stderr, "debug values:\n");
                    for (tests=0U; tests<DEBUG_VALUE_COUNT; tests++) {
                        v=0x01U<<tests;
                        (V)snul(buf, sizeof(buf), "0x", NULL, v, 16,
                            '0', (DEBUG_VALUE_COUNT+3)/4+1, logger, log_arg);
                        (V)fprintf(stderr,"%s %s\n",buf,debug_name(v));
                    }
                    fprintf(stderr, "test sequences:\n");
                    for (tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                            '0', (TEST_SEQUENCE_COUNT+3)/4+1, logger, log_arg);
                        (V)fprintf(stderr,"%s %s\n",buf,sequence_name(tests));
                    }
                    (V)fprintf(stderr, "test functions:\n");
                    for (tests=0U; tests<FUNCTION_COUNT; tests++) {
                        (V)snul(buf, sizeof(buf), "0x", NULL, 0x01U<<tests, 16,
                            '0', (FUNCTION_COUNT+3)/4+1, logger, log_arg);
                        (V)fprintf(stderr,"%s %s\n",buf,function_name(tests));
                    }
                return 1;
                default : /* recognized option with no special processing */
                    flags[c] = 1U;
                break;
            }
        }
    }
    if (argc<2) goto usage;
    /* repivot parameter sanity */
    if (repivot_cutoff<repivot_factor+2UL) repivot_cutoff=repivot_factor+2UL;
    /* defaults */
    for (z=n=0U,ul=sizeof(flags); n<ul; n++) {
        if (0U!=flags[n]) {
            switch (n) {
                case 'r' :
                    /* introsort small-array sort cutoff */
                    if (0UL==introsort_small_array_cutoff) {
                        if (0U==introsort_final_insertion_sort)
                            introsort_small_array_cutoff=9UL;
                        else introsort_small_array_cutoff=9UL;
                    }
                break;
            }
        }
    }
    /* command line */
    (V)fprintf(stdout, "%s%s", comment, prog);
    for (i = 1; i < argc; i++) { (V)fprintf(stdout, " %s", argv[i]); }
    (V)fprintf(stdout, ":\n");
    /* test for, and warn about uninterpreted shell quotes */
    for (i = 1; i < argc; i++) {
        if ((0==strncmp(argv[i],"'-",2))
        || (0==strncmp(argv[i],"\"-",2))
        ) {
            (V)fprintf(stderr,"argv[%d] \"%s\" looks suspicious\n",i,argv[i]);
            errs++;
        }
    }
    /* print configured parameters in header */
    if (0U==flags['h']) {
        if (0U!=flags['H']) {
            (V)fprintf(stdout, "%s%s ips %s\n", comment, host, iplist);
        }
        (V)fprintf(stdout, "%s%s (%s %s %s) COMPILER_USED=\"%s\" __STDC_VERSION__=%ld\n", comment,
            HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED, MEDIAN_STDC_VERSION);
#if SILENCE_WHINEY_COMPILERS
        (V)fprintf(stdout, "%sSILENCE_WHINEY_COMPILERS = %d\n", comment,
            SILENCE_WHINEY_COMPILERS);
#endif
        /* print build option strings w/o id tags */
        pcc=strchr(file_build_options,')');
        if (NULL==pcc) pcc=file_build_options; else pcc++;
        pcc2=strchr(pcc,'\\');
        if (NULL==pcc2) x=strlen(pcc); else x=pcc2-pcc;
        (V)fprintf(stdout, "%s%.*s\n", comment, (int)x, pcc);
        ul=quickselect_options();
        decode_options(stdout,ul,"compiled option support: ","");
        /* now output settings made by options */
        for (q=z=n=0U,ul=sizeof(flags); n<ul; n++) {
            if (0U!=flags[n]) {
                switch (n) {
                    case 'b' :
                        (V)fprintf(stdout,
                            "%sinstrumented_bmqsort = %d\n", comment,
                            instrumented_bmqsort);
                    break;
                    case 'd' :
                        (V)fprintf(stdout,"%s debug 0x%x: ",comment,debug);
                        for (p=tests=0U; tests<DEBUG_VALUE_COUNT; tests++) {
                            v=0x01U<<tests;
                            if (DEBUGGING(v)) {
                                (V)snul(buf,sizeof(buf),0U==p?"0x":", 0x",
                                    NULL,v,16,'0',(DEBUG_VALUE_COUNT+3)/4+1,
                                    logger,log_arg);
                                (V)fprintf(stderr,"%s %s",buf,debug_name(v));
                                p++;
                            }
                        }
                        (V)fprintf(stdout, "\n");
                        p = 0U;
                    break;
                    case 'D' :
                        (V)fprintf(stdout,
                            "%sdp_cutoff = %lu\n", comment, dp_cutoff);
                    break;
                    case 'G' :
                        (V)fprintf(stdout,
                            "%suse_shell = %d\n", comment, use_shell);
                    break;
                    case 'J' :
                        for (x=0UL; x<ngaps; x++)
                            (V)fprintf(stdout,
                            "%sShell sort gap[%lu] %lu\n", comment, x, gaps[x]);
                    break;
                    case 'k' :
                        (V)fprintf(stdout,
                            "%stiming column = %d\n", comment, col);
                    break;
                    case '3' : /*FALLTHROUGH*/
                    case '4' : /*FALLTHROUGH*/
                    case '5' : /*FALLTHROUGH*/
                    case 'c' : /*FALLTHROUGH*/
                    case 'm' : /*FALLTHROUGH*/
                    case 'O' : /*FALLTHROUGH*/
                    case 'U' : /*FALLTHROUGH*/
                    case 'q' : /*FALLTHROUGH*/
                    case 't' :
                        if (0UL==z) {
                            (V)fprintf(stdout, "%ssave_partial = %u\n", comment,
                                save_partial);
                            (V)fprintf(stdout,
                                "%squickselect small-array sort cutoff = %lu, "
                                "no_aux_repivot = %d, repivot_factor = %lu, "
                                "repivot_cutoff = %lu, "
                                "lopsided_partition_limit = %d\n",
                                comment, quickselect_small_array_cutoff,
                                no_aux_repivot, repivot_factor, repivot_cutoff,
                                lopsided_partition_limit);
                            if ((DEBUGGING(SORT_SELECT_DEBUG))||(0U==flags['h'])
                            ) {

                                pst=sorting_sampling_table;
                                pcc=sampling_table_name(pst);
                                (V)fprintf(stdout,"%s sampling table:\n%s\n",
                                    pcc,
                                    "                  max_nmemb,     samples"
                                    );
                                for (i=0,w=0UL,x=1UL; (SAMPLING_TABLE_SIZE)>w;
                                w++,x*=3UL)
                                {
                                    y = pst[w].max_nmemb;
                                    if ((65535UL<y)&&(0UL<w)
                                    &&(65535UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,"/* 65535 */\n");
                                    if ((4294967295UL<y)&&(0UL<w)
                                    &&(4294967295UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,
                                            "/* 4294967295 */\n");
                                    if (((SIZE_MAX)==y)||(2UL>y)
                                    ||((0UL<w)&&(y<pst[w-1UL].max_nmemb))
                                    ) {
                                        pst[w].max_nmemb=(SIZE_MAX);
                                        strcpy(buf,
                                            "             (SIZE_MAX),");
                                    } else
                                        i = snul(buf,sizeof(buf),NULL,"UL,",
                                            y, 10, ' ', 21, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",x,
                                        10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,
                                        "   { %s %s }, /* %s */\n",
                                        buf,buf2,pcc);
                                    if ((SIZE_MAX)==y) continue;
                                }

                                pst=middle_sampling_table;
                                pcc=sampling_table_name(pst);
                                (V)fprintf(stdout,"%s sampling table:\n%s\n",
                                    pcc,
                                    "                  max_nmemb,     samples"
                                    );
                                for (i=0,w=0UL,x=1UL; (SAMPLING_TABLE_SIZE)>w;
                                w++,x*=3UL)
                                {
                                    y = pst[w].max_nmemb;
                                    if ((65535UL<y)&&(0UL<w)
                                    &&(65535UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,"/* 65535 */\n");
                                    if ((4294967295UL<y)&&(0UL<w)
                                    &&(4294967295UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,
                                            "/* 4294967295 */\n");
                                    if (((SIZE_MAX)==y)||(2UL>y)
                                    ||((0UL<w)&&(y<pst[w-1UL].max_nmemb))
                                    ) {
                                        pst[w].max_nmemb=(SIZE_MAX);
                                        strcpy(buf,
                                            "             (SIZE_MAX),");
                                    } else
                                        i = snul(buf,sizeof(buf),NULL,"UL,",
                                            y, 10, ' ', 21, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",x,
                                        10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,
                                        "   { %s %s }, /* %s */\n",
                                        buf,buf2,pcc);
                                    if ((SIZE_MAX)==y) continue;
                                }

                                pst=ends_sampling_table;
                                pcc=sampling_table_name(pst);
                                (V)fprintf(stdout,"%s sampling table:\n%s\n",
                                    pcc,
                                    "                  max_nmemb,     samples"
                                    );
                                for (i=0,w=0UL,x=1UL; (SAMPLING_TABLE_SIZE)>w;
                                w++,x*=3UL)
                                {
                                    y = pst[w].max_nmemb;
                                    if ((65535UL<y)&&(0UL<w)
                                    &&(65535UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,"/* 65535 */\n");
                                    if ((4294967295UL<y)&&(0UL<w)
                                    &&(4294967295UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,
                                            "/* 4294967295 */\n");
                                    if (((SIZE_MAX)==y)||(2UL>y)
                                    ||((0UL<w)&&(y<pst[w-1UL].max_nmemb))
                                    ) {
                                        pst[w].max_nmemb=(SIZE_MAX);
                                        strcpy(buf,
                                            "             (SIZE_MAX),");
                                    } else
                                        i = snul(buf,sizeof(buf),NULL,"UL,",
                                            y, 10, ' ', 21, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",x,
                                        10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,
                                        "   { %s %s }, /* %s */\n",
                                        buf,buf2,pcc);
                                    if ((SIZE_MAX)==y) continue;
                                }

                                pst=mos_sorting_sampling_table;
                                (V)fprintf(stdout,"%s sampling table:\n%s\n",
                                    sampling_table_name(pst),
                                    "                  max_nmemb,     samples"
                                    );
                                for (i=0,w=0UL,x=1UL; (SAMPLING_TABLE_SIZE)>w;
                                w++,x+=2UL)
                                {
                                    y = pst[w].max_nmemb;
                                    if ((65535UL<y)&&(0UL<w)
                                    &&(65535UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,"/* 65535 */\n");
                                    if ((4294967295UL<y)&&(0UL<w)
                                    &&(4294967295UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,
                                            "/* 4294967295 */\n");
                                    if (((SIZE_MAX)==y)||(2UL>y)) {
                                        pst[w].max_nmemb=(SIZE_MAX);
                                        strcpy(buf,
                                            "             (SIZE_MAX),");
                                    } else
                                        i = snul(buf,sizeof(buf),NULL,"UL,",
                                            y, 10, ' ', 21, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",x,
                                        10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,
                                        "   { %s %s }, /* %s */\n",
                                        buf,buf2,sampling_table_name(pst));
                                    if ((SIZE_MAX)==y) continue;
                                }

                                pst=mos_middle_sampling_table;
                                (V)fprintf(stdout,"%s sampling table:\n%s\n",
                                    sampling_table_name(pst),
                                    "                  max_nmemb,     samples"
                                    );
                                for (i=0,w=0UL,x=1UL; (SAMPLING_TABLE_SIZE)>w;
                                w++,x+=2UL)
                                {
                                    y = pst[w].max_nmemb;
                                    if ((65535UL<y)&&(0UL<w)
                                    &&(65535UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,"/* 65535 */\n");
                                    if ((4294967295UL<y)&&(0UL<w)
                                    &&(4294967295UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,
                                            "/* 4294967295 */\n");
                                    if (((SIZE_MAX)==y)||(2UL>y)) {
                                        pst[w].max_nmemb=(SIZE_MAX);
                                        strcpy(buf,
                                            "             (SIZE_MAX),");
                                    } else
                                        i = snul(buf,sizeof(buf),NULL,"UL,",
                                            y, 10, ' ', 21, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",x,
                                        10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,
                                        "   { %s %s }, /* %s */\n",
                                        buf,buf2,sampling_table_name(pst));
                                    if ((SIZE_MAX)==y) continue;
                                }

                                pst=mos_ends_sampling_table;
                                (V)fprintf(stdout,"%s sampling table:\n%s\n",
                                    sampling_table_name(pst),
                                    "                  max_nmemb,     samples"
                                    );
                                for (i=0,w=0UL,x=1UL; (SAMPLING_TABLE_SIZE)>w;
                                w++,x+=2UL)
                                {
                                    y = pst[w].max_nmemb;
                                    if ((65535UL<y)&&(0UL<w)
                                    &&(65535UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,"/* 65535 */\n");
                                    if ((4294967295UL<y)&&(0UL<w)
                                    &&(4294967295UL>=pst[w-1UL].max_nmemb))
                                        (V)fprintf(stdout,
                                            "/* 4294967295 */\n");
                                    if (((SIZE_MAX)==y)||(2UL>y)) {
                                        pst[w].max_nmemb=(SIZE_MAX);
                                        strcpy(buf,
                                            "             (SIZE_MAX),");
                                    } else
                                        i = snul(buf,sizeof(buf),NULL,"UL,",
                                            y, 10, ' ', 21, f, log_arg);
                                    i = snul(buf2,sizeof(buf2),NULL,"UL",x,
                                        10, ' ', 11, f, log_arg);
                                    (V)fprintf(stdout,
                                        "   { %s %s }, /* %s */\n",
                                        buf,buf2,sampling_table_name(pst));
                                    if ((SIZE_MAX)==y) continue;
                                }

                                prt=ros_sorting_repivot_table;
                                pcc="remedian of samples sorting";
                                (V)fprintf(stdout,"%s repivot table:\n%s\n",pcc,
                                    "    min_samples, factor1, factor2"
                                    );
                                for (i=0,x=0UL; ; ++i,x=prt[i].min_samples)
                                {
                                    unsigned char factor1, factor2;
                                    x = prt[i].min_samples;
                                    factor1=prt[i].factor1;
                                    factor2=prt[i].factor2;
                                    if ((SIZE_MAX)==x) {
                                        strcpy(buf,
                                            "(SIZE_MAX),");
                                    } else
                                        (V)snul(buf,sizeof(buf),NULL,"UL,",
                                            x, 10, ' ', 8, f, log_arg);
                                    (V)snul(buf2,sizeof(buf2),NULL,"U,",
                                        (unsigned long)factor1, 10, ' ', 3,
                                        f, log_arg);
                                    (V)snul(buf3,sizeof(buf3),NULL,"U",
                                        (unsigned long)factor2, 10, ' ', 3,
                                        f, log_arg);
                                    (V)fprintf(stdout,
                                        "    { %s %s %s }, /* %s */\n",
                                        buf,buf2,buf3,
                                        pcc);
                                    if ((SIZE_MAX)==x) break;
                                }

                                prt=ros_selection_repivot_table;
                                pcc="remedian of samples selection";
                                (V)fprintf(stdout,"%s repivot table:\n%s\n",pcc,
                                    "    min_samples, factor1, factor2"
                                    );
                                for (i=0,x=0UL; ; ++i,x=prt[i].min_samples)
                                {
                                    unsigned char factor1, factor2;
                                    x = prt[i].min_samples;
                                    factor1=prt[i].factor1;
                                    factor2=prt[i].factor2;
                                    if ((SIZE_MAX)==x) {
                                        strcpy(buf,
                                            "(SIZE_MAX),");
                                    } else
                                        (V)snul(buf,sizeof(buf),NULL,"UL,",
                                            x, 10, ' ', 8, f, log_arg);
                                    (V)snul(buf2,sizeof(buf2),NULL,"U,",
                                        (unsigned long)factor1, 10, ' ', 3,
                                        f, log_arg);
                                    (V)snul(buf3,sizeof(buf3),NULL,"U",
                                        (unsigned long)factor2, 10, ' ', 3,
                                        f, log_arg);
                                    (V)fprintf(stdout,
                                        "    { %s %s %s }, /* %s */\n",
                                        buf,buf2,buf3,
                                        pcc);
                                    if ((SIZE_MAX)==x) break;
                                }

                                prt=mos_sorting_repivot_table;
                                pcc="median of samples sorting";
                                (V)fprintf(stdout,"%s repivot table:\n%s\n",pcc,
                                    "    min_samples, factor1, factor2"
                                    );
                                for (i=0,x=0UL; ; ++i,x=prt[i].min_samples)
                                {
                                    unsigned char factor1, factor2;
                                    x = prt[i].min_samples;
                                    factor1=prt[i].factor1;
                                    factor2=prt[i].factor2;
                                    if ((SIZE_MAX)==x) {
                                        strcpy(buf,
                                            "(SIZE_MAX),");
                                    } else
                                        (V)snul(buf,sizeof(buf),NULL,"UL,",
                                            x, 10, ' ', 8, f, log_arg);
                                    (V)snul(buf2,sizeof(buf2),NULL,"U,",
                                        (unsigned long)factor1, 10, ' ', 3,
                                        f, log_arg);
                                    (V)snul(buf3,sizeof(buf3),NULL,"U",
                                        (unsigned long)factor2, 10, ' ', 3,
                                        f, log_arg);
                                    (V)fprintf(stdout,
                                        "    { %s %s %s }, /* %s */\n",
                                        buf,buf2,buf3,
                                        pcc);
                                    if ((SIZE_MAX)==x) break;
                                }

                                prt=mos_selection_repivot_table;
                                pcc="median of samples selection";
                                (V)fprintf(stdout,"%s repivot table:\n%s\n",pcc,
                                    "    min_samples, factor1, factor2"
                                    );
                                for (i=0,x=0UL; ; ++i,x=prt[i].min_samples)
                                {
                                    unsigned char factor1, factor2;
                                    x = prt[i].min_samples;
                                    factor1=prt[i].factor1;
                                    factor2=prt[i].factor2;
                                    if ((SIZE_MAX)==x) {
                                        strcpy(buf,
                                            "(SIZE_MAX),");
                                    } else
                                        (V)snul(buf,sizeof(buf),NULL,"UL,",
                                            x, 10, ' ', 8, f, log_arg);
                                    (V)snul(buf2,sizeof(buf2),NULL,"U,",
                                        (unsigned long)factor1, 10, ' ', 3,
                                        f, log_arg);
                                    (V)snul(buf3,sizeof(buf3),NULL,"U",
                                        (unsigned long)factor2, 10, ' ', 3,
                                        f, log_arg);
                                    (V)fprintf(stdout,
                                        "    { %s %s %s }, /* %s */\n",
                                        buf,buf2,buf3,
                                        pcc);
                                    if ((SIZE_MAX)==x) break;
                                }

                                if (0U!=flags['m']) {
                                    (V)fprintf(stdout, "%lu order statistic%s, %c\n",
                                        selection_nk, selection_nk==1UL?"":"s",
                                        flags['m']);
                                }
                            }
                            if (0UL==z) z++;
                        }
                    break;
                    case 'a' : /*FALLTHROUGH*/
                    case 'M' : /*FALLTHROUGH*/
                    case 'p' : /*FALLTHROUGH*/
                    case 'Z' : /*FALLTHROUGH*/
                    case '|' : /*FALLTHROUGH*/
                    case '/' : /*FALLTHROUGH*/
                    case '&' : /*FALLTHROUGH*/
                    case '<' : /*FALLTHROUGH*/
                    case '@' :
                        if (0U==q) {
                            decode_options(stdout,options,comment,"");
                            if (0U!=(options&(MOD_OPTIONS))) {
                                (V)fprintf(stdout, "%soptions =", comment);
                                for (x=0UL; x<32; x++) {
                                    w=(0x01<<x);
                                    if (0U==(w&(MOD_OPTIONS))) continue;
                                    if (0U!=(options&w)) {
                                        (V)fprintf(stdout, " %s",
                                            modification_name(w));
                                    }
                                }
                                (V)fprintf(stdout, "\n");
                            }
                            q++;
                        }
                    break;
                    case 'Q' :
                        (V)fprintf(stdout, "%stest timeout = %0.2f\n", comment,
                            timeout);
                    break;
                    case 'r' :
                        (V)fprintf(stdout,
                            "%sintrosort_final_insertion_sort = %u, introsort small-array sort cutoff = %lu, recursion factor = %lu\n",
                            comment, introsort_final_insertion_sort,
                            introsort_small_array_cutoff,
                            introsort_recursion_factor);
                    break;
                    case 'V' :
                        (V)fprintf(stdout, "%suse_networks = %d\n", comment,
                            use_networks);
                    break;
                    case 'W' :
                        (V)fprintf(stdout,
                            "%slopsided quickselect no_aux_repivot = %d, repivot_factor = %lu, repivot_cutoff = %lu, lopsided_partition_limit = %d\n",
                            comment, no_aux_repivot, repivot_factor,
                            repivot_cutoff, lopsided_partition_limit);
                    break;
                    case 'y' :
                        (V)fprintf(stdout,
                            "%sYaroslavskiy's dual-pivot small-array sort cutoff = %lu\n",
                            comment, y_cutoff);
                    break;
                    case '!' :
                        do_histogram=1U;
                        (V)fprintf(stdout, "%sdo_histogram = %u\n", comment,
                            do_histogram);
                    break;
                    case '#' :
                        (V)fprintf(stdout, "%ssort_threshold = %lu\n", comment,
                            sort_threshold);
                    break;
                    case '%' :
                        (V)fprintf(stdout,
                            "%sreset_selection_factor2_count = %d\n", comment,
                            reset_selection_factor2_count);
                    break;
                    case ',' :
                        (V)fprintf(stdout, "%smerge_cutoff = %lu\n", comment,
                            merge_cutoff);
                    break;
                    case '[' :
                        (V)fprintf(stdout, "%ssize ratio multiplier = %lu\n",
                            comment, ratio);
                    break;
                    case ']' :
                        (V)fprintf(stdout, "%squickselect_cache_size = %lu\n",
                            comment,(unsigned long)quickselect_cache_size);
                    break;
                }
            }
        }
    }

    if (0U < flags['s']) { /* print type sizes requested */
        print_sizes(comment,prog);
    }

    if (0U < flags['o']) { /* operation timing tests requested */
        op_tests(comment, prog, logger, log_arg); /* 64-bit machines */
    }

#if DEBUG_CODE
if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
         "%s: %s line %d: argc=%d, optind=%d\n",__func__,source_file,__LINE__,
         argc,optind);
#endif
    if (argc > optind+2) {
        startn =
            (size_t)snlround(parse_expr(1.0,argv[optind++],&endptr,10),f,log_arg);
        if (2UL > startn) startn = 2UL;
        incr = argv[optind++];
        if (0U==flags['h'])
            (V)fprintf(stdout,
                "%sstartn = %lu, incr = %s\n",
                comment, startn, incr);
    }

    if (optind >= argc) {
        ul = 0UL;
    } else {
        ul = (size_t)snlround(parse_expr(1.0,argv[optind++], &endptr, 10),f,log_arg);
        if (0U==flags['h']) (V)fprintf(stdout, "%sul = %lu\n", comment, ul);
    }
    if (1UL > ul) ul = 10000UL;
    if (2UL > ul) ul = 2UL;
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stdout,
            "%s line %d: ul = %lu, startn = %lu, incr = %s\n",
             __func__, __LINE__, ul, startn, incr);
#endif
    if ((ul<startn)||(0UL==startn)) startn = ul;
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stdout,
            "%s line %d: startn = %lu, incr = %s\n",
             __func__, __LINE__, startn, incr);
#endif

    if (optind >= argc) {
        count = 0UL;
    } else {
        /* parse count and counting options */
        count=(unsigned long)snlround(parse_expr(1.0,argv[optind++],&endptr,10),f,
            log_arg);
        while ('\0'!=*(pcc=endptr)) {
            switch (*pcc) {
                case 'r' : /*FALLTHROUGH*/
                case 'R' : /*FALLTHROUGH*/
                case 's' : /*FALLTHROUGH*/
                case 'S' :
                    var_count = *pcc;
                    endptr= ++pcc;
                break;
                case '<' :
                      count_limit=(unsigned long)snlround(parse_expr(1.0,++pcc,
                          &endptr,10),f,log_arg);
                break;
                default:
                    (V)fprintf(stderr,
                        "%s: %s line %d: unexpected character '%c' "
                        "in \"%s\" from \"%s\"\n",
                        __func__,source_file,__LINE__,*pcc,pcc,argv[optind-1]);
                    endptr="";
                    errs++;
                break;
            }
        }
        if (0U==flags['h'])
            (V)fprintf(stdout,
                "%scount = %lu, var_count = %c, count_limit=%lu\n",
                comment, count, var_count, count_limit);
    }
    if (0U<errs) { return errs; }
    if (1UL > count)
        count = 10UL;

#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stdout,
            "%s line %d: startn = %lu, incr = %s\n",
            __func__, __LINE__, startn, incr);
    if (DEBUGGING(SORT_SELECT_DEBUG))
        if ((0U==flags['h'])&&(NULL!=f))
            f(LOG_INFO, log_arg, "%s%s: args processed", comment, prog);
#endif

    q=1UL;
    if (0U==flags['h'])
        (V)fprintf(stdout,
            "%s%s: %s line %d: startn = %lu, incr = %s,"
            " ul = %lu, count = %lu, var_count = %c, count_limit=%lu, ul*count "
            "= %lu, q = %lu\n",
            comment, __func__, source_file, __LINE__, startn, incr,
            ul, count, var_count, count_limit, ul*count, q);

    /* data types */
    /* Test all data types if none specified. */
    if (
        (0U==flags['A'])
      &&(0U==flags['F'])
      &&(0U==flags['I'])
      &&(0U==flags['L'])
      &&(0U==flags['P'])
      &&(0U==flags['S'])
      &&(0U==flags['$'])
    )
        flags['A']=
        flags['F']=
        flags['I']=
        flags['L']=
        flags['P']=
        flags['S']=
        flags['$']=
        1U;
/* XXX could use some option flag and regex with names to select types... */
    if (0U!=flags['$']) types|= (0x01U << DATA_TYPE_SHORT );
    if (0U!=flags['I']) types|= (0x01U << DATA_TYPE_INT );
    if (0U!=flags['L']) types|= (0x01U << DATA_TYPE_LONG );
    if (0U!=flags['F']) types|= (0x01U << DATA_TYPE_DOUBLE );
    if (0U!=flags['S']) types|= (0x01U << DATA_TYPE_STRUCT );
    if (0U!=flags['A']) types|= (0x01U << DATA_TYPE_STRING );
    if (0U!=flags['P']) types|= (0x01U << DATA_TYPE_POINTER );

    /* test types */
    /* Test correctness and timing if neither specified. */
    if ((0U==flags['C']) && (0U==flags['T'])) {
        flags['C']=flags['T']=1U;
        /* initialize default sequences if none specified */
        for (v=1U; v<TEST_SEQUENCE_COUNT; v++) {
            tests=0x01U<<v;
            /* default is all valid test sequences except stdin */
            /* assume adequate size */
            if (0!=valid_test(TEST_TYPE_CORRECTNESS, v, startn))
                csequences |= tests;
            if (0!=valid_test(TEST_TYPE_TIMING, v, startn))
                tsequences |= tests;
        }
    } else if ((0U==flags['C'])&&(0U!=flags['T'])) csequences=0U;
    else if ((0U!=flags['C'])&&(0U==flags['T'])) tsequences=0U;

    if (0U==flags['h']) {
        if ((0U!=csequences)||(0U!=tsequences)) {
            (V)fprintf(stdout, "%stest sequences:\n", comment);
            if ((0U!=flags['C'])&&(0U!=csequences)) {
                (V)fprintf(stdout, "%s correctness 0x%x: ", comment, csequences);
                for (p=0U,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                    if (0U != (csequences & (0x01U << tests))) {
                        (V)snul(buf, sizeof(buf), 0U==p?"0x":", 0x", NULL,
                            0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4,
                            logger, log_arg);
                        (V)fprintf(stdout,"%s %s",buf,sequence_name(tests));
                        p++;
                    }
                }
                (V)fprintf(stdout, "\n");
            }
            if ((0U!=flags['T'])&&(0U!=tsequences)) {
                (V)fprintf(stdout, "%s timing 0x%x: ", comment, tsequences);
                for (p=0U,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                    if (0U != (tsequences & (0x01U << tests))) {
                        (V)snul(buf, sizeof(buf), 0U==p?"0x":", 0x", NULL,
                            0x01U<<tests, 16, '0', (TEST_SEQUENCE_COUNT+3)/4,
                            logger, log_arg);
                        (V)fprintf(stdout,"%s %s",buf,sequence_name(tests));
                        p++;
                    }
                }
                (V)fprintf(stdout, "\n");
            }
        }
        if (0U!=functions) {
            (V)fprintf(stdout, "%stest functions:\n", comment);
            for (p=0U,tests=0U; tests<FUNCTION_COUNT; tests++) {
                if (0U != (functions & (0x01U << tests))) {
                    (V)snul(buf, sizeof(buf), 0U==p?"0x":", 0x", NULL,
                        0x01U<<tests, 16, '0', (FUNCTION_COUNT+3)/4,
                        logger, log_arg);
                    (V)fprintf(stdout,"%s %s",buf,function_name(tests));
                    p++;
                }
            }
            (V)fprintf(stdout, "\n");
        }
    }

    /* initialize (seed) random number generator and save state */
    if (0U != flags['z']) {
        unsigned int z2;

        p = 0U;
        for (z2=0U; z2<16U; z2++)
            s[z2]=z2+1U;
        i = seed_random64n(s, p);
        if (0 != i) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: seed_random64n(0x%lx, %u) returned %d: %m",
                prog, __func__, source_file, __LINE__,
                (unsigned long)s, p, i
            );
            errs++;
        }
    }
    if (0U<errs) { return errs; }

    /* allocate test arrays */
    /* Size should be large enough for 1 set of arrays at largest count, except
       when testing with permutations or combinations.
    */
    q = sizeof(long); /* refarray */
    if (0UL!=flags['I']) q+=sizeof(int);
    if ((0U!=flags['A'])||(0U!=flags['S'])||(0U!=flags['P']))
        q+=sizeof(struct data_struct);
    if (0UL!=flags['P']) q+=sizeof(struct data_struct *);
    if (0UL!=flags['F']) q+=sizeof(double);
    if (0UL!=flags['L']) q+=sizeof(long);
    if (0UL!=flags['$']) q+=sizeof(short);
    sz=ul;
    if (0U!=flags['[']) {
        sz *= ratio;
        sz++; /* offset to force non-alignment to larger size */
    }
    if (SIZE_MAX/q <= sz) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %s %lu",
            prog, __func__, source_file, __LINE__,
            "malloc will certainly fail; try again with size <",
            ul);
        return ++errs;
    }

    if (0U < flags['n']) /* do-nothing flag */
        return 0;

    /* allocate data arrays */
    /* refarray is the basis for all types */
    refarray=malloc(sizeof(long)*ul);
    if (NULL==refarray) errs++;
    if (0U<errs) {
        logger(LOG_ERR, log_arg,
            "%s: %s: %s line %d: %m", prog, __func__, source_file, __LINE__);
        goto done;
    }
#if DEBUG_CODE
    if (DEBUGGING(MEMORY_DEBUG))
        (V)fprintf(stderr, "/* %s: %s line %d: refarray at %p through %p */\n",
            __func__,source_file,__LINE__,(void *)refarray,
            ((char *)refarray)+sizeof(long)*ul);
#endif /* DEBUG_CODE */
    global_sz=sz;
    if (0UL!=flags['I']) {
        array=malloc(sizeof(int)*sz);
        if (NULL==array) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m",prog,__func__,source_file,__LINE__);
            return errs;
        }
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: array at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)array, ((char *)array)+sizeof(int)*sz);
#endif /* DEBUG_CODE */
        if (0U!=flags['[']) array++;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: array at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)array, ((char *)array)+sizeof(int)*sz);
#endif /* DEBUG_CODE */
    }
    v = (0x01U << TEST_SEQUENCE_STDIN);
    if ((0U!=(csequences&v))||(0U!=(tsequences&v))||(0U!=flags['A'])
    ||(0U!=flags['S'])||(0U!=flags['P'])
    ) {
        data_array=malloc(sizeof(struct data_struct)*sz);
        if (NULL==data_array) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m",prog,__func__,source_file,__LINE__);
            goto done;
        }
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: data_array at %p through %p */\n",
                __func__,source_file,__LINE__,(void *)data_array,
                ((char *)data_array)+sizeof(struct data_struct)*sz);
#endif /* DEBUG_CODE */
            /* offset for ratio alignment is deferred */
        if (0UL!=flags['P']) {
            parray=malloc(sizeof(struct data_struct *)*sz);
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: parray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)parray,((char *)parray)+sizeof(struct data_struct *)*sz);
#endif /* DEBUG_CODE */
            if (NULL==parray) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m",
                    prog,__func__,source_file,__LINE__);
                if (NULL!=data_array) {
                    /* not yet adjusted for ratio alignment */
                    free(data_array); data_array=NULL;
                }
                goto done;
            }
            /* pointers in parray point to structures in data_array */
            (void)set_array_pointers((char **)parray,sz,(char *)data_array,
                sizeof(struct data_struct),0UL,sz);
            if (0U!=flags['[']) parray++;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: parray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)parray,((char *)parray)+sizeof(struct data_struct *)*sz);
#endif /* DEBUG_CODE */
        }
        /* adjust data_array for ratio alignment after pointer assignment */
        if (0U!=flags['[']) data_array++;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: data_array at %p through %p */\n",
                __func__,source_file,__LINE__,(void *)data_array,
                ((char *)data_array)+sizeof(struct data_struct)*sz);
#endif /* DEBUG_CODE */
    }
    if (0UL!=flags['F']) {
        darray=malloc(sizeof(double)*sz);
        if (NULL==darray) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m",prog,__func__,source_file,__LINE__);
            goto done;
        }
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: darray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)darray, ((char *)darray)+sizeof(double)*sz);
#endif /* DEBUG_CODE */
        if (0U!=flags['[']) darray++;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: darray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)darray, ((char *)darray)+sizeof(double)*sz);
#endif /* DEBUG_CODE */
    }
    if ((0U!=flags['L'])||(0U!=flags[';'])) {
        larray=malloc(sizeof(long)*sz);
        if (NULL==larray) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m",prog,__func__,source_file,__LINE__);
            goto done;
        }
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: larray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)larray, ((char *)larray)+sizeof(long)*sz);
#endif /* DEBUG_CODE */
        if (0U!=flags['[']) larray++;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: larray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)larray, ((char *)larray)+sizeof(long)*sz);
#endif /* DEBUG_CODE */
    }
    if (0U!=flags['$']) {
        sharray=malloc(sizeof(short)*sz);
        if (NULL==sharray) errs++;
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m",prog,__func__,source_file,__LINE__);
            goto done;
        }
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: sharray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)sharray, ((char *)sharray)+sizeof(short)*sz);
#endif /* DEBUG_CODE */
        if (0U!=flags['[']) sharray++;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: sharray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)sharray, ((char *)sharray)+sizeof(short)*sz);
#endif /* DEBUG_CODE */
    }
    global_refarray = refarray, global_larray = larray, global_parray = parray,
        global_darray = darray, global_iarray = array,
        global_data_array = data_array, global_sharray = sharray;

#if DEBUG_CODE
    if (DEBUGGING(MEMORY_DEBUG))
        if ((0U==flags['h'])&&(NULL!=f))
            f(LOG_INFO, log_arg,
                "%s%s %s: %s line %d: arrays allocated, sz=%lu",
                comment, prog, __func__, source_file, __LINE__, sz);
#endif

    /* read data from stdin if requested */
    v = (0x01U << TEST_SEQUENCE_STDIN);
    if ((0U!=(csequences&v))||(0U!=(tsequences&v))) {
        f(LOG_INFO, log_arg,
            "%s%s %s: %s line %d: reading data sequence from stdin",
            comment, prog, __func__, source_file, __LINE__);
        for (i=0; NULL != fgets(buf,sizeof(buf),stdin); i++) {
            /* elide trailing newline */
            endptr=strchr(buf,'\n');
            if (NULL != endptr) *endptr='\0';
            else { /* line too long */ --i; continue; }
            /* skip leading whitespace (comment detection; not strtol/strtod) */
            for (endptr=buf; (*endptr!='\0')&&(0!=isspace(*endptr)); endptr++) ;
            /* skip comments, empty lines */
            switch (*endptr) {
                case '/' : /* C-type and C ploose ploose comments */
                /*FALLTHROUGH*/
                case '#' : /* shell-style comments */
                /*FALLTHROUGH*/
                case ';' : /* named.conf-style comments */
                    (V)fprintf(stderr,
                        "%s: %s line %d: i=%d: ignoring comment line %s\n",
                        __func__,source_file,__LINE__,i,buf);
                /*FALLTHROUGH*/
                case '\0' : /* empty line */
                    --i; /* don't count this input line */
                continue; /* next line */
            }
            /* don't try to write past end of arrays! */
            if (i+0UL>=sz) break;
            if ((0==i)&&(0U!=flags['d']))
                (V)fprintf(stderr,
                    "%s: %s line %d: first non-comment input line %s\n",
                    __func__,source_file,__LINE__,buf);
            /* interpret string as long integer, save to refarray */
            refarray[i] = strtol(endptr,NULL,10);
#if DEBUG_CODE
            if (DEBUGGING(SORT_SELECT_DEBUG))
                if (0==i)
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: string \"%s\" -> %ld */\n",
                        __func__,source_file,__LINE__,endptr,refarray[i]);
#endif
        }
        /* allocate array to save input data for reuse */
        input_data=malloc(sizeof(long)*i);
        if (NULL==input_data) errs++; /* oops, malloc failed; bail out */
        if (0U<errs) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: %m", prog, __func__,source_file,__LINE__);
            goto done;
        }
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: input_data at %p through %p */\n",
                __func__,source_file,__LINE__,(void *)input_data,
                ((char *)input_data)+sizeof(long)*i);
#endif /* DEBUG_CODE */
        /* save input data for reuse */
        /* reset counts to correspond to number of data read */
        for (ul=0UL; ul<(size_t)i; ul++)
            input_data[ul]=refarray[ul];
        startn=ul;
        /* emit revised counts if header is not suppressed */
        if (0U==flags['h'])
            (V)fprintf(stdout,
                "%s: %s line %d: startn = %lu, incr = %s, ul = %lu, count = %lu"
                ", ul*count = %lu, q = %lu\n",
                __func__, source_file, __LINE__, startn, incr, ul, count,
                ul*count, q);
    }

    /* restore (re-seed) random number generator(s) before each call to test_function() */
    if (0U == errs) {
        i = save_random64n_state(s, &p);
        if (0 != i) {
            logger(LOG_ERR, log_arg,
                "%s: %s: %s line %d: save_random64n_state(0x%lx, 0x%lx) returned %d: %m",
                prog, __func__, source_file, __LINE__,
                (unsigned long)s, (unsigned long)(&p), i
            );
            errs++;
        }
#if DEBUG_CODE
          else {
            if (0U!=flags['K']) fprintf(stderr, "# ");
            logger(LOG_INFO, log_arg, "%s: random generator initialized, state saved",
                prog);
        }
#endif
    }

    /* determine optimum (minimum) result output column */
    if (0U==flags['R']) {
        c=5; /* boilerplate */
        /* longest data type name */
        if (0U!=flags['S']) {
            c+=10; /* "structure" */
        } else if ((0U!=flags['P'])||(0U!=flags['I'])) {
            c+=8; /* "integer" or "pointer" */
        } else if ((0U!=flags['F'])||(0U!=flags['A'])) {
            c+=7; /* "double" or "string" */
        } else if (0U!=flags['$']) {
            c+=6; /* "short" */
        } else if (0U!=flags['L']) {
            c+=5; /* "long" */
        }
        if (0U!=flags['[']) {
            for (++c,v=1UL; ratio>v; v*=10UL,c++) ; /* *ratio */
        }
        /* longest sequence name */
        v=(csequences|tsequences);
        if (0U!=v) {
            for (i=0,tests=0U; tests<TEST_SEQUENCE_COUNT; tests++) {
                if (0U != (v & (0x01U << tests))) {
                    int j = (int)strlen(sequence_name(tests));
                    if (j>=i) i=j+1;
                }
            }
            c+=i;
        }
        /* longest function name */
        if (0U!=functions) {
            for (i=0,v=0U; v<FUNCTION_COUNT; v++) {
                if (0U!=(functions & (0x01U << v))) {
                    int j = (int)strlen(function_name(v));
                    if (j>=i) i=j+1;
                }
            }
            c+=i;
        }
        /* longest test type name */
        c+=5; /* "sort" */
        /* largest array size + count string */
        for (i=5,x=1UL,y=ul*count; x<=y; x*=10UL) i++;
        if (10>i) i=10;
        c+=i;
    } else {
        c=2; /* boilerplate */
        /* largest array size */
        for (x=1UL; x<=sz; x*=10UL) c++;
    }
#if 0
(V)fprintf(stderr, "%s line %d: col=%d, c=%d\n",__func__,source_file,__LINE__,col,c);
#endif
    if (c>col) col=c;

    global_ratio=ratio;

#if ((DEBUG_CODE)>0) && defined(DEBUGGING)
    if ((0U!=flags[']']) && (DEBUGGING(CACHE_DEBUG))) {
        (V)fprintf(stderr,
            "/* %s: cache size = %lu bytes */\n",
            __func__,(unsigned long)quickselect_cache_size);
    }
#endif

    /* main array-size loop */
    for (q=ul*count,n=startn; n<=ul; n=z) {
        switch (var_count) {
            case 'r' : /*FALLTHROUGH*/
            case 'R' :
                count = q / n; /* XXX for now... */
            break;
            case 's' : /*FALLTHROUGH*/
            case 'S' :
                count = q / n;
            break;
        }
        if (count>count_limit) count=count_limit;
        global_count=count;

#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stdout,
                "nmemb=%lu, ratio=%lu, count=%lu\n",n,ratio,count);
#endif

        last_adv=33U; /* new count requires new adversary sequence */

        for (sequence=0U; sequence<TEST_SEQUENCE_COUNT; sequence++) {
            u=0x01U<<sequence;
            if (0U!=(u&(csequences|tsequences))) {
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG))
                    (V)fprintf(stdout,"  %s %u for 0x%x in 0x%x or 0x%x\n",
                        sequence_name(sequence),sequence,u,csequences,
                        tsequences);
#endif
                for (func=0U; func<FUNCTION_COUNT; func++) {
                    v=0x01U<<func;
                    if (0U!=(v&functions)) {
#if DEBUG_CODE
                        if (DEBUGGING(SORT_SELECT_DEBUG)) {
                            (V)fprintf(stdout,"    %s %u for 0x%x in 0x%x\n",
                                function_name(func),func,v,functions);
                        }
#endif
/* XXX this might be handled in correctness_tests/timing_tests */
                        switch (sequence_is_randomized(sequence)) {
                            case 0U: /* do nothing */
                            break;
                            case 1U: /* reset random number generator */
                                i = seed_random64n(s, p);
                                if (0 != i) {
                                    logger(LOG_ERR, log_arg,
                                        "%s: %s: %s line %d: seed_random64n(0x"
                                        "%lx, %u) returned %d: %m",
                                        prog, __func__, source_file, __LINE__,
                                        (unsigned long)s, p, i
                                    );
                                    errs++;
                                    goto done;
                                }
                            break;
                            default : /* error */
                                ++errs;
                            goto done;
                        }
                        if (0U!=(u&csequences)) {
                            if (0==valid_test(TEST_TYPE_CORRECTNESS,sequence,n)) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: %s size %lu correctness "
                                    "test is not valid for %s\n",prog,
                                    source_file,__LINE__,
                                    sequence_name(sequence),n,
                                    function_name(func));
                                ++errs;
                                goto done;
                            }
#if DEBUG_CODE
                            if (DEBUGGING(SORT_SELECT_DEBUG))
                                (V)fprintf(stdout,"        %s %s correctness\n",
                                    sequence_name(sequence),function_name(func));
#endif
                            errs+=correctness_tests(u,v,types,options,prog,n,
                                ratio,count,col,timeout,s,p,&last_adv,f,log_arg,
                                flags);
                            if (0U<errs) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: %s size %lu correctness "
                                    "test failed for %s\n",prog,
                                    source_file,__LINE__,
                                    sequence_name(sequence),n,
                                    function_name(func));
                                goto done;
                            }
                        }
                        if (0U!=(u&tsequences)) {
                            if (0==valid_test(TEST_TYPE_TIMING,sequence,n)) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: %s size %lu timing "
                                    "test is not valid for %s\n",prog,
                                    source_file,__LINE__,
                                    sequence_name(sequence),n,
                                    function_name(func));
                                ++errs;
                                goto done;
                            }
#if DEBUG_CODE
                            if (DEBUGGING(SORT_SELECT_DEBUG))
                                (V)fprintf(stdout,"        %s %s timing\n",
                                    sequence_name(sequence),function_name(func));
#endif
                            errs+=timing_tests(u,v,types,options,prog,n,ratio,
                                count,col,timeout,s,p,&last_adv,f,log_arg,flags,
                                &warray,&uarray,&sarray,marray,&dn);
                            if (0U<errs) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: %s size %lu timing "
                                    "test failed for %s\n",prog,
                                    source_file,__LINE__,
                                    sequence_name(sequence),n,
                                    function_name(func));
                                goto done;
                            }
                        }
                    }
                }
            }
        }
        /* stability test is effectively another sequence with a single data type */
        if (0U!=flags['~']) {
            if (NULL==data_array) {
                data_array=malloc(sizeof(struct data_struct)*ul);
                if (NULL==data_array) errs++;
                if (0U<errs) {
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: %m",prog,__func__,source_file,
                        __LINE__);
                    goto done;
                }
                global_data_array = data_array;
#if DEBUG_CODE
                if (DEBUGGING(MEMORY_DEBUG))
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: data_array at %p through %p */\n",
                        __func__,source_file,__LINE__,(void *)data_array,
                        ((char *)data_array)+sizeof(struct data_struct)*ul);
#endif /* DEBUG_CODE */
            }
            i=stability_test(refarray,data_array,functions,n,s,p,ratio,
                options,f,log_arg);
            if (0!=i) errs++;
            if (0U<errs) {
                logger(LOG_ERR, log_arg,
                    "%s: %s: %s line %d: %m",prog,__func__,source_file,__LINE__);
                goto done;
            }
        }

        if (0U!=errs) break;

        buf[0]='\0';
        if (isalnum(*incr)||('('==*incr)) buf[0]='+',buf[1]='\0';
        (V)strncat(buf,incr,sizeof(buf)-3);
        z=snlround(parse_expr((double)n,buf,&endptr,10),f,log_arg);
        if (z<=n) z=++n;
    }

done: ;
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        if ((0U==errs)&&(0U==flags['h'])&&(NULL!=f))
            f(LOG_INFO, log_arg, "%s: tests ended", prog);
#endif

#if 1
    /* test compilation of _s variant; primarily a syntax check */
    /* result, correctness not tested here because "context" is useless */
    (V)qsort_s_wrapper((char *)refarray,2UL,sizeof(long),ilongcmp_s,(void *)refarray);
#endif

    if (NULL!=input_data) free(input_data);
    if (NULL!=warray) { free(warray); warray=NULL; }
    if (NULL!=uarray) { free(uarray); uarray=NULL; }
    if (NULL!=sarray) { free(sarray); sarray=NULL; }
    if (NULL!=parray) {
        if (0U!=flags['[']) parray--;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: parray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)parray,((char *)parray)+sizeof(struct data_struct *)*sz);
#endif /* DEBUG_CODE */
        free(parray); parray=NULL;
    }
    if (NULL!=larray) {
        if (0U!=flags['[']) larray--;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: larray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)larray, ((char *)larray)+sizeof(long)*sz);
#endif /* DEBUG_CODE */
        free(larray); larray=NULL;
    }
    if (NULL!=sharray) {
        if (0U!=flags['[']) sharray--;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: sharray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)sharray, ((char *)sharray)+sizeof(short)*sz);
#endif /* DEBUG_CODE */
        free(sharray); sharray=NULL;
    }
    if (NULL!=darray) {
        if (0U!=flags['[']) darray--;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: darray at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)darray, ((char *)darray)+sizeof(double)*sz);
#endif /* DEBUG_CODE */
        free(darray); darray=NULL;
    }
    if (NULL!=data_array) {
        if (0U!=flags['[']) data_array--;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: data_array at %p through %p */\n",
                __func__,source_file,__LINE__,(void *)data_array,
                ((char *)data_array)+sizeof(struct data_struct)*sz);
#endif /* DEBUG_CODE */
        free(data_array); data_array=NULL;
    }
    if (NULL!=array) {
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: array at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)array, ((char *)array)+sizeof(int)*sz);
#endif /* DEBUG_CODE */
        if (0U!=flags['[']) array--;
#if DEBUG_CODE
        if (DEBUGGING(MEMORY_DEBUG))
            (V)fprintf(stderr,
                "/* %s: %s line %d: array at %p through %p */\n",__func__,source_file,__LINE__,
                (void *)array, ((char *)array)+sizeof(int)*sz);
#endif /* DEBUG_CODE */
        free(array); array=NULL;
    }
    if (NULL!=refarray) { free(refarray); refarray=NULL; }
    global_refarray = refarray, global_larray = larray, global_parray = parray,
        global_darray = darray, global_iarray = array,
        global_data_array = data_array, global_uarray = uarray,
        global_sarray = sarray, global_warray = warray,
        global_sharray = sharray;

#if DEBUG_CODE
    if (DEBUGGING(MEMORY_DEBUG))
        if ((0U==flags['h'])&&(NULL!=f))
            f(LOG_INFO, log_arg, "%s: arrays freed", prog);
#endif

    if (DEBUGGING(SORT_SELECT_DEBUG))
        if ((0U==flags['h'])&&(NULL!=f)&&(0UL!=errs))
            f(LOG_INFO, log_arg, "%s: errs=%lu", prog, errs);
    return (0U==errs?0:1);

#undef buildstr
#undef xbuildstr
}
