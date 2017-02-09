# Description: awk script to massage indentation of case and termination statements after processing by beautifiers

# The following license covers this software, including makefiles and documentation:

# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "\"free\"" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
# ****************** Copyright notice (part of the license) *******************
# $Id: ~\|^` @(#)%M% copyright 2015 %Q%\ fix_switch.awk $
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
# This is %M% version %I% dated %O%.
# You may send bug reports to %Y% with subject "fix_switch".
# *****************************************************************************
# maintenance note: master file  %P%

BEGIN {
	if (length(debug "") == 0)
		debug = 0
	# output file comment format
	commentstart = "/* "
	commentend = " */\n"
	# output generator information
	script = "%M%"
	if (script ~ /[%]M[%]/)
		script = "fix_switch.awk"
	sversion = "%I%"
	if (sversion ~ /[%]I[%]/)
		sversion = "0.0"
	# which host is this script being run from?
	if (length(hostname "") < 1)
		hostname = ENVIRON["HOSTNAME"]
	if (length(hostname "") < 1)
		hostname = ENVIRON["HOST"]
	if (length(hostname "") > 0) {
		z = split(hostname, xx, ".")
		if (z < 2) {
			domain = ENVIRON["DOMAIN"]
			if (length(domain "") > 0)
				hostname = hostname "." domain
			z = split(hostname, xx, ".")
			if (z < 2) {
				# OpenIndiana and NetBSD hostname take no options, but always return FQDN
				"hostname" | getline hostname
				z = split(hostname, xx, ".")
				# Linux hostname sometimes returns nickname unless "-f" option is supplied
				if (z < 2) {
					"hostname -f" | getline hostname
				}
			}
		}
	}
	if (length(hostname "") < 1) {
		# OpenIndiana and NetBSD hostname take no options
		"hostname" | getline hostname
		# Linux hostname sometimes returns nickname unless "-f" option is supplied
		if (z < 2) {
			"hostname -f" | getline hostname
		}
	}
	# temporary date for logging
	# Openindiana gawk 3.1.5 (gawk 4.0.2 is OK) cannot handle 3rd argument to strftime, even if not executed
	# some versions of nawk have no systime() function
	command = ARGV[0] " --version 2>/dev/null"
	command | getline awk_version
	if (1 > length(awk_version))
		awk_version = ARGV[0]
	command = "date -u +%Y-%m-%dT%H:%M:%SZ"
	command | getline udate
	command = "date '+%Y-%m-%d %H:%M:%S %Z'"
	command | getline ldate
	cdate = udate " (" ldate ")"
	if (length(debug "") == 0)
		debug = 0
	if (length(hostname "") < 1)
		printf "%smodified by %s version %s (%s) at %s, debug=%d%s", commentstart, script, sversion, awk_version, cdate, debug+0, commentend
	else
		printf "%smodified by %s version %s (%s) at %s on %s, debug=%d%s", commentstart, script, sversion, awk_version, cdate, hostname, debug+0, commentend
	commentstart = commentstart "" script ": "
	case_indent_size = 4
	in_switch = 0
	in_braces = 0
	in_case = 0
	switch_prefix[0] = ""
	tab_spaces = ""
	for (i=0; i<case_indent_size; i++)
		tab_spaces = tab_spaces " "
}

