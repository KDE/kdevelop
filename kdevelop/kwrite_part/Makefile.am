# $Id$

INCLUDES = -I$(top_srcdir)/kdevelop/lib/kidetexteditor $(all_includes)

KDE_CXXFLAGS = $(USE_RTTI)

lib_LTLIBRARIES = libkidewritepart.la

libkidewritepart_la_SOURCES = highlight.cpp ktextprint.cpp  \
		           kwdoc.cpp kwview.cpp undohistory.cpp  \
		           kwdialog.cpp kwrite_factory.cpp kwriteIface.skel

libkidewritepart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkidewritepart_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la -lkspell

dummy.cpp:
	echo > dummy.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kidewrite.desktop

rcdir = $(kde_datadir)/kwrite
rc_DATA = kidewrite_browser.rc kidewriteui.rc

