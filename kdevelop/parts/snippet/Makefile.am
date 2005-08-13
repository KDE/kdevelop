INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevsnippet.la
libkdevsnippet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevsnippet_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevsnippet_la_SOURCES = snippet_part.cpp snippet_widget.cpp snippetdlg.ui snippetitem.cpp snippetsettingsbase.ui snippetsettings.cpp snippetconfig.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevsnippet.desktop

rcdir = $(kde_datadir)/kdevsnippet

noinst_HEADERS = snippetdlg.h snippetitem.h snippetsettingsbase.h snippetsettings.h snippetconfig.h
