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
* $Id: ~|^` @(#)    partition.c copyright 2016-2017 Bruce Lilly.   \ partition.c $
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
/* $Id: ~|^` @(#)   This is partition.c version 1.7 dated 2018-01-19T23:02:59Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.partition.c */

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
#define ID_STRING_PREFIX "$Id: partition.c ~|^` @(#)"
#define SOURCE_MODULE "partition.c"
#define MODULE_VERSION "1.7"
#define MODULE_DATE "2018-01-19T23:02:59Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if 0 /* unused: for make_partition */
/* Force integer comparison result to one of -1, 0, or 1. */
static QUICKSELECT_INLINE int ternary_value(int c)
{
    if (0 > c) return -1;
    else if (0 < c) return 1;
    return 0;
}
#endif

#if 0 /* find a run of elements that constitute a partition (which can then be
         merged with other partitions.  Merging is relatively expensive for
         random sequences (about double the cost of Kiwiel Algorithm L in terms
         of swaps, for non-stable merge).  Works well for input patterns, though
         (all-equal, already-sorted, bitonic, rotated).
      */
/* make_partition:
   On entry, the comparison result (forced to ternary value {-1,0,1}) of
   some element to the pivot is c.  If c is -1, that element is at index *pgt;
   if 1 the element's index is *plt, and if zero, the element is at index *peq.
   Elements are examined from the next element up to (but not including) index
   *pbeyond so long as they form a partition, i.e. elements form a sequence of
   contiguous (possibly empty) groups of less than, equal to, and greater than
   values compared to the pivot, in that order.  When an element that has been
   examined fails to belong to the current partition, *pbeyond is set to its
   index and its comparison value (forced to ternary value) is returned. If all
   elements are part of the partition, a value outside of the possible ternary
   values is returned. The starting indices of equal to and greater than regions
   are placed in *peq and *pgt, respectively.
*/
#include <stdio.h>
static QUICKSELECT_INLINE int make_partition(int c, char *base, size_t *plt,
    size_t *peq, size_t *pgt, size_t *pbeyond, size_t size, char *pivot,
    int(*compar)(const void*,const void*), unsigned int options)
{
    int d=-2;
    size_t i, beyond=*pbeyond;
    char *p, *pu=base+size*beyond;

    if ((((SIZE_MAX)>>1)<beyond)||(base>=pu)||(DEBUGGING(PARTITION_DEBUG)))
        (V)fprintf(stderr,"/* %s: %s line %d: c=%d, beyond=%lu, pu=%p, pivot@%p[%lu] */\n",
           __func__,source_file,__LINE__,c,beyond,pu,pivot,(pivot-base)/size);
    if (0>c) i=*plt,*peq=*pgt=i;
    else if (0<c) i=*pgt,*plt=*peq=i;
    else i=*peq,*plt=*pgt=i;
    if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
        for (++i,p=base+size*i; p<pu; i++,p+=size) {
            d=ternary_value(compar(p,pivot));
            if (d==c) continue;
            else if (d<c) {
                if (0==c) *pgt=i;
                *pbeyond=i;
                return d;
            } /* else d>c */
            if (0>c) *peq=i;
            *pgt=i;
            c=d;
        }
    } else { /* indirect */
        pivot=*((char **)pivot);
        for (++i,p=base+size*i; p<pu; i++,p+=size) {
            d=ternary_value(compar(*((char **)p),pivot));
            if (d==c) continue;
            else if (d<c) {
                if (0==c) *pgt=i;
                *pbeyond=i;
                return d;
            } /* else d>c */
            if (0>c) *peq=i;
            *pgt=i;
            c=d;
        }
    }
if ((i!=beyond)||(p!=pu)) (V)fprintf(stderr,"/* %s: %s line %d: i=%lu, beyond=%lu, p=%p, pu=%p */\n",__func__,source_file,__LINE__,i,beyond,p,pu);
    A(i==beyond);
    A(p==pu);
    if (0>c) *peq=i;
    if (0>=c) *pgt=i;
    if (DEBUGGING(PARTITION_DEBUG))
        (V)fprintf(stderr,
           "/* %s: %s line %d: c=%d, d=%d, i=%lu, *plt=%lu, *peq=%lu, *pgt=%lu */\n",
           __func__,source_file,__LINE__,c,d,i,*plt,*peq,*pgt);
    return 2;
}
#endif

/* merge 2 adjacent canonical partitions */
/* +-----------------------------+
   | L< | L= | L> | R< | R= | R> |
   +-----------------------------+
    |    |    |    |    |    |   |
    (l)  eq1  gt1  mid  eq2  gt2  (u)
     |    \       X       /    |
     |     \     / \     /     |
     |      \   /   \   /      |
     |       \ /     \ /       |
     |        X       X        |
     v       / \     / \       v
   +-----------------------------+
   | L< | R< | L= | R= | L> | R> |
   +-----------------------------+
*/
/* Supports both non-stable merge using blockmoves and stable serge using
   rotations.
*/
QUICKSELECT_INLINE
void merge_partitions(char *base, size_t first, size_t eq1, size_t gt1,
    size_t mid, size_t eq2, size_t gt2, size_t beyond, size_t size,
    void (*swapf)(char*, char*, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options, size_t *peq, size_t *pgt)
{
    register size_t leq=gt1-eq1, req=gt2-eq2, rlt=eq2-mid;
    size_t lgt=mid-gt1, m=(lgt<rlt?lgt:rlt); /* minimum of lgt, rlt */
    /* +-----------------------+
       | < | = | > | < | = | > |
       +-----------------------+
        |   |   |   |   |   |   |
        (l) eq1 gt1 mid eq2 gt2  (u)
    */
#if ASSERT_CODE + DEBUG_CODE
    if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, mid=%lu, eq1=%lu, gt1=%lu: left partition: leq=%lu, lgt=%lu */\n",
            __func__,source_file,__LINE__,first,mid,eq1,gt1,leq,lgt);
            print_some_array(base,first,mid-1UL, "/* "," */",options);
        (V)fprintf(stderr,
            "/* %s: %s line %d: mid=%lu, beyond=%lu, eq2=%lu, gt2=%lu: right partition: req=%lu, rgt=%lu, nsw=%lu */\n",
            __func__,source_file,__LINE__,mid,beyond,eq2,gt2,req,rlt,nsw);
            print_some_array(base,mid,beyond-1UL, "/* "," */",options);
    }
