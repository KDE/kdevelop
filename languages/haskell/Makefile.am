KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
SUBDIRS = app_templates file_templates compiler doc

kde_module_LTLIBRARIES = libkdevhaskellsupport.la
libkdevhaskellsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevhaskellsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevhaskellsupport_la_SOURCES = haskellsupport_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevhaskellsupport.desktop

rcdir = $(kde_datadir)/kdevhaskellsupport
rc_DATA = kdevhaskellsupport.rc
