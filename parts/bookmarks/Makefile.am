INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevbookmarks.la
libkdevbookmarks_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevbookmarks_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevbookmarks_la_SOURCES = bookmarks_part.cpp bookmarks_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_bookmarks.desktop
