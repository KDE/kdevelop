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

KDE_AUTOMOC(${kdevfileview_PART_SRCS})

SET(
vcscolorsconfigwidgetbase.ui
)

KDE_ADD_UI_FILES(kdevfileview_PART_SRCS ${kdevfileview_UI}

ADD_LIBRARY(kdevfileview SHARED ${kdevfileview_PART_SRCS})

SET(kdevfilegroups_PART_SRCS
filegroupswidget.cpp
addfilegroupdlg.cpp
filegroupsconfigwidget.cpp
filegroupspart.cpp
)

KDE_AUTOMOC(${kdevfilegroups_PART_SRCS})

SET(
filegroupsconfigwidgetbase.ui
)

KDE_ADD_UI_FILES(kdevfilegroups_PART_SRCS ${kdevfilegroups_UI}

ADD_LIBRARY(kdevfilegroups SHARED ${kdevfilegroups_PART_SRCS})



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

#######################
#previous CMakeLists.txt contents follow:

