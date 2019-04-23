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
* $Id: ~|^` @(#)    timing.c copyright 2016-2018 Bruce Lilly.   \ timing.c $
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
/* $Id: ~|^` @(#)   This is timing.c version 1.49 dated 2019-04-19T19:48:25Z. \ $ */
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
#define MODULE_VERSION "1.49"
#define MODULE_DATE "2019-04-19T19:48:25Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

static size_t best_cmp=(SIZE_MAX), best_sw=(SIZE_MAX),
    best_moves=(SIZE_MAX), worst_cmp=0UL, 
    worst_sw=0UL, worst_moves=0UL, total_rotations[MAXROTATION];
static double total_eq=0.0, total_gt=0.0, total_lt=0.0, total_sw=0.0,
    total_moves=0.0, total_pcopies=0.0, total_pderefs=0.0,
    total_piconversions=0.0, best_equiv=9.9e9, best_op=9.9e9,
    total_partitions=0.0, total_repivots=0.0, total_merges=0.0,
    total_recursions=0.0, worst_equiv=0.0, worst_op=0.0,
    total_rot_equiv=0.0;

void reset_counters(unsigned int totals_too)
{
    size_t x;
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stderr,"/* %s: totals_too=%u */\n",__func__,totals_too);
#endif
    ngt=nlt=neq=nsw=nmoves=npcopies=npderefs=npiconversions=0UL;
    nmerges=npartitions=nrecursions=nrepivot=0UL;
    for (x=0UL; x<MAXROTATION; x++)
        nrotations[x] = 0UL;
    if (0U!=totals_too) {
        for (x=0UL; x<MAXROTATION; x++)
            total_rotations[x] = 0UL;
        best_cmp=(SIZE_MAX), best_sw=(SIZE_MAX), best_moves=(SIZE_MAX),
        worst_cmp=0UL, worst_sw=0UL, worst_moves=0UL;
        total_eq=0.0, total_gt=0.0, total_lt=0.0, total_sw=0.0, total_moves=0.0,
        total_pcopies=0.0, total_pderefs=0.0, total_piconversions=0.0,
        best_equiv=9.9e9, best_op=9.9e9, worst_equiv=0.0, total_rot_equiv=0.0;
        total_partitions=0.0, total_repivots=0.0, total_merges=0.0,
        total_recursions=0.0;
    }
}

void update_totals(size_t count, void (*f)(int, void *, const char *, ...),
    void *log_arg)
{
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stderr,"/* %s: count=%lu */\n",__func__,count);
#endif
     if (0UL!=count) {
        total_partitions=(double)npartitions/(double)count;
        total_repivots=(double)nrepivot/(double)count;
        total_merges=(double)nmerges/(double)count;
        total_recursions=(double)nrecursions/(double)count;
    }
}

void update_counters(size_t size_ratio)
{
    double d;
    size_t cmp, nops, x;
    cmp = neq+ngt+nlt; /* XXX could overflow for very large nmemb (N log N > SIZE_MAX) or for adverse inputs (e.g. with quadratic performance and nmemb > sqrt(SIZE_MAX)) */
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stderr,"/* %s: size_ratio=%lu, cmp=%lu */\n",__func__,size_ratio,cmp);
#endif
    if (cmp>worst_cmp) worst_cmp=cmp;
    if (cmp<best_cmp) best_cmp=cmp;
    if (nsw>worst_sw) worst_sw=nsw;
    if (nsw<best_sw) best_sw=nsw;
    if (nmoves>worst_moves) worst_moves=nmoves;
    if (nmoves<best_moves) best_moves=nmoves;
    total_eq += (double)neq;
    total_gt += (double)ngt;
    total_lt += (double)nlt;
    total_sw += (double)nsw;
    total_moves += (double)nmoves;
    total_pcopies += (double)npcopies;
    total_pderefs += (double)npderefs;
    total_piconversions += (double)npiconversions;
    /* Swaps, moves, and rotations are scaled by
       size_ratio and an appropriate multiplier for the
       number of reads plus writes.
    */
    d=0.0;
    /* rotation = distance * (read + write) */
    for (x=1UL; x<MAXROTATION; x++) {
        if (0UL!=nrotations[x]) {
            total_rotations[x] += nrotations[x];
            d += 2.0 * (double)size_ratio
              * (double)(nrotations[x]*x);
        }
    }
    if (0UL!=nrotations[0]) {
        total_rotations[0] += nrotations[0];
        d += 2.0 * (double)size_ratio * (double)(nrotations[0]*10);
    }
    total_rot_equiv += d;
    /* swap = read, read, write, write */
    d += 4.0 * (double)size_ratio * (double)nsw;
    /* move = read, write */
    d += 2.0 * (double)size_ratio * (double)nmoves;
    /* Pointer operations are not affected by
       size_ratio.
    */
    /* pointer copy = read, write */
    d += 2.0 * (double)npcopies;
    /* dereference = read */
    d += (double)npderefs;
    /* index conversion = read, subtract, divide, write
       (only approximately correct as time depends on
       whether data resides in registers, in cache, or
       in bulk memory (or worse, is swapped to disk))
    */
    d += 4.0 * (double)npiconversions;
    d /= 4.0; /* normalize to swap equivalents */
    if (d>worst_equiv) worst_equiv=d;
    if (d<best_equiv) best_equiv=d;
    nops = (double)cmp+d;
    if (nops>worst_op) worst_op=nops;
    if (nops<best_op) best_op=nops;
}

