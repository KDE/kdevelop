# Here resides the doc tree view part.

SUBDIRS = chm

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevdoctreeview.la kio_kdeapi.la

libkdevdoctreeview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdoctreeview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libwidgets.la $(LIB_KHTML)

libkdevdoctreeview_la_SOURCES = doctreeviewfactory.cpp doctreeviewpart.cpp doctreeviewwidget.cpp doctreeglobalconfigwidgetbase.ui doctreeglobalconfigwidget.cpp adddocitemdlg.cpp docindexdlg.cpp docsearchdlg.cpp misc.cpp librarydocdlgbase.ui doctreeprojectconfigwidgetbase.ui doctreeprojectconfigwidget.cpp librarydocdlg.cpp choosedlg.cpp docindexwatcher.cpp doctreeitem.cpp

#bin_PROGRAMS = kdevelop-htdig
#kdevelop_htdig_SOURCES = htdigindex.cpp

# kdevelop_index_CXXFLAGS = -DINDEXER ## makes problems with automake 1.5 and am_edit (Walter/2.01.2002)

#kdevelop_htdig_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kdevelop_htdig_LDADD = $(top_builddir)/lib/libkdevelop.la $(LIB_KIO) $(LIB_KDEUI)

kio_kdeapi_la_SOURCES = kio_kdeapi.cpp
kio_kdeapi_la_LIBADD = $(LIB_KIO)
kio_kdeapi_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdoctreeview.desktop kdeapi.protocol

rcdir = $(kde_datadir)/kdevdoctreeview
rc_DATA = kdevdoctreeview.rc

noinst_HEADERS =  docindexwatcher.h doctreeitem.h
