
MESSAGE(STATUS "cmake_root: ${CMAKE_ROOT}")

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/src/profileengine/lib ${CMAKE_SOURCE_DIR}/src/newui ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

SET(kdevshell_LIB_SRCS
../api.cpp
../core.cpp
../documentationpart.cpp
../editorproxy.cpp
../generalinfowidget.cpp
../languageselectwidget.cpp
../mainwindowshare.cpp
../newmainwindow.cpp
../partcontroller.cpp
../partselectwidget.cpp
../plugincontroller.cpp
../projectmanager.cpp
../projectsession.cpp
../splashscreen.cpp
../statusbar.cpp
../toplevel.cpp
../shellextension.cpp
../simplemainwindow.cpp
)

SET( kdevshell_UI
../generalinfowidgetbase.ui
../mimewarningdialog.ui
)

KDE_ADD_UI_FILES(kdevshell_LIB_SRCS ${kdevshell_UI} )

KDE_AUTOMOC(${kdevshell_LIB_SRCS})

ADD_LIBRARY(kdevshell SHARED ${kdevshell_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevshell kmdi kdevelop kdevextras kdevwidgets profileengine d kinterfacedesigner )


SET(kdevelop3_SRCS
../main.cpp
../kdevideextension.cpp
)

SET( kdevelop3_UI
../settingswidget.ui
)

KDE_ADD_UI_FILES(kdevelop3_SRCS ${kdevelop3_UI} )

ADD_EXECUTABLE(kdevelop3 ${kdevelop3_SRCS})

TARGET_LINK_LIBRARIES(kdevelop3 kdevshell)

SET_TARGET_PROPERTIES(kdevelop3 PROPERTIES INSTALL_AS kdevelop)

SET(kdevassistant_SRCS
../main_assistant.cpp
../kdevassistantextension.cpp
)

ADD_EXECUTABLE(kdevassistant ${kdevassistant_SRCS})

TARGET_LINK_LIBRARIES(kdevassistant kdevshell)

INSTALL_TARGETS(/bin kdevassistant kdevelop3)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
#	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
#	-I$(top_srcdir)/src/profileengine/lib -I$(top_srcdir)/src/newui $(KDEVMDI_INCLUDES) $(all_includes)
#
#KDE_OPTIONS=nofinal
#
#lib_LTLIBRARIES = libkdevshell.la
#libkdevshell_la_LDFLAGS = $(all_libraries)
#libkdevshell_la_LIBADD = \
#	$(top_builddir)/src/profileengine/lib/libprofileengine.la $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la \
#	$(top_builddir)/src/newui/libd.la $(LIB_KDEVMDI) $(LIB_KHTML) -lktexteditor
#libkdevshell_la_SOURCES = api.cpp core.cpp documentationpart.cpp \
#	editorproxy.cpp generalinfowidget.cpp languageselectwidget.cpp mainwindowshare.cpp \
#	newmainwindow.cpp partcontroller.cpp partselectwidget.cpp plugincontroller.cpp \
#	projectmanager.cpp projectsession.cpp splashscreen.cpp statusbar.cpp toplevel.cpp \
#	generalinfowidgetbase.ui mimewarningdialog.ui shellextension.cpp simplemainwindow.cpp
#
#bin_PROGRAMS = kdevelop kdevassistant
#
#kdevelop_SOURCES = main.cpp kdevideextension.cpp settingswidget.ui
#
#kdevelop_METASOURCES = AUTO
#kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kdevelop_LDADD = libkdevshell.la
#
#rcdir = $(kde_datadir)/kdevelop
#rc_DATA = kdevelopui.rc eventsrc
#
## default KDevelop configuration
#kdevelopdatadir = $(kde_confdir)
#kdevelopdata_DATA = kdeveloprc
#
#SUBDIRS = profiles profileengine newui
#kdevassistant_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kdevassistant_SOURCES = main_assistant.cpp kdevassistantextension.cpp
#kdevassistant_LDADD = libkdevshell.la
#
#rc_assistantdir = $(kde_datadir)/kdevassistant
#rc_assistant_DATA =  kdevassistantui.rc
#shellincludedir = $(includedir)/kdevelop/shell
#shellinclude_HEADERS = api.h core.h documentationpart.h editorproxy.h\
#	generalinfowidget.h languageselectwidget.h mainwindowshare.h\
#	newmainwindow.h partcontroller.h partselectwidget.h plugincontroller.h\
#	projectmanager.h projectsession.h settingswidget.ui.h shellextension.h\
#	splashscreen.h statusbar.h toplevel.h generalinfowidgetbase.h\
#	mimewarningdialog.h settingswidget.h simplemainwindow.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces kdevextensions
#DOXYGEN_PROJECTNAME = KDevelop Generic Shell
#DOXYGEN_DOCDIRPREFIX = kdevshell
#include ../Doxyfile.am
#profilesdatadir = $(kde_datadir)/kdevelop/profiles
#profilesdata_DATA = projectprofiles
