#!/usr/bin/env kdevelop
# coding: utf-8

from urlparse import urlparse
from bzrlib import workingtree
import os

def pluginName():
	return "Bazaar"

def isVersionControlled(url):
	fullpath=urlparse(url).path
	path=fullpath
	versioned=True
	filename=None
	if not os.path.exists(fullpath):
		return False
	
	if not os.path.isdir(fullpath):
		lastdash=fullpath.rfind('/')
		filename=fullpath[lastdash+1:]
		path=fullpath[:lastdash]
    
	try:
		wt = workingtree.WorkingTree.open(path)
		if filename is not None:
			versioned=wt.has_filename(filename)
	except:
		versioned=False
		
	print fullpath+" versioned? "+str(versioned)
	return versioned

def test():
	assert name()=="Bazaar"
	assert isVersionControlled("file:///home/kde-devel/testbzr")
	assert isVersionControlled("file:///home/kde-devel/testbzr/")
	assert isVersionControlled("file:///home/kde-devel/testbzr/main.cpp")
	assert not isVersionControlled("file:///")
	assert not isVersionControlled("file:///dev/stdout")
	#assert not isVersionControlled("file:///home/kde-devel/testbzr/aaa")
	print "done"
	
if __name__ == "__main__":
    test()