# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcvsservice.la
libkdevcvsservice_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvsservice_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lcvsservice

libkdevcvsservice_la_SOURCES = cvsformbase.ui cvsform.cpp cvspart.cpp logformbase.ui logform.cpp logform.skel commitdlg.cpp cvsoptionswidgetbase.ui cvsoptionswidget.cpp cvsprocesswidget.cpp cvsprocesswidget.skel cvsentry.cpp changelog.cpp cvsoptions.cpp checkoutdialogbase.ui checkoutdialog.cpp checkoutdialog.skel tagdialog.cpp tagdialogbase.ui cvspartimpl.cpp cvsserviceimpl.cpp diffdialog.cpp diffdialogbase.ui releaseinputdialogbase.ui releaseinputdialog.cpp commitdialogbase.ui cvsdir.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvsservice.desktop


rcdir = $(kde_datadir)/kdevcvsservice
rc_DATA = kdevcvsservicepart.rc buildcvs.sh
noinst_HEADERS = changelog.h checkoutdialog.h checkoutdialogbase.h commitdlg.h cvsentry.h cvsform.h cvsformbase.h cvsoptions.h cvsoptionswidget.h cvsoptionswidgetbase.h cvspart.h cvspartimpl.h cvsprocesswidget.h cvsserviceimpl.h cvsutils.h logform.h logformbase.h tagdialog.h tagdialogbase.h diffdialog.h cvsdir.h
