SET(doccustomplugin_PART_SRCS
doccustomplugin.cpp
)

KDE_AUTOMOC(${doccustomplugin_PART_SRCS})

ADD_LIBRARY(doccustomplugin SHARED ${doccustomplugin_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libdoccustomplugin.la
#libdoccustomplugin_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries)
#libdoccustomplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la
#noinst_HEADERS = doccustomplugin.h
#libdoccustomplugin_la_SOURCES = doccustomplugin.cpp
#kde_services_DATA = doccustomplugin.desktop

#######################
#previous CMakeLists.txt contents follow:

#SET(doccustomplugin_PART_SRCS
#doccustomplugin.cpp
#)
#
#KDE_AUTOMOC(${doccustomplugin_PART_SRCS})
#
#ADD_LIBRARY(doccustomplugin SHARED ${doccustomplugin_PART_SRCS})
#
