INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevvcsmanager.la
libkdevvcsmanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevvcsmanager_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevvcsmanager_la_SOURCES = vcsmanagerpart.cpp \
	vcsmanagerprojectconfigbase.ui vcsmanagerprojectconfig.cpp

METASOURCES = AUTO

appsharedir = $(kde_datadir)/kdevvcsmanager
appshare_DATA = 

servicedir = $(kde_servicesdir)
service_DATA = kdevvcsmanager.desktop

rcdir = $(kde_datadir)/kdevvcsmanager
rc_DATA = kdevvcsmanager.rc
