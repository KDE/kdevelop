INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement/ $(all_includes)

lib_LTLIBRARIES = libjava_projectspace.la
libjava_projectspace_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libjava_projectspace_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
				 $(LIB_KFILE)

libjava_projectspace_la_SOURCES = javaprojectspace.cpp main.cpp

noinst_HEADERS = main.h javaprojectspace.h

METASOURCES = AUTO
