import kdevelopc
from pydcop import *

_appid = kdevelopc.appId()
_core = DCOPObject(_appid, "KDevCore")
_makeFrontend = DCOPObject(_appid, "KDevMakeFrontend")
_classStore = DCOPObject(_appid, "ClassStore")

# Sugar :-)
_core.Replace = 0
_core.SplitHorizontal = 1
_core.SplitVertical = 2

def core():
    return _core

def makeFrontend():
    return _makeFrontend

def classStore():
    return _classStore

def addMenuItem(menu, submenu, slot):
    kdevelopc.addMenuItem(menu, submenu, slot)
