INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement -I$(top_srcdir)/kdevelop/lib/general $(all_includes)

lib_LTLIBRARIES = libkdevkdeprojectspace.la
libkdevkdeprojectspace_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdeprojectspace_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevkdeprojectspace_la_SOURCES = kdeprojectspace.cpp main.cpp

noinst_HEADERS = main.h kdeprojectspace.h

METASOURCES = AUTO

SUBDIRS = res

servicedir = $(kde_servicesdir)
service_DATA = kdevkdeprojectspace.desktop
rcdir = $(kde_datadir)/kdevkdeprojectspace
rc_DATA = kdevkdeprojectspaceui.rc