void print_counters(const char *comment, int col, size_t n, size_t count,
    double factor, const char *pcc, const char *pfunc, const char *psize,
    const char *typename, const char *ptest, unsigned char *flags,
    size_t size_ratio, void (*f)(int, void *, const char *, ...), void *log_arg)
{
    char buf[256], bufz[256];
    int i;
    double d;
    size_t x;

    i = 1 + snprintf(buf, sizeof(buf), "%lu", n);
    d = total_eq+total_gt+total_lt;
    if (0U!=flags['R']) { /* Terse output. */
        char bufb[256], bufw[256], buf0[256];
        (V)snf(bufb,sizeof(bufb),NULL,NULL,best_cmp,'0',1,-15,f,log_arg);
        (V)snf(bufw,sizeof(bufw),NULL,NULL,worst_cmp,'0',1,-15,f,log_arg);
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d/(double)count,'0',1,-12,f,log_arg);
        (V)printf("%s%s%*.*s%.6G %s mean comparisons [%s-%s] (%s)\n",comment,
            buf,col-i,col-i," ",d/(double)count/factor,psize,bufb,bufw,buf0);
    } else { /* Verbose output w/ comparison type breakdown. */
        i=1+snprintf(bufz,sizeof(bufz),"%s %s vs. %lu %s %s total comparisons:",
            pcc,pfunc,n,typename,ptest);
        (V)printf("%s%s%*.*s%.0f < (%.1f%%), %.0f == (%.1f%%), %.0f > (%.1f%%),"
            " mean %.0f: %.6G %s\n",comment,bufz,col-i,col-i," ",total_lt,
            0.0==d? 100.0: 100.0*total_lt/d,total_eq,
            0.0==d? 100.0: 100.0*total_eq/d,total_gt,
            0.0==d? 100.0: 100.0*total_gt/d,
            d/(double)count,d/(double)count/factor,psize);
    } /* Terse or verbose comparisons. */
    if (0.0<total_sw) { /* Swaps to report? */
        char bufb[256], bufw[256], buf0[256];
        d = total_sw / (double)count;
        (V)snf(bufb,sizeof(bufb),NULL,NULL,(double)best_sw,'0',1,-15,f,log_arg);
        (V)snf(bufw,sizeof(bufw),NULL,NULL,(double)worst_sw,'0',1,-15,f,
            log_arg);
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G %s swaps [%s-%s] (%s)\n",comment,buf,col-i,
            col-i," ",d/factor,psize,bufb,bufw,buf0);
    }
    if (0.0<total_moves) { /* Data moves to report? */
        char bufb[256], bufw[256], buf0[256];
        d = total_moves / (double)count;
        (V)snf(bufb,sizeof(bufb),NULL,NULL,(double)best_moves,'0',1,-15,f,
            log_arg);
        (V)snf(bufw,sizeof(bufw),NULL,NULL,(double)worst_moves,'0',1,-15,f,
            log_arg);
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G N = %.6G %s moves [%s-%s] (%s)\n", comment,
            buf,col-i,col-i," ",d/(double)n,d/factor,psize,bufb,bufw,buf0);
    }
    for (x=1UL; x<MAXROTATION; x++) {
        if (0UL!=total_rotations[x]) {
            char buf0[256];
            d = (double)(total_rotations[x])/(double)count;
            (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
            (V)printf("%s%s%*.*s%.6G %s rotations of %lu elements (%s)\n",
                comment,buf,col-i,col-i," ",d/factor,psize,x,buf0);
        }
    }
    if (0UL!=total_rotations[0]) {
        char buf0[256];
        d = (double)(total_rotations[0])/(double)count;
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G %s rotations of >= %d elements (%s)\n",
            comment,buf,col-i,col-i," ",d/factor,psize,MAXROTATION,buf0);
    }
    if (0.0<total_pcopies) { /* Pointer copies to report? */
        char buf0[256];
        d = total_pcopies / (double)count;
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G N = %.6G %s pointer copies (%s)\n",
            comment,buf,col-i,col-i," ",d/(double)n,d/factor,psize,buf0);
    }
    if (0.0<total_pderefs) { /* Dereferences to report? */
        char buf0[256];
        d = total_pderefs / (double)count;
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G N = %.6G %s pointer dereferences (%s)\n",
            comment,buf,col-i,col-i," ",d/(double)n,d/factor,psize,buf0);
    }
    if (0.0<total_piconversions) { /* Conversions to report? */
        char buf0[256];
        d = total_piconversions / (double)count;
        (V)snf(buf0, sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G N = %.6G %s pointer to index conversions (%s)"
            "\n",comment,buf,col-i,col-i," ",d/(double)n,d/factor,psize,buf0);
    }
    /* Swaps, moves, and rotations are scaled by
       size_ratio and an appropriate multiplier for the
       number of reads plus writes.
    */
    d=0.0;
    /* swap = read, read, write, write */
    d += 4.0 * (double)size_ratio * total_sw;
    /* move = read, write */
    d += 2.0 * (double)size_ratio * total_moves;
    /* Pointer operations are not affected by
       size_ratio.
    */
    /* pointer copy = read, write */
    d += 2.0 * total_pcopies;
    /* dereference = read */
    d += total_pderefs;
    /* index conversion = read, subtract, divide, write
       (only approximately correct as time depends on
       whether data resides in registers, in cache, or
       in bulk memory (or worse, is swapped to disk))
    */
    d += 4.0 * total_piconversions;
    /* rotation = distance * (read + write) */
    d += total_rot_equiv;
    d /= 4.0; /* normalize to swap equivalents */
    if (0.0<d) { /* Scaled operations to report? */
        char bufb[256], bufw[256], buf0[256];
        d /= (double)count;
        (V)snf(bufb,sizeof(bufb),NULL,NULL,best_equiv,'0',1,-15,f,log_arg);
        (V)snf(bufw,sizeof(bufw),NULL,NULL,worst_equiv,'0',1,-15,f,log_arg);
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G %s roughly swap-equivalent non-comparison data"
            " R/W operations [%s-%s] (%s)\n",
            comment,buf,col-i,col-i," ",d/factor,psize,bufb,bufw,buf0);
        /* add comparisons */
        d += (total_eq+total_gt+total_lt) / (double)count;
        (V)snf(bufb,sizeof(bufb),NULL,NULL,best_op,'0',1,-15,f,log_arg);
        (V)snf(bufw,sizeof(bufw),NULL,NULL,worst_op,'0',1,-15,f,log_arg);
        (V)snf(buf0,sizeof(buf0),NULL,NULL,d,'0',1,-15,f,log_arg);
        (V)printf("%s%s%*.*s%.6G %s operations [%s-%s] (%s)\n",
            comment,buf,col-i,col-i," ",d/factor,psize,bufb,bufw,buf0);
    }
    if (total_partitions>0.0) {
        (V)sng(bufz,sizeof(bufz),NULL,
            total_partitions==1.0?" partition":" partitions",
            total_partitions,-6,3,f,log_arg);
        (V)printf("%s%s%*.*s%s\n",comment,buf,col-i,col-i," ",bufz);
    }
    if (total_repivots>0.0) {
        (V)sng(bufz,sizeof(bufz),NULL,
            total_repivots==1.0?" repivot":" repivots",
            total_repivots,-6,3,f,log_arg);
        (V)printf("%s%s%*.*s%s (%.3f%%)\n",comment,buf,col-i,col-i," ",bufz,
            100.0*(double)nrepivot/(double)npartitions);
    }
    if (total_recursions>0.0) {
        (V)sng(bufz,sizeof(bufz),NULL,
            total_recursions==1.0?" recursion":" recursions",
            total_recursions,-6,3,f,log_arg);
        (V)printf("%s%s%*.*s%s\n",comment,buf,col-i,col-i," ",bufz);
    }
    if (total_merges>0.0) {
        (V)sng(bufz,sizeof(bufz),NULL,total_merges==1.0?" merge":" merges",
            total_merges,-6,3,f,log_arg);
        (V)printf("%s%s%*.*s%s\n",comment,buf,col-i,col-i," ",bufz);
    }
}

unsigned int timing_tests(unsigned int sequences, unsigned int functions,
    unsigned int types, unsigned int options, const char *prog, size_t n,
    size_t ratio, size_t count, int col, double timeout, uint_least64_t *s,
    unsigned int p, unsigned int *plast_adv,
    void (*f)(int, void *, const char *, ...), void *log_arg,
    unsigned char *flags, float **pwarray, float **puarray, float **psarray,
    size_t *marray, size_t *pdn)
{
    char buf[256], buf1[256], buf2[256], buf3[256], buf4[256], buf5[256],
        buf6[256], buf7[256], buf8[256], buf9[256], buf10[256], buf11[256],
        buf12[256], buf13[256], buf14[256], buf15[256], buf16[256], buf17[256],
        buf18[256], buf19[256], buf20[256], buf21[256], buf22[256], buf23[256],
        buf24[256];
    const char *comment="", *pcc, *pfunc, *typename, *psize, *ptest;
    int i, c, method, odebug;
    unsigned int errs=0U, ff, function, inst, ss, sequence, t, tests, type;
    long l;
    float best_s=9.9e9, best_u=9.9e9, best_w=9.9e9, 
        test_s, tot_s=0.0, test_u, tot_u=0.0, test_w, tot_w=0.0,
        worst_s=0.0, worst_u=0.0, worst_w=0.0;
    double d, factor;
    size_t alignsize, b[12], best_m, carray[MAX_PERMUTATION_SIZE], ct, cycle, j,
        k, *karray=NULL, m, nc, nk, ocount, size, size_ratio, u, w, worst_m, x,
        y;
    FILE *fp=NULL;
    void *pv;
    auto struct rusage rusage_start, rusage_end;
    auto struct timespec timespec_start, timespec_end, timespec_diff;
    uint64_t max_val;
    int (*compar)(const void *, const void *);
    int (*compar_s)(const void *,const void *,void *);

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
    if (DEBUGGING(SORT_SELECT_DEBUG))
        (V)fprintf(stderr,"/* %s(sequences=0x%x,functions=0x%x,types=0x%x,"
            "options=0x%x,prog=%s,n=%lu,ratio=%lu,count=%lu,col=%d,timeout=%.3f"
            ",...) */\n",__func__,sequences,functions,types,options,prog,n,
            ratio,count,col,timeout);
#endif
#if SILENCE_WHINEY_COMPILERS
    factor=0.0, best_m=nc=nk=worst_m=0UL, ocount=count, odebug=debug;
#endif
    for (sequence=ls_bit_number(ss=sequences); sequence<32U; sequence=ls_bit_number(ss)) {
        ss &= ~(0x01<<sequence);
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: sequence=%u, ss=0x%x */\n",__func__,
                sequence,ss);
#endif
        ptest = sequence_name(sequence);
#if DEBUG_CODE
        if (DEBUGGING(SORT_SELECT_DEBUG))
            (V)fprintf(stderr,"/* %s: ptest=%s */\n",__func__,ptest);
#endif
        if (NULL == ptest) return ++errs;
        u = n - 1UL;
        k = n >>1; /* upper median */
        if (0U==errs) {
            switch (sequence) {
                case TEST_SEQUENCE_PERMUTATIONS :
                    nc=MAX_PERMUTATION_SIZE;
                    if (n>nc) return ++errs;
                    if (nc>n) nc=n;
                    cycle = factorial(nc);
                    count *= cycle;
                    ocount = count;
                    /* Progress indication to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
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
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                break;
                default :
                    cycle = count;
                break;
            }
            if ((1UL<count)&&(NULL==fp)) {
                fp = fopen("/dev/tty", "w");
                if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
            }
            /* preparation */
#if DEBUG_CODE
            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                "/* %s: %s line %d: psarray=%p, *psarray=%p, count=%lu */\n",
                __func__,source_file,__LINE__,(void *)psarray,NULL==psarray?NULL:(void *)(*psarray),count);
#endif
            *psarray=realloc(*psarray, sizeof(float)*count);
#if DEBUG_CODE
            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
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
            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                "/* %s: %s line %d: puarray=%p, *puarray=%p, count=%lu */\n",
                __func__,source_file,__LINE__,(void *)puarray,NULL==puarray?NULL:(void *)(*puarray),count);
#endif
            *puarray=realloc(*puarray, sizeof(float)*count);
#if DEBUG_CODE
            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
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
            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                "/* %s: %s line %d: pwarray=%p, *pwarray=%p, count=%lu */\n",
                __func__,source_file,__LINE__,(void *)pwarray,NULL==pwarray?NULL:(void *)(*pwarray),count);
