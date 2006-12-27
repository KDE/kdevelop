# Here resides the valgrind part

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions $(all_includes)

kde_module_LTLIBRARIES = libkdevdiff.la
libkdevdiff_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdiff_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdiff_la_SOURCES = diffpart.cpp diffwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdiff.desktop

rcdir = $(kde_datadir)/kdevdiff
rc_DATA = kdevdiff.rc
