SET(kdevfileview_PART_SRCS
fileviewpart.cpp
filetreewidget.cpp
partwidget.cpp
vcscolorsconfigwidget.cpp
stdfiletreewidgetimpl.cpp
filetreeviewwidgetimpl.cpp
vcsfiletreewidgetimpl.cpp
fileitemfactory.cpp
)

SET( kdevfileview_UI
vcscolorsconfigwidgetbase.ui
)

KDE_AUTOMOC(${kdevfileview_PART_SRCS})

KDE_ADD_UI_FILES(kdevfileview_PART_SRCS ${kdevfileview_UI} )

ADD_LIBRARY(kdevfileview MODULE ${kdevfileview_PART_SRCS})

KDE_CREATE_LIBTOOL_FILE(kdevfileview)

TARGET_LINK_LIBRARIES(kdevfileview ${KDEV_PART_LIBS})

SET(kdevfilegroups_PART_SRCS
filegroupswidget.cpp
addfilegroupdlg.cpp
filegroupsconfigwidget.cpp
filegroupspart.cpp
)

KDE_AUTOMOC(${kdevfilegroups_PART_SRCS})

SET( kdevfilegroups_UI
filegroupsconfigwidgetbase.ui
)

KDE_ADD_UI_FILES(kdevfilegroups_PART_SRCS ${kdevfilegroups_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/ )

ADD_LIBRARY(kdevfilegroups MODULE ${kdevfilegroups_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevfilegroups ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevfilegroups)

INSTALL_TARGETS(/lib/kde3 kdevfilegroups kdevfileview)

INSTALL_FILES(/share/servicetypes FILES kdevfileview.desktop kdevfilegroups.desktop)


#original Makefile.am contents follow:

## Here resides the file view part.
#
#INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevfileview.la libkdevfilegroups.la
#
#libkdevfileview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfileview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfilegroups_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfilegroups_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfileview_la_SOURCES = fileviewpart.cpp filetreewidget.cpp partwidget.cpp vcscolorsconfigwidgetbase.ui vcscolorsconfigwidget.cpp stdfiletreewidgetimpl.cpp filetreeviewwidgetimpl.cpp vcsfiletreewidgetimpl.cpp fileitemfactory.cpp
#
#libkdevfilegroups_la_SOURCES = filegroupswidget.cpp addfilegroupdlg.cpp filegroupsconfigwidget.cpp filegroupsconfigwidgetbase.ui filegroupspart.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfileview.desktop kdevfilegroups.desktop
#
## not used currently
##rcdir = $(kde_datadir)/kdevfileview
##rc_DATA = kdevfileview.rc
#noinst_HEADERS = vcscolorsconfigwidget.h stdfiletreewidgetimpl.h filetreeviewwidgetimpl.h vcsfiletreewidgetimpl.h fileitemfactory.h

