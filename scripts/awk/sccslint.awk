# Description: awk script to report (echo) lines matching sccs keyword identifiers

# The following license covers this software, including makefiles and documentation:

# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "free" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
# ****************** Copyright notice (part of the license) *******************
# $Id: ~\|^` @(#)%M% %I% %O% copyright 2017 %Q%\ sccslint.awk $
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
# You may send bug reports to %Y% with subject "sccslint.awk".
# *****************************************************************************
# maintenance note: master file  %P%

# problems can arise when e.g. a printf string contains %d%s or %e%s or %E%s
#   or %G%s ...

BEGIN {
	"date +%Y" | getline year
	"date -u +%Y" | getline utcyear
	year2d = year % 100
	"date +%m" | getline month
	"date -u +%m" | getline utcmonth
	"date +%d" | getline day
	"date -u +%d" | getline utcday
	"date +%H" | getline hour
	"date -u +%H" | getline utchour
	"date +%M" | getline minute
	"date -u +%M" | getline utcminute
	"date +%S" | getline second
	"date -u +%S" | getline utcsecond
	utcdatetime = utcyear "-" utcmonth "-" utcday "T" utchour ":" utcminute ":" utcsecond "Z"
#	"date -u '+%FT%TZ'" | getline utcdatetime
}

/[%][A-IL-UYZd-egh][%]/ {
	temp = $0
	printf "line %d: %s\n", NR, temp
	gsub(/%A%/, "%Z%%Y%	%M%	%I%%Z%", temp)
	gsub(/%W%/, "%Z%%M%	%I%", temp)
	gsub(/%I%/, "%R%.%L%.%B%.%S%", temp)
	gsub(/%B%/, "<BRANCH>", temp)
	gsub(/%C%/, NR "", temp)
	gsub(/%D%/, year2d "/" month "/" day "", temp)
	gsub(/%d%/, year "/" month "/" day "", temp)
	gsub(/%E%/, year2d "/" month "/" day "", temp) # displays current rather than delta
	gsub(/%e%/, year "/" month "/" day "", temp) # displays current rather than delta
	gsub(/%F%/, "<FILE>", temp)
	gsub(/%G%/, year2d "/" month "/" day "", temp) # displays current rather than delta
	gsub(/%g%/, year "/" month "/" day "", temp) # displays current rather than delta
	gsub(/%H%/, month "/" day "/" year2d "", temp)
	gsub(/%h%/, month "/" day "/" year "", temp)
	gsub(/%L%/, "<LEVEL>", temp)
	gsub(/%M%/, "<MODULE>", temp)
	gsub(/%N%/, utcdatetime, temp)
	gsub(/%O%/, utcdatetime, temp) # displays current rather than delta
	gsub(/%P%/, "<PATH>", temp)
	gsub(/%Q%/, "<Q FLAG>", temp)
	gsub(/%R%/, "<RELEASE>", temp)
	gsub(/%S%/, "<SEQUENCE>", temp)
	gsub(/%T%/, hour ":" minute ":" second "", temp)
	gsub(/%U%/, hour ":" minute ":" second "", temp) # displays current rather than delta
	gsub(/%Y%/, "<TYPE>", temp)
	gsub(/%Z%/, "@(#)", temp)
	printf "-> %s\n", temp
	next
}
