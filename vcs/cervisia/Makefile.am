# Here resides the Cervisia plugin

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/project -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcervisia.la
libkdevcervisia_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcervisia_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcervisia_la_SOURCES = cervisia_plugin.cpp

# Automagically builds from .skel and .ui files
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcervisia.desktop

rcdir = $(kde_datadir)/kdevcervisia

