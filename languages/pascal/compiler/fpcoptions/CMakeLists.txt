INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/widgets )

SET(kdevfpcoptions_PART_SRCS
fpcoptionsplugin.cpp
optiontabs.cpp
)

KDE_AUTOMOC(${kdevfpcoptions_PART_SRCS})

ADD_LIBRARY(kdevfpcoptions SHARED ${kdevfpcoptions_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the gcc option dialog plugin.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extras \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevfpcoptions.la
#libkdevfpcoptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#libkdevfpcoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KHTML)
#
#libkdevfpcoptions_la_SOURCES = fpcoptionsplugin.cpp optiontabs.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfpcoptions.desktop
#
#
#
#
