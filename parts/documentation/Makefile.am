INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevdocumentation.la
libkdevdocumentation_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdocumentation_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(LIB_KHTML)

libkdevdocumentation_la_SOURCES = documentation_part.cpp documentation_widget.cpp contentsview.cpp indexview.cpp docglobalconfigwidgetbase.ui docglobalconfigwidget.cpp docconfiglistview.cpp editcatalogdlgbase.ui editcatalogdlg.cpp docutils.cpp searchview.cpp bookmarkview.cpp editbookmarkdlg.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_documentation.desktop

rcdir = $(kde_datadir)/kdevdocumentation
rc_DATA = kdevpart_documentation.rc

noinst_HEADERS = contentsview.h indexview.h docglobalconfigwidget.h docconfiglistview.h editcatalogdlg.h docutils.h searchview.h bookmarkview.h
SUBDIRS = plugins tools data
