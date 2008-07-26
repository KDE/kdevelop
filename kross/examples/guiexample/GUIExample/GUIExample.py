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
	print "created"+str(widget)
	
	print "opened"+str(ICore.documentController().openDocuments())
	#for doc in ICore.documentController().openDocuments():
		#print doc.url()
		#widget.documentList.addItem(doc.url())
	
	#widget["close"].connect("clicked()", closeAll)
	
	return widget

def closeAll():
	print "clicked"
	ICore.documentController().closeAllDocuments()

def contextMenuExtension(ctx):
	print "xxxxxxxx: "+str(ctx)
	print "aaaaaaaa: "+str(ctx.type())
	return "potatoes"

print "holaaaaa"
print "opening1"+str(ICore)
print "opening2"+str(ICore.documentController())

print "opening3"+str(ICore.documentController().encoding())

IPlugin.createToolViewFactory("createView", "org.kdevelop.GUIExample", 33)

