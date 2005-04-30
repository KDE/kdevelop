INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extras )

SET(kdevpgioptions_PART_SRCS
pgioptionsfactory.cpp
pgioptionsplugin.cpp
)

KDE_AUTOMOC(${kdevpgioptions_PART_SRCS})

ADD_LIBRARY(kdevpgioptions SHARED ${kdevpgioptions_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevpgioptions ${KDEV_LANG_LIBS} kdevextras)

#original Makefile.am contents follow:

## Here resides the pgi option dialog plugin.
## This defines the options for the Portland Group compiler
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extras \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevpgioptions.la
#libkdevpgioptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#libkdevpgioptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/interfaces/extras/libkdevextras.la
#
#libkdevpgioptions_la_SOURCES = pgioptionsfactory.cpp pgioptionsplugin.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevpghpfoptions.desktop kdevpgf77options.desktop
#
