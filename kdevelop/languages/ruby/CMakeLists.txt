SUBDIRS( app_templates file_templates doc debugger )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/languages/lib/designer_integration ${CMAKE_BINARY_DIR}/languages/lib/designer_integration ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevrubysupport_PART_SRCS
rubysupport_part.cpp
rubyconfigwidget.cpp
qtdesignerrubyintegration.cpp
rubyimplementationwidget.cpp
)

SET( kdevrubysupport_UI
rubyconfigwidgetbase.ui
)

KDE_AUTOMOC(${kdevrubysupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevrubysupport_PART_SRCS ${kdevrubysupport_UI} )

ADD_LIBRARY(kdevrubysupport SHARED ${kdevrubysupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevrubysupport kdevelop designerintegration)

KDE_CREATE_LIBTOOL_FILE(kdevrubysupport)

INSTALL_TARGETS(/lib/kde3 kdevrubysupport)


#khtml kdevwidgets designerintegration)


#original Makefile.am contents follow:

##Here resides the Ruby support part.
#
#INCLUDES = -I$(top_srcdir)/languages/lib/designer_integration \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions \
#	-I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_builddir)/languages/lib/designer_integration $(all_includes)
#SUBDIRS = app_templates file_templates doc debugger
#
#
#kde_module_LTLIBRARIES = libkdevrubysupport.la
#libkdevrubysupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevrubysupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/languages/lib/designer_integration/libdesignerintegration.la
#
#libkdevrubysupport_la_SOURCES = rubysupport_part.cpp rubyconfigwidgetbase.ui \
#		rubyconfigwidget.cpp qtdesignerrubyintegration.cpp rubyimplementationwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevrubysupport.desktop
#
#rcdir = $(kde_datadir)/kdevrubysupport
#rc_DATA = kdevrubysupport.rc
#
#picsdirdir = $(kde_datadir)/kdevrubysupport/pics
#picsdir_DATA = ruby_config.png ruby_run.png
#noinst_HEADERS = qtdesignerrubyintegration.h rubyimplementationwidget.h
