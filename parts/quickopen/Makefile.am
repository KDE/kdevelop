INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util

kde_module_LTLIBRARIES = libkdevquickopen.la
libkdevquickopen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkdevelop

libkdevquickopen_la_SOURCES = quickopen_part.cpp quickopenbase.ui quickopendialog.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_quickopen.desktop

rcdir = $(kde_datadir)/kdevquickopen
rc_DATA = kdevpart_quickopen.rc
noinst_HEADERS =  quickopendialog.h
