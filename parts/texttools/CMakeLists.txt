SET(kdevtexttools_PART_SRCS
texttoolspart.cpp
texttoolswidget.cpp
)

KDE_AUTOMOC(${kdevtexttools_PART_SRCS})

ADD_LIBRARY(kdevtexttools SHARED ${kdevtexttools_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevtexttools ${KDEV_PART_LIBS})

#original Makefile.am contents follow:

## Here resides the text tools part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces 	   -I$(top_srcdir)/lib/sourceinfo 	   -I$(top_srcdir)/lib/util 	   $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevtexttools.la
#libkdevtexttools_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevtexttools_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevtexttools_la_SOURCES = texttoolspart.cpp texttoolswidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevtexttools.desktop
#
## not used currently
##rcdir = $(kde_datadir)/kdevtexttools
##rc_DATA = kdevtexttools.rc

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevtexttools_PART_SRCS
#texttoolspart.cpp
#texttoolswidget.cpp
#)
#
#KDE_AUTOMOC(${kdevtexttools_PART_SRCS})
#
#ADD_LIBRARY(kdevtexttools SHARED ${kdevtexttools_PART_SRCS})
#
