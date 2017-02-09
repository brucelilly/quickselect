# Description: awk script to remove various types of comments

# The following license covers this software, including makefiles and documentation:

# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
# ****************** Copyright notice (part of the license) *******************
# $Id: ~\|^` @(#)%M% copyright 2015 %Q%\ decomment.awk $
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
# You may send bug reports to %Y% with subject "decomment.awk".
# *****************************************************************************
# maintenance note: master file  %P%

BEGIN {
}

{
	# all lines start by being saved as a string
	temp = $0
	# C comments between /* and */
	# N.B. this is a quick hack; correct processing would require a C language parser
	if (temp ~ /[\057][*].*[*][\057]/) {
		sub(/[\057][*].*[*][\057]/, "", temp)
		if (length(debug) > 0)
			printf "C comment removed: %s\n", temp
	}
	# See ploose ploose comments from // to end-of-line
	# N.B. this is a quick hack; correct processing would require a C++ language parser
	if (temp ~ /[\057][\057]/) {
		sub(/[\057][\057].*/, "", temp)
		if (length(debug) > 0)
			printf "C++ comment removed: %s\n", temp
	}
	# RCS droppings (including leading quadrathorpe)
	if (temp ~ /[$][ \t]*[A-Za-z][A-za-z]*:[ \t]*.*[ \t]*[$]/) {
		sub(/[#]*[ \t]*[$][	\t]*[A-za-z][A-za-z]*:[ \t]*/, "", temp)
		sub(/[ \t]*[$]/, "", temp)
		if (length(debug) > 0)
			printf "RCS droppings removed: %s\n", temp
	}
	# SCCS markers (including leading quadrathorpe)
	#  if there are no unexpanded keyword markers, remove stuff after what string terminator
	if (temp ~ /[~][\\]*[|][`^][`^]/) {
		sub(/[#]*[ \t]*[~][\\]*[|][`^][`^][ \t]*/, "", temp)
		if (temp !~ /%[A-Za-z][A-Za-z]*%/) {
			sub(/[ \t]*[\\].*/, "", temp)
		}
		if (length(debug) > 0)
			printf "old SCCS marker removed: %s\n", temp
	}
	if (temp ~ /@[(][#][)]/) {
		sub(/[#]*[ \t]*@[(][#][)][ \t]*/, "", temp)
		if (temp !~ /%[A-Za-z][A-Za-z]*%/) {
			sub(/[ \t]*[\\].*/, "", temp)
		}
		if (length(debug) > 0)
			printf "SCCS marker removed: %s\n", temp
	}
	# remove extraneous leading/trailing space
	if (temp ~ /^[ \t]/) {
		sub(/^[ \t][ \t]*/, "", temp)
		if (length(debug) > 0)
			printf "leading whitespace removed: %s\n", temp
	}
	if (temp ~ /[ 	]$/) {
		sub(/[ \t]*[ \t]$/, "", temp)
		if (length(debug) > 0)
			printf "trailing whitespace removed: %s\n", temp
	}
	# emit what remains
	printf "%s\n", temp
	next
}
