#SUBDIRS = . test

INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkeditorinterface.la

libkeditorinterface_la_SOURCES = interface.cpp editor.cpp clipboard_iface.cpp undo_iface.cpp cursor_iface.cpp gotoline.ui edit_iface.cpp

kdevelopincludedir = $(includedir)/kdevelop/keditor
kdevelopinclude_HEADERS = interface.h editor.h cursor_iface.h undo_iface.h clipboard_iface.h cursor_iface.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = keditorinterface.desktop
