# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevcvsservice.la
libkdevcvsservice_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvsservice_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/widgets/libwidgets.la $(LIB_KHTML) -lcvsservice

libkdevcvsservice_la_SOURCES = cvspart.cpp cvspartimpl.cpp \
    cvsformbase.ui cvsform.cpp \
    commitdialogbase.ui commitdlg.cpp \
    cvsoptionswidgetbase.ui cvsoptionswidget.cpp \
    cvsprocesswidget.cpp \
    cvsentry.cpp cvsdir.cpp changelog.cpp cvsoptions.cpp \
    checkoutdialogbase.ui checkoutdialog.cpp \
    tagdialog.cpp tagdialogbase.ui \
    diffdialogbase.ui diffdialog.cpp \
    releaseinputdialogbase.ui releaseinputdialog.cpp \
    cvslogdialog.cpp cvslogpage.cpp cvsdiffpage.cpp diffwidget.cpp \
    jobscheduler.cpp \
    bufferedstringreader.cpp \
    cvsfileinfoprovider.cpp cvsservicedcopIface.skel

noinst_HEADERS = changelog.h checkoutdialog.h checkoutdialogbase.h commitdlg.h \
    cvsentry.h cvsform.h cvsformbase.h cvsoptions.h cvsoptionswidget.h cvsoptionswidgetbase.h \
    cvspart.h cvspartimpl.h cvsprocesswidget.h tagdialog.h tagdialogbase.h diffdialog.h \
    cvsdir.h cvslogpage.h cvslogdialog.h jobscheduler.h diffwidget.h cvsfileinfoprovider.h \
    cvsservicedcopIface.h bufferedstringreader.h
    
METASOURCES = AUTO

ICONS = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvsservice.desktop

servicepicsdir = $(kde_datadir)/kdevcvsservice/pics
servicepics_DATA = kdev_cvs.png

rcdir = $(kde_datadir)/kdevcvsservice
rc_DATA = buildcvs.sh

SUBDIRS = integrator
