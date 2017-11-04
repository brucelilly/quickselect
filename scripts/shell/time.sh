# Description: set environment variables for current (local) date-time value and zone offset from UTC:
# Description: set environment variables:
#  YEAR
#  MONTH
#  DAY
#  YESTERDAY
#  TOMORROW
#  HOUR
#  MINUTE
#  SECOND
#  ZONE

# The following license covers this software, including makefiles and documentation:
# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
################### Copyright notice (part of the license) ###################
#$Id: ~\|^` @(#)%M% copyright 2016 %Q%\ time.sh $
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
# You may send bug reports for ident scripts to %Y% with subject \"ident\".
#############################################################################
# maintenance note: master file  %P%

dir=""
thisfile=""

# make sure this is being run under a sane shell
#  test must be stand-alone and must occur before ${.sh.file}
# no recovery possible; broken shells have no way to get sourced file name
if test 0 -eq 1
then
foo=/foo/bar/baz////
bar=/foo/bar/baz
if test "${foo%%+(/)}" != "${bar}"
then
	echo broken shell detected: ${foo%%+(/)} != ${bar}
	# some broken shells cannot properly elide space characters
	baz="${bar// /}"
	# the Broken Awful shell (bash) doesn't handle ${.sh.file}
	# broken NetBSD shell barfs on ${.sh.file} even in a non-executed branch of an if statement
	# some implementations of sh refuse to accept return unless script is sourced
#	return 1 2>/dev/null || exit 1
fi
fi

# debugging control and other options
debug=0
debugflag=""
# check for debugging flag
i=1
while test ${i} -le $#
do
	eval arg=\$${i}
	case "${arg}" in
		-d)	debug=`expr ${debug} + 1`
			debugflag="${debugflag} ${arg}"
		;;
		-p)	i=`expr ${i} + 1`
			eval arg=\$${i}
			thisfile=${arg}
		;;
	esac
	i=`expr ${i} + 1`
done

if test -z "${thisfile}"
then
	case "$0" in
		ksh|ksh93|-ksh|-ksh93)
			case "${KSH_VERSION}" in
				*PD*KSH*)
					echo broken shell detected: "$0" "${KSH_VERSION}"
					echo ".sh.file unknown: specify path with -p or use a better shell"
				;;
				*)
					thisfile="${.sh.file}"
				;;
			esac
		;;
		*.sh)	
			thisfile=$0
		;;
		*)
			if test ${debug} -gt 0
			then
				echo '$0' is "$0"
			fi
		;;
	esac
fi

# realpath is part of GNU coreutils and expands relative path to full path
if test -n "${thisfile}"
then
	if test ${debug} -gt 0
	then
		echo thisfile ${thisfile}
	fi
	if realpath ${thisfile} >/dev/null 2>/dev/null 
	then
		foo=`realpath ${thisfile}`
		if test -n "${foo}"
		then
			thisfile=${foo}
			if test ${debug} -gt 0
			then
				echo thisfile ${thisfile}
			fi
			dir=`dirname ${thisfile}`
		fi
	else
		dir=`dirname ${thisfile}`
	fi
fi
if test -z "${dir}"
then
	echo "dir unknown: specify path with -p or use a better shell"
	return 1 2>/dev/null || exit 1
fi
if test -z "${thisfile}"
then
	thisfile=${dir}/time.sh
fi

# realpath is part of GNU coreutils and expands relative path to full path
if test -n "${thisfile}"
then
	if realpath ${thisfile} >/dev/null 2>/dev/null 
	then
		foo=`realpath ${thisfile}`
		if test -n "${foo}"
		then
			thisfile=${foo}
			if test ${debug} -gt 0
			then
				echo thisfile ${thisfile}
			fi
			dir=`dirname ${thisfile}`
		fi
	fi
fi

do=""
if test ${debug} -gt 0
then
	echo ${thisfile} debugging "(${debug})" ...
	do=echo
fi

# check for availability of required programs
prog=date
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi

export HOUR=`date +%H`
export MINUTE=`date +%M`
export SECOND=`date +%S`

if test ${debug} -gt 0
then
	echo HOUR \"${HOUR}\"
	echo MINUTE \"${MINUTE}\"
	echo SECOND \"${SECOND}\"
fi

if test -z "${HOUR}"
then
	echo cannot determine HOUR > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

if test -z "${MINUTE}"
then
	echo cannot determine MINUTE > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

if test -z "${SECOND}"
then
	echo cannot determine SECOND > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

# date.sh and zone.sh share some of the work
if test ${debug} -gt 0
then
	echo thisfile ${thisfile}
fi
dir=`dirname ${thisfile}`

if test ${debug} -gt 0
then
	echo thisfile ${thisfile}
	echo dir ${dir}
fi

# get local time zone offset from UTC
if test -f ${dir}/zone.sh
then
	. ${dir}/zone.sh ${debugflag} -p ${dir}/zone.sh
fi

# get date values
if test -f ${dir}/date.sh
then
	. ${dir}/date.sh ${debugflag} -p ${dir}/date.sh
fi

# don't exit (just fall off end) so that this file can be sourced
#exit 0
