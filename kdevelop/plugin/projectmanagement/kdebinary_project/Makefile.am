
INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement/ -I$(top_srcdir)/kdevelop/lib/ \
		$(all_includes)

lib_LTLIBRARIES = libkdevkdebinaryproject.la
libkdevkdebinaryproject_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdebinaryproject_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
				$(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
				 $(LIB_KFILE)

libkdevkdebinaryproject_la_SOURCES = kdebinaryprojectplugin.cpp main.cpp

noinst_HEADERS = main.h kdebinaryprojectplugin.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevkdebinaryproject.desktop
