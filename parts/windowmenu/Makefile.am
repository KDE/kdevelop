# Here resides the windowmenu part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevwindowmenu.la
libkdevwindowmenu_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevwindowmenu_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevwindowmenu_la_SOURCES = windowmenupart.cpp 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevwindowmenu.desktop

rcdir = $(kde_datadir)/kdevwindowmenu
rc_DATA = kdevwindowmenu.rc
