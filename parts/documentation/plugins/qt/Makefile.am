INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  libdocqtplugin.la
libdocqtplugin_la_LDFLAGS = $(all_libraries)
noinst_HEADERS = docqtplugin.h
libdocqtplugin_la_SOURCES = docqtplugin.cpp
kde_services_DATA =  docqtplugin.desktop
libdocqtplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
