# Here resides the doc tree view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler \
 					 -I$(top_srcdir)/kdevelop/lib/ComponentManager \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager/Components \
						$(all_includes)

lib_LTLIBRARIES = libkdevdoctreeview.la
libkdevdoctreeview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevdoctreeview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevdoctreeview_la_SOURCES = main.cpp doctreeview.cpp doctreewidget.cpp \
			        doctreeconfigwidget.cpp cdoctreepropdlg.cpp

noinst_HEADERS = main.h doctreeview.h doctreewidget.h doctreeconfigwidget.h cdoctreepropdlg.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdoctreeview.desktop
