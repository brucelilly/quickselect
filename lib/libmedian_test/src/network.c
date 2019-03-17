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
* $Id: ~|^` @(#)    network.c copyright 2016-2019 Bruce Lilly.   \ network.c $
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
/* $Id: ~|^` @(#)   This is network.c version 1.14 dated 2019-03-16T15:37:11Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.network.c */

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
#define ID_STRING_PREFIX "$Id: network.c ~|^` @(#)"
#define SOURCE_MODULE "network.c"
#define MODULE_VERSION "1.14"
#define MODULE_DATE "2019-03-16T15:37:11Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2019"

#define __STDC_WANT_LIB_EXT1__ 0
#define LIBMEDIAN_TEST_CODE 1

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes most other local and system header files required */

#include "indirect.h"           /* rearrange_array convert_pointers_to_indices
                                */
#include "initialize_src.h"

#include <stdio.h>

#if defined(__STDC__) && ( __STDC__ == 1) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void networksort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int options)
{
    char *pa, *pb;

    pa=base+first*size;
    pb=pa+size;
    switch (beyond-first) {
        case 1UL :
        return;
        case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
            CX(pa,pb);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
#endif
        return;
        case 3UL : /* 3 comparisons */
# if QUICKSELECT_STABLE
            if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc=pb+size;
                /* stable, but always 3 comparisons, cannot be parallelized */
                CX(pa,pb); /* 0,1 */ CX(pb,pc); /* 1,2 */ CX(pa,pb); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size;
            /* optimized: */
            /* not stable */
            /* compare/exchange first,last elements */
            CX(pa,pc);
            CX(pa,pb) /* N.B. no semicolon here! */
#if 1 /* 0 (no optimization) for pure sorting network comparison count measurements */
# if ! SILENCE_WHINEY_COMPILERS
            ; /* darn whiney compilers; optimization or lots of compilation noise */
# endif
            else /* if middle element is <= first, it cannot be > last */
#endif
                 CX(pb,pc);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
#endif
            }
        return;
        case 4UL : /* 5 comparisons in 3 parallel groups */
# if QUICKSELECT_STABLE
            if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc, *pd;
                pc=pb+size; pd=pc+size;
                /* 6 comparisons, 5 groups (only one of which contains more than one comparison) */
                CX(pa,pb); /* 0,1 */ CX(pb,pc); /* 1,2 */
                /* parallel group */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                /* end of group */
                CX(pb,pc); /* 1,2 */ CX(pa,pb); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size, *pd=pc+size;
            /* parallel group 1 */
                CX(pa,pc); CX(pb,pd);
            /* parallel group 2 */
                CX(pa,pb); CX(pc,pd);
            /* parallel group 3 */
                CX(pb,pc);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
#endif
            }
        return;
        case 5UL : /* 9 comparisons in 5 parallel groups */
# if QUICKSELECT_STABLE
            if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc, *pd, *pe;
                pc=pb+size; pd=pc+size; pe=pd+size;
                /* 10 comparisons, 3 opportunities for parallelism */
                CX(pa,pb); /* 0,1 */ CX(pb,pc); /* 1,2 */
                /* parallel group */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                /* end of group */
                /* parallel group */
                CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                /* end of group */
                /* parallel group */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                /* end of group */
                CX(pb,pc); /* 1,2 */ CX(pa,pb); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size;
            /* parallel group 1 */
                CX(pa,pe);
            /* parallel group 2 */
                CX(pa,pc); CX(pb,pd);
            /* parallel group 3 */
                CX(pa,pb); CX(pc,pe);
            /* parallel group 4 */
                CX(pc,pd); CX(pb,pe);
            /* parallel group 5 */
                CX(pb,pc); CX(pd,pe);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
#endif
            }
        return;
        case 6UL : /* 12 comparisons in 6 parallel groups */
