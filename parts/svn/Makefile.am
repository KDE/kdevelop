INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes) $(SVNINC)

kde_module_LTLIBRARIES = libkdevsvn.la
libkdevsvn_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(SVNLD)
libkdevsvn_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(SVNLIBS) -lsvn_client-1

libkdevsvn_la_SOURCES = svn_part.cpp svnoptionswidget.cpp svnoptions.ui commitdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_svn.desktop

