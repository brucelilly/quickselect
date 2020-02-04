# Description: set environment variables:
#  NETWORKS
#  ADDRESS
#  IPV6_PREFIX
#  DEFAULT_ROUTE
#  DNS_MASTER
#  DNS_TYPE
#  MAIL_EXCHANGER
# requires environment variables:
#  FQDN
#  OS

# The following license covers this software, including makefiles and documentation:
# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
################### Copyright notice (part of the license) ###################
#$Id: ~\|^` @(#)%M% copyright 2015 - 2016 %Q%\ network.sh $
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
	thisfile=${dir}/network.sh
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

if test ${debug} -gt 0
then
	echo thisfile ${thisfile}
fi
dir=`dirname ${thisfile}`

# awk and sed programs are mandatory
prog=awk
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=sed
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi

if test ${debug} -gt 0 -o -z "${FQDN}" -o -z "${HOST}" -o -z "${HOSTNAME}" -o -z "${DOMAIN}" -o "${HOST}" != "${HOSTNAME%%.*}" -o "${HOST}.${DOMAIN}" != "${FQDN}"
then
	if test -f ${dir}/host.sh
	then
		. ${dir}/host.sh ${debugflag} -p ${dir}/host.sh
	fi
fi
if test -z "${FQDN}"
then
	echo cannot determine FQDN > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

# in debug mode, system.sh is sourced from host.sh
if test -z "${OS}" -o -z "${WORDSIZE}" -o -z "${OSVERSION}" -o -z "${DISTRIBUTION}" -o -z "${ARCHITECTURE}" -o -z "${EXECSUBDIR32BIT}" -o -z "${EXECSUBDIR64BIT}" -o -z "${LIBDIR32BIT}" -o -z "${LIBDIR64BIT}"
then
	if test -f ${dir}/system.sh
	then
		. ${dir}/system.sh ${debugflag} -p ${dir}/system.sh
	fi
fi
if test -z "${OS}"
then
	echo cannot determine OS > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi

# test for availability of other required tools
prog=netstat
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
#prog=dig
#path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
#if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
#if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
prog=grep
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=egrep
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=head
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=cut
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
prog=sort
path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi

export NETWORKS=""
export DEFAULT_ROUTE=""
case ${OS} in
	solaris)
		NETWORKS=`netstat -r -f inet | grep "^[0-9][0-9.]*[ 	]" | awk '{print $1}' | sort -u`
		DEFAULT_ROUTE=`netstat -r -f inet -n | grep "^default[ 	]" | awk '{print $2}'`
	;;
	*bsd)
		NETWORKS=`netstat -r -f inet -n | egrep -v "^127|fwip[0-9]" | grep "^[0-9][0-9./]*[ 	]" | awk '{print $1}' | sed -e "s,/24,.0," | awk -F . '{print $1 "." $2 "." $3 "." $4}' | head -1`
		DEFAULT_ROUTE=`netstat -r -f inet -n | grep "^default[ 	]" | awk '{print $2}'`
		# try to fix missing default route
		if test -z "${DEFAULT_ROUTE}"
		then
			if test -a -r /etc/mygate -a -s /etc/mygate
			then
				uid=`id -u`
				if test ${uid} -eq 0
				then
					route add default `cat /etc/mygate`
				else
					echo -n "set default route: route add default "
					cat /etc/mygate
				fi
			fi
		fi
	;;
	linux)
		NETWORKS=`netstat -A inet -r | grep "^[0-9][0-9.]*[ 	]" | awk '{print $1}' | sort -u`
		DEFAULT_ROUTE=`netstat -A inet -r -n | grep "^0[.]0[.]0[.]0[ 	]" | awk '{print $2}'`
	;;
	*)
	;;
esac
if test ${debug} -gt 0
then
	echo NETWORKS \"${NETWORKS}\"
	echo DEFAULT_ROUTE \"${DEFAULT_ROUTE}\"
fi

case ${OS} in
	solaris)
		DIG=/usr/sbin/dig
	;;
	*)
		DIG=dig
	;;
esac
export IPV6_PREFIX=`ifconfig -a | grep inet6 | egrep -v " fe80::| ::1" | sed -e "s/addr: //" -e "s/inet6 //" | cut -d: -f1-4 | awk '{print $1 ; exit 0}'`
if test ${debug} -gt 0
then
	echo IPV6_PREFIX \"${IPV6_PREFIX}\"
fi
# if no nameservers are available, dig puts an error message on stdout, which gets assigned
#export ADDRESS=`${DIG} +short ${FQDN} A` 
#if test -z "${ADDRESS}"
#then
	export ADDRESS=`grep ${FQDN} /etc/hosts | grep -v '^#' | grep -v "::" | grep -v "^127." | awk '{print $1}'`
