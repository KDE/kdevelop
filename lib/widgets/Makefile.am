INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libkdevwidgets.la

kdevwidgetsincludedir = $(includedir)/kdevelop/widgets

libkdevwidgets_la_SOURCES = ktabzoomwidget.cpp ktabzoombar.cpp ktabzoombutton.cpp \
	ktabzoomframe.cpp ktabzoombarlayout.cpp flagboxes.cpp qcomboview.cpp klistviewaction.cpp \
	kcomboview.cpp resizablecombo.cpp doclineedit.cpp kdevhtmlpart.cpp \
	processlinemaker.cpp processwidget.cpp ksavealldialog.cpp
kdevwidgetsinclude_HEADERS = doclineedit.h klistviewaction.h ktabzoomframe.h qcomboview.h \
	flagboxes.h      ksavealldialog.h     ktabzoomposition.h  resizablecombo.h \
	kcomboview.h     ktabzoombar.h        ktabzoomwidget.h \
	kdevhtmlpart.h   ktabzoombarlayout.h  processlinemaker.h \
	kdevtabwidget.h  ktabzoombutton.h     processwidget.h
SUBDIRS = propeditor
libkdevwidgets_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la
rcdir = $(kde_datadir)/kdevelop

rc_DATA =  kdevhtml_partui.rc
