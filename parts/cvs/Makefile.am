# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcvs.la
libkdevcvs_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvs_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la

libkdevcvs_la_SOURCES = cvspart.cpp commitdlg.cpp logform.ui cvsoptionswidgetbase.ui cvsoptionswidget.cpp cvsoptionswidget.h cvswidget.cpp cvswidget.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvs.desktop
libkdevcvs_la_DEPENDENCIES = 
SUBDIRS = globalcvs
