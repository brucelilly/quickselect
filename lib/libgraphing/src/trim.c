/*INDENT OFF*/

/* Description: C source code for trim */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    trim.c copyright 2016-2017 Bruce Lilly.   \ trim.c $
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
/* $Id: ~|^` @(#)   This is trim.c version 1.5 dated 2017-02-25T03:52:04Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "trim" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libgraphing/src/s.trim.c */

/********************** Long description and rationale: ***********************
******************************************************************************/

/* compile-time configuration options */
#define ASSERT_CODE                 0    /* adds size and cost to aid debugging. 0 for tested production code. */
#define DEBUG_CODE                  0    /* adds size and cost to aid debugging. 0 for tested production code. */

/* Nothing to configure below this line. */

/* for assert.h */
#if ! ASSERT_CODE
# define NDEBUG 1
#endif

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: trim.c ~|^` @(#)"
#define SOURCE_MODULE "trim.c"
#define MODULE_VERSION "1.5"
#define MODULE_DATE "2017-02-25T03:52:04Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "trim.h"               /* trim */ /* includes exchange.h, sys/types.h */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <assert.h>             /* assert */
#include <errno.h>              /* errno E* */
#include <limits.h>             /* *_MAX */
#include <math.h>               /* sqrt */
#include <stddef.h>             /* NULL */
#if ASSERT_CODE + DEBUG_CODE
#include <stdio.h>              /* fprintf stderr */
#endif /* ASSERT_CODE + DEBUG_CODE */
#include <string.h>             /* strrchr */

/* macros */
/* LONG_BIT is enough for the stack
*/
#undef NSEGMENTS
#define NSEGMENTS (LONG_BIT)

/* space-saving */
#undef V
#define V void
#undef A
#define A(me) assert(me)

/* stringification (don't change!) */
#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

#undef dfprintf
#if DEBUG_CODE
# define dfprintf(margs)   (V)fprintf margs
#else
# define dfprintf(margs)   /* */
#endif /* DEBUG_CODE */

/* static data (for version tracking for debugging/error reports) */
static char trim_initialized=(char)0;
static const char *filenamebuf= __FILE__ ;
static const char *source_file= NULL;
const char *trim_build_options="@(#)trim.c: "
    "built with configuration options: "
    "ASSERT_CODE=" xbuildstr(ASSERT_CODE)
#ifdef DEBUG_CODE
    ", DEBUG_CODE=" xbuildstr(DEBUG_CODE)
#endif
#ifdef EXCHANGE_DO_SHORT
    ", EXCHANGE_DO_SHORT=" xbuildstr(EXCHANGE_DO_SHORT)
#endif
#ifdef BUILD_NOTES
    ", BUILD_NOTES=" BUILD_NOTES
#endif
    ;

/* data structures */

/* Segment information is stored in an array of
   structures.
*/
struct segment_struct {
    size_t l;   /* index to left-most point */
    size_t r;   /* index to right-most point */
};
static const size_t segmentsize=sizeof(struct segment_struct);

/* static functions */

/* initialization of static strings at run-time */
static void initialize_trim(V)
{
    const char *s;

    s=strrchr(filenamebuf,'/');
    if (NULL==s) s=filenamebuf; else s++;
    trim_initialized=register_build_strings(trim_build_options,
        &source_file,s);
}

/* segment manipulation */

/* Add a segment to the list of segments, return updated number of segments. */
/* If segment has 2 points or fewer, it is ignored. */
static
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
int add_segment(size_t l, size_t r, struct segment_struct *ps,
    int ns)
{

    A(NULL!=ps);
    if (r<=l+1UL) return ns;
    ps[ns].l=l, ps[ns].r=r;
    return ++ns;
}

/* Sorting the segments by size provides for processing the segments in an
   appropriate order, i.e. the largest segment to be processed will be at
   segments[0], and segments[top-1] is the smallest segment.  Et cetera.
   An in-line selection sort is used as the number of segments is
   expected to be small, and the segment size is compared directly
   (selection sort uses minimum swaps, which dominate the cost when
   comparisons are cheap).
*/
/* selection sort of segments, largest first */
static 
#if defined(__STDC__) && __STDC_VERSION__ > 199900UL
inline
#endif
void sort_segments(struct segment_struct *ps, int ns)
{
    int c, d, i, j;

    for (c=0UL,j=ns; c<j; c++) {
        for (i=c,d=c; d<j; d++)
            if (ps[d].r-ps[d].l>ps[i].r-ps[i].l) i=d;
        if (i!=c) {
            /* dedicated swap */
            struct segment_struct t;
            t=ps[i], ps[i]=ps[c], ps[c]=t;
        }
    }
}

/* trim definition */
/* no inline: Sun cc barfs */
size_t trim(double *x, double *y, unsigned char *s, size_t l, size_t r,
    double threshold)
{
    int top=0; /* stack: segments[0..top-1] */
    double a, b, c, d, e, mind, maxd;
    size_t i, mini, maxi, ol=l, or=r;

    if (0U==trim_initialized) initialize_trim();
    /* Validate supplied parameters.  Provide a hint by setting errno if
       supplied parameters are invalid.
    */
    A(NULL!=x);A(NULL!=y);A(NULL!=s);A(l<=r);A(0.0<=threshold);
    if ((NULL==x) || (NULL==y) || (NULL==s) || (l>r) || (0.0>threshold)) {
        errno=EINVAL;
        return 0UL;
    }
#if DEBUG_CODE
dfprintf((stderr,"\n// %s(x=%p, y=%p, s=%p, l=%lu, r=%lu, threshold=%G) line %d\n",__func__,x,y,s,l,r,threshold,__LINE__));
#endif /* DEBUG_CODE */

    /* When there is nothing left to do in the inner loop, the outer processing
       loop pops a pointer and size from the stack of segments to be processed
       unless the stack is empty, in which case the trim is complete.
    */
    while (1) { /* stack-not-empty loop: break out when stack is empty */
        struct segment_struct segments[(NSEGMENTS)], *p;

        /* coefficients describing line ax+by+c=0 between segment endpoints */
        a=y[l]-y[r];
        b=x[r]-x[l];
        c=x[l]*y[r]-x[r]*y[l];
        /* distance/threshold metric */
        d=sqrt(a*a+b*b);
        e=threshold*d; /* with a,b,c unchanged, this is equvalent to scaling
                          coefficients to unit normal vector, but uses 1
                          multiplication instead of 3 divisions
                       */

        /* Find min/max distant points from line between segment endpoints. */
        for (mind=1.0e300,maxd=0.0-mind,mini=l,maxi=r,i=l+1UL; i<r; i++) {

            if (0U<s[i]) continue;
            d=a*x[i]+b*y[i]; /* offset c and threshold scaling ignored here */
            if (d<mind) { mind=d, mini=i; }
            if (d>maxd) { maxd=d, maxi=i; }
        }
        /* It doesn't matter which direction points at mini,maxi are relative
           to the line segment between endpoints; the order of indices does
           matter.  Make sure mini < maxi.
        */
        if (maxi<mini) { i=mini, mini=maxi, maxi=i; d=mind, mind=maxd, maxd=d; }
        mind+=c, maxd+=c; /* apply offset */
        if (0.0>mind) mind=0.0-mind; /* cheap fabs */
        if (0.0>maxd) maxd=0.0-maxd; /* cheap fabs */
        /* compare to scaled threshold; ignore if distance <= threshold */
        if (mind<=e) mini=l;
        if (maxd<=e) maxi=r;
        if ((mini==l)&&(maxi==r)) {
            /* no interior points are required */
            for (i=l+1UL; i<r; i++) {
                if (0U<s[i]) continue;
                s[i]=1U;
            }
        } else {
            if (mini>l) { top=add_segment(l,mini,segments,top); }
            if (((mini>l)||(maxi<r))&&(mini!=maxi))
                top=add_segment(mini,maxi,segments,top);
            if (maxi<r) { top=add_segment(maxi,r,segments,top); }
            sort_segments(segments, top);
        }

        if (0>=top) break; /* Done; function returns to caller. */
        /* Pop the smallest segment from the stack and process it. */
        p=&(segments[--top]); l=p->l, r=p->r;
    } /* stack not empty loop */
    /* return count of remaining points */
    for (r=0UL,i=ol; i<=or; i++) { if (0U==s[i]) r++; }
    return r;
}
