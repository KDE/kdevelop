# Here resides the doc tree view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib $(all_includes)

lib_LTLIBRARIES = libkdevdoctreeview.la
libkdevdoctreeview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevdoctreeview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la $(LIB_KDEUI)

libkdevdoctreeview_la_SOURCES = main.cpp doctreeview.cpp doctreewidget.cpp

noinst_HEADERS = main.h doctreeview.h doctreewidget.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdoctreeview.desktop
