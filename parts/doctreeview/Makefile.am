# Here resides the doc tree view part.

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

lib_LTLIBRARIES = libkdevdoctreeview.la
libkdevdoctreeview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevdoctreeview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdoctreeview_la_SOURCES = doctreeviewfactory.cpp \
				doctreeviewpart.cpp \
				doctreeviewwidget.cpp \
			        doctreeconfigwidget.cpp \
				adddocitemdlg.cpp \
				docindexdlg.cpp \
				docsearchdlg.cpp \
				misc.cpp

bin_PROGRAMS = gideon-index
gideon_index_SOURCES = htdigindex.cpp misc.cpp
gideon_index_LDFLAGS = $(all_libraries) $(KDE_RPATH)
gideon_index_LDADD = $(LIB_KDEUI)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdoctreeview.desktop

rcdir = $(kde_datadir)/gideon
rc_DATA = kdevdoctreeview.rc
