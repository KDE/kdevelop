INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevpartselector.la
libkdevpartselector_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevpartselector_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevpartselector_la_SOURCES = partselector_factory.cpp partselector_part.cpp partselector.ui partselector_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_partselector.desktop

