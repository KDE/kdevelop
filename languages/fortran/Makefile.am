# Here resides the Fortran support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
SUBDIRS = file_templates app_templates compiler doc

kde_module_LTLIBRARIES = libkdevfortransupport.la
libkdevfortransupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfortransupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfortransupport_la_SOURCES = fortransupportpart.cpp fixedformparser.cpp ftnchekconfigwidget.cpp ftnchekconfigwidgetbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfortransupport.desktop

rcdir = $(kde_datadir)/kdevfortransupport
rc_DATA = kdevfortransupport.rc
