# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcvs.la
libkdevcvs_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvs_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcvs_la_SOURCES = cvsform.ui cvspart.cpp logformbase.ui logform.cpp commitdlg.cpp cvsoptionswidgetbase.ui cvsoptionswidget.cpp cvswidget.cpp cvsentry.cpp changelog.cpp cvsoptions.cpp cvsutils.cpp serverconfigurationwidgetbase.ui serverconfigurationwidget.cpp checkoutdialogbase.ui checkoutdialog.cpp tagdialogbase.ui tagdialog.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvs.desktop


rcdir = $(kde_datadir)/kdevcvs
rc_DATA = buildcvs.sh
