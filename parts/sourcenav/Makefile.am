INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevsourcenav.la
libkdevsourcenav_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevsourcenav_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lktexteditor

libkdevsourcenav_la_SOURCES = sourcenav_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_sourcenav.desktop

rcdir = $(kde_datadir)/kdevsourcenav
rc_DATA = kdevpart_sourcenav.rc
