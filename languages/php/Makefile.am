
INCLUDES = \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/util \
    $(all_includes)

kde_module_LTLIBRARIES = libkdevphpsupport.la
libkdevphpsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevphpsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevphpsupport_la_SOURCES = \
    phplanguagesupport.cpp \
    phpsupportfactory.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevphpsupport.desktop

rcdir = $(kde_datadir)/kdevphpsupport
rc_DATA = kdevphpsupport.rc

