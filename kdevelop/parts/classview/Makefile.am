# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/sourceinfo \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler $(all_includes)

lib_LTLIBRARIES = libkdevclassview.la
libkdevclassview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevclassview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevclassview_la_SOURCES = main.cpp classview.cpp classactions.cpp classtreebase.cpp \
			      classtreewidget.cpp classtoolwidget.cpp classtooldlg.cpp

noinst_HEADERS = main.h classview.h classactions.h classtreebase.h \
		 classtreewidget.h classtoolwidget.h classtooldlg.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevclassview.desktop

rcdir = $(kde_datadir)/kdevclassview
rc_DATA = kdevclassview.rc


QTDIR=/home/bernd/kdecvs2.0/qt-copy

foo: digraphview.cpp
	g++ digraphview.cpp -I$(QTDIR)/include -L$(QTDIR)/lib -L/usr/X11R6/lib -lqt -lX11 -lXext -o foo
