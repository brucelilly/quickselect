# Description: awk script to process cflow output for generating dot (graphviz) diagrams

# The following license covers this software, including makefiles and documentation:

# This software is covered by the zlib/libpng license.
# The zlib/libpng license is a recognized open source license by the
# Open Source Initiative: http://opensource.org/licenses/Zlib
# The zlib/libpng license is a recognized "free" software license by the
# Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
# ****************** Copyright notice (part of the license) *******************
# $Id: ~\|^` @(#)%M% %I% %O% copyright 2015 %Q%\ cflow_dot.awk $
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
# You may send bug reports to %Y% with subject "cflow_dot.awk".
# *****************************************************************************
# maintenance note: master file  %P%

BEGIN	{
	FS = "[:<>\t]+";
        if (length(debug "") < 1)
		debug = 0
	current_indent = 0
}

{
	if ($0 ~ /^[0-9][0-9]*[ \t]/) { # original cflow output format
#original cflow output:
#1	snlround: long(), <snn.c 473>
#2		int_d: long(), <snn.c 347>
#3			int_d: 2
#4	sng: int(), <snn.c 1194>
#5		errno: <>
#6		sng: 4
#7		snn_initialized: unsigned char, <snn.c 216>
#8		initialize_snn: void(), <snn.c 287>
#9			__file__: unsigned char*, <snn.c 218>
#10			isalnum: <>
#11			filenamebuf: unsigned char[], <snn.c 217>
#12			snn_c_rcs_sccs_id: unsigned char[], <snn.c 5>
#13			snn_initialized: 7
#14			snn_h_rcs_sccs_id: unsigned char[], <snn.h 7>
#15			compiled: unsigned char[], <snn.c 261>
#16			snn_almost_one: double, <snn.c 220>
#17			snn_epsilon: double, <snn.c 221>
#18			sn1en: double(), <snn.c 446>
#19				snl: int(), <snn.c 818>
#20					snul: int(), <snn.c 708>
#21						errno: 5
		if (debug > 0) {
			printf "old format line: %s\n", $0 >> "/dev/stderr"
		}
		temp = $2
		sub(/[ \t]*/, "", temp)
		current_name = temp
		x = index($0, current_name)
		y = length($1 "")
		current_indent = x - y - 2
		func_name[current_indent] = current_name
		if (debug > 0) {
			printf "current_name: %s, index($0, %s) = %d, length(%d) = %d, current_indent %d\n", current_name, current_name, x, $1+0, y, current_indent >> "/dev/stderr"
		}
		temp = $3 ""
		sub(/^[ \t]+/, "", temp)
		sub(/[ \t]+$/, "", temp)
		if (debug > 0) {
			printf "current_name: %s, temp = \"%s\"\n", current_name, temp >> "/dev/stderr"
		}
		if (temp ~ /^[1-9][0-9]*$/) {
			n = temp + 0
			if (debug > 0) {
				printf "current_name: %s, temp = \"%s\" = %d\n", current_name, temp, n >> "/dev/stderr"
			}
			v = ntype[n]
			if (debug > 0) {
				printf "%s was previously defined on line %d with type %d\n", current_name, n, v >> "/dev/stderr"
			}
			type = v
		} else if (temp !~ /.*[(].*[)].*/) {
			if (debug > 0) {
				printf "current_name: %s, temp = \"%s\" is not a number\n", current_name, temp >> "/dev/stderr"
			}
			v = 1 # global function? [cannot distinguish external data (e.g. errno) from external function calls (e.g. strtod()) in old format on OpenIndiana; don't use cflow -ix]
			n = $1 + 0
			ntype[n] = v
			type = 0
		} else {
			if (debug > 0) {
				printf "current_name: %s, temp = \"%s\" is not a number\n", current_name, temp >> "/dev/stderr"
			}
			v = 1 # function
			n = $1 + 0
			ntype[n] = v
			type = 0 # global; changed if file is specified
		}
		if (0 == current_indent) {
			previous_name = ""
			previous_indent = 0
		} else {
			for (n=current_indent-1; 0<=n; n--) {
				if (n in func_name) {
					previous_indent = n
					previous_name = func_name[n]
					break
				}
			}
		}
		if (debug > 0) {
			printf "previous_name: %s\n", previous_name >> "/dev/stderr"
		}
		if (3 < NF) {
			temp = $4
			gsub(/-/, "_", temp); # '-' is a problem for dot
			n = split(temp, ary, /[ \t]/)
			x = ary[1]
			current_file = x;
			if (1 < n) {
				line = ary[n] + 0
			}
			type = v
			n = $1 + 0
			ntype[n] = v
			definition(current_name, type, current_file, line)
		} else {
			definition(current_name, type, "", 0)
		}
		if (debug > 0) {
			printf "type: %s: %d\n", current_name, type >> "/dev/stderr"
		}
		reference(previous_name, current_name, type)
	} else { # newfangled cflow output format
# newfangled cflow output:
#sn1en() <double sn1en (int n) at snn.c:446>:
#    snl() <int snl (char *buf,int sz,const char *prefix,const char *suffix,long l,int base,int pad,int min_dig) at snn.c:818>:
#        snul() <int snul (char *buf,int sz,const char *prefix,const char *suffix,unsigned long ul,int base,int pad,int min_dig) at snn.c:708>:
#    strtod()
#snf() <int snf (char *buf,int sz,const char *prefix,const char *suffix,double d,int pad,int min_dig,int precision) at snn.c:916> (R):
		if (debug > 0) {
			printf "new format line: %s\n", $0 >> "/dev/stderr"
		}
		temp = $1
		n = length(temp)
		sub(/[ ]*/, "", temp)
		current_indent = n - length(temp)
		if (temp !~ /.*[(].*[)].*/) {
			v = 2 # data
			sub(/[ ]*$/, "", temp)
		} else {
			v = 1 # function
			sub(/[(].*[)][ ]*/, "", temp)
		}
		current_name = temp
		if (debug > 0) {
			printf "current_name: %s\n", current_name >> "/dev/stderr"
		}
		if (0 == current_indent) {
			previous_name = ""
			previous_indent = 0
		} else {
			for (n=current_indent-1; 0<=n; n--) {
				if (n in func_name) {
					previous_indent = n
					previous_name = func_name[n]
					break
				}
			}
		}
		if (debug > 0) {
			printf "previous_name: %s\n", previous_name >> "/dev/stderr"
		}
		func_name[current_indent] = current_name
		if (2 != v) {
			type = 0 # global
		} else {
			type = v
		}
		if (1 < NF) {
			temp = $2
			gsub(/-/, "_", temp); # '-' is a problem for dot
			n = split(temp, ary, /[ \t]/)
			x = ary[n]
			current_file = x;
			if (2 < NF) {
				line = $3 + 0
			}
			type = v
			definition(current_name, type, current_file, line)
		} else {
			definition(current_name, type, "", 0)
		}
		if (debug > 0) {
			printf "type: %s: %d\n", current_name, type >> "/dev/stderr"
		}
		reference(previous_name, current_name, type)
	}
}

