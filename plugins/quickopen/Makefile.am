INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevquickopen.la
libkdevquickopen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevquickopen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(LIB_KHTML)

libkdevquickopen_la_SOURCES = quickopen_part.cpp quickopen_model.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevquickopen.desktop

rcdir = $(kde_datadir)/kdevquickopen
rc_DATA = kdevpart_quickopen.rc
noinst_HEADERS = quickopen_part.h quickopen_model.h
