# Here resides the gcc option dialog plugin.

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevdccoptions.la
libkdevdccoptions_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
libkdevdccoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdccoptions_la_SOURCES = dccoptionsplugin.cpp 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdccoptions.desktop
