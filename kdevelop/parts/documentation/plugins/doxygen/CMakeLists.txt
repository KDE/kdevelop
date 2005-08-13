SET(docdoxygenplugin_LIB_SRCS
docdoxygenplugin.cpp
)

KDE_AUTOMOC(${docdoxygenplugin_LIB_SRCS})

ADD_LIBRARY(docdoxygenplugin SHARED ${docdoxygenplugin_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libdocdoxygenplugin.la
#
#kde_services_DATA =  docdoxygenplugin.desktop
#libdocdoxygenplugin_la_SOURCES = docdoxygenplugin.cpp
#noinst_HEADERS =  docdoxygenplugin.h
#
#
#libdocdoxygenplugin_la_LDFLAGS = $(all_libraries)
#libdocdoxygenplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

#######################
#previous CMakeLists.txt contents follow:

#SET(docdoxygenplugin_LIB_SRCS
#docdoxygenplugin.cpp
#)
#
#KDE_AUTOMOC(${docdoxygenplugin_LIB_SRCS})
#
#ADD_LIBRARY(docdoxygenplugin SHARED ${docdoxygenplugin_LIB_SRCS})
#
