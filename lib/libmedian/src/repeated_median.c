/*INDENT OFF*/

/* Description: C source code for repeated_median_filter */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    repeated_median.c copyright 2016 - 2017 Bruce Lilly.   \ repeated_median.c $
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
/* $Id: ~|^` @(#)   This is repeated_median.c version 1.9 dated 2017-11-03T20:58:18Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "repeated_median" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.repeated_median.c */

/********************** Long description and rationale: ***********************
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: repeated_median.c ~|^` @(#)"
#define SOURCE_MODULE "repeated_median.c"
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2017-11-03T20:58:18Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016 - 2017"

/* configuration options */

#define DEBUG_CODE                  0
#define SILENCE_WHINEY_COMPILERS    0

#if ! DEBUG_CODE
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
# define _XOPEN_SOURCE 500
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* for hstrerror in netdb.h (included by get_ips.h) */
#ifndef _BSD_SOURCE
# define _BSD_SOURCE 1
#endif
#ifndef _SVID_SOURCE
# define _SVID_SOURCE 1
#endif
#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE 1
#endif

/* local header files needed */
#include "compare.h"            /* doublecmp */
#include "quickselect.h"        /* quickselect */
#include "repeated_median.h"    /* repeated_median_filter */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <sys/types.h>          /* *_t */
#include <assert.h>             /* assert */
#include <stddef.h>             /* NULL */
#if DEBUG_CODE
#include <stdio.h>              /* FILE stderr fprintf */
#endif
#include <string.h>             /* strrchr */

/* static data */
static char repeated_median_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MEDIAN_INLINE inline
#else
# define MEDIAN_INLINE /**/
#endif /* C99 */
/* forward references */
static MEDIAN_INLINE void initialize_repeated_median(void);
static MEDIAN_INLINE size_t slope_number(size_t, size_t, size_t);
static MEDIAN_INLINE size_t window_start(size_t, size_t, size_t, unsigned int);
static MEDIAN_INLINE size_t window_end(size_t, size_t, size_t, unsigned int);
static MEDIAN_INLINE double slope(double *, double *, size_t, size_t, size_t);
static MEDIAN_INLINE size_t pair_slopes(double *, double *, size_t, size_t, double *, size_t, size_t, unsigned int);
static MEDIAN_INLINE size_t compute_window_slopes(double *, size_t, size_t, double *, unsigned int);
static MEDIAN_INLINE size_t intercepts(double *, double *, size_t, size_t, double, size_t, unsigned int);

/* initialize at run-time */
static MEDIAN_INLINE void initialize_repeated_median(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s) s = filenamebuf; else s++;
    repeated_median_initialized = register_build_strings(NULL, &source_file, s);
}

#if DEBUG_CODE
/* Print elements of double array with indices l through u inclusive. */
static void print_double_array(double *array, size_t l, size_t u)
{
    size_t i;

    for (i=l; i<=u; i++) {
        if (i != l)
            (void)fprintf(stderr, ", ");
        (void)fprintf(stderr, "%lu:%.9G", i, array[i]);
    }
    (void)fprintf(stderr, "\n");
}
#endif

/* Return an integer in the range 0..n*(w-1) for i,j in 0..n-1
   where i!=j and i and j are within w of each other (modulo n).
   Returned value is associative w.r.t. i and j.
*/
static MEDIAN_INLINE size_t slope_number(size_t n, size_t i, size_t j)
{
    size_t d, minimum, maximum, r;

    assert(i!=j); assert(i<n); assert(j<n);
    if (i > j) minimum=j, maximum=i; else minimum=i, maximum=j;
    d = maximum - minimum;
    if (d >= (n>>1)) r = maximum + n * (n - d - 1UL); else r = minimum + n * (d - 1UL);
    return r;
}

static MEDIAN_INLINE size_t window_start(size_t n, size_t i, size_t w, unsigned int wrap)
{
    size_t h = w>>1, s;

    assert(w<n); assert(i<n);
    if ((0U != wrap) || ((i>h) && (i<n-h))) {
        s=i+n-h;
        if (s>=n) s-=n;
    } else if (i<=h) {
        s = 0UL;
    } else /*i>=n-h*/ {
        s = n-w;
    }
    return s;
}

static MEDIAN_INLINE size_t window_end(size_t n, size_t i, size_t w, unsigned int wrap)
{
    size_t e, h = w>>1;

    assert(w<n); assert(i<n);
    if ((0U != wrap) || ((i>h) && (i<n-h))) {
        e=i+h;
        if (e>=n) e-=n;
    } else if (i<=h) {
        e = w-1UL;
    } else /*i>=n-h*/ {
        e = n-1UL;
    }
    return e;
}

/* compute from data in array pd of size n
   (or look up from slope number in array pz)
   parametric slope for points j,y [j,y in 0..n-1; j != y]
   (parametric slope is difference in value divided by difference in index)
*/
static MEDIAN_INLINE double slope(double *pd, double *pz, size_t n, size_t j, size_t y)
{
    double m;

    assert(NULL!=pd); assert(j<n); assert(y<n);
    if (NULL != pz) {
        size_t d = slope_number(n, j, y);
        m = pz[d];
    } else {
        if (y>j) m = (pd[y] - pd[j]) / (double)(y-j);
        else /* y<j */ m = (pd[j] - pd[y]) / (double)(j-y);
    }
    return m;
}