# if QUICKSELECT_STABLE
            if (0U!=(options&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc, *pd, *pe, *pf;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                /* 15 comparisons (more than average quicksort), 5 opportunities for parallelism */
                /* equivalent to unrolled insertion sort or bubble sort */
                CX(pa,pb); /* 0,1 */ CX(pb,pc); /* 1,2 */
                /* parallel group */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                /* end of group */
                /* parallel group */
                CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                /* end of group */
                /* parallel group */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                /* end of group */
                /* parallel group */
                CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                /* end of group */
                /* parallel group */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */
                /* end of group */
                CX(pb,pc); /* 1,2 */ CX(pa,pb); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size;
            /* parallel group 1 */
                CX(pa,pe); CX(pb,pf);
            /* parallel group 2 */
                CX(pa,pc); CX(pb,pd);
            /* parallel group 3 */
                CX(pa,pb); CX(pc,pe); CX(pd,pf);
            /* parallel group 4 */
                CX(pc,pd); CX(pe,pf);
            /* parallel group 5 */
                CX(pb,pe);
            /* parallel group 6 */
                CX(pb,pc); CX(pd,pe);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
#endif
            }
        return;
        case 7UL : /* 16 comparisons in 6 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size;
            /* parallel group 1 */
                CX(pa,pe); CX(pb,pf); CX(pc,pg);
            /* parallel group 2 */
                CX(pa,pc); CX(pb,pd); CX(pe,pg);
            /* parallel group 3 */
                CX(pa,pb); CX(pc,pe); CX(pd,pf);
            /* parallel group 4 */
                CX(pc,pd); CX(pe,pf);
            /* parallel group 5 */
                CX(pb,pe); CX(pd,pg);
            /* parallel group 6 */
                CX(pb,pc); CX(pd,pe); CX(pf,pg);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
#endif
            }
        return;
        case 8UL : /* 19 comparisons in 6 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size;
            /* parallel group 1 */
                CX(pa,pe); CX(pb,pf); CX(pc,pg); CX(pd,ph);
            /* parallel group 2 */
                CX(pa,pc); CX(pb,pd); CX(pe,pg); CX(pf,ph);
            /* parallel group 3 */
                CX(pa,pb); CX(pc,pe); CX(pd,pf); CX(pg,ph);
            /* parallel group 4 */
                CX(pc,pd); CX(pe,pf);
            /* parallel group 5 */
                CX(pb,pe); CX(pd,pg);
            /* parallel group 6 */
                CX(pb,pc); CX(pd,pe); CX(pf,pg);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
#endif
            }
        return;
        case 9UL : /* 25 comparisons in 9 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size;
            /* parallel group 1 */
                CX(pa,pb); CX(pd,pe); CX(pg,ph);
            /* parallel group 2 */
                CX(pb,pc); CX(pe,pf); CX(ph,pj);
            /* parallel group 3 */
                CX(pa,pb); CX(pd,pe); CX(pg,ph); CX(pc,pf);
            /* parallel group 4 */
                CX(pa,pd); CX(pb,pe); CX(pf,pj);
            /* parallel group 5 */
                CX(pd,pg); CX(pe,ph); CX(pc,pf);
            /* parallel group 6 */
                CX(pa,pd); CX(pb,pe); CX(pf,ph); CX(pc,pg);
            /* parallel group 7 */
                CX(pb,pd); CX(pe,pg);
            /* parallel group 8 */
                CX(pc,pe); CX(pf,pg);
            /* parallel group 9 */
                CX(pc,pd);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
