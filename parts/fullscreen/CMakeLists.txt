SET(kdevfullscreen_PART_SRCS
fullscreen_part.cpp
)

KDE_AUTOMOC(${kdevfullscreen_PART_SRCS})

ADD_LIBRARY(kdevfullscreen SHARED ${kdevfullscreen_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevfullscreen ${KDEV_PART_LIBS})


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevfullscreen.la
#libkdevfullscreen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfullscreen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfullscreen_la_SOURCES = fullscreen_part.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfullscreen.desktop
#
#rcdir = $(kde_datadir)/kdevfullscreen
#rc_DATA = kdevpart_fullscreen.rc

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevfullscreen_PART_SRCS
#fullscreen_part.cpp
#)
#
#KDE_AUTOMOC(${kdevfullscreen_PART_SRCS})
#
#ADD_LIBRARY(kdevfullscreen SHARED ${kdevfullscreen_PART_SRCS})
#