#endif
            *pwarray=realloc(*pwarray, sizeof(float)*count);
#if DEBUG_CODE
            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
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
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
            l=sysconf(_SC_PHYS_PAGES);
            if (0L>l) {
                (V)fprintf(stderr,
                   "%s: %s line %d: sysconf(_SC_PHYS_PAGES) returned %ld, "
                   "errno=%d(%s)\n",__func__,source_file,__LINE__,l,errno,
                   strerror(errno));
                x=268435456UL; /* 256MiB */
            } else {
                x=(size_t)l;
                l=sysconf(_SC_PAGESIZE);
                if (0L>l) {
                    (V)fprintf(stderr,
                       "%s: %s line %d: sysconf(_SC_PAGESIZE) returned %ld, "
                       "errno=%d(%s)\n",__func__,source_file,__LINE__,l,errno,
                       strerror(errno));
                    x=268435456UL; /* 256MiB */
                } else {
                    x*=(size_t)l;
                }
            }
#else
            x=268435456UL; /* 256MiB */
#endif
#define BELL_PAUSE_TIME 4
            y=0UL;
            if (NULL!=global_refarray) y+=sizeof(long)*n;
            if (NULL!=global_larray) y+=sizeof(long)*n*ratio;
            if (NULL!=global_iarray) y+=sizeof(int)*n*ratio;
            if (NULL!=global_sharray) y+=sizeof(short)*n*ratio;
            if (NULL!=global_darray) y+=sizeof(double)*n*ratio;
            if (NULL!=global_data_array) y+=sizeof(struct data_struct)*n*ratio;
            if (NULL!=global_parray) y+=sizeof(struct data_struct *)*n*ratio;
            if (y>=(x>>2)) {
                if (NULL==fp) {
                    /* warning to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                    else fp=stderr;
                }
                (V)sng(buf,sizeof(buf),NULL,NULL,(double)y,-4,3,f,log_arg);
                (V)fprintf(fp,
                    "%s: %s: \aWARNING: test data for ratio %lu requires %lu "
                    "(%s) bytes.\n"
                    "WARNING: Expect significant delays, or abort and specify "
                    "a smaller count or problem size.\n",
                    prog,__func__,(unsigned long)ratio,(unsigned long)y,buf);
                sleep(BELL_PAUSE_TIME); (V)fprintf(fp, "\a");
                sleep(BELL_PAUSE_TIME); (V)fprintf(fp, "\a");
                fflush(fp);
                if (fp==stderr) fp=NULL;
                sleep(BELL_PAUSE_TIME);
            }
#if DEBUG_CODE
            else if (DEBUGGING(MEMORY_DEBUG))
                (V)fprintf(stderr,
                    "/* %s: %s: test data for ratio %lu requires %lu bytes <= "
                    "physical memory %lu bytes. */\n",
                    prog,__func__,ratio,y,x);
#endif
            w=3UL*sizeof(float)*count;
            if (w>=(x>>2)) {
                if (NULL==fp) {
                    /* warning to /dev/tty */
                    fp = fopen("/dev/tty", "w");
                    if (NULL!=fp) (V) setvbuf(fp, NULL, (int)_IONBF, 0);
                    else fp=stderr;
                }
                (V)sng(buf,sizeof(buf),NULL,NULL,(double)w,-4,3,f,log_arg);
                (V)fprintf(stderr,
                    "%s: %s: \aWARNING: %lu timing data requires %lu (%s) "
                    "bytes.\n"
                    "WARNING: Expect significant delays, or abort and specify "
                    "a smaller count or problem size.\n",
                    prog,__func__,(unsigned long)count,(unsigned long)w,buf);
                sleep(BELL_PAUSE_TIME); (V)fprintf(stderr, "\a");
                sleep(BELL_PAUSE_TIME); (V)fprintf(stderr, "\a");
                fflush(fp);
                if (fp==stderr) fp=NULL;
                sleep(BELL_PAUSE_TIME);
            }
#if DEBUG_CODE
            else if (DEBUGGING(MEMORY_DEBUG))
                (V)fprintf(stderr,
                    "/* %s: %s: %lu timing data requires %lu bytes <= "
                    "physical memory %lu bytes. */\n",
                    prog,__func__,count,w,x);
#endif
            /* generate non-random, count-independent, type-independent,
               function-independent sequence data here
            */
            switch (sequence) {
                case TEST_SEQUENCE_STDIN :
                    duplicate_test_data(input_data,(char *)global_refarray,
                        DATA_TYPE_LONG,ratio,0UL,n);
                break;
                case TEST_SEQUENCE_WORST :
                    /* do nothing here; sequence is type-dependent */
                break;
                case TEST_SEQUENCE_ADVERSARY :
                    /* do nothing here; sequence is function-dependent */
                break;
                case TEST_SEQUENCE_COMBINATIONS :
                    /* do nothing here; sequence is count-dependent */
                break;
                case TEST_SEQUENCE_PERMUTATIONS :
                    /* do nothing here; sequence is count-dependent */
                break;
                default :
                    switch (sequence_is_randomized(sequence)) {
                        case 0U : /* not randomized; initialize */
                            i=generate_long_test_array(global_refarray,
                                n, sequence, 1L, LONG_MAX, f, log_arg);
                            if (0 > i) {
                                (V)fprintf(stderr,
                                    "%s: %s line %d: generate_long_test_array returned %d\n",
                                    __func__, source_file, __LINE__, i);
                                return ++errs;
                            } else if (0 < i) {
                                return errs;
                            }
                        break;
                        case 1U : /* randomized; do nothing here */
                        break;
                        default : /* error */
                        return ++errs;
                    }
                break;
            }
            for (function=ls_bit_number(ff=functions); function<32U; function=ls_bit_number(ff)) {
                ff &= ~(0x01<<function);
#if DEBUG_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG))
                    (V)fprintf(stderr,"/* %s: function=%u, ff=0x%x */\n",
                        __func__,function,ff);
#endif
                pcc=function_name(function);
                if (NULL == pcc) return ++errs;
                else {
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: pcc=%s */\n",__func__,pcc);
#endif
                    tests=0U;
                    pfunc = function_type(function, &tests);
                    if (NULL == pfunc) return ++errs;
                    factor = test_factor(tests, n);
                    psize = test_size(tests);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: pcc=%s, pfunc=%s, factor=%.3f, psize=%s, tests=0x%x */\n",__func__,pcc,pfunc,factor,psize,tests);
#endif
                }
                /* prepare karray before timing for selection */
                switch (function) {
                    case FUNCTION_QSELECT_S : /*FALLTHROUGH*/
                    case FUNCTION_QSELECT :
                        if ((1UL==selection_nk)&&(flags['m']=='m')) { /* special-case: 1 or 2 median(s) */
                            nk = 2UL;
#if DEBUG_CODE > 1
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: allocating karray  %lu ranks */\n",
                                __func__,source_file,__LINE__,nk);
#endif
                            karray = malloc(nk*sizeof(size_t));
                            if (NULL==karray) return ++errs;
#if DEBUG_CODE > 1
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: karray at %p through %p */\n",
                                __func__,source_file,__LINE__,(void *)karray, ((char *)karray)+sizeof(double)*nk);
#endif
                            karray[0] = u>>1; /* lower-median */
                            karray[1] = k; /* upper-median */
                        } else {
                            /* Notes for testing sampling tables for
                               selection:
                               Remedian of samples:
                                  Remedian of samples is used initially for
                                  rank distributions 2,3,5,6. Median of
                                  samples is used for distributions 1,4 and
                                  sorting tables are used for distributions
                                  0 and 7.  The remedian of samples "middle"
                                  sampling table is used for distribution 2
                                  and the remedian of samples "ends"
                                  sampling table is used for distribution 5
                                  (only).  Initial use of the "ends" table
                                  can be forced with -[2 -ms4 which uses
                                  size_ratio>1 to preclude median of samples
                                  and places 2 ranks at each end of the
                                  array (subsequent partitioning will use
                                  the median of samples "ends" table); 2
                                  ranks precludes a search for min/max.
                                  Initial use of remedian of samples "ends"
                                  can also be forced with -[2 -mqN with N>1,
                                  which will subsequently use the median of
                                  samples "ends" and "middle" tables.
                                  Use of the remedian of samples "middle"
                                  table can be forced with -[2 -mm, which
                                  will use that table on the first partition
                                  only.  Distributions 2 and 6 may be forced
                                  with -mr and -ml respectively; they use the
                                  sorting sampling table.
                               Median of samples:
                                  Median of samples is used always for rank
                                  distributions 1 and 4 ("ends"), and for
                                  distributions 2 ("middle") only if
                                  size_ratio==1.  Use of the median of
                                  samples "ends" table can be forced with
                                  -[1 (to force size_ratio 1) with a basic
                                  type (short, int, long, or double) and
                                  using -m{b,e,l,r}N with 1<N<nmemb/4 (which
                                  should first use the "ends" table, then
                                  the "middle" table, then sort), or using
                                  -mq (or -mq1) (which should alternate
                                  between the "ends" and "middle" tables).
                                  The "middle" table is forced using
                                  -[1 -mm, which may also use the "ends"
                                  table if the desired rank isn't caught on
                                  the initial partition.  Distributions 2 and 6
                                  may be forced with -mr and -ml respectively.
                            */
                            /* order statistic rank quantity */
                            switch (flags['m']) {
                                case 'q' : /* quartile */
                                    /* If selection_nk==1, ranks are placed at one
                                       side of the array only; for selection_nk>1,
                                       ranks may be placed on both sides.
                                    */
                                    /* Force value of selection_nk to be either 1 or
                                       2.
                                    */
                                    if (1UL<selection_nk) selection_nk=2UL;
                                    if (11UL>n) nk=1UL;
                                    else if (21UL>n) nk=selection_nk;
                                    else
                                        for (nk=selection_nk,x=21UL; x<=n;
                                        nk+=selection_nk,x=((x+1UL)<<2)-1UL
                                        )
                                            ;
                                break;
                                case 'l' : /*FALLTHROUGH*/ case 'r' : /* 110 011 */
                                    for (w=(n>>2),x=1UL,y=2UL; y<w; x++,y<<=1) ;
                                    nk = (3UL*y)>>2;
                                break;
                                default :
                                    nk = selection_nk;
                                break;
                            }
                            if (nk>n) nk=n;
#if DEBUG_CODE
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: allocating karray  %lu ranks */\n",
                                __func__,source_file,__LINE__,nk);
#endif
                            karray = malloc(nk*sizeof(size_t));
                            if (NULL==karray) return ++errs;
#if DEBUG_CODE
                            if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: karray at %p through %p */\n",
                                __func__,source_file,__LINE__,(void *)karray, ((char *)karray)+sizeof(double)*nk);
