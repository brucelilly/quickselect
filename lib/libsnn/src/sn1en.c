/* *INDENT-OFF* */
/*INDENT OFF*/
/* Description: sn1en - powers of ten
*/
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    sn1en.c copyright 2011 - 2017 Bruce Lilly.   \ sn1en.c $
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
/* $Id: ~|^` @(#)   This is sn1en.c version 2.6 2017-01-16T00:32:49Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "snn" */
/*****************************************************************************/
/* maintenance note: master file  /src/relaymail/lib/libsnn/src/s.sn1en.c */

/********************** Long description and rationale: ***********************
* sn1en returns a power of ten:
*
*  double sn1en(int n);
*
* N.B. the function name contains the numeral one, not the letter ell.
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
#define ID_STRING_PREFIX "$Id: sn1en.c ~|^` @(#)"
#define SOURCE_MODULE "sn1en.c"
#define MODULE_VERSION "2.6"
#define MODULE_DATE "2017-01-16T00:32:49Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2011 - 2017"

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

/*INDENT ON*/
/* *INDENT-ON* */

/* local header files */
#include "snn.h"      /* header file for public definitions and declarations */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

/* system header files needed for code which are not included with declaration header */
#include <ctype.h>              /* isalnum */
#include <stdlib.h>             /* NULL */
#include <string.h>             /* strrchr */
#include <syslog.h>             /* LOG_* */

