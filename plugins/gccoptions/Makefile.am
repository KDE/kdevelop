# Here resides the gcc option dialog plugin.

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevgccoptions.la
libkdevgccoptions_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
libkdevgccoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevgccoptions_la_SOURCES = gccoptionsplugin.cpp \
			       flagboxes.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevgccoptions.desktop kdevgppoptions.desktop kdevg77options.desktop
