INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevpascalsupport.la
libkdevpascalsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevpascalsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevpascalsupport_la_SOURCES = pascalsupport_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpascalsupport.desktop

rcdir = $(kde_datadir)/kdevpascalsupport
rc_DATA = kdevpascalsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = pascaltemplates
