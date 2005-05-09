SUBDIRS( integrator )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets )

SET(kdevcvsservice_PART_SRCS
cvspart.cpp
cvspartimpl.cpp
cvsform.cpp
commitdlg.cpp
cvsoptionswidget.cpp
cvsprocesswidget.cpp
cvsentry.cpp
cvsdir.cpp
changelog.cpp
cvsoptions.cpp
checkoutdialog.cpp
tagdialog.cpp
diffdialog.cpp
releaseinputdialog.cpp
cvslogdialog.cpp
cvslogpage.cpp
cvsdiffpage.cpp
diffwidget.cpp
jobscheduler.cpp
bufferedstringreader.cpp
cvsfileinfoprovider.cpp
editorsdialog.cpp
annotatedialog.cpp
annotatepage.cpp
annotateview.cpp
)

SET( kdevcvsservice_UI
cvsformbase.ui
commitdialogbase.ui
cvsoptionswidgetbase.ui
checkoutdialogbase.ui
tagdialogbase.ui
diffdialogbase.ui
releaseinputdialogbase.ui
editorsdialogbase.ui
)

SET( kdevcvsservice_DCOP_SRCS
cvsservicedcopIface.h
)

KDE_AUTOMOC(${kdevcvsservice_PART_SRCS})

KDE_ADD_UI_FILES(kdevcvsservice_PART_SRCS ${kdevcvsservice_UI} )

KDE_ADD_DCOP_SKELS(kdevcvsservice_PART_SRCS ${kdevcvsservice_DCOP_SRCS})

ADD_LIBRARY(kdevcvsservice SHARED ${kdevcvsservice_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevcvsservice ${QT_LIBRARIES} kdecore kdeui kio kparts DCOP kdevelop cvsservice)

#original Makefile.am contents follow:

## Here resides the cvs part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevcvsservice.la
#libkdevcvsservice_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevcvsservice_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML) -lcvsservice
#
#libkdevcvsservice_la_SOURCES = cvspart.cpp cvspartimpl.cpp cvsformbase.ui \
#	cvsform.cpp commitdialogbase.ui commitdlg.cpp cvsoptionswidgetbase.ui \
#	cvsoptionswidget.cpp cvsprocesswidget.cpp cvsentry.cpp cvsdir.cpp changelog.cpp \
#	cvsoptions.cpp checkoutdialogbase.ui checkoutdialog.cpp tagdialog.cpp tagdialogbase.ui \
#	diffdialogbase.ui diffdialog.cpp releaseinputdialogbase.ui releaseinputdialog.cpp \
#	cvslogdialog.cpp cvslogpage.cpp cvsdiffpage.cpp diffwidget.cpp jobscheduler.cpp \
#	bufferedstringreader.cpp cvsfileinfoprovider.cpp cvsservicedcopIface.skel editorsdialogbase.ui \
#	editorsdialog.cpp
#
#noinst_HEADERS = changelog.h checkoutdialog.h checkoutdialogbase.h commitdlg.h \
#	cvsentry.h cvsform.h cvsformbase.h cvsoptions.h cvsoptionswidget.h \
#	cvsoptionswidgetbase.h cvspart.h cvspartimpl.h cvsprocesswidget.h tagdialog.h tagdialogbase.h \
#	diffdialog.h cvsdir.h cvslogpage.h cvslogdialog.h jobscheduler.h diffwidget.h \
#	cvsfileinfoprovider.h cvsservicedcopIface.h bufferedstringreader.h editorsdialog.h \
#	editorsdialogbase.h
#
#METASOURCES = AUTO
#
#ICONS = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevcvsservice.desktop
#
#servicepicsdir = $(kde_datadir)/kdevcvsservice/pics
#servicepics_DATA = kdev_cvs.png
#
#rcdir = $(kde_datadir)/kdevcvsservice
#rc_DATA = buildcvs.sh
#
#SUBDIRS = integrator
