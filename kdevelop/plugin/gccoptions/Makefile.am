# Here resides the gcc option dialog plugin.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general $(all_includes)

lib_LTLIBRARIES = libkdevgccoptions.la
libkdevgccoptions_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevgccoptions_la_LIBADD = $(top_builddir)/kdevelop/lib/general/libkdevelop.la

libkdevgccoptions_la_SOURCES = main.cpp gccoptions.cpp flagboxes.cpp

noinst_HEADERS = main.h gccoptions.h flagboxes.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevgccoptions.desktop kdevgppoptions.desktop
