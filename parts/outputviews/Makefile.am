# Here reside the output view and make view parts.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes) 

kde_module_LTLIBRARIES = libkdevoutputviews.la
libkdevoutputviews_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevoutputviews_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevoutputviews_la_SOURCES = outputviewsfactory.cpp makeviewpart.cpp makewidget.cpp appoutputviewpart.cpp appoutputwidget.cpp directorystatusmessagefilter.cpp directorystatusmessagefilter.h outputfilter.cpp outputfilter.h compileerrorfilter.h compileerrorfilter.cpp commandcontinuationfilter.h commandcontinuationfilter.cpp makeitem.h makeitem.cpp makeactionfilter.cpp makeactionfilter.h otherfilter.cpp otherfilter.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappoutputview.desktop kdevmakeview.desktop

rcdir = $(kde_datadir)/kdevoutputviews
rc_DATA = kdevmakeview.rc
