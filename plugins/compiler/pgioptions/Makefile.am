# Here resides the pgi option dialog plugin.
# This defines the options for the Portland Group compiler

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevpgioptions.la
libkdevpgioptions_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
libkdevpgioptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevpgioptions_la_SOURCES = pgioptionsfactory.cpp pgioptionsplugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpghpfoptions.desktop kdevpgf77options.desktop

