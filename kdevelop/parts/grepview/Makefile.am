# Here resides the grep view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager/Components \
					 $(all_includes)

lib_LTLIBRARIES = libkdevgrepview.la
libkdevgrepview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevgrepview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevgrepview_la_SOURCES = main.cpp grepview.cpp grepwidget.cpp grepdlg.cpp \
			     grepconfigwidget.cpp

noinst_HEADERS = main.h grepview.h grepwidget.h grepdlg.h grepconfigwidget.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevgrepview.desktop

rcdir = $(kde_datadir)/kdevgrepview
rc_DATA = kdevgrepview.rc
