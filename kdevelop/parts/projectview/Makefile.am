# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager/Components \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler $(all_includes)

lib_LTLIBRARIES = libkdevprojectview.la
libkdevprojectview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevprojectview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevprojectview_la_SOURCES = main.cpp projectview.cpp projecttreewidget.cpp

noinst_HEADERS = main.h projectview.h projecttreewidget.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevprojectview.desktop

