# Here resides the history part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevhistory.la
libkdevhistory_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevhistory_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevhistory_la_SOURCES = historypart.cpp 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevhistory.desktop

rcdir = $(kde_datadir)/kdevhistory
rc_DATA = kdevhistory.rc