#endif
            }
        return;
        case 10UL : /* 29 comparisons in 9 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size;
            /* parallel group 1 */
                CX(pa,pf); CX(pb,pg); CX(pc,ph); CX(pd,pj); CX(pe,pk);
            /* parallel group 2 */
                CX(pa,pd); CX(pb,pe); CX(pf,pj); CX(pg,pk);
            /* parallel group 3 */
                CX(pa,pc); CX(pd,pg); CX(ph,pk);
            /* parallel group 4 */
                CX(pa,pb); CX(pc,pe); CX(pf,ph); CX(pj,pk);
            /* parallel group 5 */
                CX(pb,pc); CX(pd,pf); CX(pe,pg); CX(ph,pj);
            /* parallel group 6 */
                CX(pb,pd); CX(pc,pf); CX(pe,ph); CX(pg,pj);
            /* parallel group 7 */
                CX(pc,pd); CX(pg,ph);
            /* parallel group 8 */
                CX(pd,pe); CX(pf,pg);
            /* parallel group 9 */
                CX(pe,pf);
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
#endif
            }
        return;
        case 11UL : /* 35 comparisons in 8 groups (might not be not optimal comparators, but has minimum depth) */
            /* source: http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size;
            /* parallel group 1 */
                CX(pb,pl); /* 1,10 */ CX(pc,pk); /* 2,9 */ CX(pd,pj); /* 3,8 */
                CX(pe,ph); /* 4,7 */ CX(pf,pg); /* 5,6 */
            /* parallel group 2 */
                CX(pa,pg); /* 0,6 */ CX(pb,pe); /* 1,4 */ CX(pc,pd); /* 2,3 */
                CX(ph,pl); /* 7,10 */ CX(pj,pk); /* 8,9 */
            /* parallel group 3 */
                CX(pa,pb); /* 0,1 */ CX(pc,pf); /* 2,5 */ CX(pd,pe); /* 3,4 */
                CX(ph,pj); /* 7,8 */ CX(pk,pl); /* 9,10 */
            /* parallel group 4 */
                CX(pa,pc); /* 0,2 */ CX(pb,pf); /* 1,5 */ CX(pe,pg); /* 4,6 */
                CX(pj,pk); /* 8,9 */
            /* parallel group 5 */
                CX(pb,pj); /* 1,8 */ CX(pc,pd); /* 2,3 */ CX(pe,ph); /* 4,7 */
                CX(pf,pk); /* 5,9 */ CX(pg,pl); /* 6,10 */
            /* parallel group 6 */
                CX(pb,pe); /* 1,4 */ CX(pd,pf); /* 3,5 */ CX(pg,pk); /* 6,9 */
                CX(ph,pj); /* 7,8 */
            /* parallel group 7 */
                CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */ CX(pf,ph); /* 5,7 */
                CX(pg,pj); /* 6,8 */
            /* parallel group 8 */
                CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */ CX(pg,ph); /* 6,7 */
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
            A(0>=OPT_COMPAR(pk,pl,options));
#endif
            }
        return;
        case 12UL : /* 39 comparisons in 9 parallel groups (maybe not optimal) */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size, *pm=pl+size;
            /* parallel group 1 */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */ CX(pl,pm); /* 10,11 */
            /* parallel group 2 */
                CX(pa,pc); /* 0,2 */ CX(pb,pd); /* 1,3 */ CX(pe,pg); /* 4,6 */
                CX(pf,ph); /* 5,7 */ CX(pj,pl); /* 8,10 */ CX(pk,pm); /* 9,11 */
            /* parallel group 3 */
                CX(pa,pe); /* 0,4 */ CX(pb,pc); /* 1,2 */ CX(pf,pg); /* 5,6 */
                CX(ph,pm); /* 7,11 */ CX(pk,pl); /* 9,10 */
            /* parallel group 4 */
                CX(pb,pf); /* 1,5 */ CX(pd,ph); /* 3,7 */ CX(pe,pj); /* 4,8 */
                CX(pg,pl); /* 6,10 */
            /* parallel group 5 */
                CX(pa,pe); /* 0,4 */ CX(pc,pg); /* 2,6 */ CX(pd,pj); /* 3,8 */
                CX(pf,pk); /* 5,9 */ CX(ph,pm); /* 7,11 */
            /* parallel group 6 */
                CX(pb,pf); /* 1,5 */ CX(pc,pd); /* 2,3 */ CX(pg,pl); /* 6,10 */
                CX(pj,pk); /* 8,9 */
            /* parallel group 7 */
                CX(pb,pe); /* 1,4 */ CX(pd,pf); /* 3,5 */ CX(pg,pj); /* 6,8 */
                CX(ph,pl); /* 7,10 */
            /* parallel group 8 */
                CX(pc,pe); /* 2,4 */ CX(pf,pg); /* 5,6 */ CX(ph,pk); /* 7,9 */
            /* parallel group 9 */
                CX(pd,pe); /* 3,4 */ CX(ph,pj); /* 7,8 */
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
            A(0>=OPT_COMPAR(pk,pl,options));
            A(0>=OPT_COMPAR(pl,pm,options));