# may fail for code that is #ifdef wrapped
# may fail for code with keywords in comments
/[ \t:]*switch[ \t]*[(]..*[)]/ {
	temp = $0
	statement = temp
	prefix = temp
	sub(/^[ \t:]*/, "", statement) # switch and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	sub(/switch.*/, "", prefix) # prefix before switch
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	if ((0 < in_switch) && (in_case == in_switch)) {
		mod_prefix = switch_prefix[in_switch] "" tab_spaces "" tab_spaces
	} else {
		mod_prefix = switch_prefix[in_switch] "" tab_spaces
	}
	while (length(prefix) < length(mod_prefix))
		prefix = prefix " "
	in_switch++
	switch_prefix[in_switch] = mod_prefix
	if (0 < debug)
		printf "%sswitch statement, level %d, in_case %d, indent %d%s", commentstart, in_switch, in_case, length(mod_prefix), commentend
	i = match(statement, /{/)
	if (0 == i)
		brace_level[in_switch] = -1
	else {
		brace_level[in_switch] = in_braces
		in_braces++
		if (0 < debug)
			printf "%sopening brace, in_switch level %d, in_braces %d%s", commentstart, in_switch, in_braces, commentend
	}
	printf "%s%s\n", prefix, statement
	next
}

# beware braces in comments, multiple braces per line
/[{]/ {
	if (0 < in_switch) {
		if (brace_level[in_switch] == -1)
			brace_level[in_switch] = in_braces
		in_braces++
		if (0 < debug)
			printf "%sopening brace, in_switch level %d, in_braces %d%s", commentstart, in_switch, in_braces, commentend
	}
# continue normal processing
}

# beware braces in comments, multiple braces per line
/[}]/ {
	if (0 < in_switch) {
		in_braces--
		if (in_braces == brace_level[in_switch]) {
			mod_prefix = switch_prefix[in_switch]
			in_switch--
			if (0 < debug)
				printf "%sclosing brace in_braces %d, return to in_switch level %d%s", commentstart, in_braces, in_switch, commentend
			temp = $0
			prefix = temp
			statement = temp
			sub(/^[^}]*/, "", statement) # (first) brace and remainder of line
			sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
			sub(/}.*/, "", prefix) # prefix before (first) brace
			gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
			if (length(prefix) > length(mod_prefix))
				sub(/[ ]*$/, "", prefix) # shorten prefix at end if too long 
			while ((length(prefix) > length(mod_prefix)) && match(prefix, /^[ ]/))
				sub(/^[ ]/, "", prefix) # shorten prefix at beginning if too long and if possible
			while (length(prefix) < length(mod_prefix))
				prefix = prefix " "
			printf "%s%s\n", prefix, statement
			next
		}
	}
# continue normal processing
}

/^[ \t]*case[ \t]*..*[:]/ {
	temp = $0
	statement = temp
	sub(/^[ \t]*/, "", statement) # case and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	case_indent[in_switch] = index(temp, "case")
	prefix = switch_prefix[in_switch] "" tab_spaces
	if (0 < debug)
		printf "%scase statement, level %d, old indent %d -> %d%s", commentstart, in_switch, case_indent[in_switch], length(prefix), commentend
	printf "%s%s\n", prefix, statement
	in_case = in_switch
	next
}

# some versions of nawk cannot handle [/] in RE
#/^[ \t]*[/][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][/][ \t]*case[ \t]*..*[:]/ {
/^[ \t]*[\057][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][\057][ \t]*case[ \t]*..*[:]/ {
	temp = $0
	statement = temp
	# some versions of nawk cannot handle [/] in RE
	#sub(/^[ \t]*[/][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][/][ \t]*/, "", statement) # case and remainder of line
	sub(/^[ \t]*[\057][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][\057][ \t]*/, "", statement) # case and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	prefix = switch_prefix[in_switch] "" tab_spaces
	printf "%s/*FALLTHROUGH*/\n%s%s\n", prefix, prefix, statement
	in_case = in_switch
	next
}

/^[ \t]*default[ \t]*:/ {
	temp = $0
	statement = temp
	sub(/^[ \t]*/, "", statement) # default label and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	case_indent[in_switch] = index(temp, "default")
	prefix = switch_prefix[in_switch] "" tab_spaces
	if (0 < debug)
		printf "%sdefault case statement, level %d, old indent %d -> %d%s", commentstart, in_switch, case_indent[in_switch], length(prefix), commentend
	printf "%s%s\n", prefix, statement
	in_case = in_switch
	next
}

