# Description: wrapper makefile that determines {host,OS,tools}-specific makefiles to use

# The following license covers this software, including makefiles and documentation:
# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
################### Copyright notice (part of the license) ###################
#$Id: ~\|^` @(#)%M% copyright 2015 - 2016 %Q%. \ makefile $
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

# Add makefile name to MAKEFILES_READ macro
MAKEFILES_READ += makefile

# fake targets are listed so that it is possible to check for and remove
#   files and/or directories masquerading as fake targets (in case an
#   earlier make run generated one)
FAKE_TARGETS += .PRECIOUS .SUFFIXES

default_target : all
# update FAKE_TARGETS list before including makefile.include1
FAKE_TARGETS += default_target all analyses start host ccbase depend test
FAKE_TARGETS += indent install macros overrides clean cleaner cleanest

# N.B. MAKEFILES_DIR is not yet defined...
include makefile.include1
# Add makefile name (full path) to .PRECIOUS pseudo-target
.PRECIOUS : $(MAKEFILES_DIR)/makefile

# have now included through makefile.make,
#   included by makefile.include1
# makefile.include1 also provides recipes for
#   fake targets NO_FAKE_TARGETS and ALWAYS

# add environment variables via $(SCRIPTSDIR)/ident.sh to identify host, OS, etc.
# override environment $SHELL with something sane (defined above)
# broken dmake overrides $(OS) in an undocumented manner
#	dmake by default ignores environment variables (requires
#	   non-standard -E command-line flag option) and sets $(OS) to
#	   something like "unix"
#		this is one reason why there are makefile.stage?,
#		  makefile.include? files
#		work-around is setting environment via makefile and
#		  scripts/shell/ident.sh, makefile.include1,
#		  then re-run $(MAKE) [defined as 'dmake -E') with
#		  makefile.stage2 and makefile.include2 as well as
#		  makefile.include1
# gmake fails if some included file doesn't exist (bootstrapping fails)
#	  (reported gmake bug #47399)
#	work-around is multiple runs to build files to be included
#		this is why there are makefile.stage?, makefile.include? files
#		and why there are targets "host", "ccbase", and "depend" below

# this file provides recipes to bootstrap required included files
#   which will be included in successive stages (see gmake bug #47399 above)
# this stage starts with ensuring that there are no files or directories
#   with fake target names by running the recipe for NO_FAKE_TARGETS
# stage2: adds files included by makefile.include2
#     makefile.$(OS), makefile.project, makefile.fqdn
#   and builds makefile.$(OS)-$(FQDN)
#   using makefile.stage2 and environment variable
#    OS, FQDN, etc. from $(SCRIPTSDIR)/ident.sh
# stage3: adds file included by makefile.include3
#     makefile.$(OS)-$(FQDN)
#   and builds makefile.ccbase-$(OS)-$(FQDN)
#   using makefile.stage3
# stage4: adds files included by makefile.include4
#     makefile.ccbase-$(OS)-$(FQDN)
#     makefile.platform, makefile.$(CCBASE), makefile.suffixes,
#     makefile.suffix_rules, makefile.files, makefile.imports
#   and builds $(OVERRIDES)
#   using makefile.stage4
# stage5: adds files included by makefile.include5
#     makefile.overrides.$(CCBASE)-$(OS)-$(FQDN), makefile.depend
#   and builds $(PRE_DEPEND)
#   using makefile.stage5
# stage6: adds files included by makefile.include6
#     $(PRE_DEPEND), makefile.licenses
#   and emits licenses and builds $(DEPEND)
#   using makefile.stage6
# stage7: adds files included by makefile.include7
#     $(DEPEND), makefile.build
#   to complete the bootstrap process
#   and builds the desired target(s) (default is "all")
#   using makefile.stage7
# macros for last stage makefiles used below and in makefile.targets
LAST_STAGE = $(MAKEFILES_DIR)/makefile.stage7
LAST_INCLUDE = $(MAKEFILES_DIR)/makefile.include7
# Do not change the following line, which is used for debugging via `make macros`.
MACROS += LAST_STAGE LAST_INCLUDE

# dependencies only
start : NO_FAKE_TARGETS \
 $(SCRIPTSDIR)/date.sh $(SCRIPTSDIR)/host.sh $(SCRIPTSDIR)/ident.sh \
 $(SCRIPTSDIR)/network.sh $(SCRIPTSDIR)/system.sh $(SCRIPTSDIR)/time.sh \
 $(SCRIPTSDIR)/zone.sh \
 $(MAKEFILES_DIR)/makefile $(MAKEFILES_DIR)/makefile.include1 \
 $(MAKEFILES_DIR)/makefile.directories $(MAKEFILES_DIR)/makefile.make

