SUBDIRS = res

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general/ -I$(top_srcdir)/kdevelop/lib/projectmanagement/ \
		$(all_includes)

lib_LTLIBRARIES = libkdevkdesdiappwizard.la
libkdevkdesdiappwizard_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevkdesdiappwizard_la_LIBADD = $(top_builddir)/kdevelop/lib/general/libkdevelop.la \
				$(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
				$(LIB_KFILE)

libkdevkdesdiappwizard_la_SOURCES = kdesdiappwizardplugin.cpp main.cpp

noinst_HEADERS = main.h kdesdiappwizardplugin.h

METASOURCES = AUTO
servicedir = $(kde_servicesdir)
service_DATA = kdevkdesdiappwizard.desktop