#endif
    A(first<mid);A(mid<beyond);
    A(first<=eq1);A(eq1<=gt1);A(gt1<=mid);A(mid<=eq2);A(eq2<=gt2);A(gt2<=beyond);
    /* +-----------------------------+ */
    /* | L< | L= | L> | R< | R= | R> | */
    /* +-----------------------------+ */
    /* (l)  eq1  gt1  mid  eq2  gt2  (u) */
    switch (options&(QUICKSELECT_STABLE)) {
        default :
            /* +-----------------------------+
               | L< | L= | L> | R< | R= | R> |
               +-----------------------------+
                     a    b    c    d    e
               A series of 3 blockmoves merges the partitions:
               1.             X
               +-----------------------------+
               | L< | L= | R< | L> | R= | R> |
               +-----------------------------+
               2&3.      X     c   X
               +-----------------------------+
               | L< | R< | L= | R= | L> | R> |
               +-----------------------------+
                          b    c    d
               N.B. blockmoves do not preserve order within blocks.
            */
            {
                char *pa, *pb, *pc, *pd, *pe;
                pa=base+size*eq1, pb=base+size*gt1, pc=base+size*mid,
                    pd=base+size*eq2, pe=base+size*gt2;
                pc=blockmove(pb,pc,pd,swapf);
                pb=blockmove(pa,pb,pc,swapf);
                pd=blockmove(pc,pd,pe,swapf);
                *peq=(pb-base)/size, *pgt=(pd-base)/size;
            }
        break;
        case QUICKSELECT_STABLE :
            /* +-----------------------------+
               | L< | L= | L> | R< | R= | R> |
               +-----------------------------+
              Preservation of ordering within blocks and between blocks of like
              types results in stable partitoning, stable sort or selection.
              3 methods each consisting of 2 rotations may be used:
              1a. rotate L= through R< by 1 block
               +-----------------------------+
               | L< | R< | L= | L> | R= | R> |
               +-----------------------------+
              1b. exchange L> and R=
               +-----------------------------+
               | L< | R< | L= | R= | L> | R> |
               +-----------------------------+
              2a. rotate L> through R= by 1 block
               +-----------------------------+
               | L< | L= | R< | R= | L> | R> |
               +-----------------------------+
              2b. exchange L= and R<
               +-----------------------------+
               | L< | R< | L= | R= | L> | R> |
               +-----------------------------+
              3a. rotated L= through R= by 2 blocks
               +-----------------------------+
               | L< | R< | R= | L= | R> | R> |
               +-----------------------------+
              3b. exchange R= and L=
               +-----------------------------+
               | L< | R< | L= | R= | L> | R> |
               +-----------------------------+
              One method is chosen depending on the region sizes so as to
              minimize the total number of swaps.
            */
            if ((0UL==leq)||(0UL==req)||(leq+req<m)) { /* method 3 */
#if ASSERT_CODE + DEBUG_CODE
                if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                        print_some_array(base,first,beyond-1UL, "/* "," */",options);
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: method 3a: rotate eq1|mid|gt2: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                }
#endif
                irotate(base,eq1,mid,gt2,size,swapf,alignsize,size_ratio);
                /* +-----------------------------+ */
                /* | L< | R< | R= | L= | R> | R> | */
                /* +-----------------------------+ */
                /* (l)  eq1  gt1  mid  eq2  gt2  (u) */
                if (0UL<leq+req) {
                    gt1=eq1+rlt, mid=gt1+req, eq2=mid+leq;
#if ASSERT_CODE + DEBUG_CODE
                    if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                        (V)fprintf(stderr,
                            "/* %s: %s line %d: method 3b: exchange = blocks by rotation gt1|mid|eq2: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                            __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                            print_some_array(base,first,beyond-1UL, "/* "," */",options);
                    }
#endif
                    irotate(base,gt1,mid,eq2,size,swapf,alignsize,size_ratio);
                }
            } else if (lgt<rlt) { /* method 2 */
#if ASSERT_CODE + DEBUG_CODE
                if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                        print_some_array(base,first,beyond-1UL, "/* "," */",options);
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: method 2a: rotate gt1|mid|gt2: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                }
#endif
                irotate(base,gt1,mid,gt2,size,swapf,alignsize,size_ratio);
                /* +-----------------------------+ */
                /* | L< | L= | R< | R= | L> | R> | */
                /* +-----------------------------+ */
                /* (l)  eq1  gt1  mid  eq2  gt2  (u) */
                mid=gt1+rlt, eq2=mid+req;
