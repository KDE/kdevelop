####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kdevelop
kdevelop_SOURCES = ctoolclass.cpp kdevsession.cpp clibpropdlgimpl.cpp clibpropdlg.ui ckonsolewidget.cpp docviewman.cpp ckdevelop.cpp ckdevelop_classview.cpp ckdevelop_noslot.cpp cprjoptionsdlg.cpp ckappwizard.cpp wzconnectdlg.ui ckdevelop_init.cpp main.cpp kstartuplogo.cpp kdevlistview.cpp grepdialog.cpp doctreeview.cpp dbgtoolbar.cpp dbgpsdlg.cpp ctreeview.cpp ctreehandler.cpp ctoolsconfigdlg.cpp crealfileview.cpp cproject.cpp coutputwidget.cpp cnewfiledlg.cpp cnewclassdlg.cpp cmakemanualdlgdata.cpp cmakemanualdlg.cpp clogfileview.cpp ckdevsetupdlg.cpp ckdevelop_whatsthis.cpp ckdevelop_project.cpp cclasstreehandler.cpp ckdevaccel.cpp cgrouppropertiesdlg.cpp cgeneratenewfile.cpp cfinddoctextdlg.cpp cfilepropdlg.cpp cexecuteargdlg.cpp ceditwidget.cpp cdoctreepropdlg.cpp cdocbrowser.cpp ccvaddfolderdlg.cpp cclonefunctiondlg.cpp cclassview.cpp wzconnectdlgimpl.cpp cclasstooltreeview.cpp cclasstooldlg.cpp caddnewtranslationdlg.cpp caddexistingfiledlg.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp 
kdevelop_LDADD   = ./vc/libkdevelopvc.la ./kwrite/libkdevelopkwrite.la ./gfxview/libkdevelopgfxview.la ./dbg/libkdevelopdbg.la ./classwizard/libclasswizard.a ./kpp/libkdevelopkpp.la ./classparser/libclassparser.a ./ctags/libctags.a ./setup/libkdevelopsetup.la ./widgets/qextmdi/src/libkdevelopqextmdi.la $(LEXLIB) $(LIB_KFILE) $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = classparser kpp classwizard dbg examples gfxview kwrite pics templates tools vc widgets ctags setup 

EXTRA_DIST = caddclassattributedlg.cpp caddclassattributedlg.h caddclassmethoddlg.cpp caddclassmethoddlg.h caddexistingfiledlg.cpp caddexistingfiledlg.h caddnewtranslationdlg.cpp caddnewtranslationdlg.h cclasstooldlg.cpp cclasstooldlg.h cclasstooltreeview.cpp cclasstooltreeview.h wzconnectdlgimpl.cpp cclasstreehandler.h cclassview.cpp cclassview.h cclonefunctiondlg.cpp cclonefunctiondlg.h ccvaddfolderdlg.cpp ccvaddfolderdlg.h cdocbrowser.cpp cdocbrowser.h cdoctreepropdlg.cpp cdoctreepropdlg.h ceditwidget.cpp ceditwidget.h wzconnectdlgimpl.h cexecuteargdlg.cpp cexecuteargdlg.h cfilepropdlg.cpp cfilepropdlg.h cfinddoctextdlg.cpp cfinddoctextdlg.h cgeneratenewfile.cpp cgeneratenewfile.h cgrouppropertiesdlg.cpp cgrouppropertiesdlg.h ckdevaccel.h ckdevaccel.cpp cclasstreehandler.cpp ckdevelop.h ckdevelop_project.cpp ckdevelop_whatsthis.cpp ckdevsetupdlg.cpp ckdevsetupdlg.h clogfileview.cpp clogfileview.h cmakemanualdlg.cpp cmakemanualdlg.h cmakemanualdlgdata.cpp cnewclassdlg.cpp cnewclassdlg.h cnewfiledlg.cpp cnewfiledlg.h coutputwidget.cpp coutputwidget.h cprjoptionsdlg.h cproject.cpp cproject.h crealfileview.cpp crealfileview.h ctoolsconfigdlg.cpp ctoolsconfigdlg.h ctreehandler.cpp ctreehandler.h ctreeview.cpp ctreeview.h dbgpsdlg.cpp dbgpsdlg.h dbgtoolbar.cpp dbgtoolbar.h debug.h doctreeview.cpp doctreeview.h extractrc grepdialog.cpp grepdialog.h kdevelop-setup.desktop kdevelop.desktop kdeveloprc kdevelopui.rc kdevlistview.cpp kdevlistview.h kstartuplogo.cpp kstartuplogo.h lo16-app-kdevelop.png lo32-app-kdevelop.png lo32-mime-kdevelop_project.png main.cpp resource.h structdef.h ckdevelop_init.cpp wzconnectdlg.ui x-kdevelop-project.desktop ckappwizard.cpp ckappwizard.h cprjoptionsdlg.cpp ckdevelop_noslot.cpp ckdevelop_classview.cpp ckdevelop.cpp hi16-app-kdevelop.png hi48-app-kdevelop.png docviewman.cpp docviewman.h ckonsolewidget.cpp ckonsolewidget.h clibpropdlg.ui clibpropdlgimpl.cpp clibpropdlgimpl.h kdevsession.cpp kdevsession.h ctoolclass.cpp ctoolclass.h 

####### kdevelop will overwrite this part!!! (end)############

INCLUDES = $(all_includes) -I$(top_builddir)/kdevelop/ctags -I$(top_builddir)/kdevelop/kpp -I$(top_srcdir)/kdevelop/widgets/qextmdi/include -I$(top_srcdir)/kdevelop/kpp -I$(top_srcdir)/kdevelop/kwrite

METASOURCES = AUTO

# the library search path.
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc

KDE_ICON = AUTO
KDE_OPTIONS = nofinal

data_DATA = kdevelop.desktop kdevelop-setup.desktop
datadir = $(kde_appsdir)/Development

mime_DATA = x-kdevelop-project.desktop
mimedir = $(kde_mimedir)/application

bin_SCRIPTS = extractrc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) -ktr $$LIST -o $(podir)/kdevelop.pot; \
	fi

