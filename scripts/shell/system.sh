# Description: set environment variables:
#  OS
#  DISTRIBUTION
#  OSVERSION
#  WORDSIZE
#  ARCHITECTURE
#  EXECSUBDIR32BIT
#  EXECSUBDIR64BIT
#  LIBDIR32BIT
#  LIBDIR64BIT

# The following license covers this software, including makefiles and documentation:
# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
################### Copyright notice (part of the license) ###################
#$Id: ~\|^` @(#)%M% copyright 2015 - 2016 %Q%\ system.sh $
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

thisfile=""
dir=""

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
		*.sh)	thisfile=$0
		;;
		*)
			if test ${debug} -gt 0
			then
				echo '$0' is $0
			fi
		;;
	esac
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
	thisfile=${dir}/system.sh
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
	do="echo"
fi

# check for availability of required programs
prog=uname
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=tr
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=cat
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi

if test ${debug} -gt 0
then
	echo "uname -i (hardware platform)" `uname -i 2>/dev/null` # not in Minix, NetBSD
	echo "uname -m (machine hardware name)" `uname -m 2>/dev/null`
#	echo "uname -n (network node hostname)" `uname -n 2>/dev/null`
	echo "uname -o (operating system)" `uname -o 2>/dev/null` # not in Minix, base Solaris, NetBSD
	echo "uname -p (processor type)" `uname -p 2>/dev/null`
	echo "uname -r (kernel release)" `uname -r 2>/dev/null`
	echo "uname -s (kernel name)" `uname -s 2>/dev/null`
	echo "uname -v (kernel version)" `uname -v 2>/dev/null`
fi

os=`uname -o 2>/dev/null | tr "[A-Z]" "[a-z]" | tr -d " "`
os=${os#gnu/}
if test -z "${os}"
then
	os=`uname -s 2>/dev/null | tr "[A-Z]" "[a-z]" | tr -d " "`
fi
case ${os} in
	linux)
		export OS=${os}
		if test -f /etc/os-release
		then
			. /etc/os-release
			export DISTRIBUTION=`echo ${ID} | tr -d " "`
			case ${DISTRIBUTION} in
				*suse*)
					processor=`uname -p | tr "[A-Z]" "[a-z]"`
					export ARCHITECTURE=${processor}
					export OSVERSION=`echo ${VERSION_ID} | tr -d " "`
				;;
				gentoo)
					# uname -m (machine hardware name) x86_64
					# uname -p (processor type) Intel(R) Pentium(R) D CPU 2.80GHz
					machine=`uname -m | tr "[A-Z]" "[a-z]"`
					export ARCHITECTURE=${machine}
					export OSVERSION=`cat /etc/gentoo-release`
					if test -z "${OSVERSION}"
					then
						OSVERSION=unknown
					fi
				;;
				*)
					echo OSVERSION not determined for ${OS} ${DISTRIBUTION}
				;;
			esac
		else
			if test -f /etc/debian_version
			then
				export DISTRIBUTION=debian
				export OSVERSION=`cat /etc/debian_version`
				machine=`uname -m | tr "[A-Z]" "[a-z]"`
				export ARCHITECTURE=${machine}
			else
				echo neither /etc/os-release nor /etc/debian_version could be found
			fi
		fi
		export EXECSUBDIR32BIT=i86
		export EXECSUBDIR64BIT=amd64
		export LIBDIR32BIT=/lib
		export LIBDIR64BIT=/lib64
	;;
	sunos|solaris)
		export OS=solaris
		export OSVERSION=`uname -v | tr "[A-Z]" "[a-z]"`
		processor=`uname -p | tr "[A-Z]" "[a-z]"`
		prog=grep
		path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
		if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
		if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
		prog=pr
		path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
		if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
		if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
		for distribution in openindiana opensxce dilos tribblix
		do
			if grep -i ${distribution} /etc/release >/dev/null 2>&1
			then
				export DISTRIBUTION=`echo ${distribution} | tr -d " "`
			fi
			if test -z "${DISTRIBUTION}"
			then
				echo no recognized ${OS} distribution mentioned in /etc/release:
				pr -n -l 1 /etc/release
			fi
		done
		prog=isainfo
		path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
		if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
		if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
		prog=wc
		path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
		if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
		if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
		prog=tail
		path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
		if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
		if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
                isainfo=`isainfo`
                architecture=`echo ${isainfo} | sed -e 's/ /\n/' | head -1`
                wc=`echo ${isainfo} | wc -w`
		export ARCHITECTURE=${architecture}
		export LIBDIR32BIT=/lib/32
		export LIBDIR64BIT=/lib/64
		case ${wc} in
			1) # 32-bit system
				case ${architecture} in
					sparc*)
						export EXECSUBDIR32BIT=${architecture}
						# guesswork
						export EXECSUBDIR64BIT=sparcv9
					;;
					*86*)
						export EXECSUBDIR32BIT=i86
						export EXECSUBDIR64BIT=amd64
					;;
					*)
						echo "unknown ARCHITECTURE ${ARCHITECTURE}"
					;;
				esac
			;;
			2) # 64-bit system
				case ${architecture} in
					sparc*)
						export EXECSUBDIR32BIT=`echo ${isainfo} | sed -e 's/ /\n/' | tail -1`
						# guesswork
						export EXECSUBDIR64BIT=${ARCHITECTURE}
					;;
					*64*)
						export EXECSUBDIR32BIT=i86
						export EXECSUBDIR64BIT=${ARCHITECTURE}
					;;
					*)
						echo "unknown ARCHITECTURE ${ARCHITECTURE}"
					;;
				esac
			;;
			*)
				echo "too many results (${wc}) from isainfo (${isainfo})"
			;;
		esac
	;;
	*bsd)
		export DISTRIBUTION=`head -1 /etc/release | tr -d " "`
		export OS=${os}
		export OSVERSION=`uname -r | tr "[A-Z]" "[a-z]"`
		processor=`uname -p | tr "[A-Z]" "[a-z]"`
		export ARCHITECTURE=${processor}
		export LIBDIR64BIT=/lib
		case ${ARCHITECTURE} in
			*86*|*64)
				export EXECSUBDIR32BIT=i86
				export EXECSUBDIR64BIT=amd64
				export LIBDIR32BIT=/lib/i386
			;;
			*)
				echo "unknown ARCHITECTURE ${ARCHITECTURE}"
			;;
		esac
	;;
	*)
		echo os \"${os}\" not recognized
		# some implementations of sh refuse to accept return unless script is sourced
		return 1 2>/dev/null || exit 1
	;;
