INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevbookmarks.la
libkdevbookmarks_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevbookmarks_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevbookmarks_la_SOURCES = bookmarks_part.cpp bookmarks_widget.cpp bookmarks_settings_base.ui bookmarks_settings.cpp bookmarks_config.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevbookmarks.desktop

noinst_HEADERS =  bookmarks_settings.h bookmarks_config.h
