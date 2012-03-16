# -*- coding: utf-8 -*-
import IPlugin
import ICore
import Kross
from urlparse import urlparse

forms=Kross.module("forms")

def compileCurrent():
	comp=ICore.documentController().activeDocument().url()
	project=ICore.projectController().findProjectForUrl(comp)
	
	if project is not None:
		items=project.filesForUrl(comp)
		
		for item in items:
			builder=project.buildSystemManager().builder()
			buildjob=builder.build(item)
			ICore.runController().registerJob(buildjob)

def contextMenuExtension(ctx, cme):
	comp=ICore.documentController().activeDocument().url()
	project=ICore.projectController().findProjectForUrl(comp)
	if project is not None:
		cme.addAction(cme.ExtensionGroup, act)


act=forms.createAction(IPlugin)
act.text="Compile this file"
act.shortcut="Shift+F10"
act.connect("triggered()", compileCurrent)

toolBar=IPlugin.createToolBar("CompileOne", ICore.uiController().activeMainWindow())
toolBar.addAction(act)
