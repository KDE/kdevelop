INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libdocdevhelpplugin.la
libdocdevhelpplugin_la_LDFLAGS = $(all_libraries)
kde_services_DATA =  docdevhelpplugin.desktop
noinst_HEADERS =  docdevhelpplugin.h
libdocdevhelpplugin_la_SOURCES =  docdevhelpplugin.cpp
libdocdevhelpplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
