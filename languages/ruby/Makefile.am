#Here resides the Ruby support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/external_interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util $(all_includes)
SUBDIRS = app_templates file_templates doc


kde_module_LTLIBRARIES = libkdevrubysupport.la
libkdevrubysupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevrubysupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevrubysupport_la_SOURCES = rubysupport_part.cpp rubyconfigwidgetbase.ui rubyconfigwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevrubysupport.desktop

rcdir = $(kde_datadir)/kdevrubysupport
rc_DATA = kdevrubysupport.rc

picsdirdir = $(kde_datadir)/kdevrubysupport/pics
picsdir_DATA = ruby_config.png ruby_run.png
