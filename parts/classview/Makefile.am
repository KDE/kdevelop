# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/sourceinfo \
	   -I$(top_srcdir)/lib/compat \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes)

kde_module_LTLIBRARIES = libkdevclassview.la
libkdevclassview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevclassview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevclassview_la_SOURCES = classviewpart.cpp \
			      classviewwidget.cpp \
			      classactions.cpp \
			      classtreebase.cpp \
			      classtoolwidget.cpp \
			      classtooldlg.cpp \
			      hierarchydlg.cpp \
			      digraphview.cpp \
			      classviewconfigwidgetbase.ui \
			      classviewconfigwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevclassview.desktop

rcdir = $(kde_datadir)/kdevclassview
rc_DATA = kdevclassview.rc


QTDIR=/home/bernd/kdesrc/qt-copy

foo: digraphview.cpp digraphview.h
	$(QTDIR)/bin/moc digraphview.h > digraphview.moc ; \
	g++ digraphview.cpp -I$(QTDIR)/include -L$(QTDIR)/lib -L/usr/X11R6/lib -lqt -lX11 -lXext -o foo
