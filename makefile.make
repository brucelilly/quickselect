# Description: makefile that sets macros for SHELL and MAKE

# The following license covers this software, including makefiles and documentation:
# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
################### Copyright notice (part of the license) ###################
#$Id: ~\|^` @(#)%M% copyright 2015 - 2016 %Q%. \ makefile.make $
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the
# use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it freely,
# subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not claim
#    that you wrote the original software. If you use this software in a
#    product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source distribution.
############################## (end of license) ##############################
# This is %M% version %I% dated %O%.
# You may send bug reports for makefiles to %Y% with subject \"makefiles\".
#############################################################################
# maintenance note: master file  %P%

# Add makefile name to .PRECIOUS pseudo-target and to MAKEFILES_READ macro
.PRECIOUS : $(MAKEFILES_DIR)/makefile.make
MAKEFILES_READ += makefile.make

#   cannot use ${SHELL} below, because gmake under NetBSD resets SHELL to broken NetBSD /bin/sh
#   $(SHELL) (if defined here) can be used
#   it must be a reasonable shell (ksh works)
#     cannot use generic "sh" because NetBSD sh is broken
#   therefore (unfortunately), ksh is a hard prerequisite
# a full path is acceptable
# pick one, retain the others as comments
#SHELL = sh
SHELL = ksh
#SHELL = ksh93
#SHELL = /usr/pkg/bin/ksh93
# Do not change the following line, which is used for debugging via `make macros`.
MACROS += SHELL

# pick one of the following specifiations for running make with environment
#   variables, and preserving flags; the others are retained as comments:
# standard make (might need a full path on some systems to prevent getting
#   some other make variant by accident, or to modify make's behavior)
#MAKE = make -e$(MAKEFLAGS)
#MAKE = /usr/xpg4/bin/make -e$(MAKEFLAGS)
#MAKE = /usr/lib/svr4.make -e$(MAKEFLAGS)
# BSD-ish make
#MAKE = bmake -e -r -C $(MAKEFILES_DIR)
#MAKE = /usr/bin/make -e -r -C $(MAKEFILES_DIR)
# gmake works (except for gmake bug #47399) with standard -e option
#   and multi-stage build works around gmake bug $47933
# gmake requires an explicit list of flags as $(MAKEFLAGS)
MAKE = gmake -e$(MAKEFLAGS)
# dmake -e operation differs from standard make -e, and causes problems evaluating $(OS)
#   dmake -E works like standard -e, but is non-standard
# dmake automatically appends flags to MAKE
#MAKE = dmake -E
# Do not change the following line, which is used for debugging via `make macros`.
MACROS += MAKE

# initialization of macros which may be appended to by makefile.files and/or $(PRE_DEPEND)
ALL_HEADERS =
ALL_SOURCES =
ARCHES32 =
ARCHES64 =
EXPENDABLES =
INCLUDE =
LOCAL_PROGRAMS =
MAN_TEXT =
OBJECT_FILES =
TOP_LEVEL_PROGS =
# initialization of macros which may be appended to by makefile.$(OS)-$(FQDN) and/or makefile.$(CCBASE)
XTRAS =
LDFLAGS =
# Do not change the following line, which is used for debugging via `make macros`.
MACROS += ALL_HEADERS ALL_SOURCES ARCHES32 ARCHES64 EXPENDABLES INCLUDE LOCAL_PROGRAMS MAN_TEXT OBJECT_FILES TOP_LEVEL_PROGS XTRAS LDFLAGS

