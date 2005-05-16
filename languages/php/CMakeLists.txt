SUBDIRS( data app_templates file_templates doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets )

SET(kdevphpsupport_PART_SRCS
phpsupportpart.cpp
phpconfigwidget.cpp
phphtmlview.cpp
phpconfigparserwidget.cpp
phperrorview.cpp
phpconfigdata.cpp
phpcodecompletion.cpp
phpparser.cpp
phpnewclassdlg.cpp
)

SET( kdevphpsupport_UI
phpconfigwidgetbase.ui
phpconfigparserwidgetbase.ui
phpinfodlg.ui
phpnewclassdlgbase.ui
)

KDE_AUTOMOC(${kdevphpsupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevphpsupport_PART_SRCS ${kdevphpsupport_UI} )

ADD_LIBRARY(kdevphpsupport SHARED ${kdevphpsupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevphpsupport ${KDEV_LANG_LIBS} khtml kdevwidgets)

KDE_CREATE_LIBTOOL_FILE(kdevphpsupport)

INSTALL_TARGETS(/lib/kde3 kdevphpsupport)

#original Makefile.am contents follow:

## Here resides the PHP support part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#SUBDIRS = data app_templates file_templates doc
#
#
#kde_module_LTLIBRARIES = libkdevphpsupport.la
#libkdevphpsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevphpsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML)
#
#libkdevphpsupport_la_SOURCES = phpsupportpart.cpp phpconfigwidgetbase.ui phpconfigparserwidgetbase.ui phpconfigwidget.cpp phpinfodlg.ui phphtmlview.cpp phpconfigparserwidget.cpp phperrorview.cpp phpconfigdata.cpp phpcodecompletion.cpp phpparser.cpp phpnewclassdlgbase.ui phpnewclassdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevphpsupport.desktop
#
#rcdir = $(kde_datadir)/kdevphpsupport
#rc_DATA = kdevphpsupport.rc
#
#templatedir = ${kde_datadir}/kdevabbrev/templates
#template_DATA = phptemplates
