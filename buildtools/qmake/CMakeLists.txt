INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/base ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

SET(kdevtrollproject_PART_SRCS
caret.cpp
filebuffer.cpp
newwidgetdlg.cpp
filepropertydlg.cpp
projectconfigurationdlg.cpp
trollprojectpart.cpp
trollprojectwidget.cpp
choosesubprojectdlg.cpp
trolllistview.cpp
pathutil.cpp
)

SET( kdevtrollproject_UI
projectconfigurationdlgbase.ui
filepropertybase.ui
newwidgetdlgbase.ui
choosesubprojectdlgbase.ui
)

KDE_AUTOMOC(${kdevtrollproject_PART_SRCS})

KDE_ADD_UI_FILES(kdevtrollproject_PART_SRCS ${kdevtrollproject_UI} )

ADD_LIBRARY(kdevtrollproject SHARED ${kdevtrollproject_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevtrollproject ${KDEV_BUILDTOOLS_LIBS})

#original Makefile.am contents follow:

## Here resides the troll project part for tmake or qmake
#
#
#
#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_builddir)/buildtools/lib/widgets $(all_includes)
#
#
#kde_module_LTLIBRARIES = libkdevtrollproject.la
#libkdevtrollproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevtrollproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/util/libkdevutil.la $(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la \
#	$(top_builddir)/buildtools/lib/base/libkdevbuildbase.la
#
#libkdevtrollproject_la_SOURCES = caret.cpp filebuffer.cpp \
#	projectconfigurationdlgbase.ui filepropertybase.ui newwidgetdlgbase.ui newwidgetdlg.cpp \
#	filepropertydlg.cpp projectconfigurationdlg.cpp trollprojectpart.cpp trollprojectwidget.cpp \
#	choosesubprojectdlgbase.ui choosesubprojectdlg.cpp trolllistview.cpp pathutil.cpp
#
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevtrollproject.desktop kdevtmakeproject.desktop
#
#rcdir = $(kde_datadir)/kdevtrollproject
#rc_DATA = kdevtrollproject.rc
#noinst_HEADERS = trolllistview.h
