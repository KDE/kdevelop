# KDevelop Xul support. You dig?

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevxulsupport.la
libkdevxulsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevxulsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevxulsupport_la_SOURCES = xulsupport_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevxulsupport.desktop

rcdir = $(kde_datadir)/kdevxulsupport
rc_DATA = kdevxulsupport.rc
