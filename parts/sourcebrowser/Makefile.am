INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevsourcebrowser.la
libkdevsourcebrowser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevsourcebrowser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevsourcebrowser_la_SOURCES = sourcebrowser_factory.cpp sourcebrowser_part.cpp sourcebrowser_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_sourcebrowser.desktop

