# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevperforce.la
libkdevperforce_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevperforce_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevperforce_la_SOURCES = perforcepart.cpp commitdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevperforce.desktop

rcdir = $(kde_datadir)/kdevperforce
