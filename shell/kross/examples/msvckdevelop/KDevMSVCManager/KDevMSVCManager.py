#!/usr/bin/env kdevelop
# coding: utf-8

from urlparse import urlparse

from VCProjManager import VCProjManager
from SolutionReader import SolutionReader

import xml.dom.expatbuilder
import xml.dom.minidom

import IBuildSystemManager

projects={}

def importProject(project):
	print "project: %s %s folder: %s" %(project, project.name(), project.folder())
	
	return "/home/apol/beca/objectManager/"

def parse(item):
	url = urlparse(item.url()).path
	print "parsing %s" % (url)
	solutionfile=url+"src.sln"

	s=SolutionReader(solutionfile)
	
	print "solution read, projects: "+str(s.projects().keys())
	
	prjs=s.projects().keys()
	for projectName in prjs:
		project=s.projects()[projectName]
		print "project %s %s" % (project, projectName)
		
		lastdash=project.rfind('/')
		baseurl=project[0:lastdash]
		
		print "a"
		IBuildSystemManager.addFolder(baseurl)
		print "b"
		IBuildSystemManager.addTarget(baseurl, projectName)
		print "c"
		
		vc=VCProjManager()
		vc.titolito(project)
		vc.importProject(project)
		projects[baseurl]=vc #not really correct but good enough
		
		print "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
		
		#print "preparing to add %s" % (vc.files())
		for filter in vc.files():
			for file in vc.files()[filter]:
				print "adding file %s" % (file)
				IBuildSystemManager.addFile(baseurl, projectName, file)

def defines(item):
	pass

def includeDirectories(item):
	print "includeeeeeeeees"
	#print "*----- %s %s\n\n" % (item, item.parent())
	path=urlparse(item.parent().parent().url()).path
	#print " at: %s file: %s" % (path, item.url())
	
	project=projects[path]
	
	print project
	print project.configurations()
	incs=project.includes(project.configurations()[0])
	print incs
	return incs
