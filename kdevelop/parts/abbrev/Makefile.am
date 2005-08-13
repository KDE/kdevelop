# Here resides the abbrev part

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevabbrev.la
libkdevabbrev_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevabbrev_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevabbrev_la_SOURCES = abbrevpart.cpp abbrevconfigwidget.cpp abbrevconfigwidgetbase.ui addtemplatedlg.cpp addtemplatedlgbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevabbrev.desktop

sourcesdir = $(kde_datadir)/kdevabbrev/sources
sources_DATA = qt_classes cpp_keywords

rcdir = $(kde_datadir)/kdevabbrev
rc_DATA = kdevabbrev.rc