/* put w-1 pairwise slopes for point x (relative to window start) in window around point i in array ps */
/* slopes are between point (i+n+x-h)%n and points (i+n-h)%n..(i+h)%n where h=w/2 (if wrapped) */
/* caller provides arrays (array pz, size at least n*w, is optional) */
static MEDIAN_INLINE size_t pair_slopes(double *pd, double *ps, size_t n, size_t w, double *pz, size_t i, size_t x, unsigned int wrap)
{
    size_t e, h, j, k=0UL, s, y;

    assert(NULL!=pd); assert(NULL!=ps); assert(i<n); assert(w<n); assert(x<w);
    h = w>>1;
    s = window_start(n, i, w, wrap);
    e = window_end(n, i, w, wrap);
    j = s + x;
    for (; j >= n; j -= n) ;
    if (s > e) { /* window wraps around end of array */
        for (y=s; y<n; y++) {
            if (j==y) continue;
            ps[k++] = slope(pd, pz, n, j, y);
        }
        for (y=0UL; y<=e; y++) {
            if (j==y) continue;
            ps[k++] = slope(pd, pz, n, j, y);
        }
    } else { /* no wrapping of window */
        for (y=s; y<=e; y++) {
            if (j==y) continue;
            ps[k++] = slope(pd, pz, n, j, y);
#if DEBUG_CODE
            if (NULL != pz) {
                size_t d = slope_number(n, j, y);
                double m = slope(pd, NULL, n, j, y);
                if (m != ps[k-1UL]) (void)fprintf(stderr, "%s line %d: n=%lu, w=%lu, h=%lu, i=%lu, x=%lu, s=%lu, e=%lu, j=%lu, y=%lu, k was %lu, slope %.9g != %.9G, slope number %lu\n", __func__, __LINE__, n, w, h, i, x, s, e, j, y, k-1UL, ps[k-1UL], m, d);
            }
#endif
        }
    }
#if DEBUG_CODE
    (void)fprintf(stderr, "%s line %d: n=%lu, w=%lu, i=%lu, x=%lu, s=%lu, e=%lu, j=%lu: returning %lu\n", __func__, __LINE__, n, w, i, x, s, e, j, k);
#endif
    return k;
}

/* Compute all pairwise slopes for window size w for points in array pd, size n.
   Place results in array pz, size at least n*w.
*/
static MEDIAN_INLINE size_t compute_window_slopes(double *pd, size_t n, size_t w, double *pz, unsigned int wrap)
{
    size_t k = 0UL;

    assert(NULL!=pd); assert(w<n);
    if (NULL != pz) {
        size_t d, j, l, y, z;

        for (j=0UL,l=n-2UL; j<l; j++) {
            for (y=j+1UL,z=j+w; y<z; y++) {
                if (y>=n) break;
                d = slope_number(n, j, y);
                pz[d] = slope(pd, NULL, n, j, y);
                k++;
            }
        }
        if (0U != wrap) {
            for (j=n-w; j<n; j++) {
                for (y=0UL,z=j+w-n; y<=z; y++) {
                    d = slope_number(n, j, y);
                    pz[d] = slope(pd, NULL, n, j, y);
                    k++;
                }
            }
        }
    }
    return k;
}

/* place w intercepts for window w around point i, based on slope m, in array pintercepts */
static MEDIAN_INLINE size_t intercepts(double *pd, double *pintercepts, size_t n, size_t w, double m, size_t i, unsigned int wrap)
{
    size_t e, h, j, k=0UL, s;
    double x;

    assert(NULL!=pd); assert(NULL!=pintercepts); assert(w<n); assert(i<n);
    h = w>>1;
    s = window_start(n, i, w, wrap);
    e = window_end(n, i, w, wrap);
    /* y=mx+b, b=y-mx */
    if (s > e) { /* window wraps around end of array */
        for (j=s; j<n; j++) {
            if (i==j) x = 0.0;
            else if (j>i) x = (double)(j-i);
            else /* j<i */ x = 0.0 - (double)(i-j);
            pintercepts[k++] = pd[j] - m * x;
        }
        for (j=0UL; j<=e; j++) {
            if (i==j) x = 0.0;
            else if (j>i) x = (double)(j-i);
            else /* j<i */ x = 0.0 - (double)(i-j);
            pintercepts[k++] = pd[j] - m * x;
        }
    } else { /* no wrapping of window */
        for (j=s; j<=e; j++) {
            if (i==j) x = 0.0;
            else if (j>i) x = (double)(j-i);
            else /* j<i */ x = 0.0 - (double)(i-j);
            pintercepts[k++] = pd[j] - m * x;
        }
    }
    return k;
}