#endif
                            /* order statistic rank quality and assignment */
                            switch (flags['m']) {
                                case 'b' : /* beginning 100 */
                                    for (t=0UL; t<nk; t++)
                                        karray[t] = (t<n)?t:u;
                                break;
                                case 'd' : /*FALLTHROUGH*/
                                default : /* distributed 010 (d1-d3) 111 (d4+) */
                                    y=n/(1UL+nk);
                                    x=(n-(nk-1UL)*y)>>1;
                                    for (t=0U; t<nk; t++,x+=y)
                                        karray[t] = x;
                                break;
                                case 'e' : /* end 001 */
                                    for (t=0U; t<nk; t++)
                                        karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                break;
                                case 'l' : /* left 110 */
                                    for (t=0U,x=1UL,w=n/y; t<nk; t++,x+=w)
                                        karray[t] = x;
                                break;
                                case 'm' : /* middle 010 */
                                    x = (n-nk)>>1;
                                    for (t=0U; t<nk; t++)
                                        karray[t] = t+x;
                                break;
                                case 'q' : /* quartile 100 101 */
                                    /* If selection_nk==1UL, place ranks on one
                                       side (only) of the array such that the
                                       raw distribution is 4 or 1, so that the
                                       median of samples "ends" sampling table
                                       is used.  If selection_nk>1UL, ranks may
                                       be placed on both sides of the array so
                                       that the remedian of samples "ends"
                                       sampling table is used initially
                                       (subsequent partitions will use the
                                       median of samples "ends" sampling table
                                       for pivot selection in each region).
                                    */
                                    for (y=n,t=0U; ; ) {
                                        if (0UL<y) y=((y-1UL)>>1)-((y+1UL)>>2);
                                        karray[t] = y;
                                        t++;
                                        if (1UL<selection_nk) {
                                            if (t<=nk-t)
                                                karray[nk-t] = u-y;
                                            if (t==nk-t) break;
                                        } else if (t==nk) break;
                                    }
                                break;
                                case 'r' : /* right 011 */
                                    for (t=0U,x=u-1UL,w=n/y; t<nk; t++,x-=w)
                                        karray[nk-1UL-t] = x;
                                break;
                                case 's' : /* separated 101 */
                                    x = (nk>>1);
                                    for (t=0U; t<x; t++)
                                        karray[t] = (t<n)?t:u;
                                    for (; t<nk; t++)
                                        karray[t] = (nk<n)?n-nk+t:((t<n)?t:u);
                                break;
                            }
                        }
#if DEBUG_CODE
                        if (DEBUGGING(SORT_SELECT_DEBUG)) {
                            (V)fprintf(stderr,"/* %s: %s line %d: %lu order statistic rank%s: %lu",
                            __func__,source_file,__LINE__,nk,nk==1?"":"s",karray[0]);
                            for (t=1UL; t<nk; t++)
                            (V)fprintf(stderr, ", %lu", karray[t]);
                            (V)fprintf(stderr, " */\n");
                        }
