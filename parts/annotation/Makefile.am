INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevannotation.la
libkdevannotation_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevannotation_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevannotation_la_SOURCES = annotationpart.cpp annotationwidget.cpp \
	annotationglobalconfigbase.ui annotationglobalconfig.cpp \
	annotationprojectconfigbase.ui annotationprojectconfig.cpp

METASOURCES = AUTO

appsharedir = $(kde_datadir)/kdevannotation
appshare_DATA = 

servicedir = $(kde_servicesdir)
service_DATA = kdevannotation.desktop

rcdir = $(kde_datadir)/kdevannotation
rc_DATA = kdevannotation.rc