#fi
if test -z "${ADDRESS}"
then
	prog=ifconfig
	path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
	if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
	if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
	prog=tail
	path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
	if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
	if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
	export ADDRESS=`ifconfig -a | grep "[ 	]*inet " | tail -1 | awk '{print $2 }'`
fi
if test ${debug} -gt 0
then
	echo ADDRESS \"${ADDRESS}\"
fi
if test -z "${ADDRESS}"
then
	prog=nslookup
	path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
	if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
	if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
	prog=tail
	path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
	if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
	if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
	export ADDRESS=`nslookup -type=A ${FQDN} | grep "^Address" | grep -v "#53" | tail -1 | awk '{print $NF }'`
fi
if test ${debug} -gt 0
then
	echo ADDRESS \"${ADDRESS}\"
fi

# set DNS_MASTER, DNS_TYPE and MAIL_EXCHANGER based on site-wide host configuration if possible
export DNS_MASTER=""
export DNS_TYPE=""
export MAIL_EXCHANGER=""
CONFIG_HOME=/data/config
SERVICES_FILE=services.txt
if test -d ${CONFIG_HOME} -a -r ${CONFIG_HOME}/${SERVICES_FILE}
then
	# determine host name of DNS master
	DNS_MASTER=`grep dns ${CONFIG_HOME}/${SERVICES_FILE} | egrep -v "^(#|all-services)" | head -1 | cut -f1`
	if test -n "${DNS_MASTER}"
	then
		# reduce to nickname, then add domain
		DNS_MASTER="${DNS_MASTER%%.*}.${DOMAIN}"
		if test ${debug} -gt 1
		then
			echo DNS_MASTER "(site-wide configuration)" \"${DNS_MASTER}\"
		fi
	fi
	# if debugging, print determination
	if test ${debug} -gt 0
	then
		echo DNS_MASTER \"${DNS_MASTER}\"
	fi

	export DNS_TYPE=master
	if test -n "${DNS_TYPE}"
	then
		if test ${debug} -gt 0
		then
			echo DNS_TYPE "(based on all masters)" \"${DNS_TYPE}\"
		fi
	fi

	if test -z "${DNS_TYPE}" -a -n "${DNS_MASTER}"
	then
		if test "${DNS_MASTER}" = "${FQDN}"
		then
			DNS_TYPE=master
		else
			DNS_TYPE=slave
		fi
		if test ${debug} -gt 0
		then
			echo DNS_TYPE "(based on DNS_MASTER)" \"${DNS_TYPE}\"
		fi
	fi
	MAIL_EXCHANGER=`grep mail ${CONFIG_HOME}/${SERVICES_FILE} | egrep -v "^(#|all-services)" | head -1 | cut -f1`
	if test -n "${MAIL_EXCHANGER}"
	then
		# reduce to nickname, then add domain
		MAIL_EXCHANGER="${MAIL_EXCHANGER%%.*}.${DOMAIN}"
		if test ${debug} -gt 1
		then
			echo MAIL_EXCHANGER "(site-wide configuration)" \"${MAIL_EXCHANGER}\"
		fi
		# if debugging, print determination
		if test ${debug} -gt 0
		then
			echo MAIL_EXCHANGER \"${MAIL_EXCHANGER}\"
		fi
	fi
fi

