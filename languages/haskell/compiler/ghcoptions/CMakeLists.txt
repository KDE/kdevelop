INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/widgets )

SET(kdevghcoptions_PART_SRCS
ghcoptionsplugin.cpp
)

KDE_AUTOMOC(${kdevghcoptions_PART_SRCS})

ADD_LIBRARY(kdevghcoptions SHARED ${kdevghcoptions_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevghcoptions kdevelop kdevextras)

KDE_CREATE_LIBTOOL_FILE(kdevghcoptions)

INSTALL_TARGETS(/lib/kde3 kdevghcoptions)

#original Makefile.am contents follow:

## Here resides the gcc option dialog plugin.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extras \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevghcoptions.la
#libkdevghcoptions_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#libkdevghcoptions_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KHTML)
#
#libkdevghcoptions_la_SOURCES = ghcoptionsplugin.cpp
## optiontabs.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevghcoptions.desktop
#
#
#
#
