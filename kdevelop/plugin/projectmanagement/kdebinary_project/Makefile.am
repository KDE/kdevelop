
INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement/ $(all_includes)

lib_LTLIBRARIES = libkdebinary_project.la
libkdebinary_project_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdebinary_project_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
				 $(LIB_KFILE)

libkdebinary_project_la_SOURCES = kdebinaryprojectplugin.cpp main.cpp

noinst_HEADERS = main.h kdebinaryprojectplugin.h

METASOURCES = AUTO



