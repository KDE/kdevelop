SET(docdjvuplugin_PART_SRCS
docdjvuplugin.cpp
)

KDE_AUTOMOC(${docdjvuplugin_PART_SRCS})

ADD_LIBRARY(docdjvuplugin SHARED ${docdjvuplugin_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libdocdjvuplugin.la
#libdocdjvuplugin_la_LDFLAGS = $(all_libraries)
#libdocdjvuplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#noinst_HEADERS = docdjvuplugin.h
#libdocdjvuplugin_la_SOURCES = docdjvuplugin.cpp
#kde_services_DATA = docdjvuplugin.desktop

#######################
#previous CMakeLists.txt contents follow:

#SET(docdjvuplugin_PART_SRCS
#docdjvuplugin.cpp
#)
#
#KDE_AUTOMOC(${docdjvuplugin_PART_SRCS})
#
#ADD_LIBRARY(docdjvuplugin SHARED ${docdjvuplugin_PART_SRCS})
#
