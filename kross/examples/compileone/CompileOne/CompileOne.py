import IPlugin, ICore, Kross
from urlparse import urlparse

forms=Kross.module("forms")

def contextMenuExtension(ctx, cme):
	def triggered():
		print "doing interesting things"
	
	act=forms.createAction(0)
	triggered()
	
	a=act.connect("triggered()", triggered)
	print "nanana"+ctx.objectName+str(a)
	if ctx.hasType(1):
		act.text="I am an editor action"
	elif ctx.hasType(2):
		act.text="I am an editor action"
	else:
		act.text="I am another kind of action"
	
	cme.addAction(cme.ExtensionGroup, act)

