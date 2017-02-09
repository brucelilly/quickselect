# Description: set environment variables:
#  HOSTNAME
#  HOST
#  DOMAIN
#  FQDN
# requires environment variables:
#  OS

# The following license covers this software, including makefiles and documentation:

# The following license covers this software, including makefiles and documentation:
# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
################### Copyright notice (part of the license) ###################
#$Id: ~\|^` @(#)%M% copyright 2015 - 2016 %Q%\ host.sh $
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
			debugflag="${debugflag} $1"
		;;
		-*n*)	debug=`expr ${debug} + 1`
			debugflag="${debugflag} $1"
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
	thisfile=${dir}/host.sh
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
prog=uname
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=tr
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=hostname
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=cat
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
if test 0 -eq 1
then
prog=domainname
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
fi

uname_name=`uname -n | tr -d " "`
if test ${debug} -gt 0
then
#	echo "uname -i (hardware platform)" `uname -i` # not in Minix
#	echo "uname -m (machine hardware name)" `uname -m`
	echo "uname -n (network node hostname)" ${uname_name}
#	echo "uname -o (operating system)" `uname -o` # not in Minix, base Solaris
#	echo "uname -p (processor type)" `uname -p`
#	echo "uname -r (kernel release)" `uname -r`
#	echo "uname -s (kernel name)" `uname -s`
#	echo "uname -v (kernel version)" `uname -v`
fi

if test -z "${HOSTNAME}" -o "${HOSTNAME}" != "${uname_name}"
then
	export HOSTNAME=${uname_name}
fi

hostname_name=`hostname | tr -d " "`
if test ${debug} -gt 0
then
	echo "hostname" ${hostname_name}
fi

if test "${uname_name%%.*}" != "${hostname_name%%.*}"
then
	echo uname_name "${uname_name}" differs from hostname_name "${hostname_name}"
	# should do something here to disambiguate:
	#  could try to get interface MAC address, then look up name in /etc/ethers [see /etc/makefile.files]
fi

# make sure hostname is fully-qualified
if test -z "${HOSTNAME}" -o "${HOSTNAME}" != "${hostname_name}"
then
	export HOSTNAME=${hostname_name}
fi

export HOST=${HOSTNAME%%.*}
export DOMAIN=`domainname | tr -d " "`
DOMAIN="${DOMAIN%'.(none)'}"
if test "${DOMAIN}" = "(none)" -o "${DOMAIN}" = "--version"
then
	DOMAIN=""
fi
if test -z "${DOMAIN}"
then
	if test -s /etc/defaultdomain
	then
		DOMAIN=`cat /etc/defaultdomain | tr -d " "`
	fi
fi
if test "${DOMAIN}" = "(none)" -o "${DOMAIN}" = "--version"
then
	DOMAIN=""
fi
if test -z "${DOMAIN}"
then
	if test "${HOSTNAME}" != "${HOST}" -a "${HOSTNAME}" != "${HOST}."
	then
		export DOMAIN=${HOSTNAME#*.}
	fi
fi
if test "${DOMAIN}" = "(none)" -o "${DOMAIN}" = "--version"
then
	DOMAIN=""
fi
if test -z "${DOMAIN}"
then
	DOMAIN=blilly.net
fi
if test ! -s /etc/defaultdomain
then
	echo ${DOMAIN} > /etc/defaultdomain 2>/dev/null
fi
export FQDN=${HOST}.${DOMAIN}
if test "${HOSTNAME}" != "${FQDN}"
then
	export HOSTNAME=${FQDN}
fi

if test ${debug} -gt 0
then
	echo HOSTNAME \"${HOSTNAME}\"
	echo HOST \"${HOST}\"
	echo DOMAIN \"${DOMAIN}\"
	echo FQDN \"${FQDN}\"
fi

if test -z "${HOSTNAME}"
then
	echo cannot determine HOSTNAME > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${HOST}"
then
	echo cannot determine HOST > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${DOMAIN}"
then
	DOMAIN=example.net
fi
if test -z "${DOMAIN}"
then
	echo cannot determine DOMAIN > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${FQDN}"
then
	echo cannot determine FQDN > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

if test ${debug} -gt 0 -o -z "${OS}" -o -z "${WORDSIZE}" -o -z "${OSVERSION}" -o -z "${DISTRIBUTION}" -o -z "${ARCHITECTURE}" -o -z "${EXECSUBDIR32BIT}" -o -z "${EXECSUBDIR64BIT}" -o -z "${LIBDIR32BIT}" -o -z "${LIBDIR64BIT}"
then
	if test -f ${dir}/system.sh
	then
		. ${dir}/system.sh ${debugflag} -p ${dir}/system.sh
	fi
fi
case ${OS} in
	solaris)
		if test ! -s /etc/nodename
		then
			echo ${FQDN} > /etc/nodename
		fi
		foo=`cat /etc/nodename`
		if test "${foo}" != "${FQDN}"
		then
			echo ${FQDN} > /etc/nodename
		fi
	;;
	*bsd)
		if test ! -s /etc/myname
		then
			echo ${FQDN} > /etc/myname
		fi
		foo=`cat /etc/myname`
		if test "${foo}" != "${FQDN}"
		then
			echo ${FQDN} > /etc/myname
		fi
	;;
	linux)
		if test ! -s /etc/HOSTNAME
		then
			echo ${FQDN} > /etc/HOSTNAME
		fi
		foo=`cat /etc/HOSTNAME`
		if test "${foo}" != "${FQDN}"
		then
			echo ${FQDN} > /etc/HOSTNAME
		fi
	;;
	*)
	;;
esac

# don't exit (just fall off end) so that this file can be sourced
#exit 0
