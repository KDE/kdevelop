#!/usr/bin/env kdevelop
# coding: utf-8

from urlparse import urlparse
from bzrlib import workingtree
import os

def name():
	return "Bazaar"

def pathHelper(fullpath):
	ret={}
	if not os.path.isdir(fullpath):
		lastdash=fullpath.rfind('/')
		ret["filename"]=fullpath[lastdash+1:]
		ret["path"]=fullpath[:lastdash]
	else:
		ret["path"]=fullpath
	return ret
	
def isVersionControlled(url):
	fullpath=urlparse(url).path
	path=pathHelper(fullpath)
	versioned=True
	
	if not os.path.exists(fullpath):
		return False
	
	try:
		wt = workingtree.WorkingTree.open(path["path"])
		if path.has_key("filename"):
			versioned=wt.path2id(path["filename"])
	except:
		versioned=False
	
	return versioned

def add(files, isRecursive):
	for url in files:
		fullpath=urlparse(url).path
		path=pathHelper(fullpath)
		wt = workingtree.WorkingTree.open(path["path"])
		wt.smart_add([ fullpath ], isRecursive)

def remove(files):
	for url in files:
		fullpath=urlparse(url).path
		path=pathHelper(fullpath)
		wt = workingtree.WorkingTree.open(path["path"])
		wt.remove([ fullpath ])

def revert(files, isRecursive):
	for url in files:
		fullpath=urlparse(url).path
		path=pathHelper(fullpath)
		wt = workingtree.WorkingTree.open(path["path"])
		ret = wt.revert(fullpath)
		if ret:
			return "Conflicts detected"


def test():
	assert name()=="Bazaar"
	assert isVersionControlled("file:///home/kde-devel/testbzr")
	assert isVersionControlled("file:///home/kde-devel/testbzr/")
	assert isVersionControlled("file:///home/kde-devel/testbzr/main.cpp")
	assert not isVersionControlled("file:///")
	assert not isVersionControlled("file:///dev/stdout")
	assert not isVersionControlled("file:///home/kde-devel/testbzr/aaa")
	add(["file:///home/kde-devel/testbzr/aaa"], False)
	assert isVersionControlled("file:///home/kde-devel/testbzr/aaa")
	remove(["file:///home/kde-devel/testbzr/aaa"])
	assert not isVersionControlled("file:///home/kde-devel/testbzr/aaa")
	
	print "done"

if __name__ == "__main__":
	test()
