/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: header file for get_ips
*/
#ifndef GET_IPS_H_INCLUDED
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    get_ips.h copyright 2010-2017 Bruce Lilly.   \ $
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
/* $Id: ~|^` @(#)   This is get_ips.h version 1.7 2017-09-01T02:01:09Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "get_ips" */
/*****************************************************************************/
/* maintenance note: master file  /src/radioclk/radioclk-1.0/lib/libid/include/s.get_ips.h */

/* version-controlled header file version information */
#define GET_IPS_H_VERSION "get_ips.h 1.7 2017-09-01T02:01:09Z"

/* feature test macros defined before any header files are included */
/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600 /* >=600 for illumos */
#else
# define MAX_XOPEN_SOURCE_VERSION 500 /* <=500 for illumos */
#endif

#ifndef _XOPEN_SOURCE
# ifdef MIN_XOPEN_SOURCE_VERSION
#  define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
# else
#  ifdef MAX_XOPEN_SOURCE_VERSION
#   define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#  endif
# endif
#endif
#if defined(_XOPEN_SOURCE) && defined(MIN_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && defined(MAX_XOPEN_SOURCE_VERSION) && ( _XOPEN_SOURCE > MAX_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MAX_XOPEN_SOURCE_VERSION
#endif

#define GET_IPS_H_POSIX_C_SOURCE 200112L
#if ! defined(_POSIX_C_SOURCE) || (GET_IPS_H_POSIX_C_SOURCE > _POSIX_C_SOURCE)
# ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
# endif
# define _POSIX_C_SOURCE GET_IPS_H_POSIX_C_SOURCE
#endif

#include <sys/socket.h>         /* AF_... freeaddrinfo getaddrinfo SOCK_... */
#include <netdb.h>              /* AI_... EAI_* getaddrinfo struct hostent hstrerror
                                   Linux: _BSD_SOURCE || _SVID_SOURCE || _GNU_SOURCE || _USE_POSIX (_POSIX_C_SOURCE >= 1 || POSIX_C_SOURCE || _XOPEN_SOURCE >= 600)
                                   NetBSD: _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 520 || _NETBSD_SOURCE,
                                   Solaris: _XPG4_2 (_XOPEN_SOURCE >= 500) || _XPG6 (_XOPEN_SOURCE >= 600) || __EXTENSIONS__
                                */

/* get_ips.c function declarations */
#if defined(__cplusplus)
# define GET_IPS_EXTERN extern "C"
#else
# define GET_IPS_EXTERN extern
#endif

GET_IPS_EXTERN int get_ips(char *, int, char *, int, int, void (*)(int, void *, const char *, ...), void *);

#define GET_IPS_H_INCLUDED
#endif
