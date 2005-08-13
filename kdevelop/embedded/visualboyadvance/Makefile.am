INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevvisualboyadvance.la
libkdevvisualboyadvance_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevvisualboyadvance_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevvisualboyadvance_la_SOURCES = visualboyadvance_part.cpp vbaconfigwidgetbase.ui vbaconfigwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevvisualboyadvance.desktop

rcdir = $(kde_datadir)/kdevvisualboyadvance
rc_DATA = kdevpart_visualboyadvance.rc

