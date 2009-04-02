#!/usr/bin/env kdevelop
# coding: utf-8

from urlparse import urlparse
from bzrlib import workingtree
import os
import sys

import bzrlib
from bzrlib import (
    bugtracker,
    bundle,
    bzrdir,
    delta,
    config,
    errors,
    globbing,
    ignores,
    log,
    merge as _mod_merge,
    merge_directive,
    osutils,
    reconfigure,
    revision as _mod_revision,
    symbol_versioning,
    transport,
    tree as _mod_tree,
    ui,
    urlutils,
    )

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

def mergeStatus(a, b):
	a.added.append(b.added)
	a.removed.append(b.removed)
	a.renamed.append(b.renamed)
	a.modified.append(b.modified)
	a.unversioned.append(b.unversioned)
	a.unchanged.append(b.unchanged)
	
	return a
	
def commit(message, files, isRecursive):
	for url in files:
		fullpath=urlparse(url).path
		path=pathHelper(fullpath)
		wt = workingtree.WorkingTree.open(path["path"])
		wt.commit(message)

def update(files, revision, isRecursive):
	for url in files:
		fullpath=urlparse(url).path
		path=pathHelper(fullpath)
		wt = workingtree.WorkingTree.open(path["path"])
		wt.update()

def status(files, isRecursive):
	from bzrlib import delta
	totalChanges=None
	for url in files:
		fullpath=urlparse(url).path
		path=pathHelper(fullpath)
		
		wt = workingtree.WorkingTree.open(path["path"])
		changes = wt.changes_from(wt.basis_tree())
		if totalChanges is None:
			totalChanges=changes
		else:
			totalChanges=mergeStatus(totalChanges, changes)
	
	ch={}
	ch["Added"]=totalChanges.added
	ch["Modified"]=totalChanges.modified
	ch["Renamed"]=totalChanges.renamed
	ch["Modified"]=totalChanges.modified
	ch["Unchanged"]=totalChanges.unchanged
	ch["Unversioned"]=totalChanges.unversioned
	
	return ch

def init(url):
	fullpath=urlparse(url).path
	
	format = bzrdir.format_registry.make_bzrdir('default')
	to_transport = transport.get_transport(fullpath)
	try:
		to_transport.ensure_base()
	except errors.NoSuchFile:
		raise
	
	try:
		existing_bzrdir = bzrdir.BzrDir.open_from_transport(to_transport)
	except errors.NotBranchError:
		# really a NotBzrDir error...
		create_branch = bzrdir.BzrDir.create_branch_convenience
		branch = create_branch(to_transport.base, format=format,
								possible_transports=[to_transport])
	else:
		from bzrlib.transport.local import LocalTransport
		if existing_bzrdir.has_branch():
			if (isinstance(to_transport, LocalTransport)
				and not existing_bzrdir.has_workingtree()):
					raise errors.BranchExistsWithoutWorkingTree(location)
			raise errors.AlreadyBranchError(location)
		else:
			branch = existing_bzrdir.create_branch()
			existing_bzrdir.create_workingtree()

def test():
	folder=os.tmpnam()
	assert name()=="Bazaar"
	os.mkdir(folder)
	folderpath="file://"+folder
	try:
		assert not isVersionControlled("file:///")
		assert not isVersionControlled("file:///dev/stdout")
		assert not isVersionControlled(folderpath)
		assert not isVersionControlled(folderpath+"/")
		init(folderpath)
		assert isVersionControlled(folderpath)
		assert isVersionControlled(folderpath+"/")
		assert not isVersionControlled(folderpath+"/aaa")
		assert not isVersionControlled(folderpath+"/bbb")
		
		f=open(folder+"/aaa", "w")
		f.write("falalalalalala")
		f.close()
		
		assert not isVersionControlled(folderpath+"/aaa")
		
		add([folderpath+"/aaa"], False)
		assert isVersionControlled(folderpath+"/aaa")
		remove([folderpath+"/aaa"])
		assert not isVersionControlled(folderpath+"/aaa")
		
		f=open(folder+"/bbb", "w")
		f.write("falalalalalala")
		f.close()
		
		add([folderpath+"/bbb"], False)
		assert isVersionControlled(folderpath+"/bbb")
		
		r=status([folderpath+"/bbb"], False)
		assert r["Added"][0][0]=="bbb"
		assert len(r["Added"])==1
		
		commit("hello sportboy!", folderpath+"/bbb", False)
		r=status([folderpath+"/bbb"], False)
		for list in r:
			assert len(r)==0
		
		os.system('rm -rf  %s ' % folder)
		print "done"
	except AssertionError, inst:
		os.system('rm -rf  %s ' % folder)
		print "------------------------------------------- failed"
		raise

if __name__ == "__main__":
	test()