# some versions of nawk cannot handle [/] in RE
#/^[ \t]*[/][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][/][ \t]*default[ \t]*:/ {
/^[ \t]*[\057][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][\057][ \t]*default[ \t]*:/ {
	temp = $0
	statement = temp
	# some versions of nawk cannot handle [/] in RE
	#sub(/^[ \t]*[/][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][/][ \t]*/, "", statement) # case and remainder of line
	sub(/^[ \t]*[\057][*][ \t]*FALLTHR[A-Z]*[ \t]*[*][\057][ \t]*/, "", statement) # case and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	prefix = switch_prefix[in_switch] "" tab_spaces
	printf "%s/*FALLTHROUGH*/\n%s%s\n", prefix, prefix, statement
	in_case = in_switch
	next
}

# label other than "default", possibly inside a compound case statement
/^[ \t]*[A-za-z][A-za-z0-9_]*[ \t]*:/ {
	if (0 < in_switch) {
		temp = $0
		label = temp
		statement = temp
		sub(/^[ \t]*[A-za-z][A-za-z0-9_]*[ \t]*:/, "", statement) # remainder of line after label
		sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
		sub(/:.*$/, ":", label) # elide stuff after label
		sub(/^[ \t]*/, "", label) # elide whitespace before label
		spacer = ""
		if (0 < length(statement)) {
			if ((0 < in_switch) && (in_case == in_switch)) {
				mod_prefix = switch_prefix[in_switch] "" tab_spaces "" tab_spaces
			} else {
				mod_prefix = switch_prefix[in_switch] "" tab_spaces
			}
			while (length(label) + length(spacer) < length(mod_prefix))
				spacer = spacer " "
		}
		if (0 < debug)
			printf "%slabel %s inside switch statement, level %d, in_case %d, indent %d%s", commentstart, label, in_switch, in_case, length(mod_prefix), commentend
		printf "%s%s%s\n", label, spacer, statement
		next
	}
# continue normal processing
}

/^[ \t]*break[ \t]*;/ {
	temp = $0
	prefix = temp
	statement = temp
	sub(/^[ \t]*/, "", statement) # start of statement (w/o leading whitespace) and remainder of line
	sub(/break.*/, "", prefix) # prefix before statement
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	# break can occur outside of switch statements; it can also occur within for and while loops inside compound case statements
	if (0 < in_case) {
		prefix = switch_prefix[in_switch] "" tab_spaces
		i = index(temp, "break")
		if (0 < debug)
			printf "%sbreak statement, level %d, old indent %d -> %d%s", commentstart, in_switch, i, length(prefix), commentend
		in_case--
	}
	printf "%s%s\n", prefix, statement
	next
}

/^[ \t]*continue[ \t]*;/ {
	temp = $0
	prefix = temp
	statement = temp
	sub(/^[ \t]*/, "", statement) # start of statement (w/o leading whitespace) and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	sub(/continue.*/, "", prefix) # prefix before statement
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	# continue can occur outside of switch statements; it can also occur within for and while loops inside compound case statements
	if (0 < in_case) {
		prefix = switch_prefix[in_switch] "" tab_spaces
		i = index(temp, "continue")
		if (0 < debug)
			printf "%scontinue statement, level %d, old indent %d -> %d%s", commentstart, in_switch, i, length(prefix), commentend
		in_case--
	}
	printf "%s%s\n", prefix, statement
	next
}

/^[ \t]*goto[ \t]..*[;]/ {
	temp = $0
	prefix = temp
	statement = temp
	sub(/^[ \t]*/, "", statement) # start of statement (w/o leading whitespace) and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	sub(/goto.*/, "", prefix) # prefix before statement
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	# goto can occur outside of switch statements; it can also occur within for and while loops inside compound case statements
	if (0 < in_switch) {
		prefix = switch_prefix[in_switch] "" tab_spaces
		i = index(temp, "goto")
		if (in_case == in_switch) {
			mod_prefix = switch_prefix[in_switch] "" tab_spaces "" tab_spaces
		} else {
			mod_prefix = switch_prefix[in_switch] "" tab_spaces
		}
		prefix = mod_prefix
		if (0 < debug)
			printf "%sgoto statement, in_switch level %d, in_case %d, old indent %d -> %d%s", commentstart, in_switch, in_case, i, length(prefix), commentend
		if (0 < in_case)
			in_case--
	}
	printf "%s%s\n", prefix, statement
	next
}

