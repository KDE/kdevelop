# Here reside the output view and make view parts.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extensions/dcop -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets $(all_includes)

noinst_LTLIBRARIES = libkdevoutputviews.la  
kde_module_LTLIBRARIES = libkdevmakeview.la libkdevappview.la

libkdevoutputviews_la_SOURCES = directorystatusmessagefilter.cpp \
	outputfilter.cpp compileerrorfilter.cpp commandcontinuationfilter.cpp makeitem.cpp \
	makeactionfilter.cpp otherfilter.cpp filterdlg.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappoutputview.desktop kdevmakeview.desktop

rc1dir = $(kde_datadir)/kdevappoutputview
rc1_DATA = kdevmakeview.rc

rc2dir = $(kde_datadir)/kdevmakeview
rc2_DATA = kdevmakeview.rc

noinst_HEADERS = filterdlg.h
libkdevmakeview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevmakeview_la_LIBADD = $(top_builddir)/lib/widgets/libwidgets.la \
	$(top_builddir)/lib/interfaces/extensions/dcop/libkdevdcopextensions.la $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/outputviews/libkdevoutputviews.la $(LIB_KHTML)
libkdevmakeview_la_SOURCES = makeviewpart.cpp makewidget.cpp
libkdevappview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevappview_la_LIBADD = $(top_builddir)/lib/widgets/libwidgets.la \
	$(top_builddir)/lib/interfaces/extensions/dcop/libkdevdcopextensions.la $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/outputviews/libkdevoutputviews.la $(LIB_KHTML)
libkdevappview_la_SOURCES = appoutputviewpart.cpp appoutputwidget.cpp
