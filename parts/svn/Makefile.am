# Here resides the svn part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevsvn.la
libkdevsvn_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevsvn_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la

libkdevsvn_la_SOURCES = svnform.ui urlfileutilities.cpp svnpart.cpp commitdlg.cpp logform.ui svnoptionswidgetbase.ui svnoptionswidget.cpp svnwidget.cpp svnentry.cpp changelog.cpp svnoptions.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevsvn.desktop
libkdevsvn_la_DEPENDENCIES =

rcdir = $(kde_datadir)/kdevsvn
rc_DATA = kdevsvnpart.rc buildsvn.sh