/^[ \t]*return[ \t(]*.*[;]/ {
	temp = $0
	prefix = temp
	statement = temp
	sub(/^[ \t]*/, "", statement) # start of statement (w/o leading whitespace) and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	sub(/return.*/, "", prefix) # prefix before statement
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	# return can occur outside of switch statements; it can also occur within for and while loops inside compound case statements
	if (0 < in_case) {
		prefix = switch_prefix[in_switch] "" tab_spaces
		i = index(temp, "return")
		if (0 < debug)
			printf "%sreturn statement, level %d, old indent %d -> %d%s", commentstart, in_switch, i, length(prefix), commentend
		in_case--
	}
	printf "%s%s\n", prefix, statement
	next
}

/^[ \t]*exit[ \t(]*.*[;]/ {
	temp = $0
	prefix = temp
	statement = temp
	sub(/^[ \t]*/, "", statement) # start of statement (w/o leading whitespace) and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	sub(/exit.*/, "", prefix) # prefix before statement
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	# exit can occur outside of switch statements; it can also occur within for and while loops inside compound case statements
	if (0 < in_case) {
		prefix = switch_prefix[in_switch] "" tab_spaces
		i = index(temp, "exit")
		if (0 < debug)
			printf "%sexit call, level %d, old indent %d -> %d%s", commentstart, in_switch, i, length(prefix), commentend
		in_case--
	}
	printf "%s%s\n", prefix, statement
	next
}

/^[ \t]*_exit[ \t(]*.*[;]/ {
	temp = $0
	prefix = temp
	statement = temp
	sub(/^[ \t]*/, "", statement) # start of statement (w/o leading whitespace) and remainder of line
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	sub(/_exit.*/, "", prefix) # prefix before statement
	gsub(/[\t]/, tab_spaces, prefix) # replace tabs in prefix with spaces
	# _exit can occur outside of switch statements; it can also occur within for and while loops inside compound case statements
	if (0 < in_case) {
		prefix = switch_prefix[in_switch] "" tab_spaces
		i = index(temp, "_exit")
		if (0 < debug)
			printf "%s_exit call, level %d, old indent %d -> %d%s", commentstart, in_switch, i, length(prefix), commentend
		in_case--
	}
	printf "%s%s\n", prefix, statement
	next
}

# any line, possibly inside a switch staement
{
	temp = $0
	prefix = ""
	statement = temp
	sub(/[ \t\r\n]*$/, "", statement) # elide whitespace and line ending control characters from string, if any
	if (0 < in_switch) {
		body = statement
		sub(/^[ \t:]*/, "", body) # remainder of line
		old_indent = length(statement) - length(body)
		# don't modify preprocessor directives or comment lines
		j = match(temp, /^[ \t]*[#]/)
		# some versions of nawk cannot handle [/] in RE
		#i = match(temp, /^[ \t]*[/][/*]/)
		i = match(temp, /^[ \t]*[\057][\057*]/)
		if (0 == i + j) {
			sub(/^[ \t]*/, "", statement) # remainder of line after whitespace
			if (0 < length(statement)) {
				if (in_case == in_switch) {
					mod_prefix = switch_prefix[in_switch] "" tab_spaces "" tab_spaces
				} else {
					mod_prefix = switch_prefix[in_switch] "" tab_spaces
				}
				if (old_indent > length(mod_prefix)) {
					mod_prefix = ""
					for (k=0; k<old_indent; k+=case_indent_size) {
						mod_prefix = mod_prefix "" tab_spaces
					}
				}
				prefix = mod_prefix
			}
			if (0 < debug)
				printf "%snon-comment (%d), non-directive (%d) line, in_switch level %d, in_case %d, indent %d -> %d: %s%s", commentstart, i, j, in_switch, in_case, old_indent, length(prefix), statement, commentend
		}
	}
	printf "%s%s\n", prefix, statement
}