#endif
            }
        return;
        case 13UL : /* 45 comparisons in 10 parallel groups (maybe not optimal) */
            /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
            /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size, *pm=pl+size, *pn=pm+size;
            /* parallel group 1 */
                CX(pb,pg); /* 1,6 */ CX(pj,pk); /* 8,9 */ CX(pc,pm); /* 2,11 */
                CX(pa,pf); /* 0,5 */ CX(ph,pn); /* 7,12 */ CX(pe,pl); /* 4,10 */
            /* parallel group 2 */
                CX(pb,pc); /* 1,2 */ CX(ph,pj); /* 7,8 */ CX(pk,pn); /* 9,12 */
                CX(pa,pe); /* 0,4 */ CX(pg,pm); /* 6,11 */ CX(pf,pl); /* 5,10 */
            /* parallel group 3 */
                CX(pf,pj); /* 5,8 */ CX(pl,pn); /* 10,12 */ CX(pa,ph); /* 0,7 */
                CX(pd,pm); /* 3,11 */ CX(pe,pk); /* 4,9 */
            /* parallel group 4 */
                CX(pd,pl); /* 3,10 */ CX(pm,pn); /* 11,12 */ CX(pg,pj); /* 6,8 */
                CX(pc,pk); /* 2,9 */
            /* parallel group 5 */
                CX(pb,pd); /* 1,3 */ CX(pe,pg); /* 4,6 */ CX(pk,pm); /* 9,11 */
                CX(pc,pf); /* 2,5 */ CX(pj,pl); /* 8,10 */
            /* parallel group 6 */
                CX(pa,pb); /* 0,1 */ CX(pd,ph); /* 3,7 */ CX(pj,pk); /* 8,9 */
                CX(pl,pm); /* 10,11 */
            /* parallel group 7 */
                CX(pf,pj); /* 5,8 */ CX(pb,pc); /* 1,2 */ CX(pd,pe); /* 3,4 */
                CX(pg,ph); /* 6,7 */ CX(pk,pl); /* 9,10 */
            /* parallel group 8 */
                CX(pc,pe); /* 2,4 */ CX(pf,pg); /* 5,6 */ CX(ph,pj); /* 7,8 */
                CX(pb,pd); /* 1,3 */
            /* parallel group 9 */
                CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */ CX(pg,ph); /* 6,7 */
                CX(pj,pk); /* 8,9 */
            /* parallel group 10 */
                CX(pd,pe); /* 3,4 */ CX(pf,pg); /* 5,6 */
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
            A(0>=OPT_COMPAR(pk,pl,options));
            A(0>=OPT_COMPAR(pl,pm,options));
            A(0>=OPT_COMPAR(pm,pn,options));
