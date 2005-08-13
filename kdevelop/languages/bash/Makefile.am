#Here resides the Bash Shell support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
SUBDIRS = app_templates doc

kde_module_LTLIBRARIES = libkdevbashsupport.la
libkdevbashsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevbashsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevbashsupport_la_SOURCES = bashsupport_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevbashsupport.desktop

rcdir = $(kde_datadir)/kdevbashsupport
rc_DATA = kdevbashsupport.rc
