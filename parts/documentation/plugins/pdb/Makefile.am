INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libdocpdbplugin.la
libdocpdbplugin_la_LDFLAGS = $(all_libraries)
libdocpdbplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
noinst_HEADERS = docpdbplugin.h
libdocpdbplugin_la_SOURCES = docpdbplugin.cpp
kde_services_DATA = docpdbplugin.desktop
