SUBDIRS = res

lib_LTLIBRARIES = libkdevkdejavasdiappwizard.la
INCLUDES = 	-I$(top_srcdir)/kdevelop/lib/general \
						-I$(top_srcdir)/kdevelop/lib/projectmanagement \
						-I$(top_srcdir)/kdevelop/lib/ComponentManager \
						$(all_includes)
libkdevkdejavasdiappwizard_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdejavasdiappwizard_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevkdejavasdiappwizard_la_SOURCES = kdejavasdiappwizardplugin.cpp main.cpp

noinst_HEADERS = main.h kdejavasdiappwizardplugin.h

METASOURCES = AUTO
servicedir = $(kde_servicesdir)
service_DATA = kdevkdejavasdiappwizard.desktop



