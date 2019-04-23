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
* $Id: ~|^` @(#)    illumos.c modifications copyright 2019 Bruce Lilly.   \ illumos.c $
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
/* $Id: ~|^` @(#)   This is illumos.c version 1.11 dated 2019-04-18T02:02:40Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.illumos.c */

/********************** Long description and rationale: ***********************
* Illumos qsort implementation for testing/comparison.
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: illumos.c ~|^` @(#)"
#define SOURCE_MODULE "illumos.c"
#define MODULE_VERSION "1.11"
#define MODULE_DATE "2019-04-18T02:02:40Z"
#define COPYRIGHT_HOLDER "Sun Microsystems, Inc"
#define COPYRIGHT_DATE "2008"

/********** BL **********/
#define __STDC_WANT_LIB_EXT1__ 0
#define LIBMEDIAN_TEST_CODE 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#include "dedicated_sort_src.h"

#if QUICKSELECT_BUILD_FOR_SPEED
# include "pivot_src.h"
#endif

#include "insertion_sort_src.h" /* isort_bs */

extern size_t quickselect_cache_size; /* BL */
/********** ** **********/

/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"@(#)illumos.c	1.11	2019-04-18 SMI"

#if !defined(_KERNEL) && !defined(_KMDB)
# if 0 /* non-standard header file excluded BL */
#include "lint.h"
# endif
#endif /* !_KERNEL && !_KMDB */

#include <sys/types.h>

#if !defined(_KERNEL) && !defined(_KMDB)
#include <stdlib.h>
# if 0 /* non-standard header file excluded BL */
#include <synch.h>
# endif /* non-standard synch.h */
#endif /* !_KERNEL && !_KMDB */

#if 0 /* interpolated inline BL */
#include "qsort.h"
#else
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_QSORT_H
#define	_QSORT_H

#pragma ident	"@(#)illumos.c	1.11	2019-04-18 SMI"

/*
 * Declarations for qsort().
 */

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>


/* name change BL */
#if 0 /* moved to median_test_config.h BL */
extern void illumos_qsort(void *, size_t, size_t, int (*)(const void *, const void *));
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* _QSORT_H */
#endif

static void swapp32(uint32_t *r1, uint32_t *r2, size_t cnt);
static void swapp64(uint64_t *r1, uint64_t *r2, size_t cnt);
static void swapi(uint32_t *r1, uint32_t *r2, size_t cnt);
static void swapb(char *r1, char *r2, size_t cnt);

/*
 * choose a median of 3 values
 *
 * note: cstyle specifically prohibits nested conditional operators
 * but this is the only way to do the median of 3 function in-line
 */
#define	med3(a, b, c) (cmp((a), (b)) < 0) \
	? ((cmp((b), (c)) < 0) ? (b) : (cmp((a), (c)) < 0) ? (c) : (a)) \
	: ((cmp((b), (c)) > 0) ? (b) : (cmp((a), (c)) > 0) ? (c) : (a))

#define	THRESH_L	5	/* threshold for insertion sort */
#define	THRESH_M3	20	/* threshold for median of 3 */
#define	THRESH_M9	50	/* threshold for median of 9 */

typedef struct {
	char	*b_lim;
	size_t	nrec;
} stk_t;

/*
 * qsort() is a general purpose, in-place sorting routine using a
 * user provided call back function for comparisons.  This implementation
 * utilizes a ternary quicksort algorithm, and cuts over to an
 * insertion sort for partitions involving fewer than THRESH_L records.
 *
 * Potential User Errors
 *   There is no return value from qsort, this function has no method
 *   of alerting the user that a sort did not work or could not work.
 *   We do not print an error message or exit the process or thread,
 *   Even if we can detect an error, We CANNOT silently return without
 *   sorting the data, if we did so the user could never be sure the
 *   sort completed successfully.
 *   It is possible we could change the return value of sort from void
 *   to int and return success or some error codes, but this gets into
 *   standards  and compatibility issues.
 *
 *   Examples of qsort parameter errors might be
 *   1) record size (rsiz) equal to 0
 *      qsort will loop and never return.
 *   2) record size (rsiz) less than 0
 *      rsiz is unsigned, so a negative value is insanely large
 *   3) number of records (nrec) is 0
 *      This is legal - qsort will return without examining any records
 *   4) number of records (nrec) is less than 0
 *      nrec is unsigned, so a negative value is insanely large.
 *   5) nrec * rsiz > memory allocation for sort array
 *      a segment violation may occur
 *      corruption of other memory may occur
 *   6) The base address of the sort array is invalid
 *      a segment violation may occur
 *      corruption of other memory may occur
 *   7) The user call back function is invalid
 *      we may get alignment errors or segment violations
 *      we may jump into never-never land
 *
 *   Some less obvious errors might be
 *   8) The user compare function is not comparing correctly
 *   9) The user compare function modifies the data records
 */

/* name change BL */
void
illumos_qsort(
	void		*basep,
	size_t		nrec,
	size_t		rsiz,
	int		(*cmp)(const void *, const void *),
        unsigned int    options) /* BL */
{
	size_t		i;		/* temporary variable */

	/* variables used by swap */
	void		(*swapf)(char *, char *, size_t);
	size_t		loops;

	/* variables used by sort */
	stk_t		stack[8 * sizeof (nrec) + 1];
	stk_t		*sp;
	char		*b_lim;		/* bottom limit */
	char		*b_dup;		/* bottom duplicate */
	char		*b_par;		/* bottom partition */
	char		*t_lim;		/* top limit */
	char		*t_dup;		/* top duplicate */
	char		*t_par;		/* top partition */
	char		*m1, *m2, *m3;	/* median pointers */
	uintptr_t	d_bytelength;	/* byte length of duplicate records */
	int		b_nrec;
	int		t_nrec;
	int		cv;		/* results of compare (bottom / top) */

        int             method = QUICKSELECT_PIVOT_REMEDIAN_SAMPLES; /* BL */
        char            *mid;           /* BL: middle element for med3 */
        size_t          offset;         /* BL: offset for med3 */
        size_t          alignsize;      /* BL */
        size_t          size_ratio;     /* BL */
        size_t          cache_limit;    /* BL */
        size_t          nel;            /* BL */
        size_t          lneq;           /* BL */
        size_t          lnne;           /* BL */

        /* BL: added initialization for ID strings */
        if ((char)0==file_initialized) initialize_file(__FILE__);
        /* BL: Determine cache size once on first call. */
        if (0UL==quickselect_cache_size) quickselect_cache_size = cache_size();
        options |= QUICKSELECT_NO_REPIVOT;

	/*
	 * choose a swap function based on alignment and size
	 *
	 * The qsort function sorts an array of fixed length records.
	 * We have very limited knowledge about the data record itself.
	 * It may be that the data record is in the array we are sorting
	 * or it may be that the array contains pointers or indexes to
	 * the actual data record and all that we are sorting is the indexes.
	 *
	 * The following decision will choose an optimal swap function
	 * based on the size and alignment of the data records
	 *   swapp64	will swap 64 bit pointers
	 *   swapp32	will swap 32 bit pointers
	 *   swapi	will swap an array of 32 bit integers
	 *   swapb	will swap an array of 8 bit characters
	 *
	 * swapi and swapb will also require the variable loops to be set
	 * to control the length of the array being swapped
	 */
	if ((((uintptr_t)basep & (sizeof (uint64_t) - 1)) == 0) &&
	    (rsiz == sizeof (uint64_t))) {
		loops = 1;
		swapf = (void (*)(char *, char *, size_t))swapp64;
                alignsize = rsiz; /* BL */
	} else if ((((uintptr_t)basep & (sizeof (uint32_t) - 1)) == 0) &&
	    (rsiz == sizeof (uint32_t))) {
		loops = 1;
		swapf = (void (*)(char *, char *, size_t))swapp32;
                alignsize = rsiz; /* BL */
	} else if ((((uintptr_t)basep & (sizeof (uint32_t) - 1)) == 0) &&
	    ((rsiz & (sizeof (uint32_t) - 1)) == 0)) {
		loops = rsiz / sizeof (int);
		swapf = (void (*)(char *, char *, size_t))swapi;
                alignsize = sizeof(uint32_t); /* BL */
	} else {
		loops = rsiz;
		swapf = swapb;
                alignsize = 1UL; /* BL */
	}
        size_ratio = rsiz / alignsize; /* BL */
        if (rsiz>sizeof(char*)) /* direct sorting in data cache */
            cache_limit=(quickselect_cache_size<<1)/rsiz/3UL; /* 1.5*size within cache */
        else /* indirect mergesort */
            cache_limit=quickselect_cache_size/pointer_and_a_half; /* 1.5 pointers within cache */
        nel = nrec; /* BL */

	/*
	 * qsort is a partitioning sort
	 *
	 * the stack is the bookkeeping mechanism to keep track of all
	 * the partitions.
	 *
	 * each sort pass takes one partition and sorts it into two partitions.
	 * at the top of the loop we simply take the partition on the top
	 * of the stack and sort it. See the comments at the bottom
	 * of the loop regarding which partitions to add in what order.
	 *
	 * initially put the whole partition on the stack
	 */
	sp = stack;
	sp->b_lim = (char *)basep;
	sp->nrec = nrec;
	sp++;
	while (sp > stack) {
		sp--;
		b_lim = sp->b_lim;
		nrec = sp->nrec;

/* BL " i " should perhaps be " is "? "a qsort for" -> "qsort for a"? */
		/*
		 * a linear insertion sort i faster than a qsort for
		 * very small number of records (THRESH_L)
		 *
		 * if number records < threshold use linear insertion sort
		 *
		 * this also handles the special case where the partition
yields (BL)
		 * 0 or 1 records length.
		 */

                /* BL: permit modifications (last conditional is original code) */
                if (2UL>nrec) continue; /* BL */
                if (MOD_DEDICATED_SORT==(options&(MOD_DEDICATED_SORT))) { /* BL */
                    if ((5UL>nrec) /* small sub-array; network no worse than best-case D&C */
                        /* stable and optimize comparison methods only work within cache limits */
                    ||((nrec<=cache_limit)&&
                      ((0U!=(options&(QUICKSELECT_STABLE))) /* stable methods */
                      ||(0U!=(options&(QUICKSELECT_OPTIMIZE_COMPARISONS))) /* minimize comparisons */
                      ||(
#if 1
                      (nel>nrec)&& /* XXX not useful unless nel is a parameter set by wrapper (or internal stack is used instead of recursion) */
                      /* not first partition; D&C 1st time in case constant or binary */
                      /* can only determine non-constant, non-binary, non-ternary after first partition */
#endif
                      (lneq<=(lnne>>2)))) /* not constant, binary, ternary */
                      )
                    ) {
                        int ret;
                        ret= 
#if LIBMEDIAN_TEST_CODE
                            d_dedicated_sort
#else
                            dedicated_sort
#endif
                            (basep,(b_lim-(char*)basep)/rsiz,
                            (b_lim-(char*)basep)/rsiz+nrec,rsiz,cmp,
                            swapf,alignsize,size_ratio,
                            quickselect_cache_size,0UL,options);
                        switch (ret) {
                            case 0 : /* sorting is complete */
                /* <- */    continue; /* Done; */
                            case EINVAL : /* error */
                                fprintf(stderr,
                                    "%s: %s line %d: EINVAL from d_dedicated_sort\n",
                                    __func__,source_file,__LINE__);
                            return ;
                            case EAGAIN : /* continue with divide-and-conquer */
                            break;
                            default : /* ? */
                                fprintf(stderr,
                                    "%s: %s line %d: %d from d_dedicated_sort\n",
                                    __func__,source_file,__LINE__,ret);
                            return ;
                        }
                    }
                } else if (0U!=(options&(MOD_ISORT_BS))) {
                    if (7UL>nrec) /* same cutoff but median-of-3 @ size 7 */
                               /* reversed input now same as already-sorted */
                    {
#if LIBMEDIAN_TEST_CODE
                        d_isort_bs
#else
                        isort_bs
#endif
                        (basep,(b_lim-(char*)basep)/rsiz,
                            (b_lim-(char*)basep)/rsiz+nrec,rsiz,cmp,swapf,
                            alignsize,size_ratio,options);
                        continue; /* done; else continue divide-and-conquer */
                    }
                } else if (0U!=(options&(MOD_ISORT_LS))) {
                    if (7UL>nrec) {
                        /* use median-of-3 pivot selection @ size 7 */
#if LIBMEDIAN_TEST_CODE
                        d_isort_ls
#else
                        isort_ls
#endif
                        (basep,(b_lim-(char*)basep)/rsiz,
                            (b_lim-(char*)basep)/rsiz+nrec,rsiz,cmp,swapf,
                            alignsize,size_ratio,options);
                        continue; /* done; else continue divide-and-conquer */
                    }
		} else if (nrec < THRESH_L) {
			/*
			 * Linear insertion sort
			 */
			t_par = b_lim;
			for (i = 1; i < nrec; i++) {
				t_par += rsiz;
				b_par = t_par;
				while (b_par > b_lim) {
					b_par -= rsiz;
					if ((*cmp)(b_par, b_par + rsiz) <= 0) {
						break;
					}
					(*swapf)(b_par, b_par + rsiz, loops);
				}
			}

			/*
			 * a linear insertion sort will put all records
			 * in their final position and will not create
			 * subpartitions.
			 *
			 * therefore when the insertion sort is complete
			 * just go to the top of the loop and get the
			 * next partition to sort.
			 */
			continue;
		}

		/* quicksort */

                /* BL: freeze low-address samples which will be used for pivot selection */
                if (aqcmp==cmp) {
                    nfrozen=0UL;
		    if (nrec < THRESH_M3) {
                        if (MOD_SAMPLE_QUALITY!=(options&(MOD_SAMPLE_QUALITY))) {
                            m2 = b_lim + (nrec / 2) * rsiz;
                            (V)freeze(aqindex(m2,basep,rsiz));
                            pivot_minrank=1UL;
                        }
		    } else if (MOD_SAMPLE_QUANTITY==(options&(MOD_SAMPLE_QUANTITY))) {
                        ;
		    } else if (nrec < THRESH_M9) {
                        if (MOD_SAMPLE_QUALITY!=(options&(MOD_SAMPLE_QUALITY))) {
			    i = ((nrec - 1) / 2) * rsiz;
                            (V)freeze(aqindex(b_lim,basep,rsiz));
                            (V)freeze(aqindex(b_lim+i,basep,rsiz));
                            pivot_minrank=2UL;
                        }
		    } else {
                        if (MOD_SAMPLE_QUALITY!=(options&(MOD_SAMPLE_QUALITY))) {
			    i = ((nrec - 1) / 8) * rsiz;
                            (V)freeze(aqindex(b_lim,basep,rsiz));
                            (V)freeze(aqindex(b_lim+i,basep,rsiz));
                            (V)freeze(aqindex(b_lim+3*i,basep,rsiz));
                            (V)freeze(aqindex(b_lim+4*i,basep,rsiz));
                            pivot_minrank=4UL;
                        }
                    }
                }

		/*
		 * choose a pivot record
		 *
		 * Ideally the pivot record will divide the partition
		 * into two equal parts. however we have to balance the
		 * work involved in selecting the pivot record with the
		 * expected benefit.
		 *
		 * The choice of pivot record depends on the number of
		 * records in the partition
		 *
		 * for small partitions (nrec < THRESH_M3)
		 *   we just select the record in the middle of the partition
		 *
		 * if (nrec >= THRESH_M3 && nrec < THRESH_M9)
		 *   we select three values and choose the median of 3
		 *
		 * if (nrec >= THRESH_M9)
		 *   then we use an approximate median of 9
		 *   9 records are selected and grouped in 3 groups of 3
		 *   the median of each of these 3 groups is fed into another
		 *   median of 3 decision.
		 *
		 * Each median of 3 decision is 2 or 3 compares,
		 * so median of 9 costs between 8 and 12 compares.
		 *
		 * i is byte distance between two consecutive samples
		 * m2 will point to the pivot record
		 */
		if (nrec < THRESH_M3) {
                    m2 = b_lim + (nrec / 2) * rsiz;
                    if (MOD_SAMPLE_QUALITY==(options&(MOD_SAMPLE_QUALITY))) {
                        switch (nrec) {
                            case 0UL :
                            case 1UL :
                            case 2UL :
                            case 3UL :
                            case 4UL :
                            case 6UL :
                            case 8UL :
                                /* leave pivot at [upper-]middle element */
                            break;
                            case 5UL :
                            case 7UL :
                            case 9UL :
                                m2+=rsiz; /* away from middle for bitonic */
                            break;
                            default :
                                m2+=rsiz*((nrec-1UL)/8); /* 1/2+1/8=5/8 */
                            break;
                        }
                    }
		} else if (MOD_SAMPLE_QUANTITY==(options&(MOD_SAMPLE_QUANTITY))) {
                    /* implies also TERNARY and SAMPLE_QUALITY */
                    m2=
#if LIBMEDIAN_TEST_CODE
                        d_select_pivot
#else
                        select_pivot
#endif
                        (basep,(b_lim-(char*)basep)/rsiz,
                        (b_lim-(char*)basep)/rsiz+nrec,rsiz,cmp,swapf,alignsize,
                        size_ratio,0U,NULL,0UL,0UL,quickselect_cache_size,
                        0UL,options,&m1,&m1,&m3,&m3,&method,NULL);
		} else if (nrec < THRESH_M9) {
			/* use median of 3 */
			i = ((nrec - 1) / 2) * rsiz;
                        if (MOD_TERNARY==(options&(MOD_TERNARY))) { /* BL */
                            if (MOD_SAMPLE_QUALITY==(options&(MOD_SAMPLE_QUALITY))) {
                                mid = b_lim + i;
                                offset = (nrec / 3) * rsiz;
			        m2 = d_fmed3(mid - offset, mid, mid + offset, cmp,options,basep,rsiz);
                            } else {
			        m2 = d_fmed3(b_lim, b_lim + i, b_lim + 2 * i, cmp,options,basep,rsiz);
                            }
                        } else {
                            if (MOD_SAMPLE_QUALITY==(options&(MOD_SAMPLE_QUALITY))) {
                                mid = b_lim + i;
                                offset = (nrec / 3) * rsiz;
			        m2 = med3(mid - offset, mid, mid + offset);
                            } else {
			        m2 = med3(b_lim, b_lim + i, b_lim + 2 * i);
                            }
                        }
		} else {
			/* approx median of 9 */
                        if (MOD_TERNARY==(options&(MOD_TERNARY))) { /* BL */
                            if (MOD_SAMPLE_QUALITY==(options&(MOD_SAMPLE_QUALITY))) {
                                mid = b_lim + ((nrec - 1) / 2) * rsiz;
                                offset = (nrec / 3) * rsiz;
                                i = (nrec / 9) * rsiz;
			        m1 = d_fmed3(mid - offset - i, mid - i, mid + offset - i, cmp,options,basep,rsiz);
			        m2 = d_fmed3(mid - offset, mid, mid + offset, cmp,options,basep,rsiz);
			        m3 = d_fmed3(mid - offset + i, mid + i, mid + offset + i, cmp,options,basep,rsiz);
			        m2 = d_fmed3(m1, m2, m3, cmp,options,basep,rsiz);
                            } else {
			        i = ((nrec - 1) / 8) * rsiz;
			        m1 = d_fmed3(b_lim, b_lim +  i, b_lim + 2 * i, cmp,options,basep,rsiz);
			        m2 = d_fmed3(b_lim + 3 * i, b_lim + 4 * i, b_lim + 5 * i, cmp,options,basep,rsiz);
			        m3 = d_fmed3(b_lim + 6 * i, b_lim + 7 * i, b_lim + 8 * i, cmp,options,basep,rsiz);
			        m2 = d_fmed3(m1, m2, m3, cmp,options,basep,rsiz);
                            }
                        } else {
                            if (MOD_SAMPLE_QUALITY==(options&(MOD_SAMPLE_QUALITY))) {
                                mid = b_lim + ((nrec - 1) / 2) * rsiz;
                                offset = (nrec / 3) * rsiz;
                                i = (nrec / 9) * rsiz;
			        m1 = med3(mid - offset - i, mid - i, mid + offset - i);
			        m2 = med3(mid - offset, mid, mid + offset);
			        m3 = med3(mid - offset + i, mid + i, mid + offset + i);
			        m2 = med3(m1, m2, m3);
                            } else {
			        i = ((nrec - 1) / 8) * rsiz;
			        m1 = med3(b_lim, b_lim +  i, b_lim + 2 * i);
			        m2 = med3(b_lim + 3 * i, b_lim + 4 * i, b_lim + 5 * i);
			        m3 = med3(b_lim + 6 * i, b_lim + 7 * i, b_lim + 8 * i);
			        m2 = med3(m1, m2, m3);
                            }
                        }
		}

                if (aqcmp==cmp) pivot_minrank=nrec; /* BL: allow freezes during partitioning */

		/*
		 * quick sort partitioning
		 *
		 * The partition limits are defined by bottom and top pointers
		 * b_lim and t_lim.
		 *
		 * qsort uses a fairly standard method of moving the
		 * partitioning pointers, b_par and t_par, to the middle of
		 * the partition and exchanging records that are in the
		 * wrong part of the partition.
		 *
		 * Two enhancements have been made to the basic algorithm.
		 * One for handling duplicate records and one to minimize
		 * the number of swaps.
		 *
		 * Two duplicate records pointers are (b_dup and t_dup) are
		 * initially set to b_lim and t_lim.  Each time a record
		 * whose sort key value is equal to the pivot record is found
		 * it will be swapped with the record pointed to by
		 * b_dup or t_dup and the duplicate pointer will be
		 * incremented toward the center.
		 * When partitioning is complete, all the duplicate records
		 * will have been collected at the upper and lower limits of
		 * the partition and can easily be moved adjacent to the
		 * pivot record.
		 *
		 * The second optimization is to minimize the number of swaps.
		 * The pointer m2 points to the pivot record.
		 * During partitioning, if m2 is ever equal to the partitioning
		 * pointers, b_par or t_par, then b_par or t_par just moves
		 * onto the next record without doing a compare.
		 * If as a result of duplicate record detection,
		 * b_dup or t_dup is ever equal to m2,
		 * then m2 is changed to point to the duplicate record and
		 * b_dup or t_dup is incremented with out swapping records.
		 *
		 * When partitioning is done, we may not have the same pivot
		 * record that we started with, but we will have one with
		 * an equal sort key.
		 */
		b_dup = b_par		= b_lim;
		t_dup = t_par = t_lim	= b_lim + rsiz * (nrec - 1);
		for (;;) {

			/* move bottom pointer up */
			for (; b_par <= t_par; b_par += rsiz) {
				if (b_par == m2) {
					continue;
				}
				cv = cmp(b_par, m2);
				if (cv > 0) {
					break;
				}
				if (cv == 0) {
					if (b_dup == m2) {
						m2 = b_par;
					} else if (b_dup != b_par) {
						(*swapf)(b_dup, b_par, loops);
					}
					b_dup += rsiz;
				}
			}

			/* move top pointer down */
			for (; b_par < t_par; t_par -= rsiz) {
				if (t_par == m2) {
					continue;
				}
				cv = cmp(t_par, m2);
				if (cv < 0) {
					break;
				}
				if (cv == 0) {
					if (t_dup == m2) {
						m2 = t_par;
					} else if (t_dup != t_par) {
						(*swapf)(t_dup, t_par, loops);
					}
					t_dup -= rsiz;
				}
			}

			/* break if we are done partitioning */
			if (b_par >= t_par) {
				break;
			}

			/* exchange records at upper and lower break points */
			(*swapf)(b_par, t_par, loops);
			b_par += rsiz;
			t_par -= rsiz;
		}

		/*
		 * partitioning is now complete
		 *
		 * there are two termination conditions from the partitioning
		 * loop above.  Either b_par or t_par have crossed or
		 * they are equal.
		 *
		 * we need to swap the pivot record to its final position
		 * m2 could be in either the upper or lower partitions
		 * or it could already be in its final position
		 */
		/*
		 * R[b_par] > R[m2]
		 * R[t_par] < R[m2]
		 */
		if (t_par < b_par) {
			if (m2 < t_par) {
				(*swapf)(m2, t_par, loops);
				m2 = b_par = t_par;
			} else if (m2 > b_par) {
				(*swapf)(m2, b_par, loops);
				m2 = t_par = b_par;
			} else {
				b_par = t_par = m2;
			}
		} else {
			if (m2 < t_par) {
				t_par = b_par = t_par - rsiz;
			}
			if (m2 != b_par) {
				(*swapf)(m2, b_par, loops);
			}
			m2 = t_par;
		}

		/*
		 * move bottom duplicates next to pivot
		 * optimized to eliminate overlap
		 */
		d_bytelength = b_dup - b_lim;
		if (b_par - b_dup < d_bytelength) {
			b_dup = b_lim + (b_par - b_dup);
		}
		while (b_dup > b_lim) {
			b_dup -= rsiz;
			b_par -= rsiz;
			(*swapf)(b_dup, b_par, loops);
		}
		b_par = m2 - d_bytelength;

		/*
		 * move top duplicates next to pivot
		 */
		d_bytelength = t_lim - t_dup;
		if (t_dup - t_par < d_bytelength) {
			t_dup = t_lim - (t_dup - t_par);
		}
		while (t_dup < t_lim) {
			t_dup += rsiz;
			t_par += rsiz;
			(*swapf)(t_dup, t_par, loops);
		}
		t_par = m2 + d_bytelength;

                npartitions++; /* BL */

		/*
		 * when a qsort pass completes there are three partitions
		 * 1) the lower contains all records less than pivot
		 * 2) the upper contains all records greater than pivot
		 * 3) the pivot partition contains all record equal to pivot
		 *
		 * all records in the pivot partition are in their final
		 * position and do not need to be accounted for by the stack
		 *
		 * when adding partitions to the stack
		 * it is important to add the largest partition first
		 * to prevent stack overflow.
		 *
		 * calculate number of unsorted records in top and bottom
		 * push resulting partitions on stack
		 */
		b_nrec = (b_par - b_lim) / rsiz;
		t_nrec = (t_lim - t_par) / rsiz;
                lnne = b_nrec + t_nrec; /* BL */
                lneq = nrec - lnne; /* BL */

#if 0 /* BL: for verification of adverse sequence */
                if (aqcmp==cmp) {
                    fprintf(stderr,
                        "%s: %s line %d: b_nrec=%lu, t_nrec=%lu, nrec=%lu\n",
                        __func__,source_file,__LINE__,b_nrec,t_nrec,nrec);
                }
#endif

		if (b_nrec < t_nrec) {
			sp->b_lim = t_par + rsiz;
			sp->nrec = t_nrec;
			sp++;
			sp->b_lim = b_lim;
			sp->nrec = b_nrec;
			sp++;
		} else {
			sp->b_lim = b_lim;
			sp->nrec = b_nrec;
			sp++;
			sp->b_lim = t_par + rsiz;
			sp->nrec = t_nrec;
			sp++;
		}
	}
}

/*
 * The following swap functions should not create a stack frame
 * the SPARC call / return instruction will be executed
 * but the a save / restore will not be executed
 * which means we won't do a window turn with the spill / fill overhead
 * verify this by examining the assembly code
 */

/* ARGSUSED */
static void
swapp32(uint32_t *r1, uint32_t *r2, size_t cnt)
{
	uint32_t temp;

	temp = *r1;
	*r1++ = *r2;
	*r2++ = temp;
        nsw++; /* BL */
}

/* ARGSUSED */
static void
swapp64(uint64_t *r1, uint64_t *r2, size_t cnt)
{
	uint64_t temp;

	temp = *r1;
	*r1++ = *r2;
	*r2++ = temp;
        nsw++; /* BL */
}

static void
swapi(uint32_t *r1, uint32_t *r2, size_t cnt)
{
	uint32_t temp;

	/* character by character */
        nsw+=cnt; /* BL */
	while (cnt--) {
		temp = *r1;
		*r1++ = *r2;
		*r2++ = temp;
	}
}

static void
swapb(char *r1, char *r2, size_t cnt)
{
	char	temp;

	/* character by character */
        nsw+=cnt; /* BL */
	while (cnt--) {
		temp = *r1;
		*r1++ = *r2;
		*r2++ = temp;
	}
}
