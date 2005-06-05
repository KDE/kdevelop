SUBDIRS( app_templates doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/compat ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevsqlsupport_PART_SRCS
sqlsupport_part.cpp
sqlactions.cpp
sqloutputwidget.cpp
)

KDE_AUTOMOC(${kdevsqlsupport_PART_SRCS})

SET( kdevsqlsupport_UI
sqlconfigwidget.ui
)

KDE_ADD_UI_FILES(kdevsqlsupport_PART_SRCS ${kdevsqlsupport_UI} )

ADD_LIBRARY(kdevsqlsupport SHARED ${kdevsqlsupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevsqlsupport kdevelop)

KDE_CREATE_LIBTOOL_FILE(kdevsqlsupport)

INSTALL_TARGETS(/lib/kde3 kdevsqlsupport)

#original Makefile.am contents follow:

##Here resides the SQL support part.
#
#INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#SUBDIRS = app_templates doc
#
#kde_module_LTLIBRARIES = libkdevsqlsupport.la
#libkdevsqlsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevsqlsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevsqlsupport_la_SOURCES = sqlconfigwidget.ui sqlsupport_part.cpp sqlactions.cpp sqloutputwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevsqlsupport.desktop
#
#rcdir = $(kde_datadir)/kdevsqlsupport
#rc_DATA = kdevsqlsupport.rc
