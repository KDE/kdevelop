INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcloser.la
libkdevcloser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcloser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcloser_la_SOURCES = closer_part.cpp closer_dialog.ui closer_dialogimpl.cpp closer_dialogimpl.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_closer.desktop

rcdir = $(kde_datadir)/kdevcloser
rc_DATA = kdevpart_closer.rc
