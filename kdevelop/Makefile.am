####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kdevelop
kdevelop_SOURCES = ckonsolewidget.cpp docviewman.cpp cerrormessageparser.cpp ckdevelop.cpp ckdevelop_classview.cpp ckdevelop_noslot.cpp cprjoptionsdlg.cpp ckappwizard.cpp wzconnectdlg.ui ckdevelop_init.cpp main.cpp ktipofdaydlg.ui ktipofday.cpp kstartuplogo.cpp klangcombo.cpp kdevlistview.cpp grepdialog.cpp doctreeview.cpp dbgtoolbar.cpp dbgpsdlg.cpp cupdatekdedocdlg.cpp ctreeview.cpp ctreehandler.cpp ctoolsconfigdlg.cpp ctoolclass.cpp ctabctl.cpp crealfileview.cpp cproject.cpp coutputwidget.cpp cnewfiledlg.cpp cnewclassdlg.cpp cmakemanualdlgdata.cpp cmakemanualdlg.cpp clogfileview.cpp ckdevsetupdlg.cpp ckdevinstall.cpp ckdevelop_whatsthis.cpp ckdevelop_project.cpp cclasstreehandler.cpp ckdevaccel.cpp cgrouppropertiesdlg.cpp cgeneratenewfile.cpp cfinddoctextdlg.cpp cfilepropdlg.cpp cexecuteargdlg.cpp ceditwidget.cpp cdoctreepropdlg.cpp cdocbrowser.cpp ccvaddfolderdlg.cpp ccreatedocdatabasedlg.cpp cclonefunctiondlg.cpp cclassview.cpp wzconnectdlgimpl.cpp cclasstooltreeview.cpp cclasstooldlg.cpp caddnewtranslationdlg.cpp caddexistingfiledlg.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp 
kdevelop_LDADD   = ./vc/libvc.a ./print/libkdevelopprint.la ./kwrite/libkdevelopkwrite.la ./gfxview/libgfxview.a ./dbg/libkdevelopdbg.la ./classwizard/libclasswizard.a ./kpp/libkpp.a ./classparser/libclassparser.a -lkfile -lkhtml -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)

SUBDIRS = classparser kpp classwizard dbg examples gfxview kwrite pics print templates tools vc widgets 

EXTRA_DIST = caddclassattributedlg.cpp caddclassattributedlg.h caddclassmethoddlg.cpp caddclassmethoddlg.h caddexistingfiledlg.cpp caddexistingfiledlg.h caddnewtranslationdlg.cpp caddnewtranslationdlg.h cclasstooldlg.cpp cclasstooldlg.h cclasstooltreeview.cpp cclasstooltreeview.h wzconnectdlgimpl.cpp cclasstreehandler.h cclassview.cpp cclassview.h cclonefunctiondlg.cpp cclonefunctiondlg.h ccreatedocdatabasedlg.cpp ccvaddfolderdlg.cpp ccvaddfolderdlg.h cdocbrowser.cpp cdocbrowser.h cdoctreepropdlg.cpp cdoctreepropdlg.h ceditwidget.cpp ceditwidget.h wzconnectdlgimpl.h cerrormessageparser.h cexecuteargdlg.cpp cexecuteargdlg.h cfilepropdlg.cpp cfilepropdlg.h cfinddoctextdlg.cpp cfinddoctextdlg.h cgeneratenewfile.cpp cgeneratenewfile.h cgrouppropertiesdlg.cpp cgrouppropertiesdlg.h ckdevaccel.h ckdevaccel.cpp cclasstreehandler.cpp ckdevelop.h ckdevelop_project.cpp ckdevelop_whatsthis.cpp ckdevinstall.cpp ckdevinstall.h ckdevsetupdlg.cpp ckdevsetupdlg.h clogfileview.cpp clogfileview.h cmakemanualdlg.cpp cmakemanualdlg.h cmakemanualdlgdata.cpp cnewclassdlg.cpp cnewclassdlg.h cnewfiledlg.cpp cnewfiledlg.h coutputwidget.cpp coutputwidget.h cprjoptionsdlg.h cproject.cpp cproject.h crealfileview.cpp crealfileview.h ctabctl.cpp ctabctl.h ctoolclass.cpp ctoolclass.h ctoolsconfigdlg.cpp ctoolsconfigdlg.h ctreehandler.cpp ctreehandler.h ctreeview.cpp ctreeview.h cupdatekdedocdlg.cpp cupdatekdedocdlg.h dbgpsdlg.cpp dbgpsdlg.h dbgtoolbar.cpp dbgtoolbar.h debug.h doctreeview.cpp doctreeview.h extractrc grepdialog.cpp grepdialog.h kdevelop-setup.desktop kdevelop.desktop kdeveloprc kdevelopui.rc kdevlistview.cpp kdevlistview.h klangcombo.cpp klangcombo.h kstartuplogo.cpp kstartuplogo.h ktipofday.cpp ktipofday.h ktipofdaydlg.ui lo16-app-kdevelop.png x-designer.desktop lo32-app-kdevelop.png lo32-mime-kdevelop_project.png main.cpp resource.h structdef.h ckdevelop_init.cpp wzconnectdlg.ui x-kdevelop-project.desktop ccreatedocdatabasedlg.h ckappwizard.cpp ckappwizard.h cprjoptionsdlg.cpp ckdevelop_noslot.cpp ckdevelop_classview.cpp ckdevelop.cpp designer.desktop hi16-app-kdevelop.png hi48-app-kdevelop.png linguist.desktop x-linguist.desktop cerrormessageparser.cpp docviewman.cpp docviewman.h ckonsolewidget.cpp ckonsolewidget.h 

####### kdevelop will overwrite this part!!! (end)############

INCLUDES= $(all_includes) -I$(srcdir)/kpp -Ikpp -Ikwrite -I$(srcdir)/widgets/qextmdi/include

METASOURCES = AUTO

# the library search path.
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevelop_LDADD += $(top_builddir)/kdevelop/widgets/qextmdi/src/libkdevelopqextmdi.la

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc

KDE_ICON = AUTO

data_DATA = kdevelop.desktop kdevelop-setup.desktop designer.desktop linguist.desktop
datadir = $(kde_appsdir)/Development

mime_DATA = x-kdevelop-project.desktop x-designer.desktop x-linguist.desktop
mimedir = $(kde_mimedir)/application

bin_SCRIPTS = extractrc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kdevelop.pot; \
	fi

