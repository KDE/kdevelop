# -*- coding: utf-8 -*-
import IPlugin, ICore, Kross, DUChain
from urlparse import urlparse

forms=Kross.module("forms")
lastcalled=None

urls=None
act=None

#def compileFile(tamare):
	#pass
	#project=ICore.projectController().findProjectForUrl(in)
	#includes=project.buildSystemManager().includeDirectories()
	#line="g++ -o/tmp/somefile "+urls+" -I"+includes.join(" -I")
	#run="/tmp/somefile"

def parsingFinished(job):
	pass
	#context=DUChain.chainForDocument(urls);
	#main=context.findDeclarations("main")
	#if len(main)>0:
		#compileFile(urls)

def triggered():
	pass
	#ICore.documentController().backgroundParser().addDocument(urls)

def contextMenuExtension(ctx, cme):
	global act, urls
	act=forms.createAction(0)
	triggered()
	
	a=act.connect("triggered()", triggered)
	print "nanana"+ctx.objectName+str(a)
	if ctx.hasType(1):
		act.text="I am an editor action"
	elif ctx.hasType(2):
		act.text="I am an editor action"
	else:
		act.text="I am another kind of action"+act.url()
	
	urls=act.url()
	cme.addAction(cme.ExtensionGroup, act)

print str(dir(ICore))+"\n\n"
print str(dir(ICore.documentController()))+"\n\n"
print str(dir(ICore.languageController()))+"\n\n"
print "------ "+ICore.languageController().className()+"\n\n"
print str(ICore.languageController().documentActivated())+"\n\n"
print str(ICore.languageController().objectName)+"\n\n"
print str(dir(ICore.languageController().language("C++")))+"\n\n"
print str(ICore.languageController().backgroundParser())
#ICore.languageController().backgroundParser().connect("parseJobFinished(KDevelop::ParseJob* ))", parsingFinished);
