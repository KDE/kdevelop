SUBDIRS = res

lib_LTLIBRARIES = libkdevkdesdiappwizard.la
INCLUDES = -I$(top_srcdir)/kdevelop/lib/general/ -I$(top_srcdir)/kdevelop/lib/projectmanagement/ \
		$(all_includes)
libkdevkdesdiappwizard_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdesdiappwizard_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevkdesdiappwizard_la_SOURCES = kdesdiappwizardplugin.cpp main.cpp

noinst_HEADERS = main.h kdesdiappwizardplugin.h

METASOURCES = AUTO
servicedir = $(kde_servicesdir)
service_DATA = kdevkdesdiappwizard.desktop



