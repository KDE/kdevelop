INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcopyto.la
libkdevcopyto_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcopyto_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcopyto_la_SOURCES = copyto_part.cpp copytodlgbase.ui copytodialog.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_copyto.desktop

rcdir = $(kde_datadir)/kdevcopyto
rc_DATA = kdevpart_copyto.rc
noinst_HEADERS = copytodialog.h
