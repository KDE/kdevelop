# Here resides the CSharp support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
SUBDIRS = app_templates file_templates doc


kde_module_LTLIBRARIES = libkdevcsharpsupport.la kio_csharpdoc.la
libkdevcsharpsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcsharpsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcsharpsupport_la_SOURCES = csharpsupportpart.cpp csharpconfigwidget.cpp csharpconfigwidgetbase.ui

kio_csharpdoc_la_SOURCES = csharpdoc.cpp
kio_csharpdoc_la_LIBADD = $(LIB_KIO)
kio_csharpdoc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcsharpsupport.desktop csharpdoc.protocol

rcdir = $(kde_datadir)/kdevcsharpsupport
rc_DATA = kdevcsharpsupport.rc
