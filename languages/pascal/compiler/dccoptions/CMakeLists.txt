SET(kdevdccoptions_PART_SRCS
dccoptionsplugin.cpp
optiontabs.cpp
)

KDE_AUTOMOC(${kdevdccoptions_PART_SRCS})

ADD_LIBRARY(kdevdccoptions SHARED ${kdevdccoptions_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the gcc option dialog plugin.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extras \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdccoptions.la
#libkdevdccoptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#libkdevdccoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KHTML)
#
#libkdevdccoptions_la_SOURCES = dccoptionsplugin.cpp optiontabs.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevdccoptions.desktop

#######################
#previous CMakeLists.txt contents follow:

