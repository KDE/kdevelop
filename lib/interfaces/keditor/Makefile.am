INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkeditorinterface.la

libkeditorinterface_la_SOURCES = interface.cpp editor.cpp clipboard_iface.cpp undo_iface.cpp cursor_iface.cpp gotoline.ui edit_iface.cpp status_iface.cpp debug_iface.cpp KEditorIface.skel KEditorIface.cpp KEditorEditIface.skel KEditorEditIface.cpp KEditorClipboardIface.skel KEditorClipboardIface.cpp KEditorCursorIface.skel KEditorCursorIface.cpp KEditorDebugIface.skel KEditorDebugIface.cpp KEditorStatusIface.skel KEditorStatusIface.cpp KEditorUndoIface.skel KEditorUndoIface.cpp

kdevelopincludedir = $(includedir)/kdevelop/keditor
kdevelopinclude_HEADERS = interface.h editor.h cursor_iface.h undo_iface.h clipboard_iface.h cursor_iface.h debug_iface.h edit_iface.h status_iface.h 

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = keditorinterface.desktop
