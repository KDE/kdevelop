# Here resides the grep view part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

lib_LTLIBRARIES = libkdevgrepview.la
libkdevgrepview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevgrepview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevgrepview_la_SOURCES = grepviewfactory.cpp \
			     grepviewpart.cpp \
			     grepviewwidget.cpp \
			     grepdlg.cpp \
			     grepconfigwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevgrepview.desktop

rcdir = $(kde_datadir)/gideon
rc_DATA = kdevgrepview.rc
