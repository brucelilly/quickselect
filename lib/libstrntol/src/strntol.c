/* Description: strntol - convert up to specified number of string characters to a long integer
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    strntol.c copyright 2013-2017 Bruce Lilly.   \ strntol.c $
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
/* $Id: ~|^` @(#)   This is strntol.c version 2.9 2017-02-16T13:14:47Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "strntol" */
/*****************************************************************************/
/* maintenance note: master file  /src/relaymail/lib/libstrntol/src/s.strntol.c */

/********************** Long description and rationale: ***********************
* nstrtol (should be named strntol, but that is a reserved name) is like strtol
* but uses a specified maximum number of digits.  Leading whitespace and
* optional sign characters are not included in the count of digits, but the
* sign is of course used to produce the result.  If base is specified as zero,
* a leading '0' is not counted but if present sets the conversion base to 8
* (octal).  If base is specified as zero, a leading "0x" or "0X" is not counted
* but if present sets the conversion base to 16 (hexadecimal).  e.g.
* nstrtol("  +19850412232050", 4, ep, 10) returns 1985 with ep pointing to
* the '0' before the '4'
******************************************************************************/

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant,
   other components are version control fields
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: strntol.c ~|^` @(#)"
#define SOURCE_MODULE "strntol.c"
#define MODULE_VERSION "2.9"
#define MODULE_DATE "2017-02-16T13:14:47Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2013-2017"

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


/* local header files */
#include "strntol.h"            /* includes stddef.h */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <ctype.h>              /* isspace, isalnum */
#include <errno.h>              /* errno EINVAL ERANGE */
#include <limits.h>             /* LONG_MIN LONG_MAX */
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

#define STRNTOL_MAX_BASE 36
#define STRNTOL_TBL_SIZE UCHAR_MAX

/* static data and function definitions */
/* Table containing values associated with input characters.
   Input (unsigned) characters are used as an index into the table.
   Values range from zero to MAX_BASE (a sentinel value),
      so char is a sufficiently large integral type.
      (irrelevant whining from lint and/or splint notwithstanding)
*/
static char nstrtol_tbl[STRNTOL_TBL_SIZE];      /* RATS: ignore (large enough to index by any unsigned char) */
static char nstrtol_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;

static void nstrtol_initialize(void)
{
    const char *s;
    unsigned int i;

    for (i=0; i<= (int)(UCHAR_MAX); i++) {
        switch ((unsigned char)i) {
            case (unsigned char)'0':
                nstrtol_tbl[(unsigned char)i] = (char)0;
            break;
            case (unsigned char)'1':
                nstrtol_tbl[(unsigned char)i] = (char)1;
            break;
            case (unsigned char)'2':
                nstrtol_tbl[(unsigned char)i] = (char)2;
            break;
            case (unsigned char)'3':
                nstrtol_tbl[(unsigned char)i] = (char)3;
            break;
            case (unsigned char)'4':
                nstrtol_tbl[(unsigned char)i] = (char)4;
            break;
            case (unsigned char)'5':
                nstrtol_tbl[(unsigned char)i] = (char)5;
            break;
            case (unsigned char)'6':
                nstrtol_tbl[(unsigned char)i] = (char)6;
            break;
            case (unsigned char)'7':
                nstrtol_tbl[(unsigned char)i] = (char)7;
            break;
            case (unsigned char)'8':
                nstrtol_tbl[(unsigned char)i] = (char)8;
            break;
            case (unsigned char)'9':
                nstrtol_tbl[(unsigned char)i] = (char)9;
            break;
            case (unsigned char)'A':
            /*FALLTHROUGH*/
            case (unsigned char)'a':
                nstrtol_tbl[(unsigned char)i] = (char)10;
            break;
            case (unsigned char)'B':
            /*FALLTHROUGH*/
            case (unsigned char)'b':
                nstrtol_tbl[(unsigned char)i] = (char)11;
            break;
            case (unsigned char)'C':
            /*FALLTHROUGH*/
            case (unsigned char)'c':
                nstrtol_tbl[(unsigned char)i] = (char)12;
            break;
            case (unsigned char)'D':
            /*FALLTHROUGH*/
            case (unsigned char)'d':
                nstrtol_tbl[(unsigned char)i] = (char)13;
            break;
            case (unsigned char)'E':
            /*FALLTHROUGH*/
            case (unsigned char)'e':
                nstrtol_tbl[(unsigned char)i] = (char)14;
            break;
            case (unsigned char)'F':
            /*FALLTHROUGH*/
            case (unsigned char)'f':
                nstrtol_tbl[(unsigned char)i] = (char)15;
            break;
            case (unsigned char)'G':
            /*FALLTHROUGH*/
            case (unsigned char)'g':
                nstrtol_tbl[(unsigned char)i] = (char)16;
            break;
            case (unsigned char)'H':
            /*FALLTHROUGH*/
            case (unsigned char)'h':
                nstrtol_tbl[(unsigned char)i] = (char)17;
            break;
            case (unsigned char)'I':
            /*FALLTHROUGH*/
            case (unsigned char)'i':
                nstrtol_tbl[(unsigned char)i] = (char)18;
            break;
            case (unsigned char)'J':
            /*FALLTHROUGH*/
            case (unsigned char)'j':
                nstrtol_tbl[(unsigned char)i] = (char)19;
            break;
            case (unsigned char)'K':
            /*FALLTHROUGH*/
            case (unsigned char)'k':
                nstrtol_tbl[(unsigned char)i] = (char)20;
            break;
            case (unsigned char)'L':
            /*FALLTHROUGH*/
            case (unsigned char)'l':
                nstrtol_tbl[(unsigned char)i] = (char)21;
            break;
            case (unsigned char)'M':
            /*FALLTHROUGH*/
            case (unsigned char)'m':
                nstrtol_tbl[(unsigned char)i] = (char)22;
            break;
            case (unsigned char)'N':
            /*FALLTHROUGH*/
            case (unsigned char)'n':
                nstrtol_tbl[(unsigned char)i] = (char)23;
            break;
            case (unsigned char)'O':
            /*FALLTHROUGH*/
            case (unsigned char)'o':
                nstrtol_tbl[(unsigned char)i] = (char)24;
            break;
            case (unsigned char)'P':
            /*FALLTHROUGH*/
            case (unsigned char)'p':
                nstrtol_tbl[(unsigned char)i] = (char)25;
            break;
            case (unsigned char)'Q':
            /*FALLTHROUGH*/
            case (unsigned char)'q':
                nstrtol_tbl[(unsigned char)i] = (char)26;
            break;
            case (unsigned char)'R':
            /*FALLTHROUGH*/
            case (unsigned char)'r':
                nstrtol_tbl[(unsigned char)i] = (char)27;
            break;
            case (unsigned char)'S':
            /*FALLTHROUGH*/
            case (unsigned char)'s':
                nstrtol_tbl[(unsigned char)i] = (char)28;
            break;
            case (unsigned char)'T':
            /*FALLTHROUGH*/
            case (unsigned char)'t':
                nstrtol_tbl[(unsigned char)i] = (char)29;
            break;
            case (unsigned char)'U':
            /*FALLTHROUGH*/
            case (unsigned char)'u':
                nstrtol_tbl[(unsigned char)i] = (char)30;
            break;
            case (unsigned char)'V':
            /*FALLTHROUGH*/
            case (unsigned char)'v':
                nstrtol_tbl[(unsigned char)i] = (char)31;
            break;
            case (unsigned char)'W':
            /*FALLTHROUGH*/
            case (unsigned char)'w':
                nstrtol_tbl[(unsigned char)i] = (char)32;
            break;
            case (unsigned char)'X':
            /*FALLTHROUGH*/
            case (unsigned char)'x':
                nstrtol_tbl[(unsigned char)i] = (char)33;
            break;
            case (unsigned char)'Y':
            /*FALLTHROUGH*/
            case (unsigned char)'y':
                nstrtol_tbl[(unsigned char)i] = (char)34;
            break;
            case (unsigned char)'Z':
            /*FALLTHROUGH*/
            case (unsigned char)'z':
                nstrtol_tbl[(unsigned char)i] = (char)35;
            break;
            default:
                nstrtol_tbl[(unsigned char)i] = (char)(STRNTOL_MAX_BASE);
            break;
        }
    }
    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    nstrtol_initialized = register_build_strings(NULL, &source_file, s);
}

/* convert up to n characters to a long integer, save reject */
long nstrtol(const char *
#if defined(__STDC__)&&( __STDC_VERSION__ >= 199901L) /* restrict restricted */
                         restrict
#endif
                         s, size_t n, char **
#if defined(__STDC__)&&( __STDC_VERSION__ >= 199901L) /* restrict restricted */
                                              restrict
#endif
                                              endptr, int base,
    void (*f)(int, void *, const char *, ...), void *log_arg)
{
    long ret = 0L;
#ifndef PP__FUNCTION__
    static const char __func__[] = "nstrtol";
#endif

    if ((NULL != s) && (0 != n)) {
        int i = 0; /* digit value */
        int sign = 0; /* 0 for +, -1 for - (difference = LONG_MAX + LONG_MIN) */
        const char *p = s;

        if ((1 == base) || (STRNTOL_MAX_BASE < base) || (0 > base)) {/*0 is OK*/
            if (NULL != endptr)
                *endptr = s;
            if (NULL != f) {
                f(LOG_ERR, log_arg,
                    "%s: %s line %d: invalid base %d",
                    __func__, source_file, __LINE__,
                    base
                    );
            }
            errno = EINVAL;
            return ret;
        }
        if ((char)0 == nstrtol_initialized)
            nstrtol_initialize();
        while (0 != isspace((int)(*s)))
            s++;
        if ('+' == *s)
            s++;
        else if ('-' == *s) {
            s++;
            sign = -1;
        }
        if (0 == base) {
            if ('0' == *s) {
                s++;
                if (('x' == *s) || ('X' == *s)) {
                    base = 16;
                    s++;
                } else
                    base = 8;
            } else
                base = 10;
        } else if (8 == base) {
            if ('0' == *s)
                s++;
        } else if (16 == base) {
            if (('0' == *s) && (('x' == s[1]) || ('X' == s[1])))
                s += 2;
        }
        while (0 != isalnum((int)(*s)))
            if ((0 == n) || ((i = (int)(nstrtol_tbl[(unsigned char)(*s)])) >= base))
                break;
            /* LONG_MIN has larger magnitude than LONG_MAX 
               ret is negative here because of greater LONG_MIN magnitude
                  for negative input (sign==-1):
                  base * ret - i < LONG_MIN
                  base * ret < LONG_MIN + i
                  ret < (LONG_MIN + i) / base
                  for positive input (sign==0):
                  ret < (LONG_MIN + i + 1) / base
                  taking sign into account:
                  ret < (LONG_MIN + i + 1 + sign) / base
            */
            else if (ret < (((LONG_MIN) + i + 1 + sign) / base)) {
                if (NULL != f) {
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: %s(\"%s\", %u, '%c', %d): sign %d, i %d, ret %ld",
                        __func__, source_file, __LINE__,
                        __func__, p, (unsigned)n, *s, base,
                        sign, i, ret
                        );
                    f(LOG_DEBUG, log_arg,
                        "%s: %s line %d: %s(\"%s\", %u, '%c', %d): ret(%ld) < (LONG_MIN(%ld) + i(%d) + 1 + sign(%d)) / base(%d) [%ld]",
                        __func__, source_file, __LINE__,
                        __func__, p, (unsigned)n, *s, base,
                        ret,
                        LONG_MIN, i, sign, base,
                        ((LONG_MIN) + i + 1 + sign) / base
                        );
                }
                ret = ((0>sign)? LONG_MIN : LONG_MAX );
                errno = ERANGE;
                break;
            } else
                n--, s++, ret = base * ret - i;
        if ((0 <= sign) && (0L > ret))
            ret = 0L - ret;
    }
    if (NULL != endptr)
        *endptr = s;
    return ret;
}
