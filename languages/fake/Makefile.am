
INCLUDES = \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/util \
    $(all_includes)

kde_module_LTLIBRARIES = libkdevfakesupport.la
libkdevfakesupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfakesupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfakesupport_la_SOURCES = \
    fakesupport_part.cpp \
    fakesupport_factory.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfakesupport.desktop

rcdir = $(kde_datadir)/kdevfakesupport
rc_DATA = kdevfakesupport.rc