#endif
            }
        return;
        case 14UL : /* 51 comparisons in 10 parallel groups (maybe not optimal) */
            /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
            /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
            /* constucted by removing bottom row of 15-input network */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size, *pm=pl+size, *pn=pm+size, *po=pn+size;
            /* parallel group 1 */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */ CX(pl,pm); /* 10,11 */
                CX(pn,po); /* 12,13 */
            /* parallel group 2 */
                CX(pa,pc); /* 0,2 */ CX(pe,pg); /* 4,6 */ CX(pj,pl); /* 8,10 */
                CX(pb,pd); /* 1,3 */ CX(pf,ph); /* 5,7 */ CX(pk,pm); /* 9,11 */
            /* parallel group 3 */
                CX(pa,pe); /* 0,4 */ CX(pj,pn); /* 8,12 */ CX(pb,pf); /* 1,5 */
                CX(pk,po); /* 9,13 */ CX(pc,pg); /* 2,6 */ CX(pd,ph); /* 3,7 */
            /* parallel group 4 */
                CX(pa,pj); /* 0,8 */ CX(pb,pk); /* 1,9 */ CX(pc,pl); /* 2,10 */
                CX(pd,pm); /* 3,11 */ CX(pe,pn); /* 4,12 */ CX(pf,po); /* 5,13 */
            /* parallel group 5 */
                CX(pf,pl); /* 5,10 */ CX(pg,pk); /* 6,9 */ CX(pd,pn); /* 3,12 */
                CX(ph,pm); /* 7,11 */ CX(pb,pc); /* 1,2 */ CX(pe,pj); /* 4,8 */
            /* parallel group 6 */
                CX(pb,pe); /* 1,4 */ CX(ph,po); /* 7,13 */ CX(pc,pj); /* 2,8 */
            /* parallel group 7 */
                CX(pc,pe); /* 2,4 */ CX(pf,pg); /* 5,6 */ CX(pk,pl); /* 9,10 */
                CX(pm,po); /* 11,13 */ CX(pd,pj); /* 3,8 */ CX(ph,pn); /* 7,12 */
            /* parallel group 8 */
                CX(pd,pf); /* 3,5 */ CX(pg,pj); /* 6,8 */ CX(ph,pk); /* 7,9 */
                CX(pl,pn); /* 10,12 */
            /* parallel group 9 */
                CX(pd,pe); /* 3,4 */ CX(pf,pg); /* 5,6 */ CX(ph,pj); /* 7,8 */
                CX(pk,pl); /* 9,10 */ CX(pm,pn); /* 11,12 */
            /* parallel group 10 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
            A(0>=OPT_COMPAR(pk,pl,options));
            A(0>=OPT_COMPAR(pl,pm,options));
            A(0>=OPT_COMPAR(pm,pn,options));
            A(0>=OPT_COMPAR(pn,po,options));
#endif
            }
        return;
        case 15UL : /* 56 comparisons in 10 parallel groups (maybe not optimal) */
            /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
            /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
            /* constucted by removing bottom row of 16-input network */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size, *pm=pl+size, *pn=pm+size, *po=pn+size,
                   *pp=po+size;
            /* parallel group 1 */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */ CX(pl,pm); /* 10,11 */
                CX(pn,po); /* 12,13 */
            /* parallel group 2 */
                CX(pa,pc); /* 0,2 */ CX(pe,pg); /* 4,6 */ CX(pj,pl); /* 8,10 */
                CX(pn,pp); /* 12,14 */ CX(pb,pd); /* 1,3 */ CX(pf,ph); /* 5,7 */
                CX(pk,pm); /* 9,11 */
            /* parallel group 3 */
                CX(pa,pe); /* 0,4 */ CX(pj,pn); /* 8,12 */ CX(pb,pf); /* 1,5 */
                CX(pk,po); /* 9,13 */ CX(pc,pg); /* 2,6 */ CX(pl,pp); /* 10,14 */
                CX(pd,ph); /* 3,7 */
            /* parallel group 4 */
                CX(pa,pj); /* 0,8 */ CX(pb,pk); /* 1,9 */ CX(pc,pl); /* 2,10 */
                CX(pd,pm); /* 3,11 */ CX(pe,pn); /* 4,12 */ CX(pf,po); /* 5,13 */
                CX(pg,pp); /* 6,14 */
            /* parallel group 5 */
                CX(pf,pl); /* 5,10 */ CX(pg,pk); /* 6,9 */ CX(pd,pn); /* 3,12 */
                CX(po,pp); /* 13,14 */ CX(ph,pm); /* 7,11 */ CX(pb,pc); /* 1,2 */
                CX(pe,pj); /* 4,8 */
            /* parallel group 6 */
                CX(pb,pe); /* 1,4 */ CX(ph,po); /* 7,13 */ CX(pc,pj); /* 2,8 */
                CX(pm,pp); /* 11,14 */
            /* parallel group 7 */
                CX(pc,pe); /* 2,4 */ CX(pf,pg); /* 5,6 */ CX(pk,pl); /* 9,10 */
                CX(pm,po); /* 11,13 */ CX(pd,pj); /* 3,8 */ CX(ph,pn); /* 7,12 */
            /* parallel group 8 */
                CX(pd,pf); /* 3,5 */ CX(pg,pj); /* 6,8 */ CX(ph,pk); /* 7,9 */
                CX(pl,pn); /* 10,12 */
            /* parallel group 9 */
                CX(pd,pe); /* 3,4 */ CX(pf,pg); /* 5,6 */ CX(ph,pj); /* 7,8 */
                CX(pk,pl); /* 9,10 */ CX(pm,pn); /* 11,12 */
            /* parallel group 10 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
            A(0>=OPT_COMPAR(pk,pl,options));
            A(0>=OPT_COMPAR(pl,pm,options));
            A(0>=OPT_COMPAR(pm,pn,options));
            A(0>=OPT_COMPAR(pn,po,options));
            A(0>=OPT_COMPAR(po,pp,options));
#endif
            }
        return;
        case 16UL : /* 60 comparisons in 10 parallel groups (maybe not optimal) */
            /* http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
            /* http://www.cs.brandeis.edu/~hugues/sorting_networks.html */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size, *pm=pl+size, *pn=pm+size, *po=pn+size,
                   *pp=po+size, *pq=pp+size;
                /* parallel group 1 */
                CX(pa,pb); /* 0,1 */ CX(pc,pd); /* 2,3 */ CX(pe,pf); /* 4,5 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */ CX(pl,pm); /* 10,11 */
                CX(pn,po); /* 12,13 */ CX(pp,pq); /* 14,15 */
                /* parallel group 2 */
                CX(pa,pc); /* 0,2 */ CX(pe,pg); /* 4,6 */ CX(pj,pl); /* 8,10 */
                CX(pn,pp); /* 12,14 */ CX(pb,pd); /* 1,3 */ CX(pf,ph); /* 5,7 */
                CX(pk,pm); /* 9,11 */ CX(po,pq); /* 13,15 */
                /* parallel group 3 */
                CX(pa,pe); /* 0,4 */ CX(pj,pn); /* 8,12 */ CX(pb,pf); /* 1,5 */
                CX(pk,po); /* 9,13 */ CX(pc,pg); /* 2,6 */ CX(pl,pp); /* 10,14 */
                CX(pd,ph); /* 3,7 */ CX(pm,pq); /* 11,15 */
                /* parallel group 4 */
                CX(pa,pj); /* 0,8 */ CX(pb,pk); /* 1,9 */ CX(pc,pl); /* 2,10 */
                CX(pd,pm); /* 3,11 */ CX(pe,pn); /* 4,12 */ CX(pf,po); /* 5,13 */
                CX(pg,pp); /* 6,14 */ CX(ph,pq); /* 7,15 */
                /* parallel group 5 */
                CX(pf,pl); /* 5,10 */ CX(pg,pk); /* 6,9 */ CX(pd,pn); /* 3,12 */
                CX(po,pp); /* 13,14 */ CX(ph,pm); /* 7,11 */ CX(pb,pc); /* 1,2 */
                CX(pe,pj); /* 4,8 */
                /* parallel group 6 */
                CX(pb,pe); /* 1,4 */ CX(ph,po); /* 7,13 */ CX(pc,pj); /* 2,8 */
                CX(pm,pp); /* 11,14 */
                /* parallel group 7 */
                CX(pc,pe); /* 2,4 */ CX(pf,pg); /* 5,6 */ CX(pk,pl); /* 9,10 */
                CX(pm,po); /* 11,13 */ CX(pd,pj); /* 3,8 */ CX(ph,pn); /* 7,12 */
                /* parallel group 8 */
                CX(pd,pf); /* 3,5 */ CX(pg,pj); /* 6,8 */ CX(ph,pk); /* 7,9 */
                CX(pl,pn); /* 10,12 */
                /* parallel group 9 */
                CX(pd,pe); /* 3,4 */ CX(pf,pg); /* 5,6 */ CX(ph,pj); /* 7,8 */
                CX(pk,pl); /* 9,10 */ CX(pm,pn); /* 11,12 */
                /* parallel group 10 */
                CX(pg,ph); /* 6,7 */ CX(pj,pk); /* 8,9 */
