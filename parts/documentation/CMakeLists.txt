SUBDIRS( interfaces plugins tools data protocols )

SET(kdevdocumentation_PART_SRCS
documentation_part.cpp
documentation_widget.cpp
contentsview.cpp
indexview.cpp
docglobalconfigwidget.cpp
docconfiglistview.cpp
editcatalogdlg.cpp
docutils.cpp
searchview.cpp
bookmarkview.cpp
find_documentation.cpp
find_documentation_options.cpp
selecttopic.cpp
docprojectconfigwidget.cpp
KDevDocumentationIface.cpp
addcatalogdlg.cpp
)

KDE_AUTOMOC(${kdevdocumentation_PART_SRCS})

SET( kdevdocumentation_UI
docglobalconfigwidgetbase.ui
editcatalogdlgbase.ui
editbookmarkdlg.ui
find_documentationbase.ui
find_documentation_optionsbase.ui
selecttopicbase.ui
docprojectconfigwidgetbase.ui
addcatalogdlgbase.ui
)

KDE_ADD_UI_FILES(kdevdocumentation_PART_SRCS ${kdevdocumentation_UI} )

SET( kdevdocumentation_DCOP_SRCS
KDevDocumentationIface.h
)

KDE_ADD_DCOP_SKELS(kdevdocumentation_PART_SRCS ${kdevdocumentation_DCOP_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/parts/documentation/interfaces/)

ADD_LIBRARY(kdevdocumentation SHARED ${kdevdocumentation_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdocumentation.la
#libkdevdocumentation_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevdocumentation_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la $(LIB_KHTML)
#
#libkdevdocumentation_la_SOURCES = documentation_part.cpp \
#			documentation_widget.cpp contentsview.cpp indexview.cpp docglobalconfigwidgetbase.ui \
#			docglobalconfigwidget.cpp docconfiglistview.cpp editcatalogdlgbase.ui editcatalogdlg.cpp \
#			docutils.cpp searchview.cpp bookmarkview.cpp editbookmarkdlg.ui \
#			find_documentationbase.ui find_documentation.cpp find_documentation_optionsbase.ui \
#			find_documentation_options.cpp selecttopicbase.ui selecttopic.cpp docprojectconfigwidgetbase.ui \
#			docprojectconfigwidget.cpp KDevDocumentationIface.cpp KDevDocumentationIface.skel \
#	addcatalogdlgbase.ui addcatalogdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevdocumentation.desktop
#
#rcdir = $(kde_datadir)/kdevdocumentation
#rc_DATA = kdevpart_documentation.rc
#
#noinst_HEADERS = contentsview.h indexview.h docglobalconfigwidget.h \
#					docconfiglistview.h editcatalogdlg.h docutils.h searchview.h bookmarkview.h selecttopic.h \
#			docprojectconfigwidget.h KDevDocumentationIface.h addcatalogdlg.h
#SUBDIRS = interfaces plugins tools data protocols
#
#DOXYGEN_EMPTY = YES
#include ../../Doxyfile.am
