# -*- coding: utf-8 -*-
import IPlugin, ICore, Kross, DUChain
from urlparse import urlparse

forms=Kross.module("forms")

def compileCurrent():
	print "triggered"
	comp=ICore.documentController().activeDocument().url()
	project=ICore.projectController().findProjectForUrl(comp)
	
	if project is not None:
		items=project.filesForUrl(comp)
		
		for item in items:
			builder=project.buildSystemManager().builder(item)
			buildjob=builder.build(item)
			
			print "registering: "+str(buildjob)+"\n\n"
			print "runco:"+ICore.runController().objectName
			ICore.runController().registerJob(buildjob)

def contextMenuExtension(ctx, cme):
	comp=ICore.documentController().activeDocument().url()
	project=ICore.projectController().findProjectForUrl(comp)
	if project is not None:
		cme.addAction(cme.ExtensionGroup, act)


act=forms.createAction(0)
act.text="Compile this file"
act.connect("triggered()", compileCurrent)
