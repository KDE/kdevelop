# Here reside the output view and make view parts.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement -I$(top_srcdir)/kdevelop/lib/general $(all_includes)

lib_LTLIBRARIES = libkdevoutputviews.la
libkdevoutputviews_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevoutputviews_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevoutputviews_la_SOURCES = main.cpp outputviews.cpp makewidget.cpp appoutputwidget.cpp 

noinst_HEADERS = main.h outputviews.h makewidget.h appoutputwidget.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappoutputview.desktop kdevmakeview.desktop

rcdir = $(kde_datadir)/kdevoutputviews
rc_DATA = kdevmakeview.rc
