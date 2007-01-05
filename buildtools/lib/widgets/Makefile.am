# This directory collects some classes related to
# project management for the sole purpose that they
# can be shared between parts.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets/propeditor $(all_includes)

lib_LTLIBRARIES = libkdevbuildtoolswidgets.la

libkdevbuildtoolswidgets_la_SOURCES = addenvvardlg.cpp addfilesdialog.cpp \
	environmentdisplaydialog.cpp environmentdisplaydialogbase.ui environmentvariableswidget.cpp \
	environmentvariableswidgetbase.ui envvartools.cpp makeoptionswidget.cpp makeoptionswidgetbase.ui \
	removesubprojectdialog.cpp removesubprojectdlgbase.ui runoptionswidget.cpp runoptionswidgetbase.ui \
	subclassesdlg.cpp subclassesdlgbase.ui

METASOURCES = AUTO

kdevelopbuildtoolsincludedir = $(includedir)/kdevelop/buildtools/widgets
kdevelopbuildtoolsinclude_HEADERS = addenvvardlg.h addfilesdialog.h \
	environmentvariableswidget.h environmentvariableswidgetbase.h envvartools.h makeoptionswidget.h \
	makeoptionswidgetbase.h removesubprojectdialog.h removesubprojectdlgbase.h runoptionswidget.h \
	runoptionswidgetbase.h subclassesdlg.h subclassesdlgbase.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces kdevutil
DOXYGEN_PROJECTNAME = KDevelop Buildtool Widgets Library
DOXYGEN_DOCDIRPREFIX = kdevbt
include ../../../Doxyfile.am
noinst_HEADERS = environmentdisplaydialog.h
