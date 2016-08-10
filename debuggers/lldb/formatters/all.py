import os


def __lldb_init_module(debugger, unused):
    parent = os.path.dirname(os.path.abspath(__file__))
    debugger.HandleCommand('command script import ' + os.path.join(parent, 'qt.py'))
    debugger.HandleCommand('command script import ' + os.path.join(parent, 'kde.py'))
