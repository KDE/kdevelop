# Here resides the code view part.

INCLUDES = \
	-I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevcodeview.la
libkdevcodeview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcodeview_la_LIBADD = \
  $(top_builddir)/lib/libkdevelop.la \
  $(LIB_KHTML)

libkdevcodeview_la_SOURCES = kdevcodeview.cpp kdevcodeview_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcodeview.desktop

rcdir = $(kde_datadir)/kdevcodeview
rc_DATA = kdevcodeview.rc
