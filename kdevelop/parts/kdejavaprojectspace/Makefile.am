INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement \
           -I$(top_srcdir)/kdevelop/lib/general \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager \
					 -I$(top_srcdir)/kdevelop/lib/ComponentManager/Components \
           $(all_includes)

lib_LTLIBRARIES = libkdevkdejavaprojectspace.la
libkdevkdejavaprojectspace_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdejavaprojectspace_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la $(LIB_KIO)
libkdevkdejavaprojectspace_la_SOURCES = kdejavaprojectspace.cpp main.cpp

noinst_HEADERS = main.h kdejavaprojectspace.h

METASOURCES = AUTO

SUBDIRS = res

servicedir = $(kde_servicesdir)
service_DATA = kdevkdejavaprojectspace.desktop
rcdir = $(kde_datadir)/kdevkdejavaprojectspace
rc_DATA = kdevkdejavaprojectspaceui.rc
