SET(docchmplugin_LIB_SRCS
docchmplugin.cpp
)

KDE_AUTOMOC(${docchmplugin_LIB_SRCS})

ADD_LIBRARY(docchmplugin SHARED ${docchmplugin_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES =  libdocchmplugin.la
#libdocchmplugin_la_LDFLAGS = $(all_libraries)
#kde_services_DATA =  docchmplugin.desktop
#noinst_HEADERS =  docchmplugin.h
#libdocchmplugin_la_SOURCES =  docchmplugin.cpp
#libdocchmplugin_la_LIBADD =\
#	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

#######################
#previous CMakeLists.txt contents follow:

#SET(docchmplugin_LIB_SRCS
#docchmplugin.cpp
#)
#
#KDE_AUTOMOC(${docchmplugin_LIB_SRCS})
#
#ADD_LIBRARY(docchmplugin SHARED ${docchmplugin_LIB_SRCS})
#