#endif
                    break;
                }
                /* Generate non-random, type-independent, count-independent,
                   function-dependent sequence data here.
                   Adversary sequence is function-dependent.
                   Generate sequence for first large type.
                */
                switch (sequence) {
                    case TEST_SEQUENCE_ADVERSARY :
                        type=ls_bit_number(t=types); /* types are in descending size order */
#if DEBUG_CODE
                        if (DEBUGGING(SORT_SELECT_DEBUG))
                            (V)fprintf(stderr,"/* %s: type=%u, t=0x%x */\n",
                                __func__,type,t);
#endif
                        typename=type_name(type);
#if DEBUG_CODE
                        if (DEBUGGING(SORT_SELECT_DEBUG))
                            (V)fprintf(stderr,"/* %s: typename=%s */\n",__func__,typename);
#endif
                        if (0U!=flags[';']) {
                            pv=global_larray;;
                            size=sizeof(long);
                        } else {
                            pv=type_array(type);
                            size=ratio*type_size(type);
                        }
                        /* generate new test sequence */
                        if ((NULL!=f)&&(0U==flags[':'])) {
                            if (0U!=flags['K']) fprintf(stderr, "# ");
                            if (1UL<ratio)
                                f(LOG_INFO, log_arg, "%s: %s %s %s*%lu %s "
                                    "timing test: preparing adverse input",
                                    prog, pcc, pfunc, typename, ratio,ptest);
                            else
                                f(LOG_INFO, log_arg, "%s: %s %s %s %s "
                                    "timing test: preparing adverse input",
                                    prog, pcc, pfunc, typename, ptest);
                            fflush(stderr);
                        }
                        if (!(DEBUGGING(AQCMP_DEBUG))) odebug=debug, debug=0;
                        /* odebug is guaranteed to have been assigned a
                           value by the above statement when
                           DEBUGGING(AQCMP_DEBUG) whether or not (:-/) gcc's
                           authors realize it...
                        */
                        switch (function) {
                            /* sampling breakpoints differ between sorting and selection */
                            case FUNCTION_QSELECT :      /*FALLTHROUGH*/
                            case FUNCTION_QSELECT_S :    /*FALLTHROUGH*/
                            case FUNCTION_QSELECT_SORT : /*FALLTHROUGH*/
                                if (FUNCTION_QSELECT!=*plast_adv) {
                                    *plast_adv=FUNCTION_QSELECT;
                                    initialize_antiqsort(n,pv,type,ratio,size,
                                        global_refarray);
                                    if (0U!=flags['i'])
                                        D_QSEL(pv,0UL,u,size,aqcmp,karray,0UL,nk,0U);
                                    else
                                        QSEL(pv,0UL,u,size,aqcmp,karray,0UL,nk,0U);
                                }
                            break;
                            case FUNCTION_QSORT_WRAPPER :
                                if (FUNCTION_QSORT_WRAPPER!=*plast_adv) {
                                    *plast_adv=FUNCTION_QSORT_WRAPPER;
                                    initialize_antiqsort(n,pv,type,ratio,size,
                                        global_refarray);
                                    if (0U == flags['i'])
                                        QSORT_FUNCTION_NAME((char *)pv,n,size,aqcmp);
				    else
                                        d_qsort((char *)pv,n,size,aqcmp);
                                }
                            break;
                            case FUNCTION_SQSORT :       /*FALLTHROUGH*/
                            case FUNCTION_WQSORT :
                                if (FUNCTION_WQSORT!=*plast_adv) {
                                    *plast_adv=FUNCTION_WQSORT;
                                    initialize_antiqsort(n,pv,type,ratio,size,
                                        global_refarray);
                                    WQSORT(pv,0UL,u,size,aqcmp,karray,0UL,nk,0U);
                                }
                            break;
                            case FUNCTION_BMQSORT : /*FALLTHROUGH*/
                            case FUNCTION_IBMQSORT :
                                if (FUNCTION_WBMQSORT!=*plast_adv) {
                                    *plast_adv=FUNCTION_WBMQSORT;
                                    initialize_antiqsort(n,pv,type,ratio,size,
                                        global_refarray);
                                    WBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                                }
                            break;
                            case FUNCTION_NBQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                NBQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_DEDSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                DEDSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_DPQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                DPQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_GLQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                GLQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_HEAPSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                HEAPSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_INDIRECT_MERGESORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                IMERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_INTROSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                INTROSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_ISORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                ISORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_LOGSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                LOGSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_MBMQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                MBMQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_MERGESORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                MERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_MINMAXSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                MMSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_NETWORKSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                NETWORKSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_P9QSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                P9QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_ILLUMOSQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                ILLUMOSQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_QSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                QSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_RUNSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                RUNSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SELSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SELSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SHELLSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SHELLSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SMOOTHSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SMOOTHSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SQRTSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SQRTSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_SYSMERGESORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                SYSMERGESORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_YQSORT :
                                *plast_adv=function;
                                initialize_antiqsort(n,pv,type,ratio,size,
                                    global_refarray);
                                YQSORT(pv,0UL,u,size,aqcmp,NULL,0UL,0UL,0U);
                            break;
                        }
                        if (!(DEBUGGING(AQCMP_DEBUG))) debug=odebug;
                        if ((NULL!=f)&&(0U==flags[':'])) {
                            if (0U!=flags['K']) fprintf(stderr, "# ");
                            if (1UL<ratio)
                                f(LOG_INFO, log_arg, "%s: %s %s %s*%lu %s "
                                    "timing test: adverse input prepared",
                                    prog, pcc, pfunc, typename, ratio,ptest);
                            else
                                f(LOG_INFO, log_arg, "%s: %s %s %s %s "
                                    "timing test: adverse input prepared",
                                    prog, pcc, pfunc, typename, ptest);
                            fflush(stderr);
                        }
                        /* refarray should now contain an appropriate adverse sequence */
                    break;
                    default :
                        /* do nothing here */
                    break;
                }
                /* types */
                for (type=ls_bit_number(t=types); type<32U; type=ls_bit_number(t)) {
                    t &= ~(0x01<<type);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: type=%u, t=0x%x */\n",
                            __func__,type,t);
#endif
                    typename=type_name(type);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: typename=%s */\n",__func__,typename);
#endif
                    if (0U!=flags[';']) {
                        pv=global_larray;;
                        size=sizeof(long);
                        alignsize=type_alignment(DATA_TYPE_LONG);
                    } else {
                        pv=type_array(type);
                        size=ratio*type_size(type);
                        alignsize=type_alignment(type);
                    }
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: pv=%p, size=%lu, "
                            "alignsize=%lu */\n",__func__,pv,size,alignsize);
#endif
                    size_ratio=size/alignsize;
                    max_val=type_max(type);
                    compar=type_comparator(type,flags);
#if DEBUG_CODE
                    if (DEBUGGING(SORT_SELECT_DEBUG))
                        (V)fprintf(stderr,"/* %s: compar=%s */\n",__func__,
                            comparator_name(compar));
#endif
                    compar_s=type_context_comparator(type,flags);
                    if (0U==flags[':']) {
                        if (0U!=flags['K']) fprintf(stderr, "# ");
                        if (NULL!=f) {
                            if (1UL<ratio)
                                f(LOG_INFO,log_arg,
                                    "%s: %s %s %s*%lu %s timing test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ratio,ptest,n,count);
                            else
                                f(LOG_INFO,log_arg,
                                    "%s: %s %s %s %s timing test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ptest,n,count);
                        } else {
                            if (1UL<ratio)
                                (V)fprintf(stderr,
                                    "%s: %s %s %s*%lu %s timing test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ratio,ptest,n,count);
                            else
                                (V)fprintf(stderr,
                                    "%s: %s %s %s %s timing test: %lu %lu:",
                                    prog,pcc,pfunc,typename,ptest,n,count);
                        }
                        fflush(stderr);
                    }
                    c = snul(buf1,sizeof(buf1),NULL,NULL,count,10,'0',1,f,
                        log_arg);
                    if (0U != flags['i']) reset_counters(1U);
                    for (m=*pdn=0UL; m<count; m++) {
                        /* Generate randomized, type-dependent, count-dependent,
                           function-independent sequence data here.
                           Permutations and combinations are
                           function-independent and type-independent but
                           count-dependent.
                           Worst-case sequence is type-dependent (because of
                           size_ratio)
                           Invariant sequences are copied from refarray.
                        */
                        /* preparation */
                        switch (sequence) {
                            case TEST_SEQUENCE_COMBINATIONS :
                                /* test number */
                                if (NULL!=fp) {
                                    i = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                                        (int)n, f, log_arg);
                                    (V)fprintf(fp,"%s %2lu%% %3lu%%",buf,
                                        ((m+1UL)%cycle)*100UL/cycle,(m+1UL)*100UL/count);
                                    fflush(fp);
                                }
                                /* combinations of 0, 1 elements for this test */
                                for (ct=0UL; ct<nc; ct++) {
                                    global_refarray[ct] = (long)(((m%cycle) >> (nc-ct-1UL)) & 0x01UL);
                                }
                            break;
                            case TEST_SEQUENCE_PERMUTATIONS :
                                /* test number */
                                if (NULL!=fp) {
                                    i=snprintf(buf2,sizeof(buf2),"%lu",cycle);
                                    (V)snul(buf,sizeof(buf),NULL,NULL,m%cycle,10,' ',
                                        i+1,f,log_arg);
                                    (V)fprintf(fp,"%s/%s %2lu%% %2lu%%",buf,buf2,
                                        (m%cycle)*100UL/cycle,m*100UL/count);
                                    fflush(fp);
                                }
                                if (0UL==m%cycle) {
                                    /* initial array is sorted sequence */
                                    i = generate_long_test_array(global_refarray, n,
                                        TEST_SEQUENCE_SORTED, 1L, max_val, f, log_arg);
                                    if (0 > i) {
                                        (V)fprintf(stderr,
                                            "%s: %s line %d: generate_long_test_array returned %d\n",
                                            __func__, source_file, __LINE__, i);
                                        return ++errs;
                                    } else if (0 < i) {
                                        return ++errs;
                                    }
                                    /* initialize for permutations */
                                    for (ct=nc; 0UL<ct; carray[--ct]=0UL) ;
                                } else {
                                    permute(global_refarray,0UL,u,carray,&ct);
                                }
                            break;
                            case TEST_SEQUENCE_ADVERSARY :
                                if ((1UL<m)&&(NULL!=fp)) {
                                    (V)snul(buf,sizeof(buf),NULL,NULL,m+1UL,10,
                                       ' ',
                                        c,f,log_arg);
                                    (V)fprintf(fp," %s/%s",buf,buf1);
                                    fflush(fp);
                                }
                            break;
                            case TEST_SEQUENCE_WORST :
                                if (1UL<m) {
                                    if (NULL!=fp) {
                                        (V)snul(buf,sizeof(buf),NULL,NULL,m+1UL,10,
                                           ' ',
                                            c,f,log_arg);
                                        (V)fprintf(fp," %s/%s",buf,buf1);
                                        fflush(fp);
                                    }
                                } else {
                                    (V)generate_long_test_array(global_refarray,
                                        n,TEST_SEQUENCE_SORTED,1UL,max_val,f,
                                        log_arg);
	                            if (0==method) {
                                        if (0U==(options&(QUICKSELECT_RESTRICT_RANK))) {
                                            if (0U!=(options&(QUICKSELECT_STABLE)))
                                                method = QUICKSELECT_PIVOT_REMEDIAN_SAMPLES ;
                                            else
                                                method = QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES ;
                                        } else
                                            method = QUICKSELECT_PIVOT_MEDIAN_OF_SAMPLES ;
                                    }
                                    make_adverse(global_refarray,0UL,n,karray,
                                        0UL,nk,ratio*size_ratio,0U,method,options);
                                }
                            break;
                            default :
                                switch (sequence_is_randomized(sequence)) {
                                    case 0U : /* not randomized; restore test data */
                                        /* computed, non-randomized sequences never change */
                                        /* nothing to do here; sequence is prepared and will be duplicated */
                                    break;
                                    case 1U : /* randomized; generate new sequence */
                                        /* generate new test sequence */
                                        i=generate_long_test_array(global_refarray,
                                            n, sequence, 1L, max_val, f, log_arg);
                                        if (0 > i) {
                                            (V)fprintf(stderr,
                                                "%s: %s line %d: generate_long_test_array returned %d\n",
                                                __func__, source_file, __LINE__, i);
                                            return ++errs;
                                        } else if (0 < i) {
                                            return errs;
                                        }
                                    break;
                                    default : /* error */
                                    return ++errs;
                                }
                                if ((1UL<m)&&(NULL!=fp)) {
                                    (V)snul(buf,sizeof(buf),NULL,NULL,m+1UL,10,
                                        ' ',c,f,log_arg);
                                    (V)fprintf(fp," %s/%s",buf,buf1);
                                    fflush(fp);
                                }
                            break;
                        } /* sequence switch */
                        /* copy test sequence to typed array */
                        if (0U!=flags[';']) {
                            write_database_files(global_refarray,n,type);
                            initialize_indices(n);
                        } else {
                            duplicate_test_data(global_refarray,pv,type,ratio,0UL,n);
                        }
                        /* run test */
                        if (0U != flags['i']) reset_counters(0U);
                        test_s = test_u = test_w = 0.0;
                        (V)getrusage(RUSAGE_SELF,&rusage_start);
                        (V)clock_gettime(CLOCK_REALTIME,&timespec_start);
                        switch (function) {
                            case FUNCTION_QSELECT_S :
                                if (0U!=flags['i'])
                                    D_QSEL_S(pv,0UL,u,size,compar_s,karray,0UL,nk,0U);
                                else
                                    QSEL_S(pv,0UL,u,size,compar_s,karray,0UL,nk,0U);
                            break;
                            case FUNCTION_QSELECT_SORT :
                                if (0U!=flags['i'])
                                    D_QSEL(pv,0UL,u,size,compar,NULL,0UL,nk,0U);
                                else
                                    QSEL(pv,0UL,u,size,compar,NULL,0UL,nk,0U);
                            break;
                            case FUNCTION_QSELECT :
                                if (0U!=flags['i'])
                                    D_QSEL(pv,0UL,u,size,compar,karray,0UL,nk,0U);
                                else
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
                            case FUNCTION_ILLUMOSQSORT :
                                ILLUMOSQSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_QSORT :
                                QSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
                            break;
                            case FUNCTION_QSORT_WRAPPER :
                                A(1UL<n); A(u+1UL==n);
                                if (0U == flags['i'])
                                    QSORT_FUNCTION_NAME((char *)pv,n,size,compar);
				else
                                    d_qsort((char *)pv,n,size,compar);
                            break;
                            case FUNCTION_RUNSORT :
                                RUNSORT(pv,0UL,u,size,compar,NULL,0UL,0UL,0U);
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
                        } /* function switch */
                        (V)getrusage(RUSAGE_SELF,&rusage_end);
                        (V)clock_gettime(CLOCK_REALTIME,&timespec_end);
                        switch (sequence) {
                            case TEST_SEQUENCE_COMBINATIONS : /*FALLTHROUGH*/
                            case TEST_SEQUENCE_PERMUTATIONS :
                                if (NULL!=fp) { fputc('\r', fp); fflush(fp); }
                            break;
                            default:
                                if ((1UL<m)&&(NULL!=fp)) {
                                    (V)fprintf(fp," %3lu%%\r",
                                        (m+1UL)*100UL/count);
                                    fflush(fp);
                                }
                            break;
                        }
                        /* Update test data best/worst values and running totals. */
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
                        if (0UL<count) {
                            A(*pdn<count);
                            global_uarray[*pdn] = test_u;
                            global_sarray[*pdn] = test_s;
                            global_warray[*pdn] = test_w;
                            (*pdn)++;
                        }
                        update_counters(ratio*size_ratio);
                        if (0U < errs) break;
                        if ((tot_w > timeout)&&(m+1UL<count)) {
                            count = m; /* horrible performance; break out of loop */
                            if (NULL!= f) {
                                if (1UL<ratio)
                                    f(LOG_INFO, log_arg,
                                        "%s: %s %s %s*%lu %s timing test "
                                        "terminated early: %lu/%lu",
                                        prog,pcc,pfunc,typename,ratio,ptest,
                                        m,ocount);
                                else
                                    f(LOG_INFO, log_arg,
                                        "%s: %s %s %s %s timing test "
                                        "terminated early: %lu/%lu",
                                        prog,pcc,pfunc,typename,ptest,
                                        m,ocount);
                                fflush(stderr);
                            }
                        }
                    } /* count loop */
                    /* cleanup */
                    switch (sequence) {
                        case TEST_SEQUENCE_PERMUTATIONS :
                            if (NULL!=fp) {
                                i=snprintf(buf2,sizeof(buf2),"%lu",cycle);
                                (V)snul(buf,sizeof(buf),NULL,NULL,m%cycle,10,' ',
                                    i+1,f,log_arg);
                                (V)fprintf(fp,"%s/%s %2lu%% %2lu%%",buf,buf2,
                                    (m%cycle)*100UL/cycle,m*100UL/count);
                                fflush(fp);
                            }
                            if (NULL!=fp) { fputc('\n', fp); fflush(fp); }
                            ocount=count; /* avoid "terminated early" warning */
                        break;
                        case TEST_SEQUENCE_COMBINATIONS :
                            if (NULL!=fp) {
                                i = snul(buf, sizeof(buf), NULL, NULL, m%cycle, 2, '0',
                                    (int)n, f, log_arg);
                                (V)fprintf(fp, "%s", buf);
                                fflush(fp);
                            }
                            if (NULL!=fp) { fputc('\n', fp); fflush(fp); }
                            ocount=count; /* avoid "terminated early" warning */
                        break;
                    }
                    /* If not in silent mode and if there were no errors, output
                       timing summary (including box plot times if requested).  Then
                       output summary of operation counts.  Finally, output the test
                       sequence if requested.
                    */
                    if ((0U==flags[':'])&&(0U==errs)) {
                        /* summarize test results */
                        if (NULL!=f) {
                            /* Timestamped and logged progress report. */
                            if (0U!=flags['K']) fprintf(stderr, "# ");
                            if (1UL<ratio)
                                f(LOG_INFO,log_arg,
                                    "%s: %s %s %s*%lu %s timing test: %lu %lu "
                                    "completed: summarizing results",prog,pcc,
                                    pfunc,typename,ratio,ptest,n,count);
                            else
                                f(LOG_INFO,log_arg,
                                    "%s: %s %s %s %s timing test: %lu %lu completed: "
                                    "summarizing results",prog,pcc,pfunc,
                                    typename,ptest,n,count);
                            fflush(stderr);
                        }
                        /* Heading on stdout (in case stderr isn't captured), always
                           commented.
                        */
                        if (1UL<ratio)
                            (V)printf(
                                "# %s: %s %s %s*%lu %s timing test: %lu %lu\n",
                                prog,pcc,pfunc,typename,ratio,ptest,n,count);
                        else
                            (V)printf("# %s: %s %s %s %s timing test: %lu %lu\n",
                                prog,pcc,pfunc,typename,ptest,n,count);
                        /* Comment non-sequence output if test sequence is printed. */
                        if (0U != flags['K']) comment="#";
                        /* Comparison count from antiqsort, if used. */
                        switch (sequence) {
                            case TEST_SEQUENCE_ADVERSARY :
                                i = snul(buf24, sizeof(buf24), NULL,
                                    " total comparisons", antiqsort_ncmp, 10, '0',
                                    1, f, log_arg);
                                i = snul(buf2, sizeof(buf2), NULL, " solid",
                                    (unsigned long)antiqsort_nsolid, 10,
                                    '0', 1, f, log_arg);
                                i = 1 + snprintf(buf3, sizeof(buf3),
                                    "%s %s vs. %s:", pcc, pfunc, ptest);
                                (V)printf("%s%s%*.*s%s, %s: %.6G %s\n",
                                    comment, buf3, col-i, col-i, " ", buf24,
                                    buf2, (double)antiqsort_ncmp / factor,
                                    psize);
                            break;
                        }
                        /* count partitions, etc. before computing statistics */
                        update_totals(count,f,log_arg);
                        /* find order statistics for times */
                        if (0UL<*pdn) { /* timing results available? */
                            /* Order statistic ranks array (marray) may be modified by
                               quickselect (sorting, duplicate removal), array b remains
                               unmodified.  12 order statistics (minimum, 2%, 9%, 10%,
                               25%, lower median, upper median, 75%, 90%, 91%, 98%,
                               maximum) are computed to support a variety of box plot
                               types.
                            */
                            b[0] = marray[0] = 0UL;                  /* min */
                            b[1] = marray[1] = *pdn/50UL;            /* 2%ile */
                            b[2] = marray[2] = *pdn/11UL;            /* 9%ile */
                            b[3] = marray[3] = *pdn/10UL;            /* 10%ile */
                            b[4] = marray[4] = (*pdn)>>2;            /* 1st quartile */
                            b[5] = marray[5] = (*pdn-1UL)>>1;        /* lower median */
                            b[6] = marray[6] = (*pdn>>1);            /* upper median */
                            b[7] = marray[7] = *pdn-1UL-((*pdn)>>2); /* 3rd quartile */
                            b[8] = marray[8] = *pdn-1UL-(*pdn/10UL); /* 90%ile */
                            b[9] = marray[9] = *pdn-1UL-(*pdn/11UL); /* 91%ile */
                            b[10] = marray[10] = *pdn-1UL-(*pdn/50UL); /* 98%ile */
                            b[11] = marray[11] = *pdn-1UL;             /* max */
                            /* internal quickselect w/ uninstrumented comparison function */
                            /* N.B. partitions, merges, rotations, recursions may be counted */
                            inst = instrumented; instrumented=0U;
#if ASSERT_CODE
                            /* Initially, b and marray are identical. */
                            for (w=0UL; w<12UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                            x=npartitions,y=nrepivot;
                            quickselect((void *)global_uarray,*pdn,sizeof(float),
                                floatcmp,marray,12UL,0U);
#if ASSERT_CODE
                            /* With marray in increasing (sorted) order, marray should
                               be the same after quickselect as it was before.
                            */
                            for (w=0UL; w<12UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                            quickselect((void *)global_sarray,*pdn,sizeof(float),
                                floatcmp,marray,12UL,0U);
#if ASSERT_CODE
                            /* With marray in increasing (sorted) order, marray should
                               be the same after quickselect as it was before.
                            */
                            for (w=0UL; w<12UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                            quickselect((void *)global_warray,*pdn,sizeof(float),
                                floatcmp,marray,12UL,0U);
                            npartitions=x,nrepivot=y;
#if ASSERT_CODE
                            /* With marray in increasing (sorted) order, marray should
                               be the same after quickselect as it was before.
                            */
                            for (w=0UL; w<12UL; w++) A(b[w]==marray[w]);
#endif /* ASSERT_CODE */
                            instrumented = inst;
#if DEBUG_CODE
                            if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: quickselect returned */\n",
                                __func__, source_file,__LINE__);
#endif /* DEBUG_CODE */
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
                            i = 1 + snprintf(buf3, sizeof(buf3),
                                "%s %s of %lu %s%s: %s: (%lu * %lu)",
                                pcc, pfunc, n, typename, (n==1)?"":"s", ptest, n,
                                count);
                            (V)sng(buf4, sizeof(buf4), NULL, NULL,
                                tot_u, -4, 3, f, log_arg); /* total user */
                            (V)sng(buf15, sizeof(buf15), NULL, NULL,
                                tot_s, -4, 3, f, log_arg); /* total system */
                            (V)sng(buf16, sizeof(buf16), NULL, NULL,
                                tot_w, -4, 3, f, log_arg); /* total wall-clock */
                            test_u = (global_uarray[b[5]] + global_uarray[b[6]]) / 2.0;
                            (V)sng(buf2, sizeof(buf2), NULL, NULL,
                                test_u / factor, -4, 3, f, log_arg); /* scaled median user */
                            (V)sng(buf5, sizeof(buf5), NULL, NULL,
                                test_u, -4, 3, f, log_arg); /* median user */
                            (V)sng(buf17, sizeof(buf17), NULL, NULL,
                                global_uarray[b[0]], -4, 3, f, log_arg); /* min user */
                            (V)sng(buf18, sizeof(buf18), NULL, NULL,
                                global_uarray[b[11]], -4, 3, f, log_arg); /* max user */
                            test_s = (global_sarray[b[5]] + global_sarray[b[6]]) / 2.0;
                            (V)sng(buf11, sizeof(buf11), NULL, NULL,
                                test_s, -4, 3, f, log_arg); /* median system */
                            (V)sng(buf12, sizeof(buf12), NULL, NULL,
                                test_s / factor, -4, 3, f, log_arg); /* scaled median system */
                            (V)sng(buf19, sizeof(buf19), NULL, NULL,
                                global_sarray[b[0]], -4, 3, f, log_arg); /* min sys */
                            (V)sng(buf20, sizeof(buf20), NULL, NULL,
                                global_sarray[b[11]], -4, 3, f, log_arg); /* max sys */
                            test_w = (global_warray[b[5]] + global_warray[b[6]]) / 2.0;
                            (V)sng(buf13, sizeof(buf13), NULL, NULL,
                                test_w, -4, 3, f, log_arg); /* median wall-clock */
                            (V)sng(buf14, sizeof(buf14), NULL, NULL,
                                test_w / factor, -4, 3, f, log_arg); /* scaled median wall-clock */
                            (V)sng(buf21, sizeof(buf21), NULL, NULL,
                                global_warray[b[0]], -4, 3, f, log_arg); /* min wall */
                            (V)sng(buf22, sizeof(buf22), NULL, NULL,
                                global_warray[b[11]], -4, 3, f, log_arg); /* max wall */
#if DEBUG_CODE
                            if (DEBUGGING(SORT_SELECT_DEBUG)) (V)fprintf(stderr,
                                "/* %s: %s line %d: factor=%G, count=%lu, dn=%lu, median=%G (%s) */\n",
                                __func__, source_file,__LINE__, factor, count, *pdn, test_u, buf2);
#endif /* DEBUG_CODE */
                            if (0U!=flags['B']) {
                                /* box plot data:
                                   2%, 9%, 1/4, median, 3/4, 91%, 98%
                                   min, 2%, 9%, 10%, 1/4, median, 3/4, 90%, 91%, 98%, max
                                */
                                (V)printf(
                                    "%s%s %s %s %s user-time box plot: %s",
                                    comment,pcc,pfunc,ptest,typename,buf17); /*min*/
                                for (w=1UL; w<5UL; w++) {
                                    (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                        global_uarray[b[w]], -4, 3, f, log_arg);
                                    (V)printf(" %s",buf23);
                                }
                                (V)printf(" %s",buf5); /* median (b[5]+b[6])/2 */
                                for (w=7UL; w<11UL; w++) {
                                    (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                        global_uarray[b[w]], -4, 3, f, log_arg);
                                    (V)printf(" %s",buf23);
                                }
                                (V)printf(" %s\n",buf18); /* max */
                                (V)printf(
                                    "%s%s %s %s %s system-time box plot: %s",
                                    comment,pcc,pfunc,ptest,typename,buf19); /*min*/
                                for (w=1UL; w<5UL; w++) {
                                    (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                        global_sarray[b[w]], -4, 3, f, log_arg);
                                    (V)printf(" %s",buf23);
                                }
                                (V)printf(" %s",buf11); /* median (b[5]+b[6])/2 */
                                for (w=7UL; w<11UL; w++) {
                                    (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                        global_sarray[b[w]], -4, 3, f, log_arg);
                                    (V)printf(" %s",buf23);
                                }
                                (V)printf(" %s\n",buf20); /* max */
                                (V)printf(
                                    "%s%s %s %s %s wall-clock box plot: %s",
                                    comment,pcc,pfunc,ptest,typename,buf21); /*min*/
                                for (w=1UL; w<5UL; w++) {
                                    (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                        global_warray[b[w]], -4, 3, f, log_arg);
                                    (V)printf(" %s",buf23);
                                }
                                (V)printf(" %s",buf13); /* median (b[5]+b[6])/2 */
                                for (w=7UL; w<11UL; w++) {
                                    (V)sng(buf23, sizeof(buf23), NULL, NULL,
                                        global_warray[b[w]], -4, 3, f, log_arg);
                                    (V)printf(" %s",buf23);
                                }
                                (V)printf(" %s\n",buf22); /* max */
                            }
                            if (0U!=flags['R']) { /* Terse timing output. */
                                i = 1 + snprintf(buf3, sizeof(buf3), "%lu", n);
                                (V)printf("%s%s%*.*s%s %s user seconds per %s, %s total %s\n",
                                    comment, buf3, col-i, col-i, " ",
                                    0.0!=test_u?buf2:buf,
                                    0.0!=test_u?"median":"mean",
                                    psize,
                                    0.0!=test_u?"median":"mean",
                                    0.0!=test_u?buf5:buf6);
                                (V)printf("%s%s%*.*s%s %s system seconds per %s, %s total %s\n",
                                    comment, buf3, col-i, col-i, " ",
                                    0.0!=test_s?buf12:buf7,
                                    0.0!=test_u?"median":"mean",
                                    psize,
                                    0.0!=test_u?"median":"mean",
                                    0.0!=test_s?buf11:buf8);
                                (V)printf("%s%s%*.*s%s %s wall-clock seconds per %s, %s total %s\n",
                                    comment, buf3, col-i, col-i, " ",
                                    0.0!=test_w?buf14:buf9,
                                    0.0!=test_u?"median":"mean",
                                    psize,
                                    0.0!=test_u?"median":"mean",
                                    0.0!=test_w?buf13:buf10);
                            } else { /* Verbose with multiple statistics. */
                                (V)printf(
                                    "%s%s%*.*snormalized unit user times: %s (mean), %s (median), per %s, total user time %s%s, mean %s, median %s\n",
                                    comment, buf3, col-i, col-i, " ", buf, buf2,
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
                                    comment, buf3, col-i, col-i, " ", buf7, buf12,
                                    psize, buf15,
                                    (count!=ocount)?" (terminated early)":"",
                                    buf8, buf11);
                                (V)printf(
                                    "%s%s%*.*snormalized unit wall clock times: %s (mean), %s (median), per %s, total wall clock time %s%s, mean %s, median %s\n",
                                    comment, buf3, col-i, col-i, " ", buf9, buf14,
                                    psize, buf16,
                                    (count!=ocount)?" (terminated early)":"",
                                    buf10, buf13);
                                /* Compare mean and median times, warn if there is
                                   a large discrepancy in user or wall-clock times.
                                   Timing is expected to have a skewed distribution
                                   due to system activity and cache misses.  However,
                                   a huge discrepancy probably warrants investigation
                                   and/or re-running the test to avoid contaminated
                                   timing data.
                                */
                                d = tot_w / test_w / (double)m; /* ratio of mean to median */
                                if (d<1.0) d = 1.0/d; /* make ratio > 1 */
                                if (d > DISCREPANCY_THRESHOLD) {
                                    (V)fprintf(stderr,
                                        "%s%s %s Wall-clock time discrepancy: min %s, mean %s, max %s, median %s: %0.2f%%\n",
                                        comment,pcc,ptest,buf21,buf10,buf22,buf13,
                                        100.0*(d-1.0));
                                    fflush(stderr);
                                } /* Large mean vs. median discrepancy. */
                                switch (sequence) {
                                    case TEST_SEQUENCE_PERMUTATIONS : /*FALLTHROUGH*/
                                    case TEST_SEQUENCE_COMBINATIONS :
                                        (V)snprintf(buf5, sizeof(buf5),
                                            " s/ %s %s\n", psize, typename);
                                        (V)sng(buf, sizeof(buf), NULL, buf5,
                                            best_u / factor, -4, 3, f, log_arg);
                                        i = 1 + snprintf(buf2, sizeof(buf2),
                                            "%s %s of %lu %s%s: %s: %s(%lu)",
                                            pcc, pfunc, n, typename, (n==1)?"":"s",
                                            ptest, "best", best_m);
                                        (V)printf("%s%s%*.*s%s", comment, buf2,
                                            col-i, col-i, " ", buf);
                                        (V)sng(buf, sizeof(buf), NULL, buf5,
                                            worst_u / factor, -4, 3, f, log_arg);
                                        i = 1 + snprintf(buf2, sizeof(buf2),
                                            "%s %s of %lu %s%s: %s: %s(%lu)", pcc,
                                            pfunc, n, typename, (n==1)?"":"s",
                                            ptest, "worst", worst_m);
                                        (V)printf("%s%s%*.*s%s", comment, buf2,
                                            col-i, col-i, " ", buf);
                                    break;
                                }
                            } /* Terse or verbose timing output. */
                        } /* timing results */
                        /* If requested, scale and output summary of operation counts;
                           raw counters are not used (they may have been affected by
                           selection of order statistics), only the local totals are
                           used in computations.
                        */
                        if (0U!=flags['i']) { /* Instruments operations counts. */
                            print_counters(comment,col,n,count,factor,pcc,pfunc,psize,
                                typename,ptest,flags,size_ratio,f,log_arg);
                        } /* instrumented operations */
                    } /* test result summary if not silent and if no errors */
                } /* types loop */
                /* cleanup */
                if (NULL!=fp) { fclose(fp); fp=NULL; }
                if (NULL != karray) {
#if DEBUG_CODE
                    if (DEBUGGING(MEMORY_DEBUG)) (V)fprintf(stderr,
                        "/* %s: %s line %d: freeing karray @ %p */\n",
                        __func__,source_file,__LINE__,(void *)karray);
#endif
                    free(karray);
                    karray = NULL;
                }
                /* Output test sequence if requested. */
                if (0U != flags['K']) {
                    switch (sequence) {
                        case TEST_SEQUENCE_ADVERSARY :
                            (V)printf("#Adversary sequence:\n");
                            for(j=0UL; j<n; j++) {
                                (V)printf("%ld\n", global_refarray[j]);
                            }
                        break;
                    }
                } /* test sequence output */
            } /* functions loop */
            /* Output test sequence if requested. */
            if (0U != flags['K']) {
                switch (sequence) {
                    case TEST_SEQUENCE_DUAL_PIVOT_KILLER :
                        (V)printf("#Dual-pivot killer sequence:\n");
                        for(j=0UL; j<n; j++) {
                            (V)printf("%ld\n", global_refarray[j]);
                        }
                    break;
                    case TEST_SEQUENCE_MEDIAN3KILLER :
                        (V)printf("#Median-of-3 killer sequence:\n");
                        for(j=0UL; j<n; j++) {
                            (V)printf("%ld\n", global_refarray[j]);
                        }
                    break;
                    case TEST_SEQUENCE_WORST :
                        (V)printf("#quickselect killer sequence:\n");
                        for(j=0UL; j<n; j++) {
                            (V)printf("%ld\n", global_refarray[j]);
                        }
                    break;
                }
            } /* test sequence output */
        } /* no errors */
    } /* sequences loop */
    fflush(stdout);
    return errs;
}
