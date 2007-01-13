#Here resides the Ruby support part.

INCLUDES = -I$(top_srcdir)/languages/lib/designer_integration \
	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions \
	-I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_builddir)/languages/lib/designer_integration $(all_includes)
SUBDIRS = app_templates file_templates doc debugger


kde_module_LTLIBRARIES = libkdevrubysupport.la
libkdevrubysupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevrubysupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/languages/lib/designer_integration/libdesignerintegration.la $(top_builddir)/lib/util/libkdevutil.la

libkdevrubysupport_la_SOURCES = rubysupport_part.cpp rubyconfigwidgetbase.ui \
		rubyconfigwidget.cpp qtdesignerrubyintegration.cpp rubyimplementationwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevrubysupport.desktop

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = rubytemplates

rcdir = $(kde_datadir)/kdevrubysupport
rc_DATA = kdevrubysupport.rc

picsdirdir = $(kde_datadir)/kdevrubysupport/pics
picsdir_DATA = ruby_config.png ruby_run.png
noinst_HEADERS = qtdesignerrubyintegration.h rubyimplementationwidget.h