#if ASSERT_CODE + DEBUG_CODE
                if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: method 2b: exchange L=,R< by rotating eq1|gt1|mid first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                        print_some_array(base,first,beyond-1UL, "/* "," */",options);
                }
#endif
                irotate(base,eq1,gt1,mid,size,swapf,alignsize,size_ratio);
            } else { /* method 1 */
                /* +-----------------------------+ */
                /* | L< | L= | L> | R< | R= | R> | */
                /* +-----------------------------+ */
                /* (l)  eq1  gt1  mid  eq2  gt2  (u) */
#if ASSERT_CODE + DEBUG_CODE
                if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                        print_some_array(base,first,beyond-1UL, "/* "," */",options);
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: method 1a: rotate eq1|mid|eq2: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                }
#endif
                irotate(base,eq1,mid,eq2,size,swapf,alignsize,size_ratio);
                /* +-----------------------------+ */
                /* | L< | R< | L= | L> | R= | R> | */
                /* +-----------------------------+ */
                /* (l)  eq1  gt1  mid  eq2  gt2  (u) */
                gt1=eq1+rlt, mid=gt1+leq;
#if ASSERT_CODE + DEBUG_CODE
                if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                    (V)fprintf(stderr,
                        "/* %s: %s line %d: method 1b: exchange L>,R= by rotating mid|eq2|gt2: first=%lu, beyond=%lu, eq1=%lu, gt1=%lu, mid=%lu, eq2=%lu, gt2=%lu */\n",
                        __func__,source_file,__LINE__,first,beyond,eq1,gt1,mid,eq2,gt2);
                        print_some_array(base,first,beyond-1UL, "/* "," */",options);
                }
#endif
                irotate(base,mid,eq2,gt2,size,swapf,alignsize,size_ratio);
            }
            /* +-----------------------------+ */
            /* | L< | R< | L= | R= | L> | R> | */
            /* +-----------------------------+ */
            /* (l)  eq1  gt1  mid  eq2  gt2  (u) */
            *peq=gt1=eq1+rlt, mid=gt1+leq, *pgt=eq2=mid+req;
        break;
    }
#if ASSERT_CODE + DEBUG_CODE
    if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, beyond=%lu, *peq=%lu, *pgt=%lu: merged partition, nsw=%lu */\n",
            __func__,source_file,__LINE__,first,beyond,*peq,*pgt,nsw);
            print_some_array(base,first,beyond-1UL, "/* "," */",options);
    }
#endif
}

