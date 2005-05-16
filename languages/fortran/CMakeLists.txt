SUBDIRS( file_templates app_templates compiler doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevfortransupport_PART_SRCS
fortransupportpart.cpp
fixedformparser.cpp
ftnchekconfigwidget.cpp
)

SET( kdevfortransupport_UI
ftnchekconfigwidgetbase.ui
)

KDE_AUTOMOC(${kdevfortransupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevfortransupport_PART_SRCS ${kdevfortransupport_UI} )

ADD_LIBRARY(kdevfortransupport SHARED ${kdevfortransupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevfortransupport ${KDEV_LANG_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevfortransupport)

INSTALL_TARGETS(/lib/kde3 kdevfortransupport)

#original Makefile.am contents follow:

## Here resides the Fortran support part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#SUBDIRS = file_templates app_templates compiler doc
#
#kde_module_LTLIBRARIES = libkdevfortransupport.la
#libkdevfortransupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfortransupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfortransupport_la_SOURCES = fortransupportpart.cpp fixedformparser.cpp ftnchekconfigwidget.cpp ftnchekconfigwidgetbase.ui
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfortransupport.desktop
#
#rcdir = $(kde_datadir)/kdevfortransupport
#rc_DATA = kdevfortransupport.rc
