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
* $Id: ~|^` @(#)    network.c copyright 2016-2017 Bruce Lilly.   \ network.c $
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
/* $Id: ~|^` @(#)   This is network.c version 1.5 dated 2017-11-06T16:32:56Z. \ $ */
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
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2017-11-06T16:32:56Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void networksort_internal(char *base, size_t first, size_t beyond, size_t size,
    int(*compar)(const void *, const void *),
    void (*swapf)(char *, char *, size_t), size_t alignsize, size_t size_ratio,
    unsigned int network_map)
{
    char *pa, *pb;

    pa=base+first*size;
    pb=pa+size;
    switch (beyond-first) {
        case 1UL :
        return;
        case 2UL : /* 1 comparison, <=1 swap; low overhead; stable */
            COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
#endif
        return;
        case 3UL : /* 3 comparisons */
# if QUICKSELECT_STABLE
            if (0U!=(network_map&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc=pb+size;
                /* stable, but always 3 comparisons, cannot be parallelized */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size;
            /* optimized: */
            /* not stable */
            /* compare/exchange first,last elements */
            COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
            COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
#if 1 /* 0 (no optimization) for pure sorting network comparison count measurements */
            else /* if middle element is <= first, it cannot be > last */
#endif
                 COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
#endif
            }
        return;
        case 4UL : /* 5 comparisons in 3 parallel groups */
# if QUICKSELECT_STABLE
            if (0U!=(network_map&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc, *pd;
                pc=pb+size; pd=pc+size;
                /* 6 comparisons, 5 groups (only one of which contains more than one comparison) */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                /* parallel group */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                /* end of group */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size, *pd=pc+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
#endif
            }
        return;
        case 5UL : /* 9 comparisons in 5 parallel groups */
# if QUICKSELECT_STABLE
            if (0U!=(network_map&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc, *pd, *pe;
                pc=pb+size; pd=pc+size; pe=pd+size;
                /* 10 comparisons, 3 opportunities for parallelism */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                /* parallel group */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                /* end of group */
                /* parallel group */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                /* end of group */
                /* parallel group */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                /* end of group */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pe,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
#endif
            }
        return;
        case 6UL : /* 12 comparisons in 6 parallel groups */
# if QUICKSELECT_STABLE
            if (0U!=(network_map&(QUICKSELECT_STABLE))) { /* request stable sort */
                char *pc, *pd, *pe, *pf;
                pc=pb+size; pd=pc+size; pe=pd+size; pf=pe+size;
                /* 15 comparisons (more than average quicksort), 5 opportunities for parallelism */
                /* equivalent to unrolled insertion sort or bubble sort */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                /* parallel group */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                /* end of group */
                /* parallel group */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                /* end of group */
                /* parallel group */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                COMPARE_EXCHANGE(pe,pf,context,size,swapf,alignsize,size_ratio); /* 4,5 */
                /* end of group */
                /* parallel group */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pd,pe,context,size,swapf,alignsize,size_ratio); /* 3,4 */
                /* end of group */
                /* parallel group */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,context,size,swapf,alignsize,size_ratio); /* 2,3 */
                /* end of group */
                COMPARE_EXCHANGE(pb,pc,context,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pa,pb,context,size,swapf,alignsize,size_ratio); /* 0,1 */
            } else
# endif /* QUICKSELECT_STABLE */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
#endif
            }
        return;
        case 7UL : /* 16 comparisons in 6 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
            A(0>=compar(pf,pg));
#endif
            }
        return;
        case 8UL : /* 19 comparisons in 6 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pg,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,ph,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pg,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,ph,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pg,ph,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
            A(0>=compar(pf,pg));
            A(0>=compar(pg,ph));
#endif
            }
        return;
        case 9UL : /* 25 comparisons in 9 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pg,ph,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(ph,pj,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pg,ph,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                COMPARE_EXCHANGE(pa,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,pj,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                COMPARE_EXCHANGE(pd,pg,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,ph,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pf,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                COMPARE_EXCHANGE(pa,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,ph,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 7 */
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 8 */
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 9 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
            A(0>=compar(pf,pg));
            A(0>=compar(pg,ph));
            A(0>=compar(ph,pj));
#endif
            }
        return;
        case 10UL : /* 29 comparisons in 9 parallel groups */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pg,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,ph,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pj,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pk,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,pj,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pg,pk,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pg,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(ph,pk,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 4 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,ph,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pj,pk,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 5 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pd,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,pg,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(ph,pj,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 6 */
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pc,pf,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pe,ph,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pg,pj,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 7 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pg,ph,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 8 */
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio);
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio);
            /* parallel group 9 */
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio);
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
            A(0>=compar(pf,pg));
            A(0>=compar(pg,ph));
            A(0>=compar(ph,pj));
            A(0>=compar(pj,pk));
#endif
            }
        return;
        case 11UL : /* 35 comparisons in 8 groups (might not be not optimal comparators, but has minimum depth) */
            /* source: http://www.angelfire.com/blog/ronz/Articles/999SortingNetworksReferen.html */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pb,pl,cx,size,swapf,alignsize,size_ratio); /* 1,10 */
                COMPARE_EXCHANGE(pc,pk,cx,size,swapf,alignsize,size_ratio); /* 2,9 */
                COMPARE_EXCHANGE(pd,pj,cx,size,swapf,alignsize,size_ratio); /* 3,8 */
                COMPARE_EXCHANGE(pe,ph,cx,size,swapf,alignsize,size_ratio); /* 4,7 */
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pg,cx,size,swapf,alignsize,size_ratio); /* 0,6 */
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio); /* 1,4 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                COMPARE_EXCHANGE(ph,pl,cx,size,swapf,alignsize,size_ratio); /* 7,10 */
                COMPARE_EXCHANGE(pj,pk,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pf,cx,size,swapf,alignsize,size_ratio); /* 2,5 */
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio); /* 3,4 */
                COMPARE_EXCHANGE(ph,pj,cx,size,swapf,alignsize,size_ratio); /* 7,8 */
                COMPARE_EXCHANGE(pk,pl,cx,size,swapf,alignsize,size_ratio); /* 9,10 */
            /* parallel group 4 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio); /* 0,2 */
                COMPARE_EXCHANGE(pb,pf,cx,size,swapf,alignsize,size_ratio); /* 1,5 */
                COMPARE_EXCHANGE(pe,pg,cx,size,swapf,alignsize,size_ratio); /* 4,6 */
                COMPARE_EXCHANGE(pj,pk,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
            /* parallel group 5 */
                COMPARE_EXCHANGE(pb,pj,cx,size,swapf,alignsize,size_ratio); /* 1,8 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                COMPARE_EXCHANGE(pe,ph,cx,size,swapf,alignsize,size_ratio); /* 4,7 */
                COMPARE_EXCHANGE(pf,pk,cx,size,swapf,alignsize,size_ratio); /* 5,9 */
                COMPARE_EXCHANGE(pg,pl,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
            /* parallel group 6 */
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio); /* 1,4 */
                COMPARE_EXCHANGE(pd,pf,cx,size,swapf,alignsize,size_ratio); /* 3,5 */
                COMPARE_EXCHANGE(pg,pk,cx,size,swapf,alignsize,size_ratio); /* 6,9 */
                COMPARE_EXCHANGE(ph,pj,cx,size,swapf,alignsize,size_ratio); /* 7,8 */
            /* parallel group 7 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio); /* 3,4 */
                COMPARE_EXCHANGE(pf,ph,cx,size,swapf,alignsize,size_ratio); /* 5,7 */
                COMPARE_EXCHANGE(pg,pj,cx,size,swapf,alignsize,size_ratio); /* 6,8 */
            /* parallel group 8 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio); /* 4,5 */
                COMPARE_EXCHANGE(pg,ph,cx,size,swapf,alignsize,size_ratio); /* 6,7 */
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
            A(0>=compar(pf,pg));
            A(0>=compar(pg,ph));
            A(0>=compar(ph,pj));
            A(0>=compar(pj,pk));
            A(0>=compar(pk,pl));
#endif
            }
        return;
        case 12UL : /* 39 comparisons in 9 parallel groups (not optimal) */
            { char *pc=pb+size, *pd=pc+size, *pe=pd+size, *pf=pe+size,
                   *pg=pf+size, *ph=pg+size, *pj=ph+size, *pk=pj+size,
                   *pl=pk+size, *pm=pl+size;
            /* parallel group 1 */
                COMPARE_EXCHANGE(pa,pb,cx,size,swapf,alignsize,size_ratio); /* 0,1 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                COMPARE_EXCHANGE(pe,pf,cx,size,swapf,alignsize,size_ratio); /* 4,5 */
                COMPARE_EXCHANGE(pg,ph,cx,size,swapf,alignsize,size_ratio); /* 6,7 */
                COMPARE_EXCHANGE(pj,pk,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
                COMPARE_EXCHANGE(pl,pm,cx,size,swapf,alignsize,size_ratio); /* 10,11 */
            /* parallel group 2 */
                COMPARE_EXCHANGE(pa,pc,cx,size,swapf,alignsize,size_ratio); /* 0,2 */
                COMPARE_EXCHANGE(pb,pd,cx,size,swapf,alignsize,size_ratio); /* 1,3 */
                COMPARE_EXCHANGE(pe,pg,cx,size,swapf,alignsize,size_ratio); /* 4,6 */
                COMPARE_EXCHANGE(pf,ph,cx,size,swapf,alignsize,size_ratio); /* 5,7 */
                COMPARE_EXCHANGE(pj,pl,cx,size,swapf,alignsize,size_ratio); /* 8,10 */
                COMPARE_EXCHANGE(pk,pm,cx,size,swapf,alignsize,size_ratio); /* 9,11 */
            /* parallel group 3 */
                COMPARE_EXCHANGE(pa,pe,cx,size,swapf,alignsize,size_ratio); /* 0,4 */
                COMPARE_EXCHANGE(pb,pc,cx,size,swapf,alignsize,size_ratio); /* 1,2 */
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
                COMPARE_EXCHANGE(ph,pm,cx,size,swapf,alignsize,size_ratio); /* 7,11 */
                COMPARE_EXCHANGE(pk,pl,cx,size,swapf,alignsize,size_ratio); /* 9,10 */
            /* parallel group 4 */
                COMPARE_EXCHANGE(pb,pf,cx,size,swapf,alignsize,size_ratio); /* 1,5 */
                COMPARE_EXCHANGE(pd,ph,cx,size,swapf,alignsize,size_ratio); /* 3,7 */
                COMPARE_EXCHANGE(pe,pj,cx,size,swapf,alignsize,size_ratio); /* 4,8 */
                COMPARE_EXCHANGE(pg,pl,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
            /* parallel group 5 */
                COMPARE_EXCHANGE(pa,pe,cx,size,swapf,alignsize,size_ratio); /* 0,4 */
                COMPARE_EXCHANGE(pc,pg,cx,size,swapf,alignsize,size_ratio); /* 2,6 */
                COMPARE_EXCHANGE(pd,pj,cx,size,swapf,alignsize,size_ratio); /* 3,8 */
                COMPARE_EXCHANGE(pf,pk,cx,size,swapf,alignsize,size_ratio); /* 5,9 */
                COMPARE_EXCHANGE(ph,pm,cx,size,swapf,alignsize,size_ratio); /* 7,11 */
            /* parallel group 6 */
                COMPARE_EXCHANGE(pb,pf,cx,size,swapf,alignsize,size_ratio); /* 1,5 */
                COMPARE_EXCHANGE(pc,pd,cx,size,swapf,alignsize,size_ratio); /* 2,3 */
                COMPARE_EXCHANGE(pg,pl,cx,size,swapf,alignsize,size_ratio); /* 6,10 */
                COMPARE_EXCHANGE(pj,pk,cx,size,swapf,alignsize,size_ratio); /* 8,9 */
            /* parallel group 7 */
                COMPARE_EXCHANGE(pb,pe,cx,size,swapf,alignsize,size_ratio); /* 1,4 */
                COMPARE_EXCHANGE(pd,pf,cx,size,swapf,alignsize,size_ratio); /* 3,5 */
                COMPARE_EXCHANGE(pg,pj,cx,size,swapf,alignsize,size_ratio); /* 6,8 */
                COMPARE_EXCHANGE(ph,pl,cx,size,swapf,alignsize,size_ratio); /* 7,10 */
            /* parallel group 8 */
                COMPARE_EXCHANGE(pc,pe,cx,size,swapf,alignsize,size_ratio); /* 2,4 */
                COMPARE_EXCHANGE(pf,pg,cx,size,swapf,alignsize,size_ratio); /* 5,6 */
                COMPARE_EXCHANGE(ph,pk,cx,size,swapf,alignsize,size_ratio); /* 7,9 */
            /* parallel group 9 */
                COMPARE_EXCHANGE(pd,pe,cx,size,swapf,alignsize,size_ratio); /* 3,4 */
                COMPARE_EXCHANGE(ph,pj,cx,size,swapf,alignsize,size_ratio); /* 7,8 */
#if ASSERT_CODE > 1
            A(0>=compar(pa,pb));
            A(0>=compar(pb,pc));
            A(0>=compar(pc,pd));
            A(0>=compar(pd,pe));
            A(0>=compar(pe,pf));
            A(0>=compar(pf,pg));
            A(0>=compar(pg,ph));
            A(0>=compar(ph,pj));
            A(0>=compar(pj,pk));
            A(0>=compar(pk,pl));
            A(0>=compar(pl,pm));
#endif
            }
        return;
        default :
            /* Fall back to insertion sort in case size is out-of-bounds. */
            isort_internal(base,first,beyond,size,compar,swapf,alignsize,size_ratio);
        break;
    }
}

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
inline
#endif /* C99 */
void networksort(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    size_t alignsize=alignment_size(base,size);
    size_t size_ratio=size/alignsize;
    void (*swapf)(char *, char *, size_t);

    if ((char)0==file_initialized) initialize_file(__FILE__);
    if (0U==instrumented) swapf=swapn(alignsize); else swapf=iswapn(alignsize);
    if (1UL<nmemb) networksort_internal(base,0UL,nmemb,size,compar,swapf,alignsize,size_ratio,network_mask);
}
