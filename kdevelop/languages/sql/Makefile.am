#Here resides the SQL support part.

INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
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
