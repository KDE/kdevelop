def foo_action():
    print "foo-action called"
    gideon.core().gotoSourceFile("/home/bernd/.bashrc")

def bar():
    print "bar called"

print "spam"

gideon.addMenuItem("edit","I'm a python action :-)", foo_action)

gideon.connect(gideon.core.projectClosed, bar)
