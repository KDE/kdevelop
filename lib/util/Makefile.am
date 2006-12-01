INCLUDES = -I$(top_srcdir)/lib/compat  -I$(top_srcdir)/lib/interfaces  -I$(top_srcdir)/lib/sourceinfo  $(all_includes)

noinst_LTLIBRARIES = libkdevutil.la

libkdevutil_la_SOURCES = configwidgetproxy.cpp domutil.cpp execcommand.cpp \
	filetemplate.cpp kdevjobtimer.cpp kdevshellwidget.cpp kscriptactionmanager.cpp rurl.cpp \
	settings.cpp urlutil.cpp blockingkprocess.cpp

METASOURCES = AUTO

kdevelopincludedir = $(includedir)/kdevelop/util
kdevelopinclude_HEADERS = domutil.h execcommand.h filetemplate.h urlutil.h \
	configwidgetproxy.h rurl.h kscriptactionmanager.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevextensions kdevinterfaces
DOXYGEN_PROJECTNAME = KDevelop Utility Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am

noinst_HEADERS = kdevjobtimer.h kdevshellwidget.h settings.h blockingkprocess.h
