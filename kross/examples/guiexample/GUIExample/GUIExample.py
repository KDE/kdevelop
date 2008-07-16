import IPlugin
import Kross
forms=Kross.module("forms")

def createView(parent):
	part=forms.loadPart(parent, "libkhtmlpart", "http://proli.net")
	return part.view()

print "holaaaaa"
IPlugin.createToolViewFactory("createView", "org.kdevelop.GUIExample", 33)

