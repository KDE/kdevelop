INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevquickopen.la
libkdevquickopen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevquickopen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevquickopen_la_SOURCES = quickopen_part.cpp quickopenbase.ui quickopendialog.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_quickopen.desktop

rcdir = $(kde_datadir)/kdevquickopen
rc_DATA = kdevpart_quickopen.rc
noinst_HEADERS =  quickopendialog.h
