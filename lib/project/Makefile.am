# This directory collects some classes related to
# project management for the sole purpose that they
# can be shared between parts.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor $(all_includes)

noinst_LTLIBRARIES = libkdevproject.la

libkdevproject_la_SOURCES = runoptionswidget.cpp runoptionswidgetbase.ui makeoptionswidget.cpp makeoptionswidgetbase.ui addenvvardlg.cpp subclassesdlgbase.ui subclassesdlg.cpp environmentvariableswidgetbase.ui environmentvariableswidget.cpp addfilesdialog.cpp envvartools.cpp projectconfigurationwidget.cpp removesubprojectdialog.cpp removesubprojectdlgbase.ui

METASOURCES = AUTO

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS =  addenvvardlg.h addfilesdialog.h environmentvariableswidget.h environmentvariableswidgetbase.h envvartools.h makeoptionswidget.h makeoptionswidgetbase.h projectconfigurationwidget.h removesubprojectdialog.h removesubprojectdlgbase.h runoptionswidget.h runoptionswidgetbase.h subclassesdlg.h subclassesdlgbase.h