esac

case ${OS} in
	solaris)
		export WORDSIZE=`isainfo -b`
	;;
	*)
		case ${ARCHITECTURE} in
			*64)
				export WORDSIZE=64
			;;
			*86)
				export WORDSIZE=32
			;;
			*)
				echo WORDSIZE cannot be determined from ARCHITECTURE ${ARCHITECTURE}
			;;
		esac
	;;
esac

if test ${debug} -gt 0
then
	echo OS \"${OS}\"
	echo DISTRIBUTION \"${DISTRIBUTION}\"
	echo WORDSIZE \"${WORDSIZE}\"
	echo OSVERSION \"${OSVERSION}\"
	echo ARCHITECTURE \"${ARCHITECTURE}\"
	echo EXECSUBDIR32BIT \"${EXECSUBDIR32BIT}\"
	echo EXECSUBDIR64BIT \"${EXECSUBDIR64BIT}\"
	echo LIBDIR32BIT \"${LIBDIR32BIT}\"
	echo LIBDIR64BIT \"${LIBDIR64BIT}\"
fi

if test -z "${OS}"
then
	echo cannot determine OS > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${DISTRIBUTION}"
then
	echo cannot determine DISTRIBUTION > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${WORDSIZE}"
then
	echo cannot determine WORDSIZE > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${OSVERSION}"
then
	echo cannot determine OSVERSION > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${ARCHITECTURE}"
then
	echo cannot determine ARCHITECTURE > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${EXECSUBDIR32BIT}"
then
	echo cannot determine EXECSUBDIR32BIT > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${EXECSUBDIR64BIT}"
then
	echo cannot determine EXECSUBDIR64BIT > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${LIBDIR32BIT}"
then
	echo cannot determine LIBDIR32BIT > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${LIBDIR64BIT}"
then
	echo cannot determine LIBDIR64BIT > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

# don't exit (just fall off end) so that this file can be sourced
#exit 0
