import kdevelop
import pydcop

def foo1_action():
    print "foo1-action called"
    kdevelop.core().gotoSourceFile("/home/bernd/.bashrc", 0, kdevelop.core().Replace)

def foo2_action():
    print "foo2-action called"
    editor = kdevelop.core().activeEditorView()
    print editor
    editor.insertText("Heureka!", "false")
    editor.down()

def foo3_action():
    print "foo3-action called"
    classlist = kdevelop.classStore().getSortedClassNameList()
    print classlist

def bar1():
    print "bar1 called"

def bar2():
    print "bar2 called"

kdevelop.addMenuItem("tools", "Python action 1", foo1_action)
kdevelop.addMenuItem("tools", "Python action 2", foo2_action)
kdevelop.addMenuItem("tools", "Python action 3", foo3_action)

pydcop.connect(kdevelop.core(), "projectClosed()", bar1)
pydcop.connect(kdevelop.core(), "projectOpened()", bar2)