END	{
		# trim lists; eliminate nodes with no links (typically static char strings used for SCCS ID) and files with no nodes (typically header files)
		for (y in func_names) {
			if (func_names[y] < 1) # global function; keep it
				continue;
			if ((func_names[y] < 2) && (y in f) && !index(f[y], "/")) # function defined in a file in current directory
				continue;
			if (y in global) # caller of a global function; keep it
				continue;
			if (y in called)
				continue;
			if (y in static)
				continue;
			t = 0;
			w = " " y " ";
			for (z in global) {
				if (index(global[z], w)) { # called global (shouldn't happen)
					t = 1;
					for (z in called) {
						v = called[z]
						x = index(v, w);
						if (0 != x) {
							u = substr(v, 0, x-1) substr(v, x+length(w));
							if (0 < debug)
								printf "eliminating global %s from called[%s]: %s -> %s\n", z, z, v, u >> "/dev/stderr"
							called[z] = u
						}
					}
					break;
				}
			}
			if (0 == t) {
				for (z in called) {
					if (index(called[z], w)) {
						t = 1;
						break;
					}
				}
			}
			if (0 == t) {
				for (z in static) {
					if (index(static[z], w)) {
						t = 1;
						break;
					}
				}
			}
			if (0 != t)
				continue;
			for (z in files) {
				v = fv[z];
				w = "; " y " ";
				x = index(v, w);
                                if (!x) {
					w = "; " y " [color=blue] ";
					x = index(v, w);
				}
                                if (!x) {
					w = " " y " ;";
					x = index(v, w);
				}
                                if (!x) {
					w = " " y " [color=blue] ;";
					x = index(v, w);
				}
                                if (!x) {
					w = " " y " ";
					x = index(v, w);
				}
                                if (!x) {
					w = " " y " [color=blue] ";
					x = index(v, w);
				}
				if (x) {
					fv[z] = substr(v, 0, x-1) substr(v, x+length(w));
					if (0 < debug)
						printf "fv[%s]: eliminating %s: %s -> %s\n", z, y, v, fv[z] >> "/dev/stderr"
					v = fv[z];
				}
				if (length(v) < 1) {
					delete fv[z];
					delete files[z];
				}
			}
			if (func_names[y] == 1)
				printf "%s not referenced\n", y >> "/dev/stderr"
			delete name[y];
		}
		# generate output
		printf "digraph cflow_call_graph {\n";
		printf "size=\"17,11\"; ratio=fill; margin=0; center=1; rotate=90;\n";
		for (z in files) {
			x = z;
			gsub(/[.\/]/, "_", x);
			if (0 < length(fv[z]))
				printf "subgraph cluster_%s { %s_name_node [label=\"%s\", shape=box, color=red]; %s }\n", x, x, z, fv[z];
		}
		# globals
		n = 0
		for (y in func_names) {
			if (y in global) {
				if (0 == n++)
					printf "edge [color=green];\n";
				printf "%s -> {%s};\n", y, global[y];
			}
		}
		# static (local) calls
		n = 0
		for (y in func_names) {
			if (y in static) {
				if (0 == n++)
					printf "edge [color=blue];\n";
				printf "%s -> {%s};\n", y, static[y];
			}
		}
		# non-global, non-local calls (callee needs to be visible, but isn't global)
		n = 0
		for (y in func_names) {
			if (y in called) {
				if (0 == n++)
					printf "edge [color=red];\n";
				printf "%s -> {%s};\n", y, called[y];
			}
		}
		printf "}\n";
}

function reference(caller, callee, type) {
	if (debug > 0) {
		if ((2 != type) && (0 < length(caller)))
			printf "%s calls %s, type %d\n", caller, callee, type >> "/dev/stderr"
	}
	if (2 != type) {
		# build lists of functions/variables referenced by current_name (caller)
		if ((callee in func_names) && (func_names[callee] == 0)) { # callee is global
			if (caller in global) {
				if (!index(global[caller], " " callee " "))
					global[caller] = global[caller] "; " callee " ";
			} else
				global[caller] = " " callee " ";
		} else {
			if ((caller in f) && (callee in f) && (f[caller] == f[callee])) { # callee could be static
				if (caller in static) {
					if (!index(static[caller], " " callee " "))
						static[caller] = static[caller] "; " callee " ";
				} else
					static[caller] = " " callee " ";
			} else {
				if (caller in called) {
					if (!index(called[caller], " " callee " "))
						called[caller] = called[caller] "; " callee " ";
				} else
					called[caller] = " " callee " ";
			}
		}
		if (!(callee in func_names))
			func_names[callee] = type;
	}
	return caller
}

function definition(name, type, fname, line) {
	if (2 != type) {
		if (0 < length(fname)) {
			f[name] = file(fname)
			if (fname in fv) {
				if (!index(fv[fname], " " name " ")) {
					fv[fname] = fv[fname] "; " name " ";
				}
			} else {
				fv[fname] = " " name " ";
			}
		}
		func_names[name] = type; # add caller to associative array of names, indicate whether it's a function or variable
	}
	if (debug > 0) {
		printf "defined %s \"%s\" in file %s line %d\n", type==1?"function":"variable", name, fname, line >> "/dev/stderr"
	}
	return name
}

function file(fname) {
	files[fname] = 1; # add file to associative array of file names
	return fname
}