#if ASSERT_CODE > 1
            A(0>=OPT_COMPAR(pa,pb,options));
            A(0>=OPT_COMPAR(pb,pc,options));
            A(0>=OPT_COMPAR(pc,pd,options));
            A(0>=OPT_COMPAR(pd,pe,options));
            A(0>=OPT_COMPAR(pe,pf,options));
            A(0>=OPT_COMPAR(pf,pg,options));
            A(0>=OPT_COMPAR(pg,ph,options));
            A(0>=OPT_COMPAR(ph,pj,options));
            A(0>=OPT_COMPAR(pj,pk,options));
            A(0>=OPT_COMPAR(pk,pl,options));
            A(0>=OPT_COMPAR(pl,pm,options));
            A(0>=OPT_COMPAR(pm,pn,options));
            A(0>=OPT_COMPAR(pn,po,options));
            A(0>=OPT_COMPAR(po,pp,options));
            A(0>=OPT_COMPAR(pp,pq,options));
#endif
            }
        return;
        default :
            /* Fall back to insertion sort in case size is out-of-bounds. */
            isort_internal(base,first,beyond,size,compar,swapf,alignsize,size_ratio,options);
        break;
    }
}

#if 0 /* XXX reservoir of compare-exchange pairs */
                CX(pa,pb); /* 0,1 */
                CX(pa,pc); /* 0,2 */
                CX(pa,pe); /* 0,4 */
                CX(pa,pf); /* 0,5 */
                CX(pa,ph); /* 0,7 */
                CX(pa,pj); /* 0,8 */
                CX(pb,pc); /* 1,2 */
                CX(pb,pd); /* 1,3 */
                CX(pb,pe); /* 1,4 */
                CX(pb,pf); /* 1,5 */
                CX(pb,pk); /* 1,9 */
                CX(pb,pl); /* 1,10 */
                CX(pc,pd); /* 2,3 */
                CX(pc,pe); /* 2,4 */
                CX(pc,pf); /* 2,5 */
                CX(pc,pg); /* 2,6 */
                CX(pc,pj); /* 2,8 */
                CX(pc,pk); /* 2,9 */
                CX(pc,pl); /* 2,10 */
                CX(pd,pe); /* 3,4 */
                CX(pd,pf); /* 3,5 */
                CX(pd,ph); /* 3,7 */
                CX(pd,pj); /* 3,8 */
                CX(pd,pl); /* 3,10 */
                CX(pd,pm); /* 3,11 */
                CX(pd,pn); /* 3,12 */
                CX(pe,pf); /* 4,5 */
                CX(pe,pg); /* 4,6 */
                CX(pe,ph); /* 4,7 */
                CX(pe,pj); /* 4,8 */
                CX(pe,pk); /* 4,9 */
                CX(pe,pl); /* 4,10 */
                CX(pe,pm); /* 4,11 */
                CX(pe,pn); /* 4,12 */
                CX(pf,pg); /* 5,6 */
                CX(pf,ph); /* 5,7 */
                CX(pf,pj); /* 5,8 */
                CX(pf,pk); /* 5,9 */
                CX(pf,pl); /* 5,10 */
                CX(pf,pm); /* 5,11 */
                CX(pf,pn); /* 5,12 */
                CX(pf,po); /* 5,13 */
                CX(pg,ph); /* 6,7 */
                CX(pg,pj); /* 6,8 */
                CX(pg,pk); /* 6,9 */
                CX(pg,pl); /* 6,10 */
                CX(pg,pm); /* 6,11 */
                CX(pg,pn); /* 6,12 */
                CX(pg,po); /* 6,13 */
                CX(pg,pp); /* 6,14 */
                CX(ph,pj); /* 7,8 */
                CX(ph,pk); /* 7,9 */
                CX(ph,pl); /* 7,10 */
                CX(ph,pm); /* 7,11 */
                CX(ph,pn); /* 7,12 */
                CX(ph,po); /* 7,13 */
                CX(ph,pp); /* 7,14 */
                CX(ph,pq); /* 7,15 */
                CX(pj,pk); /* 8,9 */
                CX(pj,pl); /* 8,10 */
                CX(pj,pm); /* 8,11 */
                CX(pj,pn); /* 8,12 */
                CX(pk,pl); /* 9,10 */
                CX(pk,pm); /* 9,11 */
                CX(pk,pn); /* 9,12 */
                CX(pk,po); /* 9,13 */
                CX(pl,pm); /* 10,11 */
                CX(pl,pn); /* 10,12 */
                CX(pl,po); /* 10,13 */
                CX(pl,pp); /* 10,14 */
                CX(pm,pn); /* 11,12 */
                CX(pm,po); /* 11,13 */
                CX(pm,pp); /* 11,14 */
                CX(pm,pq); /* 11,15 */
                CX(pn,po); /* 12,13 */
                CX(pn,pp); /* 12,14 */
                CX(po,pp); /* 13,14 */
                CX(po,pq); /* 13,15 */
                CX(pp,pq); /* 14,15 */
#endif

void networksort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *),
    unsigned int options)
{
    if ((char)0==file_initialized) initialize_file(__FILE__);

    if (1UL<nmemb) {
        size_t alignsize=alignment_size(base,size);
        size_t size_ratio=size/alignsize;
        void (*swapf)(char *, char *, size_t);
        size_t *indices = NULL;
        char **pointers = NULL;
        void (*pswapf)(char *, char *, size_t);

        if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
        if (0U!=(options&(QUICKSELECT_INDIRECT))) {
            char **p=set_array_pointers(pointers,nmemb,base,size,0UL,nmemb);
            if (NULL==p) {
                if (NULL!=pointers) { free(pointers); pointers=NULL; }
                goto no_ind;
            } else {
                size_t n;
                if (p!=pointers) pointers=p;
                if (0U==instrumented) pswapf=swapn(sizeof(char *));
                else pswapf=iswapn(sizeof(char *));
                /* pswapf is guaranteed to have been assigned a value by the
                   above statement (and always if options contains
                   QUICKSELECT_INDIRECT after this block), whether or not (:-/)
                   gcc's authors realize it...
                */
                A(pointers[0]==base);
                networksort_internal((char *)pointers,0UL,nmemb,sizeof(char *),
                    compar,pswapf,sizeof(char *),1UL,options);
                if (NULL==indices) indices=(size_t *)pointers;
                indices=convert_pointers_to_indices(base,nmemb,size,pointers,
                    nmemb,indices,0UL,nmemb);
                A(NULL!=indices);
#if LIBMEDIAN_TEST_CODE
                if (DEBUGGING(SORT_SELECT_DEBUG)) {
                    (V) fprintf(stderr,"/* %s: %s line %d: indices:",
                        __func__,source_file,__LINE__);
                    for (n=0UL; n<nmemb; n++)
                        (V)fprintf(stderr," %lu:%lu",n,indices[n]);
                    (V) fprintf(stderr," */\n");
                    print_some_array(base,0UL,nmemb-1UL,"/* base array:"," */",
                        options);
                }
#endif
                n=rearrange_array(base,nmemb,size,indices,nmemb,0UL,nmemb,
                    alignsize);
                free(pointers);
                if ((void *)indices==(void *)pointers) indices=NULL;
                pointers=NULL;
                if (nmemb<(n>>1)) { /* indirection failed; use direct sort/select */
                    /* shouldn't happen */
                    (V)fprintf(stderr,"/* %s: %s line %d: size_ratio=%lu, nmemb"
                        "=%lu, indirect sort failed */\n",__func__,source_file,
                        __LINE__,size_ratio,nmemb);
no_ind:             options&=~(QUICKSELECT_INDIRECT);
                    networksort_internal(base,0UL,nmemb,size,compar,swapf,
                        alignsize,size_ratio,options);
                } else if (0UL!=n)
                    nmoves+=n*size_ratio;
            }
        } else
            networksort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,
                size_ratio,options);
    }
}
