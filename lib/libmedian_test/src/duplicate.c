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
* $Id: ~|^` @(#)    duplicate.c copyright 2016-2018 Bruce Lilly.   \ duplicate.c $
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
/* $Id: ~|^` @(#)   This is duplicate.c version 1.8 dated 2018-02-06T19:58:29Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.duplicate.c */

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
#define ID_STRING_PREFIX "$Id: duplicate.c ~|^` @(#)"
#define SOURCE_MODULE "duplicate.c"
#define MODULE_VERSION "1.8"
#define MODULE_DATE "2018-02-06T19:58:29Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "initialize_src.h"

/* duplicate (as far as possible) long data in larray to another array */
/* n items; source starts at index 0UL, copies at index o, increment by ratio r */
void duplicate_test_data(long *refarray, char *pv, int type, size_t r, size_t o, size_t n)
{
    long f, l;
    int i;
    size_t j;
    double d;
    unsigned int u;
    time_t t; /* time_t might have greater range than long on 32-bit *BSD */
    char *p;
    struct civil_time_struct cts;
    struct data_struct *pds;

    if ((char)0==file_initialized) initialize_file(__FILE__);
    f=FRACTION_COUNT;
    for (j=0UL; j<n; j++) {
        l = refarray[j];
        switch (type) {
            case DATA_TYPE_UINT_LEAST8_T :
                ((uint_least8_t *)pv)[o+r*j] = l % 0x0FFL;
            break;
            case DATA_TYPE_UINT_LEAST16_T :
                ((uint_least16_t *)pv)[o+r*j] = l % 0x0FFFFL;
            break;
            case DATA_TYPE_UINT_LEAST32_T :
                ((uint_least32_t *)pv)[o+r*j] = (uint_least64_t)l % 0x0FFFFFFFFUL;
            break;
            case DATA_TYPE_UINT_LEAST64_T :
                ((uint_least64_t *)pv)[o+r*j] = l;
            break;
            case DATA_TYPE_LONG :
                ((long *)pv)[o+r*j] = l;
            break;
            case DATA_TYPE_INT :
                ((int *)pv)[o+r*j] = l % INT_MAX;
            break;
            case DATA_TYPE_SHORT :
                ((short *)pv)[o+r*j] = l % SHRT_MAX;
            break;
            case DATA_TYPE_DOUBLE :
                ((double *)pv)[o+r*j] = (double)l;
            break;
            case DATA_TYPE_STRUCT :
            /*FALLTHROUGH*/
            case DATA_TYPE_STRING :
            /*FALLTHROUGH*/
            case DATA_TYPE_POINTER :
                switch (type) {
                    case DATA_TYPE_POINTER :
                        pds=((struct data_struct **)pv)[o+r*j];
                    break;
                    default :
                        pds=&(((struct data_struct *)pv)[o+r*j]);
                    break;
                }
                t = (time_t)l/(time_t)f;
                gmtime_r(&t, &(cts.tm));
                d=(double)(l%f)/(double)f; /* fraction of a second */
                cts.fraction=d;
                cts.offset=0.0;
                (V)sn_civil_time(pds->string,STRING_SIZE,&cts,-12,1,0,
                    NULL,NULL);
                p=strchr(pds->string,'Z');
                if (NULL!=p) *p='\0'; /* avoid comparisons of 'Z' vs. digits */
#if ASSERT_CODE
                t = utc_mktime(&(cts.tm),NULL,NULL);
                if (t != (time_t)l/(time_t)f)
                    (V)fprintf(stderr,
                        "// %s line %d: %ld -> %s (%04d-%02d-%02dT%02d:%02d:"
                        "%02dZ) -> %ld\n",__func__,__LINE__,l,pds->string,
                        cts.tm.tm_year+1900,cts.tm.tm_mon+1,cts.tm.tm_mday,
                        cts.tm.tm_hour,cts.tm.tm_min,cts.tm.tm_sec,(long)t);
                A(t==(time_t)l/(time_t)f);
#endif /* ASSERT_CODE */
                pds->year=cts.tm.tm_year+1900;
                pds->month=cts.tm.tm_mon+1;
                pds->yday=cts.tm.tm_yday+1;
                pds->mday=cts.tm.tm_mday;
                pds->hour=cts.tm.tm_hour;
                pds->minute=cts.tm.tm_min;
                pds->second=cts.tm.tm_sec;
                for (i=0; i<6; i++) {
                    d *= 100.0;
                    u = (unsigned int)d;
                    pds->fractional[i]=(char)u;
                    d -= (double)u;
                }
                /* The split of available bits between date-time with integral
                   seconds and fractional seconds (determined by the value of
                   variable f for 32- and 64-bit platforms) is such that
                   theoretically the 10^-12 component (fractional[5]) should
                   always be zero.  In practice, even using double-precision
                   floating-point arithmetic, there is a rounding error that
                   results in a value of 99 picoseconds for that component (on
                   64-bit platforms).  The following code block corrects that
                   situation.
                */
                if (99U==pds->fractional[5]) {
                    pds->fractional[5]++; /* round up */
                    for (i=5; i>0; i--) {
                        if (100U==pds->fractional[i]) {
                            pds->fractional[i-1]++;
                            pds->fractional[i]=0U;
                        }
                    }
                    /* The following should never happen... */
                    if (100U==pds->fractional[0]) {
                        pds->fractional[0]=0U;
                        pds->second++;
                        /* Because this is not expected to happen, normalization
                           of the remaining fields (in case second overflows to
                           60) is unimplemented... [handling overflow at the end
                           of a month, and dealing with leap years are somewhat
                           cumbersome]
                        */
                    }
                }
            break;
            default :
                (V)fprintf(stderr,
                   "%s: %s line %d: qsort type %d not handled in switch cases\n",
                   __func__,source_file,__LINE__,type);
            return;
        }
    }
}

void restore_test_data(size_t r, size_t o, size_t n, long *refarray, char *pv, int type)
{
    duplicate_test_data(refarray, pv, type, r, o, n);
}
