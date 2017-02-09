# Description: awk script to process dependencies

# The following license covers this software, including makefiles and documentation:

# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
# ****************** Copyright notice (part of the license) *******************
# $Id: ~\|^` @(#)%M% copyright 2015 - 2016 %Q%\ depend.awk $
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
# ****************************** (end of license) ******************************
# This is %M% version %I% dated %O%
# You may send bug reports to %Y% with subject "depend.awk".
# *****************************************************************************
# maintenance note: master file  %P%

BEGIN {
	if (length(BASE) > 1) {
		pfx1 = BASE "/"
		pfx2 = BASE "/"
	} else {
		pfx1 = "./"
		pfx2 = "./"
	}
	if (length(SUBDIR32BIT) > 1) {
		pfx1 = pfx1 SUBDIR32BIT "/"
	}
	if (length(SUBDIR64BIT) > 1) {
		pfx2 = pfx2 SUBDIR64BIT "/"
	}
	gsub(/[\057][\057]+/, "/", pfx1)
	gsub(/[\057][\057]+/, "/", pfx2)
	MAKEFILES=""
	temp = MAKEFILES_READ ""
	mfqdn = "makefile." "" FQDN
	n = split(MAKEFILES_READ, ary, /[ \t]+/)
	if (length(debug) > 0)
		printf "# FQDN %s -> %s\n", FQDN, mfqdn
	for (i=1 ; i<=n ; i++) {
		if (length(debug) > 0)
			printf "# MAKEFILE %s\n", ary[i]
		if (ary[i] == mfqdn)
			continue
		if (index(MAKEFILES, ary[i]) > 0)
			continue
		MAKEFILES = MAKEFILES " " ary[i]
	}
	if (length(debug) > 0)
		printf "# MAKEFILES %s\n", MAKEFILES
}

/^[A-Za-z][A-Za-z0-9_]*[.]o[ \t]*[:].*[\\]$/ {
	temp = $0
	if (pfx1 == pfx2) {
		printf "%s%s\n", pfx2, temp
	} else {
		obj = $1
		gsub(/[\057][\057]+/, "/", obj)
		sub(/[:]/, "", obj)
		printf "%s%s %s%s\n", pfx1, obj, pfx2, temp
	}
	next
}

/^[A-Za-z][A-Za-z0-9_]*[.]o[ \t]*[:]/ {
	temp = $0
	if (pfx1 == pfx2) {
		printf "%s%s%s\n", pfx2, temp, MAKEFILES
	} else {
		obj = $1
		gsub(/[\057][\057]+/, "/", obj)
		sub(/[:]/, "", obj)
		printf "%s%s %s%s%s\n", pfx1, obj, pfx2, temp, MAKEFILES
	}
	next
}

/^[^:]*[\\]$/ {
	printf "%s\n", $0
	next
}

{
	printf "%s%s\n", $0, MAKEFILES
}
