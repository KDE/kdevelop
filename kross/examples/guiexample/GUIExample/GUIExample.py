# -*- coding: utf-8 -*-
import IPlugin, ICore, Kross
from urlparse import urlparse

forms=Kross.module("forms")

def createView(parent):
	print "opening4"+str("kk")
	print "opening5"+str(ICore.documentController().activeDocument())
	if(ICore.documentController().activeDocument() is not None):
		print "opening6"+str(ICore.documentController().activeDocument().url())
	print "creating"+str(parent)
	#part=forms.loadPart(parent, "libkhtmlpart", "http://proli.net")
	#return part.view()
	#widget=forms.createWidgetFromUIFile(parent, urlparse(IPlugin.pluginDirectory()).path+"/example.ui")
	widget=forms.createListView(parent)
	#print "created"+str(widget)
	
	#print "opened"+str(ICore.documentController().openDocuments())+str(len(ICore.documentController().openDocuments()))
	for doc in ICore.documentController().openDocuments():
		print "lilili"+str(doc)
		print "lololo"+doc.objectName
		print doc.url()
		widget.addItem(doc.url())
	
	#widget["close"].connect("clicked()", closeAll)
	
	return widget

def closeAll():
	print "clicked"
	ICore.documentController().closeAllDocuments()

def trigg():
	print "I have been triggered"

def contextMenuExtension(ctx, cme):
	print "ctxmenuuuu"
	print "xxxxxxxx: "+str(ctx)
	print "aaaaaaaa: "+str(ctx.type())
	act=forms.createAction(0)
	act.text="I am a snake"
	
	act.connect("triggered()", trigg)
	cme.addAction(cme.ExtensionGroup, act)
	
	return "potatoes"

print "holaaaaa"
print "opening1"+str(ICore)
print "opening2"+str(ICore.documentController())

print "opening3"+str(ICore.documentController().encoding())

IPlugin.createToolViewFactory("createView", "org.kdevelop.GUIExample", 33)

