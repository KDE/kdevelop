# Here resides the gcc option dialog plugin.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extras \
	-I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevgccoptions.la
libkdevgccoptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
libkdevgccoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/widgets/libwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KHTML)

libkdevgccoptions_la_SOURCES = gccoptionsplugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevgccoptions.desktop kdevgppoptions.desktop kdevg77options.desktop




