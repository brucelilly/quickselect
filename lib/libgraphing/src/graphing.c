/* Description: C source code for graphing support functions (for use with grap)
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    graphing.c copyright 2014-2017 Bruce Lilly.   \ graphing.c $
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
/* $Id: ~|^` @(#)   This is graphing.c version 2.5 2017-02-10T22:43:03Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "graphing" */
/*****************************************************************************/
/* maintenance note: master file  /data/projects/automation/940/lib/libgraphing/src/s.graphing.c */

/********************** Long description and rationale: ***********************
******************************************************************************/

/* source module identification */
/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: graphing.c ~|^` @(#)"
#define SOURCE_MODULE "graphing.c"
#define MODULE_VERSION "2.5"
#define MODULE_DATE "2017-02-10T22:43:03Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2014-2017"

/* configuration (which might affect feature test macros) */
/* to include a main entry point for testing, compile with -DTESTING=1 */
#ifndef TESTING
# define TESTING 0
#endif

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 600  /* code uses C99 'inline', XOPEN 'LONG_BIT' */
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

#include "graphing.h"           /* formatprecision formatwidth mintick maxtick prec rem intvl */
#include "snn.h"                /* sn1en snmagnitude snmultiple snsf */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <string.h>             /* strrchr */

/* imports */
/* XXX should use logger instead of global debug */
extern volatile int debug;      /* lemd.c */

/* static data and function definitions */
static char graphing_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void initialize_graphing(void)
{
    graphing_initialized
        = register_build_strings(NULL, &source_file, filenamebuf);
}

/* return fractional part of x * scale mod res
   scaled to a fraction of res
   round to at most 9 digits precision
*/
double rem(FILE *f, double x, double scale, double res)
{
    double d, e, r;

    e=x*scale;
    d=snmultiple(e,res,NULL,NULL);
    if (d>e) d-=res;
    r=e-d;
    if (0.0!=res) r/=res;
    if (r<sn1en(-9,NULL,NULL)) r=0.0;
    return r;
}

/* return number of digits of precision required to represent
      fractional part of d
*/
int prec(FILE *f, double d)
{
    int a=snmagnitude(d,NULL,NULL), b=snsf(d,NULL,NULL);
    if (0>a) a=0;
    if (d==0.0) b=0;
    return b-a;
}

/* return number of digits of precision required to represent
      fractional parts of low, high, and interval
*/
int formatprecision(FILE *f, double low, double high, double interval)
{
    int p, q;

    if ((char)0 == graphing_initialized)
        initialize_graphing();
    p = prec(f, interval);
    q = prec(f, low);
    if (q > p)
        p = q;
    q = prec(f, high);
    if (q > p)
        p = q;
    return p;
}

/* return total number of characters required to print number
      from low through high with prec digits of precision in fraction
*/
int formatwidth(FILE *f, double low, double high, int precision)
{
    int w, x;
    double d;

    if ((char)0 == graphing_initialized)
        initialize_graphing();
    w = precision + 1; /* 1 digit plus width of fraction (if any) */
    if (precision > 0)
        w++; /* decimal point */
    if (low < 0.0)
        w++; /* minus sign */
    d=low;
    if (0.0>d) d=0.0-d;
    if (high>d) d=high;
    x=snmagnitude(d,NULL,NULL);
    if (1>=x) x=0;
    w+=x;
    return w;
}

/* compute minimum tick value for data ranging from low to high with
      inherent resolution res and tick interval interval
*/
double mintick(FILE *f, double low, double high, double res, double interval, double llim, double ulim)
{
    double d, e;

    if (debug > 0) {
        if ((char)0 == graphing_initialized)
            initialize_graphing();
        (void)fprintf(f,
            "%s %s(low %g, high %g, res %g, interval %g, llim %g, ulim %g)%s",
            COMMENTSTART, __func__,
            low, high, res, interval, llim, ulim,
            COMMENTEND);
    }
    if (low < llim)
        low = llim; /* ignore data below lower limit */
    d=snmultiple(low-interval,res,NULL,NULL);
    if (d>low) d-=res;
    d=snmultiple(e=d,interval,NULL,NULL);
    if (d>e) d-=interval;
    if (d+interval<=low) d+=interval;
    if (low < d - res / 100.0)
        low = d - interval;
    else
        low = d;
    d=snmultiple(high+interval,res,NULL,NULL);
    if (d>high) d-=res;
    d=snmultiple(e=d,interval,NULL,NULL);
    if (d>e) d-=interval;
    if (high > d + res / 100.0)
        high = d + interval;
    else
        high = d;
    if (high > ulim)
        high = ulim; /* ignore data above upper limit */
    if ((low >= high) && (low > llim))
        low -= interval; /* if data spans less than one tick, expand scale to give two ticks if possible */
    return low;
}

/* compute maximum tick value for data ranging from low to high with
      inherent resolution res and tick interval interval
*/
double maxtick(FILE *f, double low, double high, double res, double interval, double llim, double ulim)
{
    double d, e;

    if (debug > 0) {
        if ((char)0 == graphing_initialized)
            initialize_graphing();
        (void)fprintf(f,
            "%s %s(low %g, high %g, res %g, interval %g, llim %g, ulim %g)%s",
            COMMENTSTART, __func__,
            low, high, res, interval, llim, ulim,
            COMMENTEND);
    }
    if (high > ulim)
        high = ulim; /* ignore data above upper limit */
    d=snmultiple(high+interval,res,NULL,NULL);
    if (d>high) d-=res;
    d=snmultiple(e=d,interval,NULL,NULL);
    if (d>e) d-=interval;
    if (high > d + res / 100.0)
        high = d + interval;
    else
        high = d;
    if (low < llim)
        low = llim; /* ignore data below lower limit */
    if ((low >= high) && (high < ulim))
        high += interval; /* if data spans less than one tick, expand scale to give two ticks if possible */
    return high;
}

/* test ratios */
static const double zarray[]=
    {1.20,1.25,1.5,2.0,2.5,3.0,4.0,5.0,6.0,8.0,9.0,10.0};
static int nz=sizeof(zarray)/sizeof(zarray[0]);

/* compute optimum tick interval for data ranging from low to high
      with inherent resolution res
*/
double intvl(FILE *f, double low, double high, double res, double llim, double ulim, int nticks, unsigned int subintvl)
{
    unsigned char done;
    int i, j;
    double d, g, mint, maxt, r, y, fz[20], z;

    if (debug > 0) {
        if ((char)0 == graphing_initialized)
            initialize_graphing();
        (void)fprintf(f,
            "%s %s(low %g, high %g, res %g, llim %g, ulim %g, nticks %d, subintvl %u)%s",
            COMMENTSTART, __func__,
            low, high, res, llim, ulim, nticks, subintvl,
            COMMENTEND);
    }
    if (nticks < 2)
        nticks = 2; /* avoid division by zero */
    /* try power of 10 giving between nticks/10 and nticks ticks */
    if (high > ulim)
        high = ulim; /* ignore data above upper limit */
    if (low < llim)
        low = llim; /* ignore data below lower limit */
    g = (high - low) / (nticks - 1); /* absolute lower limit on interval */
    if (debug > 0)
        (void)fprintf(f,
            "%s %s: guess %g for maximum %d tick%s %s %g %s %g%s",
            COMMENTSTART, __func__,
            g, nticks, nticks!=1? "s": "", subintvl==0U? "from": "between", low, subintvl==0U? "through": "and", high,
            COMMENTEND);
    if (g <= 0.0) {
        return res;
    } else if (g <= 1.0) {
        for (d=1.0; d > g; d /= 10.0) ;
    } else {
        for (d=1.0; d < g; d *= 10.0) ;
    }
    do {
        maxt = maxtick(f, low, high, res, d, llim, ulim);
        mint = mintick(f, low, high, res, d, llim, ulim);
        i=(int)snlround((maxt-mint)/d+1.0-(subintvl!=0U?2.0:0.0),NULL,NULL);
        if (debug > 0)
            (void)fprintf(f,
                "%s %s: power of 10 %g gives %d tick%s %s %g %s %g%s",
                COMMENTSTART, __func__,
                d, i, i!=1? "s": "", subintvl==0U? "from": "between", mint, subintvl==0U? "through": "and", maxt,
                COMMENTEND);
        if ((i <= 0) && (0U == subintvl))
            return res;
        if (i > nticks)
            d *= 10.0;
        else if (i < nticks / 10)
            d /= 10.0;
    } while ((i > nticks) || (i < nticks / 10));
    /* if the tick interval is smaller than the inherent data resolution, increase the tick interval */
    while (d < res) {
        d *= 10.0;
        maxt = maxtick(f, low, high, res, d, llim, ulim);
        mint = mintick(f, low, high, res, d, llim, ulim);
        i=(int)snlround((maxt-mint)/d+1.0-(subintvl!=0U?2.0:0.0),NULL,NULL);
    }
    if (debug > 0)
        (void)fprintf(f,
            "%s %s: revised power of 10 %g gives %d tick%s %s %g %s %g%s",
            COMMENTSTART, __func__,
            d, i, i!=1? "s": "", subintvl==0U? "from": "between", mint, subintvl==0U? "through": "and", maxt,
            COMMENTEND);
    /* a smaller power of 10 will give too many ticks, but is the basis for the eventual tick interval */
    y = d / 10.0;
    d = y;
    maxt = maxtick(f, low, high, res, d, llim, ulim);
    mint = mintick(f, low, high, res, d, llim, ulim);
    i=(int)snlround((maxt-mint)/d+1.0-(subintvl!=0U?2.0:0.0),NULL,NULL);
    if (debug > 0)
        (void)fprintf(f,
            "%s %s: basis power of 10 %g for resolution %g gives %d tick%s %s %g %s %g%s",
            COMMENTSTART, __func__,
            d, res, i, i!=1? "s": "", subintvl==0U? "from": "between", mint, subintvl==0U? "through": "and", maxt,
            COMMENTEND);
    /* try factors of y until a suitable tick interval is found */
    /* use only values near integral multiples of inherent resolution for nice match to data and to facilitate subdivision */
    for (i=j=0; i<nz; i++) {
        z=zarray[i];
        r=rem(f,z,y,res);
        if ((r<0.105)||(r>0.905))
            fz[++j]=z;
    }
    /* ensure at least one value to test */
    if (j < 1) {
        z = res;
        if (z > 10.0) {
            for (; z > 10.0; z /= 10.0) ;
        } else if (z <= 1.0) {
            for (; z <= 1.0; z *= 10.0) ;
        }
        fz[++j] = z;
    }
    fz[++j] = 10 * fz[1]; /* sentinel value > 10.0 used to stop loop */
    done = 0U; /* loop completed flag */
    for (j=1;
    ((fz[j] <= 10.0) && ((0U == done) || (i > nticks) || (res >= fz[j+1] * y)));
    j++) {
        d = fz[j] * y;
        if ((d >= g * 0.9999) && (res <= fz[j+1] * y)) { /* handle roundoff issues */
            maxt = maxtick(f, low, high, res, d, llim, ulim);
            mint = mintick(f, low, high, res, d, llim, ulim);
            i=(int)snlround((maxt-mint)/d+1.0-(subintvl!=0U?2.0:0.0),NULL,NULL);
            if (debug > 0) {
                (void)fprintf(f,
                    "%s %s: d = %g next %g, maxt = %g, mint = %g, i = %d%s",
                    COMMENTSTART, __func__,
                    d, fz[j+1] * y, maxt, mint, i,
                    COMMENTEND);
            }
            if ((0U == subintvl) || ((mint >= llim * 0.999) && (maxt <= ulim * 1.001))) {
                /* avoid values which are not near an integral multiple of inherent resolution */
                r = rem(f, d, 1.0, res);
                if ((r < 0.105) || (r > 0.905)) {
                    /* avoid values which do not divide interval into equal pieces */
                    r = rem(f, maxt-mint, 1.0, d);
                    if ((r < 0.105) || (r > 0.905)) {
                        done = 1U;
                    }
                }
            }
        }
        else
            if (debug > 0) {
                (void)fprintf(f,
                    "%s %s: g = %g; d = %g next %g%s",
                    COMMENTSTART, __func__,
                    g, d, fz[j+1] * y,
                    COMMENTEND);
            }
        if ((debug > 0) && (0U == done)) {
            if (d < g)
                (void)fprintf(f,
                    "%s %s: skipping interval %g = %g * %g < %g%s",
                    COMMENTSTART, __func__,
                    d, fz[j], y, g,
                    COMMENTEND);
            if (res > fz[j+1] * y)
                (void)fprintf(f,
                    "%s %s: skipping interval %g = %g * %g because res %g > %g * %g%s",
                    COMMENTSTART, __func__,
                    d, fz[j], y, res, fz[j+1], y,
                    COMMENTEND);
            if ((r >= 0.105) && (r <= 0.905))
                (void)fprintf(f,
                    "%s %s: skipping interval %g = %g * %g because %g %% res %g = %g%s",
                    COMMENTSTART, __func__,
                    d, fz[j], y, d, res, r,
                    COMMENTEND);
            if ((0U != subintvl) && ((mint < llim * 0.999) || (maxt > ulim * 1.001)))
                (void)fprintf(f,
                    "%s %s: skipping subinterval %g = %g * %g because %g != llim %g or %g != ulim %g%s",
                    COMMENTSTART, __func__,
                    d, fz[j], y, mint, llim, maxt, ulim,
                    COMMENTEND);
        }
    }
    if (debug > 0) {
        maxt = maxtick(f, low, high, res, d, llim, ulim);
        mint = mintick(f, low, high, res, d, llim, ulim);
        i=(int)snlround((maxt-mint)/d+1.0-(subintvl!=0U?2.0:0.0),NULL,NULL);
        (void)fprintf(f,
            "%s %s: revised interval %g gives %d tick%s %s %g %s %g for data between %g and %g%s",
            COMMENTSTART, __func__,
            d, i, i!=1? "s": "", subintvl==0U? "from": "between", mint, subintvl==0U? "through": "and", maxt, low, high,
            COMMENTEND);
    }
    return d;
}

#if TESTING
volatile int debug = 1;
char *prog;
int main(int argc, char **argv)
{
    char *p;
    int nticks;
    double high, interval, llim, low, mint, maxt, res, subinterval, ulim;

    prog = argv[0];
    p = strrchr(prog, '/');
    if (NULL != p) {
        prog = p+1;
    }
    llim = 0.0;
    ulim = 220.0;
    low = 114.7;
    high = 160.5;
    res = 0.1;
    nticks = 12;
    interval = intvl(stderr, low, high, res, llim, ulim, nticks, 0U);
    mint = mintick(stderr, low, high, res, interval, llim, ulim);
    maxt = maxtick(stderr, low, high, res, interval, llim, ulim);
    printf("%s major ticks from %g through %g by %g for data between %g and %g%s",
        COMMENTSTART,
        mint, maxt, interval, low, high,
        COMMENTEND);
    if (interval > 1.9 * res) {
        subinterval = intvl(stderr, 0.0, interval, res, 0.0, interval, 6, 1U);
        if (interval > subinterval) {
            int i, j, k;
            double delta, nd, r;

            printf("%s subinterval %g between interval %g major ticks%s",
                COMMENTSTART,
                subinterval, interval,
                COMMENTEND);
        }
    }
    return 0;
}
#endif
