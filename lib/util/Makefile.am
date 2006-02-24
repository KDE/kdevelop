INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

noinst_LTLIBRARIES = libkdevutil.la

libkdevutil_la_SOURCES = execcommand.cpp kfiltermodel.cpp kdevtreeview.cpp \
	configwidgetproxy.cpp settings.cpp

METASOURCES = AUTO

kdevelopincludedir = $(includedir)/kdevelop/util
kdevelopinclude_HEADERS = execcommand.h configwidgetproxy.h kfiltermodel.h \
	kdevtreeview.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevextensions kdevinterfaces
DOXYGEN_PROJECTNAME = KDevelop Utility Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am

noinst_HEADERS = settings.h
libkdevutil_la_LIBADD = $(LIB_KDECORE) 
libkdevutil_la_LDFLAGS = $(all_libraries) -no-undefined
