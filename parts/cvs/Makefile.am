# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcvs.la
libkdevcvs_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvs_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la

#libkdevcvs_la_SOURCES = cvsform.ui cvspart.cpp commitdlg.cpp logform.ui cvsoptionswidgetbase.ui cvsoptionswidget.cpp cvswidget.cpp urlfileutilities.cpp
libkdevcvs_la_SOURCES = cvsform.ui urlfileutilities.cpp cvspart.cpp commitdlg.cpp logform.ui cvsoptionswidgetbase.ui cvsoptionswidget.cpp cvswidget.cpp cvs_commonoptions.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvs.desktop
libkdevcvs_la_DEPENDENCIES =

rcdir = $(kde_datadir)/kdevcvs
rc_DATA = kdevcvspart.rc buildcvs.sh