# generic recipes for DNS_MASTER and MAIL_EXCHANGER (used if site-wide information was not conclusive)
if test -r /etc/resolv.conf
then
	if test -z "${DNS_MASTER}"
	then
		prog=host
		path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
		if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
		if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
		default_route_is_nameserver=0
		for server in `grep ^nameserver /etc/resolv.conf | awk '{print $2}'`
		do
			# only use router as a last resort (see below)
			if test "${server}" = "${DEFAULT_ROUTE}"
			then
				default_route_is_nameserver=1
				if test ${debug} -gt 1
				then
					echo rejecting \"${server}\" from /etc/resolv.conf as DEFAULT_ROUTE router "(${DEFAULT_ROUTE})"
				fi
				continue
			fi
			if print ${server} | grep ':' >/dev/null 2>&1
			then
				if test ${debug} -gt 1
				then
					echo rejecting ipv6 \"${server}\" from /etc/resolv.conf
				fi
				continue
			fi
			if print ${server} | grep '127[.]0[.]0' >/dev/null 2>&1
			then
				if test ${debug} -gt 1
				then
					echo rejecting localhost \"${server}\" from /etc/resolv.conf
				fi
				continue
			fi
			if test ${debug} -gt 1
			then
				echo trying \"${server}\" from /etc/resolv.conf
			fi
			DNS_MASTER=`host ${server} | awk '{print $NF}' | sed -e 's/[.]$//'`
			if test -n "${DNS_MASTER}"
			then
				# reduce to nickname, then add domain
				DNS_MASTER="${DNS_MASTER%%.*}.${DOMAIN}"
				if test ${debug} -gt 1
				then
					echo DNS_MASTER "(generic rule using /etc/resolv.conf)" \"${DNS_MASTER}\"
				fi
				break
			fi
		done
	fi
	if test -z "${DNS_MASTER}"
	then
		if test ${default_route_is_nameserver} -gt 0
		then
			DNS_MASTER=`host ${DEFAULT_ROUTE} | awk '{print $NF}' | sed -e 's/[.]$//'`
			if test -n "${DNS_MASTER}"
			then
				# reduce to nickname, then add domain
				DNS_MASTER="${DNS_MASTER%%.*}.${DOMAIN}"
				if test ${debug} -gt 1
				then
					echo DNS_MASTER "(generic rule using DEFAULT_ROUTE nameserver from /etc/resolv.conf)" \"${DNS_MASTER}\"
				fi
				if test ${debug} -gt 0
				then
					echo DNS_MASTER \"${DNS_MASTER}\"
				fi
			fi
		fi
	fi
	if test -z "${DNS_MASTER}"
	then
		DNS_MASTER=${FQDN}
		if test -n "${DNS_MASTER}"
		then
			if test ${debug} -gt 1
			then
				echo DNS_MASTER "(no nameserver found; using this host FQDN)" \"${DNS_MASTER}\"
			fi
		fi
	fi
fi
if test -z "${DNS_TYPE}" -a -n "${DNS_MASTER}"
then
	if test "${DNS_MASTER}" = "${FQDN}"
	then
		DNS_TYPE=master
	else
		DNS_TYPE=slave
	fi
	if test ${debug} -gt 0
	then
		echo DNS_TYPE "(based on DNS_MASTER)" \"${DNS_TYPE}\"
	fi
fi
if test -z "${MAIL_EXCHANGER}"
then
	prog=host
	path=`whence -v ${prog} 2>/dev/null | awk '{ print $NF }'`
	if test -z "${path}" ; then path=`which -a ${prog} 2>/dev/null | awk '{ print $NF }'` ; fi
	if test -z "${path}" ; then echo ${thisfile}: mandatory program ${prog} cannot be found ; return 1 2>/dev/null || exit 1 ; fi
	MAIL_EXCHANGER=`host -t MX ${FQDN} | sort -k+6 -n | head -1 | awk '{print $NF}' | sed -e 's/[.]$//'`
	if test -n "${MAIL_EXCHANGER}"
	then
		# reduce to nickname, then add domain
		MAIL_EXCHANGER="${MAIL_EXCHANGER%%.*}.${DOMAIN}"
		if test ${debug} -gt 1
		then
			echo MAIL_EXCHANGER "(generic rule using MX DNS query)" \"${MAIL_EXCHANGER}\"
		fi
	fi
	if test -z "${MAIL_EXCHANGER}"
	then
		MAIL_EXCHANGER=${FQDN}
		if test -n "${MAIL_EXCHANGER}"
		then
			if test ${debug} -gt 1
			then
				echo MAIL_EXCHANGER "(no MX record found; using this host FQDN)" \"${MAIL_EXCHANGER}\"
			fi
			break
		fi
	fi
	if test ${debug} -gt 0
	then
		echo MAIL_EXCHANGER \"${MAIL_EXCHANGER}\"
	fi
fi

if test 0 -eq 1
then
if test -z "${NETWORKS}"
then
	echo cannot determine NETWORKS > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${DEFAULT_ROUTE}"
then
	echo cannot determine DEFAULT_ROUTE > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${IPV6_PREFIX}"
then
	echo cannot determine IPV6_PREFIX > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
#[non-fatal]	return 1 2>/dev/null || exit 1
fi
if test -z "${ADDRESS}"
then
	echo cannot determine ADDRESS > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${DNS_MASTER}"
then
	echo cannot determine DNS_MASTER > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${DNS_TYPE}"
then
	echo cannot determine DNS_TYPE > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
if test -z "${MAIL_EXCHANGER}"
then
	echo cannot determine MAIL_EXCHANGER > /dev/stderr
	# some implementations of sh refuse to accept return unless script is sourced
	return 1 2>/dev/null || exit 1
fi
fi

# don't exit (just fall off end) so that this file can be sourced
#exit 0
