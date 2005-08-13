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

SET( kdevmakeview_UI
filterdlg.ui
)

KDE_AUTOMOC(${kdevmakeview_PART_SRCS})

KDE_ADD_UI_FILES(kdevmakeview_PART_SRCS ${kdevmakeview_UI} )

ADD_LIBRARY(kdevmakeview MODULE ${kdevmakeview_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevmakeview ${KDEV_PART_LIBS} kdevwidgets)

KDE_CREATE_LIBTOOL_FILE(kdevmakeview)


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

SET( kdevappview_UI
filterdlg.ui
)

KDE_AUTOMOC(${kdevappview_PART_SRCS})

KDE_ADD_UI_FILES(kdevappview_PART_SRCS ${kdevappview_UI} )

ADD_LIBRARY(kdevappview MODULE ${kdevappview_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevappview ${KDEV_PART_LIBS} kdevwidgets)

LT_TARGET_LINK_CONV_LIBRARIES(kdevappview)

KDE_CREATE_LIBTOOL_FILE(kdevappview)

INSTALL_TARGETS(/lib/kde3 kdevappview kdevmakeview)

INSTALL_FILES(/share/servicetypes FILES kdevappoutputview.desktop kdevmakeview.desktop)
INSTALL_FILES(/share/apps/kdevappoutputview FILES kdevmakeview.rc)
INSTALL_FILES(/share/apps/kdevmakeview FILES kdevmakeview.rc)

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

