INCLUDES = -I$(top_srcdir)/lib/catalog -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevclassbrowser.la
libkdevclassbrowser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevclassbrowser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/catalog/libkdevcatalog.la

libkdevclassbrowser_la_SOURCES = classbrowser_part.cpp classbrowser_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_classbrowser.desktop

rcdir = $(kde_datadir)/kdevclassbrowser
rc_DATA = kdevpart_classbrowser.rc
