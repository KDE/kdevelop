#!/usr/bin/env python

# Preprocess Makefile.am for PyQT project
# Julian Rockey 2002

# TODO:
#   Test real_lines properly
#   Process Makefile.am's in sub-directories

from __future__ import generators
import sys
import re

# function to convert list of lists with continuation chars
# (bashslashes) to list of actual lines
def real_lines(m):
    while len(m)>0:
        result = ""
        if m[0][-1]=='\\':
            while (len(m)>0 and m[0][-1]=='\\'):
                result += m[0][:-1]
                m=m[1:]
        if len(m)>0:
            result += m[0]
            m=m[1:]
        yield result

# check for arguments
if len(sys.argv)<2:
    makefile_am = "Makefile.am"
else:
    makefile_am = sys.argv[1]


# read in Makefile.am
f = open(makefile_am,"r")
mf = f.readlines()
f.close()

# parse for variables
re_variable = re.compile("^(#kdevelop:[ \t]*)?([A-Za-z][A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$")
variables = {}


for l in real_lines(mf):
    m = re_variable.match(l)
    if m!=None:
        (nowt, lhs, rhs) = m.groups()
        variables[lhs]=rhs

# look for app sources
for (var,data) in variables.items():
    spl = var.split("_",1)
    spl.append("")
    (prefix,primary) = (spl[0], spl[1])
    if primary=="PROGRAMS":
        target=data
        if variables.has_key(target + "_SOURCES"):
            sources = variables[target+"_SOURCES"].split()

# change .ui into .py
py_sources = []
for s in sources:
    py_sources.append(re.sub("\.ui$",".py",s))
    
# replace dependencies of main target with list of .py sources
# FIXME escape target
re_maintarget = re.compile("(" + target + ".*?:).*")
out = []
for l in mf:
    m = re_maintarget.match(l)
    if m!=None:
        (tar,)=m.groups()
        l=tar + " " + " ".join(py_sources) + "\n"
    out.append(l)

# backup existing Makefile.am
f = open(makefile_am + "~", "w")
f.writelines(mf)
f.close()

# write file
f = open(makefile_am, "w")
f.writelines(out)
f.close()
