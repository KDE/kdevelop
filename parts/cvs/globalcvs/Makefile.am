# Here resides the global cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevglobalcvs.la
libkdevglobalcvs_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevglobalcvs_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la

libkdevglobalcvs_la_SOURCES = globalcvs.cpp globalcvs.h cvsform.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevglobalcvs.desktop

rcdir = $(kde_datadir)/kdevglobalcvs
rc_DATA = kdevglobalcvs.rc
