SET(docpdfplugin_PART_SRCS
docpdfplugin.cpp
)

KDE_AUTOMOC(${docpdfplugin_PART_SRCS})

ADD_LIBRARY(docpdfplugin SHARED ${docpdfplugin_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libdocpdfplugin.la
#libdocpdfplugin_la_LDFLAGS = $(all_libraries)
#libdocpdfplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#noinst_HEADERS = docpdfplugin.h
#libdocpdfplugin_la_SOURCES = docpdfplugin.cpp
#kde_services_DATA = docpdfplugin.desktop

#######################
#previous CMakeLists.txt contents follow:

