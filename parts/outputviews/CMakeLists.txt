INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/ ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/dcop/ ${CMAKE_SOURCE_DIR}/lib/widgets/ )

SET(kdevmakeview_PART_SRCS
makeviewpart.cpp
makewidget.cpp
directorystatusmessagefilter.cpp
outputfilter.cpp
compileerrorfilter.cpp
commandcontinuationfilter.cpp
makeitem.cpp
makeactionfilter.cpp
otherfilter.cpp
)

KDE_AUTOMOC(${kdevmakeview_PART_SRCS})

SET( kdevmakeview_UI
filterdlg.ui
)

KDE_ADD_UI_FILES(kdevmakeview_PART_SRCS ${kdevmakeview_UI} )

ADD_LIBRARY(kdevmakeview SHARED ${kdevmakeview_PART_SRCS})

SET(kdevappview_PART_SRCS
appoutputviewpart.cpp
appoutputwidget.cpp
directorystatusmessagefilter.cpp
outputfilter.cpp
compileerrorfilter.cpp
commandcontinuationfilter.cpp
makeitem.cpp
makeactionfilter.cpp
otherfilter.cpp
)

KDE_AUTOMOC(${kdevappview_PART_SRCS})

SET( kdevappview_UI
filterdlg.ui
)

KDE_ADD_UI_FILES(kdevappview_PART_SRCS ${kdevappview_UI} )

ADD_LIBRARY(kdevappview SHARED ${kdevappview_PART_SRCS})



#original Makefile.am contents follow:

## Here reside the output view and make view parts.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extensions/dcop -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevmakeview.la libkdevappview.la
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevappoutputview.desktop kdevmakeview.desktop
#
#rc1dir = $(kde_datadir)/kdevappoutputview
#rc1_DATA = kdevmakeview.rc
#
#rc2dir = $(kde_datadir)/kdevmakeview
#rc2_DATA = kdevmakeview.rc
#
#noinst_HEADERS = filterdlg.h
#
#libkdevmakeview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevmakeview_la_LIBADD = $(top_builddir)/lib/widgets/libkdevwidgets.la \
#	$(top_builddir)/lib/interfaces/extensions/dcop/libkdevdcopextensions.la $(top_builddir)/lib/libkdevelop.la \
#	$(LIB_KHTML)
#libkdevmakeview_la_SOURCES = makeviewpart.cpp makewidget.cpp directorystatusmessagefilter.cpp \
#	outputfilter.cpp compileerrorfilter.cpp commandcontinuationfilter.cpp makeitem.cpp \
#	makeactionfilter.cpp otherfilter.cpp filterdlg.ui
#
#libkdevappview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevappview_la_LIBADD = $(top_builddir)/lib/widgets/libkdevwidgets.la \
#	$(top_builddir)/lib/interfaces/extensions/dcop/libkdevdcopextensions.la $(top_builddir)/lib/libkdevelop.la \
#	$(LIB_KHTML)
#libkdevappview_la_SOURCES = appoutputviewpart.cpp appoutputwidget.cpp directorystatusmessagefilter.cpp \
#	outputfilter.cpp compileerrorfilter.cpp commandcontinuationfilter.cpp makeitem.cpp \
#	makeactionfilter.cpp otherfilter.cpp filterdlg.ui

