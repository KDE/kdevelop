SUBDIRS = res

INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement/ $(all_includes)

lib_LTLIBRARIES = libkdesdi_appwizard.la
libkdesdi_appwizard_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdesdi_appwizard_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
				$(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la $(LIB_KFILE)

libkdesdi_appwizard_la_SOURCES = kdesdiappwizardplugin.cpp main.cpp

noinst_HEADERS = main.h kdesdiappwizardplugin.h

METASOURCES = AUTO