/* array partitioning */
/* Partitioning based on Bentley & McIlroy's split-end partition, as
   modified by Kiwiel algorithm L and with provision to avoid redundant
   comparisons of already-partitioned elements as described by McGeoch &
   Tygar.
*/
/* handle partially partitioned array */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void d_partition(char *base, size_t first, size_t beyond, char *pc, char *pd,
    register char *pivot, char *pe, char *pf, register size_t size,
    int(*compar)(const void*,const void*), void (*swapf)(char*, char*, size_t),
    size_t alignsize, size_t size_ratio, unsigned int options, char *conditions,
    size_t *indices, char *element, size_t *peq, size_t *pgt)
{
    char *pa, *ph, *pl, *pu;
    register char *pb, *pg;
    register int c=0, d=0;
#if ASSERT_CODE + DEBUG_CODE
# if 0
    size_t max_ratio=((pc+size<pf)?3UL:beyond-first);
# endif
    size_t ratio=0UL; /* max_ratio computed here presumes median-of-medians pivot selection */
#endif

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
    if (DEBUGGING(REPARTITION_DEBUG)||DEBUGGING(PARTITION_DEBUG)) {
        (V)fprintf(stderr,
            "/* %s: %s line %d: first=%lu, pc@%lu, pd@%lu, pivot@%lu, pe@%lu, pf@%lu, beyond=%lu, options=0x%x */\n",
            __func__,source_file,__LINE__,first,(pc-base)/size,(pd-base)/size,
            (pivot-base)/size,(pe-base)/size,(pf-base)/size,beyond,options);
        print_some_array(base,first,beyond-1UL, "/* "," */",options);
    }
#endif
    npartitions++;
    switch (options&(QUICKSELECT_STABLE)) {
        default:
#if 1 /* 0 to test divide-and-conquer partition or partition by merging mini-partitions */
            /* Kiwiel Algorithm L, based on Bentley & McIlroy's split-end
               partition.
            */
            /* McGeoch & Tygar suggest that partial partition information
               from median-of-medians might be used to avoid recomparisons
               during repartitioning.
            */
#if ASSERT_CODE
/* verify proper partitioning of already-partitioned region */
 {
size_t s, t, u, v, w, x, y, z;
A(pc>=base);
A(pd>=base);
A(pe>=base);
A(pf>=base);
s=(pc-base)/size;
t=(pd-base)/size;
u=(pe-base)/size-1UL;
v=(pf-base)/size-1UL;
x=nlt, y=neq, z=ngt;
w=test_array_partition(base,s,t,u,v,size,compar,options,NULL,NULL);
nlt=x, neq=y, ngt=z;
if (w!=t) {
(V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, pc@%lu, pd@%lu, pe@%lu, pf@%lu, size=%lu: bad partition */\n",
__func__,source_file,__LINE__,first,beyond,s,t,u+1UL,v+1UL,(unsigned long)size);
print_some_array(base,s,v, "/* "," */",options);
A(0==1);
}
}
#endif
            /* +-----------------------------------------------------+ */
            /* |   =   |   <   |  ?  : < :=: > :  ?  |   >   |   =   | */
            /* +-----------------------------------------------------+ */
            /*  pl      a       b     c   d e   f   G g       h       u*/
            pa=pb=pl=base+size*first, pg=(ph=pu=base+size*beyond)-size;
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
logfile=start_log("log");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/* |        ?           : < :=: > :            ?         | */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/*  pl                   c   d e   f                      u*/\n");
fprintf(logfile,
"/*  b->                                               <-g h*/\n");
fprintf(logfile,
"/*  a                                                      */\n");
fprintf(logfile,
"/*  c                                                      */\n");
fprintf(logfile,
"/* %s: %s line %d: pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
            if (pc+size==pf) { /* pc==pd&&pd+size==pf (pivot only) */
                A(pl<=pc);A(pl<=pd);A(pe<=pu); /* reasonable bounds, pivot in region */
                if (pivot-pl<pu-pivot) { /* pivot @ middle or closer to first element */
                    if (pl!=pivot) { ph=pg;
                        EXCHANGE_SWAP(swapf,ph,pivot,size,alignsize,size_ratio,nsw++);
                        pg=(pivot=ph)-size;
                    }
                    else pa+=size, pb=pa;
                } else { /* pivot is closer to last element */
                    if (pg!=pivot) {
                        EXCHANGE_SWAP(swapf,pl,pivot,size,alignsize,size_ratio,nsw++);
                        pa=pb=(pivot=pl)+size;
                    } else pivot=ph=pg, pg-=size;
                }
                A((pivot==pl)||((pivot==ph)&&(ph+size==pu)));
                A((pl<pa)||(ph<pu)); /* must have pivot somewhere */
                A(pa==pb);A(pg+size==ph);
            } else {
                /* Rearrange blocks for split-end partition */
                A(pc<=pd);A(pd<pe);
                pd=blockmove(pc,pd,pe,swapf);
                pivot=pc;
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* rearranged <,= blocks in pivot partition */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/* |         ?        :=: < : > :            ?           | */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/*  pl                 c d   e   f                        u*/\n");
fprintf(logfile,
"/*  a                                                     h*/\n");
fprintf(logfile,
"/*  b->                                               <-g  */\n");
fprintf(logfile,
"/* %s: %s line %d: pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                if (0U!=save_partial) {
                    A(pe<=pf);A(pf<=ph);
                    pg=blockmove(pe,pf,ph,swapf)-size; /* N.B. -size */
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* rearranged >,? blocks for split-end partition */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/* |          ?        :=: < :        ?              | > | */\n");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/*  pl                  c d   e                   <-g     u*/\n");
fprintf(logfile,"/*  a                                                     h*/\n");
fprintf(logfile,"/*  b                                                      */\n");
fprintf(logfile, "/* %s: %s line %d: pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                    A(pl<=pc);A(pc<=pd);
#if 1 /* general case: already-partitioned region somewhere */
                    /* swap = block to far left; N.B. for pl==pc, this sets pa=pd */
                    pa=blockmove(pl,pc,pd,swapf);
                    pivot=pl;
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,"\n/* rearranged ?,= blocks for split-end partition */\n");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/* |=:           ?        : < :          ?           | > | */\n");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/*  pl                     d   e                  <-g     u*/\n");
fprintf(logfile,"/*  b a                                                   h*/\n");
fprintf(logfile, "/* %s: %s line %d: pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
#else /* specific to already-partitioned region at far left */
                    A(pl==pc);
                    pa=pd;
#endif
                    A(pa<=pd);A(pd<=pe);
#if 1 /* general case: already-partitioned region somewhere */
                    /* swap already-partitioned < leftward; N.B. for pa==pd, pb=pe */
                    pb=blockmove(pa,pd,pe,swapf);
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,"\n/* rearranged ?,< blocks for split-end partition */\n");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/* | =| < |                  ?                       | > | */\n");
fprintf(logfile,"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,"/*  pl a   b->                                    <-g     u*/\n");
fprintf(logfile,"/*                                                        h*/\n");
fprintf(logfile, "/* %s: %s line %d: pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
#else /* specific to already-partitioned region at far left */
                    A(pa==pd);
                    pb=pe;
#endif
                    if (pa>pl) pivot=pa-size; else pivot=pl;
                } else {
                    pa=pb=pd; /* already-partitioned < */
                    pivot=pa-size;
                }
            }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* rearranged blocks for split-end partition */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/* | = | <  |                ?                       | > | */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/*  pl  a    b->                                  <-g     u*/\n");
fprintf(logfile,
"/*                                                        h*/\n");
fprintf(logfile,
"/* %s: %s line %d: pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
#if QUICKSELECT_INDIRECT
            if (0U==(options&(QUICKSELECT_INDIRECT))) {
#endif /* QUICKSELECT_INDIRECT */
                if (pa==pb) {
                    while ((pb<=pg)&&(0==(c=compar(pivot,pb)))) { pb+=size; }
                    pa=pb;
                    if (0<c)
                        for (pb+=size; (pb<=pg)&&(0<=(c=compar(pivot,pb))); pb+=size)
                            if (0==c) { A(pa!=pb);
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                            }
                } else {
                    for (; (pb<=pg)&&(0<=(c=compar(pivot,pb))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
                }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pb over less-than and equals at left */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                if (pg==ph-size) {
                    while ((pb<pg)&&(0==(d=compar(pivot,pg)))) { pg-=size; }
                    ph=pg+size;
                    if (0>d)
                        for (pg-=size; (pb<pg)&&(0>=(d=compar(pivot,pg))); pg-=size)
                            if (0==d) { A(pg!=ph-size);
                                ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                            }
                } else {
                    for (; (pb<pg)&&(0>=(d=compar(pivot,pg))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
                }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pg over greater-than and equals at right */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                while (pb<pg) {
                    EXCHANGE_SWAP(swapf,pb,pg,size,alignsize,size_ratio,nsw++); pb+=size, pg-=size;
#if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* swapped pb,pg and updated pointers */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
#endif
                    for (;(pb<=pg)&&(0<=(c=compar(pivot,pb))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pb over less-than and equals at left */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                    for (;(pb<pg)&&(0>=(d=compar(pivot,pg))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pg over greater-than and equals at right */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* pb>=pg */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/* | = |            <           |          >         | = | */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/*  pl  a                      g b                    h   u*/\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
#if QUICKSELECT_INDIRECT
            } else { /* indirect */
                pivot=*((char **)pivot);
                if (pa==pb) {
                    while ((pb<=pg)&&(0==(c=compar(pivot,*((char **)pb))))) { pb+=size; }
                    pa=pb;
                    if (0<c)
                        for (pb+=size; (pb<=pg)&&(0<=(c=compar(pivot,*((char **)pb)))); pb+=size)
                            if (0==c) { A(pa!=pb);
                                EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                            }
                } else {
                    for (; (pb<=pg)&&(0<=(c=compar(pivot,*((char **)pb)))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
                }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pb over less-than and equals at left */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                if (pg==ph-size) {
                    while ((pb<pg)&&(0==(d=compar(pivot,*((char **)pg))))) { pg-=size; }
                    ph=pg+size;
                    if (0>d)
                        for (pg-=size; (pb<pg)&&(0>=(d=compar(pivot,*((char **)pg)))); pg-=size)
                            if (0==d) { A(pg!=ph-size);
                                ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                            }
                } else {
                    for (; (pb<pg)&&(0>=(d=compar(pivot,*((char **)pg)))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
                }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pg over greater-than and equals at right */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                while (pb<pg) {
                    EXCHANGE_SWAP(swapf,pb,pg,size,alignsize,size_ratio,nsw++); pb+=size, pg-=size;
#if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* swapped pb,pg and updated pointers */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
#endif
                    for (;(pb<=pg)&&(0<=(c=compar(pivot,*((char **)pb)))); pb+=size)
                        if (0==c) { A(pa!=pb);
                            EXCHANGE_SWAP(swapf,pa,pb,size,alignsize,size_ratio,nsw++); pa+=size;
                        }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pb over less-than and equals at left */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                    for (;(pb<pg)&&(0>=(d=compar(pivot,*((char **)pg)))); pg-=size)
                        if (0==d) { A(pg!=ph-size);
                            ph-=size; EXCHANGE_SWAP(swapf,pg,ph,size,alignsize,size_ratio,nsw++);
                        }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* skipped pg over greater-than and equals at right */\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
                }
# if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* pb>=pg */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/* | = |            <           |          >         | = | */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/*  pl  a                      g b                    h   u*/\n");
fprintf(logfile,
"/* %s: %s line %d: c=%d, d=%d, pl=%lu, pa=%lu, pb=%lu, pc=%lu, pd=%lu, pe=%lu, pf=%lu, pg=%lu, ph=%lu, pu=%lu, pivot=%lu */\n",
__func__,source_file,__LINE__,c,d,(pl-base)/size,(pa-base)/size,(pb-base)/size,
(pc-base)/size,(pd-base)/size,(pe-base)/size,(pf-base)/size,(pg-base)/size,
(ph-base)/size,(pu-base)/size,(pivot-base)/size);
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
# endif
            }
#endif /* QUICKSELECT_INDIRECT */
            if (pb>=pa) pd=blockmove(pl,pa,pb,swapf); else pd=pb;
            pe=blockmove(pb,ph,pu,swapf);
#if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
fprintf(logfile,
"\n/* blockmoves to restore canonical partition */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/* |            <         |      =    |        >         | */\n");
fprintf(logfile,
"/* +-----------------------------------------------------+ */\n");
fprintf(logfile,
"/*  pl                     d           e                  u*/\n");
fprintf(logfile,
"/* %s: %s line %d: pl=%lu, pd(pivot)=%lu, pe=%lu, pu=%lu: pd-pl=%lu, pe-pd=%lu, pu-pe=%lu, ratio=%lu */\n",
__func__,source_file,__LINE__,(pl-base)/size,(pd-base)/size,(pe-base)/size,
(pu-base)/size,(pd-pl)/size,(pe-pd)/size,(pu-pe)/size,
ratio=((pd-pl>pu-pe)?(pd-pl)/(pu-pd):(pu-pe)/(pe-pl)));
fprint_some_array(logfile,(double *)base,first,beyond-1UL, "/* "," */",options);
}
if ((pe<=pd)||(pd<pl)) {
    (V)fprintf(stderr,
       "/* %s: %s line %d: pb=%p[%lu], pd=%p[%lu], pe=%p[%lu], pg=%p[%lu], pl=%p[%lu], pa=%p[%lu], ph=%p[%lu], pu=%p[%lu] */\n",
       __func__,source_file,__LINE__,
       (void *)pb,(pb-base)/size,
       (void *)pd,(pd-base)/size,
       (void *)pe,(pe-base)/size,
       (void *)pg,(pg-base)/size,
       (void *)pl,(pl-base)/size,
       (void *)pa,(pa-base)/size,
       (void *)ph,(ph-base)/size,
       (void *)pu,(pu-base)/size
    );
    print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#endif
            A(pl<=pd); A(pd<pe); A(pe<=pu);
            *peq=(pd-base)/size;
            *pgt=(pe-base)/size;
#if DEBUG_CODE
if (DEBUGGING(REPARTITION_DEBUG)) {
(V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, pd@%lu, pe@%lu */\n",
__func__,source_file,__LINE__,first,beyond,*peq,*pgt);
    print_some_array(base,first,beyond-1UL, "/* "," */",options);
}
#endif
#if ASSERT_CODE + DEBUG_CODE
if ((DEBUGGING(REPARTITION_DEBUG))&&(1000UL>beyond-first)) {
/* verify proper partitioning */
size_t s, t, u, v, w, x, y, z;
    close_log(logfile);
s=(pl-base)/size;
t=(pd-base)/size;
u=(pe-base)/size-1UL;
v=(pu-base)/size-1UL;
x=nlt, y=neq, z=ngt;
w=test_array_partition(base,s,t,u,v,size,compar,options,NULL,NULL);
nlt=x, neq=y, ngt=z;
if (w!=t) {
(V)fprintf(stderr,
"/* %s: %s line %d: first=%lu, beyond=%lu, pl@%lu, pd@%lu, pe@%lu, pu@%lu: bad partition@%lu */\n",
__func__,source_file,__LINE__,first,beyond,s,t,u+1UL,v+1UL,w);
    print_some_array(base,first,beyond-1UL, "/* "," */",options);
(void)rename("log", "log_ng");
A(0==1);
}
# if 0 /* this only applies to median-of-medians pivot selection */
    /* verify reasonable split */
    if (max_ratio<=ratio) { (V)fprintf(stderr,
        "/* %s: %s line %d: ratio=%lu>=%lu */\n",
        __func__,source_file,__LINE__,ratio,max_ratio); (void)rename("log", "log_ng"); A(0==1); }
# endif
}
#endif
            A(pe>pd);
#else
            /* Non-stable partition by merging runs of elements constituting
               partitions w/o initial pivot element movement.  The merges are
               relatively expensive (about 2x swaps of Kiwiel L) for random
               input sequences.
            */
            {
                size_t b1, b2, b3, lt1, lt2, lt3, eq1, eq2, eq3, gt1, gt2, gt3;

                pa=base+first*size, pu=base+beyond*size;
                eq1=(pd-base)/size, gt1=(pe-base)/size;
            /* make a partition */
                if (pa<pc) {
                    c=ternary_value(compar(pa,pivot));
                    lt3=(pc-base)/size, eq3=eq1, gt3=gt1, b3=(pf-base)/size;
                    lt1=eq1=gt1=first, b1=lt3;
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    c=make_partition(c,base,&lt1,&eq1,&gt1,&b1,size,pivot,compar,options);
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    while (b1<lt3) {
                        lt2=eq2=gt2=b1,b2=lt3;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=lt3=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,lt3);
                        c=make_partition(c,base,&lt2,&eq2,&gt2,&b2,size,pivot,compar,options);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,b2);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt2,eq2,gt2,b2);
                        merge_partitions(base,lt1,eq1,gt1,lt2,eq2,gt2,b2,size,swapf,alignsize,size_ratio,options,&eq1,&gt1);
                        b1=b2;
                        if (eq1<gt1) pivot=base+size*eq1;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    }
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt3=%lu, eq3=%lu, gt3=%lu, b3=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt3,eq3,gt3,b3);
                    merge_partitions(base,lt1,eq1,gt1,lt3,eq3,gt3,b3,size,swapf,alignsize,size_ratio,options,&eq1,&gt1);
                    b1=b3;
                    if (eq1<gt1) pivot=base+size*eq1;
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                } else {
                    lt1=(pc-base)/size, b1=(pf-base)/size;
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt2,eq2,gt2,b2);
                }
                if (pf<pu) {
                    c=ternary_value(compar(pf,pivot));
                    while (b1<beyond) {
                        lt2=eq2=gt2=b1, b2=beyond;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,b2);
                        c=make_partition(c,base,&lt2,&eq2,&gt2,&b2,size,pivot,compar,options);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,b2);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt2,eq2,gt2,b2);
                        merge_partitions(base,lt1,eq1,gt1,lt2,eq2,gt2,b2,size,swapf,alignsize,size_ratio,options,&eq1,&gt1);
                        b1=b2;
                        if (eq1<gt1) pivot=base+size*eq1;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    }
                }
                *peq=eq1, *pgt=gt1;
                return;
            /* while elements below pivot, make another partition and merge */
            /* merge already-partitioned region (w/ pivot) and region to its left */
            /* while there are elements to the right, make partitions and merge */
            }
#endif
        break;
#if QUICKSELECT_STABLE /* not used if QUICKSELECT_STABLE is defined as zero */
        case QUICKSELECT_STABLE :
# if QUICKSELECT_LINEAR_STABLE
            if ((NULL!=conditions)&&(NULL!=indices)) {
                /* linear-time stable partition using O(N) extra space */
                register size_t i, ieq, igt;

                pl=base+size*first, pu=base+size*beyond;
                /* 1st scan: left-to-right to get element partial order and set
                   pointer for < elements.
                */
                /* split into pre-pivot, pivot, post-pivot */
                /* further split into direct, indirect for pivot cache */
#  if QUICKSELECT_INDIRECT
                if (0U==(options&(QUICKSELECT_INDIRECT))) { /* direct */
#  endif /* QUICKSELECT_INDIRECT */
                    for (i=ieq=first,pb=pl,pg=pivot; pb<pg; pb+=size,i++) {
                        c=COMPAR(pb,pg,/**/);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
                    A(pb==pg);
                    conditions[i]=(char)0;
                    for (++i,pb+=size; pb<pu; pb+=size,i++) {
                        c=COMPAR(pb,pg,/**/);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
#  if QUICKSELECT_INDIRECT
                } else { /* indirect; pivot data pointer cached */
                    pg=*((char **)pivot);
                    for (i=ieq=first,pb=pl; pb<pivot; pb+=size,i++) {
                        c=COMPAR(*((char **)pb),pg,/**/);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
                    A(pb==pivot);
                    conditions[i]=(char)0;
                    for (++i,pb+=size; pb<pu; pb+=size,i++) {
                        c=COMPAR(*((char **)pb),pg,/**/);
                        if (0<c) conditions[i]=(char)1;
                        else if (0==c) conditions[i]=(char)0;
                        else {
                            conditions[i]=(char)(-1);
                            indices[ieq++]=i;
                        }
                    }
                }
#  endif /* QUICKSELECT_INDIRECT */
                *peq=ieq;
                /* 2nd scan: right-to-left to set pointers for > elements. */
                for (igt=beyond,i=igt-1UL,pb=pu-size; pb>=pl; pb-=size,i--)
                    if (0<conditions[i]) indices[--igt]=i;
                *pgt=igt;
                /* 3rd scan: left to right to set pointers for == elements. */
                for (i=first,pb=pl; ieq<igt; pb+=size,i++) {
                    if (0==conditions[i]) indices[ieq++]=i;
                    A(pb<pu);A(i<beyond);
                }
                i=rearrange_array(base,beyond-first,size,indices,
                    beyond-first,first,beyond,element);
                A(beyond-first>(i>>1));
                /* Update count of moves: for direct data sort/select,
                   size_ratio reflects data size compared to alignsize (which
                   is assumed to be the basic move size used by memcpy); for
                   indirect sort/select, size_ratio==1 because only pointers
                   are moved here.
                */
                if (0UL!=i)
                    nmoves+=(i+beyond-first)*size_ratio;
            } else {
# endif /* QUICKSELECT_LINEAR_STABLE */
#if 0 /* Stable in-place partition by stable merge of runs of elements
         constituting partitions.  Merges are expensive for random input
         sequences.
      */
           {
                size_t b1, b2, b3, lt1, lt2, lt3, eq1, eq2, eq3, gt1, gt2, gt3;

                pa=base+first*size, pu=base+beyond*size;
                eq1=(pd-base)/size, gt1=(pe-base)/size;
            /* make a partition */
                if (pa<pc) {
                    c=ternary_value(compar(pa,pivot));
                    lt3=(pc-base)/size, eq3=eq1, gt3=gt1, b3=(pf-base)/size;
                    lt1=eq1=gt1=first, b1=lt3;
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    c=make_partition(c,base,&lt1,&eq1,&gt1,&b1,size,pivot,compar,options);
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    while (b1<lt3) {
                        lt2=eq2=gt2=b1,b2=lt3;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=lt3=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,lt3);
                        c=make_partition(c,base,&lt2,&eq2,&gt2,&b2,size,pivot,compar,options);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,b2);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt2,eq2,gt2,b2);
                        merge_partitions(base,lt1,eq1,gt1,lt2,eq2,gt2,b2,size,swapf,alignsize,size_ratio,options,&eq1,&gt1);
                        b1=b2;
                        if (eq1<gt1) pivot=base+size*eq1;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    }
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt3=%lu, eq3=%lu, gt3=%lu, b3=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt3,eq3,gt3,b3);
                    merge_partitions(base,lt1,eq1,gt1,lt3,eq3,gt3,b3,size,swapf,alignsize,size_ratio,options,&eq1,&gt1);
                    b1=b3;
                    if (eq1<gt1) pivot=base+size*eq1;
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                } else {
                    lt1=(pc-base)/size, b1=(pf-base)/size;
                    if (DEBUGGING(PARTITION_DEBUG))
                        (V)fprintf(stderr,
                           "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                           __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt2,eq2,gt2,b2);
                }
                if (pf<pu) {
                    c=ternary_value(compar(pf,pivot));
                    while (b1<beyond) {
                        lt2=eq2=gt2=b1, b2=beyond;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,b2);
                        c=make_partition(c,base,&lt2,&eq2,&gt2,&b2,size,pivot,compar,options);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt2,eq2,gt2,b2);
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu, lt2=%lu, eq2=%lu, gt2=%lu, b2=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1,lt2,eq2,gt2,b2);
                        merge_partitions(base,lt1,eq1,gt1,lt2,eq2,gt2,b2,size,swapf,alignsize,size_ratio,options,&eq1,&gt1);
                        b1=b2;
                        if (eq1<gt1) pivot=base+size*eq1;
                        if (DEBUGGING(PARTITION_DEBUG))
                            (V)fprintf(stderr,
                               "/* %s: %s line %d: c=%d, lt1=%lu, eq1=%lu, gt1=%lu, b1=%lu */\n",
                               __func__,source_file,__LINE__,c,lt1,eq1,gt1,b1);
                    }
                }
                *peq=eq1, *pgt=gt1;
                return;
            /* while elements below pivot, make another partition and merge */
            /* merge already-partitioned region (w/ pivot) and region to its left */
            /* while there are elements to the right, make partitions and merge */
            }
#else
            /* divide-and-conquer partition */
                pl=base+size*first, pu=base+size*beyond;

                /* Recursively partitions unpartitioned regions down to a partition
                   containing a single element, then stably merges partitions to
                   fully partition the array.
                */
                if (pl+size==pu) { /* 1 element */
                    if (pivot!=pl) { /* external pivot */
                        c=OPT_COMPAR(pl,pivot,options,/**/);
                        if (0>c) *peq=*pgt=beyond;
                        else if (0<c) *peq=*pgt=first;
                        else *peq=first, *pgt=beyond;
                    } else *peq=first, *pgt=beyond; /* pivot */
                } else if ((pl==pc)&&(pf==pu)) { /* already fully partitioned */
                    *peq=(pd-base)/size;
                    *pgt=(pe-base)/size;
                } else { /* more than 1 element, not fully partitioned */
                    /* already-partitioned region is 1 partition; 1 or 2 others */
                    if ((pl<=pc)&&(pf<=pu)) { /* pivot is in region */
                        size_t ipc, ipd, ipe, ipf;
                        /* +-----------------------+
                           |  ?  | < | = | > |  ?  |
                           +-----------------------+
                            first c   d   e   f     beyond
                        */
                        ipc=(pc-base)/size, ipd=(pd-base)/size, ipe=(pe-base)/size,
                            ipf=(pf-base)/size;
# if ASSERT_CODE + DEBUG_CODE
                        if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                            (V)fprintf(stderr,
                                "/* %s: %s line %d: first=%lu, ipc=%lu, ipd=%lu, ipe=%lu, ipf=%lu, beyond=%lu */\n",
                                __func__,source_file,__LINE__,first,ipc,ipd,ipe,ipf,
                                beyond);
                        }
# endif
                        if (pl==pc) { /* already-partitioned at left end */
                            A(pf<pu);
                            /* partition the right unpartitioned region and merge */
                            d_partition(base,ipf,beyond,pc,pd,pivot,pe,pf,size,
                                compar,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                            A(*peq<=*pgt);A(ipf<=*peq);A(*pgt<=beyond);
                            merge_partitions(base,first,ipd,ipe,ipf,*peq,*pgt,
                                beyond,size,swapf,alignsize,size_ratio,options,peq,
                                pgt);
                        } else if (pu==pf) { /* already-partitioned at right end */
                            /* partition unpartitioned left region and merge */
                            A(pl<pc);
                            d_partition(base,first,ipc,pc,pd,pivot,pe,pf,size,
                                compar,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                            A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipc);
                            merge_partitions(base,first,*peq,*pgt,ipc,ipd,ipe,
                                beyond,size,swapf,alignsize,size_ratio,options,peq,
                                pgt);
                        } else { /* already-partitioned in middle */
                            /* partition unpartitioned regions and merge */
                            size_t eq2, gt2;
                            A(pl<pc);
                            d_partition(base,first,ipc,pc,pd,pivot,pe,pf,size,
                                compar,swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                            A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipc);
                            merge_partitions(base,first,*peq,*pgt,ipc,ipd,ipe,ipf,
                                size,swapf,alignsize,size_ratio,options,peq,pgt);
                            A(*peq<=*pgt);A(first<=*peq);A(*pgt<=ipf);
                            A(pf<pu);
                            d_partition(base,ipf,beyond,pl,base+*peq*size,
                                base+*peq*size,base+*pgt*size,pf,size,compar,swapf,
                                alignsize,size_ratio,options,
                                conditions,indices,element,&eq2,&gt2);
                            A(eq2<=gt2);A(ipf<=eq2);A(gt2<=beyond);
                            merge_partitions(base,first,*peq,*pgt,ipf,eq2,gt2,
                                beyond,size,swapf,alignsize,size_ratio,options,peq,
                                pgt);
                        }
                    } else { /* external pivot; split, partition, & merge */
                        /* External pivot arises when partitioning a part of an
                           unpartitioned sub-array.
                        */
                        size_t mid=first+((beyond-first)>>1), eq2, gt2;
# if ASSERT_CODE + DEBUG_CODE
                        if ((DEBUGGING(PARTITION_DEBUG))&&(1000UL>beyond-first)) {
                            (V)fprintf(stderr,
                                "/* %s: %s line %d: first=%lu, mid=%lu, eq2=%lu, gt2=%lu, beyond=%lu */\n",
                                __func__,source_file,__LINE__,first,mid,eq2,gt2,
                                beyond);
                        }
# endif
                        d_partition(base,first,mid,pc,pd,pivot,pe,pf,size,compar,
                                swapf,alignsize,size_ratio,options,
                                conditions,indices,element,peq,pgt);
                        A(first<mid);A(mid<beyond);
                        d_partition(base,mid,beyond,pc,pd,pivot,pe,pf,size,compar,
                                swapf,alignsize,size_ratio,options,
                                conditions,indices,element,&eq2,&gt2);
                        merge_partitions(base,first,*peq,*pgt,mid,eq2,gt2,beyond,
                                size,swapf,alignsize,size_ratio,options,peq,pgt);
                    }
                }
#endif
# if QUICKSELECT_LINEAR_STABLE
            }
# endif /* QUICKSELECT_LINEAR_STABLE */
        break;
#endif /* QUICKSELECT_STABLE */
    }
}
