INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement \
           -I$(top_srcdir)/kdevelop/lib/general \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager/Components \
           $(all_includes)

lib_LTLIBRARIES = libkdevkdeobjcprojectspace.la
libkdevkdeobjcprojectspace_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdeobjcprojectspace_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la $(LIB_KIO)
libkdevkdeobjcprojectspace_la_SOURCES = kdeobjcprojectspace.cpp main.cpp

noinst_HEADERS = main.h kdeobjcprojectspace.h

METASOURCES = AUTO

SUBDIRS = res

servicedir = $(kde_servicesdir)
service_DATA = kdevkdeobjcprojectspace.desktop
rcdir = $(kde_datadir)/kdevkdeobjcprojectspace
rc_DATA = kdevkdeobjcprojectspaceui.rc
