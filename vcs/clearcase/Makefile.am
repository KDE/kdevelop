# Here resides the clearcase part

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevclearcase.la
libkdevclearcase_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevclearcase_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la

libkdevclearcase_la_SOURCES = clearcasepart.cpp commentdlg.cpp \
	clearcasefileinfoprovider.cpp clearcasemanipulator.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevclearcase.desktop

SUBDIRS = integrator