/* static data and function definitions */
static char sn1en_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
/* powers of ten */
/* pcc is very picky about parentheses in the following macro */
#define SNN_POW10_OFFSET (0-(SNN_MIN_POW10))
/* 10^N = snn_pow10[N+SNN_POW10_OFFSET] */
static double snn_pow10[] = {
    1.0e-323, 1.0e-322, 1.0e-321, 1.0e-320, 1.0e-319, 1.0e-318, 1.0e-317,
    1.0e-316, 1.0e-315, 1.0e-314, 1.0e-313, 1.0e-312, 1.0e-311, 1.0e-310,
    1.0e-309, 1.0e-308, 1.0e-307, 1.0e-306, 1.0e-305, 1.0e-304, 1.0e-303,
    1.0e-302,
    1.0e-301, 1.0e-300, 1.0e-299, 1.0e-298, 1.0e-297, 1.0e-296, 1.0e-295,
    1.0e-294, 1.0e-293, 1.0e-292, 1.0e-291, 1.0e-290, 1.0e-289, 1.0e-288,
    1.0e-287, 1.0e-286, 1.0e-285, 1.0e-284, 1.0e-283, 1.0e-282, 1.0e-281,
    1.0e-280, 1.0e-279, 1.0e-278, 1.0e-277, 1.0e-276, 1.0e-275, 1.0e-274,
    1.0e-273, 1.0e-272, 1.0e-271, 1.0e-270, 1.0e-269, 1.0e-268, 1.0e-267,
    1.0e-266, 1.0e-265, 1.0e-264, 1.0e-263, 1.0e-262, 1.0e-261, 1.0e-260,
    1.0e-259, 1.0e-258, 1.0e-257, 1.0e-256, 1.0e-255, 1.0e-254, 1.0e-253,
    1.0e-252, 1.0e-251, 1.0e-250, 1.0e-249, 1.0e-248, 1.0e-247, 1.0e-246,
    1.0e-245, 1.0e-244, 1.0e-243, 1.0e-242, 1.0e-241, 1.0e-240, 1.0e-239,
    1.0e-238, 1.0e-237, 1.0e-236, 1.0e-235, 1.0e-234, 1.0e-233, 1.0e-232,
    1.0e-231, 1.0e-230, 1.0e-229, 1.0e-228, 1.0e-227, 1.0e-226, 1.0e-225,
    1.0e-224, 1.0e-223, 1.0e-222, 1.0e-221, 1.0e-220, 1.0e-219, 1.0e-218,
    1.0e-217, 1.0e-216, 1.0e-215, 1.0e-214, 1.0e-213, 1.0e-212, 1.0e-211,
    1.0e-210, 1.0e-209, 1.0e-208, 1.0e-207, 1.0e-206, 1.0e-205, 1.0e-204,
    1.0e-203, 1.0e-202, 1.0e-201, 1.0e-200, 1.0e-199, 1.0e-198, 1.0e-197,
    1.0e-196, 1.0e-195, 1.0e-194, 1.0e-193, 1.0e-192, 1.0e-191, 1.0e-190,
    1.0e-189, 1.0e-188, 1.0e-187, 1.0e-186, 1.0e-185, 1.0e-184, 1.0e-183,
    1.0e-182, 1.0e-181, 1.0e-180, 1.0e-179, 1.0e-178, 1.0e-177, 1.0e-176,
    1.0e-175, 1.0e-174, 1.0e-173, 1.0e-172, 1.0e-171, 1.0e-170, 1.0e-169,
    1.0e-168, 1.0e-167, 1.0e-166, 1.0e-165, 1.0e-164, 1.0e-163, 1.0e-162,
    1.0e-161, 1.0e-160, 1.0e-159, 1.0e-158, 1.0e-157, 1.0e-156, 1.0e-155,
    1.0e-154, 1.0e-153, 1.0e-152, 1.0e-151, 1.0e-150, 1.0e-149, 1.0e-148,
    1.0e-147, 1.0e-146, 1.0e-145, 1.0e-144, 1.0e-143, 1.0e-142, 1.0e-141,
    1.0e-140, 1.0e-139, 1.0e-138, 1.0e-137, 1.0e-136, 1.0e-135, 1.0e-134,
    1.0e-133, 1.0e-132, 1.0e-131, 1.0e-130, 1.0e-129, 1.0e-128, 1.0e-127,
    1.0e-126, 1.0e-125, 1.0e-124, 1.0e-123, 1.0e-122, 1.0e-121, 1.0e-120,
    1.0e-119, 1.0e-118, 1.0e-117, 1.0e-116, 1.0e-115, 1.0e-114, 1.0e-113,
    1.0e-112, 1.0e-111, 1.0e-110, 1.0e-109, 1.0e-108, 1.0e-107, 1.0e-106,
    1.0e-105, 1.0e-104, 1.0e-103, 1.0e-102, 1.0e-101, 1.0e-100, 1.0e-99,
    1.0e-98, 1.0e-97, 1.0e-96, 1.0e-95, 1.0e-94, 1.0e-93, 1.0e-92, 1.0e-91,
    1.0e-90, 1.0e-89, 1.0e-88, 1.0e-87, 1.0e-86, 1.0e-85, 1.0e-84, 1.0e-83,
    1.0e-82, 1.0e-81, 1.0e-80, 1.0e-79, 1.0e-78, 1.0e-77, 1.0e-76, 1.0e-75,
    1.0e-74, 1.0e-73, 1.0e-72, 1.0e-71, 1.0e-70, 1.0e-69, 1.0e-68, 1.0e-67,
    1.0e-66, 1.0e-65, 1.0e-64, 1.0e-63, 1.0e-62, 1.0e-61, 1.0e-60, 1.0e-59,
    1.0e-58, 1.0e-57, 1.0e-56, 1.0e-55, 1.0e-54, 1.0e-53, 1.0e-52, 1.0e-51,
    1.0e-50, 1.0e-49, 1.0e-48, 1.0e-47, 1.0e-46, 1.0e-45, 1.0e-44, 1.0e-43,
    1.0e-42, 1.0e-41, 1.0e-40, 1.0e-39, 1.0e-38, 1.0e-37, 1.0e-36, 1.0e-35,
    1.0e-34, 1.0e-33, 1.0e-32, 1.0e-31, 1.0e-30, 1.0e-29, 1.0e-28, 1.0e-27,
    1.0e-26, 1.0e-25, 1.0e-24, 1.0e-23, 1.0e-22, 1.0e-21, 1.0e-20, 1.0e-19,
    1.0e-18, 1.0e-17, 1.0e-16, 1.0e-15, 1.0e-14, 1.0e-13, 1.0e-12, 1.0e-11,
    1.0e-10, 1.0e-9, 1.0e-8, 1.0e-7, 1.0e-6, 1.0e-5, 1.0e-4, 1.0e-3, 1.0e-2,
    1.0e-1, 1.0, 1.0e1, 1.0e2, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9,
    1.0e10, 1.0e11, 1.0e12, 1.0e13, 1.0e14, 1.0e15, 1.0e16, 1.0e17, 1.0e18,
    1.0e19, 1.0e20, 1.0e21, 1.0e22, 1.0e23, 1.0e24, 1.0e25, 1.0e26, 1.0e27,
    1.0e28, 1.0e29, 1.0e30, 1.0e31, 1.0e32, 1.0e33, 1.0e34, 1.0e35, 1.0e36,
    1.0e37, 1.0e38, 1.0e39, 1.0e40, 1.0e41, 1.0e42, 1.0e43, 1.0e44, 1.0e45,
    1.0e46, 1.0e47, 1.0e48, 1.0e49, 1.0e50, 1.0e51, 1.0e52, 1.0e53, 1.0e54,
    1.0e55, 1.0e56, 1.0e57, 1.0e58, 1.0e59, 1.0e60, 1.0e61, 1.0e62, 1.0e63,
    1.0e64, 1.0e65, 1.0e66, 1.0e67, 1.0e68, 1.0e69, 1.0e70, 1.0e71, 1.0e72,
    1.0e73, 1.0e74, 1.0e75, 1.0e76, 1.0e77, 1.0e78, 1.0e79, 1.0e80, 1.0e81,
    1.0e82, 1.0e83, 1.0e84, 1.0e85, 1.0e86, 1.0e87, 1.0e88, 1.0e89, 1.0e90,
    1.0e91, 1.0e92, 1.0e93, 1.0e94, 1.0e95, 1.0e96, 1.0e97, 1.0e98, 1.0e99,
    1.0e100, 1.0e101, 1.0e102, 1.0e103, 1.0e104, 1.0e105, 1.0e106, 1.0e107,
    1.0e108, 1.0e109, 1.0e110, 1.0e111, 1.0e112, 1.0e113, 1.0e114, 1.0e115,
    1.0e116, 1.0e117, 1.0e118, 1.0e119, 1.0e120, 1.0e121, 1.0e122, 1.0e123,
    1.0e124, 1.0e125, 1.0e126, 1.0e127, 1.0e128, 1.0e129, 1.0e130, 1.0e131,
    1.0e132, 1.0e133, 1.0e134, 1.0e135, 1.0e136, 1.0e137, 1.0e138, 1.0e139,
    1.0e140, 1.0e141, 1.0e142, 1.0e143, 1.0e144, 1.0e145, 1.0e146, 1.0e147,
    1.0e148, 1.0e149, 1.0e150, 1.0e151, 1.0e152, 1.0e153, 1.0e154, 1.0e155,
    1.0e156, 1.0e157, 1.0e158, 1.0e159, 1.0e160, 1.0e161, 1.0e162, 1.0e163,
    1.0e164, 1.0e165, 1.0e166, 1.0e167, 1.0e168, 1.0e169, 1.0e170, 1.0e171,
    1.0e172, 1.0e173, 1.0e174, 1.0e175, 1.0e176, 1.0e177, 1.0e178, 1.0e179,
    1.0e180, 1.0e181, 1.0e182, 1.0e183, 1.0e184, 1.0e185, 1.0e186, 1.0e187,
    1.0e188, 1.0e189, 1.0e190, 1.0e191, 1.0e192, 1.0e193, 1.0e194, 1.0e195,
    1.0e196, 1.0e197, 1.0e198, 1.0e199, 1.0e200, 1.0e201, 1.0e202, 1.0e203,
    1.0e204, 1.0e205, 1.0e206, 1.0e207, 1.0e208, 1.0e209, 1.0e210, 1.0e211,
    1.0e212, 1.0e213, 1.0e214, 1.0e215, 1.0e216, 1.0e217, 1.0e218, 1.0e219,
    1.0e220, 1.0e221, 1.0e222, 1.0e223, 1.0e224, 1.0e225, 1.0e226, 1.0e227,
    1.0e228, 1.0e229, 1.0e230, 1.0e231, 1.0e232, 1.0e233, 1.0e234, 1.0e235,
    1.0e236, 1.0e237, 1.0e238, 1.0e239, 1.0e240, 1.0e241, 1.0e242, 1.0e243,
    1.0e244, 1.0e245, 1.0e246, 1.0e247, 1.0e248, 1.0e249, 1.0e250, 1.0e251,
    1.0e252, 1.0e253, 1.0e254, 1.0e255, 1.0e256, 1.0e257, 1.0e258, 1.0e259,
    1.0e260, 1.0e261, 1.0e262, 1.0e263, 1.0e264, 1.0e265, 1.0e266, 1.0e267,
    1.0e268, 1.0e269, 1.0e270, 1.0e271, 1.0e272, 1.0e273, 1.0e274, 1.0e275,
    1.0e276, 1.0e277, 1.0e278, 1.0e279, 1.0e280, 1.0e281, 1.0e282, 1.0e283,
    1.0e284, 1.0e285, 1.0e286, 1.0e287, 1.0e288, 1.0e289, 1.0e290, 1.0e291,
    1.0e292, 1.0e293, 1.0e294, 1.0e295, 1.0e296, 1.0e297, 1.0e298, 1.0e299,
    1.0e300, 1.0e301, 1.0e302, 1.0e303, 1.0e304, 1.0e305, 1.0e306, 1.0e307,
    1.0e308, 
};