# combined dependencies and recipe
# NetBSD /usr/bin/make gets lost easily; it needs the full path to the makefiles
host : start \
 $(MAKEFILES_DIR)/makefile.stage2 $(MAKEFILES_DIR)/makefile.include2 \
 $(MAKEFILES_DIR)/makefile.project $(MAKEFILES_DIR)/makefile.fqdn # also makefile.$(OS), but environment is not yet set
	$(SHELL) -c ". $(SCRIPTSDIR)/ident.sh -p $(SCRIPTSDIR)/ident.sh ; SHELL=$(SHELL) MAKE='$(MAKE)' $(MAKE) -f $(MAKEFILES_DIR)/makefile.stage2"

# combined dependencies and recipe
# NetBSD /usr/bin/make gets lost easily; it needs the full path to the makefiles
ccbase : host \
 $(MAKEFILES_DIR)/makefile.stage3 $(MAKEFILES_DIR)/makefile.include3 \
 $(MAKEFILES_DIR)/makefile.tools # also makefile.$(OS)-$(FQDN), but environment is not yet set
	$(SHELL) -c ". $(SCRIPTSDIR)/ident.sh -p $(SCRIPTSDIR)/ident.sh ; SHELL=$(SHELL) MAKE='$(MAKE)' $(MAKE) -f $(MAKEFILES_DIR)/makefile.stage3"

# combined dependencies and recipe
overrides : ccbase \
 $(MAKEFILES_DIR)/makefile.stage4 $(MAKEFILES_DIR)/makefile.include4 \
 $(MAKEFILES_DIR)/makefile.tools $(MAKEFILES_DIR)/makefile.overrides \
 # also makefile.$(OS)-$(FQDN), but environment is not yet set
	$(SHELL) -c ". $(SCRIPTSDIR)/ident.sh -p $(SCRIPTSDIR)/ident.sh ; SHELL=$(SHELL) MAKE='$(MAKE)' $(MAKE) -f $(MAKEFILES_DIR)/makefile.stage4"

# combined dependencies and recipe
# dependencies also include makefile.ccbase-$(OS)-$(FQDN) [stage4], \
# makefile.overrides.$(CCBASE)-$(OS)-$(FQDN) [stage4], \
# $(PRE_DEPEND) [stage5]
# but dmake botches parse of makefile if these are included as a comment at
# the end of the dependency list!
# NetBSD /usr/bin/make gets lost easily; it needs the full path to the makefiles
depend : overrides \
 $(AWKSCRIPTSDIR)/decomment.awk $(AWKSCRIPTSDIR)/depend.awk \
 $(MAKEFILES_DIR)/makefile.stage5 $(MAKEFILES_DIR)/makefile.include5 \
 $(MAKEFILES_DIR)/makefile.stage6 $(MAKEFILES_DIR)/makefile.include6 \
 $(MAKEFILES_DIR)/makefile.platform $(MAKEFILES_DIR)/makefile.suffixes \
 $(MAKEFILES_DIR)/makefile.suffix_rules $(MAKEFILES_DIR)/makefile.files \
 $(MAKEFILES_DIR)/makefile.licenses $(MAKEFILES_DIR)/makefile.imports \
 $(MAKEFILES_DIR)/makefile.depend
	$(SHELL) -c ". $(SCRIPTSDIR)/ident.sh -p $(SCRIPTSDIR)/ident.sh ; SHELL=$(SHELL) MAKE='$(MAKE)' $(MAKE) -f $(MAKEFILES_DIR)/makefile.stage5"
	$(SHELL) -c ". $(SCRIPTSDIR)/ident.sh -p $(SCRIPTSDIR)/ident.sh ; SHELL=$(SHELL) MAKE='$(MAKE)' $(MAKE) -f $(MAKEFILES_DIR)/makefile.stage6"

# dependencies for primary targets
# NetBSD /usr/bin/make gets lost easily; it needs the full path to the makefiles
all test install macros clean cleaner cleanest : depend \
 $(LAST_STAGE) $(LAST_INCLUDE) $(MAKEFILES_DIR)/makefile.build # also $(DEPEND) [stage6]

# recipe for primary targets
all analyses cflow.out cflow_i.out clean cleaner cleanest cxref.out flawfinder.out indent install lint.out macros rats.out sparse.out splint.out test uno.out :
	$(SHELL) -c ". $(SCRIPTSDIR)/ident.sh -p $(SCRIPTSDIR)/ident.sh ; SHELL=$(SHELL) MAKE='$(MAKE)' $(MAKE) -f $(LAST_STAGE) $@"

# Because of gmake bug #47399, make runs in stages to build files which
# must be included.  As a result, targets in the main project makefile.files
# file are not visible to make when first run (first stage through stage3).
# Therefore targets which should be visible to make initially (to support
# `make foo`, where "foo" is one such target) are put in makefile.targets.
# NetBSD /usr/bin/make gets lost easily; it needs the full path to the makefiles
include $(MAKEFILES_DIR)/makefile.targets

