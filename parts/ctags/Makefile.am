# Here resides the ctags part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevctags.la
libkdevctags_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevctags_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevctags_la_SOURCES = ctagspart.cpp ctagsdlg.cpp ctagskinds.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevctags.desktop

rcdir = $(kde_datadir)/kdevctags
rc_DATA = kdevctags.rc
