# Here resides the file view part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/sourceinfo \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes)

kde_module_LTLIBRARIES = libkdevfileview.la
libkdevfileview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfileview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfileview_la_SOURCES = fileviewpart.cpp \
			     fileviewwidget.cpp \
			     addfilegroupdlg.cpp \
			     fileviewconfigwidget.cpp \
			     fileviewconfigwidgetbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfileview.desktop

# not used currently
#rcdir = $(kde_datadir)/kdevfileview
#rc_DATA = kdevfileview.rc
