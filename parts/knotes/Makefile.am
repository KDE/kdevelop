INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevnotes.la

libkdevnotes_la_SOURCES = knotes_plugin.cpp knotes_part.cpp
libkdevnotes_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevnotes_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_notes.desktop

rc_DATA = kdevpart_notes.rc
rcdir = $(kde_datadir)/kdevnotes
