#Here resides the SQL support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/external_interfaces	   -I$(top_srcdir)/lib/sourceinfo 	   -I$(top_srcdir)/lib/util 	   -I$(top_srcdir)/lib/compat 	   $(all_includes)
SUBDIRS = app_templates doc

kde_module_LTLIBRARIES = libkdevsqlsupport.la
libkdevsqlsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevsqlsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevsqlsupport_la_SOURCES = sqlconfigwidget.ui sqlsupport_part.cpp sqlactions.cpp sqloutputwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevsqlsupport.desktop

rcdir = $(kde_datadir)/kdevsqlsupport
rc_DATA = kdevsqlsupport.rc
