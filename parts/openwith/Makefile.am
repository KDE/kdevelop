# Here resides the openwith part.

INCLUDES = \
  -I$(top_srcdir)/lib/interfaces \
  -I$(top_srcdir)/lib/util \
  $(all_includes)

kde_module_LTLIBRARIES = libkdevopenwith.la
libkdevopenwith_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevopenwith_la_LIBADD = \
  $(top_builddir)/lib/libkdevelop.la

libkdevopenwith_la_SOURCES = \
  openwithpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevopenwith.desktop