/* Filter data in array pd using a parametric repeated median filter with
      window half-width h.
   As a parametric approach is used, multi-dimensional data may be filtered per
       dimension.
       Independent variables may be filtered if the data might contain noise or
          outliers, or can remain unfiltered.
   Place filtered values in array pfiltered, slopes in pslopes.
   Arrays pd, pfiltered, plopes have size (at least) n.
   Auxiliary arrays px and py of size at least 2*h must be provided by the caller.
   Optional array pz of size 2*n*h may be provided to expedite slope computation.
   Data wraps if parameter wrap is nonzero.
*/
/* forward reference */
int repeated_median_filter(double *, double *, double *, size_t, double *, double *, size_t, double *, unsigned int);
/* definition */
int repeated_median_filter(double *pd, double *pfiltered, double *pslopes, size_t n, double *px, double *py, size_t h, double *pz, unsigned int wrap)
{
    size_t i, j, k, karray[2], marray[1], w; /* array sizes are sufficient for ranks for medians (lower-, upper-) for filtered data and slopes */
    double b, m;

#if DEBUG_CODE
    (void)fprintf(stderr, "%s line %d: n=%lu, h=%lu\n", __func__, __LINE__, n, h);
#endif
    assert(NULL!=pd); assert(NULL!=pfiltered); assert(NULL!=pslopes); assert(NULL!=px); assert(NULL!=py); assert(h<n);
    /* each point will have:
          [ w = 2*h+1 (therefore w is always an odd number; w-1 is even) ]
          w-1 pairwise slopes with nearby points for each of w points in the window (stored in array px)
          w inner medians (stored in array py)
             inner median is median of w-1 pairwise slopes in array px
             lower median is in px[(w-2)/2] = x[w/2-1]; karray[0] = w/2-1
             upper median is in px[(w-1)/2];            karray[1] = (w-1)/2 = h
          a median (filtered) slope (stored in m and in array pslopes)
             median slope is median of w inner median slopes in array py
             median is in py[(w-1)/2]; marray[0] = (w-1)/2 = h
          w intercepts (stored in array py)
             intercepts (at the point being filtered) are computed from the
                median slope, nearby point raw values, and the distance between
                the point being filtered and the nearby point (as index
                difference) 
          a median intercept (filtered value) (stored in b and in array pfiltered)
             median intercept is median of w intercepts in array py
             median is in py[(w-1)/2]; marray[0] = (w-1)/2 = h
       caller can optionally provide an additional array of size
          at least n*(w-1) for efficient slope computation
    */
    w = (h<<1)+1UL;
    i = (w>>1)-1UL;
    karray[0] = i;
    karray[1] = marray[0] = h;
    /* if an array is provided for all relevant pairwise slopes, compute unique slopes first */
    if (NULL != pz) { k = compute_window_slopes(pd, n, w, pz, wrap); }
    /* process all given points */
    for (i=0UL; i<n; i++) {
        for (j=0UL; j<w; j++) {
            k = pair_slopes(pd, px, n, w, pz, i, j, wrap);
#if DEBUG_CODE
            if (k != w-1UL) { (void) fprintf(stderr, "%s line %d: i=%lu, j=%lu, k=%lu, w=%lu\n", __func__, __LINE__, i, j, k, w); fflush(stderr); return 1; }
#endif
            /* (inner) median slope for this point */
            quickselect(px,k,sizeof(double),doublecmp,NULL,karray,2UL,0x07F8U);
            py[j] = (px[karray[0]]+px[karray[1]])/2.0;
#if DEBUG_CODE
            if (px[karray[1]] < px[karray[0]] - 1.0e-5) {
                (void)fprintf(stderr, "%s line %d: bad array px (inner slopes) order at indices %lu, %lu\n", __func__, __LINE__, karray[0], karray[1]);
                print_double_array(px, 0UL, w-2UL);
                return 1;
            }
#endif
#if DEBUG_CODE
            (void)fprintf(stderr, "%s line %d: i=%lu, j=%lu, inner median slope=%.9G\n", __func__, __LINE__, i, j, py[j]);
#endif
        }
        /* (outer) median slope of w inner median slopes for this point = slope for this point */
        quickselect(py,w,sizeof(double),doublecmp,NULL,marray,1UL,0x07F8U);
        m = py[marray[0]];
        if (NULL != pslopes) pslopes[i] = m;
#if DEBUG_CODE
        (void)fprintf(stderr, "%s line %d: i=%lu, outer median slope=%.9G\n", __func__, __LINE__, i, m);
#endif
        /* w intercepts for this point */
        k = intercepts(pd, py, n, w, m, i, wrap);
#if DEBUG_CODE
        if (k != w) { (void) fprintf(stderr, "%s line %d: i=%lu, j=%lu, k=%lu, w=%lu\n", __func__, __LINE__, i, j, k, w); fflush(stderr); return 1; }
#endif
        /* median intercept = filtered value */
        quickselect(py,k,sizeof(double),doublecmp,NULL,marray,1UL,0x07F8U);
        b = py[marray[0]];
        if (NULL != pfiltered) pfiltered[i] = b;
    }
    if ((char)0 == repeated_median_initialized) initialize_repeated_median();
    return 0;
}
