INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets/propeditor )

SET(kdevbuildtoolswidgets_LIB_SRCS
runoptionswidget.cpp
makeoptionswidget.cpp
addenvvardlg.cpp
subclassesdlg.cpp
environmentvariableswidget.cpp
addfilesdialog.cpp
envvartools.cpp
removesubprojectdialog.cpp
)

SET( kdevbuildtoolswidgets_UI
runoptionswidgetbase.ui
makeoptionswidgetbase.ui
subclassesdlgbase.ui
environmentvariableswidgetbase.ui
removesubprojectdlgbase.ui
)

KDE_AUTOMOC(${kdevbuildtoolswidgets_LIB_SRCS})

KDE_ADD_UI_FILES(kdevbuildtoolswidgets_LIB_SRCS ${kdevbuildtoolswidgets_UI} )

ADD_LIBRARY(kdevbuildtoolswidgets SHARED ${kdevbuildtoolswidgets_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevbuildtoolswidgets ${QT_AND_KDECORE_LIBS} kdeui kdeui kio kdevelop)

INSTALL_TARGETS(/lib kdevbuildtoolswidgets)

#original Makefile.am contents follow:

## This directory collects some classes related to
## project management for the sole purpose that they
## can be shared between parts.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets/propeditor $(all_includes)
#
#lib_LTLIBRARIES = libkdevbuildtoolswidgets.la
#
#libkdevbuildtoolswidgets_la_SOURCES = runoptionswidget.cpp runoptionswidgetbase.ui \
#	makeoptionswidget.cpp makeoptionswidgetbase.ui addenvvardlg.cpp subclassesdlgbase.ui \
#	subclassesdlg.cpp environmentvariableswidgetbase.ui environmentvariableswidget.cpp \
#	addfilesdialog.cpp envvartools.cpp removesubprojectdialog.cpp removesubprojectdlgbase.ui
#
#METASOURCES = AUTO
#
#kdevelopbuildtoolsincludedir = $(includedir)/kdevelop/buildtools/widgets
#kdevelopbuildtoolsinclude_HEADERS = addenvvardlg.h addfilesdialog.h \
#	environmentvariableswidget.h environmentvariableswidgetbase.h envvartools.h makeoptionswidget.h \
#	makeoptionswidgetbase.h removesubprojectdialog.h removesubprojectdlgbase.h runoptionswidget.h \
#	runoptionswidgetbase.h subclassesdlg.h subclassesdlgbase.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces kdevutil
#DOXYGEN_PROJECTNAME = KDevelop Buildtool Widgets Library
#DOXYGEN_DOCDIRPREFIX = kdevbt
#include ../../../Doxyfile.am