static void initialize_sn1en(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    sn1en_initialized = register_build_strings(NULL, &source_file, s);
}

/* return a power of ten
   repeated multiplication or division by 10 accumulates errors
   exp10 a.k.a. pow10 is a gnu extension requiring a double argument, math.h, and -lm
   this function does not require math.h or -lm or proprietary "extensions"
*/
/* calls: sn1en_initialize */
/* called by: snn_sf_double, initialize_snn, int_d, d_int_d, snmagnitude */
double sn1en(int n, void (*f)(int, void *, const char *, ...), void *log_arg)
{
#ifndef PP__FUNCTION__
    static const char __func__[] = "sn1en";
#endif

    if ((unsigned char)0U == sn1en_initialized)
        initialize_sn1en();
    /* limits on the range of a double */
    if (SNN_MAX_POW10 < n) {
        if (NULL != f) {
            f(LOG_WARNING, log_arg,
                "%s: %s line %d: supplied exponent %d being reduced to maximum value %d",
                __func__, source_file, __LINE__,
                n, SNN_MAX_POW10);
        }
        n = SNN_MAX_POW10;
    } else if (SNN_MIN_POW10 > n) {
        if (NULL != f) {
            f(LOG_WARNING, log_arg,
                "%s: %s line %d: supplied exponent %d being increased to minimum value %d",
                __func__, source_file, __LINE__,
                n, SNN_MIN_POW10);
        }
        n = SNN_MIN_POW10;
    }
    return snn_pow10[n+SNN_POW10_OFFSET];
}
