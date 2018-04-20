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
/* $Id: ~|^` @(#)   This is duplicate.c version 1.9 dated 2018-04-19T20:38:04Z. \ $ */
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
#define MODULE_VERSION "1.9"
#define MODULE_DATE "2018-04-19T20:38:04Z"
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
                t = (time_t)l/(time_t)f; /* l = seconds */
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
                pds->preamble[0]=0U;
                /* bit 0=0; no extension */
                pds->preamble[0]|=0x50U; /* bits 1-3=101; CCS time code */
                /* bit 4=0; month/day variation */
                pds->preamble[0]|=0x06U; /* bits 5-7=110; 1e-12 resolution */
                l-=(long)t*f; /* fraction increments */
                /* resolution limited to no finer than nanoseconds
                   (see median_test_config.h) due to 32-bit LONG_MAX
                */
                pds->year=cts.tm.tm_year+1900;
                if (0U==(pds->preamble[0]&0x080U)) { /* bit 4: variation */
                    pds->u_var.s_md.month=cts.tm.tm_mon+1;
                    pds->u_var.s_md.mday=cts.tm.tm_mday;
                } else {
                    pds->u_var.yday=cts.tm.tm_yday+1;
                }
                pds->hour=cts.tm.tm_hour;
                pds->minute=cts.tm.tm_min;
                pds->second=cts.tm.tm_sec;
                /* pack fractional seconds as pairs of BCD digits
                   CCSDS 301.0-B-4 section 3.4.1
                */
                l*=1000000000L/FRACTION_COUNT; /* nanoseconds */ 
                u=(unsigned int)(l/100000000L); /* tenths */
                pds->fractional[0]=((((unsigned char)u)&0x0f)<<4);
                l-=(long)u*100000000L;
                u=(unsigned int)(l/10000000L); /* hundredths */
                pds->fractional[0]|=(((unsigned char)u)&0x0f);
                l-=(long)u*10000000L;
                u=(unsigned int)(l/1000000L); /* thousandths */
                pds->fractional[1]=((((unsigned char)u)&0x0f)<<4);
                l-=(long)u*1000000L;
                u=(unsigned int)(l/100000L); /* ten-thousandths */
                pds->fractional[1]|=(((unsigned char)u)&0x0f);
                l-=(long)u*100000L;
                u=(unsigned int)(l/10000L); /* hundred-thousandths */
                pds->fractional[2]=((((unsigned char)u)&0x0f)<<4);
                l-=(long)u*10000L;
                u=(unsigned int)(l/1000L); /* millionths */
                pds->fractional[2]|=(((unsigned char)u)&0x0f);
                l-=(long)u*1000L;
                u=(unsigned int)(l/100L); /* ten-millionths */
                pds->fractional[3]=((((unsigned char)u)&0x0f)<<4);
                l-=(long)u*100L;
                u=(unsigned int)(l/10L); /* hundred-millionths */
                pds->fractional[3]|=(((unsigned char)u)&0x0f);
                l-=(long)u*10L;
                u=(unsigned int)l; /* nanoseconds */
                pds->fractional[4]=((((unsigned char)u)&0x0f)<<4);
                pds->fractional[5]=0U;
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
