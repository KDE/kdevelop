SUBDIRS( integrator )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevperforce_PART_SRCS
perforcepart.cpp
commitdlg.cpp
)

KDE_AUTOMOC(${kdevperforce_PART_SRCS})

ADD_LIBRARY(kdevperforce MODULE ${kdevperforce_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevperforce ${QT_AND_KDECORE_LIBS} kdeui kio kparts kdevelop)

KDE_CREATE_LIBTOOL_FILE(kdevperforce)

INSTALL_TARGETS(/lib/kde3 kdevperforce)
INSTALL_FILES(/share/services FILES kdevperforce.desktop )

#original Makefile.am contents follow:

## Here resides the cvs part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevperforce.la
#libkdevperforce_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevperforce_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevperforce_la_SOURCES = perforcepart.cpp commitdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevperforce.desktop
#
#rcdir = $(kde_datadir)/kdevperforce
#
#SUBDIRS = integrator