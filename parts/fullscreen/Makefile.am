INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevfullscreen.la
libkdevfullscreen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfullscreen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfullscreen_la_SOURCES = fullscreen_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfullscreen.desktop

rcdir = $(kde_datadir)/kdevfullscreen
rc_DATA = kdevpart_fullscreen.rc
