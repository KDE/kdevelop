# Here resides the konsole view part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevkonsoleview.la
libkdevkonsoleview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevkonsoleview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevkonsoleview_la_SOURCES = konsoleviewpart.cpp konsoleviewwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevkonsoleview.desktop

