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
* $Id: ~|^` @(#)    parse.c copyright 2016-2018 Bruce Lilly.   \ parse.c $
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
/* $Id: ~|^` @(#)   This is parse.c version 1.8 dated 2018-08-13T07:24:45Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "median_test" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian_test/src/s.parse.c */

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
#define ID_STRING_PREFIX "$Id: parse.c ~|^` @(#)"
#define SOURCE_MODULE "parse.c"
#define MODULE_VERSION "1.8"
#define MODULE_DATE "2018-08-13T07:24:45Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2016-2018"

/* local header files needed */
#include "median_test_config.h" /* configuration */ /* includes all other local and system header files required */

#include "pi.h"                 /* pi */

#include "initialize_src.h"

static
QUICKSELECT_INLINE
double mult_k_ki(int p)
{
    double n=1000.0;

    switch (p) {
        case 'i' : /*FALLTHROUGH*/
        case 'I' :
            n = 1024.0;
        break;
    }
    return n;
}

static char *match(char *p)
{
    size_t n;
    for (p++,n=1UL; n>0UL; p++) {
        switch(*p) {
            case '(' : n++;
            break;
            case ')' : n--;
            break;
        }
    }
    return --p;
}

double parse_num(const char *p, char **pendptr, int base)
{
    char buf[4096], *q;
    int i;
    double n;

    if ((char)0==file_initialized) initialize_file(__FILE__);
#if DEBUG_CODE
    if (DEBUGGING(SUPPORT_DEBUG))
        (V)fprintf(stderr,
            "%s line %d: %s(\"%s\", %p, %d)\n",
            __func__,__LINE__,__func__,p,(void *)pendptr,base);
#endif
    if (isdigit(*p)) n=strtod(p,pendptr);
    else if (0==strncasecmp(p,"sqrt(",i=5)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=sqrt(n);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"ln(",i=3)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=log(n);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"log(",i=4)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=log(n);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"log2(",i=5)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=log2(n);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"log10(",i=6)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=log10(n);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"exp(",i=4)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=exp(n);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"int(",i=4)) {
        strncpy(buf,p+i-1,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        n=(double)snlround(n,NULL,NULL);
        *pendptr=p+i+(q-buf);
    } else if (0==strncasecmp(p,"e",i=1)) {
        n=exp(1.0);
        p+=1;
        *pendptr=p;
    } else if (0==strncasecmp(p,"pi",i=2)) {
        n=pi;
        p+=2;
        *pendptr=p;
    } else if ('('==*p) {
        strncpy(buf,p,sizeof(buf)-1);
        q=match(buf);
        *q='\0';
        n=parse_expr(n,buf+1,pendptr,10);
        *pendptr=p+1+(q-buf);
    }
#if DEBUG_CODE
    if (DEBUGGING(SUPPORT_DEBUG))
        (V)fprintf(stderr,
            "%s line %d: n=%G, *pendptr=\"%s\"\n",
            __func__,__LINE__,n,*pendptr);
#endif
    switch (**pendptr) {
        case 't' : /*FALLTHROUGH*/
        case 'T' :
            n *= mult_k_ki((*pendptr)[1]);
        /*FALLTHROUGH*/
        case 'g' : /*FALLTHROUGH*/
        case 'G' :
            n *= mult_k_ki((*pendptr)[1]);
        /*FALLTHROUGH*/
        case 'm' : /*FALLTHROUGH*/
        case 'M' :
            n *= mult_k_ki((*pendptr)[1]);
        /*FALLTHROUGH*/
        case 'k' : /*FALLTHROUGH*/
        case 'K' :
            n *= mult_k_ki((*pendptr)[1]);
            switch ((*pendptr)[1]) {
                case 'i' : /*FALLTHROUGH*/
                case 'I' :
                    (*pendptr)++;
                break;
            }
            (*pendptr)++;
        break;
        default :
        break;
    }
#if DEBUG_CODE
    if (DEBUGGING(SUPPORT_DEBUG))
        (V)fprintf(stderr,
            "%s line %d: n=%G, *pendptr=\"%s\"\n",
            __func__,__LINE__,n,*pendptr);
#endif
    return n;
}

QUICKSELECT_INLINE
double parse_expr(double n, const char *p, char **pendptr, int base)
{
    double n2;

#if DEBUG_CODE
    if (DEBUGGING(SUPPORT_DEBUG))
        (V)fprintf(stderr,
            "%s line %d: %s(%G, \"%s\", %p, %d)\n",
            __func__,__LINE__,__func__,n,p,(void *)pendptr,base);
#endif
    if ((NULL!=p)&&((0!=isalnum(*p))||('('==*p))) {
        n = parse_num(p, pendptr, base);
    } else
        *pendptr=p;
#if DEBUG_CODE
    if (DEBUGGING(SUPPORT_DEBUG))
        (V)fprintf(stderr,
            "%s line %d: n=%G, *pendptr=\"%s\"\n",__func__,__LINE__,n,*pendptr);
#endif
    while ('\0' != **pendptr) {
        switch (**pendptr) {
            case '+' :
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                n += n2;
            break;
            case '-' :
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                n -= n2;
            break;
            case '*' :
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                n *= n2;
            break;
            case '/' :
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                if (0.0!=n2) n /= n2;
            break;
            case '%' :
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                if (0.0!=n2) n=fmod(n,n2);
            break;
            case '@' :
                p = *pendptr;
                n2 = (double)snlround(n,NULL,NULL);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                n=n2;
                *pendptr=++p;
            break;
            case '^' :
                p = *pendptr;
                n2 = parse_num(++p, pendptr, 10);
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n2=%G\n", __func__,__LINE__,n2);
#endif
                if (0.0<=n) n=pow(n,n2);
            break;
            default :
#if DEBUG_CODE
                if (DEBUGGING(SUPPORT_DEBUG))
                    (V)fprintf(stderr,
                        "%s line %d: n=%G, *pendptr=\"%s\"\n",
                        __func__,__LINE__,n,*pendptr);
#endif
            return n;
        }
#if DEBUG_CODE
        if (DEBUGGING(SUPPORT_DEBUG))
            (V)fprintf(stderr,
                "%s line %d: n=%G, *pendptr=\"%s\"\n",__func__,__LINE__,n,*pendptr);
#endif
    }
#if DEBUG_CODE
    if (DEBUGGING(SUPPORT_DEBUG))
        (V)fprintf(stderr,
            "%s line %d: n=%G, *pendptr=\"%s\"\n",
            __func__,__LINE__,n,*pendptr);
#endif
    return n;
}
