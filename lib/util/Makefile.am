INCLUDES = -I$(top_srcdir)/lib/compat  -I$(top_srcdir)/lib/interfaces  -I$(top_srcdir)/lib/sourceinfo  $(all_includes)

noinst_LTLIBRARIES = libkdevutil.la

libkdevutil_la_SOURCES = domutil.cpp execcommand.cpp filetemplate.cpp \
	urlutil.cpp configwidgetproxy.cpp rurl.cpp

METASOURCES = AUTO

kdevelopincludedir = $(includedir)/kdevelop/util
kdevelopinclude_HEADERS = domutil.h execcommand.h filetemplate.h urlutil.h \
	configwidgetproxy.h rurl.h
