# Here resides the gcc option dialog plugin.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extras \
	-I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevghcoptions.la
libkdevghcoptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
libkdevghcoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/widgets/libwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KHTML)

libkdevghcoptions_la_SOURCES = ghcoptionsplugin.cpp
# optiontabs.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevghcoptions.desktop




