INCLUDES = -I$(top_srcdir)/lib/compat  -I$(top_srcdir)/lib/interfaces  -I$(top_srcdir)/lib/sourceinfo  $(all_includes)

noinst_LTLIBRARIES = libkdevutil.la libkdevutillgpl.la

libkdevutil_la_SOURCES = processwidget.cpp processlinemaker.cpp domutil.cpp \
	splitter.cpp execcommand.cpp ksavealldialog.cpp filetemplate.cpp urlutil.cpp \
	pathutil.cpp configwidgetproxy.cpp profiles.cpp

libkdevutillgpl_la_SOURCES = processwidget.cpp processlinemaker.cpp splitter.cpp execcommand.cpp ksavealldialog.cpp urlutil.cpp configwidgetproxy.cpp

METASOURCES = AUTO

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = domutil.h execcommand.h filetemplate.h ksavealldialog.h pathutil.h processlinemaker.h processwidget.h splitter.h urlutil.h configwidgetproxy.h
