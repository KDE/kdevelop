INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = ktabzoomwidget.cpp ktabzoombar.cpp ktabzoombutton.cpp \
	ktabzoomframe.cpp ktabzoombarlayout.cpp flagboxes.cpp qcomboview.cpp klistviewaction.cpp \
	kcomboview.cpp resizablecombo.cpp doclineedit.cpp kdevhtmlpart.cpp \
	processlinemaker.cpp processwidget.cpp ksavealldialog.cpp
noinst_HEADERS = resizablecombo.h doclineedit.h kdevhtmlpart.h
SUBDIRS = propeditor
libwidgets_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la
rcdir = $(kde_datadir)/kdevelop

rc_DATA =  kdevhtml_partui.rc
