# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcvsservice.la
libkdevcvsservice_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvsservice_la_LIBADD = $(top_builddir)/lib/libkdevelop.la     $(top_builddir)/lib/interfaces/libkdevinterfaces.la     -lcvsservice

libkdevcvsservice_la_SOURCES = cvsformbase.ui cvsform.cpp cvspart.cpp logformbase.ui logform.cpp logform.skel commitdlg.cpp cvsoptionswidgetbase.ui cvsoptionswidget.cpp cvsprocesswidget.cpp cvsprocesswidget.skel cvsentry.cpp changelog.cpp cvsoptions.cpp serverconfigurationwidgetbase.ui serverconfigurationwidget.cpp checkoutdialogbase.ui checkoutdialog.cpp checkoutdialog.skel tagdialog.cpp tagdialog.h tagdialogbase.ui cvspartimpl.h cvspartimpl.cpp cvsserviceimpl.h cvsserviceimpl.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvsservice.desktop


rcdir = $(kde_datadir)/kdevcvsservice
rc_DATA = kdevcvsservicepart.rc buildcvs.sh
