# Here resides the gcc option dialog plugin.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevfpcoptions.la
libkdevfpcoptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
libkdevfpcoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libwidgets.la

libkdevfpcoptions_la_SOURCES = fpcoptionsplugin.cpp optiontabs.h optiontabs.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfpcoptions.desktop
AM_CFLAGS = 
AM_CXXFLAGS = 
AM_FFLAGS = 
SUBDIRS = 
