import IPlugin, Kross
from urlparse import urlparse

forms=Kross.module("forms")

def createView(parent):
	print "creating"+str(parent)
	#part=forms.loadPart(parent, "libkhtmlpart", "http://proli.net")
	#return part.view()
	widget=forms.createWidgetFromUIFile(parent, urlparse(IPlugin.pluginDirectory()).path+"/example.ui")
	#widget=forms.createWidgetFromUIFile(parent, "/home/kde-devel/koffice/krita/plugins/viewplugins/scripting/scripts/pilimport.ui")
	print "created"+str(widget)
	return widget

print "holaaaaa"
IPlugin.createToolViewFactory("createView", "org.kdevelop.GUIExample", 33)

