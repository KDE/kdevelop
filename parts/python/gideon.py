import gideonc

class core:
    def gotoSourceFile(self, filename):
        gideonc.goto_source_file(filename)
        
    def gotoDocumentationFile(self, filename):
        gideonc.goto_documentation_file(filename)

    projectOpened = "core_project_opened"
    projectClosed = "core_project_closed"


def connect(signal, slot):
    gideonc.connect(signal, slot)

def disconnect(sender, signal, slot):
    gideonc.disconnect(signal, slot)

def addMenuItem(menu, submenu, slot):
    gideonc.add_menuitem(menu, submenu, slot)
