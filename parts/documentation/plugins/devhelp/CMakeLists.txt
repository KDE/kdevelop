SET(docdevhelpplugin_LIB_SRCS
docdevhelpplugin.cpp
)

KDE_AUTOMOC(${docdevhelpplugin_LIB_SRCS})

ADD_LIBRARY(docdevhelpplugin SHARED ${docdevhelpplugin_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES =  libdocdevhelpplugin.la
#libdocdevhelpplugin_la_LDFLAGS = $(all_libraries)
#kde_services_DATA =  docdevhelpplugin.desktop
#noinst_HEADERS =  docdevhelpplugin.h
#libdocdevhelpplugin_la_SOURCES =  docdevhelpplugin.cpp
#libdocdevhelpplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

#######################
#previous CMakeLists.txt contents follow:

