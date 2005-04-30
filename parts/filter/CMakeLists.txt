SET(kdevfilter_PART_SRCS
filterpart.cpp
shellfilterdlg.cpp
shellinsertdlg.cpp
kdevfilterIface.cpp
)

SET( kdevfilter_DCOP_SRCS
kdevfilterIface.h
)

KDE_AUTOMOC(${kdevfilter_PART_SRCS})

KDE_ADD_DCOP_SKELS(kdevfilter_PART_SRCS ${kdevfilter_DCOP_SRCS})

ADD_LIBRARY(kdevfilter SHARED ${kdevfilter_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevfilter ${KDEV_PART_LIBS} DCOP)


#original Makefile.am contents follow:

## Here resides the filter part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces 	   -I$(top_srcdir)/lib/util 	   $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevfilter.la
#libkdevfilter_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfilter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfilter_la_SOURCES = filterpart.cpp shellfilterdlg.cpp shellinsertdlg.cpp kdevfilterIface.cpp kdevfilterIface.skel
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfilter.desktop
#
#rcdir = $(kde_datadir)/kdevfilter
#rc_DATA = kdevfilter.rc

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevfilter_PART_SRCS
#filterpart.cpp
#shellfilterdlg.cpp
#shellinsertdlg.cpp
#kdevfilterIface.cpp
#)
#
#KDE_AUTOMOC(${kdevfilter_PART_SRCS})
#
#SET( kdevfilter_DCOP_SRCS
#kdevfilterIface.h
#)
#
#KDE_ADD_DCOP_SKELS(kdevfilter_PART_SRCS ${kdevfilter_DCOP_SRCS})
#
#ADD_LIBRARY(kdevfilter SHARED ${kdevfilter_PART_SRCS})
#
