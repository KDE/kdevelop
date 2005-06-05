SUBDIRS( app_templates file_templates doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevperlsupport_PART_SRCS
perlparser.cpp
perlsupportpart.cpp
perlconfigwidget.cpp
)

SET( kdevperlsupport_UI
perlconfigwidgetbase.ui
)

KDE_AUTOMOC(${kdevperlsupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevperlsupport_PART_SRCS ${kdevperlsupport_UI} )

ADD_LIBRARY(kdevperlsupport SHARED ${kdevperlsupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevperlsupport kdevelop )

KDE_CREATE_LIBTOOL_FILE(kdevperlsupport)

INSTALL_TARGETS(/lib/kde3 kdevperlsupport)


#original Makefile.am contents follow:

## Here resides the Perl support part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#SUBDIRS = app_templates file_templates doc
#
#
#kde_module_LTLIBRARIES = libkdevperlsupport.la kio_perldoc.la
#libkdevperlsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevperlsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevperlsupport_la_SOURCES = perlparser.cpp perlsupportpart.cpp perlconfigwidget.cpp perlconfigwidgetbase.ui
#
#kio_perldoc_la_SOURCES = perldoc.cpp
#kio_perldoc_la_LIBADD = $(LIB_KIO)
#kio_perldoc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevperlsupport.desktop perldoc.protocol
#
#rcdir = $(kde_datadir)/kdevperlsupport
#rc_DATA = kdevperlsupport.rc
