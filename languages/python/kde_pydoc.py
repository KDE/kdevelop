#!/usr/bin/env python
"""
"""

__author__ = "David Nolden<david.kde@art-master.de>,  Ka-Ping Yee <ping@lfw.org>"
__version__ = "6 April 2006"

import sys, imp, os, stat, re, types
from repr import Repr
from string import expandtabs, find, join, lower, split, strip, rstrip
import pydoc




def cleanlinks(string):
    """Changes the links to work with the pydoc:-notation"""
    finalstring = ""
    string = str(string).replace(".html","")
    pos = 0
    mark = "<a href=\""
    l = len(mark)
    while(pos != -1):
        opos = pos
        pos = string.find(mark, pos)
        if(pos == -1):
            finalstring += string[opos:]
            break
        finalstring += string[opos:pos+l]
        pos+=l
        if(string[pos] == '#' or string.find(":/",pos, pos+10) != -1):  #leave local jumps or external references untouched
            continue
        finalstring += "pydoc:"
        if(string[pos] == "." and string[pos+1] == "\""):
            pos += 1
            finalstring += "modules"
        
    return finalstring
    

#This maximum depth was introduced because the program needs a very long time to
#complete the task in big file-system-trees(like my home), and can be invoked by a simple pydoc:.
#and cannot be stopped through the browser(just by killing python)
__maxdepth = 4

def writedocs(path, pkgpath='', depth=0, notprocessed=[]):
    if(path == "/."): 
        writedoc(path)
        return
    depth+=1
    
    if os.path.isdir(path):
        if(depth > __maxdepth):
            notprocessed.append(path)
            return
        dir = path
        for file in os.listdir(dir):
            path = os.path.join(dir, file)
            if os.path.isdir(path):
                writedocs(path, file + '.' + pkgpath, depth)
            if os.path.isfile(path):
                writedocs(path, pkgpath, depth)
    if os.path.isfile(path):
        modname = pydoc.inspect.getmodulename(path)
        if modname:
            writedoc(pkgpath + modname)
            
    if(depth == 1):
        if(len(notprocessed) != 0):
            print "<br> the following paths were not processed because they are deeper than the maximum depth of " + str(__maxdepth) + ":<br>"
            for x in notprocessed:
                print x + "    "

def writedoc(key,top=False):
    """Write HTML documentation to a file in the current directory."""
    if(type(key) == str and (key == "modules" or key == "/.")):
        heading = pydoc.html.heading(
            '<br><big><big><strong>&nbsp;'
            'Python: Index of Modules'
            '</strong></big></big>',
            '#ffffff', '#7799ee')
        builtins = []
        for name in sys.builtin_module_names:
            builtins.append('<a href="%s">%s</a>' % (name, name))
        indices = ['<p>Built-in modules: ' + join(builtins, ', ')]
        seen = {}
        for dir in pydoc.pathdirs():
            indices.append(pydoc.html.index(dir, seen))
        print cleanlinks(heading + join(indices))
        return

    if(type(key) != types.ModuleType):
        object = pydoc.locate(key)
        if(object == None and top):
            print "could not locate module/object for key "+key + "<br><a href=\"pydoc:modules\">go to index</a>";
    else:
        object = key
            
    if object:
        print cleanlinks(pydoc.html.page(pydoc.describe(object), pydoc.html.document(object)))



if __name__ == '__main__':
    import getopt
    class BadUsage: pass

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'k:p:w')

        if args:
            for arg in args:
                try:
                    if os.path.isdir(arg): writedocs(arg)
                    
                    if os.path.isfile(arg):
                        arg = pydoc.importfile(arg)
                    writedoc(arg, True)
                except pydoc.ErrorDuringImport, value:
                    print 'problem in %s - %s' % (
                        value.filename, str(value.exc))
        else:
                raise BadUsage

    except (getopt.error, BadUsage):
        print "need parameters\n"
