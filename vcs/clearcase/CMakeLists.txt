SUBDIRS( integrato )

SET(kdevclearcase_PART_SRCS
clearcasepart.cpp
commentdlg.cpp
)

KDE_AUTOMOC(${kdevclearcase_PART_SRCS})

ADD_LIBRARY(kdevclearcase SHARED ${kdevclearcase_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the clearcase part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevclearcase.la
#libkdevclearcase_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevclearcase_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevclearcase_la_SOURCES = clearcasepart.cpp commentdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevclearcase.desktop
#
#SUBDIRS = integrator
#######################
#previous CMakeLists.txt contents follow:

