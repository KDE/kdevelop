# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/classparser $(all_includes)

lib_LTLIBRARIES = libkdevclassview.la
libkdevclassview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevclassview_la_LIBADD = $(top_builddir)/kdevelop/lib/general/libkdevelop.la

libkdevclassview_la_SOURCES = main.cpp classview.cpp classactions.cpp

noinst_HEADERS = main.h classview.h classactions.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevclassview.desktop

rcdir = $(kde_datadir)/kdevclassview
rc_DATA = kdevclassview.rc
