SUBDIRS = res

lib_LTLIBRARIES = libkdevkdeobjcsdiappwizard.la
INCLUDES = 	-I$(top_srcdir)/kdevelop/lib/general \
						-I$(top_srcdir)/kdevelop/lib/projectmanagement \
						-I$(top_srcdir)/kdevelop/lib/ComponentManager \
						$(all_includes)
libkdevkdeobjcsdiappwizard_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdeobjcsdiappwizard_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevkdeobjcsdiappwizard_la_SOURCES = kdeobjcsdiappwizardplugin.cpp main.cpp

noinst_HEADERS = main.h kdeobjcsdiappwizardplugin.h

METASOURCES = AUTO
servicedir = $(kde_servicesdir)
service_DATA = kdevkdeobjcsdiappwizard.desktop



