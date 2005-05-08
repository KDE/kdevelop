SUBDIRS( propeditor )

SET(kdevwidgets_LIB_SRCS
flagboxes.cpp
qcomboview.cpp
klistviewaction.cpp
kcomboview.cpp
resizablecombo.cpp
kdevhtmlpart.cpp
processlinemaker.cpp
processwidget.cpp
ksavealldialog.cpp
)

KDE_AUTOMOC(${kdevwidgets_LIB_SRCS})

ADD_LIBRARY(kdevwidgets SHARED ${kdevwidgets_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevwidgets ${QT_LIBRARIES} kdecore kdeui kparts khtml kio)

INSTALL_TARGETS(/lib kdevwidgets)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces $(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = libkdevwidgets.la
#
#kdevwidgetsincludedir = $(includedir)/kdevelop/widgets
#
#libkdevwidgets_la_SOURCES = flagboxes.cpp qcomboview.cpp klistviewaction.cpp \
#	kcomboview.cpp resizablecombo.cpp kdevhtmlpart.cpp processlinemaker.cpp \
#	processwidget.cpp ksavealldialog.cpp
#
#kdevwidgetsinclude_HEADERS = klistviewaction.h qcomboview.h flagboxes.h \
#	ksavealldialog.h resizablecombo.h kcomboview.h kdevhtmlpart.h processlinemaker.h \
#	processwidget.h
#
#SUBDIRS = propeditor
#
#libkdevwidgets_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la
#rcdir = $(kde_datadir)/kdevelop
#
#rc_DATA =  kdevhtml_partui.rc
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces
#DOXYGEN_PROJECTNAME = KDevelop Widgets Library
#DOXYGEN_DOCDIRPREFIX = kdev
#include ../../Doxyfile.am
