INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement/ $(all_includes)

lib_LTLIBRARIES = libkde_projectspace.la
libkde_projectspace_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkde_projectspace_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
				$(LIB_KFILE)

libkde_projectspace_la_SOURCES = kdeprojectspaceplugin.cpp main.cpp

noinst_HEADERS = main.h kdeprojectspaceplugin.h

METASOURCES = AUTO

SUBDIRS = res

